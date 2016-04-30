// Copyright © 2016 Canonical Ltd.
// Author: Loïc Molinari <loic.molinari@canonical.com>
//
// This file is part of Quick+.
//
// Quick+ is free software: you can redistribute it and/or modify it under the
// terms of the GNU Lesser General Public License as published by the Free
// Software Foundation; version 3.
//
// Quick+ is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
// details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Quick+. If not, see <http://www.gnu.org/licenses/>.

#include "performancemetrics.h"
#include <QtCore/QMutex>
#include <QtCore/QElapsedTimer>
#include <QtGui/QVector4D>
#include <QtGui/QOpenGLFunctions>
#if defined(QT_OPENGL_ES)
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif
#include <sys/times.h>

// BitmapText renders a monospaced bitmap Latin-1 encoded text (128 characters)
// stored in a single texture atlas using OpenGL. The font is generated by
// bitmap-text-builder and stored in the bitmaptextfont_p.h header.
class BitmapText
{
public:
    BitmapText();
    ~BitmapText();

    // Allocates/Deletes the OpenGL resources. finalise() is not called at
    // destruction, it must be explicitly called to free the resources at the
    // right time in a thread with the same OpenGL context bound than at
    // initialise().
    bool initialise();
    void finalise();

    // Sets the text. Characters below 32 and above 126 included are ignored
    // apart from line feeds (10). Implies a reallocation of internal data. Must
    // be called in a thread with the same OpenGL context bound than at
    // initialise().
    void setText(const char* text);

    // Updates the current text at the given index. In order to avoid expensive
    // layout updates, line feeds can't be added nor removed. Updates of
    // characters below 32 and above 126 in the new text are ignored.
    void updateText(const char* text, int index, int length);

    // Binds the BitmapText's shader program. Must be called prior to
    // setTransform, setOpacity and render calls.
    void bindProgram();

    // Sets the viewport size and text position. Origin is at top/left. Must be
    // set correctly prior to rendering for correct results. Must be called in a
    // thread with the same OpenGL context bound than at initialise().
    void setTransform(const QSize& viewportSize, const QPointF& position);

    // Sets the text opacity. Must be called in a thread with the same OpenGL
    // context bound than at initialise().
    void setOpacity(float opacity);

    // Renders the text. Must be called in a thread with the same OpenGL context
    // bound than at initialise().
    void render();

private:
    struct Vertex {
        float x, y, s, t;
    };
    enum {
        NotEmpty    = (1 << 0),
#if !defined(QT_NO_DEBUG)
        Initialised = (1 << 1)
#endif
    };

    QOpenGLFunctions* m_functions;
#if !defined QT_NO_DEBUG
    QOpenGLContext* m_context;
#endif
    Vertex* m_vertexBuffer;
    int* m_textToVertexBuffer;
    int m_textLength;
    int m_characterCount;
    int m_currentFont;
    GLuint m_program;
    GLint m_programTransform;
    GLint m_programOpacity;
    GLuint m_vertexShaderObject;
    GLuint m_fragmentShaderObject;
    GLuint m_texture;
    GLuint m_indexBuffer;
    quint8 m_flags;
};

// GPUTimer is a timer used to measure the amount of time taken by the GPU to
// fully complete a set of graphics commands. As opposed to a basic timer which
// would determine the time taken by the graphics driver to push the graphics
// commands in the command buffer from the CPU, this timer pushes dedicated
// synchronisation commands the command buffer, which the GPU signals whenever
// completeted. That allows to get very accurate GPU timings.
class GPUTimer
{
public:
    GPUTimer() :
#if !defined QT_NO_DEBUG
        m_context(nullptr), m_started(false),
#endif
        m_type(None) {}

    // Allocates/Deletes the OpenGL resources. finalise() is not called at
    // destruction, it must be explicitly called to free the resources at the
    // right time in a thread with the same OpenGL context bound than at
    // initialise().
    bool initialise();
    void finalise();

    // Starts/Stops the timer. stop() returns the time elapsed in nanoseconds
    // since the call to start(). Calling start()/stop() two times in a row
    // triggers an assertion in debug builds and leads to undefined results in
    // non-debug builds. Must be called in a thread with the same OpenGL context
    // bound than at initialise().
    void start();
    quint64 stop();

private:
    enum Type {
        None,
#if defined(QT_OPENGL_ES)
        KHRFence,
        NVFence,
#else
        ARBTimerQuery,
        EXTTimerQuery
#endif
    };

#if !defined QT_NO_DEBUG
    QOpenGLContext* m_context;
    bool m_started;
#endif
    Type m_type;

#if defined(QT_OPENGL_ES)
    struct {
        void (QOPENGLF_APIENTRYP genFencesNV)(GLsizei n, GLuint* fences);
        void (QOPENGLF_APIENTRYP deleteFencesNV)(GLsizei n, const GLuint* fences);
        void (QOPENGLF_APIENTRYP setFenceNV)(GLuint fence, GLenum condition);
        void (QOPENGLF_APIENTRYP finishFenceNV)(GLuint fence);
    } m_fenceNV;
    GLuint m_fence[2];

    struct {
        EGLSyncKHR (QOPENGLF_APIENTRYP createSyncKHR)(EGLDisplay dpy, EGLenum type,
                                                      const EGLint* attrib_list);
        EGLBoolean (QOPENGLF_APIENTRYP destroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync);
        EGLint (QOPENGLF_APIENTRYP clientWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags,
                                                      EGLTimeKHR timeout);
    } m_fenceSyncKHR;
    EGLSyncKHR m_beforeSync;

#else
    struct {
        void (QOPENGLF_APIENTRYP genQueries)(GLsizei n, GLuint* ids);
        void (QOPENGLF_APIENTRYP deleteQueries)(GLsizei n, const GLuint* ids);
        void (QOPENGLF_APIENTRYP beginQuery)(GLenum target, GLuint id);
        void (QOPENGLF_APIENTRYP endQuery)(GLenum target);
        void (QOPENGLF_APIENTRYP getQueryObjectui64v)(GLuint id, GLenum pname, GLuint64* params);
        void (QOPENGLF_APIENTRYP getQueryObjectui64vExt)(GLuint id, GLenum pname,
                                                         GLuint64EXT* params);
        void (QOPENGLF_APIENTRYP queryCounter)(GLuint id, GLenum target);
    } m_timerQuery;
    GLuint m_timer[2];
#endif
};

class PerformanceMetricsPrivate
{
public:
    PerformanceMetricsPrivate(QQuickWindow* window, bool overlayVisible);
    ~PerformanceMetricsPrivate();

    void setOverlayText(const QString& text);
    void setOverlayPosition(const QPointF& position);
    void setOverlayOpacity(float opacity);
    void setWindowUpdatePolicy(QuickPlusPerformanceMetrics::UpdatePolicy updatePolicy);
    void initialiseGpuResources();
    void windowSceneGraphInvalidated();
    void windowBeforeSynchronising();
    void windowAfterSynchronising();
    void windowBeforeRendering();
    void windowAfterRendering();
    void updateOverlayText();
    int cpuModel(char* buffer, int bufferSize);
    int keywordString(int index, char* buffer, int bufferSize);
    void parseOverlayText();
    void updateCpuUsage();
    void updateThreadCount();
    void updateMemoryUsage();

    // Flags.
    enum {
        Initialised       = (1 << 0),
        GpuTimerAvailable = (1 << 1),
        OverlayVisible    = (1 << 2),
        ContinuousUpdate  = (1 << 3),
        DirtyText         = (1 << 4),
        DirtyTransform    = (1 << 5),
        DirtyOpacity      = (1 << 6),
        Logging           = (1 << 7)
    };

    static const int maxOverlayIndices = 16;

    QQuickWindow* m_window;
    QIODevice* m_loggingDevice;

    char* m_overlayTextParsed;
    struct {
        quint16 counterIndex;
        quint16 overlayTextParsedIndex;
    } m_overlayIndices[maxOverlayIndices];
    quint8 m_overlayIndicesSize;
    QString m_overlayText;
    QPointF m_overlayPosition;
    float m_overlayOpacity;

    QFile m_defaultLoggingDevice;

    BitmapText m_bitmapText;
    GPUTimer m_gpuTimer;

    QElapsedTimer m_syncTimer;
    QElapsedTimer m_renderTimer;
    QElapsedTimer m_cpuTimer;

    quint16 m_cpuOnlineCores;
    quint16 m_pageSize;
    clock_t m_cpuTicks;
    struct tms m_cpuTimes;

    struct Counters {
        quint64 syncTime;
        quint64 renderTime;
        quint64 gpuRenderTime;
        quint32 frameNumber;
        quint32 cpuUsage;
        quint32 vszMemory;
        quint32 rssMemory;
        quint16 threadCount;
    } m_counters;

    QMutex m_mutex;
    quint8 m_flags;
};

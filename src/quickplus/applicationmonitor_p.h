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

#ifndef APPLICATIONMONITOR_P_H
#define APPLICATIONMONITOR_P_H

#include "applicationmonitor.h"
#include "events.h"
#include "overlay_p.h"
#include "gputimer_p.h"
#include "quickplusglobal_p.h"
#include <QtCore/QHash>
#include <QtCore/QTimer>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QElapsedTimer>
#include <QtCore/QRunnable>
#include <QtCore/QAtomicInteger>

// Extension to QuickPlusApplicationMonitor flags.
enum MonitorFlagEx {
    Started     = (1 << 8),
    StartedOnce = (1 << 9)
};

class LoggingThread : public QThread
{
public:
    LoggingThread();

    void run() override;
    void push(const QuickPlusEvent* event);
    void setLoggers(QuickPlusLogger** loggers, int count);
    LoggingThread* ref();
    void deref();

private:
    ~LoggingThread();

    enum {
        Waiting       = (1 << 0),
        JoinRequested = (1 << 1)
    };

    QuickPlusEvent* m_queue;
    QuickPlusLogger* m_loggers[QuickPlusApplicationMonitor::maxLoggers];
    int m_loggerCount;
    QMutex m_mutex;
    QWaitCondition m_condition;
    QAtomicInteger<quint32> m_refCount;
    qint8 m_queueIndex;
    qint8 m_queueSize;
    quint8 m_flags;
};

class ShowFilter : public QObject
{
    Q_OBJECT

private:
    bool eventFilter(QObject* object, QEvent* event) override;
};

class WindowMonitorDeleter : public QRunnable
{
public:
    WindowMonitorDeleter(WindowMonitor* monitor)
        : m_monitor(monitor) { DASSERT(monitor); }
    ~WindowMonitorDeleter();

    void run() override;

private:
    WindowMonitor* m_monitor;
};

class WindowMonitorFlagSetter : public QRunnable
{
public:
    WindowMonitorFlagSetter(WindowMonitor* monitor, quint8 flags)
        : m_monitor(monitor), m_flags(flags) { DASSERT(monitor); }
    ~WindowMonitorFlagSetter();

    void run() override {}

private:
    WindowMonitor* m_monitor;
    quint8 m_flags;
};

class WindowMonitor : public QObject
{
    Q_OBJECT

public:
    WindowMonitor(QQuickWindow* window, LoggingThread* loggingThread, quint8 flags, quint32 id);
    ~WindowMonitor();

    QQuickWindow* window() const { return m_window; }
    void setProcessEvent(const QuickPlusEvent& event);

private Q_SLOTS:
    void windowSceneGraphInitialised();
    void windowSceneGraphInvalidated();
    void windowBeforeSynchronising();
    void windowAfterSynchronising();
    void windowBeforeRendering();
    void windowAfterRendering();
    void windowFrameSwapped();
    void windowSceneGraphAboutToStop();

private:
    enum {
        GpuResourcesInitialised = (1 << 8),
        GpuTimerAvailable       = (1 << 9),
        SizeChanged             = (1 << 10)
    };

    bool gpuResourcesInitialised() const { return m_flags & GpuResourcesInitialised; }
    void setFlags(quint16 flags) { m_flags = flags | (m_flags & ~0xff); }
    void initialiseGpuResources();
    void finaliseGpuResources();

    LoggingThread* m_loggingThread;
    QQuickWindow* m_window;
    GPUTimer m_gpuTimer;
    Overlay m_overlay;  // Accessed from different threads (needs locking).
    QMutex m_mutex;
    QElapsedTimer m_sceneGraphTimer;
    QElapsedTimer m_deltaTimer;
    quint32 m_id;
    quint16 m_flags;
    QSize m_frameSize;
    QuickPlusEvent m_frameEvent;

    friend class WindowMonitorDeleter;
    friend class WindowMonitorFlagSetter;
};

#endif  // APPLICATIONMONITOR_P_H

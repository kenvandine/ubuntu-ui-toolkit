/*
 * Copyright 2016 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Loïc Molinari <loic.molinari@canonical.com>
 */

#include "ucshapetexturefactory_p.h"

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtGui/QImage>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>

// FIXME(loicm) Add a define to log ref counting info.

// Log shape rendering and gaussian blur performance timings.
#define PERF_DEBUG 0
#if PERF_DEBUG
#include <QtCore/QElapsedTimer>
#endif

// We use explicit template instantiation for UCShapeTextureFactory so that we
// can separate the implementation from the header. The drawback is that we have
// to keep it in sync with the shape items.
template class UCShapeTextureFactory<1>;  // Used by UCCornerMaterial (fill.[h,cpp]).
template class UCShapeTextureFactory<2>;  // Used by UCFrameCornerMaterial (frame.[h,cpp]).

// Reference counted wrapper for key hashed textures.
class KeyHash
{
public:
    KeyHash() : m_data(new QHash<quint32, UCShapeTexture>()), m_refCount(0) {}

    QHash<quint32, UCShapeTexture>* ref() {
        DASSERT(m_refCount < UINT_MAX);
        m_refCount++;
        return m_data;
    }
    quint32 unref() {
        DASSERT(m_refCount > 0);
        if (Q_UNLIKELY(--m_refCount == 0)) { DASSERT(m_data->empty()); delete m_data; }
        return m_refCount;
    }
#if !defined(QT_NO_DEBUG)
    QHash<quint32, UCShapeTexture>* data() const { return m_data; }
#endif

private:
    QHash<quint32, UCShapeTexture>* m_data;
    quint32 m_refCount;
};

static QHash<QOpenGLContext*, KeyHash> contextHash;
static QMutex contextHashMutex;

template <int N>
UCShapeTextureFactory<N>::UCShapeTextureFactory()
    : m_context(QOpenGLContext::currentContext())
{
    DASSERT(m_context);

    // Ref the texture hash associated with the current context.
    contextHashMutex.lock();
    m_keyHash = contextHash[m_context].ref();
    contextHashMutex.unlock();

    for (int i = 0; i < N; i++) {
        m_keys[i] = invalidKey;
    }
}

template <int N>
UCShapeTextureFactory<N>::~UCShapeTextureFactory()
{
    DASSERT(QOpenGLContext::currentContext() == m_context);

    // Unref current textures.
    int textureCount = 0;
    quint32 textureIds[N];
    for (int i = 0; i < N; i++) {
        auto it = m_keyHash->find(m_keys[i]);
        if (it != m_keyHash->end()) {
            if (it.value().unref() == 0) {
                textureIds[textureCount++] = it.value().id();
                m_keyHash->erase(it);
            }
        }
    }
    if (textureCount > 0) {
        m_context->functions()->glDeleteTextures(textureCount, textureIds);
    };

    // Unref the texture hash associated with the current context.
    contextHashMutex.lock();
    auto it = contextHash.find(m_context);
    DASSERT(it.value().data() == m_keyHash);
    if (it.value().unref() == 0) {
        contextHash.erase(it);
    }
    contextHashMutex.unlock();
}

template <int N>
quint32 UCShapeTextureFactory<N>::acquireTexture(
    int index, quint32 currentKey, quint32 newKey, bool* needsUpdate, bool* isNewTexture)
{
    DASSERT(index >= 0 && index < N);
    DASSERT(needsUpdate);
    DASSERT(isNewTexture);

    m_keys[index] = newKey;
    auto newIt = m_keyHash->find(newKey);
    auto currentIt = m_keyHash->find(currentKey);

    if (currentIt != m_keyHash->end() && currentIt.value().unref() == 0) {
        quint32 textureId = currentIt.value().id();
        m_keyHash->erase(currentIt);
        if (newIt == m_keyHash->end()) {
            m_keyHash->insert(newKey, UCShapeTexture(textureId));
            *needsUpdate = true;
            *isNewTexture = false;
            return textureId;
        } else {
            m_context->functions()->glDeleteTextures(1, &textureId);
            *needsUpdate = false;
            *isNewTexture = false;
            return newIt.value().ref();
        }
    } else {
        if (newIt == m_keyHash->end()) {
            quint32 textureId;
            m_context->functions()->glGenTextures(1, &textureId);
            m_keyHash->insert(newKey, UCShapeTexture(textureId));
            *needsUpdate = true;
            *isNewTexture = true;
            return textureId;
        } else {
            *needsUpdate = false;
            *isNewTexture = false;
            return newIt.value().ref();
        }
    }
}

// static
template <int N>
void renderShape(void* buffer, UCShapeType type, int radius, int stride)
{
    DASSERT(buffer);
    DASSERT(radius > 0);
    DASSERT(stride > 0);
    DASSERT((stride & 0x3) == 0);  // Ensure 32-bit alignment required by QImage.

    QImage image(buffer, radius, radius, stride, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);

    if (type == UCShapeType::Squircle) {
        // QSvgRenderer being reentrant, we use a static instance with local data.
        static QSvgRenderer svg(QByteArray(squircleSvg), 0);
        svg.render(&painter);
    } else {
        painter.setBrush(Qt::white);
        painter.setRenderHint(QPainter::Antialiasing, true);
        // Offsetting by 0.5 provides the best looking anti-aliasing.
        painter.drawEllipse(QRectF(-0.5, -0.5, radius * 2.0 + 0.5, radius * 2.0 + 0.5));
    }
}

// static
template <int N>
quint8* UCShapeTextureFactory<N>::renderMaskTexture(UCShapeType type, int radius)
{
    DASSERT(radius > 0);

    // FIXME(loicm) Add a detailed explanation and layout of the mask texture.

    const int border = 1;  // 1 pixel border around the edges for clamping reasons.
    // FIXME(loicm) Looks creepy (and the 2nd getStride is most likely useless
    //     since textureStride is 32).
    const int width = getStride(radius + 2 * border, 1, textureStride);
    const int stride = getStride(width, 1, 4);  // OpenGL unpack pixel storage is 4 by default.
    const int height = width;
    const int finalBufferSize = stride * height * sizeof(quint8);
    const int painterBufferSize = radius * radius * sizeof(quint32);
    quint8* __restrict bufferU8 = reinterpret_cast<quint8*>(
        alignedAlloc(32, finalBufferSize + painterBufferSize));
    quint32* __restrict painterBufferU32 = reinterpret_cast<quint32*>(&bufferU8[finalBufferSize]);

    // Render the shape with QPainter.
    memset(painterBufferU32, 0, painterBufferSize);
    renderShape(painterBufferU32, type, radius, radius * 4);

    // Initialise the top and left borders containing the 1 pixel border and
    // texture stride of the final buffer.
    const int offset = width - radius - border;
    memset(bufferU8, 0x00, stride * offset);
    for (int i = 0; i < radius + border; i++) {
        memset(&bufferU8[(i + offset) * stride], 0x00, offset);
        // Since QImage doesn't support floating-point formats, a conversion
        // from U32 to U8 is required once rendered (we just convert one of the
        // channel since the fill color is white).
        for (int j = 0; j < radius; j++) {
            bufferU8[(i + offset) * stride + j + offset] = painterBufferU32[i * radius + j] & 0xff;
        }
        bufferU8[(i + offset) * stride + width - border] = 0xff;
    }
    memset(&bufferU8[(height - border) * stride + offset], 0xff, radius + border);

    return bufferU8;
}

// FIXME(loicm) We should maybe use a texture atlas storing all the radii to
//     improve batching.
template <int N>
quint32 UCShapeTextureFactory<N>::maskTexture(int index, UCShapeType type, quint16 radius)
{
    DASSERT(index >= 0 && index < N);
    DASSERT((radius & 0xf000) == 0);
    DASSERT(QOpenGLContext::currentContext() == m_context);

    const quint32 currentKey = m_keys[index];
    const quint32 newKey = makeMaskTextureKey(type, radius);
    bool needsUpdate, isNewTexture;
    quint32 textureId = acquireTexture(index, currentKey, newKey, &needsUpdate, &isNewTexture);
    if (!needsUpdate) {
        return textureId;
    }

    // Bind the texture, initialise states and allocate space. The texture size
    // is a multiple of textureStride to allow GPUs to speed up uploads and
    // optimise storage.
    const int textureSize = maskTextureSize(radius);
    QOpenGLFunctions* funcs = m_context->functions();
    funcs->glBindTexture(GL_TEXTURE_2D, textureId);
    if (isNewTexture) {
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, textureSize, textureSize, 0,
                            GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    } else if (maskTextureSizeFromKey(currentKey) != textureSize) {
        funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, textureSize, textureSize, 0,
                            GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
    }

    // Render and upload texture data.
    quint8* buffer = (radius > 0) ?
        renderMaskTexture(type, radius) :
        static_cast<quint8*>(calloc(textureSize * textureSize, 1));
    funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize, GL_LUMINANCE,
                           GL_UNSIGNED_BYTE, buffer);
    free(buffer);

    return textureId;
}

// static
template <int N>
quint16* UCShapeTextureFactory<N>::renderShadowTexture(UCShapeType type, int radius, int shadow)
{
    // FIXME(loicm) shadow > 0 was asserted before but when the shadow item is
    //     shrinked because if its size, the given shadow can be 0. Rendering
    //     with shadow == 0 crashes, let's clamp to 1 for now.
    shadow = qMax(1, shadow);
    DASSERT(radius >= 0);

    const int shadowPlusRadius = shadow + radius;
    const int textureWidth = shadowPlusRadius + shadow;
    const int gutter = shadow;
    const int textureWidthPlusGutters = 2 * gutter + textureWidth;
    const int bufferSize = 2 * textureWidth * textureWidthPlusGutters * sizeof(float);
    quint16* __restrict dataU16 = static_cast<quint16*>(alignedAlloc(32, bufferSize));
    float* __restrict dataF32 = reinterpret_cast<float*>(dataU16);

    // FIXME(loicm) Try filling unset bytes instead.
    memset(dataU16, 0, bufferSize);

#if PERF_DEBUG
    QElapsedTimer timer;
    printf("texture rendering:\n  shape... ");
    timer.start();
#endif

    if (radius > 0) {
        renderShape(&dataF32[textureWidthPlusGutters * shadow + gutter + shadow], type, radius,
                    textureWidthPlusGutters * 4);
        for (int i = shadow; i < shadowPlusRadius; i++) {
            for (int j = shadow; j < shadowPlusRadius; j++) {
                const int index = i * textureWidthPlusGutters + gutter + j;
                dataF32[index] = (((quint32*)dataF32)[index] & 0xff) / 255.0f;
            }
        }
    }

#if PERF_DEBUG
    printf("%6.2f ms\n", timer.nsecsElapsed() * 0.000001f);
    printf("  quads... ");
    timer.start();
#endif

    // Fill bottom-right side of the corner.
    for (int i = shadow; i < shadowPlusRadius; i++) {
        for (int j = shadowPlusRadius; j < textureWidth + gutter; j++) {
            dataF32[i * textureWidthPlusGutters + gutter + j] = 1.0f;
        }
    }
    for (int i = shadowPlusRadius; i < textureWidth; i++) {
        for (int j = shadow; j < textureWidth + gutter; j++) {
            dataF32[i * textureWidthPlusGutters + gutter + j] = 1.0f;
        }
    }

#if PERF_DEBUG
    printf("%6.2f ms\n", timer.nsecsElapsed() * 0.000001f);
    printf("  hblur... ");
    timer.start();
#endif

    // Gaussian blur horizontal pass.
    const int gaussianIndex = shadow - 1;
    const float sumFactor = 1.0f / gaussianSums[gaussianIndex];
    const float* __restrict gaussianKernel = &gaussianKernels[gaussianOffsets[gaussianIndex]];
    float* __restrict tempF32 = &dataF32[textureWidthPlusGutters * textureWidth];
    for (int i = 0; i < textureWidth; i++) {
        const int index = textureWidthPlusGutters * i + gutter;
        const int offset = gutter + i;
        for (int j = 0; j < textureWidth; j++) {
            float sum = 0.0f;
            float* __restrict src = &dataF32[index + j];
            for (int k = -shadow; k <= shadow; k++) {
                sum += src[k] * gaussianKernel[k];
            }
            tempF32[textureWidthPlusGutters * j + offset] = sum * sumFactor;
        }
    }

#if PERF_DEBUG
    printf("%6.2f ms\n", timer.nsecsElapsed() * 0.000001f);
    printf("  gutters. ");
    timer.start();
#endif

    // Fill the bottom gutter of the temporary buffer by repeating last row.
    for (int i = 0; i < textureWidth; i++) {
        const float edge = tempF32[textureWidthPlusGutters * i + gutter + textureWidth - 1];
        float* __restrict dst = &tempF32[textureWidthPlusGutters * i + gutter + textureWidth];
        for (int j = 0; j < gutter; j++) {
            dst[j] = edge;
        }
    }

#if PERF_DEBUG
    printf("%6.2f ms\n", timer.nsecsElapsed() * 0.000001f);
    printf("  vblur... ");
    timer.start();
#endif

    // Gaussian blur vertical pass, shape masking (simply reuse the shape
    // already rendered), floating-point quantization to 8 bits and
    // store. Ensures the returned 16-bit buffer has a 4 bytes stride alignment
    // to fit the default OpenGL unpack alignment.
    const int stride = getStride(textureWidth, sizeof(quint16), 4);
    for (int i = 0; i < textureWidth; i++) {
        const int index = textureWidthPlusGutters * i + gutter;
        quint16* __restrict dst = &dataU16[stride * i];
        for (int j = 0; j < textureWidth; j++) {
            float sum = 0.0f;
            float* __restrict src = &tempF32[index + j];
            for (int k = -shadow; k <= shadow; k++) {
                sum += src[k] * gaussianKernel[k];
            }
            const float shadow = sum * sumFactor;
            const float shape = dataF32[i * textureWidthPlusGutters + j + gutter];
            const quint16 shadowU16 = (quint16) (shadow * 255.0f + 0.5f);
            const quint16 shapeU16 = (quint16) (shape * 255.0f + 0.5f);
            dst[j] = shapeU16 << 8 | shadowU16;
        }
    }

#if PERF_DEBUG
    printf("%6.2f ms\n", timer.nsecsElapsed() * 0.000001f);
#endif

    return dataU16;
}

template <int N>
quint32 UCShapeTextureFactory<N>::shadowTexture(
    int index, UCShapeType type, quint16 radius, quint16 shadow)
{
    DASSERT(index >= 0 && index < N);
    DASSERT((radius & 0xf000) == 0);
    DASSERT((shadow & 0xf000) == 0);
    DASSERT(QOpenGLContext::currentContext() == m_context);

    const quint32 currentKey = m_keys[index];
    const quint32 newKey = makeShadowTextureKey(type, radius, shadow);
    bool needsUpdate, isNewTexture;
    quint32 textureId = acquireTexture(index, currentKey, newKey, &needsUpdate, &isNewTexture);
    if (!needsUpdate) {
        return textureId;
    }

    // Bind the texture, initialise states and allocate space. The texture size
    // is a multiple of textureStride to allow GPUs to speed up uploads and
    // optimise storage.
    const int textureSize = shadowTextureSize(radius, shadow);
    QOpenGLFunctions* funcs = m_context->functions();
    funcs->glBindTexture(GL_TEXTURE_2D, textureId);
    if (isNewTexture) {
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        funcs->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, textureSize, textureSize, 0,
                            GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);
    } else if (shadowTextureSizeFromKey(currentKey) != textureSize) {
        funcs->glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, textureSize, textureSize, 0,
                            GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, NULL);
    }

    // FIXME(loicm) Should be filled at texture creation with the same size.
    quint16* buffer = (quint16*) calloc(textureSize * textureSize, 2);
    funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureSize, textureSize, GL_LUMINANCE_ALPHA,
                           GL_UNSIGNED_BYTE, buffer);
    free(buffer);

    // Render and upload texture data.
    const int size = 2 * shadow + radius;
    const int offset = textureSize - size;
    buffer = renderShadowTexture(type, radius, shadow);
    funcs->glTexSubImage2D(GL_TEXTURE_2D, 0, offset, offset, size, size, GL_LUMINANCE_ALPHA,
                           GL_UNSIGNED_BYTE, buffer);
    free(buffer);

    return textureId;
}

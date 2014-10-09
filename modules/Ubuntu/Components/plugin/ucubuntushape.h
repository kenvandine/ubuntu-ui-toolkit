/*
 * Copyright 2013-2014 Canonical Ltd.
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

#ifndef UCUBUNTUSHAPE_H
#define UCUBUNTUSHAPE_H

#include <QtQuick/QQuickItem>
#include <QtQuick/QSGNode>
#include <QtQuick/qsgtexture.h>
#include <QtGui/QOpenGLFunctions>

class UCUbuntuShape : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(HAlignment)
    Q_ENUMS(VAlignment)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QColor gradientColor READ gradientColor WRITE setGradientColor
               NOTIFY gradientColorChanged)
    Q_PROPERTY(QString radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(QVariant image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(bool stretched READ stretched WRITE setStretched NOTIFY stretchedChanged)
    Q_PROPERTY(HAlignment horizontalAlignment READ horizontalAlignment WRITE setHorizontalAlignment NOTIFY horizontalAlignmentChanged)
    Q_PROPERTY(VAlignment verticalAlignment READ verticalAlignment WRITE setVerticalAlignment NOTIFY verticalAlignmentChanged)
    Q_PROPERTY(QString borderSource READ borderSource WRITE setBorderSource NOTIFY borderSourceChanged)

public:
    UCUbuntuShape(QQuickItem* parent=0);

    enum Radius { SmallRadius, MediumRadius };
    enum Border { RawBorder, IdleBorder, PressedBorder };
    enum HAlignment { AlignLeft = 0, AlignHCenter = 1, AlignRight = 2 };
    enum VAlignment { AlignTop = 0, AlignVCenter = 1, AlignBottom = 2 };

    QColor color() const { return color_; }
    void setColor(const QColor& color);
    QColor gradientColor() const { return gradientColor_; }
    void setGradientColor(const QColor& gradientColor);
    QString radius() const { return radiusString_; }
    void setRadius(const QString& radius);
    QString borderSource() const { return borderSource_; }
    void setBorderSource(const QString& borderSource);
    QVariant image() const { return QVariant::fromValue(image_); }
    void setImage(const QVariant& image);
    bool stretched() const { return stretched_; }
    void setStretched(bool stretched);
    HAlignment horizontalAlignment() const { return hAlignment_; }
    void setHorizontalAlignment(HAlignment horizontalAlignment);
    VAlignment verticalAlignment() const { return vAlignment_; }
    void setVerticalAlignment(VAlignment verticalAlignment);

Q_SIGNALS:
    void colorChanged();
    void gradientColorChanged();
    void radiusChanged();
    void borderChanged();
    void imageChanged();
    void stretchedChanged();
    void horizontalAlignmentChanged();
    void verticalAlignmentChanged();
    void borderSourceChanged();

protected:
    virtual void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry);
    virtual QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*);

private:
    void updateFromImageProperties(QQuickItem* image);
    void connectToPropertyChange(QObject* sender, const char* property,
                                 QObject* receiver, const char* slot);
    void connectToImageProperties(QQuickItem* image);

private Q_SLOTS:
    void onImagePropertiesChanged();
    void onOpenglContextDestroyed();
    void gridUnitChanged();
    void providerDestroyed(QObject* object=0);

private:
    QSGTextureProvider* imageTextureProvider_;
    QColor color_;
    QRgb colorPremultiplied_;
    QColor gradientColor_;
    QRgb gradientColorPremultiplied_;
    bool gradientColorSet_;
    QString radiusString_;
    Radius radius_;
    QString borderSource_;
    Border border_;
    QQuickItem* image_;
    bool stretched_;
    HAlignment hAlignment_;
    VAlignment vAlignment_;
    float gridUnit_;
    QRectF geometry_;

    Q_DISABLE_COPY(UCUbuntuShape)
};

QML_DECLARE_TYPE(UCUbuntuShape)

#endif // UCUBUNTUSHAPE_H
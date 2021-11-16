/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#include "QskTextureRenderer.h"
#include "QskColorFilter.h"
#include "QskGraphic.h"
#include "QskSetup.h"

#include <qimage.h>
#include <qpainter.h>

#include <qquickwindow.h>
#include <qsgtexture.h>

QskTextureRenderer::PaintHelper::~PaintHelper()
{
}

QSGTexture *QskTextureRenderer::createTexture(
    QQuickWindow* window,
    RenderMode renderMode, const QSize& size, PaintHelper* helper )
{
    QImage image( size, QImage::Format_RGBA8888_Premultiplied );
    image.fill( Qt::transparent );

    {
        QPainter painter( &image );
        helper->paint( &painter, size );
    }
    return window->createTextureFromImage(image);
}

QSGTexture *QskTextureRenderer::createTextureFromGraphic(
    QQuickWindow* window,
    RenderMode renderMode, const QSize& size,
    const QskGraphic& graphic, const QskColorFilter& colorFilter,
    Qt::AspectRatioMode aspectRatioMode )
{
    class PaintHelper : public QskTextureRenderer::PaintHelper
    {
      public:
        PaintHelper( const QskGraphic& graphic,
                const QskColorFilter& filter, Qt::AspectRatioMode aspectRatioMode )
            : m_graphic( graphic )
            , m_filter( filter )
            , m_aspectRatioMode( aspectRatioMode )
        {
        }

        void paint( QPainter* painter, const QSize& size ) override
        {
            const QRect rect( 0, 0, size.width(), size.height() );
            m_graphic.render( painter, rect, m_filter, m_aspectRatioMode );
        }

      private:
        const QskGraphic& m_graphic;
        const QskColorFilter& m_filter;
        const Qt::AspectRatioMode m_aspectRatioMode;
    };

    PaintHelper helper( graphic, colorFilter, aspectRatioMode );
    return createTexture(window, renderMode, size, &helper );
}

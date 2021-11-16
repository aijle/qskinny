/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#ifndef QSK_GRAPHIC_NODE_H
#define QSK_GRAPHIC_NODE_H

#include "QskTextureRenderer.h"
#include <QSGSimpleTextureNode>

class QskGraphic;
class QskColorFilter;
class QQuickWindow;

class QSK_EXPORT QskGraphicNode : public QSGSimpleTextureNode
{
  public:
    QskGraphicNode();
    ~QskGraphicNode() override;

    void setGraphic( QQuickWindow*,
        const QskGraphic&, const QskColorFilter&,
        QskTextureRenderer::RenderMode, const QRectF&,
        Qt::Orientations mirrored = Qt::Orientations() );

  private:

    uint m_hash;
};

#endif

/******************************************************************************
 * QSkinny - Copyright (C) 2016 Uwe Rathmann
 * This file may be used under the terms of the QSkinny License, Version 1.0
 *****************************************************************************/

#ifndef QSK_PAINTED_NODE_H
#define QSK_PAINTED_NODE_H

#include "QskTextureRenderer.h"
#include <QSGSimpleTextureNode>

class QSK_EXPORT QskPaintedNode : public QSGSimpleTextureNode
{
  public:
    QskPaintedNode();
    ~QskPaintedNode() override;

    void update( QQuickWindow*,
        QskTextureRenderer::RenderMode, const QRect& );

  protected:
    virtual void paint( QPainter*, const QSizeF& ) = 0;

    // a hash value of '0' always results in repainting
    virtual uint hash() const = 0;

  private:
    class PaintHelper;

    uint m_hash;
};

#endif

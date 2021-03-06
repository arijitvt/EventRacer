/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CCRenderSurfaceDrawQuad_h
#define CCRenderSurfaceDrawQuad_h

#include "cc/CCDrawQuad.h"
#include <wtf/PassOwnPtr.h>

namespace WebCore {

class CCLayerImpl;

class CCRenderSurfaceDrawQuad : public CCDrawQuad {
    WTF_MAKE_NONCOPYABLE(CCRenderSurfaceDrawQuad);
public:
    static PassOwnPtr<CCRenderSurfaceDrawQuad> create(const CCSharedQuadState*, const IntRect&, CCLayerImpl*, const FloatRect& surfaceDamageRect, bool isReplica);

    CCLayerImpl* layer() const { return m_layer; }
    bool isReplica() const { return m_isReplica; }

    // The surface damage rect for the target surface this quad draws into.
    // FIXME: This can be removed once render surfaces get their own layer type.
    const FloatRect& surfaceDamageRect() const { return m_surfaceDamageRect; }

private:
    CCRenderSurfaceDrawQuad(const CCSharedQuadState*, const IntRect&, CCLayerImpl*, const FloatRect& surfaceDamageRect, bool isReplica);

    CCLayerImpl* m_layer;
    FloatRect m_surfaceDamageRect;
    bool m_isReplica;
};

}

#endif

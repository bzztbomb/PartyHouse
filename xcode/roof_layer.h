//
//  roof_layer.h
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#ifndef __haus_map__roof_layer__
#define __haus_map__roof_layer__

#include "layer.h"
#include "quad.h"

class RoofLayer : public Layer
{
public:
    RoofLayer(QuadSurface* roof_surface);
    
    // Layer
    virtual void render(cinder::gl::Fbo* frame);
    virtual void keyDown( cinder::app::KeyEvent event );
private:
    QuadSurface* mSurface;
    
    cinder::Vec2f translate(const cinder::Vec2f& target, const std::vector<cinder::Vec2f>& corners);
};

#endif /* defined(__haus_map__roof_layer__) */

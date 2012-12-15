//
//  layer.h
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#ifndef haus_map_layer_h
#define haus_map_layer_h

#include "cinder/gl/Fbo.h"

class Layer
{
public:
    virtual ~Layer() {};
    virtual void render(cinder::gl::Fbo* frame) {};
};

#endif

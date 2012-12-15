//
//  image_layer.h
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#ifndef haus_map_image_layer_h
#define haus_map_image_layer_h

#include "layer.h"
#include "cinder/gl/Texture.h"

class ImageLayer : public Layer
{
public:
    ImageLayer(const cinder::fs::path& path);
    
    // Layer
    virtual void render(cinder::gl::Fbo* frame);
private:
    cinder::gl::Texture mTexture;
};

#endif

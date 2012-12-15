//
//  color_block_layer.h
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#ifndef __haus_map__color_block_layer__
#define __haus_map__color_block_layer__

#include <vector>
#include "cinder/Color.h"
#include "layer.h"
#include "quad.h"

class ColorBlockLayer : public Layer
{
public:
    ColorBlockLayer(std::vector<QuadSurface>* surfaces);
    
    // Layer
    virtual void render(cinder::gl::Fbo* frame);
    virtual void keyDown( cinder::app::KeyEvent event );
private:
    std::vector<QuadSurface>* mSurfaces;
    std::vector<cinder::Color> mColors;
    
    void nextColors();
};

#endif /* defined(__haus_map__color_block_layer__) */

//
//  vu_layer.h
//  haus_map
//
//  Created by Brian Richardson on 12/16/12.
//
//

#ifndef __haus_map__vu_layer__
#define __haus_map__vu_layer__

#include "cinder/Color.h"
#include "layer.h"

class VULayer : public Layer
{
public:
    VULayer();
    
    void setVUMode(bool val) { mVUMode = val; }
    
    // Layer
    virtual void render(cinder::gl::Fbo* frame);
    virtual void keyDown( cinder::app::KeyEvent event );
private:
    float mLastTime;
    int nTop;
    bool mVUMode;
    std::vector<cinder::Color> mColors;
    float mTick;
    
    void nextColors();
};

#endif /* defined(__haus_map__vu_layer__) */

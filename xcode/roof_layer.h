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

class RoofPattern;

class RoofLayer : public Layer
{
public:
    enum {
        GRID_COLS = 30,
        GRID_ROWS = 10
    };

    RoofLayer(QuadSurface* roof_surface);
    virtual ~RoofLayer();

    void testPattern();
    void scanPattern();
    
    // Layer
    virtual void render(cinder::gl::Fbo* frame);
    virtual void keyDown( cinder::app::KeyEvent event );
private:
    // Grid
    cinder::TriMesh2d mGrid[GRID_COLS * GRID_ROWS];
    cinder::Color mPixels[GRID_COLS * GRID_ROWS];
    bool mGridInit;
    
    // Source surface
    QuadSurface* mSurface;
    
    // Patterns
    RoofPattern* mCurrPattern;
    
    cinder::Vec2f translate(const cinder::Vec2f& target, const std::vector<cinder::Vec2f>& corners);
    void buildGrid(cinder::gl::Fbo* frame);
    void activatePattern(RoofPattern* p);
};

#endif /* defined(__haus_map__roof_layer__) */

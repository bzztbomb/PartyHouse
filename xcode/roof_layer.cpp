//
//  roof_layer.cpp
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#include "roof_layer.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace std;

RoofLayer::RoofLayer(QuadSurface* roof_surface) :
    mSurface(roof_surface)
{
    
}

enum Corners
{
    c_ll = 0,
    c_ul = 1,
    c_ur = 2,
    c_lr = 3,
};

cinder::Vec2f RoofLayer::translate(const cinder::Vec2f& target, const std::vector<cinder::Vec2f>& corners)
{
    Vec2f row_start = corners[c_ul].lerp(target.y, corners[c_ll]);
    Vec2f row_end = corners[c_ur].lerp(target.y, corners[c_lr]);
    Vec2f pt = row_start.lerp(target.x, row_end);
    return pt;
}

// Layer
void RoofLayer::render(cinder::gl::Fbo* frame)
{
    float w = frame->getWidth();
    float h = frame->getHeight();
    
    vector<Vec2f> corners;
    
    for (auto t = mSurface->mesh.getTexCoords().begin(); t != mSurface->mesh.getTexCoords().end(); t++)
    {
        Vec2f p = *t;
        p.x *= w;
        p.y *= h;
        corners.push_back(p);
    }

    for (int j = 0; j < 29*2; j+=2)
    {
        for (int i = 0; i < 5; i++)
        {
            gl::color(Color::white());
            TriMesh2d tile;
            
            Vec2f ll((float) j * 1.0f / 29.0f, 0.2f + 0.2f * (float) i);
            Vec2f ul((float) j * 1.0f / 29.0f, 0.1f + 0.2f * (float) i);
            Vec2f ur(ll.x + (1.0f / 29.0f), 0.1f + 0.2f * (float) i);
            Vec2f lr(ll.x + (1.0f / 29.0f), 0.2f + 0.2f * (float) i);

            tile.appendVertex(translate(ll, corners));
            tile.appendVertex(translate(ul, corners));
            tile.appendVertex(translate(ur, corners));
            tile.appendVertex(translate(lr, corners));
            
            int vIdx0 = tile.getNumVertices() - 4;
            int vIdx1 = tile.getNumVertices() - 3;
            int vIdx2 = tile.getNumVertices() - 2;
            int vIdx3 = tile.getNumVertices() - 1;
            
            tile.appendTriangle( vIdx0, vIdx1, vIdx2 );
            tile.appendTriangle( vIdx0, vIdx2, vIdx3 );
            
            gl::draw(tile);
        }
    }
}

void RoofLayer::keyDown( cinder::app::KeyEvent event )
{
    
}

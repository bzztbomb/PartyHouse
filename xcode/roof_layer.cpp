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
    mSurface(roof_surface),
    mGridInit(false)
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

void RoofLayer::buildGrid(cinder::gl::Fbo* frame)
{
    vector<Vec2f> corners;

    float w = frame->getWidth();
    float h = frame->getHeight();

    for (auto t = mSurface->mesh.getTexCoords().begin(); t != mSurface->mesh.getTexCoords().end(); t++)
    {
        Vec2f p = *t;
        p.x *= w;
        p.y *= h;
        corners.push_back(p);
    }
    
    const float x_unit = 1.0f / ((float) GRID_COLS - 1);
    const float y_unit = 1.0f / (float) GRID_ROWS;
    for (int y = 0; y < GRID_ROWS; y++)
    {
        const float xoffset = (y%2==0) ? -x_unit*0.5f : 0.0f;
        for (int x = 0; x < GRID_COLS; x++)
        {
            // Skip this tile if it's off roof
            if ((x == GRID_COLS-1) && (y%2==1))
                continue;
            Vec2f ul((float) x * x_unit + xoffset, (float) y * y_unit);
            ul.x = max(ul.x, 0.0f);
            Vec2f ll(ul.x, ul.y + y_unit);
            Vec2f ur(ul.x + x_unit, ul.y);
            ur.x = min(ur.x, 1.0f);
            Vec2f lr(ur.x, ur.y + y_unit);
            
            TriMesh2d& tile = mGrid[y*GRID_COLS+x];
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
        }
    }
}

// Layer
void RoofLayer::render(cinder::gl::Fbo* frame)
{
    if (!mGridInit)
    {
        buildGrid(frame);
        mGridInit = true;
    }
    
    for (int i = 0; i < GRID_COLS * GRID_ROWS; i++)
    {
        gl::color(i%2==0 ? Color::white() : Color(1.0, 0.0f, 1.0f));
        gl::draw(mGrid[i]);
    }
}

void RoofLayer::keyDown( cinder::app::KeyEvent event )
{
    
}

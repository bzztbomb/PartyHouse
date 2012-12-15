//
//  color_block_layer.cpp
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#include "color_block_layer.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

ColorBlockLayer::ColorBlockLayer(std::vector<QuadSurface>* surfaces) :
    mSurfaces(surfaces)
{
    nextColors();
}

// Layer
void ColorBlockLayer::render(cinder::gl::Fbo* frame)
{
    float w = frame->getWidth();
    float h = frame->getHeight();
    auto color = mColors.begin();
    for (auto surf = mSurfaces->begin(); surf != mSurfaces->end(); surf++)
    {
        gl::color(*color++);
        if (color == mColors.end())
            color = mColors.begin();
        PolyLine2f quad = surf->mesh.getTexCoords();
        quad.setClosed();
        for (auto p = quad.getPoints().begin(); p != quad.getPoints().end(); p++)
        {
            p->x *= w;
            p->y *= h;
        }
        gl::drawSolid(quad);
    }
}

void ColorBlockLayer::nextColors()
{
    mColors.clear();
    for (int i = 0; i < 8; i++)
        mColors.push_back(Color(randFloat(), randFloat(), randFloat()));
}

void ColorBlockLayer::keyDown( cinder::app::KeyEvent event )
{
    if (event.getCode() == KeyEvent::KEY_SPACE)
        nextColors();
}
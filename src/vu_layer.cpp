//
//  vu_layer.cpp
//  haus_map
//
//  Created by Brian Richardson on 12/16/12.
//
//  22 levels, 816x301 pixels, @ 73,242
// image is 956x1024

#include "vu_layer.h"

#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"
#include "cinder/Timeline.h"

using namespace ci;
using namespace ci::app;

VULayer::VULayer()
{
    nextColors();
    mLastTime = timeline().getCurrentTime();
    nTop = 0;
    mVUMode = true;
    mTick = 1;
}

void VULayer::render(cinder::gl::Fbo* frame)
{
    float w = frame->getWidth();
    float h = frame->getHeight();
    auto color = mColors.begin();

    float curr_y = (242.0f / 1024.0f) * h;
    const float line_height = ((301.0f / 22.0f) / 1024.0f) * h;
    const float xstart = (73.0f / 956.0f) * w;
    const float xend = ((73.0f+816.0f) / 956.0f) * w;

    if (timeline().getCurrentTime() - mLastTime > mTick)
    {
        nTop = nTop > 0 ? nTop - 1 : 0;
        if ((nTop < 11) && (nTop > 0))
            nTop--;
    }
    
    const int numrows = 22;
    for (int i = 0; i < numrows; i++)
    {
        if (!mVUMode)
        {
            gl::color(*color++);
            if (color == mColors.end())
                color = mColors.begin();
        } else {
            if (numrows - i > nTop)
                gl::color(mColors[i] * 0.25f);
            else
                gl::color(mColors[i]);
        }
        gl::drawSolidRect(Rectf(xstart, curr_y, xend, curr_y + line_height));
        curr_y += line_height;
    }
}

void VULayer::nextColors()
{
    mColors.clear();
    for (int i = 0; i < 22; i++)
        mColors.push_back(Color(randFloat(), randFloat(), randFloat()));
}

void VULayer::keyDown( cinder::app::KeyEvent event )
{
    if (event.getCode() == KeyEvent::KEY_SPACE)
    {
        nTop = 22;
        nextColors();
    }
}
//
//  movie_layer.cpp
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#include "movie_layer.h"
#include "cinder/Rand.h"

using namespace ci;
using namespace ci::app;

MovieLayer::MovieLayer(const fs::path& path) :
    mColor(Color::white()),
    mColorCycle(false)
{
    mMovie = qtime::MovieGl(path);
    mMovie.setLoop();
    if (mMovie.hasAlpha())
        printf("has alpha!");
    mMovie.play();
}

void MovieLayer::render(gl::Fbo* frame)
{
    // Movie
    if (mMovie.hasAlpha())
        gl::enableAlphaBlending();
    gl::color(mColor);
    if (mMovie.getTexture())
        gl::draw(mMovie.getTexture(), frame->getBounds());
    gl::disableAlphaBlending();
}

void MovieLayer::keyDown( cinder::app::KeyEvent event )
{
    if ((event.getCode() == KeyEvent::KEY_SPACE) && mColorCycle)
    {
        mColor = Color(randFloat(), randFloat(), randFloat());
    }
}


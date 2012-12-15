//
//  movie_layer.cpp
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#include "movie_layer.h"

using namespace ci;

MovieLayer::MovieLayer(const fs::path& path)
{
    mMovie = qtime::MovieGl(path);
    mMovie.setLoop();
    mMovie.play();
}

void MovieLayer::render(gl::Fbo* frame)
{
    // Movie
    gl::color(Color::white());
    if (mMovie.getTexture())
        gl::draw(mMovie.getTexture(), frame->getBounds());
}

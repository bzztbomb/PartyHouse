//
//  movie_layer.h
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#ifndef __haus_map__movie_layer__
#define __haus_map__movie_layer__

#include "layer.h"
#include "cinder/qtime/QuickTime.h"

class MovieLayer : public Layer
{
public:
    MovieLayer(const cinder::fs::path& path);
    
    // Layer
    virtual void render(cinder::gl::Fbo* frame);
private:
    cinder::qtime::MovieGl	mMovie;
};

#endif /* defined(__haus_map__movie_layer__) */

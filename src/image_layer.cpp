#include "image_layer.h"
#include "cinder/ImageIo.h"

using namespace ci;

ImageLayer::ImageLayer(const fs::path& path)
{
    mTexture = gl::Texture(loadImage(path));
}

// Layer
void ImageLayer::render(gl::Fbo* frame)
{
    gl::color(Color::white());
    gl::draw(mTexture, frame->getBounds());
}

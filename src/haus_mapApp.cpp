#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct QuadSurface
{
    Rectf coords;
};

class haus_mapApp : public AppBasic {
public:
    void setup();
    void mouseDown( MouseEvent event );	
    void update();
    void draw();
    void prepareSettings(Settings* settings);
private:
    Rectf mInputRect;
    gl::Texture mInput;
    vector<QuadSurface> mSurfaces;
    
    Rectf surfToEditor(const Rectf& input);
    Rectf editorToSurf(const Rectf& input);
};

void haus_mapApp::prepareSettings(Settings* settings)
{
    settings->setTitle("Hausu");
}

void haus_mapApp::setup()
{
    mInput = gl::Texture(loadImage(loadResource("test.jpg")));
    QuadSurface q;
    q.coords = Rectf(0.0f, 0.0f, 1.0f, 1.0f);
    mSurfaces.push_back(q);
    const Rectf editor_rect = getWindowBounds();
    mInputRect = Rectf(editor_rect.x1, editor_rect.y2 / 2, editor_rect.x2 / 2, editor_rect.y2);
}

void haus_mapApp::mouseDown( MouseEvent event )
{
}

void haus_mapApp::update()
{
}

Rectf haus_mapApp::surfToEditor(const Rectf& input)
{
    Rectf result;
    result.x1 = input.x1 * (mInputRect.x2 - mInputRect.x1) + mInputRect.x1;
    result.x2 = input.x2 * (mInputRect.x2 - mInputRect.x1) + mInputRect.x1;
    result.y1 = input.y1 * (mInputRect.y2 - mInputRect.y1) + mInputRect.y1;
    result.y2 = input.y2 * (mInputRect.y2 - mInputRect.y1) + mInputRect.y1;
    return result;
}

Rectf haus_mapApp::editorToSurf(const Rectf& input)
{
    Rectf result;
    result.x1 = (input.x1 - mInputRect.x1) / (mInputRect.x2 - mInputRect.x1);
    result.x2 = (input.x2 - mInputRect.x1) / (mInputRect.x2 - mInputRect.x1);
    result.y1 = (input.y1 - mInputRect.y1) / (mInputRect.y2 - mInputRect.y1);
    result.y2 = (input.y2 - mInputRect.y1) / (mInputRect.y2 - mInputRect.y1);
    return result;
}

void haus_mapApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    //
    const Rectf editor_rect = getWindowBounds();
    const Rectf input_rect(editor_rect.x1, editor_rect.y2 / 2, editor_rect.x2 / 2, editor_rect.y2);
    gl::color(Color::white());
    gl::draw(mInput, input_rect);

    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
        const Rectf quad0 = surfToEditor(surf->coords);
        gl::color(1.0f, 1.0f, 1.0f);
        gl::drawStrokedRect(quad0);
        gl::color(1.0f, 1.0f, 0.0f);
        gl::drawSolidCircle(Vec2f(quad0.x1, quad0.y1), 4.0f);
        gl::drawSolidCircle(Vec2f(quad0.x1, quad0.y2), 4.0f);
        gl::drawSolidCircle(Vec2f(quad0.x2, quad0.y2), 4.0f);
        gl::drawSolidCircle(Vec2f(quad0.x2, quad0.y1), 4.0f);
    }
}


CINDER_APP_BASIC( haus_mapApp, RendererGl )

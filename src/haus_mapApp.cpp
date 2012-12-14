#include "cinder/ImageIo.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/TriMesh.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct QuadSurface
{
    TriMesh2d mesh;
};

class haus_mapApp : public AppBasic {
public:
    haus_mapApp();
    
    void setup();
    void mouseDown( MouseEvent event );
    void mouseDrag(MouseEvent event);
    void mouseUp( MouseEvent event );
    void update();
    void draw();
    void prepareSettings(Settings* settings);
private:
    Rectf mInputRect;
    gl::Texture mInput;
    vector<QuadSurface> mSurfaces;
    Vec2f* mActivePoint;
    
    PolyLine<Vec2f> surfToEditor(const PolyLine<Vec2f>& input);
    Vec2f surfToEditor(const Vec2f& input);
    Rectf editorToSurf(const Rectf& input);
    Vec2f editorToSurf(const Vec2f& input);
    void addSurface();
};

const float HANDLE_SIZE = 8.0f;

haus_mapApp::haus_mapApp() :
    mActivePoint(NULL)
{
    
}

void haus_mapApp::prepareSettings(Settings* settings)
{
    settings->setTitle("Hausu");
}

void haus_mapApp::setup()
{
    mInput = gl::Texture(loadImage(loadResource("test.jpg")));
    addSurface();
    const Rectf editor_rect = getWindowBounds();
    mInputRect = Rectf(editor_rect.x1, editor_rect.y2 / 2, editor_rect.x2 / 2, editor_rect.y2);
}

void haus_mapApp::addSurface()
{
    QuadSurface q;
    const GLfloat texCoords[8] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f };
	
	Vec2f verts[4];
    verts[0] = Vec2f(0.0f, 1.0f) * 100.0f;
    verts[1] = Vec2f(0.0f, 0.0f) * 100.0f;
    verts[2] = Vec2f(1.0f, 0.0f) * 100.0f;
    verts[3] = Vec2f(1.0f, 1.0f) * 100.0f;
	
	for (int i = 0; i < 4; i++)
	{
		q.mesh.appendVertex(verts[i]);
		q.mesh.appendTexCoord(Vec2f(texCoords[i*2], texCoords[i*2+1]));
	}
	
	// get the index of the vertex. not necessary with this example, but good practice
	int vIdx0 = q.mesh.getNumVertices() - 4;
	int vIdx1 = q.mesh.getNumVertices() - 3;
	int vIdx2 = q.mesh.getNumVertices() - 2;
	int vIdx3 = q.mesh.getNumVertices() - 1;
	
	// now create the triangles from the vertices
	q.mesh.appendTriangle( vIdx0, vIdx1, vIdx2 );
	q.mesh.appendTriangle( vIdx0, vIdx2, vIdx3 );
    
    mSurfaces.push_back(q);
}

void haus_mapApp::mouseDown( MouseEvent event )
{
    mActivePoint = NULL;
    const Vec2f ev_pos(event.getX(), event.getY());
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
        for (auto i = surf->mesh.getTexCoords().begin(); i != surf->mesh.getTexCoords().end(); i++)
        {
            Vec2f v = surfToEditor(*i) - ev_pos;
            if (v.lengthSquared() < HANDLE_SIZE*HANDLE_SIZE)
            {
                mActivePoint = &*i;
            }
        }
    }
}

void haus_mapApp::mouseDrag(MouseEvent event)
{
    if (mActivePoint)
        *mActivePoint = editorToSurf(Vec2f(event.getX(), event.getY()));
}

void haus_mapApp::mouseUp( MouseEvent event )
{
    mActivePoint = NULL;
}

void haus_mapApp::update()
{
}

Vec2f haus_mapApp::surfToEditor(const Vec2f& input)
{
    Vec2f v;
    v.x = input.x * (mInputRect.x2 - mInputRect.x1) + mInputRect.x1;
    v.y = input.y * (mInputRect.y2 - mInputRect.y1) + mInputRect.y1;
    return v;
}

PolyLine<Vec2f> haus_mapApp::surfToEditor(const PolyLine<Vec2f>& input)
{
    PolyLine<Vec2f> result;
    for (auto i = input.begin(); i != input.end(); i++)
    {
        result.push_back(surfToEditor(*i));
    }
    result.setClosed();
    return result;
}

Vec2f haus_mapApp::editorToSurf(const Vec2f& input)
{
    Vec2f result;
    result.x = (input.x - mInputRect.x1) / (mInputRect.x2 - mInputRect.x1);
    result.y = (input.y - mInputRect.y1) / (mInputRect.y2 - mInputRect.y1);
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

    // Surfaces
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
        // Texture coords
        PolyLine<Vec2f> quad0 = surfToEditor(surf->mesh.getTexCoords());        
        gl::color(1.0f, 1.0f, 1.0f);
        gl::draw(quad0);
        for (auto i = surf->mesh.getTexCoords().begin(); i != surf->mesh.getTexCoords().end(); i++)
        {
            if (&*i != mActivePoint)
                gl::color(1.0f, 1.0f, 0.0f);
            else
                gl::color(1.0f, 0.0f, 0.0f);
            gl::drawSolidCircle(surfToEditor(*i), HANDLE_SIZE);
        }
        
        // Output mesh
        gl::color(1.0f, 1.0f, 1.0f);
        mInput.enableAndBind();
        gl::draw(surf->mesh);
        mInput.unbind();
    }
}

CINDER_APP_BASIC( haus_mapApp, RendererGl )

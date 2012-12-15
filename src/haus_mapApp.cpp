#include "cinder/ImageIo.h"
#include "cinder/TriMesh.h"
#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"

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
    void resize( ResizeEvent event );
    void keyDown( KeyEvent event );
    void mouseDown( MouseEvent event );
    void mouseDrag(MouseEvent event);
    void mouseUp( MouseEvent event );
    void update();
    void draw();
    void prepareSettings(Settings* settings);
private:
    enum AppMode
    {
        amEditInput,
        amEditOutput,
        amPresent,
        amCount,
    };
    AppMode mAppMode;
    Rectf mInputRect;
    gl::Texture mInput;
    vector<QuadSurface> mSurfaces;
    
    Vec2f* mActiveInputPoint;
    Vec2f* mActiveOutputPoint;
    QuadSurface* mActiveSurface;
    Vec2f mDragStart;
    enum OutputEditMode
    {
        oemStandard,
        oemLowerLeft = 1, 
        oemUpperLeft = 2,
        oemUpperRight = 3,
        oemLowerRight = 4,
    };
    OutputEditMode mOutputEditMode;
    
    // Input
    gl::Fbo mFrame;
    
    PolyLine<Vec2f> surfToEditor(const PolyLine<Vec2f>& input);
    Vec2f surfToEditor(const Vec2f& input);
    Rectf editorToSurf(const Rectf& input);
    Vec2f editorToSurf(const Vec2f& input);
    void clearActive();
    void addSurface();
    void deleteCurrentSurface();
    void saveSurfaces(const fs::path& surf_path);
    void loadSurfaces(const fs::path& surf_path);
};

const float HANDLE_SIZE = 8.0f;

haus_mapApp::haus_mapApp() :
    mAppMode(amEditInput),
    mActiveInputPoint(NULL),
    mActiveOutputPoint(NULL),
    mActiveSurface(NULL),
    mOutputEditMode(oemStandard)
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
}

void haus_mapApp::resize( ResizeEvent event )
{
    mInputRect = getWindowBounds();
    gl::Fbo::Format format;
    int width = getWindowWidth();
    int height = getWindowHeight();
    mFrame = gl::Fbo(width, height, format);
}

void haus_mapApp::clearActive()
{
    mActiveInputPoint = NULL;
    mActiveOutputPoint = NULL;
    mActiveSurface = NULL;
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

void haus_mapApp::deleteCurrentSurface()
{
    if (mActiveSurface == NULL)
        return;
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
        if (&*surf == mActiveSurface)
        {
            mSurfaces.erase(surf);
            clearActive();
            return;
        }
    }
}

void haus_mapApp::saveSurfaces(const fs::path& surf_path)
{
    DataTargetRef dt = DataTargetPath::createRef(surf_path);
    OStreamRef stream = dt->getStream();
    
    stream->write(mSurfaces.size());
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
        TriMesh2d* t = &surf->mesh;
        stream->write(t->getTexCoords().size());
        stream->writeData(&t->getTexCoords()[0], t->getTexCoords().size() * sizeof(Vec2f));
        stream->write(t->getVertices().size());
        stream->writeData(&t->getVertices()[0], t->getVertices().size() * sizeof(Vec2f));
        stream->write(t->getIndices().size());
        stream->writeData(&t->getIndices()[0], t->getIndices().size() * sizeof(size_t));
    }
}

void haus_mapApp::loadSurfaces(const fs::path& surf_path)
{
    mSurfaces.clear();
    DataSourceRef ds = DataSourcePath::create(surf_path);
    IStreamRef stream = ds->createStream();
    
    size_t sz;
    stream->read(&sz);
    for (size_t i = 0; i < sz; i++)
    {
        QuadSurface q;
        size_t tc_sz;
        stream->read(&tc_sz);
        q.mesh.getTexCoords().resize(tc_sz);
        stream->readData(&q.mesh.getTexCoords()[0], tc_sz * sizeof(Vec2f));
        
        size_t v_sz;
        stream->read(&v_sz);
        q.mesh.getVertices().resize(tc_sz);
        stream->readData(&q.mesh.getVertices()[0], v_sz * sizeof(Vec2f));
        
        size_t i_sz;
        stream->read(&i_sz);
        q.mesh.getIndices().resize(i_sz);
        stream->readData(&q.mesh.getIndices()[0], i_sz * sizeof(size_t));
        
        mSurfaces.push_back(q);
    }
}

void haus_mapApp::keyDown( KeyEvent event )
{
    switch(event.getCode())
    {
		case KeyEvent::KEY_F1:
            {
                mAppMode++;
                if (mAppMode == amCount)
                    mAppMode = amEditInput;
            }
            break;
        case KeyEvent::KEY_f :
            {
                setFullScreen(!isFullScreen());
            }
            break;
        case KeyEvent::KEY_a :
            {
                addSurface();
            }
            break;
        case KeyEvent::KEY_s :
            {
                fs::path surf_path = getAppPath() / "surfaces.dat";
                saveSurfaces(surf_path);
            };
            break;
        case KeyEvent::KEY_l :
            {
                fs::path surf_path = getAppPath() / "surfaces.dat";
                loadSurfaces(surf_path);
            };
            break;
        case KeyEvent::KEY_BACKSPACE :
        case KeyEvent::KEY_DELETE :
            {
                deleteCurrentSurface();
            }
            break;
        case KeyEvent::KEY_LEFTBRACKET :
            {
                mOutputEditMode = (mOutputEditMode == oemStandard) ? oemUpperLeft : oemStandard;
            }
            break;
        case KeyEvent::KEY_RIGHTBRACKET :
            {
                mOutputEditMode = (mOutputEditMode == oemStandard) ? oemUpperRight : oemStandard;
            }
            break;
        case KeyEvent::KEY_SEMICOLON :
            {
                mOutputEditMode = (mOutputEditMode == oemStandard) ? oemLowerLeft : oemStandard;
            }
            break;
        case KeyEvent::KEY_QUOTE :
            {
                mOutputEditMode = (mOutputEditMode == oemStandard) ? oemLowerRight : oemStandard;
            }
            break;
	}
}
void haus_mapApp::mouseDown( MouseEvent event )
{
    clearActive();
    const Vec2f ev_pos(event.getX(), event.getY());
    mDragStart = ev_pos;
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
        if (mAppMode == amEditInput)
        {
            for (auto i = surf->mesh.getTexCoords().begin(); i != surf->mesh.getTexCoords().end(); i++)
            {
                Vec2f v = surfToEditor(*i) - ev_pos;
                if (v.lengthSquared() < HANDLE_SIZE*HANDLE_SIZE)
                {
                    mActiveInputPoint = &*i;
                    mActiveSurface = &*surf;
                }
            }
            if (mActiveSurface == NULL)
            {
                PolyLine2f points = surfToEditor(surf->mesh.getTexCoords());
                if (points.contains(ev_pos))
                    mActiveSurface = &*surf;
            }
        }
        
        if (mAppMode == amEditOutput)
        {
            for (auto i = surf->mesh.getVertices().begin(); i != surf->mesh.getVertices().end(); i++)
            {
                Vec2f v = *i - ev_pos;
                if (v.lengthSquared() < HANDLE_SIZE*HANDLE_SIZE)
                {
                    mActiveOutputPoint = &*i;
                    mActiveSurface = &*surf;
                }
            }
            if (mActiveSurface == NULL)
            {
                PolyLine2f points = surf->mesh.getVertices();
                if (points.contains(ev_pos))
                    mActiveSurface = &*surf;
            }
        }
    }
}

void haus_mapApp::mouseDrag(MouseEvent event)
{
    if (mAppMode == amEditInput)
    {
        if (mActiveInputPoint)
        {
            *mActiveInputPoint = editorToSurf(Vec2f(event.getX(), event.getY()));
        } else {
            if (mActiveSurface)
            {
                Vec2f cur_pos = Vec2f(event.getX(), event.getY());
                Vec2f diff_ss = cur_pos - mDragStart;
                Vec2f diff = editorToSurf(diff_ss);
                mDragStart = cur_pos;
                for (auto i = mActiveSurface->mesh.getTexCoords().begin(); i != mActiveSurface->mesh.getTexCoords().end(); i++)
                {
                    *i += diff;
                }
            }
        }
    }
    
    if (mAppMode == amEditOutput)
    {
        if (mActiveOutputPoint)
        {
            *mActiveOutputPoint = Vec2f(event.getX(), event.getY());
        } else {
            if (mOutputEditMode == oemStandard)
            {
                if (mActiveSurface)
                {
                    Vec2f cur_pos = Vec2f(event.getX(), event.getY());
                    Vec2f diff_ss = cur_pos - mDragStart;
                    mDragStart = cur_pos;
                    for (auto i = mActiveSurface->mesh.getVertices().begin(); i != mActiveSurface->mesh.getVertices().end(); i++)
                    {
                        *i += diff_ss;
                    }
                }
            } else {
                Vec2f cur_pos = Vec2f(event.getX(), event.getY());
                Vec2f diff_ss = cur_pos - mDragStart;
                mDragStart = cur_pos;
                for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
                {
                    int index = ((int) mOutputEditMode) - 1;
                    surf->mesh.getVertices()[index] += diff_ss;
                }
            }
        }
    }
}

void haus_mapApp::mouseUp( MouseEvent event )
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

void haus_mapApp::update()
{
    gl::SaveFramebufferBinding bindingSaver;
    mFrame.bindFramebuffer();
    gl::pushMatrices();
    gl::setViewport(mFrame.getBounds());
    gl::setMatricesWindow(mFrame.getBounds().getWidth(), mFrame.getBounds().getHeight());
    gl::scale(1.0f, -1.0f);
    gl::translate(0.0f, -getWindowHeight());

	gl::clear( Color::black() );
    
    // Static texture
    gl::color(Color::white());
    gl::draw(mInput, mFrame.getBounds());
    gl::popMatrices();
}

void haus_mapApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) );
    
    //
    if (mAppMode == amEditInput)
    {
        gl::color(Color::white());
        gl::draw(mFrame.getTexture(), mInputRect);

        // Surfaces
        for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
        {
            // Texture coords
            PolyLine<Vec2f> quad0 = surfToEditor(surf->mesh.getTexCoords());
            if (&*surf == mActiveSurface)
                gl::color(0.0f, 1.0f, 1.0f);
            else
                gl::color(1.0f, 1.0f, 1.0f);
            gl::draw(quad0);
            for (auto i = surf->mesh.getTexCoords().begin(); i != surf->mesh.getTexCoords().end(); i++)
            {
                if (&*i != mActiveInputPoint)
                    gl::color(1.0f, 1.0f, 0.0f);
                else
                    gl::color(1.0f, 0.0f, 0.0f);
                gl::drawSolidCircle(surfToEditor(*i), HANDLE_SIZE);
            }
        }
    }
    
    if ((mAppMode == amPresent) || (mAppMode == amEditOutput))
    {
        // Output mesh
        for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
        {
            // Output mesh
            gl::color(1.0f, 1.0f, 1.0f);
            mFrame.getTexture().enableAndBind();
            gl::draw(surf->mesh);
            mFrame.getTexture().unbind();
        }
    }
    
    if (mAppMode == amEditOutput)
    {
        // Output mesh coords
        for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
        {
            PolyLine<Vec2f> quad0 = surf->mesh.getVertices();
            quad0.setClosed();
            if (&*surf == mActiveSurface)
                gl::color(0.0f, 1.0f, 1.0f);
            else
                gl::color(1.0f, 1.0f, 1.0f);
            gl::draw(quad0);
            
            for (auto i = surf->mesh.getVertices().begin(); i != surf->mesh.getVertices().end(); i++)
            {
                if (&*i != mActiveOutputPoint)
                    gl::color(1.0f, 1.0f, 0.0f);
                else
                    gl::color(1.0f, 0.0f, 0.0f);
                gl::drawSolidCircle(*i, HANDLE_SIZE);
            }
        }
    }
}

CINDER_APP_BASIC( haus_mapApp, RendererGl )

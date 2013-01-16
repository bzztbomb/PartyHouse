#include "cinder/ImageIo.h"
#include "cinder/TriMesh.h"
#include "cinder/DataSource.h"
#include "cinder/DataTarget.h"
#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Fbo.h"
#include "cinder/qtime/QuickTime.h"
#include "quad.h"
#include "layer.h"
#include "image_layer.h"
#include "movie_layer.h"
#include "color_block_layer.h"
#include "roof_layer.h"
#include "vu_layer.h"
#include "EditorViewport.h"
#include "ControlPoints.h"
#include "surfaceCoordIterator.h"

using namespace ci;
using namespace ci::app;
using namespace std;

const fs::path BASE_PATH("/Users/bzztbomb/projects/haus_map/current/");

enum AppMode
{
  amEditInput,
  amEditOutput,
  amPresent,
  amCount,
};

enum OutputEditMode
{
  oemStandard,
  oemLowerLeft = 1,
  oemUpperLeft = 2,
  oemUpperRight = 3,
  oemLowerRight = 4,
};

struct WindowState
{
  AppMode mAppMode;
  QuadSurface* mActiveSurface;
  
  ci::app::WindowRef mWindow;
  
  // Global state
  std::vector<QuadSurface>& mSurfaces;
  
  EditorViewport mInputViewport;
  ControlPoints<SurfaceCoordIterator> mInputPoints;
  
  Vec2f* mActiveInputPoint;
  Vec2f* mActiveOutputPoint;
  Vec2f mDragStart;
  OutputEditMode mOutputEditMode;
  
  WindowState(const ci::app::WindowRef& window, std::vector<QuadSurface>& surfaces) :
    mAppMode(amEditInput),
    mWindow(window),
    mSurfaces(surfaces),
    mActiveOutputPoint(NULL),
    mActiveSurface(NULL),
    mDragStart(0.0f, 0.0f),
    mOutputEditMode(oemStandard),
    mInputPoints(std::bind(&WindowState::getSurfaceTexBegin, this),
                 std::bind(&WindowState::getSurfaceTexEnd, this),
                 std::bind(&WindowState::selectPt, this, std::_1),
                 std::bind(&WindowState::unselectPt, this, std::_1))
  {
    mInputViewport.setWindow(mWindow);
    mInputViewport.setViewportStyle(EditorViewport::EV_FULLSCREEN);
    mInputPoints.setVP(&mInputViewport);
  }
  
  void clearActive()
  {
    mActiveOutputPoint = NULL;
    mActiveSurface = NULL;
  }
  
  vector<Vec2f>::iterator getTexBegin(QuadSurface& s)
  {
    return s.mesh.getTexCoords().begin();
  }
  
  vector<Vec2f>::iterator getTexEnd(QuadSurface& s)
  {
    return s.mesh.getTexCoords().end();
  }
  
  SurfaceCoordIterator getSurfaceTexBegin()
  {
    return SurfaceCoordIterator(mSurfaces, mSurfaces.begin(),
                                   mSurfaces.begin() != mSurfaces.end() ?
                                   mSurfaces.begin()->mesh.getTexCoords().begin() : vector<Vec2f>::iterator(),
                                   std::bind(&WindowState::getTexBegin, this, std::_1),
                                   std::bind(&WindowState::getTexEnd, this, std::_1)
                                   );
  }

  SurfaceCoordIterator getSurfaceTexEnd()
  {
    return SurfaceCoordIterator(mSurfaces, mSurfaces.end(),
      mSurfaces.begin() != mSurfaces.end() ? mSurfaces.rbegin()->mesh.getTexCoords().end() : vector<Vec2f>::iterator(),
      std::bind(&WindowState::getTexBegin, this, std::_1),
      std::bind(&WindowState::getTexEnd, this, std::_1));
  }
  
  void selectPt(SurfaceCoordIterator p)
  {
    mActiveSurface = &(*p.mSurfaceIterator);
  }

  void unselectPt(SurfaceCoordIterator p)
  {
    
  }
};

class haus_mapApp : public AppBasic {
public:
  haus_mapApp();
  
  void setup();
  void resize( );
  void keyDown( KeyEvent event );
  void mouseDown( MouseEvent event );
  void mouseDrag(MouseEvent event);
  void mouseUp( MouseEvent event );
  void update();
  void draw();
  void prepareSettings(Settings* settings);
private:
  Rectf mInputRect;
  vector<QuadSurface> mSurfaces;
  
  // Input
  gl::Fbo mFrame;
  vector<Layer*> mCurrentLayers;
  
  PolyLine<Vec2f> surfToEditor(const PolyLine<Vec2f>& input);
  Vec2f surfToEditor(const Vec2f& input);
  Rectf editorToSurf(const Rectf& input);
  Vec2f editorToSurf(const Vec2f& input);
  
  void autoVert();
  
  void addSurface();
  void deleteCurrentSurface();
  void saveSurfaces(const fs::path& surf_path);
  void loadSurfaces(const fs::path& surf_path);
  
  // Layer management
  void clearLayers();
  void addLayer(Layer* layer);
  
  // Window management
  void createNewWindow();
};

//
// Main app
//
haus_mapApp::haus_mapApp()
{
  
}

void haus_mapApp::prepareSettings(Settings* settings)
{
  settings->setTitle("Party House!");
  settings->enableSecondaryDisplayBlanking(false);
}

void haus_mapApp::setup()
{
  addSurface();
  getWindow()->setUserData( new WindowState(getWindow(), mSurfaces) );
}

void haus_mapApp::resize( )
{
  static bool init = false;
  if (init)
    return;
  init = true;
  mInputRect = getWindowBounds();
  gl::Fbo::Format format;
  int width = getWindowWidth();
  int height = getWindowHeight();
  mFrame = gl::Fbo(width, height, format);
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
  WindowState *data = getWindow()->getUserData<WindowState>();

  if (data->mActiveSurface == NULL)
    return;
  for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
  {
    if (&*surf == data->mActiveSurface)
    {
      mSurfaces.erase(surf);
      data->clearActive();
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
  WindowState *data = getWindow()->getUserData<WindowState>();
  mInputRect = getWindowBounds();
  
  switch(event.getCode())
  {
		case KeyEvent::KEY_F1:
    {
      data->mAppMode++;
      if (data->mAppMode == amCount)
        data->mAppMode = amEditInput;
      
      data->mInputViewport.setEnabled(data->mAppMode == amEditInput);
    }
      break;
    case KeyEvent::KEY_f :
    {
      setFullScreen(!isFullScreen());
    }
      break;
    case KeyEvent::KEY_z :
    {
      if (data->mAppMode == amEditOutput)
        autoVert();
    }
      break;
    case KeyEvent::KEY_a :
    {
      addSurface();
    }
      break;
    case KeyEvent::KEY_s :
    {
      fs::path surf_path = BASE_PATH / "surfaces.dat";
      saveSurfaces(surf_path);
    };
      break;
    case KeyEvent::KEY_l :
    {
      fs::path surf_path = BASE_PATH / "surfaces.dat";
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
      data->mOutputEditMode = (data->mOutputEditMode == oemStandard) ? oemUpperLeft : oemStandard;
    }
      break;
    case KeyEvent::KEY_RIGHTBRACKET :
    {
      data->mOutputEditMode = (data->mOutputEditMode == oemStandard) ? oemUpperRight : oemStandard;
    }
      break;
    case KeyEvent::KEY_SEMICOLON :
    {
      data->mOutputEditMode = (data->mOutputEditMode == oemStandard) ? oemLowerLeft : oemStandard;
    }
      break;
    case KeyEvent::KEY_QUOTE :
    {
      data->mOutputEditMode = (data->mOutputEditMode == oemStandard) ? oemLowerRight : oemStandard;
    }
      break;
    case KeyEvent::KEY_c :
    {
      clearLayers();
    }
      break;
    case KeyEvent::KEY_1 :
    {
      addLayer(new ImageLayer(BASE_PATH / "align_helper.jpg"));
    }
      break;
    case KeyEvent::KEY_2 :
    {
      MovieLayer* ml = new MovieLayer(BASE_PATH / "glow.mov");
      ml->setColorCycle(true);
      addLayer(ml);
    }
      break;
    case KeyEvent::KEY_3 :
    {
      addLayer(new ColorBlockLayer(&mSurfaces));
    }
      break;
    case KeyEvent::KEY_4 :
    {
      if (mSurfaces.size() > 0)
      {
        RoofLayer* rl = new RoofLayer(&(*mSurfaces.begin()));
        rl->testPattern();
        addLayer(rl);
      }
    }
      break;
    case KeyEvent::KEY_5 :
    {
      if (mSurfaces.size() > 0)
      {
        RoofLayer* rl = new RoofLayer(&(*mSurfaces.begin()));
        rl->scanPattern();
        addLayer(rl);
      }
    }
      break;
    case KeyEvent::KEY_6 :
    {
      if (mSurfaces.size() > 0)
      {
        RoofLayer* rl = new RoofLayer(&(*mSurfaces.begin()));
        rl->sinPattern();
        addLayer(rl);
      }
    }
      break;
    case KeyEvent::KEY_7 :
    {
      addLayer(new MovieLayer(BASE_PATH / "dancers.mov"));
    }
      break;
    case KeyEvent::KEY_8 :
    {
      addLayer(new VULayer());
    }
      break;
    case KeyEvent::KEY_9 :
    {
      addLayer(new MovieLayer(BASE_PATH / "mouse.mov"));
    }
      break;
    case KeyEvent::KEY_w :
      {
        createNewWindow();
      }
      break;
    default :
    {
      for (auto layer = mCurrentLayers.begin(); layer != mCurrentLayers.end(); layer++)
      {
        (*layer)->keyDown(event);
      }
    }
      break;
	}
}
void haus_mapApp::mouseDown( MouseEvent event )
{
  WindowState *data = getWindow()->getUserData<WindowState>();
  mInputRect = getWindowBounds();

  data->clearActive();
  const Vec2f ev_pos(event.getX(), event.getY());
  data->mDragStart = ev_pos;
  for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
  {
    if (data->mAppMode == amEditOutput)
    {
      for (auto i = surf->mesh.getVertices().begin(); i != surf->mesh.getVertices().end(); i++)
      {
        Vec2f v = *i - ev_pos;
        if (v.lengthSquared() < HANDLE_SIZE*HANDLE_SIZE)
        {
          data->mActiveOutputPoint = &*i;
          data->mActiveSurface = &*surf;
        }
      }
      if (data->mActiveSurface == NULL)
      {
        PolyLine2f points = surf->mesh.getVertices();
        if (points.contains(ev_pos))
          data->mActiveSurface = &*surf;
      }
    }
  }
}

void haus_mapApp::mouseDrag(MouseEvent event)
{
  WindowState *data = getWindow()->getUserData<WindowState>();
  mInputRect = getWindowBounds();
  
  if (data->mAppMode == amEditOutput)
  {
    if (data->mActiveOutputPoint)
    {
      *data->mActiveOutputPoint = Vec2f(event.getX(), event.getY());
    } else {
      if (data->mOutputEditMode == oemStandard)
      {
        if (data->mActiveSurface)
        {
          Vec2f cur_pos = Vec2f(event.getX(), event.getY());
          Vec2f diff_ss = cur_pos - data->mDragStart;
          data->mDragStart = cur_pos;
          for (auto i = data->mActiveSurface->mesh.getVertices().begin(); i != data->mActiveSurface->mesh.getVertices().end(); i++)
          {
            *i += diff_ss;
          }
        }
      } else {
        Vec2f cur_pos = Vec2f(event.getX(), event.getY());
        Vec2f diff_ss = cur_pos - data->mDragStart;
        data->mDragStart = cur_pos;
        for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
        {
          int index = ((int) data->mOutputEditMode) - 1;
          surf->mesh.getVertices()[index] += diff_ss;
        }
      }
    }
  }
}

void haus_mapApp::mouseUp( MouseEvent event )
{
}

void haus_mapApp::update()
{
  gl::SaveFramebufferBinding bindingSaver;
  mFrame.bindFramebuffer();
  gl::pushMatrices();
  gl::setViewport(mFrame.getBounds());
  gl::setMatricesWindow(mFrame.getBounds().getWidth(), mFrame.getBounds().getHeight());
  gl::scale(1.0f, -1.0f);
  gl::translate(0.0f, -mFrame.getBounds().getHeight());
  
	gl::clear( Color::black() );
  
  // Static texture
  for (auto i = mCurrentLayers.begin(); i != mCurrentLayers.end(); i++)
  {
    (*i)->render(&mFrame);
  }
  
  gl::popMatrices();
}

void haus_mapApp::draw()
{
  WindowState *data = getWindow()->getUserData<WindowState>();
  mInputRect = getWindowBounds();

  gl::setViewport(getWindowBounds());
  
	// clear out the window with black
	gl::clear( Color::black() );
  
  //
  if (data->mAppMode == amEditInput)
  {
    data->mInputViewport.prepareForRender(true);
    gl::color(Color::white());
    const Area a = data->mInputViewport.getViewport();
    Rectf r(0, 0, a.getWidth(), a.getHeight());
    gl::draw(mFrame.getTexture(), r);
    
    // Surfaces
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
      // Texture coords
      PolyLine<Vec2f> quad0 = surfToEditor(surf->mesh.getTexCoords());
      if (&*surf == data->mActiveSurface)
        gl::color(0.0f, 1.0f, 1.0f);
      else
        gl::color(1.0f, 1.0f, 1.0f);
      gl::draw(quad0);
    }
    
    data->mInputPoints.render();
  }
  
  if ((data->mAppMode == amPresent) || (data->mAppMode == amEditOutput))
  {
    // Output mesh
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
      // Output mesh
      gl::color(Color::white());
      mFrame.getTexture().enableAndBind();
      gl::draw(surf->mesh);
      mFrame.getTexture().unbind();
    }
  }
  
  if (data->mAppMode == amEditOutput)
  {
    // Output mesh coords
    for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
    {
      PolyLine<Vec2f> quad0 = surf->mesh.getVertices();
      quad0.setClosed();
      if (&*surf == data->mActiveSurface)
        gl::color(0.0f, 1.0f, 1.0f);
      else
        gl::color(1.0f, 1.0f, 1.0f);
      gl::draw(quad0);
      
      for (auto i = surf->mesh.getVertices().begin(); i != surf->mesh.getVertices().end(); i++)
      {
        if (&*i != data->mActiveOutputPoint)
          gl::color(1.0f, 1.0f, 0.0f);
        else
          gl::color(1.0f, 0.0f, 0.0f);
        gl::drawSolidCircle(*i, HANDLE_SIZE);
      }
    }
  }
}

void haus_mapApp::autoVert()
{
  // Output mesh coords
  for (auto surf = mSurfaces.begin(); surf != mSurfaces.end(); surf++)
  {
    auto tc = surf->mesh.getTexCoords().begin();
    for (auto i = surf->mesh.getVertices().begin(); i != surf->mesh.getVertices().end(); i++)
    {
      *i = *tc++;
      i->x *= getWindowWidth();
      i->y *= getWindowHeight();
    }
  }
}

void haus_mapApp::createNewWindow()
{
	app::WindowRef newWindow = createWindow( Window::Format().size( 640, 480 ) );
	newWindow->setUserData( new WindowState( newWindow, mSurfaces ) );
}

//
// Layer management
//
void haus_mapApp::clearLayers()
{
  for (auto i = mCurrentLayers.begin(); i != mCurrentLayers.end(); i++)
  {
    delete (*i);
  }
  mCurrentLayers.clear();
}

void haus_mapApp::addLayer(Layer* layer)
{
  mCurrentLayers.push_back(layer);
}

//
// Coordinate xforming
//
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

CINDER_APP_BASIC( haus_mapApp, RendererGl )
//
//  roof_layer.cpp
//  haus_map
//
//  Created by Brian Richardson on 12/15/12.
//
//

#include "roof_layer.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "cinder/Timeline.h"
#include "cinder/Perlin.h"

using namespace ci;
using namespace std;
using namespace ci::app;

class RoofPattern
{
public:
    virtual ~RoofPattern() { }
    
    virtual void update(cinder::Color* pixels, int mCols, int mRows) { };
    virtual void reset() {};
};

//
class TestPattern : public RoofPattern
{
public:
    virtual void update(cinder::Color* pixels, int mCols, int mRows)
    {
        for (int i = 0; i < mCols * mRows; i++)
            pixels[i] = i%2==0 ? Color::white() : Color(1.0, 0.0f, 1.0f);
    }
};

class ScanPattern : public RoofPattern
{
public:
    ScanPattern(int cols, int rows) :
        mCurrIndex(0)
    {
        mSpread = 10.0f;
        reset();
    }

    virtual void reset()
    {
        mCurrIndex = 0;
        // TODO: REMOVE HACK
        timeline().apply(&mCurrIndex, (int)((30 * 10)+mSpread), 4.0f);
    }
    
    virtual ~ScanPattern()
    {
        TimelineItemRef t = timeline().find(&mCurrIndex);
        if (t != NULL)
            timeline().remove(t);
    }
    
    virtual void update(cinder::Color* pixels, int mCols, int mRows)
    {
        static bool debug = true;
        Color bright(0.98, 0.40, 0.1);
        Color dark(0.40, 0.98, 0.1);
        for (int i = 0; i < mCols * mRows; i++)
        {
            float diff = ((float) mCurrIndex - (float) i) / mSpread;
            diff = min(1.0f, diff);
            pixels[i] = i<mCurrIndex ? bright.lerp(diff, dark) : Color::black();
        }
        debug = false;
    }
private:
    Anim<int> mCurrIndex;
    float mSpread;
    Perlin mPerlin;
};

class SinPattern : public RoofPattern
{
public:
    SinPattern() :
        mT(0.0f),
        mLastTime(timeline().getCurrentTime()),
        mTimeMult(4)
    {
        
    }
    
    float sin11(float t)
    {
        return (sin(t) + 1.0f) * 0.5f;
    }
    
    virtual void update(cinder::Color* pixels, int mCols, int mRows)
    {
        mT += (timeline().getCurrentTime() - mLastTime) * mTimeMult;
        mLastTime = timeline().getCurrentTime();
        Color a(0.01, 0.01, 0.02);
        Color b(0.5, 0.1, 0.9);
        for (int y = 0; y < mRows; y++)
        {
            for (int x = 0; x < mCols; x++)
            {
                float x_sin = ((float) x / (float) mCols) * M_PI;
                float modulate = sin11(mT) * sin11(x_sin);
                float y_sin = ((float) y / (float) mRows) * M_PI;
                modulate *= (sin11(mT) / mT) * sin11(y_sin);
                pixels[y*mCols+x] = a.lerp(modulate, b);
            }
        }
    }
    
    virtual void reset()
    {
        mT = 0.0f;
    }
private:
    float mT;
    float mLastTime;
    float mTimeMult;
};

RoofLayer::RoofLayer(QuadSurface* roof_surface) :
    mSurface(roof_surface),
    mGridInit(false),
    mCurrPattern(NULL)
{
    scanPattern();
}

RoofLayer::~RoofLayer()
{
    delete mCurrPattern;
}

enum Corners
{
    c_ll = 0,
    c_ul = 1,
    c_ur = 2,
    c_lr = 3,
};

cinder::Vec2f RoofLayer::translate(const cinder::Vec2f& target, const std::vector<cinder::Vec2f>& corners)
{
    Vec2f row_start = corners[c_ul].lerp(target.y, corners[c_ll]);
    Vec2f row_end = corners[c_ur].lerp(target.y, corners[c_lr]);
    Vec2f pt = row_start.lerp(target.x, row_end);
    return pt;
}

void RoofLayer::buildGrid(cinder::gl::Fbo* frame)
{
    vector<Vec2f> corners;

    float w = frame->getWidth();
    float h = frame->getHeight();

    for (auto t = mSurface->mesh.getTexCoords().begin(); t != mSurface->mesh.getTexCoords().end(); t++)
    {
        Vec2f p = *t;
        p.x *= w;
        p.y *= h;
        corners.push_back(p);
    }
    
    const float x_unit = 1.0f / ((float) GRID_COLS - 1);
    const float y_unit = 1.0f / (float) GRID_ROWS;
    for (int y = 0; y < GRID_ROWS; y++)
    {
        const float xoffset = (y%2==0) ? -x_unit*0.5f : 0.0f;
        for (int x = 0; x < GRID_COLS; x++)
        {
            // Skip this tile if it's off roof
            if ((x == GRID_COLS-1) && (y%2==1))
                continue;
            Vec2f ul((float) x * x_unit + xoffset, (float) y * y_unit);
            ul.x = max(ul.x, 0.0f);
            Vec2f ll(ul.x, ul.y + y_unit);
            Vec2f ur(ul.x + x_unit, ul.y);
            ur.x = min(ur.x, 1.0f);
            Vec2f lr(ur.x, ur.y + y_unit);
            
            TriMesh2d& tile = mGrid[y*GRID_COLS+x];
            tile.appendVertex(translate(ll, corners));
            tile.appendVertex(translate(ul, corners));
            tile.appendVertex(translate(ur, corners));
            tile.appendVertex(translate(lr, corners));
            
            int vIdx0 = tile.getNumVertices() - 4;
            int vIdx1 = tile.getNumVertices() - 3;
            int vIdx2 = tile.getNumVertices() - 2;
            int vIdx3 = tile.getNumVertices() - 1;
            
            tile.appendTriangle( vIdx0, vIdx1, vIdx2 );
            tile.appendTriangle( vIdx0, vIdx2, vIdx3 );
            
            mLines[y*GRID_COLS+x] = tile.getVertices();
            mLines[y*GRID_COLS+x].setClosed();
        }
    }

    for (int i = 0; i < GRID_ROWS * GRID_COLS; i++)
        mPixels[i] = Color::black();
}

// Layer
void RoofLayer::render(cinder::gl::Fbo* frame)
{
    if (!mGridInit)
    {
        buildGrid(frame);
        mGridInit = true;
    }
  
    if (mCurrPattern)
        mCurrPattern->update(mPixels, GRID_COLS, GRID_ROWS);
    
    for (int i = 0; i < GRID_COLS * GRID_ROWS; i++)
    {
        gl::color(mPixels[i]);
        gl::draw(mGrid[i]);
        gl::color(Color::black());
        gl::draw(mLines[i]);
    }
}

void RoofLayer::activatePattern(RoofPattern* p)
{
    if (mCurrPattern)
        delete mCurrPattern;
    mCurrPattern = p;
}
void RoofLayer::testPattern()
{
    activatePattern(new TestPattern());
}

void RoofLayer::scanPattern()
{
    activatePattern(new ScanPattern(GRID_ROWS, GRID_COLS));
}

void RoofLayer::sinPattern()
{
    activatePattern(new SinPattern());
}

void RoofLayer::keyDown( cinder::app::KeyEvent event )
{
    if (event.getCode() == KeyEvent::KEY_SPACE)
    {
        if (mCurrPattern)
            mCurrPattern->reset();
    }
}

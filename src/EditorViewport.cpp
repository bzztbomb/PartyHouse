//
//  EditorViewport.cpp
//  dotsdotsdots
//
//  Created by Brian Richardson on 1/13/13.
//
//

#include "EditorViewport.h"

using namespace ci;

EditorViewport::EditorViewport() :
  mZoomFactor(1.0f),
  mTranslate(Vec2f::zero()),
  mStyle(EV_FULLSCREEN),
  mEnabled(true),
  mDomain(ED_UNIT)
{
  
}

void EditorViewport::setVP(const Area& vp)
{
  mViewport = vp;
  mStyle = EV_CUSTOM;
  updateTransforms();
}


void EditorViewport::setZoom(float factor)
{
  mZoomFactor = factor;
  updateTransforms();
}

void EditorViewport::setTranslate(const Vec2f& offset)
{
  mTranslate = offset;
  updateTransforms();
}

void EditorViewport::updateTransforms()
{
  const Vec2f domain = (mDomain == ED_UNIT) ? Vec2f(mViewport.getWidth(), mViewport.getHeight()) : Vec2f(1.0f, 1.0f);
  mUnitToVP = MatrixAffine2f::makeTranslate(mTranslate);
  mUnitToVP.scale(domain * mZoomFactor);

  // I'm doing this instead of a matrix invert to avoid
  // potential fp issues.
  Vec2f invSize(domain);
  invSize.x = 1.0f / invSize.x;
  invSize.y = 1.0f / invSize.y;
  Vec2f invTranslate(mTranslate * -1.0f);
  mVPtoUnit = MatrixAffine2f::makeScale(invSize / mZoomFactor);
  mVPtoUnit.translate(invTranslate);
  
  mScreenToVP = MatrixAffine2f::makeTranslate(Vec2f(-mViewport.x1, -(mWindow->getHeight() - mViewport.y2)));
}

// Sets up the gl transform for the editor
void EditorViewport::prepareForRender(bool full_xform)
{
  const Vec2f v = mViewport.getSize();
  gl::setMatricesWindow(v.x, v.y, true);
  gl::setViewport(mViewport);
  if (full_xform)
  {
    gl::translate(mTranslate);
    gl::scale(Vec2f(mZoomFactor, mZoomFactor));
  }
}

bool EditorViewport::shouldProcess(cinder::app::MouseEvent& event, cinder::Vec2f* translated)
{
  if (!getEnabled())
    return false;
  const Vec2f evPoint = screenToVP(Vec2f(event.getX(), event.getY()));
  if ((evPoint.x < 0.0f) || (evPoint.y < 0.0f) || evPoint.x > mViewport.getWidth() || (evPoint.y > mViewport.getWidth()))
    return false;
  *translated = evPoint;
  return true;
}

cinder::Vec2f EditorViewport::unitToVP(const cinder::Vec2f& p)
{
  return mUnitToVP.transformPoint(p);
}

cinder::Vec2f EditorViewport::VPtoUnit(const cinder::Vec2f& p)
{
  return mVPtoUnit.transformPoint(p);
}

cinder::Vec2f EditorViewport::VPtoUnitVec(const cinder::Vec2f& p)
{
  return mVPtoUnit.transformVec(p);
}

cinder::Vec2f EditorViewport::screenToVP(const cinder::Vec2f& p)
{
  return mScreenToVP.transformPoint(p);
}

void EditorViewport::setWindow(cinder::app::WindowRef win)
{
  mWindow = win;
  mWindow->getSignalResize().connect(std::bind(&EditorViewport::resize, this));
}

void EditorViewport::resize()
{
  Area a = mWindow->getBounds();
  switch (mStyle)
  {
    case EV_UPPER_LEFT:
      mViewport = Area(0, a.y2 / 2, a.x2 / 2, a.y2);
      break;
    case EV_UPPER_RIGHT:
      mViewport = Area(a.x2 / 2, a.y2 / 2, a.x2, a.y2);
      break;
    case EV_LOWER_LEFT:
      mViewport = Area(0, 0, a.x2 / 2, a.y2 / 2);
      break;
    case EV_LOWER_RIGHT :
      mViewport = Area(a.x2 / 2, 0, a.x2, a.y2 / 2);
      break;
    case EV_FULLSCREEN :
      mViewport = a;
      break;
    case EV_CUSTOM :
    default:
      break;
  }
  updateTransforms();
}
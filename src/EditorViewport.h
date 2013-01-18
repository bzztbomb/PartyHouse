//
//  EditorViewport.h
//  dotsdotsdots
//
//  Created by Brian Richardson on 1/13/13.
//
//

#ifndef __dotsdotsdots__EditorViewport__
#define __dotsdotsdots__EditorViewport__

#include "cinder/MatrixAffine2.h"
#include "cinder/Area.h"
#include "cinder/app/App.h"

class EditorViewport
{
public:
  enum EditorViewportStyle
  {
    EV_UPPER_LEFT,
    EV_UPPER_RIGHT,
    EV_LOWER_LEFT,
    EV_LOWER_RIGHT,
    EV_FULLSCREEN,
    EV_CUSTOM
  };
  
  enum EditorDomain
  {
    ED_UNIT,
    ED_PIXEL
  };
  
  EditorViewport();
  
  void setViewportStyle(EditorViewportStyle style) { mStyle = style; resize(); }
  void setDomain(EditorDomain domain) { mDomain = domain; }
  
  // Sets a custom viewport for the editor
  void setVP(const cinder::Area& vp);
  cinder::Area getViewport() { return mViewport; }
  
  // Sets up the gl transform for the editor
  void prepareForRender(bool full_xform);
  // Set zoom level
  void setZoom(float factor);
  float getZoom() const { return mZoomFactor; }
  // Set translation
  void setTranslate(const cinder::Vec2f& offset);
  cinder::Vec2f getTranslate() const { return mTranslate; }
  
  cinder::Vec2f unitToVP(const cinder::Vec2f& p);
  cinder::Vec2f VPtoUnit(const cinder::Vec2f& p);
  cinder::Vec2f VPtoUnitVec(const cinder::Vec2f& p);
  cinder::Vec2f screenToVP(const cinder::Vec2f& p);
  
  void setWindow(cinder::app::WindowRef win);
  cinder::app::WindowRef getWindow() { return mWindow; }
  
  void setEnabled(bool enable) { mEnabled = enable; }
  bool getEnabled() { return mEnabled; }
  
  bool shouldProcess(cinder::app::MouseEvent& event, cinder::Vec2f* translated);
private:
  cinder::app::WindowRef mWindow;
  EditorViewportStyle mStyle;
  EditorDomain mDomain;
  cinder::Area mViewport;
  
  float mZoomFactor;
  cinder::Vec2f mTranslate;
  
  bool mEnabled;
  
  cinder::MatrixAffine2f mUnitToVP;
  cinder::MatrixAffine2f mVPtoUnit;
  cinder::MatrixAffine2f mScreenToVP;
  
  void updateTransforms();
  void resize();
};


#endif /* defined(__dotsdotsdots__EditorViewport__) */

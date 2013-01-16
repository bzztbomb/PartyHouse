//
//  ControlPoints.h
//  dotsdotsdots
//
//  Created by Brian Richardson on 12/25/12.
//
//
//
// TODO:
// Better scaling, keep the selection box around?


#ifndef __dotsdotsdots__ControlPoints__
#define __dotsdotsdots__ControlPoints__

#include <vector>

#include "cinder/Vector.h"
#include "cinder/app/MouseEvent.h"

#include "EditorViewport.h"

#define HANDLE_SIZE 8.0f

template <class _InputIterator>
class ControlPoints
{
public:
  // Callback to get beginning/end iterators
  typedef std::function<_InputIterator()> GetIteratorFn;
  // Callback for point selection and unselection
  typedef std::function<void(_InputIterator)> SelectPointFn;

  ControlPoints(GetIteratorFn getBegin, GetIteratorFn getEnd) :
    mGetBegin(getBegin),
    mGetEnd(getEnd)
  {
    //
  }
  
  ControlPoints(GetIteratorFn getBegin, GetIteratorFn getEnd, SelectPointFn select, SelectPointFn unselect) :
    mGetBegin(getBegin),
    mGetEnd(getEnd),
    mSelectPoint(select),
    mUnselectPoint(unselect)
  {
  }
  
  //
  void setVP(EditorViewport* ev)
  {
    mEditorViewport = ev;
    ev->getWindow()->getSignalMouseDown().connect(std::bind(&ControlPoints::mouseDown, this, std::_1));
    ev->getWindow()->getSignalMouseDrag().connect(std::bind(&ControlPoints::mouseDrag, this, std::_1));
    ev->getWindow()->getSignalMouseUp().connect(std::bind(&ControlPoints::mouseUp, this, std::_1));
    ev->getWindow()->getSignalKeyDown().connect(std::bind(&ControlPoints::keyDown, this, std::_1));
  }

  // Point interface
  void select(_InputIterator pt)
  {
    mActivePoints.push_back(pt);
  }
  
  void clearSelection()
  {
    if (mUnselectPoint)
    {
      for (auto i = mActivePoints.begin(); i != mActivePoints.end(); i++)
        mUnselectPoint(*i);
    }
    mActivePoints.clear();
  }
  
  void render()
  {
    if (!mEditorViewport->getEnabled())
      return;
    
    mEditorViewport->prepareForRender(false);
    
    cinder::gl::color(cinder::Color(1,1,1));
    for (auto i = mGetBegin(); i != mGetEnd(); i++)
    {
      cinder::Vec2f p = mEditorViewport->unitToVP(*i);
      cinder::gl::drawSolidCircle(p, HANDLE_SIZE);
    }
    
    cinder::gl::color(cinder::Color(1,0,0));
    for (auto i = mActivePoints.begin(); i != mActivePoints.end(); i++)
    {
      cinder::Vec2f p = mEditorViewport->unitToVP(**i);
      cinder::gl::drawSolidCircle(p, HANDLE_SIZE);
    }
    
    if (mSelectionBoxActive)
    {
      cinder::gl::color(cinder::Color(1,1,1));
      cinder::gl::drawStrokedRect(mSelectionBox);
    }
  }
private:
  GetIteratorFn mGetBegin;
  GetIteratorFn mGetEnd;
  SelectPointFn mSelectPoint;
  SelectPointFn mUnselectPoint;

  EditorViewport* mEditorViewport;
  
  std::vector<_InputIterator> mActivePoints;

  enum EditMode
  {
    // Standard mode
    emSelectMove,
    // Define the selection box
    emSelectBox,
    // Move the viewport around
    emMoveViewport
  };
  EditMode mEditMode;
  cinder::Vec2f mDragStart;
  
  bool mSelectionBoxActive;
  cinder::Rectf mSelectionBox;
  cinder::Vec2f mSelectionBoxStart;
  
  void mouseDown( cinder::app::MouseEvent& event )
  {
    cinder::Vec2f evPoint;
    if (!mEditorViewport->shouldProcess(event, &evPoint))
      return;
    mDragStart = evPoint;
    
    bool point_found = false;
    for (auto i = mGetBegin(); i != mGetEnd(); i++)
    {
      cinder::Vec2f diff = mEditorViewport->unitToVP(*i) - evPoint;
      if (diff.lengthSquared() < HANDLE_SIZE*HANDLE_SIZE)
      {
        point_found = true;
        bool pointActive = find(mActivePoints.begin(), mActivePoints.end(), i) != mActivePoints.end();
        if (!pointActive)
        {
          if (!event.isMetaDown())
            clearSelection();
          mActivePoints.push_back(i);
          if (mSelectPoint)
            mSelectPoint(i);
        }
      }
    }
    
    event.setHandled();
    if (!point_found)
    {
      if (event.isMetaDown())
        mEditMode = emMoveViewport;
      else
        if (event.isAltDown())
          mEditMode = emSelectBox;
        else
          event.setHandled(false);
      mEditMode = event.isMetaDown() ? emMoveViewport : emSelectBox;
      clearSelection();
      mSelectionBoxActive = true;
      mSelectionBoxStart = evPoint;
      mSelectionBox = cinder::Rectf(evPoint, evPoint);
    } else {
      mEditMode = emSelectMove;
      mSelectionBoxActive = false;
    }    
  }
  
  void mouseDrag( cinder::app::MouseEvent& event )
  {
    cinder::Vec2f cur_pos;
    if (!mEditorViewport->shouldProcess(event, &cur_pos))
      return;
    
    switch (mEditMode)
    {
      case emSelectMove :
      {
        const cinder::Vec2f diff_ss = cur_pos - mDragStart;
        const cinder::Vec2f diff = mEditorViewport->VPtoUnitVec(diff_ss);
        mDragStart = cur_pos;
        
        if (!event.isMetaDown())
        {
          // Standard translate
          for (auto i = mActivePoints.begin(); i != mActivePoints.end(); i++)
          {
            cinder::Vec2f& v = **i;
            v += diff;
          }
        } else {
          // Scale
          std::vector<cinder::Vec2f> ap;
          for (auto i = mActivePoints.begin(); i != mActivePoints.end(); i++)
            ap.push_back(**i);
          const cinder::Vec2f center(cinder::Rectf(ap).getCenter());
          for (auto i = mActivePoints.begin(); i != mActivePoints.end(); i++)
          {
            cinder::Vec2f& p = **i;
            p.x += p.x < center.x ? diff.x : -diff.x;
            p.y += p.y < center.y ? diff.y : -diff.y;
          }
        }
      }
        break;
      case emSelectBox :
      {
        // Select points via box draw
        std::vector<cinder::Vec2f> points;
        points.push_back(mSelectionBoxStart);
        points.push_back(cur_pos);
        mSelectionBox = cinder::Rectf(points);
        cinder::Rectf normbox = cinder::Rectf(mEditorViewport->VPtoUnit(mSelectionBox.getUpperLeft()), mEditorViewport->VPtoUnit(mSelectionBox.getLowerRight()));
        mActivePoints.clear();
        for (auto i = mGetBegin(); i != mGetEnd(); i++)
        {
          if (normbox.contains(*i))
            mActivePoints.push_back(i);
        }
      }
        break;
      case emMoveViewport :
      {
        const cinder::Vec2f diff_ss = cur_pos - mDragStart;
        const cinder::Vec2f new_trans = mEditorViewport->getTranslate() + diff_ss;
        mDragStart = cur_pos;
        mEditorViewport->setTranslate(new_trans);
      }
        break;
    }
    event.setHandled();    
  }
  
  void mouseUp( cinder::app::MouseEvent& event )
  {
    cinder::Vec2f evPoint;
    if (!mEditorViewport->shouldProcess(event, &evPoint))
      return;
    
    mSelectionBoxActive = false;
    event.setHandled();
  }
  
  void keyDown( cinder::app::KeyEvent& event )
  {
    if (!mEditorViewport->getEnabled())
      return;
    if (!event.isMetaDown())
      return;
    const float ZOOM_AMT = 0.1f;
    switch (event.getCode()) {
      case cinder::app::KeyEvent::KEY_r:
      {
        mEditorViewport->setTranslate(cinder::Vec2f::zero());
        mEditorViewport->setZoom(1.0f);
      }
        break;
      case cinder::app::KeyEvent::KEY_KP_MINUS :
      case cinder::app::KeyEvent::KEY_MINUS :
      {
        mEditorViewport->setZoom(mEditorViewport->getZoom() - ZOOM_AMT);
      }
        break;
      case cinder::app::KeyEvent::KEY_KP_PLUS :
      case cinder::app::KeyEvent::KEY_EQUALS :
      {
        mEditorViewport->setZoom(mEditorViewport->getZoom() + ZOOM_AMT);
      }
        break;
      default:
        break;
    }
    event.setHandled();    
  }
};

#endif /* defined(__dotsdotsdots__ControlPoints__) */

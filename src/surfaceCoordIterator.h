//
//  surfaceCoordIterator.h
//  haus_map
//
//  Created by Brian Richardson on 1/15/13.
//
//

#ifndef haus_map_surfaceCoordIterator_h
#define haus_map_surfaceCoordIterator_h

#include <vector>
#include <functional>

#include "cinder/Vector.h"
#include "quad.h"

class SurfaceCoordIterator : public std::iterator<std::input_iterator_tag, cinder::Vec2f>
{
public:
  typedef std::function<std::vector<cinder::Vec2f>::iterator(QuadSurface&)> GetIterator;
  
  SurfaceCoordIterator(std::vector<QuadSurface>& surfaces,
                       std::vector<QuadSurface>::iterator surface,
                       std::vector<cinder::Vec2f>::iterator tex,
                        GetIterator getBegin,
                        GetIterator getEnd) :
  mSurfaces(surfaces),
  mSurfaceIterator(surface),
  mTexIterator(tex),
  mGetBegin(getBegin),
  mGetEnd(getEnd)
  {
  }
  
  SurfaceCoordIterator(const SurfaceCoordIterator& s) :
  mSurfaces(s.mSurfaces),
  mSurfaceIterator(s.mSurfaceIterator),
  mTexIterator(s.mTexIterator),
  mGetBegin(s.mGetBegin),
  mGetEnd(s.mGetEnd)
  {
  }
  
  SurfaceCoordIterator& operator++()
  {
    mTexIterator++;
    if (mTexIterator == mGetEnd(*mSurfaceIterator))
    {
      mSurfaceIterator++;
      if (mSurfaceIterator == mSurfaces.end())
      {
        return *this;
      }
      mTexIterator = mGetBegin(*mSurfaceIterator);
    }
    return *this;
  }
  
  SurfaceCoordIterator operator++(int)
  {
    SurfaceCoordIterator tmp(*this);
    operator++();
    return tmp;
  }
  
  bool operator==(const SurfaceCoordIterator& rhs)
  {
    return ((mSurfaceIterator == rhs.mSurfaceIterator) &&
            (mTexIterator == rhs.mTexIterator));
  }
  
  bool operator!=(const SurfaceCoordIterator& rhs)
  {
    return ((mSurfaceIterator != rhs.mSurfaceIterator) ||
            (mTexIterator != rhs.mTexIterator));
  }
  
  cinder::Vec2f& operator*()
  {
    return *mTexIterator;
  }
//private:
  std::vector<QuadSurface>& mSurfaces;
  std::vector<QuadSurface>::iterator mSurfaceIterator;
  std::vector<cinder::Vec2f>::iterator mTexIterator;
  GetIterator mGetBegin;
  GetIterator mGetEnd;
};

#endif

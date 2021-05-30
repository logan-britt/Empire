#ifndef PLACE
#define PLACE

#include "place_building.hpp"
#include "place_people.hpp"
#include "place_resorces.hpp"

#ifdef _WIN32
  #define dllexport __declspec( dllexport )
#else
  #define dllexport
#endif

namespace place {
  struct dllexport Scene
  {
  };

  void dllexport update(double dt, Scene* scene);
}

#endif 

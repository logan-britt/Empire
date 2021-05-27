#ifndef PLACE
#define PLACE

#include "place_building.hpp"
#include "place_people.hpp"
#include "place_resorces.hpp"

#define dllexport __declspec( dllexport )

namespace place {
  struct dllexport Scene
  {
  };

  void dllexport update(double dt, Scene* scene);
}

#endif 

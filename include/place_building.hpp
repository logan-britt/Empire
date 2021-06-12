#ifndef PLACE_BUILDINGS
#define PLACE_BUILDINGS

#include "place.hpp"
#include "place_people.hpp"
#include "place_resorces.hpp"

#ifdef _WIN32
  #define dllexport __declspec( dllexport )
#else
  #define dllexport
#endif

namespace place {
  struct dllexport Compound
  {
  };
}

#endif 

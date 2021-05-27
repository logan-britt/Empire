#ifndef PLACE_PEOPLE
#define PLACE_PEOPLE

#include "place.hpp"
#include "place_building.hpp"
#include "place_resorces.hpp"

#include <string>
#include <vector>

#define dllexport __declspec( dllexport )

namespace place {
  struct dllexport Person
  {
    std::string name;
    double weight;
    double height;

    double position[3];
    double velocity[3];
    double acceleration[3];
  };
}

#endif

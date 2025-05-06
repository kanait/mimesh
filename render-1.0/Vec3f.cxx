#include "Vec3f.h"
Vec3f Vec3f::operator=(Vec3f d) {
  x = d.x;
  y = d.y;
  z = d.z;
  return *this;
}

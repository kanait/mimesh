#ifndef __VEC_3F__
#define __VEC_3F__
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>

class Vec3f {
 public:
  double x, y, z;
  Vec3f operator=(Vec3f d);
};
/*
typedef struct Vec3f{
  double x,y,z;
}Vec3f;
Vec3f operator +(Vec3f &a, Vec3f &b)
{
  Vec3f out;
  out.x = a.x+b.x;
  out.y = a.y+b.y;
  out.z = a.z+b.z;
  return out;
}*/

#endif  // __VEC_3F__

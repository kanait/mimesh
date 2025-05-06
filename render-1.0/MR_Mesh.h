#ifndef __MR_MESH__
#define __MR_MESH__
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>

class MR_Mesh {
  // Bug info.
  //_ƒ‹[ƒg‚Ìmyself‚Ì’l‚ª‚¨‚©‚µ‚¢

 public:
  int _myself;
  MR_Mesh *_parent;
  MR_Mesh *_child[4];
  int _index[3];
  MR_Mesh();
  MR_Mesh(MR_Mesh *p_parent, int i0, int i1, int i2, int my);
  MR_Mesh operator=(MR_Mesh);
  void SetChild(MR_Mesh *p0, MR_Mesh *p1, MR_Mesh *p2, MR_Mesh *p3);
  void _print();
  int getID();

  void GetIdx(std::vector<int> *vidx, int lv);
};

#endif

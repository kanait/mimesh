#include "MR_Mesh.h"
MR_Mesh MR_Mesh::operator=(MR_Mesh _d) {
  int i;
  _myself = _d._myself;
  _parent = _d._parent;
  for (i = 0; i < 4; i++) _child[i] = _d._child[i];
  for (i = 0; i < 3; i++) _index[i] = _d._index[i];
  return *this;
}
MR_Mesh::MR_Mesh() { _myself = 0; }
MR_Mesh::MR_Mesh(MR_Mesh *p_parent, int i0, int i1, int i2, int my) {
  _parent = p_parent;
  _index[0] = i0;
  _index[1] = i1;
  _index[2] = i2;
  _myself = my;
}
void MR_Mesh::SetChild(MR_Mesh *p0, MR_Mesh *p1, MR_Mesh *p2, MR_Mesh *p3) {
  _child[0] = p0;
  _child[1] = p1;
  _child[2] = p2;
  _child[3] = p3;
}
void MR_Mesh::_print() {}
int MR_Mesh::getID() { return _myself; }

void MR_Mesh::GetIdx(std::vector<int> *vidx, int lv) {
  if (lv == 1) {
    vidx->push_back(_index[0]);
    vidx->push_back(_index[1]);
    vidx->push_back(_index[2]);
  } else {
    _child[0]->GetIdx(vidx, lv - 1);
    _child[1]->GetIdx(vidx, lv - 1);
    _child[2]->GetIdx(vidx, lv - 1);
    _child[3]->GetIdx(vidx, lv - 1);
  }
}

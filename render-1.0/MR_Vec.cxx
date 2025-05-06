#include "envDep.h"
#include "mydef.h"

#include <cstdio>
using namespace std;

#include "MR_Vec.h"
MR_Vec MR_Vec::operator=(MR_Vec d) {
  int i;
  for (i = 0; i < (int)d._vec.size(); i++) _vec.push_back(d._vec[i]);
  for (i = 0; i < (int)d._vdif.size(); i++) _vdif.push_back(d._vdif[i]);
  _flag = d._flag;
  _isStored = d._isStored;
  isParent = d.isParent;
  _out = d._out;
  _pos = d._pos;
  _p0 = d._p0;
  _p1 = d._p1;
  _myself = d._myself;
  return *this;
}
// コンストラクタ
MR_Vec::MR_Vec(std::vector<Vec3f> vec, int myself)  // 引数:座標値、自分自身のid
{
  _myself = 0;
  clearpos();
  for (int i = 0; i < (int)vec.size(); i++) {
    _vec.push_back(vec[i]);
    _vdif.push_back(vec[i]);
  }
  isParent = false;
  _isStored = false;
  _myself = myself;
};
void MR_Vec::IsParent() { isParent = true; }
bool MR_Vec::IsStore() { return _isStored; }
// 親を設定する．
void MR_Vec::SetParent(MR_Vec* p0, MR_Vec* p1)  //
{
  _p0 = p0;
  _p1 = p1;
  isParent = false;
  _isStored = true;
}
// フラグを消す
void MR_Vec::clearpos()  //
{
  _flag = false;
}
// 重みを用いて差分ベクトルを更新する．
void MR_Vec::UpdatePosition(std::vector<double> weight)  //
{
  if (isParent) {
    return;
    cerr << "#######\n";
    cerr << _pos.x << " " << _pos.y << " " << _pos.z << "\n";
    _pos.x = 0.0;
    _pos.y = 0.0;
    _pos.z = 0.0;
    for (int i = 0; i < (int)weight.size(); i++) {
      _pos.x += weight[i] * _vec[i].x;
      _pos.y += weight[i] * _vec[i].y;
      _pos.z += weight[i] * _vec[i].z;
    }

    cerr << _pos.x << " " << _pos.y << " " << _pos.z << "\n";

    return;
  }
  _flag = false;
  _pos.x = 0.0;
  _pos.y = 0.0;
  _pos.z = 0.0;

  for (int i = 0; i < (int)weight.size(); i++) {
    _pos.x += weight[i] * _vdif[i].x;
    _pos.y += weight[i] * _vdif[i].y;
    _pos.z += weight[i] * _vdif[i].z;
  }
  //  cerr<<"#######\n";
  // cerr<<_pos.x<<" "<<_pos.y<<" "<<_pos.z<<"\n";
}
// 実際の三次元座標を計算する．
Vec3f MR_Vec::GetPosition()  //
{
  return _pos;
}

// 実際の三次元座標を計算する．
Vec3f MR_Vec::GetPosition2(std::vector<double> weight)  //
{
  _pos.x = 0.0;
  _pos.y = 0.0;
  _pos.z = 0.0;
  for (int i = 0; i < (int)weight.size(); i++) {
    _pos.x += weight[i] * _vdif[i].x;
    _pos.y += weight[i] * _vdif[i].y;
    _pos.z += weight[i] * _vdif[i].z;
  }
  return _pos;
}
// テスト結果を出力する。
void MR_Vec::_print()  //
{
  return;
  Vec3f p = GetPosition();
  cerr << "=======\n";
  cerr << "id:" << WhoAmI();
  if (!isParent)
    cerr << "parent(" << _p0->WhoAmI() << "," << _p1->WhoAmI() << ")";
  else
    cerr << "Root";
  cerr << "\n";
}
// 頂点座標値を差分形式にする。
void MR_Vec::calcDif()  //
{
  if (isParent == true) {
    for (int i = 0; i < (int)_vec.size(); i++) {
      _vdif[i].x = _vec[i].x;
      _vdif[i].y = _vec[i].y;
      _vdif[i].z = _vec[i].z;
    }
    return;
  }
  for (int i = 0; i < (int)_vec.size(); i++) {
    Vec3f d0 = _p0->_vec[i];
    Vec3f d1 = _p1->_vec[i];

    _vdif[i].x = _vec[i].x - (d0.x + d1.x) * 0.5;
    _vdif[i].y = _vec[i].y - (d0.y + d1.y) * 0.5;
    _vdif[i].z = _vec[i].z - (d0.z + d1.z) * 0.5;
  }
}

// Openglに渡すための頂点座標を加える
void MR_Vec::AppendPosition(std::vector<double>& _v)  //
{
  Vec3f dv = GetPosition();
  _v.push_back(dv.x);
  _v.push_back(dv.y);
  _v.push_back(dv.z);
  return;
}
int MR_Vec::WhoAmI() { return _myself; }

void MR_Vec::SetPosition(Vec3f vec) {
  if (isParent == true) {
    _pos.x = vec.x;
    _pos.y = vec.y;
    _pos.z = vec.z;
  } else {
    _pos.x += vec.x;
    _pos.y += vec.y;
    _pos.z += vec.z;
  }
  // cerr<<" ==== \n";
  // cerr<<vec.x<<" "<<vec.y<<" "<<vec.z<<"\n";
  // cerr<<_pos.x<<" "<<_pos.y<<" "<<_pos.z<<"\n";

};  //

#ifndef __MR_VEC__
#define __MR_VEC__
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>
#include "Vec3f.h"

// classMR_Vec
// 補間メッシュの頂点を表す．
class MR_Vec {
 private:
  int _myself;       // 自分自身のID
  Vec3f _pos, _out;  // 補間座標値（差分ベクトル）

  bool _flag;      //_posが更新されたかどうか
  bool _isStored;  //_posが更新されたかどうか
  bool isParent;
  std::vector<Vec3f> _vdif;  // 差分ベクトル
  std::vector<Vec3f> _vec;   // 各頂点に割り当てられた座標値
  MR_Vec *_p0, *_p1;         // 自分の親頂点へのポインタ
 public:
  MR_Vec operator=(MR_Vec _d);
  // コンストラクタ
  MR_Vec() { std::cerr << "hoge\n"; };         // 引数:座標値、自分自身のid
  MR_Vec(std::vector<Vec3f> vec, int myself);  // 引数:座標値、自分自身のid
  void IsParent();
  // 親を設定する．
  void SetParent(MR_Vec* p0, MR_Vec* p1);  //
  // フラグを消す
  void clearpos();  //
  // 重みを用いて差分ベクトルを更新する．
  void UpdatePosition(std::vector<double> weight);  //
  // 実際の三次元座標を計算する．
  Vec3f GetPosition();                             //
  void SetPosition(Vec3f vec);                     //
  Vec3f GetPosition2(std::vector<double> weight);  //
  // テスト結果を出力する。
  void _print();  //
  // 頂点座標値を差分形式にする。
  void calcDif();  //
  //  void SetDif(std::vector<Vec3f&>);//
  // Openglに渡すための頂点座標を加える
  void AppendPosition(std::vector<double>& _v);  //
  int WhoAmI();
  bool IsStore();
};

#endif  //__MR_VEC__

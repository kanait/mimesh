#ifndef __MR_VEC__
#define __MR_VEC__
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>
#include "Vec3f.h"

// classMR_Vec
// ��ԃ��b�V���̒��_��\���D
class MR_Vec {
 private:
  int _myself;       // �������g��ID
  Vec3f _pos, _out;  // ��ԍ��W�l�i�����x�N�g���j

  bool _flag;      //_pos���X�V���ꂽ���ǂ���
  bool _isStored;  //_pos���X�V���ꂽ���ǂ���
  bool isParent;
  std::vector<Vec3f> _vdif;  // �����x�N�g��
  std::vector<Vec3f> _vec;   // �e���_�Ɋ��蓖�Ă�ꂽ���W�l
  MR_Vec *_p0, *_p1;         // �����̐e���_�ւ̃|�C���^
 public:
  MR_Vec operator=(MR_Vec _d);
  // �R���X�g���N�^
  MR_Vec() { std::cerr << "hoge\n"; };         // ����:���W�l�A�������g��id
  MR_Vec(std::vector<Vec3f> vec, int myself);  // ����:���W�l�A�������g��id
  void IsParent();
  // �e��ݒ肷��D
  void SetParent(MR_Vec* p0, MR_Vec* p1);  //
  // �t���O������
  void clearpos();  //
  // �d�݂�p���č����x�N�g�����X�V����D
  void UpdatePosition(std::vector<double> weight);  //
  // ���ۂ̎O�������W���v�Z����D
  Vec3f GetPosition();                             //
  void SetPosition(Vec3f vec);                     //
  Vec3f GetPosition2(std::vector<double> weight);  //
  // �e�X�g���ʂ��o�͂���B
  void _print();  //
  // ���_���W�l�������`���ɂ���B
  void calcDif();  //
  //  void SetDif(std::vector<Vec3f&>);//
  // Opengl�ɓn�����߂̒��_���W��������
  void AppendPosition(std::vector<double>& _v);  //
  int WhoAmI();
  bool IsStore();
};

#endif  //__MR_VEC__

#ifndef __MR_PATCH_
#define __MR_PATCH_

// #ifdef _WIN32
// #include <windows.h>
// #endif

#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <vector>
#include "Vec3f.h"
#include "MR_Mesh.h"
#include "MR_Vec.h"

//#include "extdef.h"

//using namespace std;

class MR_Patch{
 private:
  std::vector<MR_Vec> _vec;
  std::vector<MR_Mesh> _mesh;
  std::vector<double> param;
  int _mesh_res;
  int count;
 // double param;
  std::vector<int> index;
  std::vector<int> idx_table;
  int dif_table[10][500];
  int ic[10];
 public:
  MR_Patch operator=(MR_Patch d)
  {
    _mesh_res = d._mesh_res;
	int i,j;
		for(i =0;i<(int)d._vec.size();i++)_vec.push_back(d._vec[i]);
    for(i =0;i<(int)d._mesh.size();i++)_mesh.push_back(d._mesh[i]);
    count=d.count;
    param=d.param;
    index=d.index;
    for(i=0;i<10;i++)
     	for(j=0;j<500;j++)
		 dif_table[i][j] = d.dif_table[i][j];
   for(i=0;i<(int)idx_table.size();i++)
            idx_table.push_back(d.idx_table[i]);
    for(i=0;i<(int)idx_table.size();i++)ic[i] = d.ic[i];

    tcoords = d.tcoords;

    light[0] = 1.0;
    light[1] = 0.0;
    light[2] = 0.0;
    return *this;
  }
  MR_Patch(){};
  MR_Patch(std::vector<double>& src,std::vector<double>& trg,int mesh_res);
  void CreateIndex(int n);
  void MakeVTree(int myself,int p0,int p1,int lv);
  MR_Mesh* QTree(MR_Mesh* _p_parent,int x0,int y0,int x1,int y1,int x2,int y2,int lv);
  void CreateTree(int lv);
  void Render();
  void ResetParam();
  void SetParam(std::vector<double>);
  void Create_Indexes(int col,std::vector<int>* fidx);
  void Set_Resolution(int res);
  void SetEdit(Vec3f,Vec3f,Vec3f);

  /* ---- for per-pixel bumpmapping */
  std::vector<double> tcoords;  // texture0 coordinates(s,t)
  GLdouble light[3];

  void SetTexCoords(std::vector<double> &texcoords) { tcoords = texcoords; };
  //void SetTexCoords( std::vector<double>& );
  void SetLightPos(GLdouble pos[]) {light[0] = pos[0]; light[1] = light[1]; light[2] = pos[2];};
};
#endif //










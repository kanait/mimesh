#include "envDep.h"
#include "mydef.h"

#include <vector>
#include <cmath>
using namespace std;

#include "myGL.hxx"

#include "MR_Patch.h"

extern int nrm_flag;
extern int max_res;

//extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

MR_Patch::MR_Patch(std::vector<double>& src,std::vector<double>& trg,int mesh_res)
{
  count =0;
  _mesh_res = mesh_res;

  int col = (int)pow(2,_mesh_res)+1;
  int num_of_v = col*(col+1)/2;
  int i;
  for(i = 0;i<10;i++)ic[i]=0;
  CreateIndex(mesh_res);
  for(i=0;i<num_of_v;i++)
    {
      std::vector<Vec3f> v;
      Vec3f a,b;
      a.x = src[3*i];a.y = src[3*i+1];a.z = src[3*i+2];
      b.x = trg[3*i];b.y = trg[3*i+1];b.z = trg[3*i+2];
      v.push_back(a);
      v.push_back(b);
      MR_Vec mr_v(v,i);
      _vec.push_back(mr_v);
    }
  CreateTree(_mesh_res);

  /* for(int i=0;i<(int)index.size();i++)
     {
     cerr<<index[i]<<" ";
     }*/
  for(i=0;i<(int)_vec.size();i++)
    {
      _vec[i].calcDif();
    }
  //  _mesh[0].GetIdx(&index,_mesh_res);
  Create_Indexes(col,&index);
  for(i=0;i<(int)_vec.size();i++)
    {
      _vec[i]._print();
    }

  //

  light[0] = 1.0;
  light[1] = 0.0;
  light[2] = 0.0;

};

void
MR_Patch::CreateIndex(int res)//:
{
  int n = (int)pow(2,res)+1;
  // index table

  int sum=0;
  for(int i=0;i<n;i++)
    {
      idx_table.push_back(sum);
      sum += n - i;
    }
}

void
MR_Patch::MakeVTree(int myself,int p0,int p1,int lv)//:
{
  if(!_vec[myself].IsStore()){
    dif_table[lv][ic[lv]] =  myself;ic[lv]++;
    dif_table[lv][ic[lv]] =  p0;ic[lv]++;
    dif_table[lv][ic[lv]] =  p1;ic[lv]++;
    _vec[myself].SetParent(&_vec[p0],&_vec[p1]);
  }
  //  cout<<p0<<" "<<p1<<" \n";
}
MR_Mesh*
MR_Patch::QTree(MR_Mesh* _p_parent,int x0,int y0,int x1,int y1,int x2,int y2,int lv)//:
{
  // Create patch
  int local_count= count;
  MR_Mesh pch(_p_parent,x0+idx_table[y0],x1+idx_table[y1],
              x2+idx_table[y2],local_count);
  _mesh.push_back(pch);
  count++;
  // exit if tree don't have any children.
  if((x0+x1)%2==1||(x1+x2)%2==1||(x2+x0)%2==1||
     (y0+y1)%2==1||(y1+y2)%2==1||(y2+y0)%2==1)return &_mesh[local_count];

  //CREATE VERTEX TREE
  int cx0,cy0,cx1,cy1,cx2,cy2;
  cx0 = (x0 +x1)/2;  cy0 = (y0 +y1)/2;
  cx1 = (x1 +x2)/2;  cy1 = (y1 +y2)/2;
  cx2 = (x2 +x0)/2;  cy2 = (y2 +y0)/2;

  //GET SUBDIVIED VERTICES
  MakeVTree(cx0+idx_table[cy0],x0+idx_table[y0],x1+idx_table[y1],lv+1);
  MakeVTree(cx1+idx_table[cy1],x1+idx_table[y1],x2+idx_table[y2],lv+1);
  MakeVTree(cx2+idx_table[cy2],x2+idx_table[y2],x0+idx_table[y0],lv+1);

  // RECURCIVE CALL
  MR_Mesh *p1,*p2,*p3,*p4;
  p1=QTree(&_mesh[local_count], x0, y0,cx0,cy0,cx2,cy2,lv+1);
  p2=QTree(&_mesh[local_count], x1, y1,cx1,cy1,cx0,cy0,lv+1);
  p3=QTree(&_mesh[local_count], x2, y2,cx2,cy2,cx1,cy1,lv+1);
  p4=QTree(&_mesh[local_count],cx0,cy0,cx1,cy1,cx2,cy2,lv+1);

  _mesh[local_count].SetChild(p1,p2,p3,p4);
  return &_mesh[local_count];
}

void MR_Patch::CreateTree(int lv)//:
{
  int n =  (int)pow(2,lv)+1;
  _vec[0].IsParent();
  _vec[n-1].IsParent();
  _vec[idx_table[n-1]].IsParent();
  QTree((MR_Mesh*)NULL,0,0,n-1,0,0,n-1,0);
}

void MR_Patch::Render()
{
  // if(param<1.0001)param+=0.01;
  int i;
  // 差分ベクトルの補間
  for( i = 0; i <_vec.size(); i++ )
    {
      _vec[i].UpdatePosition(param);
    }

  // 差分ベクトルの加算による詳細形状の生成
//    for(int j = 1; j < max_res; j++ )
  for(int j = 1; j <= max_res; j++ )
    {
      //      cerr<<"root"<<j<<" "<<ic[j]<<"\n";
      for( i = 0; i < ic[j]; i += 3 )
        {
          Vec3f v1 = _vec[dif_table[j][i+1]].GetPosition();
          Vec3f v2 = _vec[dif_table[j][i+2]].GetPosition();
          Vec3f v;
          v.x = (v1.x+v2.x) / 2;
          v.y = (v1.y+v2.y) / 2;
          v.z = (v1.z+v2.z) / 2;
          _vec[dif_table[j][i]].SetPosition(v);
        }
    }

  // 頂点配列の計算, 頂点光線の計算
  std::vector<double> tmp_pt; // テンポラリー頂点配列
  std::vector<double> light_pt; light_pt.clear();// ??

  for( i = 0; i < _vec.size(); i++ )
    {
      Vec3f v = _vec[i].GetPosition();//2(w);
      int num = tmp_pt.size();
      tmp_pt.push_back(v.x * 1.5);
      tmp_pt.push_back(v.y * 1.5);
      tmp_pt.push_back(v.z * 1.5);

      if ( nrm_flag )
        {
          /* calc per-vertex light vector */
          double x = light[0] - v.x;
          double y = light[1] - v.y;
          double z = light[2] - v.z;
          double inv_len = 1.0 / std::sqrt(x * x + y * y + z * z);

          light_pt.push_back(x * inv_len);
          light_pt.push_back(y * inv_len);
          light_pt.push_back(z * inv_len);
        }

    }

  // 法線配列の計算
  std::vector<double> tmp_nm;
  for( i = 0; i < index.size(); i += 3 )
    {
      double v0x = tmp_pt[3*index[i+1]]-tmp_pt[3*index[i]];
      double v0y = tmp_pt[3*index[i+1]+1]-tmp_pt[3*index[i]+1];
      double v0z = tmp_pt[3*index[i+1]+2]-tmp_pt[3*index[i]+2];
      double v1x = tmp_pt[3*index[i+2]]-tmp_pt[3*index[i]];
      double v1y = tmp_pt[3*index[i+2]+1]-tmp_pt[3*index[i]+1];
      double v1z = tmp_pt[3*index[i+2]+2]-tmp_pt[3*index[i]+2];

      double nx= v0y*v1z-v0z*v1y;
      double ny= v0z*v1x-v0x*v1z;
      double nz= v0x*v1y-v0y*v1x;

      double dist = sqrt(nx*nx+ny*ny+nz*nz);

      tmp_nm.push_back(nx/dist);
      tmp_nm.push_back(ny/dist);
      tmp_nm.push_back(nz/dist);
    }
  /*glBegin(GL_TRIANGLES);
    for(i=0;i<index.size();i+=1)
    {
    double *np =tmp_nm.begin()+i/3*3;
    glNormal3dv(np);
    glVertex3d(tmp_pt[index[i]*3],tmp_pt[index[i]*3+1],tmp_pt[index[i]*3+2]);
    }
    glEnd();
  */
  // glPolygonMode(GL_,GL_LINE);
  glEnableClientState( GL_VERTEX_ARRAY );
  glVertexPointer( 3, GL_DOUBLE, 0, &(tmp_pt[0]) );

//    for ( i = 0; i < 10; ++i )
//      cout << "i = " << i << tcoords[i] << endl;
//    cout << "" << endl;
  if ( nrm_flag )
    {
      glClientActiveTexture(GL_TEXTURE0);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2,GL_DOUBLE,0,&(tcoords[0])); // normal map

      glClientActiveTexture(GL_TEXTURE1);
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(3,GL_DOUBLE,0,&(light_pt[0])); // light map
    }

  glBegin( GL_TRIANGLES );
  for( i = 0; i < index.size(); i++ )
    {
      if( i%3 == 0 )
        {
          //    double *np =tmp_nm.begin() + i / 3 * 3;
          //double *np =tmp_nm.begin() + i;
          double* np = &(tmp_nm[i]);
          glNormal3dv(np);
        }
      glArrayElement(index[i]);
    }
  glEnd();
  glDisableClientState(GL_VERTEX_ARRAY);

  if ( nrm_flag )
    {
      glClientActiveTexture(GL_TEXTURE0);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      glClientActiveTexture(GL_TEXTURE1);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
};

void MR_Patch::ResetParam( void )
{
  param[0]=1;
  for(int i=1;i<(int)param.size();i++)param[i]=0;
}

void MR_Patch::SetParam(std::vector<double> d)
{
  param = d;
}


void MR_Patch::Set_Resolution(int res){
  Create_Indexes(res,&index);
}

void MR_Patch::Create_Indexes(int res,std::vector<int>* fidx)
{
  int step = (int)pow(2,_mesh_res - res);
  if(step <1)step = 1;
  int v_col = (int)pow(2,_mesh_res)+1;
  fidx->clear();

  for(int i=0;i<v_col-step;i+=step)
    {
      int ub = idx_table[i];
      int nb = idx_table[i+step];
      for(int j=0;j<v_col-i-step;j+=step)
        {
          fidx->push_back(ub+j);
          fidx->push_back(ub+j+step);
          fidx->push_back(nb+j);
          if(j<v_col-i-step-1)
            {
              fidx->push_back(nb+j);
              fidx->push_back(ub+j+step);
              fidx->push_back(nb+j+step);
            }
        }
    }
  return;
}

void MR_Patch::SetEdit(Vec3f d0,Vec3f d1,Vec3f d2){
  int v_col = (int)pow(2,_mesh_res);
  _vec[0].SetPosition(d0);
  _vec[v_col].SetPosition(d1);
  _vec[idx_table[v_col]].SetPosition(d2);
}

//  void MR_Patch::SetTexCoords( std::vector<double> &texcoords )
//  {
//    int i;
//    if ( tcoords.size() > 5 )
//      for ( i = 0; i < 10; ++i )
//        cout << "b " << tcoords[i] << " after " << texcoords[i] << endl;

//    tcoords.clear();
//    for ( i = 0; i < texcoords.size(); ++i )
//      tcoords.push_back(texcoords[i]);
//  }

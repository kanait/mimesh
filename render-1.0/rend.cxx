#include "envDep.h"
#include "mydef.h"

#include "myGL.hxx"

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace std;

// #include <GL/glut.h>
// #ifdef _WIN32
// #include <windows.h>
// #endif

//#include <GL/wglext.h>

// #include <GL/glext.h>

#include <time.h>

// without normalmap
GLfloat light_position[] = {0.0, 0.0, 100.0, 0.0};
GLfloat light_ambient[] =  {0.0, 0.0, 0.0, 1.0};
GLfloat light_diffuse[] =  {1.0, 1.0, 1.0, 1.0};
GLfloat light_specular[] = {0.0, 0.0, 0.0, 1.0};

#if 0
GLfloat light_diffuse2[] = {0.6,0.6,0.6,1.0};
#endif

//#define GLH_EXT_SINGLE_FILE
//#include "glh_extensions.h"
//using namespace glh;

// #include "extdef.h"
#include "MR_Patch.h"
#include "MR_Mesh.h"
#include "MR_Vec.h"

#if 0
PFNGLMULTITEXCOORD2IARBPROC glMultiTexCoord2iARB;
PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB;
PFNGLMULTITEXCOORD3FVARBPROC glMultiTexCoord3fvARB;
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;

PFNGLCOMBINERPARAMETERFVNVPROC glCombinerParameterfvNV;
PFNGLCOMBINERPARAMETERIVNVPROC glCombinerParameterivNV;
PFNGLCOMBINERPARAMETERFNVPROC glCombinerParameterfNV;
PFNGLCOMBINERPARAMETERINVPROC glCombinerParameteriNV;
PFNGLCOMBINERINPUTNVPROC glCombinerInputNV;
PFNGLCOMBINEROUTPUTNVPROC glCombinerOutputNV;
PFNGLFINALCOMBINERINPUTNVPROC glFinalCombinerInputNV;
PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC glGetCombinerInputParameterfvNV;
PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC glGetCombinerInputParameterivNV;
PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC glGetCombinerOutputParameterfvNV;
PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC glGetCombinerOutputParameterivNV;
PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC glGetFinalCombinerInputfvNV;
PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC glGetFinalCombinerInputivNV;
#endif

#include "tga.h"   // tga format texture image loading

std::vector<std::vector<int> > bidx;//初期補間メッシュのインデックス
std::vector<std::vector<Vec3f> > pathv;//パスの頂点
std::vector<Vec3f> nrms;//パスの頂点
std::vector<Vec3f> nrmt;//パスの頂点
std::vector<double> weight;
std::vector<double> param;
std::vector<MR_Patch> patch;
std::vector<std::vector<double> > srcv;

// 画面の大きさ
#define PIXSIZE 640

// ステップ間隔
#define STEP .01
// 補間パラメータ
double pp = 0;

// 画像ダンプの番号
int ppp = 0;

// ベース補間メッシュの面の数
int patchnum;

// 表示の解像度レベル
int res = 4;
// ファイルの最大解像度レベル
int max_res;

// ライトの回転
int rotate_light_flag = 0;

// 法線マップ
int nrm_flag = 1;

// スムースシェーディング
int smh_flag = 0;

// 画像のダンプ
int img_flag = 0;

// モーフィング
int anm_flag = 0;

// パス表示
int path_flag = 0;

// 線形／ベジエ補間切り替え
int bez_flag = 0;

// ベース補間メッシュの表示
int bm_flag = 0;

int vt;
std::vector<int> fc,fc2;

/* defines and functuons for per-pixel bumpmapping ----------------------- */
#define M_PER_PIXEL_SELF_SHADOWING_CLAMP 1
#define M_PER_PIXEL_SELF_SHADOWING_RAMP 2

/* for texture object */
//#define DECAL_MAP 1

static int lightnum;

typedef struct {
  GLfloat x;
  GLfloat y;
  GLfloat z;
} Vector;

int modulateDiffuse = 1;

// パラメータを計算するときにしか使用しない
#define TEXTURE_HEIGHT1024 1024
#define TEXTURE_SIZE1024  126
#define TEXTURE_HEIGHT512 512
#define TEXTURE_SIZE512  62
#define TEXTURE_HEIGHT256 256
#define TEXTURE_SIZE256  30

// 1024
static char *src1024 = "venus1024.tga";
static char *trg1024 = "tiger1024.tga";
// 512
static char *src512 = "venus512.tga";
static char *trg512 = "tiger512.tga";
// 256
static char *src256 = "venus256.tga";
static char *trg256 = "tiger256.tga";

#define TEXTURE_BORDER  1

int texture_height = TEXTURE_HEIGHT256;
int texture_size = TEXTURE_SIZE256;
//  int texture_height = TEXTURE_HEIGHT512;
//  int texture_size = TEXTURE_SIZE512;
//  int texture_height = TEXTURE_HEIGHT1024;
//  int texture_size = TEXTURE_SIZE1024;

//  static int bumpmapping; // toggle bumpmapping
static GLuint texobj[2];
static gliGenericImage *src = NULL;
static gliGenericImage *dest = NULL;

// textures for interpolation
GLubyte *tmp_texture = NULL;

int initTextures( int );
// void initCombiners(int shadowing, int specular, GLfloat diffuse);
void updateTexture(double param);

// cubemap
static GLdouble light_angle;
static GLdouble light_pos0[3];
//  static GLdouble light_pos1[3];
//  static GLdouble light_pos2[3];

int init();

void print_glerror()
{
  GLenum errCode;
  const GLubyte *errString;

  if ((errCode = glGetError()) != GL_NO_ERROR) {
    errString = gluErrorString(errCode);
    fprintf(stderr, "OpenGL error: %s\n", errString);
  }
}

/*** MATRIX MANIPULATION ***/

/*
 * Compute inverse of 4x4 transformation matrix.
 * Code contributed by Jacques Leroy <jle@star.be>
 * Code lifted from Brian Paul's Mesa freeware OpenGL implementation.
 * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
 */

GLboolean
invertMatrix(GLdouble *out, const GLdouble *m)
{
  /* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

  GLdouble wtmp[4][8];
  GLdouble m0, m1, m2, m3, s;
  GLdouble *r0, *r1, *r2, *r3;

  r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

  r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
    r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
    r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

    r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
    r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
    r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

    r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
    r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
    r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

    r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
    r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
    r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

  /* choose pivot - or die */
  if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
  if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
  if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
  if (0.0 == r0[0]) {
    return GL_FALSE;
  }

  /* eliminate first variable     */
  m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
  s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
  s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
  s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
  s = r0[4];
  if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
  s = r0[5];
  if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
  s = r0[6];
  if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
  s = r0[7];
  if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

  /* choose pivot - or die */
  if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
  if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
  if (0.0 == r1[1]) {
    return GL_FALSE;
  }

  /* eliminate second variable */
  m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
  r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
  r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
  s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
  s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
  s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
  s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

  /* choose pivot - or die */
  if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
  if (0.0 == r2[2]) {
    return GL_FALSE;
  }

  /* eliminate third variable */
  m3 = r3[2]/r2[2];
  r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
    r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
    r3[7] -= m3 * r2[7];

  /* last check */
  if (0.0 == r3[3]) {
    return GL_FALSE;
  }

  s = 1.0/r3[3];              /* now back substitute row 3 */
  r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

  m2 = r2[3];                 /* now back substitute row 2 */
  s  = 1.0/r2[2];
  r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
    r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
  m1 = r1[3];
  r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
    r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
  m0 = r0[3];
  r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
    r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

  m1 = r1[2];                 /* now back substitute row 1 */
  s  = 1.0/r1[1];
  r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
    r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
  m0 = r0[2];
  r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
    r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

  m0 = r0[1];                 /* now back substitute row 0 */
  s  = 1.0/r0[0];
  r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
    r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

  MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
                          MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
                                                  MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
                                                                          MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
                                                                                                  MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
                                                                                                                          MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
                                                                                                                                                  MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
                                                                                                                                                                          MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7];

  return GL_TRUE;

#undef MAT
#undef SWAP_ROWS
}

/*
 * Compute inverse of 4x4 transformation SINGLE-PRECISION matrix.
 * Code contributed by Jacques Leroy <jle@star.be>
 * Code lifted from Brian Paul's Mesa freeware OpenGL implementation.
 f * Return GL_TRUE for success, GL_FALSE for failure (singular matrix)
*/

GLboolean
invertMatrixf(GLfloat *out, const GLfloat *m)
{
  /* NB. OpenGL Matrices are COLUMN major. */
#define SWAP_ROWS(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m,r,c) (m)[(c)*4+(r)]

  GLdouble wtmp[4][8];
  GLdouble m0, m1, m2, m3, s;
  GLdouble *r0, *r1, *r2, *r3;

  r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

  r0[0] = MAT(m,0,0), r0[1] = MAT(m,0,1),
    r0[2] = MAT(m,0,2), r0[3] = MAT(m,0,3),
    r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,

    r1[0] = MAT(m,1,0), r1[1] = MAT(m,1,1),
    r1[2] = MAT(m,1,2), r1[3] = MAT(m,1,3),
    r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,

    r2[0] = MAT(m,2,0), r2[1] = MAT(m,2,1),
    r2[2] = MAT(m,2,2), r2[3] = MAT(m,2,3),
    r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,

    r3[0] = MAT(m,3,0), r3[1] = MAT(m,3,1),
    r3[2] = MAT(m,3,2), r3[3] = MAT(m,3,3),
    r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;

  /* choose pivot - or die */
  if (fabs(r3[0])>fabs(r2[0])) SWAP_ROWS(r3, r2);
  if (fabs(r2[0])>fabs(r1[0])) SWAP_ROWS(r2, r1);
  if (fabs(r1[0])>fabs(r0[0])) SWAP_ROWS(r1, r0);
  if (0.0 == r0[0]) {
    return GL_FALSE;
  }

  /* eliminate first variable     */
  m1 = r1[0]/r0[0]; m2 = r2[0]/r0[0]; m3 = r3[0]/r0[0];
  s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
  s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
  s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
  s = r0[4];
  if (s != 0.0) { r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s; }
  s = r0[5];
  if (s != 0.0) { r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s; }
  s = r0[6];
  if (s != 0.0) { r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s; }
  s = r0[7];
  if (s != 0.0) { r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s; }

  /* choose pivot - or die */
  if (fabs(r3[1])>fabs(r2[1])) SWAP_ROWS(r3, r2);
  if (fabs(r2[1])>fabs(r1[1])) SWAP_ROWS(r2, r1);
  if (0.0 == r1[1]) {
    return GL_FALSE;
  }

  /* eliminate second variable */
  m2 = r2[1]/r1[1]; m3 = r3[1]/r1[1];
  r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
  r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
  s = r1[4]; if (0.0 != s) { r2[4] -= m2 * s; r3[4] -= m3 * s; }
  s = r1[5]; if (0.0 != s) { r2[5] -= m2 * s; r3[5] -= m3 * s; }
  s = r1[6]; if (0.0 != s) { r2[6] -= m2 * s; r3[6] -= m3 * s; }
  s = r1[7]; if (0.0 != s) { r2[7] -= m2 * s; r3[7] -= m3 * s; }

  /* choose pivot - or die */
  if (fabs(r3[2])>fabs(r2[2])) SWAP_ROWS(r3, r2);
  if (0.0 == r2[2]) {
    return GL_FALSE;
  }

  /* eliminate third variable */
  m3 = r3[2]/r2[2];
  r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
    r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6],
    r3[7] -= m3 * r2[7];

  /* last check */
  if (0.0 == r3[3]) {
    return GL_FALSE;
  }

  s = 1.0/r3[3];              /* now back substitute row 3 */
  r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

  m2 = r2[3];                 /* now back substitute row 2 */
  s  = 1.0/r2[2];
  r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
    r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
  m1 = r1[3];
  r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
    r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
  m0 = r0[3];
  r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
    r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;

  m1 = r1[2];                 /* now back substitute row 1 */
  s  = 1.0/r1[1];
  r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
    r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
  m0 = r0[2];
  r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
    r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;

  m0 = r0[1];                 /* now back substitute row 0 */
  s  = 1.0/r0[0];
  r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
    r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);

  MAT(out,0,0) = r0[4]; MAT(out,0,1) = r0[5],
                          MAT(out,0,2) = r0[6]; MAT(out,0,3) = r0[7],
                                                  MAT(out,1,0) = r1[4]; MAT(out,1,1) = r1[5],
                                                                          MAT(out,1,2) = r1[6]; MAT(out,1,3) = r1[7],
                                                                                                  MAT(out,2,0) = r2[4]; MAT(out,2,1) = r2[5],
                                                                                                                          MAT(out,2,2) = r2[6]; MAT(out,2,3) = r2[7],
                                                                                                                                                  MAT(out,3,0) = r3[4]; MAT(out,3,1) = r3[5],
                                                                                                                                                                          MAT(out,3,2) = r3[6]; MAT(out,3,3) = r3[7];

  return GL_TRUE;

#undef MAT
#undef SWAP_ROWS
}

/* Transform "in" vector by "m" transform to compute "out" vector. */
void
transformPosition(Vector *out, Vector *in, float m[4][4])
{
  float w;

  w = in->x * m[0][3] + in->y * m[1][3] + in->z * m[2][3] + m[3][3];
  out->x = (in->x * m[0][0] + in->y * m[1][0] + in->z * m[2][0] + m[3][0])/w;
  out->y = (in->x * m[0][1] + in->y * m[1][1] + in->z * m[2][1] + m[3][1])/w;
  out->z = (in->x * m[0][2] + in->y * m[1][2] + in->z * m[2][2] + m[3][2])/w;
}

/* dst = transpose(src) */
void
transposeMatrix(GLdouble dst[16], GLdouble src[16])
{
  dst[0] = src[0];
  dst[1] = src[4];
  dst[2] = src[8];
  dst[3] = src[12];

  dst[4] = src[1];
  dst[5] = src[5];
  dst[6] = src[9];
  dst[7] = src[13];

  dst[8] = src[2];
  dst[9] = src[6];
  dst[10] = src[10];
  dst[11] = src[14];

  dst[12] = src[3];
  dst[13] = src[7];
  dst[14] = src[11];
  dst[15] = src[15];
}

/* dst = a + b */
void
addMatrices(GLdouble dst[16], GLdouble a[16], GLdouble b[16])
{
  dst[0] = a[0] + b[0];
  dst[1] = a[1] + b[1];
  dst[2] = a[2] + b[2];
  dst[3] = a[3] + b[3];

  dst[4] = a[4] + b[4];
  dst[5] = a[5] + b[5];
  dst[6] = a[6] + b[6];
  dst[7] = a[7] + b[7];

  dst[8] = a[8] + b[8];
  dst[9] = a[9] + b[9];
  dst[10] = a[10] + b[10];
  dst[11] = a[11] + b[11];

  dst[12] = a[12] + b[12];
  dst[13] = a[13] + b[13];
  dst[14] = a[14] + b[14];
  dst[15] = a[15] + b[15];
}

/* Build a 4x4 matrix transform based on the parameters for gluLookAt.
 * Code lifted from Brian Paul's MesaGLU.
 */
void
buildLookAtMatrix(GLdouble eyex, GLdouble eyey, GLdouble eyez,
                  GLdouble centerx, GLdouble centery, GLdouble centerz,
                  GLdouble upx, GLdouble upy, GLdouble upz, GLdouble m[16])
{
  GLdouble x[3], y[3], z[3];
  GLdouble mag;

  /* Make rotation matrix */

  /* Z vector */
  z[0] = eyex - centerx;
  z[1] = eyey - centery;
  z[2] = eyez - centerz;
  mag = sqrt( z[0]*z[0] + z[1]*z[1] + z[2]*z[2] );
  if (mag) {  /* mpichler, 19950515 */
    z[0] /= mag;
    z[1] /= mag;
    z[2] /= mag;
  }

  /* Y vector */
  y[0] = upx;
  y[1] = upy;
  y[2] = upz;

  /* X vector = Y cross Z */
  x[0] =  y[1]*z[2] - y[2]*z[1];
  x[1] = -y[0]*z[2] + y[2]*z[0];
  x[2] =  y[0]*z[1] - y[1]*z[0];

  /* Recompute Y = Z cross X */
  y[0] =  z[1]*x[2] - z[2]*x[1];
  y[1] = -z[0]*x[2] + z[2]*x[0];
  y[2] =  z[0]*x[1] - z[1]*x[0];

  /* mpichler, 19950515 */
  /* cross product gives area of parallelogram, which is < 1.0 for
   * non-perpendicular unit-length vectors; so normalize x, y here
   */

  mag = sqrt( x[0]*x[0] + x[1]*x[1] + x[2]*x[2] );
  if (mag) {
    x[0] /= mag;
    x[1] /= mag;
    x[2] /= mag;
  }

  mag = sqrt( y[0]*y[0] + y[1]*y[1] + y[2]*y[2] );
  if (mag) {
    y[0] /= mag;
    y[1] /= mag;
    y[2] /= mag;
  }

#define M(row,col)  m[col*4+row]
  M(0,0) = x[0];  M(0,1) = x[1];  M(0,2) = x[2];  M(0,3) = -x[0]*eyex + -x[1]*eyey + -x[2]*eyez;
  M(1,0) = y[0];  M(1,1) = y[1];  M(1,2) = y[2];  M(1,3) = -y[0]*eyex + -y[1]*eyey + -y[2]*eyez;
  M(2,0) = z[0];  M(2,1) = z[1];  M(2,2) = z[2];  M(2,3) = -z[0]*eyex + -z[1]*eyey + -z[2]*eyez;
  M(3,0) = 0.0;   M(3,1) = 0.0;   M(3,2) = 0.0;   M(3,3) = 1.0;
#undef M
}

/*** FAST INVERSE SQUARE ROOT ***/

/* From "Graphics Gems V", Alan Paeth (Editor)
 * ISBN 0125434553/9649-1547332-306386
 * Published by Ap Profession, 1995
 */

/* Compute the Inverse Square Root
 * of an IEEE Single Precision Floating-Point number.
 *
 * Written by Ken Turkowski.
 */

/* Specified parameters */
#define LOOKUP_BITS    6   /* Number of mantissa bits for lookup */
#define EXP_POS       23   /* Position of the exponent */
#define EXP_BIAS     127   /* Bias of exponent */
/* The mantissa is assumed to be just down from the exponent */

/* Derived parameters */
#define LOOKUP_POS   (EXP_POS-LOOKUP_BITS)  /* Position of mantissa lookup */
#define SEED_POS     (EXP_POS-8)            /* Position of mantissa seed */
#define TABLE_SIZE   (2 << LOOKUP_BITS)     /* Number of entries in table */
#define LOOKUP_MASK  (TABLE_SIZE - 1)           /* Mask for table input */
#define GET_EXP(a)   (((a) >> EXP_POS) & 0xFF)  /* Extract exponent */
#define SET_EXP(a)   ((a) << EXP_POS)           /* Set exponent */
#define GET_EMANT(a) (((a) >> LOOKUP_POS) & LOOKUP_MASK)  /* Extended mantissa
                                                           * MSB's */
#define SET_MANTSEED(a) (((unsigned long)(a)) << SEED_POS)  /* Set mantissa
                                                             * 8 MSB's */

static unsigned char iSqrt[TABLE_SIZE];

union _flint {
  unsigned long    i;
  float            f;
} _fi, _fo;

void
makeInverseSqrtLookupTable(void)
{
  register long f;
  register unsigned char *h;
  union _flint fi, fo;

  h = iSqrt;
  for (f = 0, h = iSqrt; f < TABLE_SIZE; f++) {
    fi.i = ((EXP_BIAS-1) << EXP_POS) | (f << LOOKUP_POS);
    fo.f = (float) (1.0 / sqrt(fi.f));
    *h++ = (unsigned char)
      (((fo.i + (1<<(SEED_POS-2))) >> SEED_POS) & 0xFF); /* rounding */
  }
  iSqrt[TABLE_SIZE / 2] = 0xFF;    /* Special case for 1.0 */
}

/* Non-WinTel platforms don't need fastcall. */
#ifndef FASTCALL
#define FASTCALL
#endif

/* The following returns the inverse square root. */
static float FASTCALL
invSqrt(float x)
{
  unsigned long a = ((union _flint*)(&x))->i;
  float arg = x;
  union _flint seed;
  float r;

  seed.i = SET_EXP(((3*EXP_BIAS-1) - GET_EXP(a)) >> 1)
    | SET_MANTSEED(iSqrt[GET_EMANT(a)]);

  /* Seed: accurate to LOOKUP_BITS */
  r = seed.f;

  /* First iteration: accurate to 2*LOOKUP_BITS */
  r = (float) ((3.0 - r * r * arg) * r * 0.5);

#if 0  /* Wow!  We don't need this much precision! */
  /* Second iteration: accurate to 4*LOOKUP_BITS */
  r = (float) ((3.0 - r * r * arg) * r * 0.5);
#endif

  return r;
}
void set_diffuse_path()
{
  glDisable(GL_BLEND);
  glDepthFunc(GL_LESS);
  glColorMask(1, 1, 1, 1);

  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texobj[0]);
  // glEnable(GL_REGISTER_COMBINERS_NV);

  // initCombiners(M_PER_PIXEL_SELF_SHADOWING_RAMP, 0, 0.8); /* 0 = no specular */
}

void set_decal_path()
{
  /*glEnable(GL_BLEND);
    glDepthFunc(GL_EQUAL);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glColorMask(1, 1, 1, 0);
  */
  glDisable(GL_BLEND);

  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texobj[0]);
  glEnable(GL_TEXTURE_2D);
  // glDisable(GL_REGISTER_COMBINERS_NV);
  glDisable(GL_LIGHTING);

}

void set_specular_path()
{
  glBlendFunc(GL_DST_ALPHA, GL_ONE);
  glEnable(GL_BLEND);
  glDepthFunc(GL_EQUAL);
  glColorMask(1, 1, 1, 0);

  glActiveTexture(GL_TEXTURE1);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texobj[0]);
  // glEnable(GL_REGISTER_COMBINERS_NV);

  // initCombiners(M_PER_PIXEL_SELF_SHADOWING_RAMP, 1, 0.8); /* 1 = specular on */
}

void disable_bumpmap_path()
{
  glDisable(GL_BLEND);
  glDepthFunc(GL_LESS);
  glColorMask(1, 1, 1, 1);
  glActiveTexture(GL_TEXTURE1);
  glDisable(GL_TEXTURE_CUBE_MAP);
  glActiveTexture(GL_TEXTURE0);
  glDisable(GL_TEXTURE_2D);
  // glDisable(GL_REGISTER_COMBINERS_NV);

  glEnable(GL_LIGHTING);
}

// モーフィングの際の法線マップテクスチャのアップデート
void updateTexture(double param)
{
  if ( anm_flag )
    {
      for ( int i = 0; i < src->width * src->height *src->components; i++ )
        {
          tmp_texture[i] = (GLubyte)( src->pixels[i] * (1.0 - param)
                                      + dest->pixels[i] * param );
        }
    }

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, texobj[0] );
  glTexSubImage2D( GL_TEXTURE_2D, 0,
                   0, 0,
                   src->width, src->height,
                   src->format, GL_UNSIGNED_BYTE, tmp_texture
                   );


}

void calcTexCoords( int col, int facenum,
                    std::vector<double> *tcoords,
                    int tex_height, int tex_size )
{
  // texture row has each 8 tiles
  int xoffset = (TEXTURE_BORDER * 2 + tex_size) * (int) (facenum % 8);
  int yoffset = (TEXTURE_BORDER * 2 + tex_size) * (int) (facenum / 8);

  double step = (double) tex_size / (double)(col - 1);
  for (int j = col; j >= 0; j--) {
    for (int i = 0; i < j; i++) {
      double t = TEXTURE_BORDER + i * step + yoffset;
      double s = TEXTURE_BORDER + (col - j) * step + xoffset;
      // texture coord mapping from [0, tex_height] to [0, 1]
      tcoords->push_back(s / tex_height);
      tcoords->push_back(t / tex_height);
    }
  }
}

void updateTexCoords( int tex_height, int tex_size )
{
  int vcol = (int) pow(2, max_res) + 1;
  for( int i = 0 ; i < patchnum; i++ )
    {
      std::vector<double> texcoords;
      calcTexCoords( vcol, i, &texcoords, tex_height, tex_size );
      patch[i].SetTexCoords(texcoords);
    }
}

/*** NORMALIZATION CUBE MAP CONSTRUCTION ***/

static void
getCubeVector(int i, int cubesize, int x, int y, float *vec)
{
  float s, t, sc, tc, mag;

  s = ((float)x + 0.5) / (float)cubesize;
  t = ((float)y + 0.5) / (float)cubesize;
  sc = s*2.0 - 1.0;
  tc = t*2.0 - 1.0;

  switch (i) {
  case 0:
    vec[0] = 1.0;
    vec[1] = -tc;
    vec[2] = -sc;
    break;
  case 1:
    vec[0] = -1.0;
    vec[1] = -tc;
    vec[2] = sc;
    break;
  case 2:
    vec[0] = sc;
    vec[1] = 1.0;
    vec[2] = tc;
    break;
  case 3:
    vec[0] = sc;
    vec[1] = -1.0;
    vec[2] = -tc;
    break;
  case 4:
    vec[0] = sc;
    vec[1] = -tc;
    vec[2] = 1.0;
    break;
  case 5:
    vec[0] = -sc;
    vec[1] = -tc;
    vec[2] = -1.0;
    break;
  }

  mag = 1.0/sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
  vec[0] *= mag;
  vec[1] *= mag;
  vec[2] *= mag;
}

void
makeNormalizeVectorCubeMap(int size)
{
  float vector[3];
  int i, x, y;
  GLubyte *pixels;

  pixels = (GLubyte*) malloc(size*size*3);
  if (pixels == NULL) {
    fprintf(stderr, "npeturb: malloc failed in makeNormalizedVectorCubeMap\n");
    exit(1);
  }

  print_glerror();
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  print_glerror();
  for (i = 0; i < 6; i++) {
    for (y = 0; y < size; y++) {
      for (x = 0; x < size; x++) {
        getCubeVector(i, size, x, y, vector);
        pixels[3*(y*size+x) + 0] = 128 + 127*vector[0];
        pixels[3*(y*size+x) + 1] = 128 + 127*vector[1];
        pixels[3*(y*size+x) + 2] = 128 + 127*vector[2];
      }
    }
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT+i, 0, GL_RGB8,
                 size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
  }

  print_glerror();
  free(pixels);
}

gliGenericImage *
readImage(char *filename)
{
  FILE *file;
  gliGenericImage *image;

  file = fopen(filename, "rb");
  if (file == NULL) {
    printf("cm_demo: could not open \"%s\"\n", filename);
    exit(1);
  }

  image = gliReadTGA(file, filename);
  fclose(file);

  return image;
}

//  int loadTexture( gliGenericImage *image, int mipmaps )
//  {
//    return 1;
//  }

void freeTextures( void )
{
  if ( src ) free( src );
  if ( dest ) free ( dest );
  if ( tmp_texture ) free( tmp_texture );
}

// 法線テクスチャの初期化
int initTextures( int num )
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  // デフォルトの画像の読み込み
  if ( num == 0 )
    { // 1024
      src  = readImage(src1024); dest = readImage(trg1024);
    }
  else if ( num == 1 )
    { // 512
      src  = readImage(src512); dest = readImage(trg512);
    }
  else
    { // 256
      src  = readImage(src256); dest = readImage(trg256);
    }

  tmp_texture = (GLubyte *)malloc( sizeof(GLubyte) *
                                   src->width *
                                   src->height *
                                   src->components );
  for ( int i = 0; i < src->width * src->height *src->components; i++ )
    {
      tmp_texture[i] = (GLubyte)(src->pixels[i] * (1.0 - pp) + dest->pixels[i] * pp);
      //    tmp_texture[i] = (GLubyte) (src->pixels[i]);
    }

  glActiveTexture( GL_TEXTURE0 );
  glBindTexture(GL_TEXTURE_2D, texobj[0]);

  //loadTexture( src, 0 );
  int mipmaps = 1;
  if ( mipmaps ) {
    gluBuild2DMipmaps(GL_TEXTURE_2D, src->components, src->width, src->height,
                      src->format, GL_UNSIGNED_BYTE, tmp_texture );
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, src->components, src->width, src->height, 0,
                 src->format, GL_UNSIGNED_BYTE, tmp_texture );
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  print_glerror();

  return 1;
}


void check_extension( void )
{
  if (!glutExtensionSupported("GL_ARB_multitexture")) {
    fprintf(stderr, "GL_ARB_multitexture extension is required.\n");
    exit(1);
  }

#if 0
  if (!glutExtensionSupported("GL_NV_register_combiners")) {
    fprintf(stderr, "GL_NV_register_combiners extension is required.\n");
    exit(1);
  }
#endif

#if 0
  if (!glutExtensionSupported("GL_EXT_texture_cube_map")) {
    fprintf(stderr, "GL_EXT_texture_cube_map extension is required.\n");
    exit(1);
  }
#endif
}

int init()
{
  glewInit();

  if (!GLEW_ARB_texture_env_combine) {
    std::cerr << "GL_ARB_texture_env_combine not supported!" << std::endl;
    exit(1);
  }
  //  check_extension();
#if 0
#ifdef _WIN32

  /* Retrieve some ARB_multitexture routines. */
  glActiveTextureARB =
    (PFNGLACTIVETEXTUREARBPROC)
    wglGetProcAddress("glActiveTextureARB");

  glClientActiveTextureARB =
    (PFNGLCLIENTACTIVETEXTUREARBPROC)
    wglGetProcAddress("glClientActiveTextureARB");

  /* Retrieve all NV_register_combiners routines. */
  glCombinerParameterfvNV =
    (PFNGLCOMBINERPARAMETERFVNVPROC)
    wglGetProcAddress("glCombinerParameterfvNV");
  glCombinerParameterivNV =
    (PFNGLCOMBINERPARAMETERIVNVPROC)
    wglGetProcAddress("glCombinerParameterivNV");
  glCombinerParameterfNV =
    (PFNGLCOMBINERPARAMETERFNVPROC)
    wglGetProcAddress("glCombinerParameterfNV");
  glCombinerParameteriNV =
    (PFNGLCOMBINERPARAMETERINVPROC)
    wglGetProcAddress("glCombinerParameteriNV");
  glCombinerInputNV =
    (PFNGLCOMBINERINPUTNVPROC)
    wglGetProcAddress("glCombinerInputNV");
  glCombinerOutputNV =
    (PFNGLCOMBINEROUTPUTNVPROC)
    wglGetProcAddress("glCombinerOutputNV");
  glFinalCombinerInputNV =
    (PFNGLFINALCOMBINERINPUTNVPROC)
    wglGetProcAddress("glFinalCombinerInputNV");
  glGetCombinerInputParameterfvNV =
    (PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC)
    wglGetProcAddress("glGetCombinerInputParameterfvNV");
  glGetCombinerInputParameterivNV =
    (PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC)
    wglGetProcAddress("glGetCombinerInputParameterivNV");
  glGetCombinerOutputParameterfvNV =
    (PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC)
    wglGetProcAddress("glGetCombinerOutputParameterfvNV");
  glGetCombinerOutputParameterivNV =
    (PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC)
    wglGetProcAddress("glGetCombinerOutputParameterivNV");
  glGetFinalCombinerInputfvNV =
    (PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC)
    wglGetProcAddress("glGetFinalCombinerInputfvNV");
  glGetFinalCombinerInputivNV =
    (PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC)
    wglGetProcAddress("glGetFinalCombinerInputivNV");
#endif
#endif

  lightnum = 0;

  makeInverseSqrtLookupTable();

  light_angle = 0.0;

  light_pos0[0] = light_position[0];
  light_pos0[1] = light_position[1];
  light_pos0[2] = light_position[2];

  glEnable(GL_NORMALIZE);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_LIGHTING);

  glGenTextures(2, texobj);

  initTextures( 2 );

#if 0
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texobj[0]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texobj[1]);
  makeNormalizeVectorCubeMap(32);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glActiveTexture(GL_TEXTURE0);
#endif

  // ノーマルマップをテクスチャユニット0にバインド
  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_2D);
  // glBindTexture(GL_TEXTURE_2D, normalMapTexID);
  glBindTexture(GL_TEXTURE_2D, texobj[0]);

  // テクスチャの RGB を [-1, 1] に展開し、ライトベクトルとの内積をとる
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);

  // ノーマルマップ（接空間の法線）＝ SOURCE0
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_TEXTURE);
  // ライトベクトル or 固定ベクトル = SOURCE1
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_PRIMARY_COLOR); // glColor で渡す

  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

  // RGB 結果は glColor と MODULATE される（環境光に影響）
  glEnable(GL_COLOR_MATERIAL);
  glColor3f(.9f, .9f, .9f); // 白色 → ライトベクトルとして使う
  // initCombiners(M_PER_PIXEL_SELF_SHADOWING_CLAMP, 0, 0.8);

  glActiveTexture(GL_TEXTURE1);
  // glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexID);
  glEnable(GL_TEXTURE_CUBE_MAP);
  //makeNormalizeVectorCubeMap(32);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texobj[1]);

  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
  glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);  // DOT3出力と加算
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);  // ユニット0のDOT3出力
  glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);   // cube mapの反射色

  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
  glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

  return 1;
}

#if 0
void
initCombiners(int shadowing, int specular, GLfloat diffuse)
{
  GLfloat constant0[4] = {0.0, 0.0, 0.0, 0.0};
  GLfloat diffuse1[4] = {diffuse, diffuse, diffuse, 0.0};

  if (specular) {
    shadowing = M_PER_PIXEL_SELF_SHADOWING_CLAMP;
  }

  if (shadowing == M_PER_PIXEL_SELF_SHADOWING_CLAMP || specular) {
    glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 2);
  } else {
    glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 1);
  }
  glCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV, constant0);
  glCombinerParameterfvNV(GL_CONSTANT_COLOR1_NV, diffuse1);

  /*** GENERAL Combiner ZERO, RGB portion. ***/
  /* Argb = 3x3 matrix column1 = expand(texture0rgb) = N' */
  glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV,
                    GL_TEXTURE0_ARB, GL_EXPAND_NORMAL_NV, GL_RGB);
  /* Brgb = expand(texture1rgb) = L (or H if specular) */
  glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV,
                    GL_TEXTURE1_ARB, GL_EXPAND_NORMAL_NV, GL_RGB);

  /* spare0rgb = Argb dot Brgb = expand(texture0rgb) dot expand(texture1rgb) = L dot N' */
  /* Or if specular, spare0rgb = H dot N' */
  glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB,
                     GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
                     GL_NONE, GL_NONE, GL_TRUE, GL_FALSE, GL_FALSE);

  /*** GENERAL Combiner ZERO, Alpha portion. ***/
  if (shadowing == M_PER_PIXEL_SELF_SHADOWING_RAMP) {
    /* Aa = 1 */
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV,
                      GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
    /* Ba = expand(texture1b) = Lz */
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV,
                      GL_TEXTURE1_ARB, GL_EXPAND_NORMAL_NV, GL_BLUE);
    /* Ca = 1 */
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_C_NV,
                      GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
    /* Da = expand(texture1b) = Lz */
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_D_NV,
                      GL_TEXTURE1_ARB, GL_EXPAND_NORMAL_NV, GL_BLUE);

    /* spare0a = 4*(1*Lz + 1*Lz) = 8*expand(texture1a) */
    glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA,
                       GL_DISCARD_NV, GL_DISCARD_NV, GL_SPARE0_NV,
                       GL_SCALE_BY_FOUR_NV, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);
  } else {
    /* Aa = 1 */
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_A_NV,
                      GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
    /* Ba = texture1b = unexpanded Lz (or Hz if specular) */
    glCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, GL_VARIABLE_B_NV,
                      GL_TEXTURE1_ARB, GL_UNSIGNED_IDENTITY_NV, GL_BLUE);

    /* spare0a = 1 * texture1b = unexpanded Lz (or Hz if specular) */
    glCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA,
                       GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
                       GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);
  }

  if (shadowing == M_PER_PIXEL_SELF_SHADOWING_CLAMP) {

    /*** GENERAL Combiner ONE, RGB portion. ***/
    /* Argb = 0 */
    glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV,
                      GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    /* Brgb = 0 */
    glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV,
                      GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    if (specular) {
      /* Crgb = spare0rgb = H dot N' */
      glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_C_NV,
                        GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    } else {
      /* Crgb = 1 */
      glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_C_NV,
                        GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
    }
    /* Drgb = spare0rgb = L dot N' (or H dot N' if specular) */
    glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_D_NV,
                      GL_SPARE0_NV, GL_SIGNED_IDENTITY_NV, GL_RGB);

    /* spare0rgb = ((spare0a >= 0.5) ? spare0rgb^2 : 0) = ((L dot N > 0) ? (L dot N')^2 : 0) */
    glCombinerOutputNV(GL_COMBINER1_NV, GL_RGB,
                       GL_DISCARD_NV, GL_DISCARD_NV, GL_SPARE0_NV,
                       GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_TRUE);

    /*** GENERAL Combiner ONE, Alpha portion. ***/
    /* Aa = 1 */
    glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_A_NV,
                      GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_ALPHA);
    /* Ba = expand(texture1b) = Lz */
    glCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, GL_VARIABLE_B_NV,
                      GL_TEXTURE1_ARB, GL_EXPAND_NORMAL_NV, GL_BLUE);

    /* spare0a = 1 * expand(texture1b) = Lz (or Hz if specular) */
    glCombinerOutputNV(GL_COMBINER1_NV, GL_ALPHA,
                       GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
                       GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);
  }

  if (shadowing != M_PER_PIXEL_SELF_SHADOWING_CLAMP && specular) {
    /*** GENERAL Combiner ONE, RGB portion. ***/
    /* Argb = spare0rgb = H dot N' */
    glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV,
                      GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    /* Brgb = spare0rgb = H dot N' */
    glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV,
                      GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    /* spare0rgb = spare0rgb^2 = (H dot N')^2 */
    glCombinerOutputNV(GL_COMBINER1_NV, GL_RGB,
                       GL_SPARE0_NV, GL_DISCARD_NV, GL_DISCARD_NV,
                       GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);

    /*** GENERAL Combiner ONE, Alpha portion. ***/
    /* Discard entire alpha portion. */
    glCombinerOutputNV(GL_COMBINER1_NV, GL_ALPHA,
                       GL_DISCARD_NV, GL_DISCARD_NV, GL_DISCARD_NV,
                       GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);
  }

  /*** FINAL Combiner. ***/
  if (shadowing == M_PER_PIXEL_SELF_SHADOWING_RAMP) {
    /* A = spare0a = per-pixel self-shadowing term */
    glFinalCombinerInputNV(GL_VARIABLE_A_NV,
                           GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
  } else {
    /* A = EF */
    glFinalCombinerInputNV(GL_VARIABLE_A_NV,
                           GL_E_TIMES_F_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  }
  /* B = EF */
  glFinalCombinerInputNV(GL_VARIABLE_B_NV,
                         GL_E_TIMES_F_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  /* C = zero */
  glFinalCombinerInputNV(GL_VARIABLE_C_NV,
                         GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  if (specular) {
    /* D = zero = no extra specular illumination contribution */
    glFinalCombinerInputNV(GL_VARIABLE_D_NV,
                           GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  } else {
    /* D = C0 = ambient illumination contribution */
    glFinalCombinerInputNV(GL_VARIABLE_D_NV,
                           GL_CONSTANT_COLOR0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
    glFinalCombinerInputNV(GL_VARIABLE_E_NV,
                           GL_CONSTANT_COLOR1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  }
  if (specular) {
    /* E = spare0rgb = diffuse illumination contribution = H dot N' */
    glFinalCombinerInputNV(GL_VARIABLE_E_NV,
                           GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
  } else {
    if (modulateDiffuse) {
      /* This is the right thing to do for diffuse. */

      /* E = tex0 alpha = shortened length of N' */
      glFinalCombinerInputNV(GL_VARIABLE_E_NV,
                             GL_TEXTURE0_ARB, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
    } else {
      /* This is wrong, but someone may think it is cool.  If
         we do not modulate by the peturbed vector magnitude,
         the diffuse component of minified regions of the
         normal map may appear to bright. */

      /* E = 1 */
      glFinalCombinerInputNV(GL_VARIABLE_E_NV,
                             GL_ZERO, GL_UNSIGNED_INVERT_NV, GL_RGB);
    }
  }
  /* F = spare0rgb = diffuse illumination contribution = L dot N' (or H dot N' if specular) */
  glFinalCombinerInputNV(GL_VARIABLE_F_NV,
                         GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);

  /* diffuse RGB color  = A*E*F + D = diffuse modulated by self-shadowing term + ambient */
  /* specular RGB color = A*E*F     = specular modulated by self-shadowing term */

  /* G = spare0a = diffuse illumination contribution = L dot N' */
  glFinalCombinerInputNV(GL_VARIABLE_G_NV,
                         GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);

  glEnable(GL_REGISTER_COMBINERS_NV);
}
#endif

/* ------------------------------------------------ */

void drawPatch(int patchnum, double *light)
{
  //printf("light %f %f %f\n", light[0], light[1], light[2] );
  if ( nrm_flag ) {
    patch[patchnum].SetLightPos(light);
    set_diffuse_path();
    patch[patchnum].Render();

    set_decal_path();
    patch[patchnum].Render();

    //set_specular_path();
    //patch[patchnum].Render();
  } else {
    patch[patchnum].Render();
  }

}

void DrawPath(int c,int id)
{

  glLineWidth(4.0);
  glBegin(GL_LINE_STRIP);

  if(bez_flag ==1)
    {
      for(int i=0;i<(int)pathv.size();i++)
        {
          glVertex3d(pathv[i][id].x * 1.5,pathv[i][id].y * 1.5,pathv[i][id].z * 1.5);
        }
    }
  else{
    glVertex3d(pathv[0][id].x * 1.5,pathv[0][id].y * 1.5,pathv[0][id].z * 1.5);
    glVertex3d(pathv[pathv.size()-1][id].x * 1.5,
               pathv[pathv.size()-1][id].y * 1.5,pathv[pathv.size()-1][id].z * 1.5);

  }
  glEnd();
}

void Morph( void )
{
  int current=0;
  int i;

  for(i=0;i<weight.size();i++)
    {
      if(pp<weight[i])break;
    }
  current = i;//どの点の間を見るか

  if(pp>1.0)
    {
      pp=1.0;
      current--;
    }

  double dw = (pp-weight[current-1])/(weight[current]-weight[current-1]);
  if(path_flag==1){
    DrawPath(current,11); DrawPath(current,12);
  }
  std::vector<Vec3f> result;
  std::vector<double> tmp_pt, tmp_nm;
  result.clear();
  for(i=0;i<(int)pathv[current].size();i++)
    //cerr<<dw<<" "<<pp<<" "<<i<<" "<<(weight[current]-weight[current-1])<<" \n";
    // ベース補間メッシュの結果を埋め込む
    std::vector<double> tmp_pt,tmp_nm;
  for(i=0;i<(int)pathv[current].size();i++)
    {

      Vec3f rv;
      if(bez_flag==1&&(i==11 ||i ==12))
        {
          rv.x = (1.0-dw)*pathv[current-1][i].x+dw*pathv[current][i].x;
          rv.y = (1.0-dw)*pathv[current-1][i].y+dw*pathv[current][i].y;
          rv.z = (1.0-dw)*pathv[current-1][i].z+dw*pathv[current][i].z;
        }
      else
        {
          rv.x = (1.0-pp)*pathv[0][i].x+pp*pathv[pathv.size()-1][i].x;
          rv.y = (1.0-pp)*pathv[0][i].y+pp*pathv[pathv.size()-1][i].y;
          rv.z = (1.0-pp)*pathv[0][i].z+pp*pathv[pathv.size()-1][i].z;
        }
      tmp_pt.push_back(rv.x); tmp_pt.push_back(rv.y); tmp_pt.push_back(rv.z);
      result.push_back(rv);
    }

  if( bm_flag == 1 )
    {
      for(i= 0;i<(int)bidx.size();i+=1)
        {
          double v0x = tmp_pt[bidx[i][1]*3+0]-tmp_pt[bidx[i][0]*3+0];
          double v0y = tmp_pt[bidx[i][1]*3+1]-tmp_pt[bidx[i][0]*3+1];
          double v0z = tmp_pt[bidx[i][1]*3+2]-tmp_pt[bidx[i][0]*3+2];
          double v1x = tmp_pt[bidx[i][2]*3+0]-tmp_pt[bidx[i][0]*3+0];
          double v1y = tmp_pt[bidx[i][2]*3+1]-tmp_pt[bidx[i][0]*3+1];
          double v1z = tmp_pt[bidx[i][2]*3+2]-tmp_pt[bidx[i][0]*3+2];
          double nx= v0y*v1z-v0z*v1y;
          double ny= v0z*v1x-v0x*v1z;
          double nz= v0x*v1y-v0y*v1x;

          double dist = sqrt(nx*nx+ny*ny+nz*nz);

          tmp_nm.push_back(nx/dist);
          tmp_nm.push_back(ny/dist);
          tmp_nm.push_back(nz/dist);
        }

      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

      // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      //GLfloat bmat[] = {0.9,0.8,0.5,1.0};
      //glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,bmat);

      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3,GL_DOUBLE,0,&(tmp_pt[0]));

      glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE, light_diffuse);
      glMaterialfv(GL_FRONT,GL_AMBIENT,light_ambient);

      glBegin(GL_TRIANGLES);
      for(i=0;i<bidx.size();i++)
        {
          //double* np =tmp_nm.begin()+i*3;
          double* np =&(tmp_nm[i*3]);
          glNormal3dv(np);
          glArrayElement(bidx[i][0]);
          glArrayElement(bidx[i][1]);
          glArrayElement(bidx[i][2]);
        }
      glEnd();
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisable(GL_BLEND);
    }
  param.clear();
  param.push_back(1.0-pp);
  param.push_back(pp);

  //if ( bumpmapping ) {
  if ( nrm_flag )
    {
      //        if ( anm_flag )
      updateTexture( pp );
    }
  else
    {
      disable_bumpmap_path();
    }

  for( i = 0; i < patchnum; i++ )
    {
      patch[i].SetParam( param );
      patch[i].SetEdit( result[bidx[i][0]],
                        result[bidx[i][1]],
                        result[bidx[i][2]]);
    }

  for ( i = 0; i < patchnum; i++ )
    {
      drawPatch( i, light_pos0 );
      //        switch ( lightnum )
      //  	{
      //  	case 0:
	
      //  	  break;	
      //  	case 1:
      //  	  drawPatch(i, light_pos1);
      //  	  break;	
      //  	case 2:
      //  	  drawPatch(i, light_pos2);
      //  	  break;	
      //  	default:
      //  	  break;
      //  	}
    }	


  if( anm_flag )
    {
      ppp++;
      pp += STEP;
    }
}

void DrawImage( int n )
{

  int pix_i, pix_j, pix_k;
  FILE *ppmf;
  GLubyte piximage[PIXSIZE][PIXSIZE][3];
  char filename[80];
  if(n<10)sprintf(filename,"image00%d.ppm",n);
  else if(n<100)sprintf(filename,"image0%d.ppm",n);
  else sprintf(filename,"image%d.ppm",n);

  glReadPixels(0, 0, PIXSIZE, PIXSIZE, GL_RGB, GL_UNSIGNED_BYTE, piximage);

  ppmf=fopen(filename,"w");

  fprintf(ppmf,"P6\n");
  fprintf(ppmf,"%d %d\n",PIXSIZE,PIXSIZE);
  fclose(ppmf);
  ppmf=fopen(filename,"ab");
  fputc(50,ppmf);
  fputc(53,ppmf);
  fputc(53,ppmf);
  fputc(10,ppmf);

  for(pix_j=0;pix_j<PIXSIZE;pix_j++)
    for(pix_i=0;pix_i<PIXSIZE;pix_i++)
      for(pix_k=0;pix_k<3;pix_k++)
        fputc(piximage[PIXSIZE-1-pix_j][pix_i][pix_k],ppmf);
  fclose(ppmf);
}





void Create_Indexes(int col,std::vector<int>* fidx)
{
  int bb = 0;
  fidx->clear();

  for(int i=0;i<col-1;i++)
    {
      int ub = bb;//全部の大きさ
      bb =ub + col-i;
      for(int j=0;j<col-i-1;j++)
        {
          fidx->push_back(ub+j);
          fidx->push_back(ub+j+1);
          fidx->push_back(bb+j);
          if(j!=col-i-2)
            {
              fidx->push_back(bb+j);
              fidx->push_back(ub+j+1);
              fidx->push_back(bb+j+1);
            }
        }
    }
  return;
}

GLfloat angle = 0.0, angle_beta=0.0;    /* Angle of rotation for object. */
int moving, move_begin,begy;      /* For interactive object rotation. */
int size = 1;           /* Size of lines and points. */

int bef=0;
int now=0;

/*
  render gets called both by "display" (in OpenGL render mode)
*/

void render( void )
{
  //    int i;
  //    float mat[4]={1,1,1,1.0};

  glPushMatrix();
  glRotatef(angle, 0.0, 1.0, 0.0);
  glRotatef(angle_beta, 1.0, 0.0, 0.0);

  if ( !(nrm_flag) )
    {
      glEnable( GL_LIGHTING );
      glLightfv( GL_LIGHT0, GL_POSITION, light_position );
    }

  Morph();

  glPopMatrix();
}

void display( void )
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  render();
  glutSwapBuffers();

  if(img_flag == 1) DrawImage( ppp );
  return;
}

void idle( void )
{
  //    printf("aa\n");
#if 0
  light_position[0] = light_pos0[0] = 100.0 * sin(light_angle);
  light_position[1] = light_pos0[1] = 0.0;
  light_position[2] = light_pos0[2] = 100.0 * cos(light_angle);
#endif

  if ( rotate_light_flag )
    {
      light_angle += 0.1;
      if (light_angle > 2 * 3.141592) light_angle = 0.0;
    }

  glutPostRedisplay();
  //    display();
}

/* ARGSUSED2 */
void mouse( int button, int state, int x, int y )
{
  if ( button == GLUT_LEFT_BUTTON && state == GLUT_DOWN )
    {
      moving = 1;
      move_begin = x;
    }
  if ( button == GLUT_LEFT_BUTTON && state == GLUT_UP )
    {
      moving = 0;
    }
}

void key( unsigned char key , int x, int y )
{
  switch( key )
    {
    case 'z': // 解像度 UP
      res++;
      if( res > max_res ) res = max_res;
      break;

    case 'x': // 解像度 DOWN
      res--;
      if( res < 0 ) res = 0;
      break;

    case 'q':
      freeTextures();
      exit(0);
      break;

    default:
      break;
    }
  // 解像度レベルの制御
  for( int i = 0; i < patchnum; i++ )
    patch[i].Set_Resolution( res );
}

/* ARGSUSED1 */
void
motion(int x, int y)
{
  if (moving) {
    angle = angle + (x - move_begin);
    move_begin = x;
    angle_beta = angle_beta+(y-begy);
    begy=y;

    glutPostRedisplay();
  }
}

void change_image( int value )
{
  switch ( value ) {
  case 201: // 1024 x 1024
    texture_height = TEXTURE_HEIGHT1024;
    texture_size = TEXTURE_SIZE1024;
    freeTextures();
    initTextures( 0 );
    break;

  case 202: // 512 x 512
    texture_height = TEXTURE_HEIGHT512;
    texture_size = TEXTURE_SIZE512;
    freeTextures();
    initTextures( 1 );
    break;

  case 203: // 256 x 256
    texture_height = TEXTURE_HEIGHT256;
    texture_size = TEXTURE_SIZE256;
    freeTextures();
    initTextures( 2 );
    break;
  }
  updateTexCoords( texture_height, texture_size );

  glutPostRedisplay();
}

void choice( int value )
{
  int i;
  switch ( value ) {

  case 0:
    img_flag = 1;
    break;

  case 101: // normal map toggle
    nrm_flag = 1 - nrm_flag;
    break;

  case 102: // start morph
    for( i = 0; i < patchnum; i++ )
      patch[i].ResetParam();
    pp  = 0;
    ppp = 0;
    anm_flag = 1;
    break;

  case 103: // stop morph
    anm_flag = 0;
    break;

  case 104:
    rotate_light_flag = 1 - rotate_light_flag;
    break;

  case 105:
    path_flag = 1 - path_flag;
    break;

  case 106:
    bez_flag = 1 - bez_flag;
    //path_flag = 0;
    break;

  case 9:
    bm_flag = 1 - bm_flag;
    break;

  case 666:
    freeTextures();
    exit( 0 );
    break;
  }
  glutPostRedisplay();
}

int main( int argc, char **argv )
{
  int i, j;

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA | GLUT_ACCUM);

  if(argc > 0)//  if(argc>2)
    {
      int mapn;
      //      FILE* fp = fopen(argv[1],"r");

      FILE* fp;
      if ( (fp = fopen("veti.mim","r")) == NULL ) {
        fprintf(stderr, "file not found!\n");
        exit(1);
      }
      fscanf(fp,"%d %d %d", &(max_res), &(patchnum), &(mapn) );
      int vcol = (int) pow(2, max_res) + 1;

      for(i = 0; i < patchnum; i++ )
        {
          std::vector<double> pos;
          for(j=0;j<vcol*(vcol+1)/2*3;j++)
            {
              double val ;
              fscanf(fp,"%lf",&val);
              pos.push_back(val);
            }
          srcv.push_back(pos);
          pos.clear();

        }

      for( i = 0; i < patchnum; i++ )
        {
          std::vector<double> pos;
          for( j = 0; j < vcol*(vcol+1)/2*3; j++ )
            {
              double val ;
              fscanf(fp, "%lf", &(val) );
              pos.push_back( val );
            }
          patch.push_back( MR_Patch(srcv[i], pos, max_res) );
          srcv[i].clear();
          pos.clear();

          //  texcoords.clear();
          std::vector<double> texcoords;
          calcTexCoords( vcol, i, &texcoords, texture_height, texture_size );
          patch[i].SetTexCoords(texcoords);

        }

      srcv.clear();
      //fclose(fp);
      //      Read　Path　File
      //fp = fopen("veti_5.pth","r");
      int vnum,pnum;
      fscanf(fp,"%d %d",&vnum,&pnum);
      weight.clear();
      for(i = 0;i<pnum;i++)
        {
          double dw;
          fscanf(fp,"%lf",&dw);
          weight.push_back(dw);
        }
      for(j = 0;j<pnum;j++)
        {
          std::vector<Vec3f> dvec;
          for(i = 0;i<vnum;i++)
            {
              Vec3f v;
              fscanf(fp,"%lf%lf%lf",&(v.x),&(v.y),&(v.z));
              dvec.push_back(v);
            }
          pathv.push_back(dvec);
          dvec.clear();
        }
      for(i = 0;i<patchnum;i++)
        {
          std::vector<int> idxtmp;
          for(j=0;j<3;j++)
            {
              int i;
              fscanf(fp,"%d", &i);
              idxtmp.push_back(i);
            }
          bidx.push_back(idxtmp);
          idxtmp.clear();
        }
      fclose(fp);
    }
  else{
    fprintf(stderr, "usage:%s filename\n",argv[0]);
    exit(1);
  }

  // 解像度レベルの制御
  for( i = 0; i < patchnum; i++ )
    patch[i].Set_Resolution( res );

  glutInitWindowSize(PIXSIZE,PIXSIZE);
  glutCreateWindow("MIMesh Demo");
  glutDisplayFunc(display);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(key);
  glutIdleFunc(idle);

  init();
  fprintf(stdout, "default image size: 256 x 256.\n");
  fprintf(stdout, "default subdivision level: 4.\n");

  int submenu;
  submenu = glutCreateMenu( change_image );
  glutAddMenuEntry("1024x1024", 201);
  glutAddMenuEntry("512x512", 202);
  glutAddMenuEntry("256x256", 203);

  glutCreateMenu(choice);
  glutAddMenuEntry("Normal mapping", 101);
  glutAddSubMenu("Change Normal Map", submenu );
  glutAddMenuEntry("Start Morph",    102);
  glutAddMenuEntry("Stop Morph",     103);
  glutAddMenuEntry("Rotate Light",   104);
  glutAddMenuEntry("Show Path",      105);
  glutAddMenuEntry("Linear/Bezier Interpolation",106);
  glutAddMenuEntry("Quit", 666);
  glutAttachMenu( GLUT_RIGHT_BUTTON );

  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  //glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);

#if 0
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse2);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
  glEnable(GL_LIGHT2);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, light_diffuse2);
  glLightfv(GL_LIGHT3, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT3, GL_POSITION, light_position4);
  glEnable(GL_LIGHT3);
#endif

  //glEnable(GL_LIGHTING);
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  gluPerspective( /* field of view in degree */ 22.0,
                  /* aspect ratio */ 1.0,
                  /* Z near */ 5.0, /* Z far */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, 3.0,  /* eye is at (0,0,5) */
            0.0, 0.0, 0.0,      /* center is at (0,0,0) */
            0.0, 1.0, 0.);      /* up is in postivie Y direction */
  glTranslatef(0.0, 0.0, -3.0);
  /* Give the object an "interesting" orientation. */
  glEnable(GL_DEPTH_TEST);

  glClearColor( 1.0f, 1.0f, 1.0f, 0.0 );
  glClearDepth( 1.0f );

  glClearAccum(0.0, 0.0, 0.0, 0.0);
  //    glPolygonMode( GL_FRONT_AND_BACK, GL_POLYGON );
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}

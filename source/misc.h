#pragma once
#include <vector>
#include "box2d\Box2D.h"

typedef unsigned int   u32;
typedef unsigned char  u8;

inline u32  rgba2i(u8 r, u8 g, u8 b, u8 a)
{
  return u32(r | (g<<8) | (b<<16) | (a<<24));
}
inline u8   getB(u32 rgba)
{
  return u8((rgba>>16) & 0x000000ff);
}
inline u8   getA(u32 rgba)
{
  return u8((rgba>>24) & 0x000000ff);
}
inline u8   getR(u32 rgba)
{
  return u8(rgba & 0x000000ff);
}
inline u8   getG(u32 rgba)
{
  return u8((rgba>>8) & 0x000000ff);
}

inline float  getBf(u32 rgba)
{
  return getB(rgba)/255.0f;
}
inline float  getGf(u32 rgba)
{
  return getG(rgba)/255.0f;
}
inline float  getRf(u32 rgba)
{
  return getR(rgba)/255.0f;
}
inline float  getAf(u32 rgba)
{
  return getA(rgba)/255.0f;
}


const float rad2deg = 180.0f / 3.141592f;
const float deg2rad = 3.141592f / 180.0f;

struct  Input
{
  enum
  {
    Idle,
    Down,
    Hold,
    Up,
  };
  int ms[2];
  int mx;
  int my;
  int kb[255];

  Input()
  {
    ms[0] = ms[1] = 0;
    mx = my = 0;
    memset(kb, 0, sizeof(kb));
  };
};

class   ContextGL
{
  HWND  hwin;
  HDC   hdc;
  HGLRC hgl;
public:
  ContextGL() : hwin(NULL), hdc(NULL), hgl(NULL){}
  void  create(HWND hwin);
  void  destroy();
  void  swap();
};

struct Texture
{
  u32 tex_id;
  int size_x;
  int size_y;

  Texture()
  {
    tex_id = 0;
    size_x = 0;
    size_y = 0;
  }
};
Texture loadTexture(const char* filename);


struct  VtxPT
{
  float x, y;
  float u, v;
  VtxPT(float px, float py, float tu = 0.0f, float tv = 0.0f)
  {
    x = px;
    y = py;
    u = tu;
    v = tv;
  }
};
class   Mesh
{
  std::vector<VtxPT> vvtxs;
  Texture texture;
public:
  //pozycja
  float x, y;
  //rotacja w stopniach
  float roto;
  //sx,sy -> skala szerko�ci i wysko�ci
  float sx, sy;
  //przesuni�cie na teksturze w pixelach
  float tx, ty;
  //kolor rgba 
  u32   color;
  //szeroko�� obiektu(podawana przy tworzeniu)
  float size_dx;
  //wysoko�� obiektu(podawana przy tworzeniu)
  float size_dy;

  Mesh();
  //sizex/size_y - szeroko�� obiektu, 
  //rgba - kolor, 
  //tex-tekstura, 
  //tex_x0/y0 - lewy-g�rny punkt pocz�tku tekstury dla obiektu (jej wielko�� definiuje size_x/y)
  //wsp�rzedne tekstury jak w mspaint - lewy g�rny r�g -> (0,0)
  void  createRect(int size_x, int size_y, u32 rgba = 0xffffffff, const Texture* tex = NULL, int tex_x0 = 0, int tex_y0 = 0);
  //rad-promie�, 
  //tris_cnt - ile tr�jk�t�w tworzt ko�o, 
  //tex- tekstura
  //tex_x0, tex_y0 - punkt �rodkowy w teksturze dla obiektu
  void  createWheel(int rad, u32 rgba = 0xffffffff, int tris_cnt = 10, const Texture* tex = NULL, int tex_x0=0, int tex_y0=0);
  void  render();
};

typedef std::vector<Mesh*> stdv_mesh;

void  drawLine(float x0, float y0, float x1, float y1, int rgba);
void  drawLine(std::vector<b2Vec2>& v, int rgba);

class  b2World;
void  debug_draw(const b2World* world, float scale);

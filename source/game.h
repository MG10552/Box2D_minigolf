#pragma once

#include "box2d\Box2D.h"

struct Object
{
    b2Body*     body;
    Mesh*       mesh;
    int         lifetime;
    int         color;
    Object() : body(NULL),mesh(NULL), lifetime(0)
    {

    }
};

struct Filters
{
  enum Bits
  {
	  Bit_None = 0,
	  Bit_Boundary = 1,
	  Bit_Ball = 2,
	  Bit_Wall_Obstacle = 4,
	  Bit_Sensor_Obstacle = 8,
      Bit_Water = 16,
      Bit_Particle = 32,
	  Bit_Exit = 64
  };
};

class Game
{
    enum GameState{
        Map1,
        Map2
    } gameState;

    enum MapState{
		SubMap1 = 0,
		SubMap2 = 1,
		SubMap3 = 2
	  } mapState;

  int       timer;
  float     win_dx;
  float     win_dy;
  float     scroll_x, scroll_y;

  //swiat
  b2World*     b2world;
  
  
  typedef std::vector<Object*>  stdv_objs;

  //nasza pi³eczka
  Object    ball;
  bool ballAtStartPos;
  //line
  bool  bdraw_line;
  float vline_x[3], vline_y[3];
  std::vector<b2Vec2> shootRay;
  
  //lista partikli
  stdv_objs   vparts;
  //textura
  Texture     tex_0;

  void      createParticles(const b2Vec2& vpos, int parts_cnt);
  void      userInput(const Input& in);
  void      render();

  //bpilat.cpp
  struct Map1
  {
	  // Czy pod³o¿e symulowaæ dampingiem pi³eczki?
	  // Co gdy ró¿ne pi³eki maj¹ ró¿ny damping?
	  // Co gdy dwa obiekty s¹ na sobie i oba wp³ywaj¹ na pi³eczkê? Priorytet?

	  //linear damping dla pi³ki na poszczególnych submapach
	  float ballDamping[3];

	  //punkty startowe dla pi³ki
	  b2Vec2 start_points[3];

      //pole gry
	  Object boundary;

	  //przeszkody 1 podmapy i do³ek
	  Object sand_pit;
	  Object bouncer;
	  Object water;
	  Object hole_1;

	  //przeszkody 2 podmapy
	  Object windmill_static; 
      Object windmill_cross1;
      Object windmill_cross2;
	  Object hole_2;

	  //przeszkody 3 podmapy
	  Object force_field_1;
	  Object force_field_2;
	  Object force_field_3;
      Object bouncer2;
	  Object hole_3;

	  Map1() { }

  } map1;

  void  createMap1(int window_x, int window_y);
  //end bpilat.cpp

  //start of mglodowski.cpp
  struct Map2
  {
	  //linear damping dla pi³ki na poszczególnych submapach
	  float ballDamping[3];

	  //punkty startowe dla pi³ki
	  b2Vec2 start_points[3];

	  //pole gry
	  Object boundary;

	  //Objekty nale¿¹ce do pierwszej podmapy
	  Object bouncingWall;
	  Object bouncingWall2;
	  Object bouncingWall3;
	  Object bouncingWall4;
	  Object bouncingWall5;
	  Object gate_rightwing;
	  Object gate_leftwing;
	  Object waterF1;
	  Object waterF2;
	  Object hole_1;
	  Object gate_static1;
	  Object gate_static2;

	  //Objekty nale¿¹ce do drugiej podmapy
	  Object bouncer;
	  Object bouncer2;
	  Object bouncer3;
	  Object bouncer4;
	  Object bouncer5;
	  Object hole_2;

	  //Objekty nale¿¹ce do trzeciej podmapy
	  Object waterF3;
	  Object hole_3;

	  Map2() { }

  } map2;

  void  createMap2(int window_x, int window_y);
  //end of mglodowski.cpp

  //common
  void CreateBall();
  void moveBallAtPos(b2Vec2 pos);
  void shootRayCast(b2Vec2 v1, b2Vec2 v2, int bounces);
  //end of common


public:
  Game() : win_dx(0), win_dy(0){}
  void      create(int win_size_x, int win_size_y);
  void      destroy();
  void      process(const Input& in);
};
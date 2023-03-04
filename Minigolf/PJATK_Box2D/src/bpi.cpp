#include <Windows.h>
#include <gl/GL.h>
#include "box2d/Box2D.h"
#include "misc.h"
#include "game.h"

//relacja skali boxa do ekranu
//box to screen
const float b2s = 10.0f;
//screen to box
const float s2b = 1.0f/b2s;


void  Game::createMap1(int win_x, int win_y)
{
	//tworzymy statyczny obiekt, ograniczaj¹cy pole gry
	//struktura b2BodyDef domyœlnie typ ma ustawiony na b2_staticBody, dla jasnoœci przyk³adu ustawiamy j¹ jawnie
	b2BodyDef bd;
	bd.type = b2_staticBody;
	map1.boundary.body = b2world->CreateBody(&bd);
	b2FixtureDef fd;
	fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	fd.friction = 5;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	fd.restitution = 1.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
	fd.filter.categoryBits = Filters::Bit_Boundary; //boundary kolidowaæ bêdzie z ...
    fd.filter.maskBits = Filters::Bit_Ball | Filters::Bit_Particle; //... nasz¹ pi³eczk¹
	b2ChainShape cs;

	//800x600
	//SubMap1
	b2Vec2 boundary_verts[6] = { 
		s2b*b2Vec2(-390, 290), 
		s2b*b2Vec2(140, 290), 
		s2b*b2Vec2(140, -290),
		s2b*b2Vec2(-40, -290),
		s2b*b2Vec2(-40, 110),
		s2b*b2Vec2(-390, 110) };
	cs.CreateLoop(boundary_verts, 6);
	fd.shape = &cs;
	map1.boundary.body->CreateFixture(&fd);
	//SubMap2
	b2ChainShape cs2;
	b2Vec2 boundary_verts2[4] = { 
		s2b*b2Vec2(-390, 40), 
		s2b*b2Vec2(-60, 40), 
		s2b*b2Vec2(-60, -290), 
		s2b*b2Vec2(-390, -290) };
	cs2.CreateLoop(boundary_verts2, 4);
	fd.shape = &cs2;
	map1.boundary.body->CreateFixture(&fd);
	//SubMap3
	b2ChainShape cs3;
	b2Vec2 boundary_verts3[4] = { 
		s2b*b2Vec2(160, 290), 
		s2b*b2Vec2(390, 290), 
		s2b*b2Vec2(390, -290), 
		s2b*b2Vec2(160, -290) };
	cs3.CreateLoop(boundary_verts3, 4);
	fd.shape = &cs3;
	map1.boundary.body->CreateFixture(&fd);
	map1.boundary.body->SetUserData(&map1.boundary);

    //
	map1.start_points[0] = b2Vec2(s2b*-370, s2b*200);
	map1.start_points[1] = b2Vec2(s2b*-80, s2b*20);
	map1.start_points[2] = b2Vec2(s2b*275, s2b*270);

    map1.ballDamping[0] = 0.7f;
    map1.ballDamping[1] = 0.5f;
    map1.ballDamping[2] = 0.5f;


	//SubMap1 - obstacles
	// sand pit
	b2BodyDef spb;
	spb.type = b2_staticBody;
    spb.position = b2Vec2(-40*s2b,110*s2b);
    //spb.position = b2Vec2(1,1);
	map1.sand_pit.body = b2world->CreateBody(&spb);
	b2FixtureDef spfd;
	spfd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	spfd.friction = 0.9f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	spfd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
    spfd.isSensor = true;
    //filtr kolizji
	spfd.filter.categoryBits = Filters::Bit_Sensor_Obstacle; //piasek kolidowaæ bêdzie z ...
    spfd.filter.maskBits = Filters::Bit_Ball; //... niczym / nasz¹ pi³eczk¹
	b2PolygonShape sp;
    b2Vec2 sandpit_verts[4] = { 
        s2b*b2Vec2(0, 0),
		s2b*b2Vec2(-200, 0), 
		s2b*b2Vec2(-200, 60), 
		s2b*b2Vec2(0, 100) };
    sp.Set(sandpit_verts, 4);
    spfd.shape = &sp;
    map1.sand_pit.body->CreateFixture(&spfd);
    map1.sand_pit.color = rgba2i(255, 255, 0, 150);
    map1.sand_pit.body->SetUserData(&map1.sand_pit);
    
    // bouncer
	b2BodyDef bouncer_body;
	bouncer_body.type = b2_staticBody;
    bouncer_body.position = b2Vec2(120*s2b,270*s2b);
    bouncer_body.angle = 10;
	map1.bouncer.body = b2world->CreateBody(&bouncer_body);
	b2FixtureDef bouncer_fd;
	bouncer_fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncer_fd.friction = 0.0f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncer_fd.restitution = 2.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
    bouncer_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; //boundary kolidowaæ bêdzie z ...
    bouncer_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; //... particlami
    b2CircleShape bouncer_circle;
    bouncer_circle.m_radius = 4;
    bouncer_fd.shape = &bouncer_circle;
    map1.bouncer.body->CreateFixture(&bouncer_fd);
    map1.bouncer.color = rgba2i(255, 0, 255, 255);
    map1.bouncer.body->SetUserData(&map1.bouncer);

    // water
	b2BodyDef water_body;
	water_body.type = b2_staticBody;
    water_body.position = b2Vec2(-40*s2b,110*s2b);
    map1.water.body = b2world->CreateBody(&water_body);
	b2FixtureDef water_fd;
	water_fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	water_fd.friction = 5.0f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	water_fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
    water_fd.filter.categoryBits = Filters::Bit_Water; //woda kolidowaæ bêdzie z ...
	water_fd.filter.maskBits = Filters::Bit_Ball; //... nasz¹ pi³eczk¹
	b2PolygonShape water_polygon;
	//water_polygon.SetAsBox(s2b*50,s2b*400);
    b2Vec2 water_verts[5] = {
		s2b*b2Vec2(0, 0),
		s2b*b2Vec2(0, -400), 
		s2b*b2Vec2(40, -400), 
		s2b*b2Vec2(80, -200), 
        s2b*b2Vec2(50, 0)
		};

    water_polygon.Set(water_verts, 5);
    water_fd.shape = &water_polygon;
    map1.water.body->CreateFixture(&water_fd);
    map1.water.color = rgba2i(0, 0, 255, 255);
    map1.water.body->SetUserData(&map1.water);

    // hole_1
	b2BodyDef hole1_body;
	hole1_body.type = b2_staticBody;
    hole1_body.position = b2Vec2(50*s2b,-250*s2b);
    map1.hole_1.body = b2world->CreateBody(&hole1_body);
	b2FixtureDef hole1_fd;
	hole1_fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	hole1_fd.friction = 0.0f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	hole1_fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
    hole1_fd.filter.categoryBits = Filters::Bit_Exit; //boundary kolidowaæ bêdzie z ...
    hole1_fd.filter.maskBits = Filters::Bit_Ball; //... pi³eczk¹
    b2CircleShape hole1_circle;
    hole1_circle.m_radius = 1;
    hole1_fd.shape = &hole1_circle;
    map1.hole_1.body->CreateFixture(&hole1_fd);
    map1.hole_1.color = rgba2i(0, 255, 0, 255);
    map1.hole_1.body->SetUserData(&map1.hole_1);

	//SubMap2 - obstacles
    //windmill_static
    b2BodyDef windmill_static_body;
    windmill_static_body.type = b2_staticBody;
    windmill_static_body.position.Set(s2b*-225, s2b*-125);
    map1.windmill_static.body = b2world->CreateBody(&windmill_static_body);
    b2CircleShape windmill_static_cs;
    windmill_static_cs.m_radius = s2b * 20.0f;
    map1.windmill_static.body->CreateFixture(&windmill_static_cs, 1.0f);
    map1.windmill_static.color = rgba2i(160, 120, 0, 255);
    map1.windmill_static.body->SetUserData(&map1.windmill_static);

    //windmill_cross1
    b2BodyDef bd_cr;
    bd_cr.type = b2_dynamicBody;
    bd_cr.position.Set(s2b*-225, s2b*-125);
    map1.windmill_cross1.body = b2world->CreateBody(&bd_cr);
    
    b2PolygonShape ps_horz;
    ps_horz.SetAsBox(s2b * 150, s2b * 10);
    map1.windmill_cross1.body->CreateFixture(&ps_horz, 0.2f);
    b2PolygonShape ps_vert;
    ps_vert.SetAsBox(s2b * 10, s2b * 150);
    map1.windmill_cross1.body->CreateFixture(&ps_vert, 0.2f);

    map1.windmill_cross1.color = rgba2i(200, 150, 0, 255);
    map1.windmill_cross1.body->SetUserData(&map1.windmill_cross1);

    //po³¹czenie windmill_static-windmill_cross1
    b2RevoluteJointDef rjd;
    rjd.collideConnected = false;
    rjd.Initialize(map1.windmill_static.body, map1.windmill_cross1.body, map1.windmill_static.body->GetPosition());
    rjd.enableMotor = true;
    rjd.maxMotorTorque = 20;
    //rjd.motorSpeed = 10;
    b2world->CreateJoint(&rjd);
    
    // hole_2
    b2BodyDef hole2_body;
	hole2_body.type = b2_staticBody;
    hole2_body.position = b2Vec2(-360*s2b,-260*s2b);
    map1.hole_2.body = b2world->CreateBody(&hole2_body);
	b2FixtureDef hole2_fd;
	hole2_fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	hole2_fd.friction = 0.0f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	hole2_fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
    hole2_fd.filter.categoryBits = Filters::Bit_Exit; //boundary kolidowaæ bêdzie z ...
    hole2_fd.filter.maskBits = Filters::Bit_Ball; //... pi³eczk¹
    b2CircleShape hole2_circle;
    hole2_circle.m_radius = 1;
    hole2_fd.shape = &hole2_circle;
    map1.hole_2.body->CreateFixture(&hole2_fd);
    map1.hole_2.color = rgba2i(0, 255, 0, 255);
    map1.hole_2.body->SetUserData(&map1.hole_2);

	//SubMap3 - obstacles
    //FF1
    b2BodyDef ff1bd;
	ff1bd.type = b2_staticBody;
    ff1bd.position = b2Vec2(160*s2b,200*s2b);
	map1.force_field_1.body = b2world->CreateBody(&ff1bd);
	b2FixtureDef ff1fd;
	ff1fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	ff1fd.friction = 0.7f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	ff1fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
    ff1fd.isSensor = true;
    //filtr kolizji
	ff1fd.filter.categoryBits = Filters::Bit_Sensor_Obstacle; //piasek kolidowaæ bêdzie z ...
    ff1fd.filter.maskBits = Filters::Bit_Ball; //... niczym / nasz¹ pi³eczk¹
	b2PolygonShape ff1s;
    b2Vec2 ff1_verts[4] = { 
        s2b*b2Vec2(0, 0), 
		s2b*b2Vec2(0, -60), 
		s2b*b2Vec2(230, -60), 
		s2b*b2Vec2(230, 0) };
    ff1s.Set(ff1_verts, 4);
    ff1fd.shape = &ff1s;
    map1.force_field_1.body->CreateFixture(&ff1fd);
    map1.force_field_1.color = rgba2i(255, 255, 255, 150);
    map1.force_field_1.body->SetUserData(&map1.force_field_1);
    //FF2
    b2BodyDef ff2bd;
	ff2bd.type = b2_staticBody;
    ff2bd.position = b2Vec2(160*s2b,140*s2b);
	map1.force_field_2.body = b2world->CreateBody(&ff2bd);
	b2FixtureDef ff2fd;
	ff2fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	ff2fd.friction = 1.4f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	ff2fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
    ff2fd.isSensor = true;
    //filtr kolizji
	ff2fd.filter.categoryBits = Filters::Bit_Sensor_Obstacle; //piasek kolidowaæ bêdzie z ...
    ff2fd.filter.maskBits = Filters::Bit_Ball; //... niczym / nasz¹ pi³eczk¹
	b2PolygonShape ff2s;
    b2Vec2 ff2_verts[4] = { 
        s2b*b2Vec2(0, 0), 
		s2b*b2Vec2(0, -60), 
		s2b*b2Vec2(230, -60), 
		s2b*b2Vec2(230, 0) };
    ff2s.Set(ff2_verts, 4);
    ff2fd.shape = &ff2s;
    map1.force_field_2.body->CreateFixture(&ff2fd);
    map1.force_field_2.color = rgba2i(255, 255, 255, 100);
    map1.force_field_2.body->SetUserData(&map1.force_field_2);
    //FF3
    b2BodyDef ff3bd;
	ff3bd.type = b2_staticBody;
    ff3bd.position = b2Vec2(160*s2b,80*s2b);
	map1.force_field_3.body = b2world->CreateBody(&ff3bd);
	b2FixtureDef ff3fd;
	ff3fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	ff3fd.friction = 0.7f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	ff3fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
    ff3fd.isSensor = true;
    //filtr kolizji
	ff3fd.filter.categoryBits = Filters::Bit_Sensor_Obstacle; //piasek kolidowaæ bêdzie z ...
    ff3fd.filter.maskBits = Filters::Bit_Ball; //... niczym / nasz¹ pi³eczk¹
	b2PolygonShape ff3s;
    b2Vec2 ff3_verts[4] = { 
        s2b*b2Vec2(0, 0), 
		s2b*b2Vec2(0, -60), 
		s2b*b2Vec2(230, -60), 
		s2b*b2Vec2(230, 0) };
    ff3s.Set(ff3_verts, 4);
    ff3fd.shape = &ff3s;
    map1.force_field_3.body->CreateFixture(&ff3fd);
    map1.force_field_3.color = rgba2i(255, 255, 255, 150);
    map1.force_field_3.body->SetUserData(&map1.force_field_3);
    
    // bouncer
	b2BodyDef bouncer2_body;
	bouncer2_body.type = b2_staticBody;
    bouncer2_body.position = b2Vec2(275*s2b,-50*s2b);
    bouncer2_body.angle = 10;
	map1.bouncer2.body = b2world->CreateBody(&bouncer2_body);
	b2FixtureDef bouncer2_fd;
	bouncer2_fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncer2_fd.friction = 0.0f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncer2_fd.restitution = 2.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
    bouncer2_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; //boundary kolidowaæ bêdzie z ...
    bouncer2_fd.filter.maskBits = ~Filters::Bit_Particle | Filters::Bit_Ball; //... particlami
    b2CircleShape bouncer2_circle;
    bouncer2_circle.m_radius = 6;
    bouncer2_fd.shape = &bouncer_circle;
    map1.bouncer2.body->CreateFixture(&bouncer2_fd);
    map1.bouncer2.color = rgba2i(255, 0, 255, 255);
    map1.bouncer2.body->SetUserData(&map1.bouncer2);

    //hole_3
    b2BodyDef hole3_body;
	hole3_body.type = b2_staticBody;
    hole3_body.position = b2Vec2(275*s2b,-250*s2b);
    map1.hole_3.body = b2world->CreateBody(&hole3_body);
	b2FixtureDef hole3_fd;
	hole3_fd.density = 0.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	hole3_fd.friction = 0.0f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	hole3_fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
    hole3_fd.filter.categoryBits = Filters::Bit_Exit; //boundary kolidowaæ bêdzie z ...
    hole3_fd.filter.maskBits = Filters::Bit_Ball; //... pi³eczk¹
    b2CircleShape hole3_circle;
    hole3_circle.m_radius = 1;
    hole3_fd.shape = &hole3_circle;
    map1.hole_3.body->CreateFixture(&hole3_fd);
    map1.hole_3.color = rgba2i(0, 255, 0, 255);
    map1.hole_3.body->SetUserData(&map1.hole_3);

    gameState = GameState::Map1;
	mapState = MapState::SubMap1;
}
/*
void Game::CreateBall()
{
    b2BodyDef bd_bul;
    bd_bul.type = b2_dynamicBody;
    bd_bul.position = b2Vec2(-370*s2b,200*s2b);
    ball.body = b2world->CreateBody(&bd_bul);

    b2FixtureDef ball_fd;
	ball_fd.density = 1.0f; //gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	ball_fd.friction = 0.0f;   //wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	ball_fd.restitution = 0.0f; //wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	//filtr kolizji
    ball_fd.filter.categoryBits = Filters::Bit_Ball; //boundary kolidowaæ bêdzie z ...
    ball_fd.filter.maskBits = Filters::Bit_Exit | Filters::Bit_Wall_Obstacle | Filters::Bit_Boundary | Filters::Bit_Water; //... pi³eczk¹
    b2CircleShape cs_bul;
    cs_bul.m_radius = 5* s2b;
    ball_fd.shape = &cs_bul;
    ball.body->CreateFixture(&ball_fd);
    ball.color = rgba2i(255, 0, 0, 255);
    ball.body->SetUserData(&ball.body);

	ball.body->SetLinearDamping(0.80f);
    //Ball
    ballAtStartPos = true;

	
}

void Game::moveBallAtPos(b2Vec2 pos)
{
    ball.body->SetAngularVelocity(0.0f);
    ball.body->SetLinearVelocity(b2Vec2(0,0));
    ball.body->SetTransform(pos, 0);
}8/*/


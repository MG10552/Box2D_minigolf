#include <Windows.h>
#include <gl/GL.h>

#include "box2D/Box2D.h"

#include "misc.h"
#include "game.h"


// Relacja skali boxa do ekranu
// Box to screen
const float b2s = 10.0f;

// Screen to box
const float s2b = 1.0f / b2s;


void  Game::createMap2(int win_x, int win_y) {
	// Tworzymy statyczny obiekt, ograniczaj¹cy pole gry
	// Struktura b2BodyDef domyœlnie typ ma ustawiony na b2_staticBody, dla jasnoœci przyk³adu ustawiamy j¹ jawnie
	b2BodyDef bd;
	bd.type = b2_staticBody;
	map2.boundary.body = b2world -> CreateBody(&bd);
	b2FixtureDef fd;
	fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	fd.friction = 5;   // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	fd.restitution = 1.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	fd.filter.categoryBits = Filters::Bit_Boundary; // boundary kolidowaæ bêdzie z ...
	fd.filter.maskBits = Filters::Bit_Ball | Filters::Bit_Particle; // ... nasz¹ pi³eczk¹
	b2ChainShape cs;

	// Res 800x600
	// SubMap1  ->  piasek
	b2Vec2 boundary_verts[17] = {
		s2b * b2Vec2(-390, 290), s2b * b2Vec2(-390, 190),
		s2b * b2Vec2(-190, 150), s2b * b2Vec2(-90, 90),
		s2b * b2Vec2(0, -50), s2b * b2Vec2(-390, -190),
		s2b * b2Vec2(-200, -290), s2b * b2Vec2(0, -150),
		s2b * b2Vec2(0, -290), s2b * b2Vec2(190, -290),
		s2b * b2Vec2(190, -150), s2b * b2Vec2(300, -290),
		s2b * b2Vec2(390, -150), s2b * b2Vec2(190, -50),
		s2b * b2Vec2(120, 100), s2b * b2Vec2(0, 200),
		s2b * b2Vec2(-190, 290), 
	};
	cs.CreateLoop(boundary_verts, 17);
	fd.shape = &cs;
	map2.boundary.body -> CreateFixture(&fd);
	
	// SubMap2  ->  kwardrat
	b2ChainShape cs2;
	b2Vec2 boundary_verts2[4] = {
		s2b * b2Vec2(390, -120),
		s2b * b2Vec2(190, -40),
		s2b * b2Vec2(40, 290),
		s2b * b2Vec2(390, 290) 
	};
	cs2.CreateLoop(boundary_verts2, 4);
	fd.shape = &cs2;
	map2.boundary.body -> CreateFixture(&fd);
	
	// SubMap3  ->  trójk¹t
	b2ChainShape cs3;
	b2Vec2 boundary_verts3[3] = {
		s2b * b2Vec2(-390, 180),
		s2b * b2Vec2(-10, -45),
		s2b * b2Vec2(-390, -180) 
	};
	cs3.CreateLoop(boundary_verts3, 3);
	fd.shape = &cs3;
	map2.boundary.body -> CreateFixture(&fd);
	map2.boundary.body -> SetUserData(&map2.boundary);

	//
	map2.start_points[0] = b2Vec2(s2b * -370, s2b * 240);
	map2.start_points[1] = b2Vec2(s2b * 70, s2b * 270);
	map2.start_points[2] = b2Vec2(s2b * -370, s2b * 140);

	map2.ballDamping[0] = 0.9f;
	map2.ballDamping[1] = 0.9f;
	map2.ballDamping[2] = 0.9f;


	//------------------------------------------------------ SubMap1 - obstacles
	// Bouncing_wall
	b2BodyDef bouncingWall_body;
	bouncingWall_body.type = b2_staticBody;
	bouncingWall_body.position = b2Vec2(-190 * s2b, 290 * s2b);
	bouncingWall_body.angle = 0;//10;
	map2.bouncingWall.body = b2world -> CreateBody(&bouncingWall_body);
	b2FixtureDef bouncingWall_fd;
	bouncingWall_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncingWall_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncingWall_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncingWall_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncingWall_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2PolygonShape bouncingWall_polygon;
	b2Vec2 bouncingWall_verts[4] = {
		s2b * b2Vec2(190, -85), s2b * b2Vec2(-5, 5),
		s2b * b2Vec2(-5, -5), s2b * b2Vec2(190, -95) 
	};
	bouncingWall_polygon.Set(bouncingWall_verts, 4);
	bouncingWall_fd.shape = &bouncingWall_polygon;
	map2.bouncingWall.body -> CreateFixture(&bouncingWall_fd);
	map2.bouncingWall.color = rgba2i(255, 0, 255, 255);
	map2.bouncingWall.body -> SetUserData(&map2.bouncingWall);

	// Bouncing_wall2
	b2BodyDef bouncingWall2_body;
	bouncingWall2_body.type = b2_staticBody;
	bouncingWall2_body.position = b2Vec2(0 * s2b, 195 * s2b);
	bouncingWall2_body.angle = 0; //10;
	map2.bouncingWall2.body = b2world -> CreateBody(&bouncingWall2_body);
	b2FixtureDef bouncingWall2_fd;
	bouncingWall2_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncingWall2_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncingWall2_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncingWall2_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; //boundary kolidowaæ bêdzie z ...
	bouncingWall2_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; //... particlami
	b2PolygonShape bouncingWall2_polygon;
	b2Vec2 bouncingWall2_verts[4] = {
		s2b * b2Vec2(0,0), s2b * b2Vec2(5, 5),
		s2b * b2Vec2(120, -90), s2b * b2Vec2(120, -100) 
	};
	bouncingWall2_polygon.Set(bouncingWall2_verts, 4);
	bouncingWall2_fd.shape = &bouncingWall2_polygon;
	map2.bouncingWall2.body -> CreateFixture(&bouncingWall2_fd);
	map2.bouncingWall2.color = rgba2i(255, 0, 255, 255);
	map2.bouncingWall2.body -> SetUserData(&map2.bouncingWall2);

	// Bouncing_wall3
	b2BodyDef bouncingWall3_body;
	bouncingWall3_body.type = b2_staticBody;
	bouncingWall3_body.position = b2Vec2(120 * s2b, 95 * s2b);
	bouncingWall3_body.angle = 0; //10;
	map2.bouncingWall3.body = b2world -> CreateBody(&bouncingWall3_body);
	b2FixtureDef bouncingWall3_fd;
	bouncingWall3_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncingWall3_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncingWall3_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncingWall3_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // Boundary kolidowaæ bêdzie z ...
	bouncingWall3_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2PolygonShape bouncingWall3_polygon;
	b2Vec2 bouncingWall3_verts[4] = {
		s2b * b2Vec2(0,0), s2b * b2Vec2(5, 5),
		s2b * b2Vec2(75, -150), s2b * b2Vec2(70, -150) 
	};
	bouncingWall3_polygon.Set(bouncingWall3_verts, 4);
	bouncingWall3_fd.shape = &bouncingWall3_polygon;
	map2.bouncingWall3.body -> CreateFixture(&bouncingWall3_fd);
	map2.bouncingWall3.color = rgba2i(255, 0, 255, 255);
	map2.bouncingWall3.body -> SetUserData(&map2.bouncingWall3);

	// Bouncing_wall4
	b2BodyDef bouncingWall4_body;
	bouncingWall4_body.type = b2_staticBody;
	bouncingWall4_body.position = b2Vec2(-390 * s2b, 190 * s2b);
	bouncingWall4_body.angle = 0; //10;
	map2.bouncingWall4.body = b2world -> CreateBody(&bouncingWall4_body);
	b2FixtureDef bouncingWall4_fd;
	bouncingWall4_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncingWall4_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncingWall4_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncingWall4_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncingWall4_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2PolygonShape bouncingWall4_polygon;
	b2Vec2 bouncingWall4_verts[4] = {
		s2b * b2Vec2(0,0), s2b * b2Vec2(0, 5),
		s2b * b2Vec2(200, -35), s2b * b2Vec2(200, -40) 
	};
	bouncingWall4_polygon.Set(bouncingWall4_verts, 4);
	bouncingWall4_fd.shape = &bouncingWall4_polygon;
	map2.bouncingWall4.body -> CreateFixture(&bouncingWall4_fd);
	map2.bouncingWall4.color = rgba2i(255, 0, 255, 255);
	map2.bouncingWall4.body -> SetUserData(&map2.bouncingWall4);

	// Bouncing_wall5
	b2BodyDef bouncingWall5_body;
	bouncingWall5_body.type = b2_staticBody;
	bouncingWall5_body.position = b2Vec2(-90 * s2b, 90 * s2b);
	bouncingWall5_body.angle = 0; //10;
	map2.bouncingWall5.body = b2world -> CreateBody(&bouncingWall5_body);
	b2FixtureDef bouncingWall5_fd;
	bouncingWall5_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncingWall5_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncingWall5_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncingWall5_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncingWall5_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2PolygonShape bouncingWall5_polygon;
	b2Vec2 bouncingWall5_verts[4] = {
		s2b * b2Vec2(0,0), s2b * b2Vec2(0, 5),
		s2b * b2Vec2(95, -140), s2b * b2Vec2(90, -140) 
	};
	bouncingWall5_polygon.Set(bouncingWall5_verts, 4);
	bouncingWall5_fd.shape = &bouncingWall5_polygon;
	map2.bouncingWall5.body -> CreateFixture(&bouncingWall5_fd);
	map2.bouncingWall5.color = rgba2i(255, 0, 255, 255);
	map2.bouncingWall5.body -> SetUserData(&map2.bouncingWall5);

	// Water field1
	b2BodyDef waterF1_body;
	waterF1_body.type = b2_staticBody;
	waterF1_body.position = b2Vec2(30 * s2b, -150 * s2b);
	map2.waterF1.body = b2world -> CreateBody(&waterF1_body);
	b2FixtureDef waterF1_fd;
	waterF1_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	waterF1_fd.friction = 5.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	waterF1_fd.restitution = 0.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	waterF1_fd.filter.categoryBits = Filters::Bit_Water; // Woda kolidowaæ bêdzie z ...
	waterF1_fd.filter.maskBits = Filters::Bit_Ball; // ... nasz¹ pi³eczk¹
	b2PolygonShape waterF1_polygon;
	b2Vec2 waterF1_verts[4] = {
		s2b * b2Vec2(130, 0), s2b * b2Vec2(130, -120),
		s2b * b2Vec2(0, -120), s2b * b2Vec2(0, 0) 
	};
	waterF1_polygon.Set(waterF1_verts, 4);
	waterF1_fd.shape = &waterF1_polygon;
	map2.waterF1.body -> CreateFixture(&waterF1_fd);
	map2.waterF1.color = rgba2i(0, 0, 255, 255);
	map2.waterF1.body -> SetUserData(&map2.waterF1);

	// Water field2
	b2BodyDef waterF2_body;
	waterF2_body.type = b2_staticBody;
	waterF2_body.position = b2Vec2(220 * s2b, -90 * s2b);
	map2.waterF2.body = b2world -> CreateBody(&waterF2_body);
	b2FixtureDef waterF2_fd;
	waterF2_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	waterF2_fd.friction = 5.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	waterF2_fd.restitution = 0.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	waterF2_fd.filter.categoryBits = Filters::Bit_Water; // woda kolidowaæ bêdzie z ...
	waterF2_fd.filter.maskBits = Filters::Bit_Ball; // ... nasz¹ pi³eczk¹
	b2PolygonShape waterF2_polygon;
	b2Vec2 waterF2_verts[4] = {
		s2b * b2Vec2(140, -70), s2b * b2Vec2(80, -170),
		s2b * b2Vec2(-20, -40), s2b * b2Vec2(0, 0) 
	};
	waterF2_polygon.Set(waterF2_verts, 4);
	waterF2_fd.shape = &waterF2_polygon;
	map2.waterF2.body -> CreateFixture(&waterF2_fd);
	map2.waterF2.color = rgba2i(0, 0, 255, 255);
	map2.waterF2.body -> SetUserData(&map2.waterF2);

	// Gate_static1 
	b2BodyDef gate_static1_body;
	gate_static1_body.type = b2_staticBody;
	gate_static1_body.position.Set(s2b * -15, s2b * -60);
	map2.gate_static1.body = b2world -> CreateBody(&gate_static1_body);
	b2CircleShape gate_static1_cs;
	gate_static1_cs.m_radius = s2b * 1.0f;
	map2.gate_static1.body -> CreateFixture(&gate_static1_cs, 1.0f);
	map2.gate_static1.color = rgba2i(160, 120, 0, 255);
	map2.gate_static1.body -> SetUserData(&map2.gate_static1);

	// Gate_rightwing
	b2BodyDef bd_cr;
	bd_cr.type = b2_dynamicBody;
	bd_cr.position.Set(s2b * -15, s2b * -82);
	map2.gate_rightwing.body = b2world -> CreateBody(&bd_cr);

	//Tworzenie skrzyd³a drzwi
	b2PolygonShape ps_vert;
	ps_vert.SetAsBox(s2b * 6, s2b * 22);
	map2.gate_rightwing.body -> CreateFixture(&ps_vert, 0.4f);
	map2.gate_rightwing.color = rgba2i(200, 150, 0, 255);
	map2.gate_rightwing.body -> SetUserData(&map2.gate_rightwing);

	// Po³¹czenie bazy bramy ze skrzyd³em drzwi
	b2RevoluteJointDef rjd;
	rjd.collideConnected = false;
	rjd.Initialize(map2.gate_static1.body, map2.gate_rightwing.body, map2.gate_static1.body -> GetPosition());
	rjd.enableMotor = true;
	rjd.maxMotorTorque = 20;
	//rjd.motorSpeed = 10;
	b2world -> CreateJoint(&rjd);

	// Gate_static2 
	b2BodyDef gate_static2_body;
	gate_static2_body.type = b2_staticBody;
	gate_static2_body.position.Set(s2b * -15, s2b * -150);
	map2.gate_static2.body = b2world -> CreateBody(&gate_static2_body);
	b2CircleShape gate_static2_cs;
	gate_static1_cs.m_radius = s2b * 1.0f;
	map2.gate_static2.body -> CreateFixture(&gate_static1_cs, 1.0f);
	map2.gate_static2.color = rgba2i(160, 120, 0, 255);
	map2.gate_static2.body -> SetUserData(&map2.gate_static2);

	// Gate_leftwing
	b2BodyDef bd_cr2;
	bd_cr2.type = b2_dynamicBody;
	bd_cr2.position.Set(s2b * -15, s2b * -128);
	map2.gate_leftwing.body = b2world -> CreateBody(&bd_cr2);

	// Tworzenie skrzyd³a drzwi
	b2PolygonShape ps_vert2;
	ps_vert2.SetAsBox(s2b * 6, s2b * 22);
	map2.gate_leftwing.body -> CreateFixture(&ps_vert2, 0.4f);
	map2.gate_leftwing.color = rgba2i(200, 150, 0, 255);
	map2.gate_leftwing.body -> SetUserData(&map2.gate_leftwing);

	// Po³¹czenie bazy bramy ze skrzyd³em drzwi
	b2RevoluteJointDef rjd2;
	rjd2.collideConnected = false;
	rjd2.Initialize(map2.gate_static2.body, map2.gate_leftwing.body, map2.gate_static2.body -> GetPosition());
	rjd2.enableMotor = true;
	rjd2.maxMotorTorque = 5;
	//rjd.motorSpeed = 10;
	b2world -> CreateJoint(&rjd2);

	// Hole_1
	b2BodyDef hole1_body;
	hole1_body.type = b2_staticBody;
	hole1_body.position = b2Vec2(-200 * s2b, -260 * s2b); // loc
	map2.hole_1.body = b2world -> CreateBody(&hole1_body);
	b2FixtureDef hole1_fd;
	hole1_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	hole1_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	hole1_fd.restitution = 0.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	hole1_fd.filter.categoryBits = Filters::Bit_Exit; // boundary kolidowaæ bêdzie z ...
	hole1_fd.filter.maskBits = Filters::Bit_Ball; // ... pi³eczk¹
	b2CircleShape hole1_circle;
	hole1_circle.m_radius = 1;
	hole1_fd.shape = &hole1_circle;
	map2.hole_1.body -> CreateFixture(&hole1_fd);
	map2.hole_1.color = rgba2i(0, 255, 0, 255); //color
	map2.hole_1.body -> SetUserData(&map2.hole_1);


	//------------------------------------------------------ SubMap2 - obstacles
	// bouncer Snowman
	b2BodyDef bouncer_body;
	bouncer_body.type = b2_staticBody;
	bouncer_body.position = b2Vec2(230 * s2b, 260 * s2b);
	bouncer_body.angle = 10;
	map2.bouncer.body = b2world -> CreateBody(&bouncer_body);
	b2FixtureDef bouncer_fd;
	bouncer_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncer_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncer_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	// Filtr kolizji
	bouncer_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncer_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2CircleShape bouncer_circle;
	bouncer_circle.m_radius = 3;
	bouncer_fd.shape = &bouncer_circle;
	map2.bouncer.body -> CreateFixture(&bouncer_fd);
	map2.bouncer.color = rgba2i(255, 0, 255, 255);
	map2.bouncer.body -> SetUserData(&map2.bouncer);

	// Bouncer2 Snowman
	b2BodyDef bouncer2_body;
	bouncer2_body.type = b2_staticBody;
	bouncer2_body.position = b2Vec2(330 * s2b, 200 * s2b);
	bouncer2_body.angle = 10;
	map2.bouncer2.body = b2world -> CreateBody(&bouncer2_body);
	b2FixtureDef bouncer2_fd;
	bouncer2_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncer2_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncer2_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncer2_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncer2_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2CircleShape bouncer2_circle;
	bouncer2_circle.m_radius = 3;
	bouncer2_fd.shape = &bouncer2_circle;
	map2.bouncer2.body -> CreateFixture(&bouncer_fd);
	map2.bouncer2.color = rgba2i(255, 0, 255, 255);
	map2.bouncer2.body -> SetUserData(&map2.bouncer2);

	// Bouncer3 Snowman
	b2BodyDef bouncer3_body;
	bouncer3_body.type = b2_staticBody;
	bouncer3_body.position = b2Vec2(130 * s2b, 160 * s2b);
	bouncer3_body.angle = 10;
	map2.bouncer3.body = b2world -> CreateBody(&bouncer3_body);
	b2FixtureDef bouncer3_fd;
	bouncer3_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncer3_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncer3_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncer3_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncer3_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2CircleShape bouncer3_circle;
	bouncer3_circle.m_radius = 3;
	bouncer3_fd.shape = &bouncer3_circle;
	map2.bouncer3.body -> CreateFixture(&bouncer_fd);
	map2.bouncer3.color = rgba2i(255, 0, 255, 255);
	map2.bouncer3.body -> SetUserData(&map2.bouncer3);

	// Bouncer4 Snowman
	b2BodyDef bouncer4_body;
	bouncer4_body.type = b2_staticBody;
	bouncer4_body.position = b2Vec2(260 * s2b, 120 * s2b);
	bouncer4_body.angle = 10;
	map2.bouncer4.body = b2world -> CreateBody(&bouncer4_body);
	b2FixtureDef bouncer4_fd;
	bouncer4_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncer4_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncer4_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncer4_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncer4_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2CircleShape bouncer4_circle;
	bouncer4_circle.m_radius = 3;
	bouncer4_fd.shape = &bouncer_circle;
	map2.bouncer4.body -> CreateFixture(&bouncer_fd);
	map2.bouncer4.color = rgba2i(255, 0, 255, 255);
	map2.bouncer4.body -> SetUserData(&map2.bouncer4);

	// Bouncer5 Snowman
	b2BodyDef bouncer5_body;
	bouncer5_body.type = b2_staticBody;
	bouncer5_body.position = b2Vec2(270 * s2b, 0 * s2b);
	bouncer5_body.angle = 10;
	map2.bouncer5.body = b2world -> CreateBody(&bouncer5_body);
	b2FixtureDef bouncer5_fd;
	bouncer5_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	bouncer5_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	bouncer5_fd.restitution = 2.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	bouncer5_fd.filter.categoryBits = Filters::Bit_Wall_Obstacle; // boundary kolidowaæ bêdzie z ...
	bouncer5_fd.filter.maskBits = -Filters::Bit_Particle | Filters::Bit_Ball; // ... particlami
	b2CircleShape bouncer5_circle;
	bouncer5_circle.m_radius = 3;
	bouncer5_fd.shape = &bouncer5_circle;
	map2.bouncer5.body -> CreateFixture(&bouncer5_fd);
	map2.bouncer5.color = rgba2i(255, 0, 255, 255);
	map2.bouncer5.body -> SetUserData(&map2.bouncer5);

	// Hole_2
	b2BodyDef hole2_body;
	hole2_body.type = b2_staticBody;
	hole2_body.position = b2Vec2(360 * s2b, -70 * s2b); // loc
	map2.hole_2.body = b2world -> CreateBody(&hole2_body);
	b2FixtureDef hole2_fd;
	hole2_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	hole2_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	hole2_fd.restitution = 0.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	hole2_fd.filter.categoryBits = Filters::Bit_Exit; // boundary kolidowaæ bêdzie z ...
	hole2_fd.filter.maskBits = Filters::Bit_Ball; // ... pi³eczk¹
	b2CircleShape hole2_circle;
	hole2_circle.m_radius = 1;
	hole2_fd.shape = &hole2_circle;
	map2.hole_2.body -> CreateFixture(&hole2_fd);
	map2.hole_2.color = rgba2i(0, 255, 0, 255); // color
	map2.hole_2.body -> SetUserData(&map2.hole_2);

	//------------------------------------------------------ SubMap3 - obstacles
	// WaterF3
	b2BodyDef waterF3_body;
	waterF3_body.type = b2_staticBody;
	waterF3_body.position = b2Vec2(-390 * s2b, 120 * s2b);
	map2.waterF3.body = b2world -> CreateBody(&waterF3_body);
	b2FixtureDef waterF3_fd;
	waterF3_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	waterF3_fd.friction = 5.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	waterF3_fd.restitution = 0.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	waterF3_fd.filter.categoryBits = Filters::Bit_Water; // woda kolidowaæ bêdzie z ...
	waterF3_fd.filter.maskBits = Filters::Bit_Ball; // ... nasz¹ pi³eczk¹
	b2PolygonShape waterF3_polygon; // loc
	b2Vec2 waterF3_verts[3] = {
		s2b * b2Vec2(0, 0), s2b * b2Vec2(0, -250),
		s2b * b2Vec2(290, -150) 
	};
	waterF3_polygon.Set(waterF3_verts, 3);
	waterF3_fd.shape = &waterF3_polygon;
	map2.waterF3.body -> CreateFixture(&waterF3_fd);
	map2.waterF3.color = rgba2i(0, 0, 255, 255);
	map2.waterF3.body -> SetUserData(&map2.waterF3);

	// Hole_3
	b2BodyDef hole3_body;
	hole3_body.type = b2_staticBody;
	hole3_body.position = b2Vec2(-360 * s2b, -150 * s2b); // loc
	map2.hole_3.body = b2world -> CreateBody(&hole3_body);
	b2FixtureDef hole3_fd;
	hole3_fd.density = 0.0f; // gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
	hole3_fd.friction = 0.0f; // wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
	hole3_fd.restitution = 0.0f; // wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
	
	// Filtr kolizji
	hole3_fd.filter.categoryBits = Filters::Bit_Exit; // boundary kolidowaæ bêdzie z ...
	hole3_fd.filter.maskBits = Filters::Bit_Ball; // ... pi³eczk¹
	b2CircleShape hole3_circle;
	hole3_circle.m_radius = 1;
	hole3_fd.shape = &hole3_circle;
	map2.hole_3.body -> CreateFixture(&hole3_fd);
	map2.hole_3.color = rgba2i(0, 255, 0, 255); // color
	map2.hole_3.body -> SetUserData(&map2.hole_3);

	gameState = GameState::Map2;
	mapState = MapState::SubMap1;
}
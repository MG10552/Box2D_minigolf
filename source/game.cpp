#include <Windows.h>
#include <gl/GL.h>
#include "cmath"

#include "box2D/Box2D.h"
#include "misc.h"
#include "game.h"


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// Relacja skali boxa do ekranu
// Box to screen
const float b2s = 10.0f;

// Screen to box
const float s2b = 1.0f / b2s;


class RaycastQuery: public b2RayCastCallback {
    public:
        float fraction_t;
        b2Vec2 vpt;
        b2Vec2 normal;
        RaycastQuery() {
            fraction_t = 1.0f;
            vpt.SetZero();
        }

        float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& norm, float32 fraction) {
            const b2Filter filter = fixture -> GetFilterData();

            if (filter.categoryBits & (Filters::Bit_Boundary | Filters::Bit_Wall_Obstacle)) {
                if (fraction < fraction_t) {
                    fraction_t = fraction;
                    vpt = point;
                    normal = norm;
                }
            }
            return fraction_t;
        }
};

void Game::shootRayCast(b2Vec2 v1, b2Vec2 v2, int bounces) {
    if (bounces > 0) {
        shootRay.push_back(b2Vec2(b2s * v1.x, b2s * v1.y));
        //b2Vec2 r2 =  v1 + b2Mul(b2Rot(ball.body -> GetAngle()), b2Vec2(s2b * win_dx, 0));

        RaycastQuery rc_query;
        b2world -> RayCast(&rc_query, v1, v2);
        
        if (rc_query.fraction_t < 1.0f) {
            //shootRay.push_back(b2Vec2(b2s * rc_query.vpt.x,b2s * rc_query.vpt.y));
            b2Vec2 v3;
            v3.x = rc_query.vpt.x + (1 * s2b * rc_query.normal.x);
            v3.y = rc_query.vpt.y + (1 * s2b * rc_query.normal.y);

            b2Vec2 n = -2 * b2Dot(v3 - v1, rc_query.normal) * rc_query.normal + (v3 - v1);
            float rot = std::atan2(n.y, n.x);
            b2Vec2 v4 = v3 + b2Mul(b2Rot(rot), b2Vec2(s2b * win_dx, 0));

            shootRayCast(v3, v4, bounces - 1);
        
        } else {
            shootRay.push_back(b2Vec2(b2s * v2.x, b2s * v2.y));
        }
    }
}

void Game::createParticles(const b2Vec2& vpos, int parts_cnt) {
    b2BodyDef bd;
    bd.type = b2_dynamicBody;
    bd.position = vpos;
    b2FixtureDef fd;
    fd.density = 1.0f;
    
    // Nie koliduj¹ ze sob¹
    fd.filter.categoryBits = ~Filters::Bit_Particle;
    
    // Nie koliiduj¹ z bulletami
    fd.filter.maskBits &= ~Filters::Bit_Ball;
    fd.filter.maskBits |= Filters::Bit_Wall_Obstacle | Filters::Bit_Boundary;
    b2CircleShape cs;
    cs.m_radius = s2b * 2.0f;
    fd.shape = &cs;
    
    for (int q = 0; q < parts_cnt; ++q) {
        bd.linearVelocity = b2Mul(b2Rot(deg2rad * q * 360.0f / parts_cnt), b2Vec2(0, s2b * (10.0f + rand() % 40)));
        Object* obj = new Object();
        obj -> color = rgba2i(255, 0, 0, 255);
        obj -> body = b2world -> CreateBody(&bd);
        obj -> body -> CreateFixture(&fd);
        obj -> body -> SetUserData(&obj);
        vparts.push_back(obj);
    }
}

void Game::create(int win_size_x, int win_size_y) {
    timer = 0;
    win_dx = (float)win_size_x;
    win_dy = (float)win_size_y;
    scroll_x = 0;
    scroll_y = 0;

    tex_0 = loadTexture("assets/sample_tex.png");

    // Tworzenie œwiata - bez grawitacji
    b2world = new b2World(b2Vec2(0, 0));

    // Tworzenie mapy
    createMap1(win_dx / 2, win_dy / 2);
    // CreateMap2(win_dx/2, win_dy/2);

    // Tworzymy kulkê
    CreateBall();
}

void  Game::userInput(const Input& in) {
    if (in.kb[VK_LEFT] == Input::Hold) { }
    if (in.kb[VK_RIGHT] == Input::Hold) { }
    if (in.kb[VK_UP] == Input::Hold) { }
    if (in.kb[VK_DOWN] == Input::Hold) { }
    if (in.kb[VK_SPACE] == Input::Down || in.ms[0] == Input::Down) { }

    // Raycast
    bdraw_line = false;
    
    if (ball.body -> GetLinearVelocity() == b2Vec2_zero && (in.kb[VK_LSHIFT] == Input::Hold || in.ms[1] == Input::Hold)) {
        bdraw_line = true;

        b2Vec2 mp = b2Vec2(in.mx * s2b, in.my * s2b);
        b2Vec2 bp = ball.body -> GetPosition();
        b2Vec2 n = b2Vec2((mp.x - bp.x), (mp.y - bp.y));
        float force = n.Length() / 10;
        float brot = std::atan2(n.y, n.x);
        ball.body -> SetTransform(bp, brot);

        if (in.kb[VK_SPACE] == Input::Down || in.ms[0] == Input::Down) {
            ball.body -> ApplyLinearImpulse(b2Vec2(n.x * force, n.y * force), bp, true);
            //createParticles(mp, 16);
        }
    }
}

void  Game::process(const Input& in) {
    stdv_objs vobj2del;
    vobj2del.reserve(64);
    b2world -> Step(1.0f / 60, 10, 10);
    userInput(in);

    if (bdraw_line) {
        shootRay.clear();
        b2Vec2 v0 = ball.body -> GetPosition();
        b2Vec2 v1 = v0 + b2Mul(b2Rot(ball.body -> GetAngle()), b2Vec2(s2b * win_dx, 0));
        shootRayCast(v0, v1, 4); // 4 wierzcho³ki linii - 2 odbicia
    }

    // Kolizja pi³ki
    for (b2ContactEdge* ce = ball.body -> GetContactList(); ce != NULL; ce = ce -> next) {
        // true  ->  kontakt nast¹pi³, 
        // false  ->  jedynie pr¹stok¹ty opisuj¹ce sie pokrywaj¹

        if (ce -> contact -> IsTouching()) {
            b2Filter filter = ce -> other -> GetFixtureList() -> GetFilterData();

            // Kolizja z wod¹ 
            if (filter.categoryBits & Filters::Bit_Water) {
                ballAtStartPos = true;
            }

            // Kolizja z do³kiem
            if (filter.categoryBits & Filters::Bit_Exit) {
                mapState = (MapState)((mapState + 1) % 3);
                ballAtStartPos = true;
            }

            // Kolizja z map¹
            if (filter.categoryBits & (Filters::Bit_Boundary | Filters::Bit_Wall_Obstacle)) {
                b2WorldManifold wm;
                ce -> contact -> GetWorldManifold(&wm);
                createParticles(wm.points[0], 8);
            }

            // Kolizja z map¹
            if (filter.categoryBits & (Filters::Bit_Sensor_Obstacle)) {
                /* b2Vec2 force = b2Vec2();
                force *= ce -> other -> GetFixtureList()[0].GetFriction();
                ball.body -> ApplyLinearImpulse(force, ball.body -> GetPosition(),true); */

                b2Vec2 force = ball.body -> GetLinearVelocity();
                force *= ce -> other -> GetFixtureList()[0].GetFriction();
                ball.body -> SetLinearVelocity(force);
            }
        }
    }

    // Pi³eczka ma byæ przemieszczona na pocz¹tek "aktualnego" etapu?
    if (ballAtStartPos) {
        switch (gameState) {
            case Game::Map1:
                ball.body -> SetLinearDamping(map1.ballDamping[mapState]);
                moveBallAtPos(map1.start_points[mapState]);
                ballAtStartPos = false;
                break;

            case Game::Map2:
                ball.body -> SetLinearDamping(map2.ballDamping[mapState]);
                moveBallAtPos(map2.start_points[mapState]);
                ballAtStartPos = false;
                break;
        
            default:
                break;
        }
    }

    // Particle - usuwamy jeœli lifietime > 1s
    for (size_t p = 0; p < vparts.size(); ++p) {
        if (++vparts[p] -> lifetime > 1 * 60) {
            // Dodaj do listy do usuniecia
            vobj2del.push_back(vparts[p]);
            vparts[p] = NULL;
        }
    }

    vparts.erase(std::remove(vparts.begin(), vparts.end(), (Object*)NULL), vparts.end());

    // Niszczy obiekty i usuwamy zniszczone z list
    // Do wyjaœnienia na zajêciach
    std::sort(vobj2del.begin(), vobj2del.end());
    Object* last_destroyed = NULL;
    
    for (size_t q = 0; q < vobj2del.size(); ++q) {
        if (last_destroyed != vobj2del[q]) {
            last_destroyed = vobj2del[q];
            b2world -> DestroyBody(vobj2del[q] -> body);

            delete vobj2del[q] -> mesh;
            delete vobj2del[q];
        }
    }

    if (ball.body -> GetLinearVelocity().Length() < 10 * s2b)
        ball.body -> SetLinearVelocity(b2Vec2_zero);
    
    render();
}

void  Game::destroy() {
    glDeleteTextures(1, &tex_0.tex_id);
    delete b2world;
    b2world = NULL;
}

void  Game::render() {
    // GL global settings
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.0f);
    
    //Camera
    glViewport(0, 0, (int)win_dx, (int)win_dy);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0f, 1.0f, -win_dy / win_dx, win_dy / win_dx, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glScalef(2.0f / win_dx, 2.0f / win_dx, 1.0f);
    glTranslatef(-scroll_x, -scroll_y, -1.0f);

    //Clear screen
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    //Render
    glPushMatrix();

    debugDraw(b2world, b2s);

    /* for(size_t a = 0; a<vasteroids.size(); ++a)
      vasteroids[a] -> mesh -> render(); */

    if (bdraw_line) {
        // drawLine(vline_x[0], vline_y[0], vline_x[1], vline_y[1], rgba2i(255, 255, 255, 255));
        // drawLine(vline_x[1], vline_y[1], vline_x[2], vline_y[2], rgba2i(255, 255, 255, 255));
        drawLine(shootRay, rgba2i(255, 255, 255, 255));
    }

    glPopMatrix();
}

void Game::CreateBall() {
    b2BodyDef bd_bul;
    bd_bul.type = b2_dynamicBody;
    bd_bul.position = b2Vec2(-370 * s2b, 200 * s2b);
    ball.body = b2world -> CreateBody(&bd_bul);

    b2FixtureDef ball_fd;

    // Gêstoœæ materia³u - dla obiektu statycznego nie ma to znaczenia, dla obiektów dynamicznych - gêstoœæ*pole powierzchni = masa obiektu.
    ball_fd.density = 1.0f; 

    // Wspó³czynnik tarcia przy styku cia³ - w tym przyk³adzie mo¿e byæ dowolny
    ball_fd.friction = 0.0f;

    // Wspó³czynnik odbicia - mno¿nik dla prêdkoœci odbicia po kolizji; 0 wygasza odbicie
    ball_fd.restitution = 0.0f;
    
    // Filtr kolizji
    // Boundary kolidowaæ bêdzie z ...
    ball_fd.filter.categoryBits = Filters::Bit_Ball;
    //... pi³eczk¹
    ball_fd.filter.maskBits = Filters::Bit_Exit | Filters::Bit_Wall_Obstacle | Filters::Bit_Boundary | Filters::Bit_Water | Filters::Bit_Sensor_Obstacle; 
    b2CircleShape cs_bul;
    cs_bul.m_radius = 5 * s2b;
    ball_fd.shape = &cs_bul;
    ball.body -> CreateFixture(&ball_fd);
    ball.color = rgba2i(255, 0, 0, 255);
    ball.body -> SetUserData(&ball.body);

    ball.body -> SetLinearDamping(0.80f);
    // Ball
    ballAtStartPos = true;
}

void Game::moveBallAtPos(b2Vec2 pos) {
    ball.body -> SetAngularVelocity(0.0f);
    ball.body -> SetLinearVelocity(b2Vec2(0, 0));
    ball.body -> SetTransform(pos, 0);
}
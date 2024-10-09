#include <Windows.h>
#include <wincodec.h>
#include <GL/GL.h>

#include "misc.h"
#include "game.h"

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "opengl32.lib")

// Context GL
void ContextGL::create(HWND hwin) {
    this -> hwin = hwin;
    PIXELFORMATDESCRIPTOR pfd = {
      sizeof(PIXELFORMATDESCRIPTOR),
      1,
      PFD_SUPPORT_COMPOSITION | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_GENERIC_ACCELERATED | PFD_DOUBLEBUFFER, // Flags
      PFD_TYPE_RGBA,
      32, // Rgba-bits
      0, 0, 0, 0, 0, 0,
      0,
      0,
      0,
      0, 0, 0, 0,
      0, // Depth-bits
      0, // Stencil-bits
      0,
      PFD_MAIN_PLANE,
      0,
      0, 0, 0
    };

    hdc = GetDC(hwin);
    int  pfi = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pfi, &pfd);
    HGLRC hgl = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hgl);

    typedef void(__stdcall *glProc)(int);
    glProc wglSwapIntervalProc = (glProc)wglGetProcAddress("wglSwapIntervalEXT");
    
    if (wglSwapIntervalProc)
        wglSwapIntervalProc(1);
}

void ContextGL::destroy() {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hgl);
}

void ContextGL::swap() {
    SwapBuffers(hdc);
}

Texture loadTexture(const char *filename) {
    wchar_t wfilename[MAX_PATH];
    mbstowcs(wfilename, filename, MAX_PATH - 1);
    unsigned tex_channels = 0;

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    IWICImagingFactory* wicFactory = NULL;
    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
    IWICBitmapDecoder* wicDecoder = NULL;
    wicFactory -> CreateDecoderFromFilename(wfilename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder);
    IWICBitmapFrameDecode* wicBitmapFrame = NULL;
    wicDecoder -> GetFrame(0, &wicBitmapFrame);
    wicDecoder -> Release();
    IWICBitmapSource* wicBitmapSrc = NULL;
    wicBitmapFrame -> QueryInterface(IID_IWICBitmapSource, (void**)&wicBitmapSrc);
    wicBitmapFrame -> Release();
    IWICFormatConverter* wicConverter = NULL;
    wicFactory -> CreateFormatConverter(&wicConverter);

    WICPixelFormatGUID wic_pix_fmt;
    wicBitmapSrc -> GetPixelFormat(&wic_pix_fmt);
    IWICPixelFormatInfo* wicPixFmtInfo = NULL;
    IWICComponentInfo* wicCompInfo = NULL;
    wicFactory -> CreateComponentInfo(wic_pix_fmt, &wicCompInfo);
    wicCompInfo -> QueryInterface(IID_IWICPixelFormatInfo, (void**)&wicPixFmtInfo);
    UINT tex_bpp = 0;
    wicPixFmtInfo -> GetBitsPerPixel(&tex_bpp);
    wicPixFmtInfo -> GetChannelCount(&tex_channels);
    wicPixFmtInfo -> Release();
    wicPixFmtInfo = NULL;
    wicCompInfo -> Release();
    wicCompInfo = NULL;
    wicConverter -> Initialize(wicBitmapSrc, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeCustom);

    IWICBitmapSource* wicBitmapDst = NULL;
    wicConverter -> QueryInterface(IID_PPV_ARGS(&wicBitmapDst));
    wicConverter -> Release();
    UINT  tex_dx = 0, tex_dy = 0;
    wicBitmapDst -> GetSize(&tex_dx, &tex_dy);
    UINT  tex_size = tex_dx * tex_dy * tex_bpp / 8;
    void* tex_ptr = malloc(tex_size);
    wicBitmapDst -> CopyPixels(NULL, tex_dx * tex_bpp / 8, tex_size, (BYTE*)tex_ptr);
    wicBitmapSrc -> Release();
    wicBitmapDst -> Release();
    wicFactory -> Release();
    CoUninitialize();

    Texture tex;
    tex.tex_id = 0;
    tex.size_x = tex_dx;
    tex.size_y = tex_dy;

    glGenTextures(1, (UINT*)&tex.tex_id);
    glBindTexture(GL_TEXTURE_2D, tex.tex_id);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)tex.size_x, (int)tex.size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_ptr);

    free(tex_ptr);

    return tex;
}

// Mesh
Mesh::Mesh() {
    x = y = 0;
    roto = 0;
    sx = sy = 1.0f;
    color = 0xffffffff;
    size_dx = 0;
    size_dy = 0;
    tx = ty = 0;
}

void Mesh::createRect(int size_x, int size_y, u32 rgba, const Texture* tex, int tex_x0, int tex_y0) {
    color = rgba;

    if (tex)
        texture = *tex;
    else
        texture.tex_id = 0;
    
    size_dx = (float)size_x;
    size_dy = (float)size_y;

    float t0x = 0.0f;
    float t0y = 0.0f;
    float t1x = 0.5f;
    float t1y = 0.5f;

    if (tex) {
        t0x = tex_x0 / (float)tex -> size_x;
        t0y = tex_y0 / (float)tex -> size_y;
        t1x = (tex_x0 + size_x) / (float)tex -> size_x;
        t1y = (tex_y0 + size_y) / (float)tex -> size_y;
    }

    vvtxs.push_back(VtxPT(-size_dx / 2, size_dy / 2, t0x, t0y));
    vvtxs.push_back(VtxPT(-size_dx / 2, -size_dy / 2, t0x, t1y));
    vvtxs.push_back(VtxPT(size_dx / 2, -size_dy / 2, t1x, t1y));

    vvtxs.push_back(VtxPT(-size_dx / 2, size_dy / 2, t0x, t0y));
    vvtxs.push_back(VtxPT(size_dx / 2, -size_dy / 2, t1x, t1y));
    vvtxs.push_back(VtxPT(size_dx / 2, size_dy / 2, t1x, t0y));
}

void Mesh::createWheel(int rad, u32 rgba, int tris_cnt, const Texture* tex, int tex_x0, int tex_y0) {
    size_dx = (float)rad * 2;
    size_dy = (float)rad * 2;

    if (tex)
        texture = *tex;
    else
        texture.tex_id = 0;
    
    color = rgba;

    float step = 360.0f / tris_cnt;
    VtxPT vtx((float)rad, 0);
    float vc = 0;
    float uc = 0;
    float rad_uv = 1.0f;
    
    if (tex) {
        rad_uv = (float)rad / tex -> size_x;
        uc = (float)tex_x0 / tex -> size_x;
        vc = (float)1.0f - ((float)tex_y0 / tex -> size_y);
        vtx.u = ((float)tex_x0 + rad) / tex -> size_x;
        vtx.v = vc;
    }

    for (int q = 1; q <= tris_cnt; ++q) {
        vvtxs.push_back(VtxPT(0, 0, uc, vc));
        vvtxs.push_back(vtx);

        float ang = step * q * deg2rad;
        vtx.x = cosf(ang) * rad;
        vtx.y = sinf(ang) * rad;

        if (tex) {
            vtx.u = uc + cosf(ang) * rad_uv;
            vtx.v = vc + sinf(ang) * rad_uv;
        }

        vvtxs.push_back(vtx);
    }

    for (size_t q = 0; q < vvtxs.size(); ++q) {
        vvtxs[q].v = 1.0f - vvtxs[q].v;
    }
}

void Mesh::render() {
    glPushMatrix();
    glBindTexture(GL_TEXTURE_2D, texture.tex_id);
    glTranslatef(x, y, 0);
    glRotatef(roto, 0, 0, 1.0f);
    glScalef(sx, sy, 1.0f);

    glColor4ubv((GLubyte*)&color); //getRf(color), getGf(color), getBf(color), getAf(color));
    
    if (texture.tex_id) {
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glTranslatef(tx / texture.size_x, ty / texture.size_y, 0);
    }

    glBegin(GL_TRIANGLES);
    
    for (size_t q = 0; q < vvtxs.size(); ++q) {
        glTexCoord2f(vvtxs[q].u, vvtxs[q].v);
        glVertex2f(vvtxs[q].x, vvtxs[q].y);
    }

    glEnd();

    if (texture.tex_id) {
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glPopMatrix();
}

void drawLine(float x0, float y0, float x1, float y1, int rgba) {
    glBindTexture(GL_TEXTURE_2D, 0);
    glColor4ubv((u8*)&rgba);
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void drawLine(std::vector<b2Vec2>& v, int rgba) {
    glBindTexture(GL_TEXTURE_2D, 0);
    glColor4ubv((u8*)&rgba);
    glBegin(GL_LINES);

    for (int i = 0; i < v.size() - 1; i++) {
        glVertex2f(v[i].x, v[i].y);
        glVertex2f(v[i + 1].x, v[i + 1].y);
    }

    glEnd();
}

void debugDraw(const b2World* world, float scale) {
    u32 dynamic_color = rgba2i(255, 0, 0, 128);
    u32 static_color = rgba2i(0, 0, 255, 128);
    u32 body_color = 0xffffffff;
    Mesh mesh_circle;
    mesh_circle.createWheel(1);

    glPushMatrix();
    glScalef(scale, scale, 1.0f);

    for (const b2Body* body = world -> GetBodyList(); body != NULL; body = body -> GetNext()) {
        /* if(body -> GetType() == b2_dynamicBody)
            body_color = dynamic_color;
        
        else if(body -> GetType() == b2_staticBody)
          body_color = static_color; */
        
        if (body -> GetUserData() != NULL)
            body_color = ((Object*)body -> GetUserData()) -> color;
        
        else
            if (body -> GetType() == b2_dynamicBody)
                body_color = dynamic_color;
            
            else if (body -> GetType() == b2_staticBody)
                body_color = static_color;

        for (const b2Fixture* fixture = body -> GetFixtureList(); fixture != NULL; fixture = fixture -> GetNext()) {
            const b2Shape* shape = fixture -> GetShape();
            glPushMatrix();
            glTranslatef(body -> GetTransform().p.x, body -> GetTransform().p.y, 0);
            float angle = body -> GetTransform().q.GetAngle() * rad2deg;
            glRotatef(angle, 0, 0, 1.0f);
            glColor4f(getRf(body_color), getGf(body_color), getBf(body_color), getAf(body_color));
            
            switch (shape -> GetType()) {
                case b2Shape::e_circle: {
                    const b2CircleShape* shp = (b2CircleShape*)shape;
                    glTranslatef(shp -> m_p.x, shp -> m_p.y, 0);
                    glScalef(shp -> m_radius, shp -> m_radius, 1.0f);
                    mesh_circle.color = body_color;
                    mesh_circle.render();
                    break;
                }

                case b2Shape::e_polygon: {
                    const b2PolygonShape* shp = (b2PolygonShape*)shape;
                    glBegin(GL_TRIANGLE_FAN);
                    glVertex2f(0, 0);

                    for (int v = 0; v < shp -> GetVertexCount(); ++v)
                        glVertex2f(shp -> GetVertex(v).x, shp -> GetVertex(v).y);
                
                    glVertex2f(shp -> GetVertex(0).x, shp -> GetVertex(0).y);
                    glEnd();
                    break;
                }

                case b2Shape::e_edge: {
                    const b2EdgeShape* shp = (b2EdgeShape*)shape;
                    glBegin(GL_LINES);
                    glVertex2f(shp -> m_vertex1.x, shp -> m_vertex1.y);
                    glVertex2f(shp -> m_vertex2.x, shp -> m_vertex2.y);
                    glEnd();
                    break;
                }

                case b2Shape::e_chain: {
                    const b2ChainShape* shp = (b2ChainShape*)shape;
                    glBegin(GL_LINE_STRIP);

                    for (int q = 0; q < shp -> m_count; ++q)
                        glVertex2f(shp -> m_vertices[q].x, shp -> m_vertices[q].y);
                
                    glEnd();
                    break;
                }
            }

            glPopMatrix();
        }
    }

    glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

    for (const b2Joint* joint = world -> GetJointList(); joint != NULL; joint = joint -> GetNext()) {
        glBegin(GL_LINES);
        glVertex2f(joint -> GetAnchorA().x, joint -> GetAnchorA().y);
        glVertex2f(joint -> GetAnchorB().x, joint -> GetAnchorB().y);
        glEnd();
    }

    glPopMatrix();
};
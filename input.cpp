// input.cpp
#include <gl/freeglut.h>
#include <vector>
#include <cmath>
extern float camYaw, camPitch;
extern float bradX, bradY, bradZ;
extern bool jumping;
extern float jumpSpeed;
extern float gravity;
extern float camDist;
extern void toggleRadio();


extern int winW, winH;
extern float radioX, radioY,radioZ;

extern void nextSong();
extern void prevSong();
extern bool dragVolume;

extern bool g_useHeadlight;

bool collidesWithRadio(float, float);
bool isInsideSoundBar(int, int);
void updateVolumeFromMouse(int, int);

void keyNormal(unsigned char key, int, int)
{
    float speed = 0.3f;

    float newX = bradX;
    float newY = bradY;

    if (key == 'w' || key == 'W') newY += speed;
    if (key == 's' || key == 'S') newY -= speed;
    if (key == 'a' || key == 'A') newX -= speed;
    if (key == 'd' || key == 'D') newX += speed;

    if (!collidesWithRadio(newX, newY))
    {
        bradX = newX;
        bradY = newY;
    }

    if (key == ' ' && !jumping)
    {
        jumping = true;
        jumpSpeed = 0.28f;
    }

    if (key == 27) exit(0);
}

void keySpecial(int key, int, int)
{
    if (key == GLUT_KEY_LEFT)  camYaw -= 0.05f;
    if (key == GLUT_KEY_RIGHT) camYaw += 0.05f;
    if (key == GLUT_KEY_UP)    camPitch += 0.03f;
    if (key == GLUT_KEY_DOWN)  camPitch -= 0.03f;

    if (camPitch > 1.3f) camPitch = 1.3f;
    if (camPitch < -1.3f) camPitch = -1.3f;
}
bool clickOnRadioButton(int x, int y, bool& isNext)
{
    GLdouble model[16], proj[16];
    GLint viewport[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble sx = (GLdouble)x;
    GLdouble sy = (GLdouble)(viewport[3] - y);

    GLdouble nx, ny, nz;
    GLdouble fx, fy, fz;
    gluUnProject(sx, sy, 0.0, model, proj, viewport, &nx, &ny, &nz);
    gluUnProject(sx, sy, 1.0, model, proj, viewport, &fx, &fy, &fz);

    double dirX = fx - nx;
    double dirY = fy - ny;
    double dirZ = fz - nz;

    if (fabs(dirY) < 1e-6)
        return false;

    // PLANUL FEȚEI RADIO-ULUI (−Y)
    double yPlane = radioY + 0.76;
    double t = (yPlane - ny) / dirY;
    if (t < 0.0)
        return false;

    double ix = nx + dirX * t;
    double iz = nz + dirZ * t;

    double buttonZ = radioZ + 1.0 + 0.02;  // EXACT ca în scene.cpp

    // PREV ⏮ (stânga)
    if (fabs(ix - (radioX - 0.5)) < 0.35 &&
        fabs(iz - buttonZ) < 0.35)
    {
        isNext = false;
        return true;
    }

    // NEXT ⏭ (dreapta)
    if (fabs(ix - (radioX + 0.5)) < 0.35 &&
        fabs(iz - buttonZ) < 0.35)
    {
        isNext = true;
        return true;
    }

    return false;
}


// ===== helpers: ray -> plane intersection =====
static bool getMouseRayIntersectionPlaneY(
    int x, int y,
    double yPlane,
    double& ix, double& iy, double& iz)
{
    GLdouble model[16], proj[16];
    GLint viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble sx = (GLdouble)x;
    GLdouble sy = (GLdouble)(viewport[3] - y);

    GLdouble nx, ny, nz;
    GLdouble fx, fy, fz;

    gluUnProject(sx, sy, 0.0, model, proj, viewport, &nx, &ny, &nz);
    gluUnProject(sx, sy, 1.0, model, proj, viewport, &fx, &fy, &fz);

    double dirX = fx - nx;
    double dirY = fy - ny;
    double dirZ = fz - nz;

    if (fabs(dirY) < 1e-9) return false;

    double t = (yPlane - ny) / dirY;
    if (t < 0.0) return false;

    ix = nx + dirX * t;
    iy = yPlane;
    iz = nz + dirZ * t;
    return true;
}

static bool getMouseRayIntersectionPlaneX(
    int x, int y,
    double xPlane,
    double& ix, double& iy, double& iz)
{
    GLdouble model[16], proj[16];
    GLint viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble sx = (GLdouble)x;
    GLdouble sy = (GLdouble)(viewport[3] - y);

    GLdouble nx, ny, nz;
    GLdouble fx, fy, fz;

    gluUnProject(sx, sy, 0.0, model, proj, viewport, &nx, &ny, &nz);
    gluUnProject(sx, sy, 1.0, model, proj, viewport, &fx, &fy, &fz);

    double dirX = fx - nx;
    double dirY = fy - ny;
    double dirZ = fz - nz;

    if (fabs(dirX) < 1e-9) return false;

    double t = (xPlane - nx) / dirX;
    if (t < 0.0) return false;

    ix = xPlane;
    iy = ny + dirY * t;
    iz = nz + dirZ * t;
    return true;
}

// ===== hit tests =====
static bool hitPrevNextOnFront(double ix, double iz, bool& isNext)
{
    double buttonZ = radioZ + 1.0 + 0.02; // exact ca in scene.cpp

    // PREV
    if (fabs(ix - (radioX - 0.5)) < 0.35 && fabs(iz - buttonZ) < 0.35)
    {
        isNext = false;
        return true;
    }

    // NEXT
    if (fabs(ix - (radioX + 0.5)) < 0.35 && fabs(iz - buttonZ) < 0.35)
    {
        isNext = true;
        return true;
    }

    return false;
}

static bool hitPowerOnRightSide(double iy, double iz)
{
    // buton rosu e la (radioX + 1.15, radioY, radioZ + 1.0)
    return fabs(iy - radioY) < 0.35 &&
        fabs(iz - (radioZ + 1.0)) < 0.35;
}

// ===== mouse =====
void mouse(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON) return;

    if (state == GLUT_DOWN)
    {
        // UI volum (2D)
        if (isInsideSoundBar(x, y))
        {
            dragVolume = true;
            updateVolumeFromMouse(x, y);
            return;
        }

        // 1) POWER: intersecție cu planul lateral (X = radioX + 1.15)
        {
            double ix, iy, iz;
            double xPlane = radioX + 1.15; // exact unde desenezi torus-ul

            if (getMouseRayIntersectionPlaneX(x, y, xPlane, ix, iy, iz))
            {
                if (hitPowerOnRightSide(iy, iz))
                {
                    toggleRadio();
                    return;
                }
            }
        }

        // 2) PREV/NEXT: intersecție cu planul feței (Y = radioY + 0.76)
        {
            double ix, iy, iz;
            double yPlane = radioY + 0.76;

            if (getMouseRayIntersectionPlaneY(x, y, yPlane, ix, iy, iz))
            {
                bool next;
                if (hitPrevNextOnFront(ix, iz, next))
                {
                    if (next) nextSong();
                    else prevSong();
                    return;
                }
            }
        }
    }

    if (state == GLUT_UP)
    {
        dragVolume = false;
    }
}



void motion(int x, int y)
{
    (void)y;
    if (dragVolume)
    {
        updateVolumeFromMouse(x, y);
    }
}

void reshape(int w, int h)
{
    winW = w;
    winH = h;

    if (h == 0) h = 1;
    float r = float(w) / float(h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, r, 0.1, 300);
    glMatrixMode(GL_MODELVIEW);
}

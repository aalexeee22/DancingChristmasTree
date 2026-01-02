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

    if (key == GLUT_KEY_F1)
    {
        g_useHeadlight = !g_useHeadlight;
        glutPostRedisplay();
    }

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




void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if (isInsideSoundBar(x, y))
        {
            dragVolume = true;
            updateVolumeFromMouse(x, y);
            return;
        }

        bool next;
        if (clickOnRadioButton(x, y, next))
        {
            if (next) nextSong();
            else prevSong();
            return;
        }
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
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

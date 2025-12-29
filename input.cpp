// input.cpp
#include <gl/freeglut.h>
#include <vector>

extern float camYaw, camPitch;
extern float bradX, bradY, bradZ;
extern bool jumping;
extern float jumpSpeed;
extern float gravity;

extern int winW, winH;
extern bool dragVolume;

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

void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            if (isInsideSoundBar(x, y))
            {
                dragVolume = true;
                updateVolumeFromMouse(x, y);
            }
        }
        else if (state == GLUT_UP)
        {
            dragVolume = false;
        }
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

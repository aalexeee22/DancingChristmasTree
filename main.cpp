#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <gl/freeglut.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>

// ===========================================================
// MUSIC SYSTEM
// ===========================================================

int volumeLevel = 10;   // 0..100
bool musicLoaded = false;

void setMusicVolume()
{
    if (!musicLoaded) return;

    int mciVol = volumeLevel * 10; // 0..1000
    std::string cmd = "setaudio music volume to " + std::to_string(mciVol);
    mciSendStringA(cmd.c_str(), NULL, 0, NULL);
}

void playMusic()
{
    std::string path = "assets/sounds/brad.mp3";

    std::ifstream f(path);
    if (!f.good())
    {
        std::cout << "[error] audio file not found: " << path << std::endl;
        return;
    }
    f.close();

    mciSendStringA("close music", NULL, 0, NULL);

    std::string cmdOpen = "open \"" + path + "\" type mpegvideo alias music";
    if (mciSendStringA(cmdOpen.c_str(), NULL, 0, NULL) != 0)
    {
        std::cout << "[error] cannot open audio file" << std::endl;
        return;
    }

    musicLoaded = true;
    setMusicVolume();
    mciSendStringA("play music repeat", NULL, 0, NULL);
}


// ===========================================================
// CAMERA AND MOVEMENT
// ===========================================================

float camYaw = 0.0f;
float camPitch = 0.3f;
float camDist = 35.0f;

float bradX = 0.0f;
float bradY = 0.0f;
float bradZ = 0.0f;

bool jumping = false;
float jumpSpeed = 0.0f;
float gravity = -0.015f;

float danceT = 0.0f;

// ===========================================================
// LIGHTING
// ===========================================================

void setupLight()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    GLfloat pos[] = { 5.0f, -5.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    GLfloat diff[] = { 1, 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

// ===========================================================
// GROUND
// ===========================================================

void drawGround()
{
    glDisable(GL_LIGHTING);
    glColor3f(0.85f, 0.85f, 0.95f);

    glBegin(GL_QUADS);
    glVertex3f(-50, -50, 0);
    glVertex3f(50, -50, 0);
    glVertex3f(50, 50, 0);
    glVertex3f(-50, 50, 0);
    glEnd();

    glEnable(GL_LIGHTING);
}

// ===========================================================
// FACE
// ===========================================================

void drawFace()
{
    glPushMatrix();

    // pozitionarea ochilor
    float baseZ = 7.4f;        // inaltime
    float front = 1.25f;       // cat ies din brad

    float eyeSize = 0.60f;     // ochi mai mari
    float irisSize = 0.38f;    // iris vizibil
    float pupilSize = 0.22f;   // pupila proportionala

    // ===============================
    // OCHI (SCLERA)
    // ===============================
    glColor3f(1, 1, 1);

    glPushMatrix();
    glTranslatef(0.9f, 0.0f, baseZ);
    glTranslatef(0, front, 0);
    glutSolidSphere(eyeSize, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.9f, 0.0f, baseZ);
    glTranslatef(0, front, 0);
    glutSolidSphere(eyeSize, 32, 32);
    glPopMatrix();


    // ===============================
    // IRIS MARE SI VIZIBIL (MARO)
    // ===============================
    glColor3f(0.45f, 0.22f, 0.05f); // maro ca trunchiul

    glPushMatrix();
    glTranslatef(0.9f, 0.0f, baseZ - 0.10f);
    glTranslatef(0, front + 0.35f, 0);    // mult mai in fata ca sa se vada clar
    glutSolidSphere(irisSize, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.9f, 0.0f, baseZ - 0.10f);
    glTranslatef(0, front + 0.35f, 0);
    glutSolidSphere(irisSize, 32, 32);
    glPopMatrix();


    // ===============================
    // PUPILA
    // ===============================
    glColor3f(0, 0, 0);

    glPushMatrix();
    glTranslatef(0.9f, 0.0f, baseZ - 0.12f);
    glTranslatef(0, front + 0.55f, 0);
    glutSolidSphere(pupilSize, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.9f, 0.0f, baseZ - 0.12f);
    glTranslatef(0, front + 0.55f, 0);
    glutSolidSphere(pupilSize, 32, 32);
    glPopMatrix();


    // ===============================
    // REFLEXIE ALBA
    // ===============================
    glColor3f(1, 1, 1);

    glPushMatrix();
    glTranslatef(1.05f, 0.15f, baseZ + 0.05f);
    glTranslatef(0, front + 0.55f, 0);
    glutSolidSphere(0.10f, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.75f, 0.15f, baseZ + 0.05f);
    glTranslatef(0, front + 0.55f, 0);
    glutSolidSphere(0.10f, 20, 20);
    glPopMatrix();


    // ===============================
    // GURA
    // ===============================
    glColor3f(1.0f, 0.22f, 0.22f);

    glPushMatrix();
    glTranslatef(0, -0.6f, baseZ - 0.3f);
    glTranslatef(0, front, 0);
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.08f, 0.45f, 20, 30);
    glPopMatrix();

    glPopMatrix();
}





// ===========================================================
// LIMBS (GOLD TINSEL ARMS)
// ===========================================================

void drawLimbs()
{
    glColor3f(1.0f, 0.8f, 0.0f);

    float t = danceT;
    float L = 3.0f;
    int segments = 12;

    // left arm
    glPushMatrix();
    glTranslatef(-2.0f, 0.0f, 4.5f);
    for (int i = 0; i < segments; i++)
    {
        float u = float(i) / segments;
        float x = u * L;
        float y = sin(t * 3 + u * 6) * 0.4f;
        float z = cos(t * 2 + u * 5) * 0.2f;

        glPushMatrix();
        glTranslatef(-x, y, z);
        glutSolidTorus(0.12f, 0.25f, 12, 20);
        glPopMatrix();
    }
    glPopMatrix();

    // right arm
    glPushMatrix();
    glTranslatef(2.0f, 0.0f, 4.5f);
    for (int i = 0; i < segments; i++)
    {
        float u = float(i) / segments;
        float x = u * L;
        float y = sin(t * 3 + u * 6 + 3.14f) * 0.4f;
        float z = cos(t * 2 + u * 5 + 3.14f) * 0.2f;

        glPushMatrix();
        glTranslatef(x, y, z);
        glutSolidTorus(0.12f, 0.25f, 12, 20);
        glPopMatrix();
    }
    glPopMatrix();
}

// ===========================================================
// TREE
// ===========================================================

void drawTree()
{
    glPushMatrix();

    glTranslatef(bradX, bradY, bradZ);

    glColor3f(0.4f, 0.25f, 0.1f);
    glPushMatrix(); glTranslatef(0, 0, 1); glutSolidCylinder(0.7f, 2, 20, 20); glPopMatrix();

    glColor3f(0, 0.7f, 0);
    glPushMatrix(); glTranslatef(0, 0, 3); glutSolidCone(3.5f, 3, 30, 30); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0, 5); glutSolidCone(2.7f, 3, 30, 30); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0, 7); glutSolidCone(2.0f, 3, 30, 30); glPopMatrix();

    drawFace();
    drawLimbs();

    glPopMatrix();
}

// ===========================================================
// UI: SOUND BAR IN DREAPTA SUS
// ===========================================================
// window size (pentru sound bar)
int winW = 900;
int winH = 700;
bool dragVolume = false;

// parametrii barei in pixeli
const int barWidth = 160;
const int barHeight = 18;

void drawSoundBar()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // trecem in modul 2d
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int margin = 20;
    int x0 = winW - barWidth - margin;
    int y0 = winH - barHeight - margin;
    int x1 = x0 + barWidth;
    int y1 = y0 + barHeight;

    // bara de fundal
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2i(x0, y0);
    glVertex2i(x1, y0);
    glVertex2i(x1, y1);
    glVertex2i(x0, y1);
    glEnd();

    // volum curent (verde inchis)
    float frac = volumeLevel / 100.0f;
    int xFill = x0 + int(barWidth * frac);

    glColor3f(0.0f, 0.4f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2i(x0, y0);
    glVertex2i(xFill, y0);
    glVertex2i(xFill, y1);
    glVertex2i(x0, y1);
    glEnd();

    // contur
    glColor3f(0, 0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x0, y0);
    glVertex2i(x1, y0);
    glVertex2i(x1, y1);
    glVertex2i(x0, y1);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

bool isInsideSoundBar(int mx, int my)
{
    int margin = 20;
    int x0 = winW - barWidth - margin;
    int y0 = winH - barHeight - margin;
    int x1 = x0 + barWidth;
    int y1 = y0 + barHeight;

    // in glut, originea pentru mouse este sus-stanga
    int yGL = my;

    return (mx >= x0 && mx <= x1 && (winH - yGL) >= y0 && (winH - yGL) <= y1);
}

void updateVolumeFromMouse(int mx, int my)
{
    int margin = 20;
    int x0 = winW - barWidth - margin;
    int x1 = x0 + barWidth;

    if (mx < x0) mx = x0;
    if (mx > x1) mx = x1;

    float frac = float(mx - x0) / float(barWidth);
    volumeLevel = int(frac * 100.0f);
    if (volumeLevel < 0) volumeLevel = 0;
    if (volumeLevel > 100) volumeLevel = 100;

    setMusicVolume();
}

// ===========================================================
// DISPLAY
// ===========================================================

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float camX = camDist * cos(camPitch) * cos(camYaw);
    float camY = camDist * cos(camPitch) * sin(camYaw);
    float camZ = camDist * sin(camPitch);

    gluLookAt(camX, camY, camZ,
        0, 0, 3,
        0, 0, 1);

    drawGround();
    drawTree();
    drawSoundBar();

    glutSwapBuffers();
}

// ===========================================================
// IDLE
// ===========================================================

void idle()
{
    danceT += 0.005f;

    if (jumping)
    {
        bradZ += jumpSpeed;
        jumpSpeed += gravity;

        if (bradZ <= 0)
        {
            bradZ = 0;
            jumping = false;
        }
    }

    glutPostRedisplay();
}

// ===========================================================
// KEY NORMAL
// ===========================================================

void keyNormal(unsigned char key, int, int)
{
    float speed = 0.3f;

    if (key == 'w' || key == 'W') bradY += speed;
    if (key == 's' || key == 'S') bradY -= speed;
    if (key == 'a' || key == 'A') bradX -= speed;
    if (key == 'd' || key == 'D') bradX += speed;

    if (key == ' ' && !jumping)
    {
        jumping = true;
        jumpSpeed = 0.28f;
    }

    if (key == 27) exit(0);
}

// ===========================================================
// KEY SPECIAL
// ===========================================================

void keySpecial(int key, int, int)
{
    if (key == GLUT_KEY_LEFT)  camYaw -= 0.05f;
    if (key == GLUT_KEY_RIGHT) camYaw += 0.05f;
    if (key == GLUT_KEY_UP)    camPitch += 0.03f;
    if (key == GLUT_KEY_DOWN)  camPitch -= 0.03f;

    if (camPitch > 1.3f) camPitch = 1.3f;
    if (camPitch < -1.3f) camPitch = -1.3f;
}
// ===========================================================
// MOUSE (SOUND BAR CONTROL)
// ===========================================================

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
    if (dragVolume)
    {
        updateVolumeFromMouse(x, y);
    }
}

// ===========================================================
// RESHAPE
// ===========================================================

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    float r = float(w) / float(h);

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, r, 0.1, 300);
    glMatrixMode(GL_MODELVIEW);
}

// ===========================================================
// MAIN
// ===========================================================

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("brad dansator 3d");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.6f, 0.9f, 0.95f, 1);

    setupLight();
    playMusic();

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyNormal);
    glutSpecialFunc(keySpecial);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}

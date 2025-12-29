// main.cpp
#include <algorithm>
#include <gl/freeglut.h>
#include <cmath>
#include <string>
#include <cstdlib>

struct MusicNote
{
    float x, y, z;
    float phase;
    bool active;
};

extern MusicNote musicNote;
extern int noteCooldown;




extern int winW, winH;

extern float camYaw, camPitch, camDist;
extern float bradX, bradY, bradZ;

extern bool jumping;
extern float jumpSpeed, gravity;

extern float danceT;

extern int volumeLevel;
extern bool musicLoaded;

extern float radioX, radioY;
extern bool dragVolume;

void setupLight();
void playMusic();
void setMusicVolume();
void setMusicVolumeAttenuated(float dist);

void drawGround();
void drawRadio();
void drawTree();

void drawMusicNotes();
void drawSoundBar();

void keyNormal(unsigned char, int, int);
void keySpecial(int, int, int);
void mouse(int, int, int, int);
void motion(int, int);
void reshape(int, int);

void spawnMusicNote()
{
    if (musicNote.active) return;

    musicNote.x = radioX;
    musicNote.y = radioY;
    musicNote.z = 1.5f;

    musicNote.phase = 0.0f;
    musicNote.active = true;
}



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
    drawRadio();
    drawTree();
    drawMusicNotes();
    drawSoundBar();

    glutSwapBuffers();
}

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

    // volum cu distanță
    if (musicLoaded)
    {
        float dx = bradX - radioX;
        float dy = bradY - radioY;
        float dist = sqrt(dx * dx + dy * dy);
        setMusicVolumeAttenuated(dist);
    }

    // spawn notă la interval
    if (!musicNote.active)
    {
        noteCooldown++;
        if (noteCooldown > 60)   // ≈ 1 notă / secundă
        {
            spawnMusicNote();
            noteCooldown = 0;
        }
    }
    else
    {
        // mișcare LENTĂ în sus
        musicNote.z += 0.01f;

        // balans fin (aer)
        musicNote.phase += 0.03f;
        musicNote.x = radioX + sin(musicNote.phase) * 0.15f;

        // dispare sus
        if (musicNote.z > 9.0f)
        {
            musicNote.active = false;
        }
    }

    glutPostRedisplay();
}



int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("brad dansator 3d + radio");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.6f, 0.9f, 0.95f, 1);

    setupLight();
    playMusic();

    glutReshapeFunc(reshape);
    reshape(winW, winH); // asigură proiecția de la start (safe)

    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyNormal);
    glutSpecialFunc(keySpecial);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);

    glutMainLoop();
    return 0;
}

// main.cpp
#include <algorithm>
#include <gl/freeglut.h>
#include <cmath>
#include <string>
#include <cstdlib>
#include <ctime>

struct MusicNote
{
    float x, y, z;
    float baseX;
    float phase;
    bool active;
    int type;
};

extern float SKY_BG_RGBA[4];
extern int burstCount;
extern int burstRemaining;
extern MusicNote musicNote;
extern int noteCooldown;
extern void nextSong();
extern void prevSong();



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
extern bool radioOn;

void setupLight();
void applyLightPosition();
extern bool g_useHeadlight;   // folosit doar pentru if-uri (demo)

void playMusic();
void setMusicVolume();
void setMusicVolumeAttenuated(float dist);

void initGroundTexture();
void drawGroundTextured();
void drawGround();
void drawRadio();
void drawTree();
void drawSun();
void drawMilkySkyBackdrop();

void drawMusicNotes();
void drawSoundBar();

// ninsoare
void initSnow();
void setSnowCamera(float camX, float camY, float camZ);
void updateSnow(float dt);
void drawSnow();

void keyNormal(unsigned char, int, int);
void keySpecial(int, int, int);
void mouse(int, int, int, int);
void motion(int, int);
void reshape(int, int);

void spawnMusicNote()
{
    if (musicNote.active) return;

    // offset mic diferit pentru fiecare notă
    float offset = ((rand() % 100) / 100.0f - 0.5f) * 0.6f;

    musicNote.x = radioX + offset;
    musicNote.baseX = musicNote.x;
    musicNote.y = radioY;
    musicNote.z = 1.5f;

    musicNote.phase = (rand() % 100) / 100.0f * 6.28f;
    musicNote.active = true;

    int r = rand() % 100;
    if (r < 60)       musicNote.type = 0;
    else if (r < 85)  musicNote.type = 1;
    else              musicNote.type = 2;
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMilkySkyBackdrop(); // cer laptos (2D), nu afecteaza scena 3D
    glLoadIdentity();

    // Dacă e headlight, setăm poziția luminii ACUM (înainte de view)
    if (g_useHeadlight)
        applyLightPosition();

    // pozitia camerei pe sfera
    float camX = camDist * cos(camPitch) * cos(camYaw);
    float camY = camDist * cos(camPitch) * sin(camYaw);
    float camZ = camDist * sin(camPitch);

    gluLookAt(camX, camY, camZ,
        0, 0, 3,
        0, 0, 1);

    // Dacă e world sun, setăm poziția luminii DUPĂ view (fixă în lume)
    if (!g_useHeadlight)
        applyLightPosition();

    drawSun();

    // scena 3D principala (brad + radio)
    //drawGround();
    drawGroundTextured();
    drawRadio();
    drawTree();
    drawMusicNotes();
    
    // desenare ninsoare
    setSnowCamera(camX, camY, camZ);
    drawSnow();

    // bara de volum se deseaza la final, in coordonate de ecran, peste tot restul
    drawSoundBar();


    glutSwapBuffers();
}


void idle()
{
    static int lastMS = glutGet(GLUT_ELAPSED_TIME);
    int nowMS = glutGet(GLUT_ELAPSED_TIME);
    float dt = (nowMS - lastMS) / 1000.0f;
    lastMS = nowMS;

    // ninsoare
    updateSnow(dt);

    // animatie dans
    danceT += dt + 0.005f;

    // saritura
    if (jumping)
    {
        bradZ += jumpSpeed;
        jumpSpeed += gravity;
        if (bradZ <= 0.0f)
        {
            bradZ = 0.0f;
            jumping = false;
        }
    }

    // =========================
    // AUDIO – DOAR DACĂ RADIO E ON
    // =========================
    if (musicLoaded && radioOn)
    {
        float dx = bradX - radioX;
        float dy = bradY - radioY;
        float dist = sqrt(dx * dx + dy * dy);
        setMusicVolumeAttenuated(dist);
    }

    // =========================
    // NOTE MUZICALE
    // =========================
    if (!radioOn)
    {
        // radio OFF → fara note
        musicNote.active = false;
    }
    else
    {
        // spawn notă
        if (!musicNote.active)
        {
            noteCooldown++;

            if (noteCooldown > 40)
            {
                if (burstRemaining == 0)
                {
                    burstRemaining = 2 + rand() % 2; // 2–3 note
                }

                spawnMusicNote();
                burstRemaining--;
                noteCooldown = 0;
            }
        }
        else
        {
            // miscare notă
            musicNote.z += 0.01f;

            musicNote.phase += 0.03f;
            musicNote.x =
                musicNote.baseX +
                sin(musicNote.phase) * 0.12f;

            if (musicNote.z > 9.0f)
            {
                musicNote.active = false;
            }
        }
    }

    glutPostRedisplay();
}




int main(int argc, char** argv)
{
    srand((unsigned)time(nullptr));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Brad 3D dansator");

    glEnable(GL_DEPTH_TEST);
    glClearColor(SKY_BG_RGBA[0], SKY_BG_RGBA[1], SKY_BG_RGBA[2], SKY_BG_RGBA[3]);
    
    // lumina pentru intreaga scena (brad, radio, fulgi)
    setupLight();

    // podea texturata
    initGroundTexture();

    // muzica de fundal
    playMusic();

    // se porneste ninsoarea si timer-ul
    initSnow();

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

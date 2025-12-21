#include <GL/freeglut.h>
#include <stdlib.h>
#include <math.h> 
#include <time.h>

#define M_PI 3.1415926f
#define NUM_FLAKES 15

const float FLAKE_DEPTH = 0.03f;   // grosimea fulgului (pe axa Z)
GLuint g_snowflakeList = 0;

// animație fulg
float g_time = 0.0f;
float g_snowY = 1.5f;   // poziție verticală (sus de tot)
float g_snowX = 0.0f;   // poziție orizontală
float g_rotZ = 0.0f;   // rotire în jurul axei proprii 
float g_tiltX = 0.0f;   // balans pe X
float g_tiltY = 0.0f;   // balans pe Y;

const float FALL_SPEED = 0.30f;  // viteză de cădere (unități / secundă)
const float SMALL = 0.05f;
const float BIG = 0.45f;
// zona „utilă” în care vrem să vedem fulgii
const float SNOW_AREA_X = 3.0f;   // răspândire orizontală vizibilă
const float SNOW_AREA_Y = 3.0f;   // răspândire verticală vizibilă

// margini puțin în afara ecranului, pentru spawn/despawn
const float SNOW_SPAWN_Y = 3.8f;   // reapariție deasupra
const float SNOW_DESPAWN_Y = -3.8f;  // când cade sub atât, îl resetăm

typedef struct Snowflake {
    float x;          // poziția curentă X
    float y;          // poziția curentă Y
    float baseX;      // „centrul” oscilației pe X
    float scale;      // scalare (mărimea fulgului)
    float rotZ;       // unghi curent pe Z
    float fallSpeed;  // viteza de cădere
    float rotSpeed;   // viteza de rotire pe Z
    float swayAmp;    // amplitudinea legănării pe X
    float swaySpeed;  // viteza de legănare
    float swayPhase;  // fază pentru sin
} Snowflake;

Snowflake g_flakes[NUM_FLAKES];

float frand(float a, float b)
{
    return a + (b - a) * (rand() / (float)RAND_MAX);
}

void drawStem3D(float y0, float y1, float halfWidth, float depth)
{
    float x0 = -halfWidth, x1 = halfWidth;
    float z0 = -depth * 0.5f, z1 = depth * 0.5f;

    glBegin(GL_QUADS);
    // fața din față (z1) – normal (0,0,1)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(x0, y0, z1);
    glVertex3f(x1, y0, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x0, y1, z1);

    // fața din spate (z0) – normal (0,0,-1)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y1, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y0, z0);

    // lateral stânga – normal (-1,0,0)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(x0, y0, z0);
    glVertex3f(x0, y0, z1);
    glVertex3f(x0, y1, z1);
    glVertex3f(x0, y1, z0);

    // lateral dreapta – normal (1,0,0)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y1, z0);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y0, z1);

    // jos – normal (0,-1,0)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(x0, y0, z0);
    glVertex3f(x1, y0, z0);
    glVertex3f(x1, y0, z1);
    glVertex3f(x0, y0, z1);

    // sus – normal (0,1,0)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x0, y1, z0);
    glVertex3f(x0, y1, z1);
    glVertex3f(x1, y1, z1);
    glVertex3f(x1, y1, z0);
    glEnd();
}

void drawBranch(float yCenter, float len, float halfWidth)
{
    // ramură înclinată în stânga
    glPushMatrix();
    glTranslatef(0.0f, yCenter, 0.0f);
    glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
    drawStem3D(0.0f, len, halfWidth, FLAKE_DEPTH);
    glPopMatrix();

    // ramură înclinată în dreapta
    glPushMatrix();
    glScalef(-1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, yCenter, 0.0f);
    glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
    drawStem3D(0.0f, len, halfWidth, FLAKE_DEPTH);
    glPopMatrix();
}

void drawArm()
{
    float stemHalfWidth = 0.035f;
    float branchHalfWidth = 0.035f;

    // trunchiul principal al brațului
    drawStem3D(0.0f, 0.65f, stemHalfWidth, FLAKE_DEPTH);

    // rand de brate laterale
    drawBranch(0.35f, 0.3f, branchHalfWidth);

    // vârful brațului (un romb)
    // vârful brațului – un romb extrudat
    float vx[4] = { 0.0f, -0.20f, 0.0f,  0.20f };
    float vy[4] = { 0.80f, 0.65f, 0.50f, 0.65f };
    float z0 = -FLAKE_DEPTH * 0.5f;
    float z1 = FLAKE_DEPTH * 0.5f;

    glBegin(GL_QUADS);
    // fața din față (rombul)
    glVertex3f(vx[0], vy[0], z1);
    glVertex3f(vx[1], vy[1], z1);
    glVertex3f(vx[2], vy[2], z1);
    glVertex3f(vx[3], vy[3], z1);

    // fața din spate
    glVertex3f(vx[0], vy[0], z0);
    glVertex3f(vx[3], vy[3], z0);
    glVertex3f(vx[2], vy[2], z0);
    glVertex3f(vx[1], vy[1], z0);

    // laterale
    for (int i = 0; i < 4; ++i) {
        int j = (i + 1) % 4;
        glVertex3f(vx[i], vy[i], z0);
        glVertex3f(vx[i], vy[i], z1);
        glVertex3f(vx[j], vy[j], z1);
        glVertex3f(vx[j], vy[j], z0);
    }
    glEnd();
}

void drawCenterStar3D()
{
    const int spikes = 6;
    const int N = 2 * spikes;
    const float outerR = 0.25f;
    const float innerR = 0.12f;
    float z0 = -FLAKE_DEPTH * 0.5f;
    float z1 = FLAKE_DEPTH * 0.5f;

    float vx[N], vy[N];

    for (int i = 0; i < N; ++i) {
        float angle = (float)i * (float)M_PI / (float)spikes; // 0..2π
        float r = (i % 2 == 0) ? outerR : innerR;
        vx[i] = r * cosf(angle);
        vy[i] = r * sinf(angle);
    }

    // fața din față
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, z1);
    for (int i = 0; i < N; ++i)
        glVertex3f(vx[i], vy[i], z1);
    glVertex3f(vx[0], vy[0], z1);
    glEnd();

    // fața din spate
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, z0);
    glVertex3f(vx[0], vy[0], z0);
    for (int i = N - 1; i >= 0; --i)
        glVertex3f(vx[i], vy[i], z0);
    glEnd();

    // lateralele
    glBegin(GL_QUADS);
    for (int i = 0; i < N; ++i) {
        int j = (i + 1) % N;
        glVertex3f(vx[i], vy[i], z0);
        glVertex3f(vx[i], vy[i], z1);
        glVertex3f(vx[j], vy[j], z1);
        glVertex3f(vx[j], vy[j], z0);
    }
    glEnd();
}

void drawSnowflake()
{
    glPushMatrix();
    // fulg format din 6 brațe rotite cu 60°
    for (int i = 0; i < 6; ++i) {
        glPushMatrix();
        glRotatef(i * 60.0f, 0.0f, 0.0f, 1.0f);
        drawArm();
        glPopMatrix();
    }

    drawCenterStar3D();

    glPopMatrix();
}

void initSnowflakes()
{
    srand((unsigned int)time(NULL));

    // display list cu geometria unui fulg
    g_snowflakeList = glGenLists(1);
    glNewList(g_snowflakeList, GL_COMPILE);
    drawSnowflake();
    glEndList();

    // initializare parametri pentru fiecare fulg
    for (int i = 0; i < NUM_FLAKES; ++i) {
        g_flakes[i].y = frand(-SNOW_AREA_Y, SNOW_AREA_Y);   // răspândiți pe verticală
        g_flakes[i].baseX = frand(-SNOW_AREA_X, SNOW_AREA_X);   // poziție de bază pe X
        g_flakes[i].x = g_flakes[i].baseX;

        g_flakes[i].scale = frand(SMALL, BIG);    // unii mai mici, unii mai mari

        g_flakes[i].rotZ = frand(0.0f, 360.0f);
        g_flakes[i].fallSpeed = frand(0.15f, 0.40f);  // unii cad mai repede
        g_flakes[i].rotSpeed = frand(10.0f, 35.0f);  // viteză rotire

        g_flakes[i].swayAmp = frand(0.15f, 0.45f);  // cât se leagănă
        g_flakes[i].swaySpeed = frand(0.7f, 1.5f);    // ritmul legănării
        g_flakes[i].swayPhase = frand(0.0f, 6.28f);   // fază inițială (0..2π)
    }

}

void initLighting()
{
    GLfloat lightPos[] = { 1.5f, 3.0f, 4.0f, 1.0f };
    GLfloat lightAmb[] = { 0.5f, 0.8f, 1.0f, 1.0f }; // 0.5, 0.8, 1.0
    GLfloat lightDiff[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat lightSpec[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    // material pentru fulg
    GLfloat matDiff[] = { 0.75f, 0.85f, 1.0f, 1.0f };  // albstru-deschis
    GLfloat matSpec[] = { 0.9f, 0.9f, 1.0f, 1.0f };  // reflexe puternice
    GLfloat shininess = 64.0f;

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

    //glDisable(GL_COLOR_MATERIAL);   

    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

}

void timer(int value)
{
    const float dt = 0.016f;   // ~60 cadre / sec
    for (int i = 0; i < NUM_FLAKES; ++i) {
        Snowflake& f = g_flakes[i];   // referință la elementul din vector

        // cădere
        f.y -= f.fallSpeed * dt;
        if (f.y < SNOW_DESPAWN_Y) {
            f.y = SNOW_SPAWN_Y;
            f.baseX = frand(-SNOW_AREA_X, SNOW_AREA_X);
            f.scale = frand(SMALL, BIG);
        }

        // legănare
        f.swayPhase += f.swaySpeed * dt;
        f.x = f.baseX + f.swayAmp * sinf(f.swayPhase);

        // rotire
        f.rotZ += f.rotSpeed * dt;
        if (f.rotZ > 360.0f) f.rotZ -= 360.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // camera un pic mai departe
    glTranslatef(0.0f, 0.0f, -6.0f);

    // desenăm fiecare fulg
    for (int i = 0; i < NUM_FLAKES; ++i) {
        Snowflake& f = g_flakes[i];

        glPushMatrix();
        glTranslatef(f.x, f.y, 0.0f);
        glRotatef(f.rotZ, 0.0f, 0.0f, 1.0f);
        glScalef(f.scale, f.scale, f.scale);

        glCallList(g_snowflakeList);  // fulgul extrudat, cu iluminare
        glPopMatrix();
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    if (h == 0) h = 1;
    float aspect = (float)w / (float)h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // proiecție în perspectivă
    gluPerspective(60.0f, aspect, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Dans fulgi de zapada");

    // fundal inchis la culoare
    glClearColor(0.5f, 0.6f, 0.7f, 1.0f);

    glEnable(GL_DEPTH_TEST);   // avem nevoie de buffer de adâncime în 3D

    initLighting();
    initSnowflakes();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}

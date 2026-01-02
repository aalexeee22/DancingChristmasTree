// Implementeaza modulul de ninsoare pentru scena principala 3D (efect infinit de ninsoare):
// - fulgii sunt pastrati in coordonate relative camerei (intr-o cutie in jurul ei)
// - la randare, pozitia finala = pozCamera + pozRelativa


#include <GL/freeglut.h>
#include <stdlib.h>
#include <math.h> 
#include <time.h>

static const int   SNOW_NUM_FLAKES = 110;

// cutia in jurul camerei:
// - X/Y: lateral (stanga-dreapta / fata-spate)
// - Z: vertical (sus-jos)
static const float SNOW_HALF_XY = 22.0f;    // raza pe X/Y in jurul camerei
static const float SNOW_TOP_Z = 18.0f;      // fulgii apar la +TOP deasupra camerei
static const float SNOW_BOTTOM_Z = -6.0f;   // sub acest prag (fata de camera) fulgii sunt redesenati sus

// dimensiuni fulgi 
static const float SNOW_SCALE_MIN = 0.05f;
static const float SNOW_SCALE_MAX = 0.40f;

// grosimea fulgilor
static const float SNOW_FLAKE_DEPTH = 0.012f;

// display list cu geometria fulgului
static GLuint g_snowflakeDL = 0;

// pozitia curenta a camerei (actualizata din scena principala)
static float g_camX = 0.0f, g_camY = 0.0f, g_camZ = 0.0f;


static float frand(float a, float b)
{
    return a + (b - a) * (rand() / (float)RAND_MAX);
}

struct Snowflake
{
    // pozitia relativa fata de camera
    float x, y, z;

    // efect de leganare – pastram un centru si oscilam pe X
    float baseX;
    float swayAmp;
    float swaySpeed;
    float swayPhase;

    // cadere + rotire
    float fallSpeed;
    float rotZ;
    float rotSpeed;

    // tilt mic (ca sa fie vizibil din orice unghi)
    float tiltX, tiltY;
    float tiltSpeedX, tiltSpeedY;

    // scalare
    float scale;
};

static Snowflake g_flakes[SNOW_NUM_FLAKES];

// geometria fulgului
static void drawStem3D(float y0, float y1, float halfWidth, float depth)
{
    float x0 = -halfWidth, x1 = halfWidth;
    float z0 = -depth * 0.5f, z1 = depth * 0.5f;

    glBegin(GL_QUADS);

    // fata (z+)
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(x0, y0, z1); 
    glVertex3f(x1, y0, z1); 
    glVertex3f(x1, y1, z1); 
    glVertex3f(x0, y1, z1);

    // spate (z-)
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(x0, y0, z0); 
    glVertex3f(x0, y1, z0); 
    glVertex3f(x1, y1, z0); 
    glVertex3f(x1, y0, z0);

    // stanga (x-)
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glVertex3f(x0, y0, z0); 
    glVertex3f(x0, y0, z1); 
    glVertex3f(x0, y1, z1); 
    glVertex3f(x0, y1, z0);

    // dreapta (x+)
    glNormal3f(1.0f, 0.0f, 0.0f);
    glVertex3f(x1, y0, z0); 
    glVertex3f(x1, y1, z0); 
    glVertex3f(x1, y1, z1); 
    glVertex3f(x1, y0, z1);

    // jos (y-)
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(x0, y0, z0); 
    glVertex3f(x1, y0, z0); 
    glVertex3f(x1, y0, z1); 
    glVertex3f(x0, y0, z1);

    // sus (y+)
    glNormal3f(0.0f, 1.0f, 0.0f);
    glVertex3f(x0, y1, z0); 
    glVertex3f(x0, y1, z1); 
    glVertex3f(x1, y1, z1); 
    glVertex3f(x1, y1, z0);

    glEnd();
}

static void drawBranch(float yCenter, float len, float halfWidth)
{
    // ramura stanga
    glPushMatrix();
    glTranslatef(0.0f, yCenter, 0.0f);
    glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
    drawStem3D(0.0f, len, halfWidth, SNOW_FLAKE_DEPTH);
    glPopMatrix();

    // ramura dreapta (mirror pe X)
    glPushMatrix();
    glScalef(-1.0f, 1.0f, 1.0f);
    glTranslatef(0.0f, yCenter, 0.0f);
    glRotatef(60.0f, 0.0f, 0.0f, 1.0f);
    drawStem3D(0.0f, len, halfWidth, SNOW_FLAKE_DEPTH);
    glPopMatrix();
}

static void drawArm()
{
    float stemHalfWidth = 0.035f;
    float branchHalfWidth = 0.035f;

    drawStem3D(0.0f, 0.65f, stemHalfWidth, SNOW_FLAKE_DEPTH);
    drawBranch(0.35f, 0.3f, branchHalfWidth);

    // varf (romb extrudat)
    float vx[4] = { 0.0f, -0.20f, 0.0f,  0.20f };
    float vy[4] = { 0.80f, 0.65f, 0.50f, 0.65f };
    float z0 = -SNOW_FLAKE_DEPTH * 0.5f;
    float z1 = SNOW_FLAKE_DEPTH * 0.5f;

    glBegin(GL_QUADS);

    // fata
    glVertex3f(vx[0], vy[0], z1);
    glVertex3f(vx[1], vy[1], z1);
    glVertex3f(vx[2], vy[2], z1);
    glVertex3f(vx[3], vy[3], z1);

    // spate
    glVertex3f(vx[0], vy[0], z0);
    glVertex3f(vx[3], vy[3], z0);
    glVertex3f(vx[2], vy[2], z0);
    glVertex3f(vx[1], vy[1], z0);

    // laterale
    for (int i = 0; i < 4; ++i)
    {
        int j = (i + 1) % 4;
        glVertex3f(vx[i], vy[i], z0);
        glVertex3f(vx[i], vy[i], z1);
        glVertex3f(vx[j], vy[j], z1);
        glVertex3f(vx[j], vy[j], z0);
    }

    glEnd();
}

static void drawCenterStar3D()
{
    const int   spikes = 6;
    const int   N = 2 * spikes;
    const float PI = 3.1415926f;

    const float outerR = 0.25f;
    const float innerR = 0.12f;

    float z0 = -SNOW_FLAKE_DEPTH * 0.5f;
    float z1 = SNOW_FLAKE_DEPTH * 0.5f;

    float vx[N], vy[N];
    for (int i = 0; i < N; ++i)
    {
        float angle = (float)i * PI / (float)spikes;
        float r = (i % 2 == 0) ? outerR : innerR;
        vx[i] = r * (float)cos(angle);
        vy[i] = r * (float)sin(angle);
    }

    // fata
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, z1);
    for (int i = 0; i < N; ++i) glVertex3f(vx[i], vy[i], z1);
    glVertex3f(vx[0], vy[0], z1);
    glEnd();

    // spate
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, z0);
    glVertex3f(vx[0], vy[0], z0);
    for (int i = N - 1; i >= 0; --i) glVertex3f(vx[i], vy[i], z0);
    glEnd();

    // laterale
    glBegin(GL_QUADS);
    for (int i = 0; i < N; ++i)
    {
        int j = (i + 1) % N;
        glVertex3f(vx[i], vy[i], z0);
        glVertex3f(vx[i], vy[i], z1);
        glVertex3f(vx[j], vy[j], z1);
        glVertex3f(vx[j], vy[j], z0);
    }
    glEnd();
}

static void drawSnowflake()
{
    glPushMatrix();

    for (int i = 0; i < 6; ++i)
    {
        glPushMatrix();
        glRotatef(i * 60.0f, 0.0f, 0.0f, 1.0f);
        drawArm();
        glPopMatrix();
    }

    drawCenterStar3D();

    glPopMatrix();
}

void initSnow()
{
    // nu se repeta initializarea fulgilor daca s-a facut deja
    if (g_snowflakeDL != 0) return;

    srand((unsigned int)time(NULL));

    // display list cu geometria fulgului (se compileaza o singura data)
    g_snowflakeDL = glGenLists(1);
    glNewList(g_snowflakeDL, GL_COMPILE);
    drawSnowflake();
    glEndList();

    // initializare parametri pentru fiecare fulg (pozitii random + viteze diferite)
    for (int i = 0; i < SNOW_NUM_FLAKES; ++i)
    {
        Snowflake& f = g_flakes[i];

        f.x = frand(-SNOW_HALF_XY, SNOW_HALF_XY);
        f.y = frand(-SNOW_HALF_XY, SNOW_HALF_XY);
        f.z = frand(SNOW_BOTTOM_Z, SNOW_TOP_Z);

        f.baseX = f.x;

        f.scale = frand(SNOW_SCALE_MIN, SNOW_SCALE_MAX);

        f.fallSpeed = frand(1.2f, 2.8f);

        f.swayAmp = frand(0.2f, 0.8f);
        f.swaySpeed = frand(0.7f, 1.5f);
        f.swayPhase = frand(0.0f, 6.28f);

        f.rotZ = frand(0.0f, 360.0f);
        f.rotSpeed = frand(10.0f, 35.0f);

        f.tiltX = frand(-25.0f, 25.0f);
        f.tiltY = frand(-25.0f, 25.0f);
        f.tiltSpeedX = frand(-15.0f, 15.0f);
        f.tiltSpeedY = frand(-15.0f, 15.0f);
    }
}

// setare pozitie camera
void setSnowCamera(float camX, float camY, float camZ)
{
    g_camX = camX;
    g_camY = camY;
    g_camZ = camZ;
}

// actualizare pozitii fulgi
void updateSnow(float dt)
{
    if (g_snowflakeDL == 0) return;

    if (dt > 0.05f) dt = 0.05f;
    if (dt < 0.0f)  dt = 0.0f;

    for (int i = 0; i < SNOW_NUM_FLAKES; ++i)
    {
        Snowflake& f = g_flakes[i];

        // cadere pe Z (verticala scenei)
        f.z -= f.fallSpeed * dt;

        // daca a trecut sub limita de jos (relativ camerei), il facem sa reapara de sus
        if (f.z < SNOW_BOTTOM_Z)
        {
            f.z = SNOW_TOP_Z;

            // repozitionare aleatorie in planul XY (tot relativ camerei)
            f.y = frand(-SNOW_HALF_XY, SNOW_HALF_XY);
            f.baseX = frand(-SNOW_HALF_XY, SNOW_HALF_XY);
            f.scale = frand(SNOW_SCALE_MIN, SNOW_SCALE_MAX);

            f.fallSpeed = frand(1.2f, 2.8f);
            f.swayAmp = frand(0.2f, 0.8f);
        }

        // leganare - osciltie pe X in jurul lui baseX
        f.swayPhase += f.swaySpeed * dt;
        f.x = f.baseX + f.swayAmp * (float)sin(f.swayPhase);

        // rotire usoara in jurul Z
        f.rotZ += f.rotSpeed * dt;
        if (f.rotZ > 360.0f) f.rotZ -= 360.0f;

        // tilt lent pe XY
        f.tiltX += f.tiltSpeedX * dt;
        f.tiltY += f.tiltSpeedY * dt;
    }
}

// desenare ninsoare
void drawSnow()
{
    if (g_snowflakeDL == 0) return;

    // salvare stare, ca ninsoarea sa nu afecteze restul scenei
    glPushAttrib(GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glDisable(GL_COLOR_MATERIAL);

    // normalizare utila pentru scalare
    glEnable(GL_NORMALIZE);

    // boost de lumina doar pentru ninsoare
    GLfloat lightAmb[] = { 0.40f, 0.53f, 0.65f, 1.00f }; // mai mult ambient => fulgi mai luminosi
    GLfloat lightSpec[] = { 0.60f, 0.60f, 0.60f, 1.00f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpec);

    const GLfloat alpha = 0.75f;

    GLfloat matAmbient[] = { 0.87f, 0.87f, 0.9f, alpha };  // valori mai mari => mai putin intunecat in umbra
    GLfloat matDiffuse[] = { 0.85f, 0.85f, 1.00f, alpha };  // alb-albastrui
    GLfloat matSpecular[] = { 0.75f, 0.80f, 0.85f, 1.00f }; // reflexe fulgi
    GLfloat matEmission[] = { 0.10f, 0.10f, 0.12f, 1.00f }; // glow discret, rece

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, matEmission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 20.0f);

    // desenam fiecare fulg: coordonate finale = camera + coordonate relative
    for (int i = 0; i < SNOW_NUM_FLAKES; ++i)
    {
        const Snowflake& f = g_flakes[i];

        glPushMatrix();

        glTranslatef(g_camX + f.x, g_camY + f.y, g_camZ + f.z);

        // tilt (X/Y) ca sa nu fie mereu perfect plan
        glRotatef(f.tiltX, 1.0f, 0.0f, 0.0f);
        glRotatef(f.tiltY, 0.0f, 1.0f, 0.0f);

        glRotatef(f.rotZ, 0.0f, 0.0f, 1.0f);

        glScalef(f.scale, f.scale, f.scale);

        glCallList(g_snowflakeDL);

        glPopMatrix();
    }

    // revenire la starea initiala
    glPopAttrib();
}

// scene.cpp
#include <gl/freeglut.h>
#include <cmath>

extern float camYaw, camPitch, camDist;

extern float bradX, bradY, bradZ;
extern float danceT;

extern float radioX, radioY, radioZ;
extern float radioRadius, bradRadius;

struct MusicNote
{
    float x, y, z;
    float phase;
    bool active;
};

extern MusicNote musicNote;
extern int noteCooldown;



bool collidesWithRadio(float newX, float newY)
{
    float dx = newX - radioX;
    float dy = newY - radioY;
    float dist = std::sqrt(dx * dx + dy * dy);
    return dist < (radioRadius + bradRadius);
}

void setupLight()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // OPTIONAL dar safe: ajută când scalezi obiecte (radio-ul)
    glEnable(GL_NORMALIZE);

    GLfloat pos[] = { 5.0f, -5.0f, 10.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    GLfloat diff[] = { 1, 1, 1, 1 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);

    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

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

void drawRadio()
{
    glPushMatrix();
    glTranslatef(radioX, radioY, radioZ + 1.0f);

    glColor3f(0.18f, 0.18f, 0.18f);
    glScalef(3.0f, 1.5f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(radioX + 1.15f, radioY, radioZ + 1.0f);
    glColor3f(0.05f, 0.05f, 0.05f);
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.08f, 0.35f, 16, 24);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(radioX - 1.2f, radioY + 0.55f, radioZ + 1.2f);
    glColor3f(0.75f, 0.75f, 0.75f);
    glutSolidSphere(0.20f, 20, 20);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_LINES);
    glVertex3f(radioX - 1.2f, radioY - 0.2f, radioZ + 2.0f);
    glVertex3f(radioX - 2.2f, radioY - 0.7f, radioZ + 4.2f);
    glEnd();
    glEnable(GL_LIGHTING);
}

static void drawFace()
{
    glPushMatrix();

    float baseZ = 7.4f;
    float front = 1.25f;

    float eyeSize = 0.60f;
    float irisSize = 0.38f;
    float pupilSize = 0.22f;

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

    glColor3f(0.45f, 0.22f, 0.05f);
    glPushMatrix();
    glTranslatef(0.9f, 0.0f, baseZ - 0.10f);
    glTranslatef(0, front + 0.35f, 0);
    glutSolidSphere(irisSize, 32, 32);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.9f, 0.0f, baseZ - 0.10f);
    glTranslatef(0, front + 0.35f, 0);
    glutSolidSphere(irisSize, 32, 32);
    glPopMatrix();

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

    glColor3f(1.0f, 0.22f, 0.22f);
    glPushMatrix();
    glTranslatef(0, -0.6f, baseZ - 0.3f);
    glTranslatef(0, front, 0);
    glRotatef(90, 1, 0, 0);
    glutSolidTorus(0.08f, 0.45f, 20, 30);
    glPopMatrix();

    glPopMatrix();
}

static void drawLimbs()
{
    glColor3f(1.0f, 0.8f, 0.0f);

    float t = danceT;
    float L = 3.0f;
    int segments = 12;

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

void drawMusicNotes()
{
    if (!musicNote.active) return;

    glPushMatrix();
    glTranslatef(musicNote.x, musicNote.y, musicNote.z);

    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f);

    // cap notă
    glutSolidSphere(0.18f, 16, 16);

    // coadă
    glBegin(GL_LINES);
    glVertex3f(0.15f, 0.0f, 0.0f);
    glVertex3f(0.15f, 0.0f, 0.8f);
    glEnd();

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

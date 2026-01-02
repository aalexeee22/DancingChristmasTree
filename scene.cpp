#include <gl/freeglut.h>
#include <cmath>
#include "SOIL.h" 


extern float camYaw, camPitch, camDist;
extern float SKY_BG_RGBA[4];
extern float bradX, bradY, bradZ;
extern float danceT;

extern float radioX, radioY, radioZ;
extern float radioRadius, bradRadius;

bool g_useHeadlight = true;
static GLuint g_groundTex = 0;

struct MusicNote
{
    float x, y, z;
    float baseX;
    float phase;
    bool active;
    int type;   // 0 = doime ♩, 1 = optime ♪, 2 = saisprezecime ♬
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

    GLfloat pos[] = { 5.0f, 5.0f, 10.0f, 1.0f };
    //GLfloat pos[] = { 1.5f, 3.0f, 4.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    GLfloat diff[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);

    GLfloat amb[] = { 0.25f, 0.28f, 0.33f, 1.0f };
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);

    GLfloat spec[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec);


    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void applyLightPosition()
{
    // IMPORTANT:
    // - dacă vrem headlight: apelăm funcția înainte de gluLookAt (modelview = identitate)
    // - dacă vrem world sun: apelăm funcția după gluLookAt (modelview = view)
    if (g_useHeadlight)
    {
        // Lumină direcțională dinspre cameră (w=0) – “lanternă pe cameră”
        GLfloat dir[] = { 0.0f, 0.0f, 1.0f, 0.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, dir);
    }
    else
    {
        // Lumină punctuală fixă în lume (w=1) – “soare”/sursă în scenă
        // Păstrează aici aceeași poziție pe care o folosiți ca "soare".
        GLfloat pos[] = { 5.0f, 5.0f, 10.0f, 1.0f };
        glLightfv(GL_LIGHT0, GL_POSITION, pos);
    }
}

void drawSun()
{
    // soarele e doar un element vizual, nu se doreste sa influenteze restul scenei
    // salvam starea scenei in momentul curent
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_FOG_BIT);

    glDisable(GL_LIGHTING);                 // soarele emite lumina, nu e iluminat de scena 3D
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDisable(GL_DEPTH_TEST);               // sa nu fie acoperit de obiecte
    glDepthMask(GL_FALSE);                  // sa nu scrie in depth buffer

    // ceata doar peste soare (halo)
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);
    glFogfv(GL_FOG_COLOR, SKY_BG_RGBA);
    glFogf(GL_FOG_DENSITY, 0.016f);

    glPushMatrix();

    if (g_useHeadlight)
        glTranslatef(-20.0f, -15.0f, 18.0f);
    else
        glTranslatef(18.0f, -12.0f, 22.0f);

    // creare efect de halo
    // nucleul (mai luminos)
    glColor4f(0.98f, 0.98f, 0.92f, 0.55f);
    glutSolidSphere(1.8, 18, 18);

    // halo 1 (mai mare, mai transparent)
    glColor4f(0.90f, 0.95f, 1.00f, 0.18f);
    glutSolidSphere(3.8, 18, 18);

    // halo 2 (si mai mare)
    glColor4f(0.85f, 0.92f, 1.00f, 0.10f);
    glutSolidSphere(6.5, 18, 18);

    glPopMatrix();

    // revenim la depth buffer normal pentru restul scenei
    glDepthMask(GL_TRUE);

    glPopAttrib();

}

void drawMilkySkyBackdrop()
{
    // gradient 2D peste fundal: cer laptos fara sa afecteze scena 3D
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glBegin(GL_QUADS);

    // sus: mai deschis
    glColor3f(0.65f, 0.72f, 0.80f);
    glVertex2f(0.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);

    // jos: culoarea de fundal
    glColor3f(SKY_BG_RGBA[0], SKY_BG_RGBA[1], SKY_BG_RGBA[2]);
    glVertex2f(1.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);

    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDepthMask(GL_TRUE);
    glPopAttrib();
}

void initGroundTexture()
{
    if (g_groundTex != 0) return; // deja încărcată

    glGenTextures(1, &g_groundTex);
    glBindTexture(GL_TEXTURE_2D, g_groundTex);

    // repetare (tiling) pe ambele direcții
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // filtrare (arată mai bine decât NEAREST pe sol mare)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width = 0, height = 0, channels = 0;

    // alege imaginea
    const char* path = "assets/pattern/realistic-snow.jpg";

    // incarca imaginea
    unsigned char* img = SOIL_load_image(path, &width, &height, &channels, SOIL_LOAD_RGBA);
    if (!img)
    {
        // fallback: nu blocăm scena dacă lipsește fișierul
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

    SOIL_free_image_data(img);

    glBindTexture(GL_TEXTURE_2D, 0);
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

void drawGroundTextured()
{
    // Dacă textura nu e disponibilă, folosește solul vechi (fallback)
    if (g_groundTex == 0)
    {
        drawGround();
        return;
    }

    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);

    // Ca să fie în ton cu scena, păstrăm fără iluminare, ca înainte.
    glDisable(GL_LIGHTING);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_groundTex);

    // scadem contrastul pentru a nu iesi umbrele din imagine/pattern in evidenta
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(0.92f, 0.93f, 0.96f);

    // cat de des se repeta textura 
    const float tile = 1.0f;


    glBegin(GL_QUADS);

    glNormal3f(0.0f, 0.0f, 1.0f);

    glTexCoord2f(0.0f, 0.0f);      glVertex3f(-50.0f, -50.0f, 0.0f);
    glTexCoord2f(tile, 0.0f);      glVertex3f(50.0f, -50.0f, 0.0f);
    glTexCoord2f(tile, tile);      glVertex3f(50.0f, 50.0f, 0.0f);
    glTexCoord2f(0.0f, tile);      glVertex3f(-50.0f, 50.0f, 0.0f);

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glPopAttrib();
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
    glColor3f(1, 0, 0);
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

    // =====================
// BUTOANE RADIO – PE FAȚA REALĂ
// =====================
    glDisable(GL_LIGHTING);

    float frontY = radioY + 0.76f;
    float buttonZ = radioZ + 1.0f + 0.02f; // scoase PUȚIN în afară

    // PREV ⏮
    glPushMatrix();
    glTranslatef(radioX - 0.5f, frontY, buttonZ);
    glColor3f(0.9f, 0.2f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.0f, 0.25f);
    glVertex3f(-0.3f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -0.25f);
    glEnd();
    glPopMatrix();

    // NEXT ⏭
    glPushMatrix();
    glTranslatef(radioX + 0.5f, frontY, buttonZ);
    glColor3f(0.2f, 0.9f, 0.2f);
    glBegin(GL_TRIANGLES);
    glVertex3f(0.0f, 0.0f, 0.25f);
    glVertex3f(0.3f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, -0.25f);
    glEnd();
    glPopMatrix();

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

    // tija
    glBegin(GL_LINES);
    glVertex3f(0.15f, 0.0f, 0.0f);
    glVertex3f(0.15f, 0.0f, 0.9f);
    glEnd();

    // OPTIME ♪ → un steguleț
    if (musicNote.type >= 1)
    {
        glBegin(GL_TRIANGLES);
        glVertex3f(0.15f, 0.0f, 0.9f);
        glVertex3f(0.6f, 0.0f, 0.7f);
        glVertex3f(0.15f, 0.0f, 0.7f);
        glEnd();
    }

    // SAISPREZECIME ♬ → două stegulețe
    if (musicNote.type >= 2)
    {
        glBegin(GL_TRIANGLES);
        glVertex3f(0.15f, 0.0f, 0.7f);
        glVertex3f(0.6f, 0.0f, 0.5f);
        glVertex3f(0.15f, 0.0f, 0.5f);
        glEnd();
    }

    glEnable(GL_LIGHTING);
    glPopMatrix();
}

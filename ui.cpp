// ui.cpp
#include <gl/freeglut.h>

extern int winW, winH;
extern int volumeLevel;
extern bool dragVolume;
extern const int barWidth;
extern const int barHeight;

void setMusicVolume();

void drawSoundBar()
{
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

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

    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2i(x0, y0);
    glVertex2i(x1, y0);
    glVertex2i(x1, y1);
    glVertex2i(x0, y1);
    glEnd();

    float frac = volumeLevel / 100.0f;
    int xFill = x0 + int(barWidth * frac);

    glColor3f(0.0f, 0.4f, 0.0f);
    glBegin(GL_QUADS);
    glVertex2i(x0, y0);
    glVertex2i(xFill, y0);
    glVertex2i(xFill, y1);
    glVertex2i(x0, y1);
    glEnd();

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

    int yGL = my;
    return (mx >= x0 && mx <= x1 && (winH - yGL) >= y0 && (winH - yGL) <= y1);
}

void updateVolumeFromMouse(int mx, int /*my*/)
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

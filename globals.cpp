// Contine toate constantele globale

// culoare fundal – RGBA
float SKY_BG_RGBA[4] = { 0.45f, 0.56f, 0.68f, 1.0f };

int volumeLevel = 10;   // 0..100
bool musicLoaded = false;

// camera
float camYaw = 0.0f;
float camPitch = 0.3f;
float camDist = 35.0f;

// brad
float bradX = 0.0f;
float bradY = 0.0f;
float bradZ = 0.0f;

bool jumping = false;
float jumpSpeed = 0.0f;
float gravity = -0.008f;

float danceT = 0.0f;

// radio
float radioX = 6.0f;
float radioY = 0.0f;
float radioZ = 0.0f;

float radioRadius = 2.2f;
float bradRadius = 2.0f;

// UI
int winW = 900;
int winH = 700;
bool dragVolume = false;

// IMPORTANT: const globale între fișiere => definire cu external linkage
extern const int barWidth = 160;
extern const int barHeight = 18;

struct MusicNote
{
    float x, y, z;
    float baseX;
    float phase;
    bool active;
    int type;
};


MusicNote musicNote;
int noteCooldown = 0;

int burstCount = 0;
int burstRemaining = 0;


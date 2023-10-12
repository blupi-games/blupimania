
/* ========== */
/* bm_smaky.c */
/* ========== */

#include <errno.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>

#include "actions.h"
#include "argtable3/argtable3.h"
#include "blupimania.h"
#include "sdl/SDL3_IMG.h"
#include <SDL2/SDL_mixer.h>

/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Pixmap pmicon1c = {0}; /* pixmap des icônes1 (chair) */
static Pixmap pmicon2c = {0}; /* pixmap des icônes2 (chair) */
static Pixmap pmicon3c = {0}; /* pixmap des icônes3 (chair) */
static Pixmap pmicon4c = {0}; /* pixmap des icônes4 (chair) */
static Pt     origine;        /* coin sup/gauche de l'origine */

static unsigned int nextrand[10]; /* valeurs aléatoires suivantes */

static short     soundon = 1; /* son on/off */
static int       filsson = 0; /* son à entendre */
static KeyStatus keystatus;   /* etat des flèches du clavier */

static Pixmap pmsave = {0}; /* pixmap sauvé en mémoire étendue (XMS) */

static short       g_soundVolume       = 0;
static short       g_musicVolume       = 0;
static Mix_Chunk * g_sounds[SOUND_MAX] = {NULL};
static Mix_Music * g_music             = NULL;
static SDL_bool    g_musicStopped      = SDL_FALSE;

static const SDL_Color g_colorsDOS[] = {
  {255, 255, 255, SDL_ALPHA_OPAQUE}, // BLANC
  {255, 255, 0, SDL_ALPHA_OPAQUE},   // JAUNE
  {255, 204, 64, SDL_ALPHA_OPAQUE},  // ORANGE
  {255, 0, 0, SDL_ALPHA_OPAQUE},     // ROUGE
  {220, 220, 220, SDL_ALPHA_OPAQUE}, // GRIS CLAIR
  {190, 190, 190, SDL_ALPHA_OPAQUE}, // GRIS FONCE
  {0, 255, 255, SDL_ALPHA_OPAQUE},   // CYAN
  {0, 0, 255, SDL_ALPHA_OPAQUE},     // BLEU
  {0, 255, 0, SDL_ALPHA_OPAQUE},     // VERT CLAIR
  {0, 205, 0, SDL_ALPHA_OPAQUE},     // VERT FONCE
  {224, 161, 255, SDL_ALPHA_OPAQUE}, // VIOLET
  {255, 0, 255, SDL_ALPHA_OPAQUE},   // MAGENTA
  {224, 164, 164, SDL_ALPHA_OPAQUE}, // BRUN CLAIR
  {187, 0, 0, SDL_ALPHA_OPAQUE},     // BRUN FONCE
  {169, 216, 255, SDL_ALPHA_OPAQUE}, // BLEU MOYEN
  {0, 0, 0, SDL_ALPHA_OPAQUE},       // NOIR
};

static const SDL_Color g_colorsSmaky100[] = {
  {0x00, 0xED, 0x18, SDL_ALPHA_OPAQUE}, // (BLANC) GREEN (OLD SMAKY 100)
  {255, 255, 0, SDL_ALPHA_OPAQUE},      // JAUNE
  {255, 204, 64, SDL_ALPHA_OPAQUE},     // ORANGE
  {0, 0, 0, SDL_ALPHA_OPAQUE},          // (ROUGE) BLACK
  {220, 220, 220, SDL_ALPHA_OPAQUE},    // GRIS CLAIR
  {190, 190, 190, SDL_ALPHA_OPAQUE},    // GRIS FONCE
  {0, 255, 255, SDL_ALPHA_OPAQUE},      // CYAN
  {0, 0, 255, SDL_ALPHA_OPAQUE},        // BLEU
  {0x00, 0xED, 0x18, SDL_ALPHA_OPAQUE}, // (VERT CLAIR) GREEN (OLD SMAKY 100)
  {0, 205, 0, SDL_ALPHA_OPAQUE},        // VERT FONCE
  {224, 161, 255, SDL_ALPHA_OPAQUE},    // VIOLET
  {255, 0, 255, SDL_ALPHA_OPAQUE},      // MAGENTA
  {224, 164, 164, SDL_ALPHA_OPAQUE},    // BRUN CLAIR
  {187, 0, 0, SDL_ALPHA_OPAQUE},        // BRUN FONCE
  {169, 216, 255, SDL_ALPHA_OPAQUE},    // BLEU MOYEN
  {0, 0, 0, SDL_ALPHA_OPAQUE},          // NOIR
};

/* --------------------------- */
/* Variables globales externes */
/* --------------------------- */

const SDL_Color * g_colors;
const SDL_Color * g_colorsTheme[2] = {g_colorsDOS, g_colorsSmaky100};

void
GetThemeRGB (int color, Uint8 * r, Uint8 * g, Uint8 * b, Uint8 * a)
{
  /* Special hack for the dark style where the white is not white */
  if (color == COLORBLANC && g_theme == 0 && GetWorldStyle () == DARK)
  {
    *r = 0xD0;
    *g = 0xD0;
    *b = 0xD0;
    *a = g_colors[color].a;
    return;
  }

  *r = g_colors[color].r;
  *g = g_colors[color].g;
  *b = g_colors[color].b;
  *a = g_colors[color].a;
}

/* ========= */
/* GetRandom */
/* ========= */

/*
    Retourne une valeur aléatoire comprise entre min et max-1.
    min <= h < max
 */

short
GetRandom (short g, short min, short max)
{
  nextrand[g] = nextrand[g] * 1103515245 + 12345;
  return (short) min + (nextrand[g] / 65536) % (max - min);
}

/* ============ */
/* InitRandomEx */
/* ============ */

/*
    Initialise un tirage exclusif.
 */

void
InitRandomEx (short g, short min, short max, char * pex)
{
  short i;

  for (i = 0; i < (max - min); i++)
    pex[i] = 0; /* met tout le tableau à zéro */
}

/* =========== */
/* GetRandomEx */
/* =========== */

/*
    Retourne une valeur aléatoire exclusive.
 */

short
GetRandomEx (short g, short min, short max, char * pex)
{
  short i, val;

  val = GetRandom (g, 0, max - min); /* cherche une valeur quelconque */

  for (i = 0; i < (max - min); i++)
  {
    if (pex[val] == 0) /* valeur déjà sortie ? */
    {
      pex[val] = 1; /* indique valeur utilisée */
      return min + val;
    }
    else
    {
      val++;
      if (val == max - min)
        val = 0;
    }
  }

  InitRandomEx (g, min, max, pex);        /* recommence */
  val      = GetRandom (g, 0, max - min); /* cherche une valeur quelconque */
  pex[val] = 1;                           /* indique valeur utilisée */
  return min + val;
}

/* =========== */
/* StartRandom */
/* =========== */

/*
    Coup de sac du générateur aléatoire.
        mode = 0	->	toujours la même chose
        mode = 1	->	aléatoire 100%
 */

void
StartRandom (short g, short mode)
{
  /* grand nombre premier ou l'heure */
  nextrand[g] = mode == 0 ? 33554393 : time (NULL);
}

/* ========== */
/* MusicStart */
/* ========== */

/*
    Démarre une musique de fond donnée (song).
    song = 0	->		musique pendant générique initial
    song = 1	->		musique si terminé un niveau
    song = 2	->		musique si terminé une énigme
    song = 3	->		musique pendant réglages
    song = 4	->		musique pendant jeu (choix aléatoire)
 */

void
MusicStart (short song)
{
  static int prev = -1;

  if (song < 3)
    return;

  static const char * musics[] = {
    "bmx000.ogg", "bmx001.ogg", "bmx002.ogg", "bmx003.ogg",
    "bmx004.ogg", "bmx005.ogg", "bmx006.ogg", "bmx007.ogg",
    "bmx008.ogg", "bmx009.ogg", "bmx010.ogg", "bmx011.ogg",
  };

  int idx = 0;

  if (song != 3)
  {
    while ((idx = GetRandom (1, 0, countof (musics))) == prev)
      ;
    prev = idx;
  }

  char filename[4096];
  snprintf (
    filename, sizeof (filename), "%s../share/blupimania/music/%s",
    SDL_GetBasePath (), musics[idx]);

  if (g_music)
    Mix_FreeMusic (g_music);

  g_music = Mix_LoadMUS (filename);
  if (!g_music)
  {
    printf ("%s\n", Mix_GetError ());
    return;
  }

  Mix_VolumeMusic (g_musicVolume);

  if (Mix_PlayMusic (g_music, 0) == -1)
  {
    printf ("%s\n", Mix_GetError ());
    return;
  }

  g_musicStopped = SDL_FALSE;
}

/* ========= */
/* MusicStop */
/* ========= */

/*
    Stoppe la musique de fond.
 */

void
MusicStop (void)
{
  for (int i = SOUND_MUSIC11; i < SOUND_MAX; ++i)
    Mix_HaltChannel (i);

  Mix_HaltMusic ();
  g_musicStopped = SDL_TRUE;
}

void
MusicPause (void)
{
  Mix_PauseMusic ();

  for (int i = SOUND_MUSIC11; i < SOUND_MAX; ++i)
    Mix_Pause (i);
}

void
MusicResume (void)
{
  Mix_ResumeMusic ();

  for (int i = SOUND_MUSIC11; i < SOUND_MAX; ++i)
    Mix_Resume (i);
}

SDL_bool
MusicStoppedOnDemand (void)
{
  return g_musicStopped;
}

/* =============== */
/* PlayNoiseVolume */
/* =============== */

/*
    Détermine le volume des bruitages (0..10).
 */

void
PlayNoiseVolume (short volume)
{
  soundon       = volume == 0 ? 0 : 1;
  g_soundVolume = volume * 10;
}

/* =============== */
/* PlayMusicVolume */
/* =============== */

/*
    Détermine le volume de la musique de fond (0..10).
 */

void
PlayMusicVolume (short volume)
{
  g_musicVolume = volume * 10;
  Mix_VolumeMusic (g_musicVolume);
}

/* ========= */
/* PlaySound */
/* ========= */

/*
    Fait entendre un bruit quelconque.
 */

void
PlayAudio (short sound, const Pt * cel)
{
  if (soundon == 0)
    return;
  filsson = sound; /* donne le numéro au processus fils */

  if (sound < 1 || sound >= SOUND_MAX)
    return;

  if (!g_sounds[sound])
    return;

  if (SoundPlaying (sound))
    return;

  Sint32 volume = g_soundVolume;

  if (cel)
  {
    Pt pos = CelToGra2 (*cel, SDL_TRUE);
    pos.x += LXICO / 2;
    pos.y += LYICO / 2;

    Sint32 volumex, volumey;
    Uint8  panRight, panLeft;
    volumex = g_soundVolume;
    volumey = g_soundVolume;

    if (pos.x < 0)
    {
      panRight = 64;
      panLeft  = 255;
      volumex += pos.x;
      if (volumex < 0)
        volumex = 25;
    }
    else if (pos.x > DIMXDRAW)
    {
      panRight = 255;
      panLeft  = 64;
      volumex -= pos.x - DIMXDRAW;
      if (volumex < 0)
        volumex = 25;
    }
    else
    {
      panRight = 255 * (Uint16) (pos.x) / DIMXDRAW;
      panLeft  = 255 - panRight;
    }

    if (pos.y < 0)
    {
      volumey += pos.y;
      if (volumey < 0)
        volumey = 25;
    }
    else if (pos.y > DIMYDRAW)
    {
      volumey -= pos.y - DIMYDRAW;
      if (volumey < 0)
        volumey = 25;
    }

    volume = volumex < volumey ? volumex : volumey;

    Mix_SetPanning (sound, panLeft, panRight);
  }

  Mix_Volume (sound, volume);
  Mix_PlayChannel (sound, g_sounds[sound], 0);
}

SDL_bool
SoundPlaying (short sound)
{
  return Mix_Playing (sound);
}

/* ========= */
/* ClrEvents */
/* ========= */

/*
    Vide le fifo du clavier.
 */

void
ClrEvents (void)
{
  g_clearKeyEvents = SDL_TRUE;
}

int
SDLEventToSmakyKey (const SDL_Event * event)
{
  int key = 0;

  switch (event->type)
  {
  case SDL_TEXTINPUT:
    if (!strncmp (event->text.text, "á", sizeof ("á")))
      key = KEYAAIGU;
    else if (!strncmp (event->text.text, "à", sizeof ("à")))
      key = KEYAGRAVE;
    else if (!strncmp (event->text.text, "â", sizeof ("â")))
      key = KEYACIRCON;
    else if (!strncmp (event->text.text, "ä", sizeof ("ä")))
      key = KEYATREMA;
    else if (!strncmp (event->text.text, "é", sizeof ("é")))
      key = KEYEAIGU;
    else if (!strncmp (event->text.text, "è", sizeof ("è")))
      key = KEYEGRAVE;
    else if (!strncmp (event->text.text, "ê", sizeof ("ê")))
      key = KEYECIRCON;
    else if (!strncmp (event->text.text, "ë", sizeof ("ë")))
      key = KEYETREMA;
    else if (!strncmp (event->text.text, "í", sizeof ("í")))
      key = KEYIAIGU;
    else if (!strncmp (event->text.text, "ì", sizeof ("ì")))
      key = KEYIGRAVE;
    else if (!strncmp (event->text.text, "î", sizeof ("î")))
      key = KEYICIRCON;
    else if (!strncmp (event->text.text, "ï", sizeof ("ï")))
      key = KEYITREMA;
    else if (!strncmp (event->text.text, "ó", sizeof ("ó")))
      key = KEYOAIGU;
    else if (!strncmp (event->text.text, "ò", sizeof ("ò")))
      key = KEYOGRAVE;
    else if (!strncmp (event->text.text, "ô", sizeof ("ô")))
      key = KEYOCIRCON;
    else if (!strncmp (event->text.text, "ö", sizeof ("ö")))
      key = KEYOTREMA;
    else if (!strncmp (event->text.text, "ú", sizeof ("ú")))
      key = KEYUAIGU;
    else if (!strncmp (event->text.text, "ù", sizeof ("ù")))
      key = KEYUGRAVE;
    else if (!strncmp (event->text.text, "û", sizeof ("û")))
      key = KEYUCIRCON;
    else if (!strncmp (event->text.text, "ü", sizeof ("ü")))
      key = KEYUTREMA;
    else if (!strncmp (event->text.text, "ç", sizeof ("ç")))
      key = KEYCCEDILLE;

    if (key)
      break;

    if (
      (event->text.text[0] >= SDLK_a && event->text.text[0] <= SDLK_z) ||
      (event->text.text[0] >= SDLK_0 && event->text.text[0] <= SDLK_9) ||
      (event->text.text[0] >= 65 && event->text.text[0] <= 90)) /* A..Z */
    {
      key = event->text.text[0];
      break;
    }

    switch (event->text.text[0])
    {
    case SDLK_SPACE:
    case SDLK_EXCLAIM:
    case SDLK_QUOTEDBL:
    case SDLK_HASH:
    case SDLK_PERCENT:
    case SDLK_DOLLAR:
    case SDLK_AMPERSAND:
    case SDLK_QUOTE:
    case SDLK_LEFTPAREN:
    case SDLK_RIGHTPAREN:
    case SDLK_ASTERISK:
    case SDLK_PLUS:
    case SDLK_COMMA:
    case SDLK_MINUS:
    case SDLK_PERIOD:
    case SDLK_SLASH:
    case SDLK_COLON:
    case SDLK_SEMICOLON:
    case SDLK_LESS:
    case SDLK_EQUALS:
    case SDLK_GREATER:
    case SDLK_QUESTION:
    case SDLK_AT:
    case SDLK_LEFTBRACKET:
    case SDLK_BACKSLASH:
    case SDLK_RIGHTBRACKET:
    case SDLK_CARET:
    case SDLK_UNDERSCORE:
    case SDLK_BACKQUOTE:
      key = event->text.text[0];
      break;
    }
    break;

  case SDL_KEYDOWN:
    switch (event->key.keysym.sym)
    {
    case SDLK_ESCAPE:
      key = KEYQUIT;
      break;
    case SDLK_HOME:
      key = KEYHOME;
      break;
    case SDLK_BACKSPACE:
      key = KEYDEL;
      break;
    case SDLK_RETURN:
      key = KEYRETURN;
      break;
    case SDLK_PAUSE:
    case SDLK_KP_PERIOD:
      key = KEYPAUSE;
      break;
    case SDLK_F1:
      key = KEYF1;
      break;
    case SDLK_F2:
      key = KEYF2;
      break;
    case SDLK_F3:
      key = KEYF3;
      break;
    case SDLK_F4:
      key = KEYF4;
      break;
    case SDLK_F10:
      key = KEYSAVE;
      break;
    case SDLK_F11:
      key = KEYLOAD;
      break;
    case SDLK_F12:
      key = KEYIO;
      break;
    case SDLK_KP_7:
      if (g_pause || g_typejeu != 1 || g_modetelecom == 1)
        break;
    case SDLK_LEFT:
    case SDLK_KP_4:
      key = KEYLEFT;
      break;
    case SDLK_KP_3:
      if (g_pause || g_typejeu != 1 || g_modetelecom == 1)
        break;
    case SDLK_RIGHT:
    case SDLK_KP_6:
      key = KEYRIGHT;
      break;
    case SDLK_KP_9:
      if (g_pause || g_typejeu != 1 || g_modetelecom == 1)
        break;
    case SDLK_UP:
    case SDLK_KP_8:
      key = KEYUP;
      break;
    case SDLK_KP_1:
      if (g_pause || g_typejeu != 1 || g_modetelecom == 1)
        break;
    case SDLK_DOWN:
    case SDLK_KP_2:
      key = KEYDOWN;
      break;
    case SDLK_SPACE:
      key = KEYCENTER;
      break;
    case SDLK_F5:
      key = KEYF5;
      break;
    case SDLK_F6:
      key = KEYF6;
      break;
    case SDLK_F7:
      key = KEYF7;
      break;
    case SDLK_F8:
      key = KEYF8;
      break;
    case SDLK_F9:
      key = KEYF9;
      break;
    case SDLK_PAGEDOWN:
      PushUserEvent (RESET, NULL);
    default:
      key = 0;
    }
    break;
  case SDL_KEYUP:
    switch (event->key.keysym.sym)
    {
    case SDLK_LEFT:
    case SDLK_KP_4:
      keystatus &= ~STLEFT;
      break;
    case SDLK_RIGHT:
    case SDLK_KP_6:
      keystatus &= ~STRIGHT;
      break;
    case SDLK_UP:
    case SDLK_KP_8:
      keystatus &= ~STUP;
      break;
    case SDLK_DOWN:
    case SDLK_KP_2:
      keystatus &= ~STDOWN;
      break;
    }
    break;
  case SDL_MOUSEBUTTONDOWN:
  case SDL_MOUSEBUTTONUP: {
    SDL_MouseButtonEvent * _event = (SDL_MouseButtonEvent *) event;
    if (_event->state == SDL_PRESSED)
    {
      if (_event->button == SDL_BUTTON_LEFT)
        key = KEYCLIC;
      else if (_event->button == SDL_BUTTON_RIGHT)
        key = KEYCLICR;
      g_keyMousePos.x   = _event->x;
      g_keyMousePos.y   = _event->y;
      g_keyMousePressed = SDL_TRUE;
    }
    else if (_event->state == SDL_RELEASED)
    {
      key               = KEYCLICREL;
      g_keyMousePressed = SDL_FALSE;
      if (
        abs (g_keyMousePos.x - g_lastmouse.x) > 40 ||
        abs (g_keyMousePos.y - g_lastmouse.y) > 20)
        g_keyMousePos = g_lastmouse;
    }
    break;
  }
  default:
    key = 0;
  }

  switch (key)
  {
  case KEYLEFT:
    keystatus = STLEFT;
    break;
  case KEYRIGHT:
    keystatus = STRIGHT;
    break;
  case KEYUP:
    keystatus = STUP;
    break;
  case KEYDOWN:
    keystatus = STDOWN;
    break;
  }

  return key;
}

/* ============ */
/* GetKeyStatus */
/* ============ */

/*
    Retourne l'etat du clavier, c'est-à-dire l'enfoncement
    éventuel des touches flèches.
 */

KeyStatus
GetKeyStatus (void)
{
  return keystatus;
}

/* ======= */
/* IfColor */
/* ======= */

/*
    Indique si la machine dispose d'un écran couleur.
    Retourne != 0 si oui (vrai).
 */

short
IfColor (void)
{
  return g_theme == 0; /* couleur */
}

/* ========= */
/* DuplPixel */
/* ========= */

/*
    Duplique entièrement un pixmap dans un autre.
 */

void
DuplPixel (Pixmap * ppms, Pixmap * ppmd)
{
  Pt p, dim;

  p.y   = 0;
  p.x   = 0;
  dim.y = ppmd->dy;
  dim.x = ppmd->dx;
  CopyPixel (ppms, p, ppmd, p, dim);
}

/* =============== */
/* ScrollPixelRect */
/* =============== */

/*
    Décale un pixmap dans l'une des quatre directions.
        *ppm		*pixmap source et destination
        od			origine destination
        dim			dimensions
        shift.x		décalage horizontal (<0 vers la droite, >0 vers la gauche)
        shift.y		décalage vertical (<0 vers le bas, >0 vers le haut)
        color		couleur assignée à la nouvelle zone (0..15)
                    -1 laisse la zone originale
        *pzone		retourne la zone à redessiner après ScrollPixel
 */

void
ScrollPixelRect (
  Pixmap * ppm, Pt pos, Pt dim, Pt shift, char color, Rect * pzone)
{
  Pt p1, p2, _dim;

  (*pzone).p1.x = pos.x;
  (*pzone).p1.y = pos.y;
  (*pzone).p2.x = pos.x + dim.x;
  (*pzone).p2.y = pos.y + dim.y;

  if (shift.x == 0 && shift.y == 0)
    goto fill;
  if (shift.x != 0 && shift.y != 0)
    goto fill;

  Pixmap tmp = {0};
  GetPixmap (&tmp, dim, 1, 0);
  DuplPixel (ppm, &tmp);

  if (shift.x < 0 && shift.x > -dim.x)
  {
    p1.y   = pos.y;
    p1.x   = pos.x;
    p2.y   = pos.y;
    p2.x   = pos.x - shift.x;
    _dim.y = dim.y;
    _dim.x = dim.x + shift.x;
    CopyPixel (ppm, p1, &tmp, p2, _dim);
    (*pzone).p2.x = pos.x - shift.x;
  }
  if (shift.x > 0 && shift.x < dim.x)
  {
    p1.y   = pos.y;
    p1.x   = pos.x + shift.x;
    p2.y   = pos.y;
    p2.x   = pos.x;
    _dim.y = dim.y;
    _dim.x = dim.x - shift.x;
    CopyPixel (ppm, p1, &tmp, p2, _dim);
    (*pzone).p1.x = pos.x + dim.x - shift.x;
  }
  if (shift.y < 0 && shift.y > -dim.y)
  {
    p1.y   = pos.y;
    p1.x   = pos.x;
    p2.y   = pos.y - shift.y;
    p2.x   = pos.x;
    _dim.y = dim.y + shift.y;
    _dim.x = dim.x;
    CopyPixel (ppm, p1, &tmp, p2, _dim);
    (*pzone).p2.y = pos.y - shift.y;
  }
  if (shift.y > 0 && shift.y < dim.y)
  {
    p1.y   = pos.y + shift.y;
    p1.x   = pos.x;
    p2.y   = pos.y;
    p2.x   = pos.x;
    _dim.y = dim.y - shift.y;
    _dim.x = dim.x;
    CopyPixel (ppm, p1, &tmp, p2, _dim);
    (*pzone).p1.y = pos.y + dim.y - shift.y;
  }

  DuplPixel (&tmp, ppm);
  GivePixmap (&tmp);

fill:
  if (color == -1)
    return;
  DrawFillRect (ppm, *pzone, color); /* init la zone à mettre à jour */
}

/* =========== */
/* ScrollPixel */
/* =========== */

/*
    Décale un pixmap dans l'une des quatre directions.
        *ppm		*pixmap source et destination
        shift.x		décalage horizontal (<0 vers la droite, >0 vers la gauche)
        shift.y		décalage vertical (<0 vers le bas, >0 vers le haut)
        color		couleur assignée à la nouvelle zone (0..15)
                    -1 laisse la zone originale
        *pzone		retourne la zone à redessiner après ScrollPixel
 */

void
ScrollPixel (Pixmap * ppm, Pt shift, char color, Rect * pzone)
{
  Pt pos, dim;

  pos.x = 0;
  pos.y = 0;
  dim.x = ppm->dx;
  dim.y = ppm->dy;

  ScrollPixelRect (ppm, pos, dim, shift, color, pzone);
}

/* ========= */
/* CopyPixel */
/* ========= */

/*
    Copie un rectangle de pixel dans un autre (raster-op).
    Cette procédure est la seule qui dessine dans l'écran !
        *ppms	*pixmap source (si == 0 -> écran)
        os		origine source (coin sup/gauche)
        *ppmd	*pixmap destination (si == 0 -> écran)
        od		origine destination (coin sup/gauche)
        dim		dimensions du rectangle
    Retourne 1 si rien n'a été dessiné.
 */

short
CopyPixel (Pixmap * ppms, Pt os, Pixmap * ppmd, Pt od, Pt dim)
{
  if (ppmd != 0)
  {
    if (od.x < 0) /* dépasse à gauche ? */
    {
      dim.x += od.x;
      if (dim.x <= 0)
        return 1;
      os.x -= od.x;
      od.x = 0;
    }
    if (od.x + dim.x > ppmd->dx) /* dépasse à droite ? */
    {
      dim.x -= od.x + dim.x - ppmd->dx;
      if (dim.x <= 0)
        return 1;
    }

    if (od.y < 0) /* dépasse en haut ? */
    {
      dim.y += od.y;
      if (dim.y <= 0)
        return 1;
      os.y -= od.y;
      od.y = 0;
    }
    if (od.y + dim.y > ppmd->dy) /* dépasse en bas ? */
    {
      dim.y -= od.y + dim.y - ppmd->dy;
      if (dim.y <= 0)
        return 1;
    }
  }

  if (ppms == 0) /* source dans l'écran ? */
  {
    os.x += origine.x;
    os.y += origine.y;
  }
  else
  {
    os.x += ppms->orig.x;
    os.y += ppms->orig.y;
  }

  if (ppmd == 0) /* destination dans l'écran ? */
  {
    od.x += origine.x;
    od.y += origine.y;
  }
  else
  {
    od.x += ppmd->orig.x;
    od.y += ppmd->orig.y;
  }

  SDL_Rect srcRect, dstRect;
  srcRect.x = os.x;
  srcRect.y = os.y;
  srcRect.w = dim.x;
  srcRect.h = dim.y;
  dstRect.x = od.x;
  dstRect.y = od.y;
  dstRect.w = dim.x;
  dstRect.h = dim.y;

  SDL_Texture * target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, ppmd ? ppmd->texture : g_screen.texture);
  SDL_RenderCopy (
    g_renderer, ppms ? ppms->texture : g_screen.texture, &srcRect, &dstRect);
  SDL_SetRenderTarget (g_renderer, target);

  return 0;
}

/* ======== */
/* DrawLine */
/* ======== */

/*
    Dessine un segment de droite d'un pixel d'épaisseur.
        *ppm		->	pixmap où dessiner (0 = écran)
        p1			->	départ
        p2			->	arrivée
        color		->	0 = blanc .. 15 = noir
 */

void
DrawLine (Pixmap * ppm, Pt p1, Pt p2, int color)
{
  Uint8 r, g, b, a;

  if (!ppm) /* source dans l'écran ? */
  {
    p1.x += origine.x;
    p1.y += origine.y;
    p2.x += origine.x;
    p2.y += origine.y;
  }
  else
  {
    p1.x += ppm->orig.x;
    p1.y += ppm->orig.y;
    p2.x += ppm->orig.x;
    p2.y += ppm->orig.y;
  }

  GetThemeRGB (color, &r, &g, &b, &a);

  SDL_Texture * target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_SetRenderDrawColor (g_renderer, r, g, b, a);
  SDL_SetRenderTarget (g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_RenderDrawLine (g_renderer, p1.x, p1.y, p2.x, p2.y);
  SDL_SetRenderTarget (g_renderer, target);
}

/* ======== */
/* DrawRect */
/* ======== */

/*
    Dessine un rectangle d'un pixel d'épaisseur.
        *ppm		->	pixmap où dessiner (0 = écran)
        rect.p1		->	coin sup/gauche
        rect.p2		->	coin inf/droite
        color		->	0 = blanc .. 15 = noir
 */

void
DrawRect (Pixmap * ppm, Rect rect, int color)
{
  Uint8    r, g, b, a;
  SDL_Rect _rect;

  _rect.x = rect.p1.x;
  _rect.y = rect.p1.y;
  _rect.w = rect.p2.x - rect.p1.x;
  _rect.h = rect.p2.y - rect.p1.y;

  GetThemeRGB (color, &r, &g, &b, &a);

  SDL_Texture * target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_SetRenderDrawColor (g_renderer, r, g, b, a);
  SDL_RenderDrawRect (g_renderer, &_rect);
  SDL_SetRenderTarget (g_renderer, target);
}

/* ============ */
/* DrawFillRect */
/* ============ */

/*
    Dessine une surface rectangulaire remplie avec une couleur donnée
    dans un pixmap.
        *ppm		->	pixmap où dessiner (0 = écran)
        rect.p1		->	coin sup/gauche
        rect.p2		->	coin inf/droite
        color		->	0 = blanc .. 15 = noir
 */

void
DrawFillRect (Pixmap * ppm, Rect rect, int color)
{
  Uint8    r, g, b, a;
  SDL_Rect _rect;

  _rect.x = rect.p1.x;
  _rect.y = rect.p1.y;
  _rect.w = rect.p2.x - rect.p1.x;
  _rect.h = rect.p2.y - rect.p1.y;

  GetThemeRGB (color, &r, &g, &b, &a);

  SDL_Texture * target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_SetRenderDrawColor (g_renderer, r, g, b, a);
  SDL_RenderFillRect (g_renderer, &_rect);
  SDL_SetRenderTarget (g_renderer, target);
}

/* ========== */
/* SavePixmap */
/* ========== */

/*
    Sauve un pixmap en mémoire étendue (XMS sur PC).
 */

short
SavePixmap (Pixmap * ppm)
{
  pmsave = *ppm;

  pmsave.texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, ppm->dx,
    ppm->dy);
  SDL_SetTextureBlendMode (pmsave.texture, SDL_BLENDMODE_BLEND);
  DuplPixel (ppm, &pmsave);
  return 0;
}

/* ============= */
/* RestorePixmap */
/* ============= */

/*
    Restitue le pixmap en mémoire étendue (XMS sur PC).
 */

short
RestorePixmap (Pixmap * ppm)
{
  if (pmsave.texture == 0)
    return 1;

  DuplPixel (&pmsave, ppm);
  SDL_DestroyTexture (pmsave.texture);
  pmsave.texture = NULL;
  return 0;
}

/* --------- */
/* LoadImage */
/* --------- */

/*
    Charge un fichier image codé, si nécessaire.
 */

static int
LoadImage (int numero, Pixmap * pim, Style style)
{
  int    err = 1;
  char   name[4096]; /* nom de l'image BLUPIXnn.IMAGE */
  char * lang = "";

  switch (g_langue)
  {
  case 0:
    lang = "en/";
    break;
  case 1:
    lang = "fr/";
    break;
  case 2:
    lang = "de/";
    break;
  }

  if (numero < 22 || numero == 33)
    lang = "";

  snprintf (
    name, sizeof (name), "%s../share/blupimania/image/%sblupix%02d.webp",
    SDL_GetBasePath (), lang, numero);

  SDL_Texture *   texture;
  IMG_Animation * anim;
  int             index;

  anim = IMG_LoadWEBPAnimation (name);

  index = 1;
  if (g_theme == 1)
    index = anim->count > 2 ? 2 : 0;
  else if (anim->count > 2)
  {
    switch (style)
    {
    default:
      break;
    case PASTEL:
      index = 3;
      break;
    case DARK:
      index = 4;
      break;
    case ROSY:
      index = 5;
      break;
    case BLUISH:
      index = 0;
      break;
    }
  }

  texture = SDL_CreateTextureFromSurface (g_renderer, anim->frames[index]);
  IMG_FreeAnimation (anim);

  Uint32 format;
  Sint32 access, ow, oh;
  SDL_QueryTexture (texture, &format, &access, &ow, &oh);
  pim->dx = ow;
  pim->dy = oh;

  pim->texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, pim->dx,
    pim->dy);
  SDL_SetTextureBlendMode (pim->texture, SDL_BLENDMODE_BLEND);

  SDL_Texture * target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, pim->texture);
  SDL_RenderCopy (g_renderer, texture, NULL, NULL);
  SDL_DestroyTexture (texture);
  SDL_SetRenderTarget (g_renderer, target);

  err = 0; /* chargement image ok */
  return err;
}

/* ========= */
/* GetPixmap */
/* ========= */

/*
    Ouvre un pixmap quelconque, tout blanc ou tout noir.
        fill:	0 -> pixmap tout blanc
                1 -> pixmap tout noir
        color:	0 -> monochrome (1 bit/pixel)
                1 -> couleur (si possible)
                2 -> couleur (toujours)
 */

short
GetPixmap (Pixmap * ppm, Pt dim, short fill, short color)
{
  if (ppm->texture)
    SDL_DestroyTexture (ppm->texture);

  ppm->texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, dim.x, dim.y);
  SDL_SetTextureBlendMode (ppm->texture, SDL_BLENDMODE_BLEND);
  ppm->dx = dim.x;
  ppm->dy = dim.y;

  if (fill >= 0)
  {
    SDL_Texture * target = SDL_GetRenderTarget (g_renderer);
    SDL_SetRenderTarget (g_renderer, ppm->texture);
    if (fill == 0)
      SDL_SetRenderDrawColor (g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    else if (fill == 1)
      SDL_SetRenderDrawColor (g_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    else if (fill == 2)
      SDL_SetRenderDrawColor (g_renderer, 255, 255, 255, SDL_ALPHA_TRANSPARENT);
    SDL_RenderClear (g_renderer);
    SDL_SetRenderTarget (g_renderer, target);
  }

  return 0;
}

/* ========== */
/* GivePixmap */
/* ========== */

/*
    Libère un pixmap quelconque, obtenu avec GetPixmap ou GetImage,
    mais pas avec GetIcon (give pas nécessaire).
 */

short
GivePixmap (Pixmap * ppm)
{
  if (ppm->texture)
    SDL_DestroyTexture (ppm->texture);
  SDL_memset (ppm, 0, sizeof (*ppm));
  return 0;
}

/* ======== */
/* GetImage */
/* ======== */

/*
    Ouvre une image en la lisant si nécessaire, puis en la décodant
    dans un pixmap.
 */

short
GetImage (Pixmap * ppm, short numero, Style style)
{
  int err;

  err = LoadImage (numero, ppm, style); /* charge l'image */
  if (err)
    goto error;
error:
  return err;
}

/* ======= */
/* GetIcon */
/* ======= */

/*
    Conversion d'une icône en pixmap noir/blanc.
    Le pointeur au data (ppm->data) est directement initialisé dans
    l'une des grandes images contenant toutes les icônes (IMAICON
    ou IMAICON+IMAMASK).
    Puisque la mémoire pour le data n'est pas allouée par cette
    procédure, il ne faut donc pas faire de GivePixmap !
    Si le mode vaut zéro, le data n'est pas rendu, mais seulement
    les dimensions, pour gagner du temps.
 */

short
GetSprite (Pixmap * ppm, short numero, short mode)
{
  int no;

  ppm->dx = LXICO;
  ppm->dy = LYICO;

  no = numero;
  if ((numero & ICONMASK) < 128 * 1)
  {
    ppm->texture = pmicon1c.texture;
    goto data;
  }
  else if ((numero & ICONMASK) < 128 * 2)
  {
    ppm->texture = pmicon2c.texture;
    no -= 128 * 1;
    goto data;
  }
  else if ((numero & ICONMASK) < 128 * 3)
  {
    ppm->texture = pmicon3c.texture;
    no -= 128 * 2;
    goto data;
  }
  else if ((numero & ICONMASK) < 128 * 4)
  {
    ppm->texture = pmicon4c.texture;
    no -= 128 * 3;
    goto data;
  }
  else
    return 1;

data:
  ppm->orig.x = LXICO * ((no & ICONMASK) % 16);
  ppm->orig.y = LYICO * ((no & ICONMASK) / 16);

  switch (no & (~ICONMASK))
  {
  case UL:
    ppm->dx /= 2L;
    ppm->dy /= 2L;
    break;

  case UR:
    ppm->dx /= 2L;
    ppm->dy /= 2L;
    ppm->orig.x += LXICO / 2;
    break;

  case DL:
    ppm->dx /= 2L;
    ppm->dy /= 2L;
    ppm->orig.y += LYICO / 2;
    break;

  case DR:
    ppm->dx /= 2L;
    ppm->dy /= 2L;
    ppm->orig.x += LXICO / 2;
    ppm->orig.y += LYICO / 2;
    break;
  }

  return 0; /* retour toujours ok */
}

/* -------- */
/* LoadIcon */
/* -------- */

/*
    Chargement de l'image des icônes.
 */

int
LoadSprites (Style style)
{
  static Style _style  = -1;
  static short _theme  = -1;
  static short _langue = -1;
  int          err;

  if (_style == style && _theme == g_theme && _langue == g_langue)
    return 0;

  _style = style;
  _theme = g_theme;

  err =
    GetImage (&pmicon1c, IMAICON + 0, style); /* charge l'image des icônes */
  if (err)
    return err;

  err =
    GetImage (&pmicon2c, IMAICON + 1, style); /* charge l'image des icônes */
  if (err)
    return err;

  err =
    GetImage (&pmicon3c, IMAICON + 2, style); /* charge l'image des icônes */
  if (err)
    return err;

  err =
    GetImage (&pmicon4c, IMAICON + 3, style); /* charge l'image des icônes */
  if (err)
    return err;

  return 0;
}

void
UnloadSprites ()
{
  GivePixmap (&pmicon1c);
  GivePixmap (&pmicon2c);
  GivePixmap (&pmicon3c);
  GivePixmap (&pmicon4c);
}

static void
MusicStopped (void)
{
  PushUserEvent (MUSIC_STOP, NULL);
}

static int
InitSoundSystem ()
{
  if (
    Mix_OpenAudio (44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)
    return -1;

  Mix_AllocateChannels (SOUND_MAX);
  Mix_HookMusicFinished (MusicStopped);
  return 0;
}

/**
 * Load sounds
 */
static int
LoadSounds (void)
{
  static const char * sounds[] = {
    NULL,

    /* sounds */
    "saut1.wav",
    "saut2.wav",
    "trop_bu.wav",
    "tombe.wav",
    "trouve_b.wav",
    "trouve_c.wav",
    "boit.wav",
    "magie.wav",
    "electro.wav",
    "depart.wav",
    "repos.wav",
    "dort.wav",
    "glisse.wav",
    "tourte.wav",
    "lunettes.wav",
    "creve_ba.wav",
    "livre.wav",
    "malade.wav",
    "pousse.wav",
    "sens_uni.wav",
    "porte_ou.wav",
    "porte_bl.wav",
    "un_seul.wav",
    "vitre_ca.wav",
    "bombe.wav",
    "non-non.wav",
    "clic.wav",
    "tombe_ca.wav",
    "tombe_st.wav",
    "tombe_bo.wav",
    "action_d.wav",
    "mechant.wav",
    "passe_mu.wav",
    "aimant.wav",
    "porte_b2.wav",
    "machine.wav",
    "oiseaux.wav",
    "burp.wav",
    "tombe_ma.wav",

    /* jingles */
    "funk_1-1.wav",
    "funk_1-2.wav",
    "funk_1-3.wav",
    "funk_1-b.wav",
    "funk_2-1.wav",
    "funk_2-2.wav",
    "funk_2-3.wav",
    "funk_2-b.wav",
    "dixie_1-1.wav",
    "dixie_1-2.wav",
    "dixie_1-3.wav",
    "dixie_1-b.wav",
    "rock_1-1.wav",
    "rock_1-2.wav",
    "rock_1-3.wav",
    "rock_1-b.wav",
  };

  for (int i = 1; i < countof (sounds); ++i)
  {
    char filename[4096];
    snprintf (
      filename, sizeof (filename), "%s../share/blupimania/sound/%s",
      SDL_GetBasePath (), sounds[i]);
    Mix_Chunk * chunk = Mix_LoadWAV (filename);
    if (!chunk)
      continue;

    g_sounds[i] = chunk;
  }

  return 0;
}

static void
UnloadSounds (void)
{
  for (int i = 0; i < countof (g_sounds); ++i)
    if (g_sounds[i])
    {
      Mix_FreeChunk (g_sounds[i]);
      g_sounds[i] = NULL;
    }

  if (g_music)
  {
    Mix_FreeMusic (g_music);
    g_music = NULL;
  }

  Mix_CloseAudio ();
}

/* =========== */
/* BlackScreen */
/* =========== */

/*
    Efface tout l'écran (noir), pendant le changement de la clut.
 */

void
BlackScreen (void)
{
  SDL_SetRenderDrawColor (g_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear (g_renderer);
}

static SDL_bool
isShare (char file)
{
  return file >= '1' && file <= '8';
}

static void
getLocationPattern (char file, char * filename, size_t length)
{
  char * lang = "";

  switch (g_langue)
  {
  case 0:
    lang = "en/";
    break;
  case 1:
    lang = "fr/";
    break;
  case 2:
    lang = "de/";
    break;
  }

  if (isShare (file))
    snprintf (
      filename, length, "%s../share/blupimania/data/%sblupixa.dat",
      SDL_GetBasePath (), lang);
  else
    snprintf (
      filename, length, "%sblupixa.dat",
      SDL_GetPrefPath ("Epsitec SA", "Blupimania"));
}

/* ======== */
/* FileRead */
/* ======== */

/*
    Lit nb bytes dans un fichier de données à la position pos.
    Retourne 0 si la lecture est ok.
 */

short
FileRead (void * pdata, long pos, short nb, char file)
{
  FILE * channel;
  char   filename[4096];
  short  n = 0;

  getLocationPattern (file, filename, sizeof (filename));

  filename[strlen (filename) - 5] = file;
  channel = fopen (filename + n, "rb"); /* ouvre le fichier */
  if (channel == NULL)
    return errno;

  if (fseek (channel, pos, SEEK_SET) != 0)
    goto close;

  if (fread (pdata, nb, 1, channel) != 1)
    goto close;

close:
  fclose (channel); /* ferme le fichier */
  return 0;
}

/* ========= */
/* FileWrite */
/* ========= */

/*
    Ecrit nb bytes dans un fichier de données à la position pos.
    Retourne 0 si l'écriture est ok.
 */

short
FileWrite (void * pdata, long pos, short nb, char file)
{
  FILE * channel;
  char   filename[4096];

  getLocationPattern (file, filename, sizeof (filename));

  filename[strlen (filename) - 5] = file;

  channel = fopen (filename, "r+b");
  if (!channel)
  {
    channel = fopen (filename, "wb");
    if (!channel)
      return errno;
    fclose (channel);

    channel = fopen (filename, "r+b");
  }

  if (!channel)
    return errno;

  if (fseek (channel, pos, SEEK_SET) != 0)
    goto close;

  if (fwrite (pdata, nb, 1, channel) != 1)
    goto close;

close:
  fclose (channel); /* ferme le fichier */
  return 0;
}

/* ============= */
/* FileGetLength */
/* ============= */

/*
    Retourne la longueur d'un fichier.
    En cas d'erreur, retourne 0 (longueur nulle).
 */

long
FileGetLength (char file)
{
  FILE * channel;
  char   filename[4096];
  short  n = 0;
  long   lg;

  getLocationPattern (file, filename, sizeof (filename));

  filename[strlen (filename) - 5] = file;
  channel = fopen (filename + n, "r"); /* ouvre le fichier */
  if (channel == NULL)
    return 0;

  fseek (channel, 0, SEEK_END);
  lg = ftell (channel);

  fclose (channel); /* ferme le fichier */
  return lg;
}

/* ========== */
/* FileDelete */
/* ========== */

/*
    Détruit un fichier.
    Retourne 0 si la destruction est ok.
 */

short
FileDelete (char file)
{
  char filename[4096];

  getLocationPattern (file, filename, sizeof (filename));

  filename[strlen (filename) - 5] = file;
  return remove (filename);
}

/* ========== */
/* FileRename */
/* ========== */

/*
    Renomme un fichier.
    Retourne 0 si le changement est ok.
 */

short
FileRename (char oldfile, char newfile)
{
  char oldfilename[4096];
  char newfilename[4096];

  getLocationPattern (oldfile, oldfilename, sizeof (oldfilename));
  getLocationPattern (newfile, newfilename, sizeof (newfilename));

  oldfilename[strlen (oldfilename) - 5] = oldfile;
  newfilename[strlen (newfilename) - 5] = newfile;
  return rename (oldfilename, newfilename);
}

static int
parseOptions (int argc, char * argv[], struct arguments * arguments)
{
  struct arg_int * s, *t, *z;
  struct arg_lit * h, *v, *f;
  struct arg_str * m, *r, *d;
  struct arg_end * end;

  void * argtable[] = {
    h = arg_lit0 ("h", "help", "print this help message and exit"),
    v = arg_lit0 ("V", "version", "print version and exit"),
    s = arg_int0 (
      "s", "speed-rate", NULL, "Change the speed rate [0;1;2] (default: 1)"),
    t = arg_int0 (
      "t", "timer-interval", NULL,
      "set the timer interval (refresh) (default: 25)"),
    f =
      arg_lit0 ("f", "fullscreen", "load in fullscreen [on;off] (default: on)"),
    z = arg_int0 (
      "z", "zoom", NULL,
      "change the window scale (only if fullscreen is off) [1;2] (default: 2)"),
    m = arg_str0 (
      "m", "theme", NULL, "change the theme [dos;smaky100] (default: dos)"),
    r = arg_str0 (
      "r", "renderer", NULL,
      "set a renderer [auto;software;accelerated] (default: auto)"),
    d = arg_str0 (
      "d", "driver", NULL,
      "set a driver [auto;direct3d;direct3d11;opengl;opengles2;opengles] "
      "(default: auto, ignored with software renderer)"),
    end = arg_end (2),
  };

  int  exitcode   = 0;
  char progname[] = "blupimania";

  int nerrors;
  nerrors = arg_parse (argc, argv, argtable);

  /* special case: '--help' takes precedence over error reporting */
  if (h->count > 0)
  {
    printf ("Usage: %s", progname);
    arg_print_syntax (stdout, argtable, "\n");
    arg_print_glossary (stdout, argtable, "    %-25s\n        %s\n");
    exitcode = -1;
    goto exit;
  }

  /* special case: '--version' takes precedence error reporting */
  if (v->count > 0)
  {
    printf (BLUPIMANIA_VERSION_STR);
    exitcode = -1;
    goto exit;
  }

  if (nerrors > 0)
  {
    /* Display the error details contained in the arg_end struct.*/
    arg_print_errors (stdout, end, progname);
    printf ("Try '%s --help' for more information.\n", progname);
    exitcode = 1;
    goto exit;
  }

  if (s->count > 0)
  {
    g_settingsOverload |= SETTING_SPEEDRATE;
    if (*s->ival < 0 || *s->ival > 2)
    {
      fprintf (stderr, "speed-rate must be 0, 1 or 2");
      exitcode = 1;
      goto exit;
    }
    arguments->speedrate = *s->ival;
  }

  if (t->count > 0)
  {
    g_settingsOverload |= SETTING_TIMERINTERVAL;
    if (*t->ival < 10)
    {
      fprintf (stderr, "timer-interval must be greater or equal to 10");
      exitcode = 1;
      goto exit;
    }
    arguments->timerinterval = *t->ival;
  }

  if (f->count > 0)
  {
    g_settingsOverload |= SETTING_FULLSCREEN;
    arguments->fullscreen = SDL_TRUE;
  }

  if (z->count > 0)
  {
    g_settingsOverload |= SETTING_ZOOM;
    if (*z->ival < 1 || *z->ival > 2)
    {
      fprintf (stderr, "zoom must be 1 or 2");
      exitcode = 1;
      goto exit;
    }
    arguments->zoom = *z->ival;
  }

  if (m->count > 0)
  {
    g_settingsOverload |= SETTING_THEME;
    free (arguments->theme);
    arguments->theme = SDL_strdup (*m->sval);
  }

  if (r->count > 0)
  {
    g_settingsOverload |= SETTING_RENDERER;
    free (arguments->renderer);
    arguments->renderer = SDL_strdup (*r->sval);
  }

  if (d->count > 0)
  {
    g_settingsOverload |= SETTING_DRIVER;
    free (arguments->driver);
    arguments->driver = SDL_strdup (*d->sval);
  }

exit:
  arg_freetable (argtable, sizeof (argtable) / sizeof (argtable[0]));
  return exitcode;
}

static int
parseArgs (int argc, char * argv[], struct arguments * arguments)
{
  arguments->speedrate     = 1;
  arguments->timerinterval = g_timerInterval;
  arguments->fullscreen    = SDL_FALSE;
  arguments->zoom          = 2;
  arguments->renderer      = SDL_strdup ("auto");
  arguments->driver        = SDL_strdup ("auto");
  arguments->theme         = SDL_strdup ("dos");

  int rc = parseOptions (argc, argv, arguments);
  if (rc)
    return rc;

  g_timerInterval = arguments->timerinterval;

  if (!SDL_strcasecmp (arguments->renderer, "auto"))
    g_rendererType = 0;
  else if (!SDL_strcasecmp (arguments->renderer, "software"))
    g_rendererType = SDL_RENDERER_SOFTWARE;
  else if (!SDL_strcasecmp (arguments->renderer, "accelerated"))
    g_rendererType = SDL_RENDERER_ACCELERATED;

  if ((!g_rendererType || g_rendererType == SDL_RENDERER_ACCELERATED))
  {
    if (
      SDL_strstr (arguments->driver, "direct3d") ||
      SDL_strstr (arguments->driver, "direct3d11") ||
      SDL_strstr (arguments->driver, "opengl") ||
      SDL_strstr (arguments->driver, "opengles2") ||
      SDL_strstr (arguments->driver, "opengles"))
      SDL_SetHint (SDL_HINT_RENDER_DRIVER, arguments->driver);
  }

  return EXIT_SUCCESS;
}

/* =========== */
/* OpenMachine */
/* =========== */

/*
    Ouverture générale, chargement des librairies, gencar, etc.
 */

int
OpenMachine (int argc, char * argv[], struct arguments * arguments)
{
  int rc = parseArgs (argc, argv, arguments);
  if (rc)
    return rc;

  SDL_Locale * locales = SDL_GetPreferredLocales ();
  if (locales)
  {
    if (!strncmp (locales[0].language, "en", 2))
      g_langue = 0;
    else if (!strncmp (locales[0].language, "fr", 2))
      g_langue = 1;
    else if (!strncmp (locales[0].language, "de", 2))
      g_langue = 2;
  }

  g_colors = g_colorsTheme[g_theme];

#ifdef __LINUX__
  if (!getenv ("ALSA_CONFIG_DIR"))
  {
    static char env[256];
    snprintf (env, sizeof (env), "ALSA_CONFIG_DIR=/usr/share/alsa");
    putenv (env);
  }
#endif /* __LINUX__ */

#ifdef _WIN32
  /* Fix laggy sounds on Windows by not using winmm driver. */
  SDL_setenv ("SDL_AUDIODRIVER", "directsound", SDL_TRUE);
#endif /* _WIN32 */

  int res = SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
  if (res < 0)
  {
    SDL_Log ("Unable to initialize SDL: %s", SDL_GetError ());
    return EXIT_FAILURE;
  }

  g_window = SDL_CreateWindow (
    "Blupimania", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, LXIMAGE (),
    LYIMAGE (), 0);

  if (!g_window)
  {
    printf ("%s", SDL_GetError ());
    return EXIT_FAILURE;
  }

#ifdef __linux__
  {
    char iconFile[4096] = {0};
    snprintf (
      iconFile, sizeof (iconFile),
      "%s../share/icons/hicolor/256x256/apps/blupimania.png",
      SDL_GetBasePath ());
    SDL_Surface * icon = IMG_Load (iconFile);
    SDL_SetWindowIcon (g_window, icon);
    SDL_FreeSurface (icon);
  }
#endif /* __linux__ */

  g_renderer = SDL_CreateRenderer (
    g_window, -1,
    g_rendererType | SDL_RENDERER_TARGETTEXTURE /*| SDL_RENDERER_SOFTWARE*/);
  if (!g_renderer)
  {
    printf ("%s", SDL_GetError ());
    SDL_DestroyWindow (g_window);
    return EXIT_FAILURE;
  }

  SDL_RenderSetLogicalSize (g_renderer, LXIMAGE (), LYIMAGE ());

  const int renders = SDL_GetNumRenderDrivers ();
  for (int i = 0; i < renders; ++i)
  {
    SDL_RendererInfo info = {0};
    if (SDL_GetRenderDriverInfo (i, &info))
    {
      SDL_LogError (SDL_LOG_CATEGORY_APPLICATION, "renderer[%d]: failed\n", i);
      continue;
    }

    SDL_LogInfo (
      SDL_LOG_CATEGORY_APPLICATION, "renderer[%d]: name=%s", i, info.name);
    SDL_LogInfo (
      SDL_LOG_CATEGORY_APPLICATION, "renderer[%d]: flags=%u", i, info.flags);
    SDL_LogInfo (
      SDL_LOG_CATEGORY_APPLICATION, "renderer[%d]: num_texture_formats=%u", i,
      info.num_texture_formats);
    SDL_LogInfo (
      SDL_LOG_CATEGORY_APPLICATION, "renderer[%d]: max_texture_width=%u", i,
      info.max_texture_width);
    SDL_LogInfo (
      SDL_LOG_CATEGORY_APPLICATION, "renderer[%d]: max_texture_height=%u", i,
      info.max_texture_height);
  }

  InitSoundSystem ();
  LoadSounds ();

  StartRandom (0, 0); /* coup de sac du générateur aléatoire toto */
  StartRandom (1, 1); /* coup de sac du générateur aléatoire décor */

  keystatus = 0;

  return 0;
}

/* ============ */
/* CloseMachine */
/* ============ */

/*
    Fermeture générale.
 */

void
CloseMachine (struct arguments * arguments)
{
  UnloadSprites ();
  UnloadSounds ();

  if (g_renderer)
    SDL_DestroyRenderer (g_renderer);

  if (g_window)
    SDL_DestroyWindow (g_window);

  SDL_Quit ();

  if (arguments->theme)
    free (arguments->theme);
  if (arguments->renderer)
    free (arguments->renderer);
  if (arguments->driver)
    free (arguments->driver);
}

/* =============== */
/* MachinePartieLg */
/* =============== */

/*
    Retourne la longueur nécessaire pour sauver les variables de la partie en
   cours.
 */

long
MachinePartieLg (void)
{
  return sizeof (unsigned long) * 10;
}

/* ================== */
/* MachinePartieWrite */
/* ================== */

/*
    Sauve les variables de la partie en cours.
 */

short
MachinePartieWrite (long pos, char file)
{
  short err;

  err = FileWrite (&nextrand, pos, sizeof (unsigned long) * 10, file);
  return err;
}

/* ================= */
/* MachinePartieRead */
/* ================= */

/*
    Lit les variables de la partie en cours.
 */

short
MachinePartieRead (long pos, char file)
{
  short err;

  err = FileRead (&nextrand, pos, sizeof (unsigned long) * 10, file);
  return err;
}

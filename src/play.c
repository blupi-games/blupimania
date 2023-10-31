
/* ========= */
/* bm_play.c */
/* ========= */

/*
    Version	Date		Modifications
    -------------------------------------------------------------------------------
    1.0	24.08.94	début des travaux ...
 */

#include <stdio.h>
#include <string.h>

#ifdef USE_CURL
#include <curl/curl.h>
#endif /* USE_CURL */

#include "actions.h"
#include "blupimania.h"
#include "icon.h"

#if 0
#define DEMONC 1 /* si version demo -> pas de construction */
#endif

static SDL_bool         g_standby = SDL_FALSE;
static struct arguments arguments;

static SDL_Cursor * g_cursor; /* mouse sprite */
static short        g_zoom;

int                      g_settingsOverload;
static volatile SDL_bool g_updateAbort       = SDL_FALSE;
static SDL_Thread *      g_updateThread      = NULL;
static char              g_updateVersion[16] = {0};
static int               g_updateBlinking    = 0;

#ifdef USE_CURL
struct url_data {
  CURLcode status;
  char *   buffer;
  size_t   size;
};
#endif /* USE_CURL */

/* Fichiers sur disque */
/* ------------------- */

/*
    BLUPIX?.DAT		banque	contenu
    ---------------------------------------------------------------------
    1..8		A..H	énigmes niveaux 1A, 1T, 2A, 2T, ... 4T
    E..H		I..L	énigmes niveaux 5 (privé) pour les 4 joueurs
    I..P		A..H	énigmes mode démo
    W				parties sauvées (démo)
    X				joueurs (démo)
    Y				parties sauvées (normal)
    Z				joueurs (normal)
 */

/* ------------- */
/* Phases du jeu */
/* ------------- */

typedef enum {
  PHASE_GENERIC,
  PHASE_IDENT,
  PHASE_AIDE,
  PHASE_AIDE11,
  PHASE_AIDE12,
  PHASE_AIDE13,
  PHASE_AIDE21,
  PHASE_AIDE22,
  PHASE_AIDE23,
  PHASE_AIDE24,
  PHASE_AIDE31,
  PHASE_AIDE32,
  PHASE_AIDE33,
  PHASE_AIDE34,
  PHASE_AIDE35,
  PHASE_AIDE36,
  PHASE_AIDE41,
  PHASE_AIDE42,
  PHASE_INIT,
  PHASE_REGLAGE,
  PHASE_OBJECTIF,
  PHASE_PLAY,
  PHASE_RECOMMENCE,
  PHASE_SUIVANT,
  PHASE_PRIVE,
  PHASE_PARAM,
  PHASE_FINI0,
  PHASE_FINI1,
  PHASE_FINI2,
  PHASE_FINI3,
  PHASE_FINI4,
  PHASE_FINI5,
  PHASE_FINI6,
  PHASE_FINI7,
  PHASE_FINI8,
  PHASE_OPER,
  PHASE_DEPLACE,
  PHASE_ATTENTE,
  PHASE_REGLAGE2,
  PHASE_GOODBYE
} Phase;

/* ----------------------------- */
/* Actions pour changer de phase */
/* ----------------------------- */

typedef enum {
  ACTION_AIDE,
  ACTION_AIDE11,
  ACTION_AIDE12,
  ACTION_AIDE13,
  ACTION_AIDE21,
  ACTION_AIDE22,
  ACTION_AIDE23,
  ACTION_AIDE24,
  ACTION_AIDE31,
  ACTION_AIDE32,
  ACTION_AIDE33,
  ACTION_AIDE34,
  ACTION_AIDE35,
  ACTION_AIDE36,
  ACTION_AIDE41,
  ACTION_AIDE42,
  ACTION_OBJECTIF,
  ACTION_JOUE,
  ACTION_REGLAGE,
  ACTION_SUIVANT,
  ACTION_DEBUT,
  ACTION_FINI,
  ACTION_ANNULE,
  ACTION_STOPPEOK,
  ACTION_STOPPEKO,
  ACTION_EDIT,
  ACTION_PARAM,
  ACTION_MONDEPREC,
  ACTION_MONDESUIV,
  ACTION_MONDEBAR,
  ACTION_OPER,
  ACTION_DEPLACE,
  ACTION_DUPLIQUE,
  ACTION_DETRUIT,
  ACTION_ORDRE,
  ACTION_JOUEUR0,
  ACTION_JOUEUR1,
  ACTION_JOUEUR2,
  ACTION_JOUEUR3,
  ACTION_NIVEAU0,
  ACTION_NIVEAU1,
  ACTION_NIVEAU2,
  ACTION_NIVEAU3,
  ACTION_NIVEAU4,
  ACTION_NIVEAU5,
  ACTION_NIVEAU6,
  ACTION_NIVEAU7,
  ACTION_NIVEAU8,
  ACTION_NIVEAUK1,
  ACTION_NIVEAUK2,
  ACTION_NIVEAUK3,
  ACTION_NIVEAUK4,
  ACTION_NIVEAUK5,
  ACTION_NIVEAUGO,
  ACTION_VITESSE0,
  ACTION_VITESSE1,
  ACTION_VITESSE2,
  ACTION_SCROLL0,
  ACTION_SCROLL1,
  ACTION_BRUIT0,
  ACTION_BRUIT1,
  ACTION_NOISEVOLP,
  ACTION_NOISEVOLM,
  ACTION_MUSICVOLP,
  ACTION_MUSICVOLM,
  ACTION_TELECOM0,
  ACTION_TELECOM1,
  ACTION_COULEUR0,
  ACTION_COULEUR1,
  ACTION_COULEUR2,
  ACTION_COULEUR3,
  ACTION_COULEUR4,
  ACTION_IDENT,
  ACTION_QUITTE,
  ACTION_REGLAGE2,
  ACTION_SCREEN_1,
  ACTION_SCREEN_2,
  ACTION_SCREEN_FULL,
  ACTION_LANG_EN,
  ACTION_LANG_FR,
  ACTION_LANG_DE,
  ACTION_THEME_DOS,
  ACTION_THEME_SMAKY100,
} PhAction;

/* -------------------------- */
/* Animations dans les images */
/* -------------------------- */

typedef enum { ANIM_JOUE, ANIM_CHOIX, ANIM_QUITTE } Anim;

/* -------------------------------- */
/* Structure du fichier des joueurs */
/* -------------------------------- */

#define MAXJOUEUR 4 /* nb max de joueurs différents */
#define MAXNOMJ 40  /* lg max du nom d'un joueur */

typedef struct {
  short majrev, minrev;          /* révision.version du logiciel */
  short check;                   /* anti-bidouilleurs */
  short reserve1[100];           /* réserve */
  short joueur;                  /* dernier joueur utilisé */
  char  nom[MAXJOUEUR][MAXNOMJ]; /* noms des joueurs */
  short niveau[MAXJOUEUR];       /* niveau de difficulté */
  short progres[MAXJOUEUR][9];   /* mondes atteints selon les niveaux */
  short vitesse;                 /* vitesse (0..2) */
  short scroll;                  /* scroll (0..1) */
  short bruitage;                /* bruitages (0..1) */
  short modetelecom;             /* mode de la télécommande (0..1) */
  short noisevolume;             /* volume bruitages */
  short musicvolume;             /* volume musique */
  short language;                /* language (en, fr, de) */
  short screen;                  /* zoom (normal, double, fullscreen) */
  short theme;                   /* theme (DOS, Smaky 100) */
  short resolved[MAXJOUEUR][9];  /* fully resolved by levels */
  short reserve2[54];            /* réserve */
} Joueur;

/* --------------------------- */
/* Variables globales externes */
/* --------------------------- */

short g_langue = 0;   /* numéro de la langue */
short g_theme  = 0;   /* 1 -> theme Smaky 100 */
short g_monde;        /* monde actuel */
short g_updatescreen; /* 1 -> écran à mettre à jour */
short g_typejeu;      /* type de jeu (0..1) */
short g_typeedit;     /* 1 -> édition d'un monde */
short g_typetext;     /* 1 -> édition d'un texte */
short g_modetelecom;  /* 1 -> mode télécommande gauche/droite */
short g_pause;        /* 1 -> pause */
short g_passdaniel;   /* 1 -> toujours construction */
short g_passpower;    /* 1 -> force infinie */
short g_passnice;     /* 1 -> toujours gentil */
short g_passhole;     /* 1 -> ne tombe pas dans trou */
short g_construit;    /* 1 -> construit */

SDL_Renderer * g_renderer;
SDL_Window *   g_window;
Pixmap         g_screen = {0};

int      g_rendererType    = 0;
Sint32   g_timerInterval   = 50;
Sint32   g_timerSkip       = 2;
Pt       g_lastmouse       = {0};
SDL_bool g_clearKeyEvents  = SDL_FALSE;
SDL_bool g_ignoreKeyClicUp = SDL_FALSE;
Pt       g_keyMousePos     = {0};
SDL_bool g_keyMousePressed = SDL_FALSE;
Sint32   g_keyFunctionUp   = 0;
SDL_bool g_subMenu         = SDL_FALSE;
SDL_bool g_stopMenu        = SDL_FALSE;
SDL_bool g_saveMenu        = SDL_FALSE;

/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Pixmap   pmimage    = {0}; /* pixmap pour image */
static int      pmimageNum = -1;
static Pixmap   pmtemp     = {0}; /* pixmap temporaire */
static Phase    phase;            /* phase du jeu */
static char     banque;           /* banque utilisée */
static short    mondeinit;        /* numéro du monde initial */
static short    maxmonde;         /* nb max de mondes */
static Monde    descmonde;        /* description du monde */
static Monde    savemonde;        /* sauvetage d'un monde */
static Joueur   fj;               /* fichier des joueurs */
static char     lastkey;          /* dernière touche pressée */
static SDL_bool fromClic = SDL_FALSE;
static short    retry;                   /* nb de tentatives */
static short    generic;                 /* pas du générique */
static short    musique    = 0;          /* musique de fond */
static short    lastaccord = -1;         /* dernier accord joué */
static char     musiquehex[10];          /* musique hazard exclusif */
static short    lastnoisevolume = 8 - 3; /* dernier volume bruiutages */
static short    lastmusicvolume = 8 - 3; /* dernier volume musique */

static short * animpb;   /* animation de base en cours */
static short * animpt;   /* animation en cours */
static short   animnext; /* pas en cours (0..n) */
static short   animdel;  /* délai avant la première animation */
static Pt      animpos;  /* dernière position de la souris */

static short passrang;  /* rang du mot de passe */
static short passindex; /* index de l'édition du mot de passe */

static char randomexrecommence[30]; /* tirage exclusif texte si recommence */
static char randomexsuivant[30];    /* tirage exclusif texte si réussi */

SDL_bool g_afterglow        = SDL_TRUE;
Pixmap   g_screenBase       = {0};
Pixmap   g_screenAfterglow0 = {0};
Pixmap   g_screenAfterglow1 = {0};

typedef struct {
  short ident;       /* identificateur */
  int   lg[10];      /* longueurs */
  short reserve[10]; /* réserve */
} Header;

typedef struct {
  int   check;       /* vérification */
  short monde;       /* monde actuel */
  short typejeu;     /* type de jeu (0..1) */
  char  banque;      /* banque utilisée */
  short reserve[10]; /* réserve */
} Partie;

static void AnimDrawInit (void);
static void PlayRelease (void);
void        FatalBreak (short err);

#define ___ ICO_SOLPAVE /* sol pavé normal */

static short tabmonde[MAXCELY][MAXCELX] = {
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___},
  {___, ___, ___, ___, ___, ___, ___, ___, ___, ___,
   ___, ___, ___, ___, ___, ___, ___, ___, ___, ___}};

/* Palette d'icônes pendant la construction */
/* ---------------------------------------- */

static short tabpalette[] = {
  ICO_OUTIL_TRACKS,
  999,
  ICO_OUTIL_SOLPAVE,
  999,
  ICO_OUTIL_SOLCARRE,
  999,
  ICO_OUTIL_SOLDALLE1,
  999,
  ICO_OUTIL_SOLDALLE2,
  999,
  ICO_OUTIL_SOLDALLE3,
  999,
  ICO_OUTIL_SOLDALLE4,
  999,
  ICO_OUTIL_SOLDALLE5,
  999,
  ICO_OUTIL_SOLELECTRO,
  999,
  ICO_OUTIL_SOLOBJET,
  999,
  -1,

  ICO_OUTIL_BARRIERE,
  999,
  ICO_OUTIL_MUR,
  999,
  ICO_OUTIL_VITRE,
  999,
  ICO_OUTIL_PLANTE,
  999,
  ICO_OUTIL_PLANTEBAS,
  999,
  ICO_OUTIL_BOIS,
  999,
  ICO_OUTIL_ELECTRO,
  999,
  ICO_OUTIL_ELECTROBAS,
  999,
  ICO_OUTIL_TECHNO,
  999,
  ICO_OUTIL_MEUBLE,
  999,
  ICO_OUTIL_OBSTACLE,
  999,
  -1,

  ICO_OUTIL_VISION,
  999,
  ICO_OUTIL_BOIT,
  999,
  ICO_OUTIL_AIMANT,
  999,
  ICO_OUTIL_LIVRE,
  999,
  ICO_OUTIL_MAGIC,
  999,
  ICO_OUTIL_SENSUNI,
  999,
  ICO_OUTIL_UNSEUL,
  999,
  ICO_OUTIL_CLE,
  999,
  ICO_OUTIL_PORTE,
  999,
  ICO_OUTIL_DETONATEUR,
  999,
  ICO_OUTIL_BOMBE,
  999,
  ICO_OUTIL_BAISSE,
  999,
  -1,

  ICO_OUTIL_DEPART,
  999,
  ICO_OUTIL_JOUEUR,
  999,
  ICO_OUTIL_TANK,
  999,
  ICO_OUTIL_INVINCIBLE,
  999,
  ICO_OUTIL_TROU,
  999,
  ICO_OUTIL_ACCEL,
  999,
  ICO_OUTIL_GLISSE,
  999,
  ICO_OUTIL_CAISSE,
  999,
  ICO_OUTIL_ARRIVEE,
  999,
  -1,

  0};

static short tabpalette0[] = {
  ICO_OUTIL_TRACKSBAR,
  10,
  -1,
  ICO_OUTIL_BARRIERE,
  10,
  -1,
  ICO_OUTIL_VISION,
  10,
  -1,
  ICO_OUTIL_BOIT,
  10,
  -1,
  0};

void
ChangeLanguage (short language)
{
  g_langue = language;
}

static void
LoadCursor ()
{
  char cursorFile[4096] = {0};
  snprintf (
    cursorFile, sizeof (cursorFile),
    "%s../share/blupimania/image/cursor.%s.png", SDL_GetBasePath (),
    g_theme == 0 ? "image" : "smaky");

  SDL_Surface * surface       = IMG_Load (cursorFile);
  SDL_Surface * surfaceScaled = SDL_CreateRGBSurfaceWithFormat (
    0, surface->w * g_zoom, surface->h * g_zoom, 32, SDL_PIXELFORMAT_RGBA32);
  SDL_BlitScaled (surface, NULL, surfaceScaled, NULL);
  SDL_FreeSurface (surface);

  if (g_cursor)
    SDL_FreeCursor (g_cursor);
  g_cursor = SDL_CreateColorCursor (surfaceScaled, 2 * g_zoom, 2 * g_zoom);
  SDL_FreeSurface (surfaceScaled);

  SDL_SetCursor (g_cursor);
}

void
ChangeScreen (short zoom)
{
  ++zoom;
  g_zoom = zoom;

  SDL_bool fullscreen = g_zoom == 3;

  SDL_SetWindowFullscreen (
    g_window, fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
  SDL_SetWindowBordered (g_window, fullscreen ? SDL_FALSE : SDL_TRUE);

  if (!fullscreen)
    SDL_SetWindowSize (g_window, LXIMAGE () * g_zoom, LYIMAGE () * g_zoom);

  SDL_RenderClear (g_renderer);
  SDL_RenderPresent (g_renderer);

  LoadCursor ();

  int displayIndex = SDL_GetWindowDisplayIndex (g_window);
  SDL_SetWindowPosition (
    g_window, SDL_WINDOWPOS_CENTERED_DISPLAY (displayIndex),
    SDL_WINDOWPOS_CENTERED_DISPLAY (displayIndex));
  SDL_Delay (100);
}

void
ChangeTheme (short theme)
{
  g_theme  = theme;
  g_colors = g_colorsTheme[g_theme];

  switch (theme)
  {
  case 0:
    g_afterglow = SDL_FALSE;
    break;
  case 1:
    g_afterglow = SDL_TRUE;
    break;
  }

  LoadCursor ();
}

/* ----------- */
/* PlayEvSound */
/* ----------- */

/*
    Fait entendre un bruitage seulement s'il n'y a pas déjà une musique en
   cours.
 */

void
PlayEvSound (short sound)
{
  // if ( musique != 0 )  return;		/* rien si musique en cours */
  PlayAudio (sound, NULL);
}

/* ------ */
/* PutNum */
/* ------ */

/*
    Met un nombre entier positif en base dix dans un buffer.
    Si nbdigits = 0, met juste le nombre nécessaire de digits.
 */

void
PutNum (char ** ppch, short num, short nbdigits)
{
  short i;
  short shift = 10000;
  short digit;
  char  put = 0;

  if (nbdigits > 0)
  {
    for (i = nbdigits; i > 0; i--)
    {
      *(*ppch + i - 1) = (num % 10) + '0';
      num /= 10;
    }
    *ppch += nbdigits;
  }
  else
  {
    for (i = 4; i >= 0; i--)
    {
      digit = (num / shift) % 10;
      shift /= 10;
      if (put == 1 || digit != 0 || i == 0)
      {
        *(*ppch)++ = digit + '0';
        put        = 1;
      }
    }
  }
  **ppch = 0;
}

/* --------- */
/* MondeEdit */
/* --------- */

/*
    Prépare un monde pour pouvoir l'éditer tranquillement, avec tous les
   outils.
 */

void
MondeEdit (void)
{
  short i = 0;

  do
  {
    descmonde.palette[i] = tabpalette[i];
  } while (tabpalette[i++] != 0);
}

/* --------- */
/* MondeVide */
/* --------- */

/*
    Initialise un monde entièrement vide.
 */

void
MondeVide (void)
{
  short x, y;
  short i = 0;

  memset (&descmonde, 0, sizeof (Monde));

  for (y = 0; y < MAXCELY; y++)
    for (x = 0; x < MAXCELX; x++)
      descmonde.tmonde[y][x] = tabmonde[y][x];

  descmonde.freq = 50;

  do
  {
    descmonde.palette[i] = tabpalette0[i];
  } while (tabpalette0[i++] != 0);
}

/* ------------ */
/* BanqueToFile */
/* ------------ */

/*
    Conversion d'une banque en fichier.
 */

static char
BanqueToFile (char banque)
{
  if (banque >= 'a' && banque <= 'h')
    return banque - 'a' + '1'; /* 1..8 */

  if (banque >= 'i' && banque <= 'l')
    return banque - 'i' + 'e'; /* e..h */

  return banque;
}

/* -------- */
/* MondeMax */
/* -------- */

/*
    Cherche le nombre maximum de mondes possibles.
 */

static void
MondeMax (char banque)
{
  maxmonde = FileGetLength (BanqueToFile (banque)) / sizeof (Monde);
  if (g_construit)
    maxmonde++; /* si construit -> toujours un monde vide à la fin */
}

static void
convshort (short * s)
{
  char   t;
  char * p = (char *) s;

  t    = p[0];
  p[0] = p[1];
  p[1] = t;
}

static void
ConvMonde (Monde * m)
{
  int i, j;

  for (i = 0; i < MAXCELX; i++)
    for (j = 0; j < MAXCELY; j++)
      convshort (&m->tmonde[j][i]);

  for (i = 0; i < MAXPALETTE; i++)
    convshort (&m->palette[i]);

  convshort (&m->freq);
  convshort (&m->color);
}

/* --------- */
/* MondeRead */
/* --------- */

/*
    Lit un nouveau monde sur le disque.
    Retourne 0 si la lecture est ok.
 */

static short
MondeRead (short monde, char banque)
{
  short err = 0;
  short max;

  if (g_construit)
    max = maxmonde - 1;
  else
    max = maxmonde;

  if (monde >= max)
    goto vide;

  err = FileRead (
    &descmonde, monde * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
  if (err)
  {
    maxmonde = 0;
    goto vide;
  }
  ConvMonde (&descmonde);
  return 0;

vide:
  MondeVide (); /* retourne un monde vide */
  return err;
}

/* ---------- */
/* MondeWrite */
/* ---------- */

/*
    Ecrit un monde sur le disque.
    Retourne 0 si l'�criture est ok.
 */

static short
MondeWrite (short monde, char banque)
{
  short err;

  ConvMonde (&descmonde);
  err = FileWrite (
    &descmonde, monde * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
  ConvMonde (&descmonde);
  MondeMax (banque);

  return err;
}

Style
GetWorldStyle ()
{
  return phase != PHASE_PLAY ? NORMAL : descmonde.color;
}

/* ---------- */
/* JoueurRead */
/* ---------- */

/*
    Lit le fichier des joueurs sur le disque.
    Retourne 0 si la lecture est ok.
 */

static short
JoueurRead (struct arguments * arguments)
{
  short err;

  memset (&fj, 0, sizeof (fj)); /* met tout à zéro */
  fj.vitesse = 1;               /* vitesse normale */

  err = FileRead (&fj, 0, sizeof (fj), 'z');
  if (err)
  {
    fj.noisevolume = 10 - 3;
    fj.musicvolume = 10 - 6;
    fj.screen      = 1; /* Use the double windowed zoom by default */
  }

  if (arguments)
  {
    if (g_settingsOverload & (SETTING_FULLSCREEN | SETTING_ZOOM))
      fj.screen = arguments->fullscreen ? 2 : arguments->zoom - 1;
    if (g_settingsOverload & SETTING_SPEEDRATE)
      fj.vitesse = arguments->speedrate;
    if (g_settingsOverload & SETTING_THEME)
      fj.theme = !SDL_strcmp (arguments->theme, "smaky100") ? 1 : 0;
  }

  g_modetelecom = fj.modetelecom;

  ChangeLanguage (fj.language);
  ChangeScreen (fj.screen);
  ChangeTheme (fj.theme);

  PlayNoiseVolume (fj.noisevolume);
  PlayMusicVolume (fj.musicvolume);
  return err;
}

/* ----------- */
/* JoueurWrite */
/* ----------- */

/*
    Ecrit le fichier des joueurs sur le disque.
    Retourne 0 si l'écriture est ok.
 */

static short
JoueurWrite (void)
{
  short  err;
  Joueur diskFj, saveFj;

  SDL_memset (&diskFj, 0, sizeof (diskFj));
  diskFj.vitesse = 1;
  SDL_memcpy (&saveFj, &fj, sizeof (saveFj));

  err = FileRead (&diskFj, 0, sizeof (diskFj), 'z');
  if (!err)
  {
    if (g_settingsOverload & (SETTING_FULLSCREEN | SETTING_ZOOM))
      saveFj.screen = diskFj.screen;
    if (g_settingsOverload & SETTING_SPEEDRATE)
      saveFj.vitesse = diskFj.vitesse;
    if (g_settingsOverload & SETTING_THEME)
      saveFj.theme = diskFj.theme;
  }

  err = FileWrite (&saveFj, 0, sizeof (saveFj), 'z');
  return err;
}

/* ------------------- */
/* ConvPhaseToNumImage */
/* ------------------- */

/*
    Retourne le numéro d'image correspondant à une phase de jeu.
 */

static short
ConvPhaseToNumImage (Phase ph)
{
  switch (ph)
  {
  case PHASE_PLAY:
    return 20;
  case PHASE_INIT:
    return 21;
  case PHASE_OBJECTIF:
    return 22;
  case PHASE_RECOMMENCE:
    return 23;
  case PHASE_SUIVANT:
    return 24;
  case PHASE_PRIVE:
    return 25;
  case PHASE_PARAM:
    return 26;
  case PHASE_DEPLACE:
    return 27;
  case PHASE_AIDE21:
    return 28;
  case PHASE_ATTENTE:
    return 29;
  case PHASE_OPER:
    return 30;
  case PHASE_IDENT:
    return 31;
  case PHASE_AIDE22:
    return 32;
  case PHASE_FINI0:
    return 33;
  case PHASE_FINI1:
    return 33;
  case PHASE_FINI2:
    return 33;
  case PHASE_FINI3:
    return 33;
  case PHASE_FINI4:
    return 33;
  case PHASE_FINI5:
    return 33;
  case PHASE_FINI6:
    return 33;
  case PHASE_FINI7:
    return 33;
  case PHASE_FINI8:
    return 33;
  case PHASE_AIDE23:
    return 34;
  case PHASE_GENERIC:
    return 36;
  case PHASE_REGLAGE:
    return 37;
  case PHASE_AIDE31:
    return 40;
  case PHASE_AIDE32:
    return 41;
  case PHASE_AIDE33:
    return 42;
  case PHASE_AIDE34:
    return 43;
  case PHASE_AIDE35:
    return 44;
  case PHASE_AIDE36:
    return 45;
  case PHASE_AIDE:
    return 46;
  case PHASE_AIDE41:
    return 47;
  case PHASE_AIDE42:
    return 48;
  case PHASE_AIDE24:
    return 49;
  case PHASE_AIDE11:
    return 50;
  case PHASE_AIDE12:
    return 51;
  case PHASE_AIDE13:
    return 52;
  case PHASE_REGLAGE2:
    return 53;
  case PHASE_GOODBYE:
    return 54;
  }
  return -1;
}

/* Codage des lettres accentuées */
/* ----------------------------- */

/*	272		"a" aigu		*/
/*	271		"a" grave		*/
/*	270		"a" circonflêxe	*/
/*	267		"a" trêma		*/
/*	266		"e" aigu		*/
/*	265		"e" grave		*/
/*	264		"e" circonflêxe	*/
/*	263		"e" trêma		*/
/*	262		"i" aigu		*/
/*	261		"i" grave		*/
/*	260		"i" circonflêxe	*/
/*	257		"i" trêma		*/
/*	256		"o" aigu		*/
/*	255		"o" grave		*/
/*	254		"o" circonflêxe	*/
/*	253		"o" trêma		*/
/*	252		"u" aigu		*/
/*	251		"u" grave		*/
/*	250		"u" circonflêxe	*/
/*	247		"u" trêma		*/
/*	246		"c" cédille		*/
/*	245		"C" cédille		*/

/* --------- */
/* ShowImage */
/* --------- */

/*
    Affiche une image de base dans la fenêtre.
 */

static void
ShowImage (void)
{
  Rect         rect;
  const char * ptx;
  short        image, err, nbessai, max;

  static const char * txrecommence_en[] = {
    "\001\002Oops! It looks as though you'll have to try again!",
    "\001\002Bad luck!\nGive it another go...",
    "\001\002Looked good...\n...but not good enough!",
    "\001\002You'll need another try, I'm afraid...",
    "\003\004Sorry, no good! Maybe you'll\nmake it next time.",
    "\003\004Well, it could have been right... Go for more!",
    "\003\004No, no, no!\nHow about another try?",
    "\003\004Hum... not good enough!",
    "\003\004It may not look like it,\nbut it's not that easy!",
    "\005\006Well, well, well... It seems you'll need to try again!",
    "\005\006Mind twisting, isn't it?\nGive it another go...",
    "\005\006Boggles the mind, doesn't it?",
    "\007\012You can't be far...\nKeep going!",
    "\007\012Tough luck! Take a deep breath and go for more!",
    "\007\012Who made that one up?\nKeep going...",
    "\013\144Don't you feel like screaming? Keep it up...",
    "\013\144Why don't you sleep on it, and try again tomorrow?",
    "\013\144Patience, my foot! I'm gonna scream!",
    "\013\144Take a deep breath... and try again!"};

  static const char * txrecommence_fr[] = {
    "\001\002D\266sol\266, mais c'est rat\266 !\nEssaye encore une fois ...",
    "\001\002Eh non, ce n'est pas r\266ussi, mais il faut pers\266v\266rer !",
    "\001\002C'est rat\266 !\nLa prochaine sera la bonne.",
    "\001\002Mais non, pas comme \246a\nA refaire ...",
    "\001\002Zut, c'est pas \246a !\nEssaye une autre tactique ...",
    "\001\002Eh bien non, ce n'est pas la bonne solution !",
    "\001\002Non non, pas comme \246a !\nTrouve une autre id\266e ...",
    "\003\005Eh non, toujours pas !\nIl faut ruser un peu plus ...",
    "\003\005Caramba, encore rat\266 !\nTrouve autre chose ...",
    "\003\005Toujours et encore \271 c\254t\266 de la plaque ...",
    "\003\005Non, non et re-non !\nPers\266v\265re ...",
    "\003\005D'autres que toi ont aussi transpir\266 sur cette \266nigme ...",
    "\006\012En bien, ce n'est pas ton jour !",
    "\006\012Vraiment difficile, n'est-il pas ?",
    "\006\012Cool mec, restons calme, la solution viendra ...",
    "\006\012Ne d\266sesp\265re pas, tu finiras bien par trouver ...",
    "\013\144D\266cid\266ment, cela ira peut-\264tre mieux demain ?",
    "\013\144Laisse tomber et reprends ce jeu dans un mois !",
    "\013\144Empoigne le probl\265me par un autre bout ?"};

  static const char * txrecommence_de[] = {
    "\001\002Tut mir leid, aber Du hast es verpatzt ! Nochmal ...",
    "\001\002Nein, das war falsch ! Das Ganze nochmal !",
    "\001\002Schon wider falsch ! N\267chtes mal mach es besser !",
    "\001\002Nein, doch nicht so ! Mach es nochmal ...",
    "\001\002Verflixt, das war schon wieder nichts !",
    "\001\002Tja, das war ja wohl wieder nichts !",
    "\001\002Nein, nein so geht das nicht ! Finde eine andere Idee ...",
    "\003\005Ah, immer noch nicht, ber-\nlege ein bisschen mehr ...",
    "\003\005Verflixt, schon wieder verpatzt !",
    "\003\005Das is ja schon wieder in die Hosen gegangen !",
    "\003\005Nein, nein und nochmals nein. Probier weiter ...",
    "\003\005Du bist nicht der erste, der sich hier die Z\267hne ausbeisst",
    "\006\012Heute ist wohl nicht Dein Tag, oder ?",
    "\006\012Ganz sch\253n schwer, was ?",
    "\006\012Keine Aufregung, die L\253sung wird schon kommen.",
    "\006\012Nicht verzweifeln, Du wirst schon noch draufkommen.",
    "\013\144Vielleicht versuchst Du's morgen nochmal ?",
    "\013\144Gehe das Problem ganz anders an !"};

  static const char * const * txrecommence[] = {
    txrecommence_en, txrecommence_fr, txrecommence_de};
  static const size_t txrecommence_size[] = {
    countof (txrecommence_en), countof (txrecommence_fr),
    countof (txrecommence_de)};

  static const char * txsuivant_en[] = {
    "\001\001Top marks!",
    "\001\001Ten out of ten!",
    "\001\001Amazing!",
    "\001\001Perfect!",
    "\001\001There's only one word : YES!",
    "\001\001You're pretty good at this!",
    "\001\001Can't do better!",
    "\001\001You can't beat that!",
    "\001\001Spot on!",
    "\001\001Bulls eye!\nGo for more!",
    "\001\001BINGO!",
    "\002\003That's better...!\nTry the next one...",
    "\002\003It wasn't that bad, was it?",
    "\002\003Well done!\nMove on to the next one...",
    "\002\003Very good!\nBut the next one may be harder!",
    "\002\003Good shot!\nLet's see what you'll make of the next one...",
    "\002\003Excellent!\nNow what about the next one?",
    "\002\003Yep! That's it...\n Let's see how you manage the next step...",
    "\002\003Great stuff!\nKeep it up!",
    "\004\006Okay, that's it!",
    "\004\006Feels good to be through!",
    "\004\006That one's over,\nlet's try the next one...",
    "\004\006Okay!\nHow about the next one?",
    "\004\006It's not quite as easy as it looks, is it..?",
    "\004\006Whew! That one's done...\nLet's go for more!",
    "\004\006I could feel the heat!\nBut it's over...",
    "\004\006Difficult, but not impossible!\nTry more!",
    "\007\012Quite a sweat, isn't it?\nKeep it up!",
    "\007\012I was sure you'd make it!\nMove up one!",
    "\007\012All right, it was hard,\nbut you made it!",
    "\007\012Through at last!\nKeep going!",
    "\007\012Made it just in time to try the next one...",
    "\007\012Well, it was about time...\nGo for more!",
    "\007\012How about that, you made it!\nWhat's the next one like?",
    "\013\144Don't worry!\nThe next one might be even harder!",
    "\013\144Rats! That one gave me a headache...",
    "\013\144Oh well, you can't be the best every time..!",
    "\013\144Good job you didn't give up!",
    "\013\144Never lose hope!\nNext time will be better!"};

  static const char * txsuivant_fr[] = {
    "\001\001Super extra chouette, c'est r\266ussi du premier coup !",
    "\001\001Bravo champion, z\266ro faute ...",
    "\001\001Extra, la perfection quoi !",
    "\001\001Super extra hyper m\266ga,\neuh ... tr\265s bien, quoi !",
    "\001\001En un mot comme en mille:\nB-R-A-V-O, bravo, bravo ...",
    "\002\003Bravo, super, c'est r\266ussi !",
    "\002\003Champion, c'est juste !",
    "\002\003Parfait, tu peux passer \271 l'\266nigme suivante ...",
    "\002\003Tr\265s bien, mais la prochaine \266nigme sera peut-\264tre "
    "beaucoup plus difficile !",
    "\002\003Youpie, c'est tout juste !",
    "\004\006Ouaip, c'est dans la poche !",
    "\004\006\245a va, tu peux passer \271 l'\266nigme suivante ...",
    "\004\006Correct, passe plus loin.",
    "\004\006OK. (point \271 la ligne)",
    "\007\012\245a ira pour cette fois, mais c'\266tait dur dur, non ?",
    "\007\012Bon, \246a passe pour cette fois, mais t\270che d'y arriver plus "
    "vite la prochaine fois !",
    "\007\012Sans commentaire, \246a vaut mieux ...",
    "\013\144Ouf, c'est enfin r\266ussi. Bel effort ...",
    "\013\144Bravo, que d'efforts pour en arriver l\271 !",
    "\013\144C'est le moment ...\nEsp\266rons que l'\266nigme suivante sera "
    "plus facile ... mais rien n'est moins s\250r !"};

  static const char * txsuivant_de[] = {
    "\001\001Echt Super, gleich beim erstenmal geschafft !",
    "\001\001Bravo Champion, null Fehler ...",
    "\001\001Wirklich perfekt !",
    "\001\001Das war echt Spitze !",
    "\001\001Phantastisch wie du das machst !",
    "\002\003Bravo, super, Du hast es geschafft !",
    "\002\003Richtig, Meister !",
    "\002\003Einwandfrei, Du kannst zum n\267chsten R\267tsel gehen ...",
    "\002\003Sehr gut, aber das n\267chste R\267tsel ist vielleicht viel "
    "schwieriger !",
    "\002\003Juppie, alles richtig !",
    "\004\006Geschafft !",
    "\004\006Ganz gut, und nun auf zum n\267chsten R\267tsel.",
    "\004\006Richtig. Weiter geht's.",
    "\004\006OK. Auf zum n\267chsten R\267tsel.",
    "\007\012Mittelpr\267chtig.",
    "\007\012OK, Du hast es geschafft, aber beeil Dich beim n\267chsten mal",
    "\007\012Ohne Kommentar, es ist besser so ...",
    "\013\144Uff, endlich geschafft.",
    "\013\144Bravo, na endlich hast du's geschafft !",
    "\013\144Es wurde aber auch langsam Zeit. Hoffentlich ist das n\267chste "
    "R\267tsel einfacher.",
  };

  static const char * const * txsuivant[] = {
    txsuivant_en, txsuivant_fr, txsuivant_de};
  static const size_t txsuivant_size[] = {
    countof (txsuivant_en), countof (txsuivant_fr), countof (txsuivant_de)};

  static const char * txfini_en[] = {
    "Well done, you've finished the first part of level 1.\nNow use the "
    "radio-controls to steer BLUPI through the second part...",
    "Great going, level 1 is done.\nNow try level 2...",

    "Well, that's it for the first part of level 2.\nHave you tried to "
    "radio-control BLUPI through this level?",
    "Okay, you've steered BLUPI through level 2.\nDo you think you can handle "
    "level 3?",

    "Excellent, you've finished the first part of level 3!\nTake a look at the "
    "second part of this level...",
    "Excellent, you've got through level 3.\nBut wait till you try level 4!",

    "It seems you're a winner...\nBut the second part (with the "
    "radio-controlled BLUPI) is no joke..!",
    "Hooray! This game has no secrets for you...\nBut now you can draw your "
    "own puzzles in level 5.",

    "Why don't you build a few even harder worlds... and send them to other "
    "BLUPIMANIACS?"};

  static const char * txfini_fr[] = {
    "Bravo, tu as termin\266 la premi\265re partie du niveau\0011.\nEssaye "
    "maintenant la deuxi\265me partie, en t\266l\266commandant BLUPI\001...",
    "Bravo, le niveau 1 est termin\266.\nEssaye maintenant le "
    "niveau\0012\001...",

    "Bravo, tu as termin\266 le niveau\0012, lorsque BLUPI est "
    "autonome.\nT\266l\266commande maintenant BLUPI dans la deuxi\265me "
    "partie\001...",
    "Bravo, tu as termin\266 le niveau\0012.\nPenses-tu pouvoir r\266soudre le "
    "niveau\0013 (c'est dur dur)\001?",

    "Formidable, tu as termin\266 la premi\265re partie du "
    "niveau\0013\001!\nAttaque maintenant la deuxi\265me partie de ce "
    "niveau\001...",
    "Formidable, tu as termin\266 le niveau\0013\001!\nIl reste le "
    "niveau\0014, mais attention, c'est du b\266ton\001...",

    "Hyper extra m\266ga chouette !\nMais attention, la deuxi\265me partie "
    "(avec BLUPI t\266l\266command\266) n'est pas franchement facile\001...",
    "Hyper extra m\266ga chouette !\nCe jeu n'a plus de secrets pour toi. "
    "Heureusement, tu peux encore dessiner tes propres \266nigmes, pour tes "
    "copains (niveau\0015)\001...",

    "Tr\265s bien, tu as termin\266 le niveau\0015.\nEssaye encore de dessiner "
    "d'autres \266nigmes plus difficiles\001..."};

  static const char * txfini_de[] = {
    "Bravo, Du hast den ersten Teil der Stufe eins beendet.\nVersuche jetzt "
    "den zweiten Teil, wo Du Blupi fernsteuern kannst ...",
    "Bravo, Du hast Stufe eins geschafft. Versuche nun Stufe zwei ...",

    "Bravo, Du hast Stufe zwei mit dem automatischen BLUPI "
    "geschafft.\nVersuche nun Stufe zwei mit dem ferngesteuerten BLUPI !",
    "Bravo, Stufe zwei ist geschafft. Glaubst Du,\nDu schaffst auch Stufe drei "
    "?",

    "Phantastisch, Du hast den ersten Teil der Stufe drei geschafft.\nNimm "
    "jetzt den zweiten Teil der Stufe drei in Angriff !",
    "Super, Du hast Stufe drei beendet, aber es bleibt Dir noch Stufe vier, da "
    "wird's echt hart.",

    "Bravo, super, phantastisch, aber Achtung, der zweite Teil mit dem "
    "ferngesteuerten Blupi ist nicht gerade einfach !",
    "Kolossal, Du hast alle R\267tsel in diesesm Spiel gel\253st. Zumindest "
    "gelesen. Zudem Gl\247ck kannst Du auch Deine eigenen R\267tsel "
    "konstruieren (Stufe 5).",

    "Ausgezeichnet ! Konstruiere nun neue, noch schwierigere R\267tsel."};

  static const char * const * txfini[] = {txfini_en, txfini_fr, txfini_de};

  if (phase != PHASE_GENERIC)
    BlackScreen (); /* efface tout l'écran */

  image = ConvPhaseToNumImage (phase);

  err        = GetImage (&pmimage, image, descmonde.color);
  pmimageNum = image;
  if (err)
    FatalBreak (err); /* erreur fatale */

  nbessai = retry + 1;
  if (nbessai > 100)
    nbessai = 100;

  if (phase == PHASE_RECOMMENCE)
  {
    max = 0;
    do
    {
      ptx = txrecommence[g_langue][GetRandomEx (
        1, 0, txrecommence_size[g_langue], randomexrecommence)];
      max++;
    } while ((nbessai < ptx[0] || nbessai > ptx[1]) && max < 100);
    rect.p1.x = 113;
    rect.p1.y = LYIMAGE () - 319;
    rect.p2.x = 113 + 446;
    rect.p2.y = LYIMAGE () - 319 + 72;
    DrawParagraph (&pmimage, rect, ptx + 2, TEXTSIZEMID);
  }

  if (phase == PHASE_SUIVANT)
  {
    max = 0;
    do
    {
      ptx = txsuivant[g_langue][GetRandomEx (
        1, 0, txsuivant_size[g_langue], randomexsuivant)];
      max++;
    } while ((nbessai < ptx[0] || nbessai > ptx[1]) && max < 100);
    rect.p1.x = 85;
    rect.p1.y = LYIMAGE () - 275;
    rect.p2.x = 85 + 470;
    rect.p2.y = LYIMAGE () - 275 + 163;
    DrawParagraph (&pmimage, rect, ptx + 2, TEXTSIZEMID);
  }

  if (phase >= PHASE_FINI0 && phase <= PHASE_FINI8)
  {
    ptx       = txfini[g_langue][phase - PHASE_FINI0];
    rect.p1.x = 85;
    rect.p1.y = LYIMAGE () - 266;
    rect.p2.x = 85 + 470;
    rect.p2.y = LYIMAGE () - 266 + 190;
    DrawParagraph (&pmimage, rect, ptx, TEXTSIZEMID);
  }

  Pt dim  = {pmimage.dy, pmimage.dx};
  Pt orig = {0, 0};
  CopyPixel /* affiche l'image de base */
    (&pmimage, orig, 0, orig, dim);

  if (phase == PHASE_PLAY)
  {
    GivePixmap (&pmimage); /* libère l'image si jeu */
    pmimageNum = -1;
  }
  else
    AnimDrawInit (); /* affiche les animations au départ */
}

/* ------------- */
/* ChangeCouleur */
/* ------------- */

/*
    Change les couleurs de la palette pendant le jeu.
 */

void
ChangeCouleur (void)
{
  if (!IfColor ())
    return;

  LoadSprites (descmonde.color);
}

void
DrawSprite (short num, Pt p1, Pt p2, Pt dim)
{
  Pixmap pm = {0};
  GetSprite (&pm, num, 1);
  CopyPixel (&pm, p1, 0, p2, dim);
}

static void
DrawSpriteTemp (short num, Pt p1, Pt p2, Pt dim)
{
  Pixmap pm = {0};
  GetSprite (&pm, num, 1);
  CopyPixel (&pm, p1, &pmtemp, p2, dim);
}

/* --------------- */
/* DrawRadioButton */
/* --------------- */

/*
    Dessine un bouton rond relâché ou enfoncé.
 */

void
DrawRadioButton (Pt pos, short state)
{
  short icon;
  Pt    src, dim;

  src.x = 0;
  src.y = 0;

  dim.x = 31;
  dim.y = 31;

  if (state)
    icon = ICO_BUTTON_ROND1;
  else
    icon = ICO_BUTTON_ROND0;

  DrawSprite (icon, src, pos, dim); /* dessine le bouton */
}

/* ---------- */
/* DrawJoueur */
/* ---------- */

/*
    Dessine le numéro du joueur en enfonçant un bouton rond.
 */

void
DrawJoueur (void)
{
  short i;
  Pt    pos;

  pos.x = 241;
  pos.y = LYIMAGE () - 297 - 1;

  for (i = 0; i < MAXJOUEUR; i++)
  {
    if (fj.joueur == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.y += 40;
  }
}

/* ----------- */
/* DrawVitesse */
/* ----------- */

/*
    Dessine la vitesse en enfonçant un bouton rond.
 */

void
DrawVitesse (void)
{
  short i;
  Pt    pos;

  pos.x = 31;
  pos.y = LYIMAGE () - 292 - 1;

  for (i = 0; i < 3; i++)
  {
    if (fj.vitesse == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.y += 32;
  }
}

/* ---------- */
/* DrawScroll */
/* ---------- */

/*
    Dessine le mode de scroll en enfonçant un bouton rond.
 */

void
DrawScroll (void)
{
  short i;
  Pt    pos;

  pos.x = 272;
  pos.y = LYIMAGE () - 292 - 1;

  for (i = 0; i < 2; i++)
  {
    if (fj.scroll == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.y += 32;
  }
}

/* ---------- */
/* DrawVolume */
/* ---------- */

/*
    Dessine le contenu d'un potentiomètre pour bruitage.
 */

void
DrawVolume (short pot, short volume)
{
  Rect rect;

  if (pot == 0)
  {
    rect.p1.x = 21 + 26;
    rect.p2.x = 21 + 26 + 4;
  }
  else
  {
    rect.p1.x = 21 + 40 + 16 + 10;
    rect.p2.x = 21 + 40 + 16 + 10 + 4;
  }

  rect.p1.y = LYIMAGE () - 135 - 1 + 3;
  rect.p2.y = LYIMAGE () - 135 - 1 + 3 + ((10 - volume) * 50 / 10);

  DrawFillRect (0, rect, COLORBLANC);

  rect.p1.y = rect.p2.y;
  rect.p2.y = LYIMAGE () - 135 - 1 + 3 + 50;

  DrawFillRect (0, rect, COLORROUGE);
}

/* ------------ */
/* DrawBruitage */
/* ------------ */

/*
    Dessine le mode de bruitage en enfonçant un bouton rond.
 */

void
DrawBruitage ()
{
  DrawVolume (0, fj.noisevolume);
  DrawVolume (1, fj.musicvolume);
}

/* ----------- */
/* DrawTelecom */
/* ----------- */

/*
    Dessine le mode de télécommande en enfonçant un bouton rond.
 */

void
DrawTelecom (void)
{
  short i;
  Pt    pos;

  pos.x = 272;
  pos.y = LYIMAGE () - 172 - 1;

  for (i = 0; i < 2; i++)
  {
    if (fj.modetelecom == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.y += 32;
  }
}

/* ------------ */
/* DrawLanguage */
/* ------------ */

/*
    Draw the language selector
 */

void
DrawLanguage (void)
{
  short i;
  Pt    pos;

  pos.x = 31;
  pos.y = LYIMAGE () - 292 - 1;

  for (i = 0; i < 3; i++)
  {
    if (fj.language == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.y += 32;
  }
}

/* ---------- */
/* DrawScreen */
/* ---------- */

/*
    Draw the screen settings
 */

void
DrawScreen (void)
{
  short i;
  Pt    pos;

  pos.x = 272;
  pos.y = LYIMAGE () - 292 - 1;

  for (i = 0; i < 3; i++)
  {
    if (fj.screen == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.y += 32;
  }
}

/* ---------- */
/* DrawTheme  */
/* ---------- */

/*
    Draw the theme selection
 */

void
DrawTheme (void)
{
  short i;
  Pt    pos;

  pos.x = 272;
  pos.y = 202;

  for (i = 0; i < 2; i++)
  {
    if (fj.theme == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.y += 32;
  }
}

/* ----------- */
/* DrawCouleur */
/* ----------- */

/*
    Dessine le mode de couleur en enfonçant un bouton rond.
 */

void
DrawCouleur (void)
{
  short i;
  Pt    pos;

  pos.x = 146;
  pos.y = LYIMAGE () - 101 - 1;

  for (i = 0; i < 5; i++)
  {
    if (descmonde.color == i)
      DrawRadioButton (pos, 1);
    else
      DrawRadioButton (pos, 0);
    pos.x += 16 * 6;
  }
}

/* ---------- */
/* DrawArrows */
/* ---------- */

/*
    Affiche les 4 flèches ou la télécommande.
 */

void
DrawArrows (char mode)
{
  short icon;
  Pt    src, dst, dim;
  Rect  rect;

  if (g_typejeu == 0 || g_pause)
    icon = ICO_ARROWS;
  else
    icon = ICO_TELECOM;

  src.x = 0;
  src.y = 0;

  dst.x = 7;
  dst.y = LYIMAGE () - 92 - 1;

  dim.x = 54;
  dim.y = 52;

  DrawSprite (icon, src, dst, dim); /* dessine flèches ou télécommande */

  if (icon == ICO_TELECOM)
  {
    dim.x = 16;
    dim.y = 16;

    dst.x = 7 + 9;
    dst.y = LYIMAGE () - 92 - 1 + 26;
    src.x = 0;
    src.y = 52;
    if (mode == KEYGOFRONT)
      src.x = 15;
    if (mode == KEYGOBACK)
      src.x = 30;
    DrawSprite (icon, src, dst, dim); /* dessine la manette avant/arrière */

    dst.x = 7 + 29;
    dst.y = LYIMAGE () - 92 - 1 + 26;
    src.x = 54;
    src.y = 0;
    if (mode == KEYGOLEFT)
      src.y = 15;
    if (mode == KEYGORIGHT)
      src.y = 30;
    DrawSprite (icon, src, dst, dim); /* dessine la manette gauche/droite */
  }

  if (g_typeedit)
  {
    rect.p1.x = 26 - 1;
    rect.p1.y = LYIMAGE () - 1 - 28 - 2;
    rect.p2.x = 26 + 36;
    rect.p2.y = LYIMAGE () - 1 - 28 + 18 + 2;
    DrawFillRect (0, rect, COLORBLANC); /* efface pause + disquette */
  }
}

/* --------- */
/* DrawPause */
/* --------- */

/*
    Affiche le bouton pause.
 */

void
DrawPause (void)
{
  Pt src, dst, dim;

  if (g_typeedit)
    return;

  if (g_pause)
    src.x = 20;
  else
    src.x = 0;
  src.y = 0;

  dst.x = 26;
  dst.y = LYIMAGE () - 1 - 28;

  dim.x = 18;
  dim.y = 18;

  DrawSprite (ICO_BUTTON_PAUSE, src, dst, dim);
}

/* ------------ */
/* DrawBigDigit */
/* ------------ */

/*
    Affiche un gros chiffre sur l'écran.
        pos		->	coin sup/gauche
        num		->	chiffre 0..9
 */

void
DrawBigDigit (Pt pos, short num)
{
  Pt src, dim;

  src.x = (num % 4) * 20;
  src.y = (num / 4) * 26;

  dim.x = 20;
  dim.y = 26;

  DrawSprite (ICO_CHAR_BIG, src, pos, dim);
}

/* ---------- */
/* DrawDigNum */
/* ---------- */

/*
    Affiche un gros nombre compris entre 0 et 99.
 */

void
DrawBigNum (Pt pos, short num)
{
  DrawBigDigit (pos, 10); /* efface les dizaines */
  pos.x += 20;
  DrawBigDigit (pos, 10); /* efface les unités */
  pos.x -= 20;

  if (num > 99)
    num = 99;

  if (num < 10)
  {
    pos.x += 20 / 2;
    DrawBigDigit (pos, num); /* affiche les unités au milieu */
  }
  else
  {
    DrawBigDigit (pos, num / 10); /* affiche les dizaines */
    pos.x += 20;
    DrawBigDigit (pos, num % 10); /* affiche les unités */
  }
}

/* ------------ */
/* DrawObjectif */
/* ------------ */

/*
    Affiche l'objectif du jeu.
 */

static void
DrawObjectif (void)
{
  Rect                rect;
  const char *        ptext     = descmonde.text;
  static const char * tomake[3] = {
    "Puzzle to build ...",        //
    "Enigme \271 construire ...", //
    "R\267tsel zum bauen ..."};

  switch (phase)
  {
  case PHASE_RECOMMENCE:
    rect.p1.x = 130;
    rect.p1.y = LYIMAGE () - 230;
    rect.p2.x = 130 + 419;
    rect.p2.y = LYIMAGE () - 230 + 102;
    break;
  case PHASE_DEPLACE:
    rect.p1.x = 415;
    rect.p1.y = LYIMAGE () - 160;
    rect.p2.x = 415 + 181;
    rect.p2.y = LYIMAGE () - 160 + 63;
    break;
  default:
    rect.p1.x = 49;
    rect.p1.y = LYIMAGE () - 254;
    rect.p2.x = 49 + 343;
    rect.p2.y = LYIMAGE () - 254 + 130;
  }

  if (g_construit && g_monde == maxmonde - 1)
  {
    ptext = tomake[g_langue];
  }

  DrawFillRect (0, rect, COLORBLANC);          /* efface le rectangle */
  DrawParagraph (0, rect, ptext, TEXTSIZELIT); /* affiche la consigne */
}

/* ------------- */
/* RectStatusBar */
/* ------------- */

/*
    Retourne le rectangle à utiliser pour la barre d'avance.
 */

static void
RectStatusBar (Rect * prect)
{
  switch (phase)
  {
  case PHASE_OBJECTIF:
  case PHASE_PRIVE:
    prect->p1.x = 488;
    prect->p1.y = LYIMAGE () - 168;
    prect->p2.x = 488 + 113;
    prect->p2.y = LYIMAGE () - 168 + 12;
    break;
  case PHASE_ATTENTE:
    prect->p1.x = 170;
    prect->p1.y = LYIMAGE () - 113;
    prect->p2.x = 170 + 309;
    prect->p2.y = LYIMAGE () - 113 + 12;
    break;
  default:
    return;
  }
}

/* ------------- */
/* DrawStatusBar */
/* ------------- */

/*
    Affiche une barre d'avance en %.
 */

static void
DrawStatusBar (short avance, short max)
{
  short pos;
  Rect  rect, partLeft, partRight;
  Pt    pgra;
  char  lcolor, rcolor;
  char  chaine[6];

  if (max != 0)
    pos = (avance * 100) / max;
  else
    pos = 0;

  if (pos < 0)
    pos = 0;
  if (pos > 100)
    pos = 100;

  RectStatusBar (&rect);

  if (IfColor ())
  {
    lcolor = COLORVERTC;
    rcolor = COLORROUGE;
  }
  else
  {
    lcolor = COLORNOIR;
    rcolor = COLORBLANC;
  }

  partLeft      = rect;
  partLeft.p2.x = partLeft.p1.x + ((partLeft.p2.x - partLeft.p1.x) * pos) / 100;
  DrawFillRect (0, partLeft, lcolor); /* dessine le rectangle gauche */

  partRight      = partLeft;
  partRight.p1.x = partRight.p2.x;
  partRight.p2.x = rect.p2.x;
  DrawFillRect (0, partRight, rcolor); /* dessine le rectangle droite */

  pgra.x = (rect.p2.x + rect.p1.x) / 2;
  pgra.y = rect.p1.y + TEXTSIZELIT + 1;

  if (pos < 10)
  {
    pgra.x -= 7;
    chaine[0] = pos + '0'; /* unités */
    chaine[1] = '%';
    chaine[2] = 0;
  }
  else
  {
    if (pos < 100)
    {
      pgra.x -= 10;
      chaine[0] = pos / 10 + '0'; /* dizaines */
      chaine[1] = pos % 10 + '0'; /* unités */
      chaine[2] = '%';
      chaine[3] = 0;
    }
    else
    {
      pgra.x -= 14;
      chaine[0] = '1';
      chaine[1] = '0';
      chaine[2] = '0';
      chaine[3] = '%';
      chaine[4] = 0;
    }
  }

  DrawPercent (0, pgra, chaine, &partLeft, &partRight);
}

/* --------------- */
/* DetectStatusBar */
/* --------------- */

/*
    Détecte le monde à atteindre selon la position de la souris.
 */

static short
DetectStatusBar (Pt pos, short max, Rect * prect)
{
  short monde, progres;

  if (max == 0)
    return 0;

  monde = ((pos.x - prect->p1.x) * max) / (prect->p2.x - prect->p1.x);
  if (monde < 0)
    monde = 0;
  if (monde > max - 1)
    monde = max - 1;

  progres = fj.progres[fj.joueur][fj.niveau[fj.joueur]];
  if (!g_construit && monde > progres)
    monde = progres;

  return monde;
}

/* ------------ */
/* DrawNumMonde */
/* ------------ */

/*
    Affiche le numéro du monde actuel.
 */

static void
DrawNumMonde (void)
{
  Pt pos, src, dim;

  pos.x = 557;
  pos.y = LYIMAGE () - 249;

  DrawBigNum (pos, g_monde + 1); /* dessine le numéro du monde */

  src.x = 0;
  src.y = 0;

  dim.x = 58;
  dim.y = 50;

  pos.x = 478, pos.y = LYIMAGE () - 283 - 1;

  /* dessine la flèche supérieure (+) */
  if (
    g_monde < maxmonde - 1 &&
    (g_construit || g_monde < fj.progres[fj.joueur][fj.niveau[fj.joueur]]))
    DrawSprite (ICO_ARROWUP + 1, src, pos, dim);
  else
    DrawSprite (ICO_ARROWUP, src, pos, dim);

  pos.y = LYIMAGE () - 230 - 1;

  /* dessine la flèche inférieure (-) */
  if (g_monde > 0)
    DrawSprite (ICO_ARROWDOWN + 1, src, pos, dim);
  else
    DrawSprite (ICO_ARROWDOWN, src, pos, dim);

  if (phase == PHASE_DEPLACE)
    return;

  DrawStatusBar (g_monde, maxmonde - 1); /* dessine la barre d'avance */
}

static void
DrawUpdate (const char * version, Pt pos)
{
  char                text[256] = {0};
  static const char * format[3] = {
    "New version available for download on blupi.org (v%s)",
    "Une nouvelle version est disponible sur blupi.org (v%s)",
    "Neue Version (v%s) auf blupi.org zum Download verf\247gbar"};

  snprintf (text, sizeof (text), format[g_langue], version);

  DrawString (0, pos, text, TEXTSIZELIT, SDL_FALSE);
}

/* ----------------- */
/* TrackingStatusBar */
/* ----------------- */

/*
    Choix d'un monde tant que la souris est pressée.
 */

static void
TrackingStatusBar (Pt pos)
{
  Rect  rect     = {0};
  short newmonde = g_monde;

  RectStatusBar (&rect);

  newmonde = DetectStatusBar (pos, maxmonde, &rect);
  if (newmonde == g_monde)
    return;

  PlayEvSound (SOUND_CLIC);
  g_monde = newmonde;
  DrawNumMonde ();             /* affiche le numéro du monde */
  MondeRead (g_monde, banque); /* lit le nouveau monde sur disque */
  DrawObjectif ();             /* affiche l'objectif */
}

/* ------------ */
/* MondeDeplace */
/* ------------ */

/*
    Déplace un monde dans un autre.
    Retourne 0 si tout est ok.
 */

static short
MondeDeplace (short src, short dst)
{
  short i;
  Monde first, temp;

  if (src == dst || src == dst - 1)
    return 1;

  if (src < dst)
  {
    FileRead (
      &first, src * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));

    for (i = src + 1; i < dst; i++)
    {
      DrawStatusBar (i - (src + 1), dst - (src + 1));
      FileRead (
        &temp, i * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
      FileWrite (
        &temp, (i - 1) * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
    }
    DrawStatusBar (100, 100);

    FileWrite (
      &first, (dst - 1) * sizeof (Monde), sizeof (Monde),
      BanqueToFile (banque));
  }

  if (src > dst)
  {
    FileRead (
      &first, src * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));

    for (i = src - 1; i >= dst; i--)
    {
      DrawStatusBar (i - (src - 1), dst - src);
      FileRead (
        &temp, i * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
      FileWrite (
        &temp, (i + 1) * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
    }
    DrawStatusBar (100, 100);

    FileWrite (
      &first, dst * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
  }

  return 0;
}

/* ------------- */
/* MondeDuplique */
/* ------------- */

/*
    Duplique un monde juste après.
    Retourne 0 si tout est ok.
 */

static short
MondeDuplique (short m)
{
  short max, i;
  Monde temp;

  max = maxmonde;
  if (g_construit)
    max--;

  if (m >= max)
    return 1;

  for (i = max - 1; i >= m; i--)
  {
    DrawStatusBar (i - (max - 1), m - max);
    FileRead (&temp, i * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
    FileWrite (
      &temp, (i + 1) * sizeof (Monde), sizeof (Monde), BanqueToFile (banque));
  }
  DrawStatusBar (100, 100);

  g_monde++;
  maxmonde++;
  return 0;
}

/* ------------ */
/* MondeDetruit */
/* ------------ */

/*
    Détruit un monde.
    Retourne 0 si tout est ok.
 */

static short
MondeDetruit (short m)
{
  short max, i, j;
  Monde temp;

  max = maxmonde;
  if (g_construit)
    max--;

  if (m >= max)
    return 1;

  FileDelete ('-'); /* détruit le fichier temporaire (év.) */

  j = 0;
  for (i = 0; i < max; i++)
  {
    if (i != m)
    {
      DrawStatusBar (j, max - 1);
      if (FileRead (
            &temp, i * sizeof (Monde), sizeof (Monde), BanqueToFile (banque)))
        goto error;
      if (FileWrite (&temp, j * sizeof (Monde), sizeof (Monde), '-'))
        goto error;
      j++;
    }
  }
  DrawStatusBar (100, 100);

  FileDelete (BanqueToFile (banque)); /* détruit l'ancien fichier définitif */
  /* renomme le fichier temporaire -> définitif */
  FileRename ('-', BanqueToFile (banque));

  maxmonde--;
  return 0;

error:
  FileDelete ('-');
  return 1;
}

/* ------------ */
/* PlayPartieLg */
/* ------------ */

/*
    Retourne la longueur nécessaire pour sauver les variables de la partie en
   cours.
 */

static int
PlayPartieLg (void)
{
  return sizeof (Monde) + sizeof (Partie);
}

/* --------------- */
/* PlayPartieWrite */
/* --------------- */

/*
    Sauve les variables de la partie en cours.
 */

static short
PlayPartieWrite (int pos, char file)
{
  short  err;
  Partie partie;

  memset (&partie, 0, sizeof (Partie));

  partie.check   = 123456;
  partie.monde   = g_monde;
  partie.typejeu = g_typejeu;
  partie.banque  = banque;

  err = FileWrite (&partie, pos, sizeof (Partie), file);
  if (err)
    return err;
  pos += sizeof (Partie);

  err = FileWrite (&descmonde, pos, sizeof (Monde), file);
  return err;
}

/* -------------- */
/* PlayPartieRead */
/* -------------- */

/*
    Lit les variables de la partie en cours.
 */

static short
PlayPartieRead (int pos, char file)
{
  short  err;
  Partie partie;

  err = FileRead (&partie, pos, sizeof (Partie), file);
  if (err)
    return err;
  pos += sizeof (Partie);

  if (partie.check != 123456)
    return 1;

  g_monde   = partie.monde;
  g_typejeu = partie.typejeu;
  banque    = partie.banque;

  if (banque < 'i')
  {
    fj.niveau[fj.joueur] = banque - 'a';
  }
  else
  {
    fj.niveau[fj.joueur] = 8;
  }
  MondeMax (banque);

  err = FileRead (&descmonde, pos, sizeof (Monde), file);
  return err;
}

/* --------------- */
/* PartieCheckFile */
/* --------------- */

/*
    Vérifie si le fichier de sauvegarde de la partie est correct,
    c'est-à-dire s'il correspond à cette version de soft !
 */

static short
PartieCheckFile ()
{
  short  err;
  Header header;

  err = FileRead (&header, 0, sizeof (Header), 'y');
  if (err == 0)
  {
    if (
      header.ident == 1 && header.lg[0] == PlayPartieLg () &&
      header.lg[1] == MovePartieLg () && header.lg[2] == DecorPartieLg () &&
      header.lg[3] == PalPartieLg () && header.lg[4] == MachinePartieLg () &&
      header.lg[5] == 0)
      return 0; /* fichier ok */
  }
  FileDelete ('y');

  memset (&header, 0, sizeof (Header));

  header.ident = 1;
  header.lg[0] = PlayPartieLg ();
  header.lg[1] = MovePartieLg ();
  header.lg[2] = DecorPartieLg ();
  header.lg[3] = PalPartieLg ();
  header.lg[4] = MachinePartieLg ();

  FileWrite (&header, 0, sizeof (Header), 'y');

  return 1; /* le fichier n'était pas correct */
}

#define MAXPARTIE 4 /* nb max de parties sauvables par joueur */

/* ----------- */
/* PartieSauve */
/* ----------- */

/*
    Sauve la partie en cours.
 */

static short
PartieSauve (short rang)
{
  int   pos;
  short err;

  PartieCheckFile (); /* adapte le fichier si nécessaire */

  pos =
    sizeof (Header) + (PlayPartieLg () + MovePartieLg () + DecorPartieLg () +
                       PalPartieLg () + MachinePartieLg ()) *
                        (fj.joueur * MAXPARTIE + rang);

  err = PlayPartieWrite (pos, 'y');
  if (err)
    return err;
  pos += PlayPartieLg ();

  err = MovePartieWrite (pos, 'y');
  if (err)
    return err;
  pos += MovePartieLg ();

  err = DecorPartieWrite (pos, 'y');
  if (err)
    return err;
  pos += DecorPartieLg ();

  err = PalPartieWrite (pos, 'y');
  if (err)
    return err;
  pos += PalPartieLg ();

  err = MachinePartieWrite (pos, 'y');
  return err;
}

/* ----------- */
/* PartiePrend */
/* ----------- */

/*
    Reprend la partie en cours.
 */

static short
PartiePrend (short rang)
{
  int   pos;
  short err;

  err = PartieCheckFile (); /* fichier ok ? */
  if (err)
    return err;

  pos =
    sizeof (Header) + (PlayPartieLg () + MovePartieLg () + DecorPartieLg () +
                       PalPartieLg () + MachinePartieLg ()) *
                        (fj.joueur * MAXPARTIE + rang);

  err = PlayPartieRead (pos, 'y');
  if (err)
    return err;
  pos += PlayPartieLg ();

  err = MovePartieRead (pos, 'y');
  if (err)
    return err;
  pos += MovePartieLg ();

  err = DecorPartieRead (pos, 'y');
  if (err)
    return err;
  pos += DecorPartieLg ();

  err = PalPartieRead (pos, 'y');
  if (err)
    return err;
  pos += PalPartieLg ();

  err = MachinePartieRead (pos, 'y');
  if (err)
    return err;

  IconDrawOpen ();
  MoveRedraw (); /* redessine sans changement */
  IconDrawClose (1);

  ChangeCouleur (); /* change les couleurs */
  MusicStart (4);

  return 0;
}

/* -------------- */
/* PartieDrawIcon */
/* -------------- */

/*
    Dessine l'icône prend ou sauve au milieu de la fenêtre.
 */

static void
PartieDrawIcon (short key)
{
  Pt pos, zero = {0, 0}, dim = {LYICO, LXICO};

  pos.x = POSXDRAW + 20;
  pos.y = POSYDRAW + DIMYDRAW - LYICO - 20;

  if (key == KEYLOAD || key == -KEYLOAD)
    pos.x += LXICO + 20;

  if (key == KEYSAVE)
    DrawSprite (ICO_SAUVE, zero, pos, dim);
  else if (key == KEYLOAD)
    DrawSprite (ICO_PREND, zero, pos, dim);
  else if (key == -KEYSAVE)
    DrawSprite (ICO_ATTENTE + 0, zero, pos, dim);
  else if (key == -KEYLOAD)
    DrawSprite (ICO_ATTENTE + 1, zero, pos, dim);
}

/* ----------------- */
/* PartieClicToEvent */
/* ----------------- */

/*
    Conversion d'un clic à une position donnée en un événement clavier.
 */

static short
PartieClicToEvent (Pt pos)
{
  short * ptable;

  static short table[] = {2,   77, 19, 19, KEYUNDO, /* case de fermeture */
                          7,   55, 31, 23, KEYF1,   /* partie #1 */
                          41,  55, 31, 23, KEYF2,   /* partie #2 */
                          7,   29, 31, 23, KEYF3,   /* partie #3 */
                          41,  29, 31, 23, KEYF4,   /* partie #4 */

                          102, 77, 19, 19, KEYUNDO, /* case de fermeture */
                          107, 55, 31, 23, '1',     /* partie #1 */
                          141, 55, 31, 23, '2',     /* partie #2 */
                          107, 29, 31, 23, '3',     /* partie #3 */
                          141, 29, 31, 23, '4',     /* partie #4 */

                          -1};

  pos.x -= POSXDRAW + 20;
  pos.y -= POSYDRAW + DIMYDRAW - LYICO - 20;

  ptable = table;
  while (ptable[0] != -1)
  {
    if (
      pos.x >= ptable[0] && pos.x <= ptable[0] + ptable[2] &&
      pos.y >= LYICO - 1 - ptable[1] &&
      pos.y <= LYICO - 1 - ptable[1] + ptable[3])
      return ptable[4];
    ptable += 5;
  }

  return 0;
}

/* ------------ */
/* PartieDisque */
/* ------------ */

/*
    Prend ou sauve la partie en cours.
 */

static void
PartieDisque (short key, Pt pos)
{
  short           mode = key;
  static SDL_bool open = SDL_FALSE;

  if (open == SDL_FALSE)
  {
    PlayEvSound (SOUND_CLIC);
    if (mode != KEYLOAD)
      PartieDrawIcon (KEYSAVE); /* dessine l'icône */
    if (mode != KEYSAVE)
      PartieDrawIcon (KEYLOAD); /* dessine l'icône */
    open       = SDL_TRUE;
    g_saveMenu = SDL_TRUE;
  }

  if (key == KEYCLIC || key == KEYCLICR)
    key = PartieClicToEvent (pos);

  if (!(key == KEYUNDO || key == KEYQUIT || key == KEYHOME || key == KEYF1 ||
        key == KEYF2 || key == KEYF3 || key == KEYF4 || key == '1' ||
        key == '2' || key == '3' || key == '4'))
    return;

  open              = SDL_FALSE;
  g_saveMenu        = SDL_FALSE;
  g_ignoreKeyClicUp = SDL_TRUE;

  if (mode != KEYLOAD && key <= KEYF1 && key >= KEYF4)
  {
    PlayEvSound (SOUND_CLIC);
    PartieDrawIcon (-KEYSAVE);  /* dessine l'icône d'attente */
    PartieSauve (-key + KEYF1); /* sauve la partie */
  }

  if (mode == KEYSAVE && key >= '1' && key <= '4')
  {
    PlayEvSound (SOUND_CLIC);
    PartieDrawIcon (-KEYSAVE); /* dessine l'icône d'attente */
    PartieSauve (key - '1');   /* sauve la partie */
  }

  if (mode != KEYSAVE && key >= '1' && key <= '4')
  {
    PlayEvSound (SOUND_CLIC);
    PartieDrawIcon (-KEYLOAD); /* dessine l'icône d'attente */
    PartiePrend (key - '1');   /* reprend une partie */
  }

  IconDrawAll (); /* faudra tout redessiner */
}

/* ------------ */
/* StopDrawIcon */
/* ------------ */

/*
    Dessine les icônes stoppe oui/non au milieu de la fenêtre.
 */

static void
StopDrawIcon (void)
{
  Pt pos, p = {0, 0}, dim = {LYICO, LXICO};

  pos.x = POSXDRAW + 20;
  pos.y = POSYDRAW + DIMYDRAW - LYICO - 20;

  DrawSprite (ICO_STOPOUI, p, pos, dim);

  pos.x += LXICO + 20;

  DrawSprite (ICO_STOPNON, p, pos, dim);
}

/* --------------- */
/* StopClicToEvent */
/* --------------- */

/*
    Conversion d'un clic à une position donnée en un événement clavier.
 */

static short
StopClicToEvent (Pt pos)
{
  pos.x -= POSXDRAW + 20;
  pos.y -= POSYDRAW + DIMYDRAW - LYICO - 20;

  if (pos.y < 0 || pos.y > LYICO)
    return KEYUNDO;

  if (pos.x >= 0 && pos.x <= LXICO)
    return KEYHOME;

  return KEYUNDO;
}

/* ---------- */
/* StopPartie */
/* ---------- */

/*
    Demande s'il faut stopper la partie en cours.
 */

static short
StopPartie (short key, Pt pos)
{
  static Pixmap   pmsave = {0};
  Pt              spos, sdim;
  Pt              p;
  static SDL_bool open = SDL_FALSE;
  g_ignoreKeyClicUp    = SDL_TRUE;

  spos.x = POSXDRAW + 20;
  spos.y = POSYDRAW + DIMYDRAW - LYICO - 20;
  sdim.x = LXICO + 20 + LXICO;
  sdim.y = LYICO;

  if (open == SDL_FALSE)
  {
    PlayEvSound (SOUND_CLIC);
    if (GetPixmap (&pmsave, sdim, 0, 2) != 0)
      return KEYHOME;
    p.y = 0;
    p.x = 0;
    CopyPixel (0, spos, &pmsave, p, sdim); /* sauve l'écran */
    open       = SDL_TRUE;
    g_stopMenu = SDL_TRUE;
    key        = 0;
  }

  StopDrawIcon (); /* dessine les icônes */

  // while (1)
  {
    // key = GetEvent(&pos);
    if (key == KEYCLIC)
    {
      key = StopClicToEvent (pos);
      if ((key == KEYUNDO || key == KEYHOME))
        goto next;
    }

    return 0;
  }

next:
  PlayEvSound (SOUND_CLIC);

  p.y = 0;
  p.x = 0;
  CopyPixel (&pmsave, p, 0, spos, sdim); /* restitue l'écran */
  GivePixmap (&pmsave);
  // SDL_RenderPresent(g_renderer);
  open       = SDL_FALSE;
  g_stopMenu = SDL_FALSE;

  return key;
}

/* -------------- */
/* JoueurEditOpen */
/* -------------- */

/*
    Prépare l'édition du nom des joueurs.
 */

static void
JoueurEditOpen (void)
{
  Rect rect;

  rect.p1.x = 299;
  rect.p1.y = LYIMAGE () - 297 + fj.joueur * 40;
  rect.p2.x = 299 + 180;
  rect.p2.y = rect.p1.y + 22;
  EditOpen (fj.nom[fj.joueur], MAXNOMJ, rect);

  g_typetext = 1;
}

/* --------------- */
/* JoueurEditClose */
/* --------------- */

/*
    Fin de l'édition du nom des joueurs.
 */

static void
JoueurEditClose (void)
{
  EditClose ();
  g_typetext = 0;
}

/* --------- */
/* DrawIdent */
/* --------- */

/*
    Affiche tous les noms des joueurs.
 */

static void
DrawIdent (void)
{
  short joueur;
  char  chaine[20];
  Pt    pos;

  SDL_memset (chaine, 0, sizeof (chaine));

  joueur = fj.joueur;
  for (fj.joueur = 0; fj.joueur < MAXJOUEUR; fj.joueur++)
  {
    JoueurEditOpen (); /* affiche le nom du joueur */
    JoueurEditClose ();
  }
  fj.joueur = joueur;

  pos.y = LYIMAGE () - 286;
  for (joueur = 0; joueur < MAXJOUEUR; joueur++)
  {
    pos.x = 500;
    if (fj.nom[joueur][0] != 0)
    {
      SDL_snprintf (chaine, sizeof (chaine), "A:");
      DrawString (0, pos, chaine, TEXTSIZELIT, SDL_FALSE);

      pos.x += 15;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][0] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][0]);

      pos.x += 22;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][2] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][2]);

      pos.x += 22;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][4] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][4]);

      pos.x += 22;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][6] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][6]);
    }

    pos.x = 500;
    pos.y += 15;
    if (fj.nom[joueur][0] != 0)
    {
      SDL_snprintf (chaine, sizeof (chaine), "T:");
      DrawString (0, pos, chaine, TEXTSIZELIT, SDL_FALSE);

      pos.x += 15;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][1] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][1]);

      pos.x += 22;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][3] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][3]);

      pos.x += 22;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][5] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][5]);

      pos.x += 22;
      SDL_snprintf (chaine, sizeof (chaine), "%2d", fj.progres[joueur][7] + 1);
      DrawString (0, pos, chaine, TEXTSIZELIT, fj.resolved[joueur][7]);
    }
    pos.y += 40 - 15;
  }
}

/* ------------- */
/* PhaseEditOpen */
/* ------------- */

/*
    Prépare le monde dans descmonde pour pouvoir l'éditer.
 */

static void
PhaseEditOpen (void)
{
  MondeRead (g_monde, banque); /* lit le monde à éditer sur disque */
  savemonde = descmonde;       /* sauve le monde (palette, etc.) */
  MondeEdit ();                /* modifie le monde pour pouvoir l'éditer */
}

/* -------------- */
/* PhaseEditClose */
/* -------------- */

/*
    Fin de l'édition du monde dans descmonde.
 */

static void
PhaseEditClose (void)
{
  short i;

  for (i = 0; i < MAXPALETTE; i++)
  {
    descmonde.palette[i] = savemonde.palette[i]; /* remet la palette initiale */
  }

  MondeWrite (g_monde, banque);

  g_typeedit = 0; /* fin de l'édition */
}

static short
RedrawPhase (Phase phase)
{
  Rect rect;

  ShowImage (); /* affiche l'image de base */

  switch (phase)
  {
  case PHASE_GENERIC:
  case PHASE_SUIVANT:
  case PHASE_FINI0:
  case PHASE_FINI1:
  case PHASE_FINI2:
  case PHASE_FINI3:
  case PHASE_FINI4:
  case PHASE_FINI5:
  case PHASE_FINI6:
  case PHASE_FINI7:
  case PHASE_FINI8:
  case PHASE_GOODBYE:
    break;

  case PHASE_IDENT:
    DrawJoueur ();     /* affiche le joueur */
    DrawIdent ();      /* affiche tous les noms */
    JoueurEditOpen (); /* prépare l'édition du nom */
    break;

  case PHASE_REGLAGE:
    DrawVitesse ();  /* affiche la vitesse */
    DrawScroll ();   /* affiche le scroll */
    DrawBruitage (); /* affiche le mode de bruitages */
    DrawTelecom ();  /* affiche le mode de télécommande */
    break;

  case PHASE_REGLAGE2:
    DrawLanguage ();
    DrawScreen ();
    DrawTheme ();
    break;

  case PHASE_PARAM:
    PaletteEditOpen (descmonde.palette);
    rect.p1.x = 218;
    rect.p1.y = LYIMAGE () - 47;
    rect.p2.x = 218 + 180;
    rect.p2.y = LYIMAGE () - 47 + 23;
    EditOpen (descmonde.text, MAXTEXT, rect);
    DrawCouleur (); /* affiche le mode de couleur */
    break;

  case PHASE_PRIVE:
    DrawNumMonde (); /* affiche le numéro du monde */
    DrawObjectif (); /* affiche l'objectif */
    break;

  case PHASE_DEPLACE:
    DrawNumMonde (); /* affiche le numéro du monde */
    DrawObjectif (); /* affiche l'objectif */
    break;

  case PHASE_OBJECTIF:
    DrawNumMonde (); /* affiche le numéro du monde */
    DrawObjectif (); /* affiche l'objectif */
    break;

  case PHASE_RECOMMENCE:
    DrawObjectif (); /* affiche l'objectif */
    break;

  case PHASE_PLAY:
    DrawArrows (0); /* dessine les flèches */
    DrawPause ();   /* dessine le bouton pause */
    PaletteDraw ();

    /* Redraw the whole main game screen */
    Pt ovisu = DecorGetOrigine ();
    DecorSetOrigine (ovisu, 1);
    break;

  default:
    break;
  }

  return 0; /* nouvelle phase ok */
}

/* ----------- */
/* ChangePhase */
/* ----------- */

/*
    Change la phase du jeu.
    Retourne !=0 en cas d'erreur.
 */

static short
ChangePhase (Phase newphase)
{
  short err, type;
  Rect  rect;

  /*	Ferme la phase de jeu en cours. */

  MusicStop ();
  ClrEvents ();

  switch (phase)
  {
  case PHASE_IDENT:
    JoueurEditClose (); /* fin de l'édition du nom */
    BlackScreen ();
    JoueurWrite (); /* écrit le fichier des joueurs */
    break;

  case PHASE_REGLAGE:
  case PHASE_REGLAGE2:
    BlackScreen ();
    JoueurWrite (); /* Ecrit le fichier des joueurs */
    break;

  case PHASE_PARAM:
    PaletteEditClose (descmonde.palette);
    EditClose ();
    BlackScreen ();
    MondeWrite (g_monde, banque);
    g_typetext = 0;
    break;

  case PHASE_DEPLACE:
    g_monde = mondeinit;
    break;

  case PHASE_PLAY:
    if (g_typeedit)
    {
      BlackScreen ();
      PhaseEditClose ();
    }
    DecorClose (); /* fermeture des décors */
    IconClose ();  /* fermeture des icônes */
    break;

  case PHASE_FINI0:
    fj.niveau[fj.joueur] = 1; /* 1A -> 1T */
    break;

  case PHASE_FINI1:
    fj.niveau[fj.joueur] = 2; /* 1T -> 2A */
    break;

  case PHASE_FINI2:
    fj.niveau[fj.joueur] = 3; /* 2A -> 2T */
    break;

  case PHASE_FINI3:
    fj.niveau[fj.joueur] = 4; /* 2T -> 3A */
    break;

  case PHASE_FINI4:
    fj.niveau[fj.joueur] = 5; /* 3A -> 3T */
    break;

  case PHASE_FINI5:
    fj.niveau[fj.joueur] = 6; /* 3T -> 4A */
    break;

  case PHASE_FINI6:
    fj.niveau[fj.joueur] = 7; /* 4A -> 4T */
    break;

  case PHASE_FINI7:
    fj.niveau[fj.joueur] = 8; /* 4T -> 5 */
    break;

  case PHASE_FINI8:
    g_monde = maxmonde - 1; /* à construire */
    break;

  default:
    break;
  }

  /*	Change la phase de jeu. */

  g_ignoreKeyClicUp = SDL_TRUE;
  phase             = newphase; /* change la phase */

  if (phase == PHASE_GENERIC)
  {
    JoueurRead (&arguments); /* lit le fichier des joueurs sur disque */
    JoueurWrite ();
    LoadSprites (NORMAL); /* charge l'image des icônes */
  }
  else if (phase != PHASE_PLAY)
    LoadSprites (NORMAL);

  ShowImage (); /* affiche l'image de base */

  /*	Ouvre la nouvelle phase de jeu. */

  musique = 0; /* pas de musique de fond */

  switch (phase)
  {
  case PHASE_GENERIC:
    MusicStart (0);
    musique    = 1;
    lastaccord = -1;
    break;

  case PHASE_SUIVANT:
    MusicStart (2);
    break;

  case PHASE_FINI0:
  case PHASE_FINI1:
  case PHASE_FINI2:
  case PHASE_FINI3:
  case PHASE_FINI4:
  case PHASE_FINI5:
  case PHASE_FINI6:
  case PHASE_FINI7:
  case PHASE_FINI8:
    MusicStart (1);
    musique    = 2;
    lastaccord = -1;
    break;

  case PHASE_IDENT:
    JoueurRead (&arguments); /* lit le fichier des joueurs sur disque */
    DrawJoueur ();           /* affiche le joueur */
    DrawIdent ();            /* affiche tous les noms */
    JoueurEditOpen ();       /* prépare l'édition du nom */
    break;

  case PHASE_REGLAGE:
    DrawVitesse ();  /* affiche la vitesse */
    DrawScroll ();   /* affiche le scroll */
    DrawBruitage (); /* affiche le mode de bruitages */
    DrawTelecom ();  /* affiche le mode de télécommande */
    MusicStart (3);
    break;

  case PHASE_REGLAGE2:
    DrawLanguage ();
    DrawScreen ();
    DrawTheme ();
    break;

  case PHASE_PARAM:
    MondeRead (g_monde, banque); /* lit le monde à modifier sur disque */
    PaletteEditOpen (descmonde.palette);
    rect.p1.x = 218;
    rect.p1.y = LYIMAGE () - 47;
    rect.p2.x = 218 + 180;
    rect.p2.y = LYIMAGE () - 47 + 23;
    EditOpen (descmonde.text, MAXTEXT, rect);
    g_typetext = 1;
    DrawCouleur (); /* affiche le mode de couleur */
    break;

  case PHASE_PRIVE:
    MondeRead (g_monde, banque); /* lit le nouveau monde sur disque */
    DrawNumMonde ();             /* affiche le numéro du monde */
    DrawObjectif ();             /* affiche l'objectif */
    retry = 0;
    break;

  case PHASE_DEPLACE:
    mondeinit = g_monde;
    DrawNumMonde (); /* affiche le numéro du monde */
    DrawObjectif (); /* affiche l'objectif */
    break;

  case PHASE_OBJECTIF:
    MondeRead (g_monde, banque); /* lit le nouveau monde sur disque */
    DrawNumMonde ();             /* affiche le numéro du monde */
    DrawObjectif ();             /* affiche l'objectif */
    retry = 0;
    break;

  case PHASE_RECOMMENCE:
    PlayEvSound (SOUND_NON);
    MondeRead (g_monde, banque); /* relit le monde sur disque */
    DrawObjectif ();             /* affiche l'objectif */
    retry++;
    break;

  case PHASE_PLAY:
    ChangeCouleur (); /* change les couleurs */

    if (g_typeedit)
      PhaseEditOpen ();

    err = IconOpen (); /* ouverture des icônes */
    if (err)
      FatalBreak (err);

    err = MoveOpen (); /* ouverture des objets en mouvement */
    if (err)
      FatalBreak (err);

    err = DecorOpen (); /* ouverture des décors */
    if (err)
      FatalBreak (err);

    IconDrawFlush ();           /* vide tous les buffers internes */
    DecorNewMonde (&descmonde); /* initialise le monde */

    type = 0;
    if (g_typejeu == 0 || g_typeedit)
      type = 1;
    PaletteNew (descmonde.palette, type);

    DecorMake (1);  /* fabrique le décor */
    IconDrawAll (); /* redessine toute la fenêtre */

    g_pause = 0;
    DrawArrows (0); /* dessine les flèches */
    DrawPause ();   /* dessine le bouton pause */
    if (g_typeedit == 0)
      MusicStart (4);
    break;

  case PHASE_GOODBYE:
    MusicStart (5);
    break;

  default:
    break;
  }

  passindex = 0;

  ClrEvents ();
  return 0; /* nouvelle phase ok */
}

/* Tables décrivants les zones cliquables dans les images */
/* ------------------------------------------------------ */

static short timage21[] =                        /* initial */
  {331, 110, 56, 70, 0,         ACTION_NIVEAU8,  //
   -1,  -1,  -1, -1, '5',       ACTION_NIVEAUK5, //

   123, 279, 40, 46, 0,         ACTION_NIVEAU0,  //
   160, 302, 41, 53, 0,         ACTION_NIVEAU1,  //
   298, 284, 50, 59, 0,         ACTION_NIVEAU2,  //
   348, 280, 50, 66, 0,         ACTION_NIVEAU3,  //
   150, 150, 48, 65, 0,         ACTION_NIVEAU4,  //
   193, 180, 51, 77, 0,         ACTION_NIVEAU5,  //
   376, 174, 58, 65, 0,         ACTION_NIVEAU6,  //
   436, 165, 56, 67, 0,         ACTION_NIVEAU7,  //
   -1,  -1,  -1, -1, '1',       ACTION_NIVEAUK1, //
   -1,  -1,  -1, -1, '2',       ACTION_NIVEAUK2, //
   -1,  -1,  -1, -1, '3',       ACTION_NIVEAUK3, //
   -1,  -1,  -1, -1, '4',       ACTION_NIVEAUK4, //
   -1,  -1,  -1, -1, KEYRETURN, ACTION_NIVEAUGO, //
   543, 150, 71, 78, KEYDEF,    ACTION_REGLAGE,  //
   482, 71,  68, 60, 0,         ACTION_AIDE,     //
   554, 82,  67, 65, 0,         ACTION_IDENT,    //
   526, 287, 67, 82, KEYUNDO,   ACTION_QUITTE,   //
   0};

static short timage22[] =                           /* objectif */
  {22,  73,  208, 56,  KEYRETURN, ACTION_JOUE,      //
   40,  272, 361, 161, KEYRETURN, ACTION_JOUE,      //
   425, 73,  190, 56,  KEYUNDO,   ACTION_ANNULE,    //
   473, 284, 63,  53,  KEYUP,     ACTION_MONDESUIV, //
   473, 233, 63,  53,  KEYDOWN,   ACTION_MONDEPREC, //
   472, 174, 136, 31,  0,         ACTION_MONDEBAR,  //
   0};

static short timage23[] =                          /* recommence */
  {21,  72,  292, 56,  KEYRETURN, ACTION_JOUE,     //
   121, 240, 435, 120, KEYRETURN, ACTION_JOUE,     //
   400, 72,  209, 56,  KEYUNDO,   ACTION_STOPPEKO, //
   0};

static short timage24[] =                        /* suivant */
  {27,  76, 207, 56, KEYRETURN, ACTION_SUIVANT,  //
   419, 76, 185, 56, KEYUNDO,   ACTION_STOPPEOK, //
   0};

static short timage25[] =                           /* privé */
  {22,  73,  208, 56,  KEYRETURN, ACTION_JOUE,      //
   40,  272, 361, 161, KEYRETURN, ACTION_JOUE,      //
   248, 72,  132, 26,  'E',       ACTION_EDIT,      //
   248, 45,  132, 26,  'T',       ACTION_PARAM,     //
   425, 73,  190, 56,  KEYUNDO,   ACTION_DEBUT,     //
   473, 284, 63,  53,  KEYUP,     ACTION_MONDESUIV, //
   473, 233, 63,  53,  KEYDOWN,   ACTION_MONDEPREC, //
   472, 174, 136, 31,  0,         ACTION_MONDEBAR,  //
   470, 142, 138, 31,  'M',       ACTION_OPER,      //
   0};

static short timage26[] =                       /* paramètres */
  {146, 101, 88,  32, 0,       ACTION_COULEUR0, //
   242, 101, 88,  32, 0,       ACTION_COULEUR1, //
   338, 101, 88,  32, 0,       ACTION_COULEUR2, //
   434, 101, 88,  32, 0,       ACTION_COULEUR3, //
   530, 101, 88,  32, 0,       ACTION_COULEUR4, //
   414, 64,  210, 56, KEYUNDO, ACTION_OBJECTIF, //
   0};

static short timage27[] =                          /* déplace */
  {33,  73,  207, 56, KEYRETURN, ACTION_ORDRE,     //
   395, 73,  207, 56, KEYUNDO,   ACTION_OBJECTIF,  //
   473, 284, 63,  53, KEYUP,     ACTION_MONDESUIV, //
   473, 233, 63,  53, KEYDOWN,   ACTION_MONDEPREC, //
   0};

static short timage28[] =                     /* aide 2.1 */
  {109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE22, //
   0};

static short timage30[] =                       /* opération */
  {41,  294, 237, 56, 'R',     ACTION_DETRUIT,  //
   41,  227, 237, 56, 'M',     ACTION_DEPLACE,  //
   41,  160, 237, 56, 'C',     ACTION_DUPLIQUE, //
   417, 81,  190, 56, KEYUNDO, ACTION_OBJECTIF, //
   0};

static short timage31[] =                                   /* identification */
  {230, 299,       250,   34,    KEYF1,     ACTION_JOUEUR0, //
   230, 259,       250,   34,    KEYF2,     ACTION_JOUEUR1, //
   230, 219,       250,   34,    KEYF3,     ACTION_JOUEUR2, //
   230, 179,       250,   34,    KEYF4,     ACTION_JOUEUR3, //
   24,  340 - 269, 153,   51,    KEYRETURN, ACTION_DEBUT,   //
   191, 340 - 269, 153,   51,    'H',       ACTION_AIDE,    //
   466, 340 - 269, 151,   51,    KEYUNDO,   ACTION_QUITTE,  //
   355, 90,        LXICO, LYICO, 0,         ACTION_REGLAGE2, //
   0};

static short timage32[] =                     /* aide 2.2 */
  {21,  71, 77, 55, 0,         ACTION_AIDE21, //
   109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE23, //
   0};

static short timage33[] =                   /* fini niveau */
  {22, 72, 139, 56, KEYRETURN, ACTION_FINI, //
   0};

static short timage34[] =                     /* aide 2.3 */
  {21,  71, 77, 55, 0,         ACTION_AIDE22, //
   109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE24, //
   0};

static short timage37[] =                                /* réglages 1 */
  {31,  292,       160, 32, 'S',       ACTION_VITESSE0,  //
   31,  260,       160, 32, 'N',       ACTION_VITESSE1,  //
   31,  228,       160, 32, 'Q',       ACTION_VITESSE2,  //
   272, 292,       270, 32, 0,         ACTION_SCROLL0,   //
   272, 260,       270, 32, 0,         ACTION_SCROLL1,   //
   19,  137,       21,  21, 0,         ACTION_NOISEVOLP, //
   19,  99,        21,  21, 0,         ACTION_NOISEVOLM, //
   97,  137,       21,  21, 0,         ACTION_MUSICVOLP, //
   97,  99,        21,  21, 0,         ACTION_MUSICVOLM, //
   272, 172,       270, 32, 0,         ACTION_TELECOM0,  //
   272, 140,       270, 32, 0,         ACTION_TELECOM1,  //
   43,  340 - 281, 166, 51, KEYRETURN, ACTION_DEBUT,     //
   225, 340 - 281, 279, 51, KEYDEF,    ACTION_IDENT,     //
   520, 340 - 281, 73,  51, 0,         ACTION_REGLAGE2,  //
   0};

static short timage36[] =             /* générique */
  {0, 339, 640, 340, 0, ACTION_IDENT, //
   0};

static short timage40[] =                     /* aide 3.1 */
  {109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE32, //
   0};

static short timage41[] =                     /* aide 3.2 */
  {21,  71, 77, 55, 0,         ACTION_AIDE31, //
   109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE33, //
   0};

static short timage42[] =                     /* aide 3.3 */
  {21,  71, 77, 55, 0,         ACTION_AIDE32, //
   109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE34, //
   0};

static short timage43[] =                     /* aide 3.4 */
  {21,  71, 77, 55, 0,         ACTION_AIDE33, //
   109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE35, //
   0};

static short timage44[] =                     /* aide 3.5 */
  {21,  71, 77, 55, 0,         ACTION_AIDE34, //
   109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE36, //
   0};

static short timage45[] =                   /* aide 3.6 */
  {21,  71, 77, 55, 0,       ACTION_AIDE35, //
   109, 71, 77, 55, KEYUNDO, ACTION_AIDE,   //
   0};

static short timage46[] =               /* aide */
  {15,  262, 184, 53, 0, ACTION_AIDE11, //
   15,  200, 184, 53, 0, ACTION_AIDE21, //
   15,  135, 184, 53, 0, ACTION_AIDE31, //
   15,  72,  184, 53, 0, ACTION_AIDE41, //
   446, 72,  173, 53, 0, ACTION_DEBUT,  //
   0};

static short timage47[] =                     /* aide 4.1 */
  {109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE42, //
   0};

static short timage48[] =                   /* aide 4.2 */
  {21,  71, 77, 55, 0,       ACTION_AIDE41, //
   109, 71, 77, 55, KEYUNDO, ACTION_AIDE,   //
   0};

static short timage49[] =                   /* aide 2.4 */
  {21,  71, 77, 55, 0,       ACTION_AIDE23, //
   109, 71, 77, 55, KEYUNDO, ACTION_AIDE,   //
   0};

static short timage50[] =                     /* aide 1.1 */
  {109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE12, //
   0};

static short timage51[] =                     /* aide 1.2 */
  {21,  71, 77, 55, 0,         ACTION_AIDE11, //
   109, 71, 77, 55, KEYUNDO,   ACTION_AIDE,   //
   196, 71, 77, 55, KEYRETURN, ACTION_AIDE13, //
   0};

static short timage52[] =                   /* aide 1.3 */
  {21,  71, 77, 55, 0,       ACTION_AIDE12, //
   109, 71, 77, 55, KEYUNDO, ACTION_AIDE,   //
   0};

static short timage53[] =                                     /* réglages 2 */
  {31,  340 - 49,  160, 29, 'E',       ACTION_LANG_EN,        //
   31,  340 - 81,  160, 29, 'F',       ACTION_LANG_FR,        //
   31,  340 - 113, 160, 29, 'D',       ACTION_LANG_DE,        //
   272, 340 - 49,  210, 29, 0,         ACTION_SCREEN_1,       //
   272, 340 - 81,  210, 29, 0,         ACTION_SCREEN_2,       //
   272, 340 - 113, 210, 29, 0,         ACTION_SCREEN_FULL,    //
   272, 340 - 204, 210, 29, 0,         ACTION_THEME_DOS,      //
   272, 340 - 236, 210, 29, 0,         ACTION_THEME_SMAKY100, //
   43,  340 - 281, 73,  51, 0,         ACTION_REGLAGE,        //
   132, 340 - 281, 166, 51, KEYRETURN, ACTION_DEBUT,          //
   314, 340 - 281, 279, 51, KEYDEF,    ACTION_IDENT,          //
   0};

/* --------- */
/* GetTimage */
/* --------- */

/*
    Retourne le pointeur à la table timage??[].
 */

static short *
GetTimage (void)
{
  short * pt;

  switch (ConvPhaseToNumImage (phase))
  {
  case 21:
    pt = timage21;
    return pt;

  case 22:
    return timage22;
  case 23:
    return timage23;
  case 24:
    return timage24;
  case 25:
    return timage25;
  case 26:
    return timage26;
  case 27:
    return timage27;
  case 28:
    return timage28;
  case 30:
    return timage30;
  case 31:
    return timage31;
  case 32:
    return timage32;
  case 33:
    return timage33;
  case 34:
    return timage34;
  case 37:
    return timage37;
  case 36:
    return timage36;
  case 40:
    return timage40;
  case 41:
    return timage41;
  case 42:
    return timage42;
  case 43:
    return timage43;
  case 44:
    return timage44;
  case 45:
    return timage45;
  case 46:
    return timage46;
  case 47:
    return timage47;
  case 48:
    return timage48;
  case 49:
    return timage49;
  case 50:
    return timage50;
  case 51:
    return timage51;
  case 52:
    return timage52;
  case 53:
    return timage53;
  }
  return 0;
}

/* ------------ */
/* ClicToAction */
/* ------------ */

/*
    Retourne l'action correspondant à la position d'un clic souris.
    Retourne -1 en cas d'erreur.
 */

static PhAction
ClicToAction (Pt pos)
{
  short * pt;

  pt = GetTimage ();
  if (pt == 0)
    return -1;

  while (pt[0] != 0)
  {
    if (
      pos.x >= pt[0] && pos.x <= pt[0] + pt[2] && pos.y >= LYIMAGE () - pt[1] &&
      pos.y <= LYIMAGE () - pt[1] + pt[3])
    {
      return pt[5]; /* retourne l'action cliquée */
    }
    pt += 6;
  }

  return -1;
}

/* ------------- */
/* EventToAction */
/* ------------- */

/*
    Conversion d'un événement en une action, selon la phase en cours.
    Retourne -1 en cas d'erreur.
 */

static PhAction
EventToAction (char event)
{
  short * pt;

  if (event == 0 || event == KEYCLICREL)
    return -1;

  pt = GetTimage ();
  if (pt == 0)
    return -1;

  if (pt[6] == 0) /* une seule action ? */
    return pt[5]; /* retourne la seule action possible */

  while (pt[0] != 0)
  {
    if (event == pt[4])
    {
      return pt[5]; /* retourne l'action cliquée */
    }
    pt += 6;
  }

  return -1;
}

/* Tables décrivants les animations dans les images */
/* ------------------------------------------------ */

/* clang-format off */
static short tanim21[] =				/* initial */
{
	ACTION_NIVEAU8,		320,122,	DELNORM,6,
	128+88,128+89,128+90,128+89,128+90,128+89,

	ACTION_NIVEAU1,		141,327,	DELNORM,12,
	128+97,128+96,128+97,128+96,128+97,128+96,
												128+98,128+98,128+98,128+96,128+97,128+96,

	ACTION_NIVEAU0,		107,310,	DELNORM,12,	128+81,128+80,128+81,128+80,128+81,128+80,
												128+82,128+82,128+82,128+80,128+81,128+80,

	ACTION_NIVEAU2,		285,303,	DELNORM,8,	128+83,128+84,128+85,128+84,
												128+83,128+83,128+83,128+83,

	ACTION_NIVEAU3,		331,292,	DELNORM,10,	128+99,128+100,128+101,128+100,128+101,128+100,
												128+99,128+99,128+99,128+99,

	ACTION_NIVEAU5,		176,182,	DELNORM,16,	128+1,128+16,128+10,128+10,128+10,128+16,
												128+1,128+1,128+1,128+18,128+4,128+4,
												128+4,128+18,128+1,128+1,

	ACTION_NIVEAU4,		137,164,	DELNORM,4,	1,2,1,3,
	ACTION_NIVEAU6,		375,185,	DELNORM,12,	128+86,128+87,128+86,128+87,128+86,128+87,
												128+86,128+87,128+86,128+87,128+87,128+87,
	ACTION_NIVEAU7,		416,175,	DELNORM,12,	128+102,128+103,128+102,128+103,128+102,128+103,
												128+102,128+103,128+102,128+103,128+103,128+103,
	ACTION_REGLAGE,		538,149,	DELNORM,2,	256+76,128+104,
	ACTION_IDENT,		551,96,		DELNORM,4,	128+116,128+117,128+116,128+115,
	ACTION_AIDE,		485,91,		DELNORM,8,	105,106,106,107,107,104,104,105,
	ACTION_QUITTE,		516,282,	DELNORM,3,	128+105,128+106,128+105,
	-1
};

static short tanim22[] =				/* objectif */
{
	ACTION_JOUE,		89,137,		DELNORM,4,	2,1,3,1,
	ACTION_ANNULE,		482,136,	DELSLOW,15,	68,68,68,64,64,64,33,33,33,64,64,64,68,68,68,
	-1
};

static short tanim23[] =				/* recommence */
{
	ACTION_JOUE,		173,140,	DELNORM,4,	6,4,5,4,
	ACTION_STOPPEKO,	469,134,	DELSLOW,15,	68,68,68,64,64,64,33,33,33,64,64,64,68,68,68,
	-1
};

static short tanim24[] =				/* suivant */
{
	ACTION_SUIVANT,		42,140,		DELNORM,5,	20,21,21,21,1,
	ACTION_STOPPEOK,	475,139,	DELSLOW,15,	68,68,68,64,64,64,33,33,33,64,64,64,68,68,68,
	-1
};

static short tanim25[] =				/* privé */
{
	ACTION_JOUE,		89,137,		DELNORM,4,	2,1,3,1,
	ACTION_DEBUT,		482,136,	DELSLOW,15,	68,68,68,64,64,64,33,33,33,64,64,64,68,68,68,
	-1
};

static short tanim26[] =				/* paramètres */
{
	-1
};

static short tanim27[] =				/* déplace */
{
	-1
};

static short tanim30[] =				/* opération */
{
	ACTION_DETRUIT,		118,98,		DELNORM,8,	58,59,58,58,59,58,59,18,
	ACTION_DEPLACE,		118,98,		DELNORM,8,	58,59,58,58,59,58,59,18,
	ACTION_DUPLIQUE,	118,98,		DELNORM,8,	58,59,58,58,59,58,59,18,
	ACTION_OBJECTIF,	475,142,	DELSLOW,15,	68,68,68,64,64,64,33,33,33,64,64,64,68,68,68,
	-1
};

static short tanim31[] =				/* identification */
{
	ACTION_DEBUT,		62,136,		DELNORM,4,	2,1,3,1,
	ACTION_AIDE,		236,139,	DELNORM,8,	105,106,106,107,107,104,104,105,
	ACTION_QUITTE,		503,138,	DELNORM,4,	50,33,50,36,
        ACTION_REGLAGE2,        360,95,         DELNORM,2,      256+76,128+104,
	-1
};

static short tanim33[] =				/* fini niveau */
{
	ACTION_FINI,		53,138,		DELNORM,4,	46,45,18,45,
	-1
};

static short tanim37[] =				/* réglages */
{
	ACTION_VITESSE0,	158,298,	3,12,		2,2,2,1,1,1,3,3,3,1,1,1,
	ACTION_VITESSE1,	158,298,	2,8,		2,2,1,1,3,3,1,1,
	ACTION_VITESSE2,	158,298,	DELQUICK,4,	2,1,3,1,

	ACTION_SCROLL0,		543,306,	2,16,		108,108,109,109,110,110,111,111,
												112,112,111,111,110,110,109,109,
	ACTION_SCROLL1,		543,306,	3,12,		108,108,108,108,108,108,112,112,112,112,112,112,

	ACTION_BRUIT0,		140,141,	2,4,		77,77,78,78,
	ACTION_BRUIT1,		140,147,	2,4,		93,93,94,94,

	ACTION_NOISEVOLP,	140,141,	2,4,		77,77,78,78,
	ACTION_MUSICVOLP,	140,141,	2,4,		77,77,78,78,
	ACTION_NOISEVOLM,	140,147,	2,4,		93,93,94,94,
	ACTION_MUSICVOLM,	140,147,	2,4,		93,93,94,94,
	-1
};

static short tanim53[] =				/* réglages 2 */
{
	ACTION_SCREEN_1,	480,298,	DELNORM,2,	256+128+93,256+128+94,
	ACTION_SCREEN_2,	480,298,	DELNORM,2,	256+128+93,256+128+94,
	ACTION_SCREEN_FULL,	480,298,	DELNORM,2,	256+128+93,256+128+94,

	ACTION_THEME_DOS,	480,165,	DELNORM,2,	128+88,128+89,
	ACTION_THEME_SMAKY100,	480,165,	DELNORM,2,	128+89,128+90,
	-1
};
/* clang-format on */

/* ------------ */
/* AnimGetTable */
/* ------------ */

/*
    Cherche une table selon la phase de jeu.
 */

static short *
AnimGetTable (void)
{
  short * pt;

  switch (ConvPhaseToNumImage (phase))
  {
  case 21:
    pt = tanim21;
    return pt;

  case 22:
    return tanim22;
  case 23:
    return tanim23;
  case 24:
    return tanim24;
  case 25:
    return tanim25;
  case 26:
    return tanim26;
  case 27:
    return tanim27;
  case 30:
    return tanim30;
  case 31:
    return tanim31;
  case 33:
    return tanim33;
  case 37:
    return tanim37;
  case 53:
    return tanim53;
  }
  return 0;
}

/* ---------- */
/* AnimSearch */
/* ---------- */

/*
    Cherche une animation dans la table.
 */

static short *
AnimSearch (PhAction ac)
{
  short * pt = AnimGetTable ();

  if (pt == 0)
    return 0;

  while (pt[0] != -1)
  {
    if (ac == pt[0])
      return pt;
    pt += 5 + pt[4];
  }
  return 0;
}

/* --------------- */
/* AnimIconAddBack */
/* --------------- */

/*
    Ajoute dans pmtemp toutes les icônes placées derrière ou devant.
 */

static void
AnimIconAddBack (Pt pos, char bFront)
{
  short * pt = animpb;
  Pt      ipos;
  Pt      orig = {0, 0};
  Pt      dim  = {LYICO, LXICO};

  if (phase != PHASE_INIT || pt == 0)
    return;

  while (pt[0] != -1)
  {
    if (bFront == 0 && pt >= animpt)
      return;
    if (bFront == 0 || pt > animpt)
    {
      ipos.x = pt[1];
      ipos.y = LYIMAGE () - pt[2] - 1;
      if (
        ipos.x < pos.x + LXICO && ipos.x + LXICO > pos.x &&
        ipos.y < pos.y + LYICO && ipos.y + LYICO > pos.y)
      {
        Pt _pos = {ipos.y - pos.y, ipos.x - pos.x};
        DrawSpriteTemp (pt[5], orig, _pos, dim);
      }
    }
    pt += 5 + pt[4];
  }
}

/* ------------ */
/* AnimDrawIcon */
/* ------------ */

/*
    Dessine une icône en conservant l'image de fond.
 */

static void
AnimDrawIcon (Pixmap * ppm, short icon, Pt pos, char bOther)
{
  Pt orig = {0, 0};
  Pt dim  = {LYICO, LXICO};

  /* handle Y overflows */
  if (pos.y < 0)
  {
    dim.y  = LYICO + pos.y;
    orig.y = -pos.y;
    pos.y  = 0;
  }
  else if (pos.y > LYIMAGE () - LYICO)
    dim.y = LYIMAGE () - pos.y;

  /* copie l'image originale */
  CopyPixel (&pmimage, pos, &pmtemp, orig, dim);

  if (bOther)
    AnimIconAddBack (pos, 0); /* ajoute les autres icônes derrière */

  DrawSpriteTemp (icon, orig, orig, dim);

  if (bOther)
    AnimIconAddBack (pos, 1); /* ajoute les autres icônes devant */

  /* met dans l'écran */
  CopyPixel (&pmtemp, orig, ppm, pos, dim);
}

/* -------- */
/* AnimDraw */
/* -------- */

/*
    Dessine l'animation en cours.
 */

static short
AnimDraw (void)
{
  short icon;
  Pt    pos;

  if (animpt == 0)
    return DELNORM;

  icon = animpt[5 + animnext % animpt[4]];

  pos.x = animpt[1];
  pos.y = LYIMAGE () - animpt[2] - 1;

  AnimDrawIcon (0, icon, pos, 1); /* dessine l'icône */

  return animpt[3]; /* retourne le délai */
}

/* ------------ */
/* AnimDrawInit */
/* ------------ */

/*
    Dessine toutes les animations en position initiale.
 */

static void
AnimDrawInit (void)
{
  short * pt = AnimGetTable ();

  if (pt == 0)
    return;

  animpb = pt;
  while (pt[0] != -1)
  {
    if (
      phase == PHASE_REGLAGE &&
      (pt[0] == ACTION_BRUIT0 || pt[0] == ACTION_NOISEVOLP ||
       pt[0] == ACTION_MUSICVOLP) &&
      fj.noisevolume == 0)
      goto next;
    if (
      phase == PHASE_REGLAGE &&
      (pt[0] == ACTION_BRUIT1 || pt[0] == ACTION_NOISEVOLM ||
       pt[0] == ACTION_MUSICVOLM) &&
      fj.noisevolume != 0)
      goto next;

    animpt   = pt;
    animnext = 0;
    AnimDraw ();

  next:
    pt += 5 + pt[4];
  }

  if (phase == PHASE_INIT)
  {
    pt = animpb;
    while (pt[0] != -1)
    {
      if (fj.niveau[fj.joueur] == pt[0] - ACTION_NIVEAU0)
      {
        animpt    = pt;
        animnext  = 0;
        animdel   = 5;
        animpos.x = 0;
        animpos.y = 0;
        return;
      }
      pt += 5 + pt[4];
    }
  }

  animpt  = 0;
  animdel = 0;
}

/* ------------ */
/* AnimTracking */
/* ------------ */

/*
    Initialise une nouvelle animation (si nécessaire).
 */

static void
AnimTracking (Pt pos)
{
  short * pt;

  pt = AnimGetTable ();
  if (pt == 0)
    return;

  if (animdel != 0)
  {
    if (animpos.x != pos.x || animpos.y != pos.y)
    {
      animpos = pos;
      animdel--;
    }
    goto anim;
  }

  pt = AnimSearch (ClicToAction (pos)); /* détecte l'animation à effectuer */

  if (pt != animpt)
  {
    if (animpt != 0)
    {
      animnext = 0;
      AnimDraw (); /* remet l'animation initiale */
    }
    animpb   = AnimGetTable ();
    animpt   = pt;
    animnext = 0;
  }

anim:
  if (animpt == 0)
    return;

  AnimDraw ();

  animnext++;
}

/* Table du générique */
/* ------------------ */

static short tgeneric[] = { //

  1, 45,  128, 1, //
  1, 45,  128, 1, //
  1, 45,  128, 1, //
  1, 45,  128, 1, //
  1, 45,  128, 3, //
  1, 45,  128, 1, //
  1, 45,  128, 2, //
  1, 45,  128, 1, //
  1, 45,  128, 3, //
  1, 45,  128, 1, //
  1, 45,  128, 2, //
  1, 45,  128, 1, //
  1, 45,  128, 3, //
  1, 45,  128, 1, //
  1, 45,  128, 2, //
  1, 45,  128, 1, //
  1, 45,  128, 3, //
  1, 45,  128, 1, //
  1, 45,  128, 2, //
  2, 45,  128, 2, //

  1, 45,  128, 2, //
  1, 45,  128, 2, //
  1, 45,  128, 2, //
  1, 45,  128, 2, //
  1, 53,  128, 2, //
  1, 61,  128, 2, //
  1, 69,  128, 2, //
  1, 77,  128, 2, //
  1, 85,  128, 2, //
  1, 93,  128, 2, //
  1, 101, 128, 2, //
  1, 109, 128, 2, //
  1, 116, 128, 2, //
  1, 116, 128, 2, //
  1, 116, 128, 2, //
  1, 116, 128, 2, //

  1, 116, 128, 1,  //
  1, 116, 128, 1,  //
  1, 116, 128, 1,  //
  1, 116, 128, 20, //
  1, 116, 132, 21, //
  1, 116, 136, 21, //
  1, 116, 140, 21, //
  1, 116, 142, 21, //
  1, 116, 140, 21, //
  1, 116, 136, 21, //
  1, 116, 132, 21, //
  1, 116, 128, 20, //
  1, 116, 128, 1,  //
  1, 116, 128, 1,  //
  1, 116, 128, 18, //
  1, 116, 128, 4,  //
  1, 116, 128, 4,  //
  1, 116, 128, 22, //
  1, 116, 132, 23, //
  1, 116, 136, 23, //
  1, 116, 140, 23, //
  1, 116, 142, 23, //
  1, 116, 140, 23, //
  1, 116, 136, 23, //
  1, 116, 132, 23, //
  1, 116, 128, 22, //
  1, 116, 128, 23, //
  1, 116, 128, 23, //
  2, 116, 128, 23, //

  1, 116, 128, 4, //
  1, 116, 128, 4, //
  1, 116, 128, 4, //
  1, 116, 128, 4, //
  1, 124, 128, 4, //
  1, 132, 128, 4, //
  1, 140, 128, 4, //
  1, 148, 128, 4, //
  1, 156, 128, 4, //
  1, 172, 128, 4, //
  1, 180, 128, 4, //
  1, 186, 128, 4, //
  1, 186, 128, 4, //
  1, 186, 128, 4, //
  1, 186, 128, 4, //

  1, 186, 128, 18, //
  1, 186, 128, 1,  //
  1, 186, 128, 33, //
  1, 186, 128, 81, //
  1, 186, 128, 83, //
  1, 186, 128, 81, //
  1, 186, 128, 82, //
  1, 186, 128, 81, //
  1, 186, 128, 83, //
  1, 186, 128, 81, //
  1, 186, 128, 82, //
  1, 186, 128, 81, //
  1, 186, 128, 83, //
  1, 186, 128, 81, //
  1, 186, 128, 82, //
  1, 186, 128, 81, //
  1, 186, 128, 83, //
  1, 186, 128, 81, //
  1, 186, 128, 82, //
  2, 186, 128, 82, //

  1, 186, 128, 82, //
  1, 186, 128, 82, //
  1, 186, 128, 82, //
  1, 186, 128, 82, //
  1, 193, 128, 82, //
  1, 200, 128, 82, //
  1, 207, 128, 82, //
  1, 214, 128, 82, //
  1, 221, 128, 82, //
  1, 228, 128, 82, //
  1, 235, 128, 82, //
  1, 242, 128, 82, //
  1, 249, 128, 82, //
  1, 253, 128, 82, //
  1, 253, 128, 82, //
  1, 253, 128, 82, //
  1, 253, 128, 82, //

  1, 253, 128, 33, //
  1, 253, 128, 1,  //
  1, 253, 128, 18, //
  1, 253, 128, 4,  //
  1, 253, 128, 17, //
  1, 253, 128, 7,  //
  1, 253, 128, 19, //
  1, 253, 128, 10, //
  1, 253, 128, 16, //
  1, 253, 128, 1,  //
  1, 253, 128, 18, //
  1, 253, 128, 4,  //
  1, 253, 128, 17, //
  1, 253, 128, 7,  //
  1, 253, 128, 19, //
  1, 253, 128, 10, //
  1, 253, 128, 16, //
  1, 253, 128, 1,  //
  1, 253, 128, 18, //
  1, 253, 128, 18, //
  1, 253, 128, 58, //
  1, 253, 129, 59, //
  1, 253, 128, 58, //
  1, 253, 129, 59, //
  1, 253, 128, 58, //
  1, 253, 129, 59, //
  1, 253, 128, 58, //
  1, 253, 128, 18, //
  1, 253, 128, 18, //
  2, 253, 128, 18, //

  1, 253, 128, 18, //
  1, 253, 128, 18, //
  1, 253, 128, 18, //
  1, 253, 128, 18, //
  1, 261, 128, 18, //
  1, 269, 128, 18, //
  1, 277, 128, 18, //
  1, 285, 128, 18, //
  1, 293, 128, 18, //
  1, 301, 128, 18, //
  1, 309, 128, 18, //
  1, 317, 128, 18, //
  1, 324, 128, 18, //
  1, 324, 128, 18, //
  1, 324, 128, 18, //
  1, 324, 128, 18, //

  1, 324, 128, 4,  //
  1, 324, 128, 5,  //
  1, 324, 128, 4,  //
  1, 324, 128, 6,  //
  1, 324, 128, 4,  //
  1, 324, 128, 5,  //
  1, 324, 128, 4,  //
  1, 324, 128, 6,  //
  1, 324, 128, 4,  //
  1, 324, 128, 5,  //
  1, 324, 128, 4,  //
  1, 324, 128, 6,  //
  1, 324, 128, 4,  //
  1, 324, 128, 18, //
  1, 324, 128, 45, //
  1, 324, 128, 46, //
  1, 324, 128, 45, //
  1, 324, 128, 18, //
  1, 324, 128, 45, //
  1, 324, 128, 46, //
  1, 324, 128, 45, //
  1, 324, 128, 18, //
  1, 324, 128, 4,  //
  1, 324, 128, 5,  //
  1, 324, 128, 4,  //
  1, 324, 128, 6,  //
  2, 324, 128, 6,  //

  1, 324, 128, 6, //
  1, 324, 128, 6, //
  1, 324, 128, 6, //
  1, 324, 128, 6, //
  1, 332, 128, 6, //
  1, 340, 128, 6, //
  1, 348, 128, 6, //
  1, 356, 128, 6, //
  1, 364, 128, 6, //
  1, 372, 128, 6, //
  1, 380, 128, 6, //
  1, 388, 128, 6, //
  1, 388, 128, 6, //
  1, 388, 128, 6, //
  1, 388, 128, 6, //

  1, 388, 128, 4,   //
  1, 388, 128, 18,  //
  1, 388, 128, 1,   //
  1, 388, 128, 113, //
  1, 388, 128, 113, //
  1, 388, 128, 113, //
  1, 388, 128, 114, //
  1, 388, 128, 113, //
  1, 388, 128, 115, //
  1, 388, 128, 113, //
  1, 388, 128, 114, //
  1, 388, 128, 113, //
  1, 388, 128, 115, //
  1, 388, 128, 113, //
  1, 388, 128, 114, //
  1, 388, 128, 113, //
  1, 388, 128, 100, //
  1, 388, 128, 100, //
  1, 388, 128, 113, //
  1, 388, 128, 100, //
  1, 388, 128, 100, //
  1, 388, 128, 113, //
  1, 388, 128, 115, //
  1, 388, 128, 113, //
  1, 388, 128, 114, //
  1, 388, 128, 113, //
  2, 388, 128, 113, //

  1, 388, 128, 113, //
  1, 388, 128, 113, //
  1, 388, 128, 113, //
  1, 388, 128, 113, //
  1, 396, 128, 113, //
  1, 404, 128, 113, //
  1, 412, 128, 113, //
  1, 420, 128, 113, //
  1, 428, 128, 113, //
  1, 436, 128, 113, //
  1, 444, 128, 113, //
  1, 449, 128, 113, //
  1, 449, 128, 113, //
  1, 449, 128, 113, //
  1, 449, 128, 113, //

  1, 449, 128, 33, //
  1, 449, 128, 50, //
  1, 449, 128, 36, //
  1, 449, 128, 36, //
  1, 449, 128, 36, //
  1, 449, 128, 36, //
  1, 449, 128, 65, //
  1, 449, 128, 65, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 69, //
  1, 449, 128, 65, //
  1, 449, 128, 65, //
  1, 449, 128, 65, //
  2, 449, 128, 65, //

  1, 449, 128, 65, //
  1, 449, 128, 65, //
  1, 449, 128, 65, //
  1, 449, 128, 65, //
  1, 457, 128, 65, //
  1, 465, 128, 65, //
  1, 473, 128, 65, //
  1, 481, 128, 65, //
  1, 489, 128, 65, //
  1, 497, 128, 65, //
  1, 505, 128, 65, //
  1, 512, 128, 65, //
  1, 512, 128, 65, //
  1, 512, 128, 65, //
  1, 512, 128, 65, //

  1, 512, 128, 36, //
  1, 512, 128, 36, //
  1, 512, 128, 29, //
  1, 512, 128, 29, //
  1, 512, 128, 36, //
  1, 512, 128, 29, //
  1, 512, 128, 36, //
  1, 512, 128, 36, //
  1, 512, 128, 33, //
  1, 512, 128, 50, //
  1, 512, 128, 36, //
  1, 512, 128, 50, //
  1, 512, 128, 33, //
  1, 512, 128, 50, //
  1, 512, 128, 36, //
  1, 512, 128, 50, //
  1, 512, 128, 33, //
  1, 512, 128, 50, //
  1, 512, 128, 36, //
  1, 512, 128, 50, //
  2, 512, 128, 50, //

  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //
  1, 512, 128, 50, //

  0};

/* ----------- */
/* GenericNext */
/* ----------- */

/*
    Effectue l'animation du générique.
    Retourne 1 si c'est fini !
 */

static void
GenericNext (void)
{
  Pixmap * ppm;
  Pt       pos;

  if (tgeneric[generic * 4] == 0)
  {
    ShowImage (); /* réaffiche l'image générique */
    generic = 0;
  }

  if (tgeneric[generic * 4] == 1)
    ppm = NULL;
  else
    ppm = &pmimage;

  pos.x = tgeneric[generic * 4 + 1];
  pos.y = LYIMAGE () - tgeneric[generic * 4 + 2];

  AnimDrawIcon (ppm, tgeneric[generic * 4 + 3], pos, 0);

  generic++;
}

static void
GoodbyeNext (int index)
{
  Pt              pos;
  static int      x[2]     = {0, 0};
  static int      y1[2]    = {LYIMAGE () + 80, LYIMAGE () + 90};
  static int      y2[2]    = {LYIMAGE () + 80 + LYICO, LYIMAGE () + 90 + LYICO};
  static int      icon[2]  = {79, 63};
  static SDL_bool fall[2]  = {SDL_FALSE, SDL_FALSE};
  static int      speed[2] = {8, 12};

  if (!x[index])
  {
    if (index == 0)
      x[index] = GetRandom (0, 0, LXIMAGE () / 2 - LXICO);
    else
      x[index] = GetRandom (0, LXIMAGE () / 2, LXIMAGE () - LXICO);
  }

  pos.x = x[index];

  pos.y = y1[index];
  AnimDrawIcon (NULL, 47, pos, 0);

  pos.y = y2[index];
  AnimDrawIcon (NULL, icon[index], pos, 0);

  y1[index] += -speed[index];
  if (fall[index] && (icon[index] == 28 || y1[index] < GetRandom (0, 0, 80)))
  {
    y1[index] += -speed[index];
    y2[index] += +20;
    icon[index] = 28;
  }
  else
    y2[index] += -speed[index];

  /* restart */
  if (
    (!fall[index] && y2[index] < -LYICO) ||
    (fall[index] && y1[index] < -LYICO * 2 && y2[index] > LYIMAGE ()))
  {
    y1[index]    = LYIMAGE () + 80;
    y2[index]    = LYIMAGE () + 80 + LYICO;
    x[index]     = 0;
    fall[index]  = SDL_FALSE;
    speed[index] = 6 + GetRandom (0, 0, 7);

    int select = GetRandom (0, 0, 3);
    switch (select)
    {
    case 0:
      icon[index] = 79;
      break;
    case 1:
      icon[index] = 63;
      break;
    case 2:
      icon[index] = 79;
      fall[index] = SDL_TRUE;
      break;
    }
  }
}

/* ------------- */
/* ExecuteAction */
/* ------------- */

/*
    Exécute une action (change de phase).
    Retourne 2 s'il faut quitter.
 */

static short
ExecuteAction (char event, Pt pos)
{
  PhAction action;
  short    dstmonde;
  short    lastmusique;

  if (event == KEYCLIC)
  {
    action = ClicToAction (pos); /* action selon la position visée */
  }
  else
  {
    action = EventToAction (event); /* action selon la touche pressée */
  }

  if (g_keyMousePressed)
  {
    PhAction _action = ClicToAction (pos);
    if (_action == ACTION_MONDEBAR)
      action = ACTION_MONDEBAR;
  }

  if (action != -1 && action != ACTION_MONDEBAR)
  {
    PlayEvSound (SOUND_CLIC);
  }

  if (action == ACTION_IDENT)
  {
    ChangePhase (PHASE_IDENT);
    return 0;
  }

  if (action == ACTION_REGLAGE)
  {
    ChangePhase (PHASE_REGLAGE);
    return 0;
  }

  if (action == ACTION_REGLAGE2)
  {
    ChangePhase (PHASE_REGLAGE2);
    return 0;
  }

  if (action == ACTION_DEBUT)
  {
    if (fj.nom[fj.joueur][0] != 0) /* nom du joueur existe ? */
      ChangePhase (PHASE_INIT);
    else
      ChangePhase (PHASE_IDENT);
    return 0;
  }

  if (action == ACTION_FINI)
  {
    lastmusique = musique;
    if (fj.niveau[fj.joueur] == 8) /* privé ? */
    {
      g_monde = maxmonde - 1;
      ChangePhase (PHASE_PRIVE);
      musique = lastmusique;
      return 0;
    }
    if (fj.nom[fj.joueur][0] != 0) /* nom du joueur existe ? */
    {
      ChangePhase (PHASE_INIT);
    }
    else
    {
      ChangePhase (PHASE_IDENT);
    }
    musique = lastmusique;
    return 0;
  }

  if (action >= ACTION_AIDE && action <= ACTION_AIDE42)
  {
    ChangePhase (action - ACTION_AIDE + PHASE_AIDE);
    return 0;
  }

  if (action == ACTION_OBJECTIF)
  {
    if (g_construit)
      ChangePhase (PHASE_PRIVE);
    else
      ChangePhase (PHASE_OBJECTIF);
    return 0;
  }

  if (action == ACTION_JOUE)
  {
    ChangePhase (PHASE_PLAY);
    return 0;
  }

  if (action == ACTION_SUIVANT)
  {
    g_monde++;
    if (fj.progres[fj.joueur][fj.niveau[fj.joueur]] < g_monde)
    {
      fj.progres[fj.joueur][fj.niveau[fj.joueur]] = g_monde;
    }
    JoueurWrite (); /* écrit le fichier des joueurs */
    if (g_monde >= maxmonde)
      g_monde = 0;
    lastmusique = musique;
    if (g_construit)
      ChangePhase (PHASE_PRIVE);
    else
      ChangePhase (PHASE_OBJECTIF);
    musique = lastmusique;
    return 0;
  }

  if (action == ACTION_ANNULE)
  {
    if (g_construit)
      ChangePhase (PHASE_PRIVE);
    else
      ChangePhase (PHASE_INIT);
    return 0;
  }

  if (action == ACTION_STOPPEOK)
  {
    if (fj.progres[fj.joueur][fj.niveau[fj.joueur]] < g_monde + 1)
    {
      fj.progres[fj.joueur][fj.niveau[fj.joueur]] = g_monde + 1;
    }
    JoueurWrite (); /* écrit le fichier des joueurs */
    if (g_construit)
      ChangePhase (PHASE_PRIVE);
    else
      ChangePhase (PHASE_OBJECTIF);
    return 0;
  }

  if (action == ACTION_STOPPEKO)
  {
    if (g_construit)
      ChangePhase (PHASE_PRIVE);
    else
      ChangePhase (PHASE_OBJECTIF);
    return 0;
  }

  if (action == ACTION_PARAM)
  {
    ChangePhase (PHASE_PARAM);
    return 0;
  }

  if (action == ACTION_MONDEPREC)
  {
    if (g_monde > 0)
    {
      g_monde--;
      DrawNumMonde ();             /* affiche le numéro du monde */
      MondeRead (g_monde, banque); /* lit le nouveau monde sur disque */
      DrawObjectif ();             /* affiche l'objectif */
      return 0;
    }
    return 1;
  }

  if (action == ACTION_MONDESUIV)
  {
    if (
      g_monde < maxmonde - 1 &&
      (g_construit || g_monde < fj.progres[fj.joueur][fj.niveau[fj.joueur]]))
    {
      g_monde++;
      DrawNumMonde ();             /* affiche le numéro du monde */
      MondeRead (g_monde, banque); /* lit le nouveau monde sur disque */
      DrawObjectif ();             /* affiche l'objectif */
      return 0;
    }
    return 1;
  }

  if (action == ACTION_MONDEBAR)
  {
    TrackingStatusBar (pos);
    return 0;
  }

  if (action == ACTION_EDIT)
  {
    g_typeedit = 1;
    ChangePhase (PHASE_PLAY);
    return 0;
  }

  if (action == ACTION_OPER)
  {
    ChangePhase (PHASE_OPER);
    return 0;
  }

  if (action == ACTION_DEPLACE)
  {
    ChangePhase (PHASE_DEPLACE);
    return 0;
  }

  if (action == ACTION_ORDRE)
  {
    dstmonde = g_monde;
    ChangePhase (PHASE_ATTENTE); /* affiche "attendez-un instant ..." */
    MondeDeplace (mondeinit, dstmonde);
    ChangePhase (PHASE_PRIVE);
    return 0;
  }

  if (action == ACTION_DUPLIQUE)
  {
    ChangePhase (PHASE_ATTENTE); /* affiche "attendez-un instant ..." */
    MondeDuplique (g_monde);
    ChangePhase (PHASE_PRIVE);
    return 0;
  }

  if (action == ACTION_DETRUIT)
  {
    ChangePhase (PHASE_ATTENTE); /* affiche "attendez-un instant ..." */
    MondeDetruit (g_monde);
    ChangePhase (PHASE_PRIVE);
    return 0;
  }

  if (action >= ACTION_JOUEUR0 && action <= ACTION_JOUEUR3)
  {
    JoueurEditClose (); /* fin de l'édition du nom en cours */
    fj.joueur = action - ACTION_JOUEUR0;
    DrawJoueur ();
    JoueurEditOpen (); /* prépare l'édition du nouveau nom */
    return 0;
  }

  if (
    (action >= ACTION_NIVEAU0 && action <= ACTION_NIVEAU8) ||
    action == ACTION_NIVEAUGO)
  {
    if (action != ACTION_NIVEAUGO)
    {
      fj.niveau[fj.joueur] = action - ACTION_NIVEAU0;
    }

    if (fj.niveau[fj.joueur] < 8) /* fastoche/costaud/durdur/méga ? */
    {
      banque = fj.niveau[fj.joueur] + 'a';
      if (g_passdaniel)
        g_construit = 1;
      else
        g_construit = 0;
      MondeMax (banque);
      g_monde = fj.progres[fj.joueur][fj.niveau[fj.joueur]];
    }
    else /* privé ? */
    {
      banque      = fj.joueur + 'i';
      g_construit = 1;
      MondeMax (banque);
      g_monde = 0;
    }
    if (g_construit)
      ChangePhase (PHASE_PRIVE);
    else
      ChangePhase (PHASE_OBJECTIF);
    return 0;
  }

  if (action == ACTION_NIVEAUK1)
  {
    if (fj.niveau[fj.joueur] == 0)
      fj.niveau[fj.joueur] = 1;
    else
      fj.niveau[fj.joueur] = 0;
    AnimDrawInit ();
    return 0;
  }
  if (action == ACTION_NIVEAUK2)
  {
    if (fj.niveau[fj.joueur] == 2)
      fj.niveau[fj.joueur] = 3;
    else
      fj.niveau[fj.joueur] = 2;
    AnimDrawInit ();
    return 0;
  }
  if (action == ACTION_NIVEAUK3)
  {
    if (fj.niveau[fj.joueur] == 4)
      fj.niveau[fj.joueur] = 5;
    else
      fj.niveau[fj.joueur] = 4;
    AnimDrawInit ();
    return 0;
  }
  if (action == ACTION_NIVEAUK4)
  {
    if (fj.niveau[fj.joueur] == 6)
      fj.niveau[fj.joueur] = 7;
    else
      fj.niveau[fj.joueur] = 6;
    AnimDrawInit ();
    return 0;
  }
  if (action == ACTION_NIVEAUK5)
  {
    fj.niveau[fj.joueur] = 8;
    AnimDrawInit ();
    return 0;
  }

  if (action >= ACTION_VITESSE0 && action <= ACTION_VITESSE2)
  {
    fj.vitesse = action - ACTION_VITESSE0;
    g_settingsOverload &= ~SETTING_SPEEDRATE;
    DrawVitesse ();
    return 0;
  }

  if (action >= ACTION_SCROLL0 && action <= ACTION_SCROLL1)
  {
    fj.scroll = action - ACTION_SCROLL0;
    DrawScroll ();
    return 0;
  }

  if (action >= ACTION_BRUIT0 && action <= ACTION_BRUIT1)
  {
    if (action == ACTION_BRUIT0)
    {
      fj.noisevolume = 10;
      fj.musicvolume = 10;
    }
    else
    {
      fj.noisevolume = 0;
      fj.musicvolume = 0;
    }
    PlayNoiseVolume (fj.noisevolume);
    PlayMusicVolume (fj.musicvolume);
    DrawBruitage ();
    return 0;
  }

  if (action == ACTION_NOISEVOLP && fj.noisevolume < 10)
  {
    fj.noisevolume++;
    PlayNoiseVolume (fj.noisevolume);
    DrawBruitage ();
    PlayAudio (SOUND_MAGIE, NULL);
    return 0;
  }

  if (action == ACTION_NOISEVOLM && fj.noisevolume > 0)
  {
    fj.noisevolume--;
    PlayNoiseVolume (fj.noisevolume);
    DrawBruitage ();
    PlayAudio (SOUND_MAGIE, NULL);
    return 0;
  }

  if (action == ACTION_MUSICVOLP && fj.musicvolume < 10)
  {
    fj.musicvolume++;
    PlayMusicVolume (fj.musicvolume);
    DrawBruitage ();
    return 0;
  }

  if (action == ACTION_MUSICVOLM && fj.musicvolume > 0)
  {
    fj.musicvolume--;
    PlayMusicVolume (fj.musicvolume);
    DrawBruitage ();
    return 0;
  }

  if (action >= ACTION_TELECOM0 && action <= ACTION_TELECOM1)
  {
    fj.modetelecom = action - ACTION_TELECOM0;
    g_modetelecom  = fj.modetelecom;
    DrawTelecom ();
    return 0;
  }

  if (action >= ACTION_LANG_EN && action <= ACTION_LANG_DE)
  {
    fj.language = action - ACTION_LANG_EN;
    DrawLanguage ();
    ChangeLanguage (fj.language);
    PushUserEvent (RESET, NULL);
    return 0;
  }

  if (action >= ACTION_SCREEN_1 && action <= ACTION_SCREEN_FULL)
  {
    fj.screen = action - ACTION_SCREEN_1;
    g_settingsOverload &= ~(SETTING_FULLSCREEN | SETTING_ZOOM);
    DrawScreen ();
    ChangeScreen (fj.screen);
    return 0;
  }

  if (action >= ACTION_THEME_DOS && action <= ACTION_THEME_SMAKY100)
  {
    fj.theme = action - ACTION_THEME_DOS;
    g_settingsOverload &= ~SETTING_THEME;
    DrawTheme ();
    ChangeTheme (fj.theme);
    PushUserEvent (RESET, NULL);
    return 0;
  }

  if (action >= ACTION_COULEUR0 && action <= ACTION_COULEUR4)
  {
    descmonde.color = action - ACTION_COULEUR0;
    DrawCouleur ();
    return 0;
  }

  if (action == ACTION_QUITTE)
  {
    return 2;
  }

  return 1;
}

/* Table pour les musiques de fond */
/* ------------------------------- */

static short tmusic[] = {1, SOUND_MUSIC11, //
                         2, SOUND_MUSIC21, //
                         3, SOUND_MUSIC31, //
                         4, SOUND_MUSIC41, //
                         0};

/* --------------- */
/* MusicBackground */
/* --------------- */

/*
    Gère les musiques de fond, selon la phase.
 */

static void
MusicBackground (void)
{
  short * ptable = tmusic;
  short   sound;

  if (musique == 0)
    return;

  for (int i = SOUND_MUSIC11; i < SOUND_MAX; ++i)
    if (SoundPlaying (i))
      return;

  while (ptable[0] != 0)
  {
    if (musique == ptable[0])
    {
      if (lastaccord == -1)
      {
        sound = 0; /* accord de base */
      }
      else
      {
        do
        {
          sound = GetRandom (1, 0, 8 + 1) / 2; /* sound <- 0..4 */
        } while (sound == lastaccord);
      }
      lastaccord = sound;
      if (sound != 0)
        sound--; /* accord de base plus souvent */

      PlayAudio (ptable[1] + sound, NULL);
      return;
    }
    ptable += 2;
  }
}

static void
LoadTextures ()
{
  g_screen.texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXIMAGE (),
    LYIMAGE ());
  SDL_SetTextureBlendMode (g_screen.texture, SDL_BLENDMODE_BLEND);
  g_screen.dx = LXIMAGE ();
  g_screen.dy = LYIMAGE ();

  g_screenBase.texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXIMAGE (),
    LYIMAGE ());
  SDL_SetTextureBlendMode (g_screenBase.texture, SDL_BLENDMODE_BLEND);
  g_screenBase.dx = LXIMAGE ();
  g_screenBase.dy = LYIMAGE ();

  g_screenAfterglow0.texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXIMAGE (),
    LYIMAGE ());
  SDL_SetTextureBlendMode (g_screenAfterglow0.texture, SDL_BLENDMODE_BLEND);
  g_screenAfterglow0.dx = LXIMAGE ();
  g_screenAfterglow0.dy = LYIMAGE ();

  g_screenAfterglow1.texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXIMAGE (),
    LYIMAGE ());
  SDL_SetTextureBlendMode (g_screenAfterglow1.texture, SDL_BLENDMODE_BLEND);
  g_screenAfterglow1.dx = LXIMAGE ();
  g_screenAfterglow1.dy = LYIMAGE ();

  pmtemp.texture = SDL_CreateTexture (
    g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXICO, LYICO);
  SDL_SetTextureBlendMode (pmtemp.texture, SDL_BLENDMODE_BLEND);
  pmtemp.dx = LXICO;
  pmtemp.dy = LYICO;
}

static void
UnloadTextures ()
{
  GivePixmap (&g_screen);
  GivePixmap (&g_screenBase);
  GivePixmap (&g_screenAfterglow0);
  GivePixmap (&g_screenAfterglow1);
  GivePixmap (&pmtemp);
}

#ifdef USE_CURL
static size_t
updateCallback (void * ptr, size_t size, size_t nmemb, void * data)
{
  size_t            realsize = size * nmemb;
  struct url_data * mem      = data;

  mem->buffer = (char *) (realloc (mem->buffer, mem->size + realsize + 1));
  if (mem->buffer)
  {
    memcpy (&(mem->buffer[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->buffer[mem->size] = 0;
  }

  return realsize;
}

static int
progressCallback (
  void * userData, double dltotal, double dlnow, double ultotal, double ulnow)
{
  return g_updateAbort ? 1 : 0;
}
#endif /* USE_CURL */

#ifdef USE_CURL
static int
CheckForUpdates (void * data)
{
  struct url_data chunk;

  chunk.buffer = NULL; /* we expect realloc(NULL, size) to work */
  chunk.size   = 0;    /* no data at this point */
  chunk.status = CURLE_FAILED_INIT;

  CURL * curl = curl_easy_init ();
  if (!curl)
    return 0;

  curl_easy_setopt (curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1);
  curl_easy_setopt (curl, CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt (curl, CURLOPT_TIMEOUT, 20);
  curl_easy_setopt (curl, CURLOPT_CONNECTTIMEOUT, 5);

  curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, updateCallback);

  curl_easy_setopt (curl, CURLOPT_URL, "http://blupi.org/update/mania.txt");
  curl_easy_setopt (curl, CURLOPT_WRITEDATA, (void *) &chunk);

  curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 0);
  curl_easy_setopt (curl, CURLOPT_PROGRESSDATA, NULL);
  curl_easy_setopt (curl, CURLOPT_PROGRESSFUNCTION, progressCallback);

  chunk.status = curl_easy_perform (curl);

  if (chunk.status)
  {
    const char * err = curl_easy_strerror (chunk.status);
    SDL_LogError (
      SDL_LOG_CATEGORY_APPLICATION, "Check for updates, error: %s", err);
  }
  else
  {
    char * res = SDL_strdup (chunk.buffer);
    PushUserEvent (CHECKUPDATE, res);
  }

  if (chunk.buffer)
    free (chunk.buffer);

  curl_easy_cleanup (curl);
  return 0;
}
#endif /* USE_CURL */

/* ======== */
/* PlayInit */
/* ======== */

/*
    Initialise le jeu.
    Retourne != 0 en cas d'erreur.
 */

static int
PlayInit (int argc, char * argv[])
{
  int rc = OpenMachine (argc, argv, &arguments); /* ouverture générale */
  if (rc)
    return rc == -1 ? 0 : rc;

  LoadTextures ();

  g_monde       = 0;   /* premier monde */
  banque        = 'a'; /* banque de base */
  phase         = -1;  /* pas de phase connue */
  g_pause       = 0;
  g_construit   = 0;
  g_typejeu     = 0; /* jeu sans télécommande */
  g_typeedit    = 0; /* pas d'édition en cours */
  g_typetext    = 0; /* pas d'édition de ligne */
  g_modetelecom = 0;
  lastkey       = 0;
  g_passdaniel  = 0; /* mode normal (sans passe-droit) */
  g_passpower   = 0; /* mode normal (sans passe-droit) */
  g_passnice    = 0; /* mode normal (sans passe-droit) */
  g_passhole    = 0; /* mode normal (sans passe-droit) */
  animpb        = 0; /* pas d'animation en cours */
  animpt        = 0; /* pas d'animation en cours */
  generic       = 0; /* pas du générique */

  InitRandomEx (1, 1, 4 + 1, musiquehex); /* init hazard musique exclusive */

  MondeVide ();

  BlackScreen (); /* efface l'écran */

#ifdef USE_CURL
  g_updateThread = SDL_CreateThread (CheckForUpdates, "CheckForUpdates", NULL);
#endif /* USE_CURL */

  return ChangePhase (PHASE_GENERIC); /* première phase du jeu */
}

/* ========= */
/* PlayEvent */
/* ========= */

/*
    Donne un événement.
    Retourne 1 si l'action est terminée.
    Retourne 2 si le jeu est terminé.
 */

static short
PlayEvent (int key, Pt pos, SDL_bool next)
{
  char      ev;
  Pt        ovisu;
  short     term = 1, max, delai, last;
  KeyStatus keystatus;

  static char * pass[] = {"petitblupi",  "enigmeblupi", "totalblupi",
                          "gentilblupi", "sauteblupi",  "megablupi"};

  if (phase == PHASE_GENERIC)
  {
    MusicBackground (); /* gère la musique de fond */
    if (next)
      GenericNext (); /* anime le générique */
    if (key != 0)
      ChangePhase (PHASE_IDENT);
    return 1;
  }

  if (phase == PHASE_GOODBYE)
  {
    if (
      key == KEYCLIC || key == KEYENTER || key == KEYRETURN ||
      key == KEYCENTER || key == KEYQUIT || key == KEYHOME || key == KEYUNDO)
      return 2;

    if (next)
    {
      ShowImage ();
      for (int i = 0; i < 2; ++i)
        GoodbyeNext (i);
    }
  }

  if (phase != PHASE_PLAY)
  {
    g_timerSkip = 2; /* Use the normal speed in the menus */

    if (
      phase == PHASE_INIT &&
      ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')))
    {
      if (passindex == 0)
      {
        for (passrang = 0; passrang < 6; passrang++)
        {
          if (key == pass[passrang][0])
            break;
        }
      }
      if (passrang < 6 && key == pass[passrang][passindex])
      {
        passindex++;
        if (pass[passrang][passindex] == 0) /* fin du mot de passe ? */
        {
          passindex = 0;
          switch (passrang)
          {
          case 1:
            g_passdaniel = 1; /* passe-droit spécial */
            break;
          case 2:
            g_passpower = 1; /* passe-droit spécial */
            break;
          case 3:
            g_passnice = 1; /* passe-droit spécial */
            break;
          case 4:
            g_passhole = 1; /* passe-droit spécial */
            break;
          case 5:
            g_passdaniel = 1; /* passe-droit spécial */
            g_passpower  = 1; /* passe-droit spécial */
            g_passnice   = 1; /* passe-droit spécial */
            g_passhole   = 1; /* passe-droit spécial */
            break;
          default:
            g_passdaniel = 0; /* plus de passe-droit */
            g_passpower  = 0;
            g_passnice   = 0;
            g_passhole   = 0;
          }

          PlayAudio (SOUND_MAGIE, NULL);

          for (max = 0; max < 10; max++)
          {
            /* flash */
            if (max % 2)
              SDL_RenderCopy (g_renderer, g_screen.texture, NULL, NULL);
            else
              SDL_RenderClear (g_renderer);
            SDL_RenderPresent (g_renderer);
            SDL_Delay (30);
          }

          ChangePhase (PHASE_INIT);
          return 1;
        }
      }
      else
      {
        passindex = 0;
      }
    }

    if (phase == PHASE_IDENT && !!g_updateVersion[0])
    {
      static SDL_bool clear = SDL_FALSE;
      Pt              dest  = {LYIMAGE () - 15, 19};

      if (g_updateBlinking % 60 < 30)
      {
        if (!clear)
        {
          Pt pos = dest;
          pos.x += 5;
          pos.y += 10;
          DrawUpdate (g_updateVersion, pos);
          clear = SDL_TRUE;
        }
      }
      else if (clear)
      {
        Pixmap pixmap = {0};
        int    image  = ConvPhaseToNumImage (phase);
        int    err    = GetImage (&pixmap, image, NORMAL);
        if (err)
          FatalBreak (err);

        Pt dim = {13, 370};
        CopyPixel (&pixmap, dest, 0, dest, dim);
        GivePixmap (&pixmap);
        clear = SDL_FALSE;
      }

      g_updateBlinking++;
    }

    if (
      phase == PHASE_INIT && key == KEYQUIT && StopPartie (key, pos) == KEYHOME)
    {
      ChangePhase (PHASE_GOODBYE);
      return 1;
    }

    if (key == KEYQUIT || key == KEYHOME || key == KEYUNDO)
    {
      ChangePhase (PHASE_INIT);
      return 1;
    }

    if (phase == PHASE_IDENT && key != KEYRETURN)
    {
      if (EditEvent (key, pos) >= 0)
        return 1;
    }

    if (phase == PHASE_PARAM)
    {
      if (PaletteEditEvent (descmonde.palette, key, pos) == 0)
        return 1;
      if (EditEvent (key, pos) >= 0)
        return 1;
    }

    if (!g_stopMenu)
      term = ExecuteAction (key, pos);

    if ((term == 2 || g_stopMenu) && StopPartie (key, pos) == KEYHOME)
    {
      ChangePhase (PHASE_GOODBYE);
      return 1;
    }

    if (term != 0 && !g_stopMenu)
    {
      MusicBackground (); /* gère la musique de fond */
      if (next)
        AnimTracking (pos); /* tracking de l'animation */
    }

    return 1;
  }
  else
  {
    SDL_bool forPalette = SDL_FALSE;
    if (g_ignoreKeyClicUp == SDL_TRUE)
    {
      /* Prevent key up just when entering in the play phase */
      if (key == KEYCLICREL)
      {
        g_ignoreKeyClicUp = SDL_FALSE;
        key               = 0;
      }
    }
    if (
      key == KEYCLIC || key == KEYCLICREL || g_keyMousePressed ||
      g_keyFunctionUp == 1 || (key >= KEYF4 && key <= KEYF1))
    {
      ev = PaletteEvent (key, g_lastmouse);
      if (key == KEYCLIC || key == KEYCLICREL || (key >= KEYF4 && key <= KEYF1))
        forPalette = SDL_TRUE;
    }
    if (g_keyFunctionUp)
      g_keyFunctionUp--;
    if (forPalette)
    {
      if (g_typejeu == 0 || g_typeedit)
      {
        if (ev < 0)
          key = ev;
        if (ev == 1)
        {
          DecorEvent (g_keyMousePos, 0, PaletteGetPress (), key);
        }
      }
      else
      {
        if (ev < 0)
          key = ev;
        if (ev == 2)
        {
          MoveBuild (PaletteGetPress (), key);
          g_keyFunctionUp = 5;
        }
      }

      if (
        key == KEYGOLEFT || key == KEYGORIGHT || key == KEYGOFRONT ||
        key == KEYGOBACK)
        lastkey = key;
      fromClic = SDL_TRUE;
    }

    if (
      g_saveMenu ||
      (g_typeedit == 0 && (key == KEYSAVE || key == KEYLOAD || key == KEYIO)))
    {
      PartieDisque (key, pos); /* prend/sauve la partie en cours ... */
      DrawArrows (0);          /* dessine les manettes de la télécommande */
      lastkey  = 0;
      fromClic = SDL_FALSE;
      return 1;
    }

    if (key == KEYF5) /* bruitages oui/non */
    {
      if (fj.noisevolume == 0)
      {
        if (lastnoisevolume == 0)
          lastnoisevolume = 10 - 3;
        if (lastmusicvolume == 0)
          lastmusicvolume = 10 - 3;
        fj.noisevolume = lastnoisevolume;
        fj.musicvolume = lastmusicvolume;
        PlayNoiseVolume (fj.noisevolume);
        PlayMusicVolume (fj.musicvolume);
      }
      else
      {
        lastnoisevolume = fj.noisevolume;
        lastmusicvolume = fj.musicvolume;
        fj.noisevolume  = 0;
        fj.musicvolume  = 0;
        PlayNoiseVolume (0);
        PlayMusicVolume (0);
      }
      return 1;
    }

    if (key == KEYF6) /* décalage progressif/rapide */
    {
      if (fj.scroll)
        fj.scroll = 0;
      else
        fj.scroll = 1;
      return 1;
    }

    if (key == KEYF7) /* vitesse = tortue */
    {
      fj.vitesse = 0;
      return 1;
    }
    if (key == KEYF8) /* vitesse = normal */
    {
      fj.vitesse = 1;
      return 1;
    }
    if (key == KEYF9) /* vitesse = guépard */
    {
      fj.vitesse = 2;
      return 1;
    }

    if (g_typejeu == 1 && fj.modetelecom == 1 && g_pause == 0 && !fromClic)
    {
      keystatus = GetKeyStatus ();
      if (keystatus != 0)
      {
        if (keystatus == STLEFT)
          key = KEYGOLEFT;
        if (keystatus == STRIGHT)
          key = KEYGORIGHT;
        if (keystatus == STUP)
          key = KEYGOFRONT;
        if (keystatus == STDOWN)
          key = KEYGOBACK;
        lastkey = key;
      }
      else
      {
        if (lastkey != 0)
          key = KEYCLICREL;
        lastkey = 0;
      }
    }

    if (
      key == KEYGOFRONT || key == KEYGOBACK || key == KEYGOLEFT ||
      key == KEYGORIGHT)
    {
      DrawArrows (key); /* dessine les manettes de la télécommande */
    }

    if (key == KEYCLICREL)
    {
      DrawArrows (0); /* dessine les manettes de la télécommande */
      lastkey  = 0;
      fromClic = SDL_FALSE;
    }

    if (g_typejeu == 1 && g_pause == 0)
    {
      if (lastkey)
        key = lastkey;
    }

    if (g_stopMenu || key == KEYQUIT || key == KEYHOME || key == KEYUNDO)
    {
      int stop = 0;
      fromClic = SDL_FALSE;
      SoundPause ();
      if (g_typeedit == 1 || (stop = StopPartie (key, pos)) == KEYHOME)
      {
        if (g_typeedit)
          ChangePhase (PHASE_PRIVE);
        else
          ChangePhase (PHASE_RECOMMENCE);
        return 1;
      }
      if (stop == KEYUNDO)
      {
        SoundResume ();
        return 1;
      }
    }

    if (g_subMenu || g_stopMenu || g_saveMenu)
      return 1;

    if (g_typejeu == 0 || g_typeedit || g_pause)
    {
      ovisu = DecorGetOrigine ();
      if (key == KEYRIGHT && ovisu.x > -8)
      {
        PlayEvSound (SOUND_CLIC);
        ovisu.x -= 4;
        DecorSetOrigine (ovisu, fj.scroll);
      }
      if (key == KEYLEFT && ovisu.x < 16)
      {
        PlayEvSound (SOUND_CLIC);
        ovisu.x += 4;
        DecorSetOrigine (ovisu, fj.scroll);
      }
      if (key == KEYDOWN && ovisu.y > -20)
      {
        PlayEvSound (SOUND_CLIC);
        ovisu.y -= 5;
        DecorSetOrigine (ovisu, fj.scroll);
      }
      if (key == KEYUP && ovisu.y < 0)
      {
        PlayEvSound (SOUND_CLIC);
        ovisu.y += 5;
        DecorSetOrigine (ovisu, fj.scroll);
      }
      if (key == KEYCENTER)
      {
        PlayEvSound (SOUND_CLIC);
        ovisu.x = 4;
        DecorSetOrigine (ovisu, fj.scroll);
        ovisu.y = -10;
        DecorSetOrigine (ovisu, fj.scroll);
      }
    }
    else
    {
      last = g_typejeu;
      MoveScroll (fj.scroll); /* décale év. selon le toto du joueur */
      if (last != g_typejeu)  /* type de jeu changé ? */
        DrawArrows (0);       /* oui -> remet les flèches/télécommande */
    }

    if (key == KEYPAUSE && g_typeedit == 0)
    {
      PlayEvSound (SOUND_CLIC);
      g_pause ^= 1;   /* met/enlève la pause */
      DrawPause ();   /* dessine le bouton pause */
      DrawArrows (0); /* dessine les flèches */
      if (g_pause)
        SoundPause ();
      else
        SoundResume ();
    }

    switch (fj.vitesse)
    {
    case 0:
      delai       = DELSLOW;
      g_timerSkip = 4;
      break;
    case 2:
      delai       = DELQUICK;
      g_timerSkip = 0;
      break;
    default:
      delai       = DELNORM;
      g_timerSkip = 2;
      break;
    }

    if (g_pause == 0 && next)
    {
      IconDrawOpen ();
      DecorSuperCel (pos);        /* indique la cellule visée par la souris */
      term = MoveNext (key, pos); /* anime jusqu'au pas suivant */
      IconDrawClose (1);

      if (term == 1) /* terminé gagné ? */
      {
        if (g_construit)
          max = maxmonde - 1;
        else
          max = maxmonde;
        if (g_monde >= max - 1)
        {
          ChangePhase (PHASE_FINI0 + fj.niveau[fj.joueur]);
          fj.resolved[fj.joueur][fj.niveau[fj.joueur]] = SDL_TRUE;
          JoueurWrite();
          return 1;
        }
        ChangePhase (PHASE_SUIVANT);
        musique    = GetRandomEx (1, 1, 4 + 1, musiquehex);
        lastaccord = -1;
        return 1;
      }
      if (term == 2) /* terminé perdu ? */
      {
        max = 0;
        do
        {
          IconDrawOpen ();
          MoveNext (key, pos); /* anime jusqu'au pas suivant */
          IconDrawClose (1);
          max += delai;
        } while (max < 100); /* attend une seconde ... */

        if (g_typeedit)
          ChangePhase (PHASE_PRIVE);
        else
          ChangePhase (PHASE_RECOMMENCE);
        return 1;
      }
    }
    if (g_pause != 0 || (g_pause == 0 && !next))
    {
      IconDrawOpen ();
      DecorSuperCel (pos); /* indique la cellule visée par la souris */
      MoveRedraw ();       /* redessine sans changement */
      IconDrawClose (1);
    }
  }
  return 1;
}

/* ---------- */
/* FatalBreak */
/* ---------- */

/*
    Quitte le jeu après une erreur fatale !
 */

void
FatalBreak (short err)
{
  PlayRelease (); /* libère tout */
}

/* =========== */
/* PlayRelease */
/* =========== */

/*
    Fermeture générale.
 */

static void
PlayRelease (void)
{
  StartRandom (0, 1);
  PlayAudio (GetRandom (0, SOUND_SAUT1, SOUND_CAISSEG + 1), NULL);

  BlackScreen (); /* efface tout l'écran */

  GivePixmap (&pmimage);
  pmimageNum = -1;

  UnloadTextures ();
  DecorClose (); /* fermeture des décors */
  IconClose ();  /* fermeture des icônes */

  if (g_updateThread)
  {
    g_updateAbort = SDL_TRUE;

    int threadReturnValue;
    SDL_WaitThread (g_updateThread, &threadReturnValue);
  }

  CloseMachine (&arguments); /* fermeture générale */
}

void
PushUserEvent (Sint32 code, void * data)
{
  SDL_Event event;

  event.type       = SDL_USEREVENT;
  event.user.code  = code;
  event.user.data1 = data;
  event.user.data2 = NULL;

  SDL_PushEvent (&event);
}

static Uint32
MainLoop (Uint32 interval, void * param)
{
  static int skip;
  PushUserEvent (FRAME_TICK, NULL);
  ++skip;
  return interval;
}

void
Render ()
{
  SDL_Texture * target;

  if (!g_afterglow)
  {
    SDL_RenderCopy (g_renderer, g_screen.texture, NULL, NULL);
    SDL_RenderPresent (g_renderer);
    return;
  }

  // Copy new texture to base texture
  target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, g_screenBase.texture);
  SDL_RenderCopy (g_renderer, g_screen.texture, NULL, NULL);
  SDL_SetRenderTarget (g_renderer, target);

  // Apply previous (green) tetxure on the base texture
  target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, g_screenBase.texture);
  SDL_SetTextureAlphaMod (g_screenAfterglow0.texture, 160);
  SDL_RenderCopy (g_renderer, g_screenAfterglow0.texture, NULL, NULL);
  SDL_SetRenderTarget (g_renderer, target);

  // Apply previous older (green) tetxure on the base texture
  target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, g_screenBase.texture);
  SDL_SetTextureAlphaMod (g_screenAfterglow1.texture, 80);
  SDL_RenderCopy (g_renderer, g_screenAfterglow1.texture, NULL, NULL);
  SDL_SetRenderTarget (g_renderer, target);

  // Show the base texture
  SDL_RenderCopy (g_renderer, g_screenBase.texture, NULL, NULL);
  SDL_RenderPresent (g_renderer);

  // Save the previous (green) texture as older texture
  target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, g_screenAfterglow1.texture);
  SDL_RenderCopy (g_renderer, g_screenAfterglow0.texture, NULL, NULL);
  SDL_SetRenderTarget (g_renderer, target);

  // Save the current texture as next previous (green) texture
  target = SDL_GetRenderTarget (g_renderer);
  SDL_SetRenderTarget (g_renderer, g_screenAfterglow0.texture);
  SDL_RenderCopy (g_renderer, g_screen.texture, NULL, NULL);
  SDL_SetRenderTarget (g_renderer, target);
}

/* =================== */
/* Programme principal */
/* =================== */

int
main (int argc, char * argv[])
{
  int   err;     /* condition de sortie */
  short key = 0; /* touche pressée  */

  int rc = PlayInit (argc, argv); /* initialise le jeu */
  if (rc)
    return rc;

  SDL_TimerID updateTimer = SDL_AddTimer (g_timerInterval, &MainLoop, NULL);

  SDL_Event event;
  SDL_bool  next        = SDL_FALSE;
  int       nextKeys[4] = {0};
  int       skip        = 0;
  while (SDL_WaitEvent (&event))
  {
    next = SDL_FALSE;

    if (
      event.type == SDL_RENDER_DEVICE_RESET ||
      event.type == SDL_RENDER_TARGETS_RESET ||
      (event.type == SDL_USEREVENT && event.user.code == RESET))
    {
      BlackScreen ();

      UnloadSprites ();
      UnloadTextures ();
      UnloadDecor ();

      Style style = phase == PHASE_PLAY ? descmonde.color : NORMAL;

      LoadSprites (style);
      LoadTextures ();
      LoadDecor ();

      GivePixmap (&pmimage);
      if (pmimageNum > -1)
      {
        int err = GetImage (&pmimage, pmimageNum, style);
        if (err)
          FatalBreak (err);
      }

      RedrawPhase (phase);
      continue;
    }

    if (event.type == SDL_MOUSEMOTION)
    {
      SDL_MouseMotionEvent * _event = (SDL_MouseMotionEvent *) &event;
      g_lastmouse.x                 = _event->x;
      g_lastmouse.y                 = _event->y;
      continue;
    }

    if (event.type == SDL_USEREVENT && event.user.code == CHECKUPDATE)
    {
      int          res;
      int          v1, v2, v3;
      const char * update = event.user.data1;

      if (!update)
        continue;

      res = sscanf (update, "%d.%d.%d", &v1, &v2, &v3);
      if (res != 3)
        continue;

      if (
        (!!BM_VERSION_EXTRA[0] &&
         BM_VERSION_INT (v1, v2, v3) >= BLUPIMANIA_VERSION_INT) ||
        BM_VERSION_INT (v1, v2, v3) > BLUPIMANIA_VERSION_INT)
        snprintf (
          g_updateVersion, sizeof (g_updateVersion), "%d.%d.%d", v1, v2, v3);

      free ((char *) update);
      continue;
    }

    if (event.type == SDL_WINDOWEVENT)
    {
#ifndef DEBUG
      static const char * stop[3] = {
        "Blupimania -- stop", "Blupimania -- arrêt", "Blupimania -- Halt"};

      switch (event.window.event)
      {
      case SDL_WINDOWEVENT_FOCUS_GAINED:
        g_standby = SDL_FALSE;
        SDL_SetWindowTitle (g_window, "Blupimania");
        MusicResume ();
        SoundResume ();
        continue;

      case SDL_WINDOWEVENT_FOCUS_LOST:
        g_standby = SDL_TRUE;
        SDL_SetWindowTitle (g_window, stop[g_langue]);
        MusicPause ();
        SoundPause ();
        continue;
      }
#endif /* !DEBUG */
    }

    if (
      event.type == SDL_USEREVENT && event.user.code == FRAME_TICK &&
      (!g_timerSkip || !(skip++ % g_timerSkip)) && !g_standby)
    {
      next        = SDL_TRUE;
      key         = nextKeys[0];
      nextKeys[0] = nextKeys[1];
      nextKeys[1] = nextKeys[2];
      nextKeys[2] = nextKeys[3];
      nextKeys[3] = 0;
    }
    else if (event.type == SDL_USEREVENT && event.user.code == MUSIC_STOP)
    {
      if (!MusicStoppedOnDemand ())
        MusicStart (4);
      continue;
    }
    else if (!g_standby)
    {
      key = SDLEventToSmakyKey (&event);
      /* Ensure that the action is done on the next (game) frame */
      if (
        phase == PHASE_PLAY &&
        (key == KEYCLIC || key == KEYCLICR || key == KEYCLICREL ||
         key == KEYLEFT || key == KEYRIGHT || key == KEYUP || key == KEYDOWN))
      {
        for (int i = 0; i < 4; ++i)
          if (!nextKeys[i])
          {
            nextKeys[i] = key;
            break;
          }
        continue;
      }
    }

    Render ();

    err = PlayEvent (key, g_lastmouse, next); /* fait évoluer le jeu */
    if (err == 2)
      break; /* quitte si terminé */
    if (event.type == SDL_QUIT)
      break;

    if (g_clearKeyEvents)
    {
      memset (nextKeys, 0, sizeof (nextKeys));
      g_clearKeyEvents = SDL_FALSE;
    }
  }

  SDL_RemoveTimer (updateTimer);
  PlayRelease (); /* fermeture générale */
  return 0;
}


/* ========== */
/* bm_smaky.c */
/* ========== */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/file.h>
#include <errno.h>
#include <math.h>
#include <time.h>

#include <SDL2/SDL_mixer.h>
#include "bm.h"
#include "actions.h"




#define GCTEXT		{'N','E'+0x80,11} /* NEF11 */



/* ---------------------------- */
/* Constantes globales internes */
/* ---------------------------- */

#define MEMBW			820000		/* mmoire ncessaire pour noir-blanc */
#define MEMCOLOR		2048000		/* mmoire ncessaire pour couleur */
#define MEMRUNBW		273000		/* mmoire ncessaire pour variables */
#define MEMRUNCOLOR		385000		/* mmoire ncessaire pour variables */
#define MEMAUDIO		200000		/* mmoire ncessaire pour audio */
#define MEMBRUIT		390000		/* mmoire ncessaire pour BLUPIX01.AUDIO */
#define MEMMUSIC		290000		/* mmoire ncessaire pour BLUPIX02.AUDIO */

#define KSHIFT		(1<<5)			/* valeur  ajouter si SHIFT */




/* ----------------------------- */
/* Structure d'une image charge */
/* ----------------------------- */

typedef struct
{
  void		*head;			/* pointe l'en-tte */
  char		*clut;			/* pointe la clut */
  char		*data;			/* pointe l'image code */
}
ImageSmaky;



/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Pixmap		pmicon1c = {0};	/* pixmap des icnes1 (chair) */
static Pixmap		pmicon2c = {0};	/* pixmap des icnes2 (chair) */
static Pixmap		pmicon3c = {0};	/* pixmap des icnes3 (chair) */
static Pixmap		pmicon4c = {0};	/* pixmap des icnes4 (chair) */
static Pt			origine;					/* coin sup/gauche de l'origine */

static unsigned int		nextrand[10];				/* valeurs alatoires suivantes */

static short		colormode = 1;				/* 1 = couleur possible */

static short		soundon = 1;				/* son on/off */
static int			filsson = 0;				/* son  entendre */
static int			filsrep = 0;				/* pas de rptition */
static KeyStatus	keystatus;					/* tat des flches du clavier */

static Pixmap		pmsave = {0};	/* pixmap sauv en mmoire tendue (XMS) */

static short g_soundVolume = 0;
static short g_musicVolume = 0;
static Mix_Chunk *g_sounds[SOUND_MAX] = {NULL};
static Mix_Music *g_music = NULL;
static SDL_bool g_musicStopped = SDL_FALSE;

static const SDL_Color g_colors[] = {
  {255,255,255,SDL_ALPHA_OPAQUE}, // BLANC
  {255,255,0,SDL_ALPHA_OPAQUE}, // JAUNE
  {255,204,64,SDL_ALPHA_OPAQUE}, // ORANGE
  {255,0,0,SDL_ALPHA_OPAQUE}, // ROUGE
  {220,220,220,SDL_ALPHA_OPAQUE}, // GRIS CLAIR
  {190,190,190,SDL_ALPHA_OPAQUE}, // GRIS FONCE
  {0,255,255,SDL_ALPHA_OPAQUE}, // CYAN
  {0,0,255,SDL_ALPHA_OPAQUE}, // BLEU
  {0,255,0,SDL_ALPHA_OPAQUE}, // VERT CLAIR
  {0,205,0,SDL_ALPHA_OPAQUE}, // VERT FONCE
  {224,161,255,SDL_ALPHA_OPAQUE}, // VIOLET
  {255,0,255,SDL_ALPHA_OPAQUE}, // MAGENTA
  {224,164,164,SDL_ALPHA_OPAQUE}, // BRUN CLAIR
  {187,0,0,SDL_ALPHA_OPAQUE}, // BRUN FONCE
  {169,216,255,SDL_ALPHA_OPAQUE}, // BLEU MOYEN
  {0,0,0,SDL_ALPHA_OPAQUE}, // NOIR
};



/* ========= */
/* GetRandom */
/* ========= */

/*
	Retourne une valeur alatoire comprise entre min et max-1.
	min <= h < max
 */

short GetRandom (short g, short min, short max)
{
	nextrand[g] = nextrand[g] * 1103515245 + 12345;
	return (short)min + (nextrand[g]/65536)%(max-min);
}


/* ============ */
/* InitRandomEx */
/* ============ */

/*
	Initialise un tirage exclusif.
 */

void InitRandomEx (short g, short min, short max, char *pex)
{
	short		i;

	for( i=0 ; i<(max-min) ; i++ )
    {
		pex[i] = 0;			/* met tout le tableau  zro */
    }
}


/* =========== */
/* GetRandomEx */
/* =========== */

/*
	Retourne une valeur alatoire exclusive.
 */

short GetRandomEx (short g, short min, short max, char *pex)
{
	short		i, val;

	val = GetRandom(g, 0, max-min);		/* cherche une valeur quelconque */

	for( i=0 ; i<(max-min) ; i++ )
	{
		if ( pex[val] == 0 )			/* valeur dj sortie ? */
		{
			pex[val] = 1;				/* indique valeur utilise */
			return min+val;
		}
		else
		{
			val ++;
			if ( val == max-min )  val = 0;
		}
	}

	InitRandomEx(g, min, max, pex);		/* recommence */
	val = GetRandom(g, 0, max-min);		/* cherche une valeur quelconque */
	pex[val] = 1;						/* indique valeur utilise */
	return min+val;
}


/* =========== */
/* StartRandom */
/* =========== */

/*
	Coup de sac du gnrateur alatoire.
		mode = 0	->	toujours la mme chose
		mode = 1	->	alatoire 100%
 */

void StartRandom (short g, short mode)
{
	if ( mode == 0 )
	{
		nextrand[g] = 33554393;		/* grand nombre premier */
	}
	else
	{
		nextrand[g] = time(NULL);
	}
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

void MusicStart (short song)
{
  static int prev = -1;

  if (song < 3)
    return;

  static const char* musics[] = {
    "bmx000.ogg",
    "bmx001.ogg",
    "bmx002.ogg",
    "bmx003.ogg",
    "bmx004.ogg",
    "bmx005.ogg",
    "bmx006.ogg",
    "bmx007.ogg",
    "bmx008.ogg",
    "bmx009.ogg",
    "bmx010.ogg",
    "bmx011.ogg",
  };

  int idx = 0;

  if (song != 3)
  {
    while ((idx = GetRandom(1, 0, countof(musics))) == prev)
      ;
    prev = idx;
  }

  char filename[4096];
  snprintf(filename, sizeof(filename), "%s../share/blupimania/music/%s", SDL_GetBasePath (), musics[idx]);

  if (g_music)
    Mix_FreeMusic (g_music);

  g_music = Mix_LoadMUS(filename);
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

void MusicStop (void)
{
  for (int i = SOUND_MUSIC11; i < SOUND_MAX; ++i)
    Mix_HaltChannel(i);

  Mix_HaltMusic ();
  g_musicStopped = SDL_TRUE;
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
	Dtermine le volume des bruitages (0..10).
 */

void PlayNoiseVolume (short volume)
{
	if ( volume == 0 )  soundon = 0;
	else                soundon = 1;
  g_soundVolume = volume * 10;
}

/* =============== */
/* PlayMusicVolume */
/* =============== */

/*
	Dtermine le volume de la musique de fond (0..10).
 */

void PlayMusicVolume (short volume)
{
  g_musicVolume = volume * 10;
  Mix_VolumeMusic (g_musicVolume);
}


/* =========== */
/* IfPlayReady */
/* =========== */

/*
	Test s'il est possible de donner un nouveau son  entendre.
	Si oui, retourne 1 (true).
 */

short IfPlayReady (void)
{
	/*if ( filsson == 0 )*/  return 1;
	return 0;
}


/* ============= */
/* PlaySoundLoop */
/* ============= */

/*
	Met en boucle (rptition) les bruits donns avec PlaySound.
	Si mode =  0  ->  normal (single)
	Si mode >  0  ->  nb de rptitions
	Si mode = -1  ->  rptition infinie (loop)
 */

void PlaySoundLoop (short mode)
{
	filsrep = mode;			/* donne le mode au processus fils */
}


/* ========= */
/* PlaySound */
/* ========= */

/*
	Fait entendre un bruit quelconque.
 */

void PlaySound (short sound, const Pt * cel)
{
	if ( soundon == 0 )  return;
	filsson = sound;			/* donne le numro au processus fils */

  if (sound < 1 || sound >= SOUND_MAX)
    return;

  if (!g_sounds[sound])
    return;

  if (SoundPlaying(sound))
    return;

  Sint32 volume = g_soundVolume;

  if (cel)
  {
    Pt pos = CelToGra2(*cel, SDL_TRUE);
    pos.x += LXICO / 2;
    pos.y += LYICO / 2;

    Sint32 volumex, volumey;
    Uint8 panRight, panLeft;
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
  Mix_PlayChannel(sound, g_sounds[sound], 0);
}

SDL_bool
SoundPlaying(short sound)
{
  return Mix_Playing(sound);
}

/* ========= */
/* ClrEvents */
/* ========= */

/*
	Vide le fifo du clavier.
 */

void ClrEvents (void)
{
  g_clearKeyEvents = SDL_TRUE;
}


/* ======== */
/* GetEvent */
/* ======== */

/*
	Lecture d'un vnement du clavier, sans attendre.
	Retourne la touche presse si != 0.
 */

short GetEvent (Pt *ppos)
{
	long		key;

	*ppos = g_lastmouse;					/* rend la dernire position de la souris */
#if 0
	N_settim(0);						/* met un timeout nul */
	key = L_getkey();					/* lecture du clavier sans attendre */
	N_settim(0xFFFF);					/* remet un timeout infini */
#endif
	//lastmouse = getmousepos();			/* lit la position de la souris */
#if 0
	if ( errno )			return 0;	/* retourne si timeout */

	if ( key>>16 == 'D' )  keystatus = STLEFT;
	if ( key>>16 == 'F' )  keystatus = STRIGHT;
	if ( key>>16 == 'R' )  keystatus = STUP;
	if ( key>>16 == 'C' )  keystatus = STDOWN;

	if ( key>>16 == 0x74 )  keystatus = STLEFT;
	if ( key>>16 == 0x76 )  keystatus = STRIGHT;
	if ( key>>16 == 0x78 )  keystatus = STUP;
	if ( key>>16 == 0x72 )  keystatus = STDOWN;

	if ( g_typejeu == 1 && g_modetelecom == 0 && g_pause == 0 )	/* tlcommand en gauche/droite ? */
	{
		if ( key>>16 == 0x77 )  keystatus = STLEFT;
		if ( key>>16 == 0x73 )  keystatus = STRIGHT;
		if ( key>>16 == 0x79 )  keystatus = STUP;
		if ( key>>16 == 0x71 )  keystatus = STDOWN;
	}

	if ( key & 0x8000 )					/* touche relche ? */
	{
		if ( key>>16 == ('D'|0x80) )  keystatus &= ~STLEFT;
		if ( key>>16 == ('F'|0x80) )  keystatus &= ~STRIGHT;
		if ( key>>16 == ('R'|0x80) )  keystatus &= ~STUP;
		if ( key>>16 == ('C'|0x80) )  keystatus &= ~STDOWN;

		if ( key>>16 == (0x74|0x80) )  keystatus &= ~STLEFT;
		if ( key>>16 == (0x76|0x80) )  keystatus &= ~STRIGHT;
		if ( key>>16 == (0x78|0x80) )  keystatus &= ~STUP;
		if ( key>>16 == (0x72|0x80) )  keystatus &= ~STDOWN;

		if ( key>>16 == (0x77|0x80) )  keystatus &= ~STLEFT;
		if ( key>>16 == (0x73|0x80) )  keystatus &= ~STRIGHT;
		if ( key>>16 == (0x79|0x80) )  keystatus &= ~STUP;
		if ( key>>16 == (0x71|0x80) )  keystatus &= ~STDOWN;

		return 0;						/* retourne si pas touche presse */
	}

	maj = key&0xFF;
	if ( maj >= 'a' && maj <= 'z' )  maj -= 'a'-'A';

	if ( g_typetext == 0 )
	{
		if ( key>>16 ==  'D' )		return KEYLEFT;
		if ( key>>16 ==  'F' )		return KEYRIGHT;
		if ( key>>16 ==  'R' )		return KEYUP;
		if ( key>>16 ==  'C' )		return KEYDOWN;

		if ( key>>16 ==  0x74 )		return KEYLEFT;
		if ( key>>16 ==  0x76 )		return KEYRIGHT;
		if ( key>>16 ==  0x78 )		return KEYUP;
		if ( key>>16 ==  0x72 )		return KEYDOWN;
	}
	else
	{
		if ( (key&0xFF00) == (KCURSOR<<8) )
		{
			if ( maj == 'D' )		return KEYLEFT;
			if ( maj == 'F' )		return KEYRIGHT;
			if ( maj == 'R' )		return KEYUP;
			if ( maj == 'C' )		return KEYDOWN;
			return  0;
		}
	}

	key &= 0xFFFF;

	if ( key == KEYMPO )	return 0;

	if ( key == F0 )		return KEYQUIT;
	if ( key == F1 )		return KEYF1;
	if ( key == F2 )		return KEYF2;
	if ( key == F3 )		return KEYF3;
	if ( key == F4 )		return KEYF4;
	if ( key == F5 )		return KEYF5;
	if ( key == F6 )		return KEYF6;
	if ( key == F7 )		return KEYF7;
	if ( key == F8 )		return KEYF8;
	if ( key == F9 )		return KEYF9;
	if ( key == F10 )		return KEYF10;
	if ( key == F11 )		return KEYF11;
	if ( key == F12 )		return KEYF12;

	if ( key == F13 )		return KEYSAVE;
	if ( key == F14 )		return KEYLOAD;
	if ( key == F15 )		return KEYPAUSE;

	if ( key == END )		return KEYHOME;
	if ( key == DEFINE )	return KEYDEF;
	if ( key == UNDO )		return KEYUNDO;
	if ( key == DEL )		return KEYDEL;
	if ( key == CR )		return KEYRETURN;
	if ( key == ENTER )		return KEYENTER;
	if ( key == POINT )		return KEYPAUSE;

	if ( key == KEYMGP )	return KEYCLIC;
	if ( key == KEYMMP )	return KEYCLIC;
	if ( key == KEYMDP )	return KEYCLICR;

	if ( key == KEYMGR )	return KEYCLICREL;
	if ( key == KEYMMR )	return KEYCLICREL;
	if ( key == KEYMDR )	return KEYCLICREL;

	if ( typetext == 1 )
	{
		if ( key == '' )	return KEYCIRCON;
		if ( key == '' )	return KEYTREMA;

		if ( key == '' )	return KEYAGRAVE;
		if ( key == '' )	return KEYATREMA;

		if ( key == '' )	return KEYEAIGU;
		if ( key == '' )	return KEYEGRAVE;

		if ( key == '' )	return KEYOTREMA;

		if ( key == '' )	return KEYUGRAVE;
		if ( key == '' )	return KEYUTREMA;

		if ( key == '' )	return KEYCCEDILLE;
	}
#endif
	return (short)key;
}

int
SDLEventToSmakyKey (const SDL_Event * event)
{
    int key = 0;

    switch (event->type)
    {
      case SDL_TEXTINPUT:
        if (!strncmp(event->text.text, "á", sizeof("á")))
          key = KEYAAIGU;
        else if (!strncmp(event->text.text, "à", sizeof("à")))
          key = KEYAGRAVE;
        else if (!strncmp(event->text.text, "â", sizeof("â")))
          key = KEYACIRCON;
        else if (!strncmp(event->text.text, "ä", sizeof("ä")))
          key = KEYATREMA;
        else if (!strncmp(event->text.text, "é", sizeof("é")))
          key = KEYEAIGU;
        else if (!strncmp(event->text.text, "è", sizeof("è")))
          key = KEYEGRAVE;
        else if (!strncmp(event->text.text, "ê", sizeof("ê")))
          key = KEYECIRCON;
        else if (!strncmp(event->text.text, "ë", sizeof("ë")))
          key = KEYETREMA;
        else if (!strncmp(event->text.text, "í", sizeof("í")))
          key = KEYIAIGU;
        else if (!strncmp(event->text.text, "ì", sizeof("ì")))
          key = KEYIGRAVE;
        else if (!strncmp(event->text.text, "î", sizeof("î")))
          key = KEYICIRCON;
        else if (!strncmp(event->text.text, "ï", sizeof("ï")))
          key = KEYITREMA;
        else if (!strncmp(event->text.text, "ó", sizeof("ó")))
          key = KEYOAIGU;
        else if (!strncmp(event->text.text, "ò", sizeof("ò")))
          key = KEYOGRAVE;
        else if (!strncmp(event->text.text, "ô", sizeof("ô")))
          key = KEYOCIRCON;
        else if (!strncmp(event->text.text, "ö", sizeof("ö")))
          key = KEYOTREMA;
        else if (!strncmp(event->text.text, "ú", sizeof("ú")))
          key = KEYUAIGU;
        else if (!strncmp(event->text.text, "ù", sizeof("ù")))
          key = KEYUGRAVE;
        else if (!strncmp(event->text.text, "û", sizeof("û")))
          key = KEYUCIRCON;
        else if (!strncmp(event->text.text, "ü", sizeof("ü")))
          key = KEYUTREMA;
        else if (!strncmp(event->text.text, "ç", sizeof("ç")))
          key = KEYCCEDILLE;

        if (key)
          break;

        if ((event->text.text[0] >= SDLK_a && event->text.text[0] <= SDLK_z)
            || (event->text.text[0] >= SDLK_0 && event->text.text[0] <= SDLK_9))
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
          case SDLK_LEFT:
          case SDLK_KP_4:
            key = KEYLEFT;
            break;
          case SDLK_RIGHT:
          case SDLK_KP_6:
            key = KEYRIGHT;
            break;
          case SDLK_UP:
          case SDLK_KP_8:
            key = KEYUP;
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
      case SDL_MOUSEBUTTONUP:
      {
        SDL_MouseButtonEvent * _event = (SDL_MouseButtonEvent *) event;
        if (_event->state == SDL_PRESSED)
        {
          if (_event->button == SDL_BUTTON_LEFT)
            key = KEYCLIC;
          else if (_event->button == SDL_BUTTON_RIGHT)
            key = KEYCLICR;
          g_keyMousePos.x = _event->x;
          g_keyMousePos.y = _event->y;
          g_keyMousePressed = SDL_TRUE;
        }
        else if (_event->state == SDL_RELEASED)
        {
          key = KEYCLICREL;
          g_keyMousePressed = SDL_FALSE;
          if (abs(g_keyMousePos.x - g_lastmouse.x) > 40 || abs(g_keyMousePos.y - g_lastmouse.y) > 20)
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
	Retourne l'tat du clavier, c'est--dire l'enfoncement
	ventuel des touches flches.
 */

KeyStatus GetKeyStatus (void)
{
	return keystatus;
}



/* ======= */
/* IfColor */
/* ======= */

/*
	Indique si la machine dispose d'un cran couleur.
	Retourne != 0 si oui (vrai).
 */

short IfColor (void)
{
	if ( colormode )  return 1;				/* couleur */
	return 0;								/* noir/blanc */
}


/* ======== */
/* ModColor */
/* ======== */

/*
	Modifie les composantes rouge/vert/bleu d'une couleur.
	Une composante est comprise entre 0 et 255.
 */

void ModColor (short color, short red, short green, short blue)
{

}


/* ========= */
/* DuplPixel */
/* ========= */

/*
	Duplique entirement un pixmap dans un autre.
 */

void DuplPixel(Pixmap *ppms, Pixmap *ppmd)
{
	Pt		p, dim;

        p.y = 0;
        p.x = 0;
        dim.y = ppmd->dy;
        dim.x = ppmd->dx;
	CopyPixel
	(
		ppms, p,
		ppmd, p,
		dim
	);
}


/* =============== */
/* ScrollPixelRect */
/* =============== */

/*
	Dcale un pixmap dans l'une des quatre directions.
		*ppm		*pixmap source et destination
		od			origine destination
		dim			dimensions
		shift.x		dcalage horizontal (<0 vers la droite, >0 vers la gauche)
		shift.y		dcalage vertical (<0 vers le bas, >0 vers le haut)
		color		couleur assigne  la nouvelle zone (0..15)
					-1 laisse la zone originale
		*pzone		retourne la zone  redessiner aprs ScrollPixel
 */

void ScrollPixelRect (Pixmap *ppm, Pt pos, Pt dim, Pt shift, char color, Rectangle *pzone)
{
	Pt		p1, p2, _dim;

	(*pzone).p1.x = pos.x;
	(*pzone).p1.y = pos.y;
	(*pzone).p2.x = pos.x+dim.x;
	(*pzone).p2.y = pos.y+dim.y;

	if ( shift.x == 0 && shift.y == 0 )  goto fill;
	if ( shift.x != 0 && shift.y != 0 )  goto fill;

        Pixmap tmp = {0};
        GetPixmap(&tmp, dim, 1, 0);
        DuplPixel(ppm, &tmp);

	if ( shift.x < 0 && shift.x > -dim.x )
	{
          p1.y = pos.y;
          p1.x = pos.x;
          p2.y=pos.y;
          p2.x=pos.x-shift.x;
          _dim.y=dim.y;
          _dim.x=dim.x+shift.x;
		CopyPixel
		(
			ppm, p1,
			&tmp, p2,
			_dim
		);
		(*pzone).p2.x = pos.x - shift.x;
	}
	if ( shift.x > 0 && shift.x < dim.x )
	{
          p1.y=pos.y;
          p1.x=pos.x+shift.x;
                p2.y = pos.y;
                p2.x = pos.x;
                _dim.y=dim.y;
                _dim.x=dim.x-shift.x;
		CopyPixel
		(
			ppm, p1,
			&tmp, p2,
			_dim
		);
		(*pzone).p1.x = pos.x + dim.x - shift.x;
	}
	if ( shift.y < 0 && shift.y > -dim.y )
	{
                p1.y = pos.y;
                p1.x = pos.x;
                p2.y = pos.y-shift.y;
                p2.x=pos.x;
                _dim.y=dim.y+shift.y;
                _dim.x=dim.x;
		CopyPixel
		(
			ppm, p1,
			&tmp, p2,
			_dim
		);
		(*pzone).p2.y = pos.y - shift.y;
	}
	if ( shift.y > 0 && shift.y < dim.y )
	{
                p1.y = pos.y+shift.y;
                p1.x = pos.x;
                p2.y = pos.y;
                p2.x = pos.x;
                _dim.y=dim.y-shift.y;
                _dim.x=dim.x;
		CopyPixel
		(
			ppm, p1,
			&tmp, p2,
			_dim
		);
		(*pzone).p1.y = pos.y + dim.y - shift.y;
	}

	DuplPixel(&tmp, ppm);
        GivePixmap(&tmp);

	fill:
	if ( color == -1 )  return;
	DrawFillRect(ppm, *pzone, color);		/* init la zone à mettre à jour */
}


/* =========== */
/* ScrollPixel */
/* =========== */

/*
	Dcale un pixmap dans l'une des quatre directions.
		*ppm		*pixmap source et destination
		shift.x		dcalage horizontal (<0 vers la droite, >0 vers la gauche)
		shift.y		dcalage vertical (<0 vers le bas, >0 vers le haut)
		color		couleur assigne  la nouvelle zone (0..15)
					-1 laisse la zone originale
		*pzone		retourne la zone  redessiner aprs ScrollPixel
 */

void ScrollPixel (Pixmap *ppm, Pt shift, char color, Rectangle *pzone)
{
	Pt		pos, dim;

	pos.x = 0;
	pos.y = 0;
	dim.x = ppm->dx;
	dim.y = ppm->dy;

	ScrollPixelRect(ppm, pos, dim, shift, color, pzone);
}


/* ========= */
/* CopyPixel */
/* ========= */

/*
	Copie un rectangle de pixel dans un autre (raster-op).
	Cette procdure est la seule qui dessine dans l'cran !
		*ppms	*pixmap source (si == 0 -> cran)
		os		origine source (coin sup/gauche)
		*ppmd	*pixmap destination (si == 0 -> cran)
		od		origine destination (coin sup/gauche)
		dim		dimensions du rectangle
	Retourne 1 si rien n'a t dessin.
 */

short CopyPixel(Pixmap *ppms, Pt os, Pixmap *ppmd, Pt od, Pt dim)
{
	if ( ppmd != 0 )
	{
		if ( od.x < 0 )					/* dpasse  gauche ? */
		{
			dim.x += od.x;
			if ( dim.x <= 0 )  return 1;
			os.x -= od.x;
			od.x = 0;
		}
		if ( od.x+dim.x > ppmd->dx )	/* dpasse  droite ? */
		{
			dim.x -= od.x+dim.x - ppmd->dx;
			if ( dim.x <= 0 )  return 1;
		}

		if ( od.y < 0 )					/* dpasse en haut ? */
		{
			dim.y += od.y;
			if ( dim.y <= 0 )  return 1;
			os.y -= od.y;
			od.y = 0;
		}
		if ( od.y+dim.y > ppmd->dy )	/* dpasse en bas ? */
		{
			dim.y -= od.y+dim.y - ppmd->dy;
			if ( dim.y <= 0 )  return 1;
		}
	}

	if ( ppms == 0 )				/* source dans l'cran ? */
	{
		os.x += origine.x;
		os.y += origine.y;
	}
	else
	{
                os.x += ppms->orig.x;
                os.y += ppms->orig.y;
	}

	if ( ppmd == 0 )				/* destination dans l'cran ? */
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
	Dessine un segment de droite d'un pixel d'paisseur.
		*ppm		->	pixmap o dessiner (0 = cran)
		p1			->	dpart
		p2			->	arrive
		color		->	0 = blanc .. 15 = noir
 */

void DrawLine (Pixmap *ppm, Pt p1, Pt p2, int color)
{
  if (!ppm)				/* source dans l'cran ? */
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

  SDL_Texture * target = SDL_GetRenderTarget(g_renderer);
  SDL_SetRenderTarget(g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_SetRenderDrawColor(g_renderer, g_colors[color].r, g_colors[color].g, g_colors[color].b, g_colors[color].a);
  SDL_SetRenderTarget(g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_RenderDrawLine(g_renderer, p1.x, p1.y, p2.x, p2.y);
  SDL_SetRenderTarget(g_renderer, target);
}


/* ======== */
/* DrawRect */
/* ======== */

/*
	Dessine un rectangle d'un pixel d'paisseur.
		*ppm		->	pixmap o dessiner (0 = cran)
		rect.p1		->	coin sup/gauche
		rect.p2		->	coin inf/droite
		color		->	0 = blanc .. 15 = noir
 */

void DrawRect (Pixmap *ppm, Rectangle rect, int color)
{
  SDL_Rect _rect;
  _rect.x = rect.p1.x;
  _rect.y = rect.p1.y;
  _rect.w = rect.p2.x - rect.p1.x;
  _rect.h = rect.p2.y - rect.p1.y;

  SDL_Texture * target = SDL_GetRenderTarget(g_renderer);
  SDL_SetRenderTarget(g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_SetRenderDrawColor(g_renderer, g_colors[color].r, g_colors[color].g, g_colors[color].b, g_colors[color].a);
  SDL_RenderDrawRect(g_renderer, &_rect);
  SDL_SetRenderTarget(g_renderer, target);
}


/* ============ */
/* DrawFillRect */
/* ============ */

/*
	Dessine une surface rectangulaire remplie avec une couleur donne
	dans un pixmap.
		*ppm		->	pixmap où dessiner (0 = écran)
		rect.p1		->	coin sup/gauche
		rect.p2		->	coin inf/droite
		color		->	0 = blanc .. 15 = noir
 */

void DrawFillRect (Pixmap *ppm, Rectangle rect, int color)
{
  SDL_Rect _rect;
  _rect.x = rect.p1.x;
  _rect.y = rect.p1.y;
  _rect.w = rect.p2.x - rect.p1.x;
  _rect.h = rect.p2.y - rect.p1.y;

  SDL_Texture * target = SDL_GetRenderTarget(g_renderer);
  SDL_SetRenderTarget(g_renderer, ppm ? ppm->texture : g_screen.texture);
  SDL_SetRenderDrawColor(g_renderer, g_colors[color].r, g_colors[color].g, g_colors[color].b, g_colors[color].a);
  SDL_RenderFillRect(g_renderer, &_rect);
  SDL_SetRenderTarget(g_renderer, target);
}

/* ========== */
/* SavePixmap */
/* ========== */

/*
	Sauve un pixmap en mmoire tendue (XMS sur PC).
 */

short SavePixmap (Pixmap *ppm)
{
	pmsave = *ppm;

        pmsave.texture = SDL_CreateTexture (
            g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, ppm->dx,
            ppm->dy);
        SDL_SetTextureBlendMode(pmsave.texture, SDL_BLENDMODE_BLEND);
        DuplPixel(ppm, &pmsave);
	return 0;
}

/* ============= */
/* RestorePixmap */
/* ============= */

/*
	Restitue le pixmap en mmoire tendue (XMS sur PC).
 */

short RestorePixmap (Pixmap *ppm)
{
	if ( pmsave.texture == 0 )  return 1;

        DuplPixel(&pmsave, ppm);
        SDL_DestroyTexture(pmsave.texture);
        pmsave.texture = NULL;
	return 0;
}


/* --------- */
/* LoadImage */
/* --------- */

/*
	Charge un fichier image codé, si nécessaire.
 */

static int LoadImage(int numero, Pixmap *pim)
{
	int			err = 1;
	char		name[4096];				/* nom de l'image BLUPIXnn.IMAGE */
	char *lang = "fr/";

        if (numero < 22 || numero == 33)
          lang = "";

	if ( colormode && (numero < IMAMASK || numero >= 20) )
	{
		snprintf(name, sizeof(name), "%s../share/blupimania/image/%sblupix%02d.color.png", SDL_GetBasePath (), lang, numero);
	}
	else
	{
		snprintf(name, sizeof(name), "%s../share/blupimania/image/%sblupix%02d.image.png", SDL_GetBasePath (), lang, numero);
	}

	SDL_Surface * surface = IMG_Load (name);
        SDL_Texture * texture = SDL_CreateTextureFromSurface (g_renderer, surface);
        SDL_FreeSurface (surface);

        Uint32        format;
        Sint32        access, ow, oh;
        SDL_QueryTexture (texture, &format, &access, &ow, &oh);
        pim->dx = ow;
        pim->dy = oh;

        pim->texture = SDL_CreateTexture (
          g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, pim->dx,
          pim->dy);
        SDL_SetTextureBlendMode(pim->texture, SDL_BLENDMODE_BLEND);

        SDL_Texture * target = SDL_GetRenderTarget (g_renderer);
        SDL_SetRenderTarget (g_renderer, pim->texture);
        SDL_RenderCopy(g_renderer, texture, NULL, NULL);
        SDL_DestroyTexture(texture);
        SDL_SetRenderTarget (g_renderer, target);

	err = 0;												/* chargement image ok */										/* ferme le fichier */
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

short GetPixmap(Pixmap *ppm, Pt dim, short fill, short color)
{
    if (ppm->texture)
      SDL_DestroyTexture(ppm->texture);

        ppm->texture = SDL_CreateTexture (
          g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, dim.x,
          dim.y);
        SDL_SetTextureBlendMode(ppm->texture, SDL_BLENDMODE_BLEND);
        ppm->dx = dim.x;
        ppm->dy = dim.y;

        if (fill >= 0)
        {
        SDL_Texture * target = SDL_GetRenderTarget(g_renderer);
        SDL_SetRenderTarget(g_renderer, ppm->texture);
        if (fill == 0)
          SDL_SetRenderDrawColor (g_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
        else if (fill == 1)
          SDL_SetRenderDrawColor (g_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        else if (fill == 2)
          SDL_SetRenderDrawColor (g_renderer, 255, 255, 255, SDL_ALPHA_TRANSPARENT);
        SDL_RenderClear (g_renderer);
        SDL_SetRenderTarget(g_renderer, target);
        }

	return 0;
}



/* ========== */
/* GivePixmap */
/* ========== */

/*
	Libre un pixmap quelconque, obtenu avec GetPixmap ou GetImage,
	mais pas avec GetIcon (give pas ncessaire).
 */

short GivePixmap(Pixmap *ppm)
{
  if (ppm->texture)
  {
    SDL_DestroyTexture(ppm->texture);
      ppm->texture = NULL;
  }
	return 0;
}



/* ======== */
/* GetImage */
/* ======== */

/*
	Ouvre une image en la lisant si ncessaire, puis en la dcodant
	dans un pixmap.
 */

short GetImage(Pixmap *ppm, short numero)
{
	int			err;

	err = LoadImage(numero, ppm);				/* charge l'image */
	if ( err )  goto error;
error:
	return err;
}

/* ======= */
/* GetIcon */
/* ======= */

/*
	Conversion d'une icne en pixmap noir/blanc.
	Le pointeur au data (ppm->data) est directement initialis dans
	l'une des grandes images contenant toutes les icnes (IMAICON
	ou IMAICON+IMAMASK).
	Puisque la mmoire pour le data n'est pas alloue par cette
	procdure, il ne faut donc pas faire de GivePixmap !
	Si le mode vaut zro, le data n'est pas rendu, mais seulement
	les dimensions, pour gagner du temps.
 */

short GetIcon(Pixmap *ppm, short numero, short mode)
{
	int			no;

	ppm->dx     = LXICO;
	ppm->dy     = LYICO;

        no = numero;
        if ( (numero&ICONMASK) < 128*1 )
        {
                ppm->texture  = pmicon1c.texture;
                goto data;
        }
        else if ( (numero&ICONMASK) < 128*2 )
        {
                ppm->texture  = pmicon2c.texture;
                no -= 128*1;
                goto data;
        }
        else if ( (numero&ICONMASK) < 128*3 )
        {
                ppm->texture  = pmicon3c.texture;
                no -= 128*2;
                goto data;
        }
        else if ( (numero&ICONMASK) < 128*4 )
        {
                ppm->texture  = pmicon4c.texture;
                no -= 128*3;
                goto data;
        }
        else
          return 1;


	data:
        ppm->orig.x = LXICO * ((no&ICONMASK)%16);
        ppm->orig.y = LYICO * ((no&ICONMASK)/16);

	switch ( no&(~ICONMASK) )
	{
		case UL:
			ppm->dx /= 2L;
			ppm->dy /= 2L;
			break;

		case UR:
			//ppm->data += (LXICO/8L/2)*ppm->nbp;
			ppm->dx /= 2L;
			ppm->dy /= 2L;
                        ppm->orig.x += LXICO / 2;
			break;

		case DL:
			//ppm->data += (160L*LYICO/2)*ppm->nbp;
			ppm->dx /= 2L;
			ppm->dy /= 2L;
                        ppm->orig.y += LYICO / 2;
			break;

		case DR:
			//ppm->data += (160L*LYICO/2 + LXICO/8L/2)*ppm->nbp;
			ppm->dx /= 2L;
			ppm->dy /= 2L;
                        ppm->orig.x += LXICO / 2;
                        ppm->orig.y += LYICO / 2;
			break;
	}

	return 0;				/* retour toujours ok */
}




/* -------- */
/* LoadIcon */
/* -------- */

/*
	Chargement de l'image des icnes.
 */

int LoadIcon(void)
{
	int		err;

	err = GetImage(&pmicon1c, IMAICON+0);			/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon2c, IMAICON+1);			/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon3c, IMAICON+2);			/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon4c, IMAICON+3);			/* charge l'image des icnes */
	if ( err )  return err;

	return 0;
}

void UnloadIcon()
{
  GivePixmap(&pmicon1c);
  GivePixmap(&pmicon2c);
  GivePixmap(&pmicon3c);
  GivePixmap(&pmicon4c);
}

static void
MusicStopped(void)
{
  PushUserEvent (MUSIC_STOP, NULL);
}

static int
InitSoundSystem()
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
  static const char* sounds[] = {
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

  for (int i = 1; i < countof(sounds); ++i)
  {
    char filename[4096];
    snprintf(filename, sizeof(filename), "%s../share/blupimania/sound/%s", SDL_GetBasePath (), sounds[i]);
    Mix_Chunk * chunk = Mix_LoadWAV(filename);
    if (!chunk)
      continue;

    g_sounds[i] = chunk;
  }

  return 0;
}

void
UnloadSounds (void)
{
  for (int i = 0; i < countof (g_sounds); ++i)
    if (g_sounds[i])
    {
      Mix_FreeChunk(g_sounds[i]);
      g_sounds[i] = NULL;
    }

  if (g_music)
  {
    Mix_FreeMusic(g_music);
    g_music = NULL;
  }

  Mix_CloseAudio ();
}


/* =========== */
/* BlackScreen */
/* =========== */

/*
	Efface tout l'cran (noir), pendant le changement de la clut.
 */

void BlackScreen (void)
{
    SDL_SetRenderDrawColor (g_renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear (g_renderer);
}



/* ======== */
/* FileRead */
/* ======== */

/*
	Lit nb bytes dans un fichier de donnes  la position pos.
	Retourne 0 si la lecture est ok.
 */

short FileRead (void *pdata, long pos, short nb, char file)
{
	FILE		*channel;
	char		filename[4096];
	short		n = 0;

        snprintf(filename, sizeof(filename), "%s../share/blupimania/data/blupixa.dat", SDL_GetBasePath ());

	filename[strlen(filename) - 5] = file;
	//if ( file >= 'a' )  n = 6;
	channel = fopen(filename+n, "rb");	/* ouvre le fichier */
	if ( channel == NULL )  return errno;

	if ( fseek(channel, pos, SEEK_SET) != 0 )
	{
		goto close;
	}

	if ( fread(pdata, nb, 1, channel) != 1 )
	{
		goto close;
	}

	close:
	fclose(channel);					/* ferme le fichier */
	return 0;
}


/* ========= */
/* FileWrite */
/* ========= */

/*
	Ecrit nb bytes dans un fichier de donnes  la position pos.
	Retourne 0 si l'criture est ok.
 */

short FileWrite (void *pdata, long pos, short nb, char file)
{
	FILE		*channel;
	char		filename[4096];

        snprintf(filename, sizeof(filename), "%s../share/blupimania/data/blupixa.dat", SDL_GetBasePath ());

	filename[strlen(filename) - 5] = file;
	//if ( file >= 'a' )  n = 6;

        channel = fopen(filename, "r+b");
        if (!channel)
        {
          channel = fopen(filename, "wb");
          if (!channel)
            return errno;
          fclose(channel);

          channel = fopen(filename, "r+b");
        }

	if (!channel)
          return errno;

	if ( fseek(channel, pos, SEEK_SET) != 0 )
	{
		goto close;
	}

	if ( fwrite(pdata, nb, 1, channel) != 1 )
	{
		goto close;
	}

	close:
	fclose(channel);					/* ferme le fichier */
	return 0;
}


/* ============= */
/* FileGetLength */
/* ============= */

/*
	Retourne la longueur d'un fichier.
	En cas d'erreur, retourne 0 (longueur nulle).
 */

long FileGetLength (char file)
{
	FILE		*channel;
	char		filename[4096];
	short		n = 0;
	long		lg;

        snprintf(filename, sizeof(filename), "%s../share/blupimania/data/blupixa.dat", SDL_GetBasePath ());

	filename[strlen(filename) - 5] = file;
	//if ( file >= 'a' )  n = 6;
	channel = fopen(filename+n, "r");	/* ouvre le fichier */
	if ( channel == NULL )  return 0;

	fseek(channel, 0, SEEK_END);
	lg = ftell(channel);

	fclose(channel);					/* ferme le fichier */
	return lg;
}


/* ========== */
/* FileDelete */
/* ========== */

/*
	Dtruit un fichier.
	Retourne 0 si la destruction est ok.
 */

short FileDelete (char file)
{
	char		filename[4096];

        snprintf(filename, sizeof(filename), "%s../share/blupimania/data/blupixa.dat", SDL_GetBasePath ());

	filename[strlen(filename) - 5] = file;
	return remove(filename);
}


/* ========== */
/* FileRename */
/* ========== */

/*
	Renomme un fichier.
	Retourne 0 si le changement est ok.
 */

short FileRename (char oldfile, char newfile)
{
        char		oldfilename[4096];
        char		newfilename[4096];

        snprintf(oldfilename, sizeof(oldfilename), "%s../share/blupimania/data/blupixa.dat", SDL_GetBasePath ());
        snprintf(newfilename, sizeof(newfilename), "%s../share/blupimania/data/blupixa.dat", SDL_GetBasePath ());

	oldfilename[strlen(oldfilename) - 5] = oldfile;
	newfilename[strlen(newfilename) - 5] = newfile;
	return rename(oldfilename, newfilename);
}


/* ----------- */
/* IfFileExist */
/* ----------- */

/*
	Teste si un fichier existe.
	Si oui, retourne 1 (true).
 */

short IfFileExist (char *pfilename)
{
	FILE		*channel;

	channel = fopen(pfilename, "r");		/* ouvre le fichier */
	if ( channel == NULL )  return 0;		/* fichier n'existe pas */

	fclose(channel);						/* ferme le fichier */
	return 1;								/* fichier existe */
}

/* =========== */
/* OpenMachine */
/* =========== */

/*
	Ouverture gnrale, chargement des librairies, gencar, etc.
 */

int OpenMachine(void)
{
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
  SDL_setenv ("SDL_AUDIODRIVER", "directsound", true);
#endif /* _WIN32 */

  int res = SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
  if (res < 0)
  {
    SDL_Log ("Unable to initialize SDL: %s", SDL_GetError ());
    return EXIT_FAILURE;
  }

  g_window = SDL_CreateWindow (
      "Blupimania", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      LXIMAGE (), LYIMAGE (), 0);

  if (!g_window)
  {
    printf ("%s", SDL_GetError ());
    return EXIT_FAILURE;
  }

  g_renderer = SDL_CreateRenderer (
    g_window, -1, g_rendererType | SDL_RENDERER_TARGETTEXTURE /*| SDL_RENDERER_SOFTWARE*/);
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

	LoadIcon();					/* charge l'image des icônes */

        InitSoundSystem();
        LoadSounds();

	StartRandom(0, 0);					/* coup de sac du générateur aléatoire toto */
	StartRandom(1, 1);					/* coup de sac du générateur aléatoire décor */

	keystatus = 0;

      return 0;
}


/* ============ */
/* CloseMachine */
/* ============ */

/*
	Fermeture gnrale.
 */

void CloseMachine(void)
{
  UnloadIcon();
  UnloadSounds();
}




/* =============== */
/* MachinePartieLg */
/* =============== */

/*
	Retourne la longueur ncessaire pour sauver les variables de la partie en cours.
 */

long MachinePartieLg (void)
{
	return sizeof(unsigned long)*10;
}

/* ================== */
/* MachinePartieWrite */
/* ================== */

/*
	Sauve les variables de la partie en cours.
 */

short MachinePartieWrite (long pos, char file)
{
	short		err;

	err = FileWrite(&nextrand, pos, sizeof(unsigned long)*10, file);
	return err;
}

/* ================= */
/* MachinePartieRead */
/* ================= */

/*
	Lit les variables de la partie en cours.
 */

short MachinePartieRead (long pos, char file)
{
	short		err;

	err = FileRead(&nextrand, pos, sizeof(unsigned long)*10, file);
	return err;
}

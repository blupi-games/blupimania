
/* ==== */
/* bm.h */
/* ==== */


#pragma once

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

typedef unsigned short u_short;


/* ---------- */
/* Constantes */
/* ---------- */

#define MAJREV		1			/* rvision */
#define MINREV		0			/* version */

#define LXIMAGE()	640			/* largeur d'une image */
#define LYIMAGE()	340			/* hauteur d'une image */

#define POSXPALETTE	8			/* coin sup/gauche de la palette d'icnes */
#define POSYPALETTE	8			/* coin sup/gauche de la palette d'icnes */
#define DIMXPALETTE	52			/* largeur de la palette d'icnes */
#define DIMYPALETTE	324			/* hauteur de la palette d'icnes */

#define POSXDRAW	72			/* coin sup/gauche de la fentre de jeu */
#define POSYDRAW	8			/* coin sup/gauche de la fentre de jeu */
#define DIMXDRAW	560			/* largeur de la fentre de jeu */
#define DIMYDRAW	324			/* hauteur de la fentre de jeu */

#define DELSLOW		8			/* dlai entre deux images (lent) */
#define DELNORM		5			/* dlai entre deux images (normal) */
#define DELQUICK	2			/* dlai entre deux images (maximum) */

#define IMABASE		1			/* image de base */
#define IMAICON		1			/* image des icnes */

#define MAXIMAGE	(20*2)		/* nb max d'images */
#define MAXSOUND	10			/* nb max de sons */

#define KEYQUIT		(-1)		/* touche quitte */
#define KEYHOME		(-2)		/* touche retourne  la maison */
#define KEYDEF		(-3)		/* touche dfinitions */
#define KEYINFO		(-4)		/* touche informations */
#define KEYUNDO		(-5)		/* touche annule */
#define KEYDEL		(-6)		/* touche dtruit  gauche */
#define KEYRETURN	(-7)		/* touche retour principale */
#define KEYENTER	(-8)		/* touche d'entre (pav numrique) */
#define KEYPAUSE	(-9)		/* touche pause */
#define KEYF1		(-10)		/* touche F1 */
#define KEYF2		(-11)		/* touche F2 */
#define KEYF3		(-12)		/* touche F3 */
#define KEYF4		(-13)		/* touche F4 */
#define KEYSAVE		(-14)		/* touche sauve partie */
#define KEYLOAD		(-15)		/* touche reprend partie */
#define KEYIO		(-16)		/* touche sauve/reprend partie */
#define KEYCLIC		(-20)		/* bouton souris press (gauche) */
#define KEYCLICR	(-21)		/* bouton souris press (droite) */
#define KEYCLICREL	(-22)		/* bouton souris relch */
#define KEYLEFT		(-30)		/* touche gauche */
#define KEYRIGHT	(-31)		/* touche droite */
#define KEYUP		(-32)		/* touche haut */
#define KEYDOWN		(-33)		/* touche bas */
#define KEYSLEFT	(-34)		/* touche shift gauche */
#define KEYSRIGHT	(-35)		/* touche shift droite */
#define KEYSUP		(-36)		/* touche shift haut */
#define KEYSDOWN	(-37)		/* touche shift bas */
#define KEYCENTER	(-40)		/* touche centre */
#define KEYGOFRONT	(-50)		/* touche en avant */
#define KEYGOBACK	(-51)		/* touche en arrire */
#define KEYGOLEFT	(-52)		/* touche tourne  gauche */
#define KEYGORIGHT	(-53)		/* touche tourne  droite */
#define KEYAIGU		(-60)		/* touche accent aigu */
#define KEYGRAVE	(-61)		/* touche accent grave */
#define KEYCIRCON	(-62)		/* touche accent circonflxe */
#define KEYTREMA	(-63)		/* touche accent trma */
#define KEYAAIGU	(-70)		/* touche "a" aigu*/
#define KEYAGRAVE	(-71)		/* touche "a" grave */
#define KEYACIRCON	(-72)		/* touche "a" circonflxe*/
#define KEYATREMA	(-73)		/* touche "a" trma */
#define KEYEAIGU	(-74)		/* touche "e" aigu*/
#define KEYEGRAVE	(-75)		/* touche "e" grave */
#define KEYECIRCON	(-76)		/* touche "e" circonflxe*/
#define KEYETREMA	(-77)		/* touche "e" trma */
#define KEYIAIGU	(-78)		/* touche "i" aigu*/
#define KEYIGRAVE	(-79)		/* touche "i" grave */
#define KEYICIRCON	(-80)		/* touche "i" circonflxe*/
#define KEYITREMA	(-81)		/* touche "i" trma */
#define KEYOAIGU	(-82)		/* touche "o" aigu*/
#define KEYOGRAVE	(-83)		/* touche "o" grave */
#define KEYOCIRCON	(-84)		/* touche "o" circonflxe*/
#define KEYOTREMA	(-85)		/* touche "o" trma */
#define KEYUAIGU	(-86)		/* touche "u" aigu*/
#define KEYUGRAVE	(-87)		/* touche "u" grave */
#define KEYUCIRCON	(-88)		/* touche "u" circonflxe*/
#define KEYUTREMA	(-89)		/* touche "u" trma */
#define KEYCCEDILLE	(-90)		/* touche "c" cdille */
#define KEYcCEDILLE	(-91)		/* touche "C" cdille */
#define KEYF5		(-100)		/* touche F5 */
#define KEYF6		(-101)		/* touche F6 */
#define KEYF7		(-102)		/* touche F7 */
#define KEYF8		(-103)		/* touche F8 */
#define KEYF9		(-104)		/* touche F9 */
#define KEYF10		(-105)		/* touche F10 */
#define KEYF11		(-106)		/* touche F11 */
#define KEYF12		(-107)		/* touche F12 */

#define MAXCELX		20			/* nb max de cellules en x */
#define MAXCELY		20			/* nb max de cellules en y */

#define MAXTOTO		(10+2)		/* nb max de toto anims simultans */



typedef enum {
  FRAME_TICK = 1,
  MUSIC_STOP = 2,
  RESET = 3,
} UserEvent;

/* --------- */
/* KeyStatus */
/* --------- */

typedef enum
{
  STLEFT	=	1<<0,			/* va  gauche */
  STRIGHT 	=	1<<1,			/* va  droite */
  STUP		=	1<<2,			/* va en haut */
  STDOWN	=	1<<3			/* va en bas */
}
KeyStatus;



/* -------------------- */
/* Structure d'un monde */
/* -------------------- */

#define MAXPALETTE	100
#define MAXTEXT		500

typedef struct
{
	short	tmonde[MAXCELY][MAXCELX];		/* cellules du monde */
	short	palette[MAXPALETTE];			/* palette d'icnes */
	char	text[MAXTEXT];					/* texte de description */
	short	freq;							/* frquence de sortie */
	short	color;							/* palette de couleurs */
	short	reserve[100];					/* rserve */
}
Monde;

/* ----- */
/* Icne */
/* ----- */

#define UL		(1<<14)			/* quart sup/gauche */
#define UR		(1<<13)			/* quart sup/droite */
#define DL		(1<<12)			/* quart inf/gauche */
#define DR		(1<<11)			/* quart inf/droite */

#define ICONMASK	0x07FF		/* masque pour ne conserver que l'icne */

#define LXICO		80			/* largeur d'une icne */
#define LYICO		80			/* hauteur d'une icne */
#define PLXICO		44			/* pas gauche en x */
#define PLYICO		11			/* pas gauche en y */
#define PRXICO		36			/* pas droite en x */
#define PRYICO		18			/* pas droite en y */
#define PZICO		51			/* pas vertical en z */

#define ICOVIDE		0			/* icne vide (pour effacer) */





/* ----- */
/* Point */
/* ----- */

typedef struct
{
  short		y;			/* coordonne signe y */
  short		x;			/* coordonne signe x */
}
Pt;



/* --------- */
/* Rectangle */
/* --------- */

typedef struct
{
  Pt		p1;			/* coin sup/gauche */
  Pt		p2;			/* coin inf/droite */
}
Rectangle;



/* ------ */
/* Rgion */
/* ------ */

typedef struct
{
  Rectangle	r;			/* rectangle de la rgion */
}
Reg;



/* --------- */
/* Pixel map */
/* --------- */

typedef struct
{
  short		dy;				/* hauteur */
  short		dx;				/* largeur */
  Pt orig;
  SDL_Texture *texture;
}
Pixmap;



/* -------- */
/* Couleurs */
/* -------- */

#define COLORBLANC		0		/* blanc */
#define COLORJAUNE		1		/* jaune */
#define COLORORANGE		2		/* orange */
#define COLORROUGE		3		/* rouge */
#define COLORGRISC		4		/* gris clair */
#define COLORGRISF		5		/* gris fonc */
#define COLORCYAN		6		/* cyan */
#define COLORBLEU		7		/* bleu */
#define COLORVERTC		8		/* vert clair */
#define COLORVERTF		9		/* vert fonc */
#define COLORVIOLET		10		/* violet */
#define COLORMAGENTA	11		/* magenta */
#define COLORBRUNC		12		/* brun clair */
#define COLORBRUNF		13		/* brun fonc */
#define COLORBLEUM		14		/* bleu moyen */
#define COLORNOIR		15		/* noir */



/* ------------------ */
/* Variables globales */
/* ------------------ */

extern	short	g_langue;				/* numéro de la langue */
extern  short   g_theme;                /* 1 -> theme Smaky 100 */
extern	short	g_monde;				/* monde actuel (0..n) */
extern	short	g_updatescreen;		/* 1 -> écran à mettre à jour */
extern	short	g_typejeu;			/* type de jeu (0..1) */
extern	short	g_typeedit;			/* 1 -> édition d'un monde */
extern	short	g_typetext;			/* 1 -> édition d'un texte */
extern	short	g_modetelecom;		/* 1 -> mode télécommande gauche/droite */
extern	short	g_pause;				/* 1 -> pause */
extern	short	g_passdaniel;			/* 1 -> toujours construction */
extern	short	g_passpower;			/* 1 -> force infinie */
extern	short	g_passnice;			/* 1 -> toujours gentil */
extern	short	g_passhole;			/* 1 -> ne tombe pas dans trou */

extern SDL_Renderer * g_renderer;
extern SDL_Window *   g_window;
extern Pixmap g_screen;

extern int g_rendererType;
extern Sint32         g_timerInterval;
extern Sint32         g_timerSkip;
extern Pt g_lastmouse;			/* dernière position de la souris */
extern SDL_bool g_clearKeyEvents;
extern SDL_bool g_ignoreKeyClicUp;
extern Pt g_keyMousePos;
extern SDL_bool g_keyMousePressed;
extern Sint32 g_keyFunctionUp;
extern SDL_bool g_subMenu;
extern SDL_bool g_superInvalid;

extern const SDL_Color *g_colors;
extern const SDL_Color *g_colorsTheme[2];



/* --------------------- */
/* Dclarations externes */
/* --------------------- */


/* bm_pal.c */
/* -------- */

void	PaletteUseObj	(short icon);
short	PaletteStatus	(short rang);
short	PaletteGetPress	(void);
short	PaletteEvent	(short event, Pt pos);
void	PaletteNew		(short *pdesc, short type);
void PaletteDraw (void);

void	PaletteEditOpen	(short palette[]);
short	PaletteEditEvent (short palette[], short event, Pt pos);
void	PaletteEditClose (short palette[]);

void	InfoDraw		(short status, short force, short vision,
							 short mechant, short magic, short cles);

int	PalPartieLg		(void);
short	PalPartieWrite	(int pos, char file);
short	PalPartieRead	(int pos, char file);


/* bm_move.c */
/* --------- */

void	MoveModifCel	(Pt cel);
short	MoveGetCel		(Pt cel);
void	MoveBack		(Pt cel);
short	MoveNext		(char event, Pt pos);
void	MoveRedraw		(void);
short	MoveBuild		(short outil, int key);
void	MoveScroll		(short quick);
void	MoveNewMonde	(short freq);
short	MoveOpen		(void);
void	MoveClose		(void);

int	MovePartieLg	(void);
short	MovePartieWrite	(int pos, char file);
short	MovePartieRead	(int pos, char file);


/* bm_decor.c */
/* ---------- */

typedef struct {
  SDL_bool super;
  short icon;
  Pt cel;
  Pt off;
  Pt dim;
} ImageStack;

Pt		GraToCel		(Pt gra);
Pt		CelToGra		(Pt cel);
Pt		CelToGra2		(Pt cel, SDL_bool shift);
short	DecorGetInitCel	(Pt cel);
void	DecorPutInitCel	(Pt cel, short icon);
short	DecorGetCel		(Pt cel);
void	DecorPutCel		(Pt cel, short icon);
const ImageStack *	DecorIconMask	(Pt pos, short posz, Pt cel);
Pt GetSuperCel();
Pt		DecorDetCel		(Pt pmouse);
void	DecorSuperCel	(Pt pmouse);
short	DecorEvent		(Pt pos, short poscel, short icon, int key);
void	DecorModif		(Pt cel, short icon);
Pixmap*	DecorGetPixmap	(void);
Pt		DecorGetOrigine	(void);
void	DecorSetOrigine	(Pt origine, short quick);
void	DecorMake		(short bSuperCel);
short	DecorNewMonde	(Monde *pmonde);
short	DecorOpen		(void);
void	DecorClose		(void);

long	DecorPartieLg	(void);
short	DecorPartieWrite (long pos, char file);
short	DecorPartieRead	(long pos, char file);


/* bm_icone.c */
/* ---------- */

short	IfNilRegion		(Reg rg);
short	IfSectRegion	(Reg r1, Reg r2);
Reg		OrRegion		(Reg r1, Reg r2);
Reg		AndRegion		(Reg r1, Reg r2);
void	IconDrawAll		(void);
void	IconDrawFlush	(void);
void	IconDrawOpen	(void);
short	IconDrawPut		(short ico, short btransp, Pt pos, short posz, Pt cel, Reg clip);
void	IconDrawUpdate	(Reg rg);
void	IconDrawClose	(short bdraw);
Pixmap*	IconGetPixmap	(void);
short	IconOpen		(void);
void	IconClose		(void);

/* bm_text.c */
/* --------- */

#define TEXTSIZELIT		10		/* petite taille */
#define TEXTSIZEMID		21		/* taille moyenne */

Pt		DrawText		(Pixmap *ppm, Pt pos, char *pstring, short size);
Rectangle GetRectText	(Pt pos, char *pstring, short size);
void	DrawParagraph	(Pixmap *ppm, Rectangle rect, const char * pstring,
							 short size);

short	EditEvent		(short key, Pt pos);
short	EditOpen		(char *p, short max, Rectangle rect);
short	EditClose		(void);


/* bm_smaky.c */
/* ---------- */

void	InitRandomEx	(short g, short min, short max, char *pex);
short	GetRandomEx		(short g, short min, short max, char *pex);
short	GetRandom		(short g, short min, short max);
void	StartRandom		(short g, short mode);

short	PrintScreen		(Pt p1, Pt p2);

void	MusicStart		(short song);
void	MusicStop		(void);
SDL_bool MusicStoppedOnDemand(void);
void	PlayNoiseVolume	(short volume);
void	PlayMusicVolume	(short volume);
short	IfPlayReady		(void);
void	PlaySoundLoop	(short mode);
void	PlaySound		(short sound, const Pt * cel);
SDL_bool SoundPlaying(short sound);

void	ClrEvents		(void);
short	GetEvent		(Pt *ppos);
int SDLEventToSmakyKey (const SDL_Event * event);
KeyStatus GetKeyStatus	(void);
short	IfColor			(void);
void	ModColor		(short color, short red, short green, short blue);
void DrawIcon(short num, Pt p1, Pt p2, Pt dim);
void DrawIconTemp(short num, Pt p1, Pt p2, Pt dim);
short	GetIcon			(Pixmap *ppm, short numero, short mode);
short	GetPixmap		(Pixmap *ppm, Pt dim, short fill, short colormode);
short	GetImage		(Pixmap *ppm, short numero);
short	GivePixmap		(Pixmap *ppm);
void	DuplPixel		(Pixmap *ppms, Pixmap *ppmd);
void	ScrollPixel		(Pixmap *ppm, Pt shift, char color, Rectangle *pzone);
void	ScrollPixelRect	(Pixmap *ppm, Pt od, Pt dim, Pt shift, char color, Rectangle *pzone);
short	CopyPixel		(Pixmap *ppms, Pt os,
								 Pixmap *ppmd, Pt od,
								 Pt dim);
void	DrawLine		(Pixmap *ppm, Pt p1, Pt p2, int color);
void	DrawRect		(Pixmap *ppm, Rectangle rect, int color);
void	DrawFillRect	(Pixmap *ppm, Rectangle rect, int color);
char	GetPixel		(Pixmap *ppm, Pt pos);
void	BlackScreen		(void);
short	SavePixmap		(Pixmap *ppm);
short	RestorePixmap	(Pixmap *ppm);

short	FileRead		(void *pdata, long pos, short nb, char file);
short	FileWrite		(void *pdata, long pos, short nb, char file);
long	FileGetLength	(char file);
short	FileDelete		(char file);
short	FileRename		(char oldfile, char newfile);

void	FatalError		(short err);
int	OpenMachine		(void);
void	CloseMachine	(void);

long	MachinePartieLg		(void);
short	MachinePartieWrite	(long pos, char file);
short	MachinePartieRead	(long pos, char file);

void PushUserEvent (Sint32 code, void * data);
void Render ();

int LoadIcon();
void UnloadIcon();
int LoadDecor();
void UnloadDecor();

#ifdef _WIN32
#define countof(a) _countof (a)
#else /* _WIN32 */
#define countof(a) (sizeof (a) / sizeof (*a))
#endif /* !_WIN32 */

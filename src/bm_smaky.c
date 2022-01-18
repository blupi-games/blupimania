
/* ========== */
/* bm_smaky.c */
/* ========== */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/file.h>
#include <smakylib.h>
#include <smaky.h>
#include <errno.h>
#include <audio.h>
#include <tcolor.h>

#include "bm.h"
#include "blupix.h"




#define GCTEXT		{'N','E'+0x80,11} /* NEF11 */


/* ------- */
/* En-tte */
/* ------- */

u_long	_stksize	= 8000L;				/* longueur du tas et de la pile  */
u_long	_SMASOVER	= 0L;					/* 0: pas d'entre overlay, 1: ou */
u_char	_SMAPRIO	= 11;					/* priorit                       */
u_char	_SMAREV		= MAJREV;				/* rvision                       */
u_char	_SMAVER		= MINREV;				/* version                        */
u_long	_SMAUNIT	= 0x7;					/* units                         */
u_short	_SMADIS[]	= {640,2000,340,2000};	/* largeur+hauteur (min+max)      */
u_short	_SMASYN		= 2;     				/* niveau du haut-parleur         */
u_char	_SMAGC[]	= GCTEXT;				/* fonte                          */
char	_SMAIDE[]	= "(C)  Daniel ROUX et EPSITEC-system sa";




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
  Image		*head;			/* pointe l'en-tte */
  char		*clut;			/* pointe la clut */
  char		*data;			/* pointe l'image code */
}
ImageSmaky;



/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Windesc		pgradesc;					/* pointeur au descripteur de fentre */
static u_long		chres;						/* canal des ressources */
static Pixmap		pmicon1c = {0,0,0,0,0,0,0};	/* pixmap des icnes1 (chair) */
static Pixmap		pmicon1m = {0,0,0,0,0,0,0};	/* pixmap des icnes1 (masque) */
static Pixmap		pmicon2c = {0,0,0,0,0,0,0};	/* pixmap des icnes2 (chair) */
static Pixmap		pmicon2m = {0,0,0,0,0,0,0};	/* pixmap des icnes2 (masque) */
static Pixmap		pmicon3c = {0,0,0,0,0,0,0};	/* pixmap des icnes3 (chair) */
static Pixmap		pmicon3m = {0,0,0,0,0,0,0};	/* pixmap des icnes3 (masque) */
static Pixmap		pmicon4c = {0,0,0,0,0,0,0};	/* pixmap des icnes4 (chair) */
static Pixmap		pmicon4m = {0,0,0,0,0,0,0};	/* pixmap des icnes4 (masque) */
static Pt			origine;					/* coin sup/gauche de l'origine */
static long			atime = 0;					/* temps de dbut */

static u_long		nextrand[10];				/* valeurs alatoires suivantes */

static short		colormode = 0;				/* 1 = couleur possible */
static unsigned long tcolcanal;					/* canal TCOLOR */

static unsigned int	adcanal;					/* canal module audio */
static unsigned int	adcanalson1 = 0;			/* canal du son audio */
static unsigned int	adcanalson2 = 0;			/* canal du son audio */
static unsigned int	admode;						/* 0 = sons audio possibles */
static short		soundon = 1;				/* son on/off */
static short		sound1 = 0;					/* bruitages */
static short		sound2 = 0;					/* musiques */
static int			filsson = 0;				/* son  entendre */
static int			filsrep = 0;				/* pas de rptition */
static u_short		filsabort = 0;				/* fin du processus fils */
static u_short		visumouse = 1;				/* 1 = souris visible */
static Pt			lastmouse = {0,0};			/* dernire position de la souris */
static KeyStatus	keystatus;					/* tat des flches du clavier */
static char			demo = 0;					/* 1 = mode dmo */

static Pixmap		pmsave = {0,0,0,0,0,0,0};	/* pixmap sauv en mmoire tendue (XMS) */


/* Sauvetage d'une partie du descripteur de fentre */
/* ------------------------------------------------ */

static Point	savedf;			/* origine */
static Point	savedfd;		/* dimensions */
static Point	savedff;		/* origine fentre */
static Point	savedffd;		/* dimensions fenetre */
static Point	savedfwb;		/* debut (sous-)fenetre */
static Point	savedfwe;		/* fin (sous-)fenetre */
static void*	savedfabs;		/* adresse de base du bitmap */
static short	savedfiix;		/* 0 => ecran SM8 et 1 => memoire */
static short	savedfiiy;		/* increment si y=y+1 */
static u_short	savedfcnb;		/* nombre de couleurs (2, 4, 16 ou 256) */
static u_char	savedfcnp;		/* nombre de plans (1, 2, 4 ou 8) */
static u_char	savedfcmd;		/* mode (0=noir-blanc, 1=couleur) */






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
	long		datebcd;

	if ( mode == 0 )
	{
		nextrand[g] = 33554393;		/* grand nombre premier */
	}
	else
	{
		L_rdclock(&datebcd, &nextrand[g]);
	}
}




/* --------- */
/* FilsSound */
/* --------- */

/*
	Processus fils de gestion du son.
 */

static void FilsSound (void)
{
	while ( filsabort == 0 )
	{
		if ( filsson != 0 )
		{
			if ( filsson < 100 && adcanalson1 != 0 )
			{
				audio_music(&adcanal, &adcanalson1, filsson-1);
			}
			if ( filsson > 100 && adcanalson2 != 0 )
			{
				audio_music(&adcanal, &adcanalson2, filsson-101);
			}
			if ( filsrep == 0)  filsson = 0;
			if ( filsrep >  0)  filsrep --;
		}
		N_delms(5);			/* attend un poil ... */
	}
}


/* ========== */
/* MusicStart */
/* ========== */

/*
	Dmarre une musique de fond donne (song).
	song = 0	->		musique pendant gnrique initial
	song = 1	->		musique si termin un niveau
	song = 2	->		musique si termin une nigme
	song = 3	->		musique pendant rglages
	song = 4	->		musique pendant jeu (choix alatoire)
 */

void MusicStart (short song)
{
}

/* ========= */
/* MusicStop */
/* ========= */

/*
	Stoppe la musique de fond.
 */

void MusicStop (void)
{
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
}

/* =============== */
/* PlayMusicVolume */
/* =============== */

/*
	Dtermine le volume de la musique de fond (0..10).
 */

void PlayMusicVolume (short volume)
{
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
	if ( filsson == 0 )  return 1;
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

void PlaySound (short sound)
{
	if ( soundon == 0 )  return;
	filsson = sound;			/* donne le numro au processus fils */
}




/* ======== */
/* OpenTime */
/* ======== */

/*
	Indique le dbut d'une opration plus ou moins longue
	(ne fait rien, enregistre simplement le temps absolu).
 */

void OpenTime (void)
{
	long		hightotal,highunused,lowunused;

	N_getsytime(&hightotal, &atime, &highunused, &lowunused);
}


/* ========= */
/* CloseTime */
/* ========= */

/*
	Indique la fin d'une opration plus on moins longue,
	avec attente si la dure tait plus petite que le temps total souhait.
 */

void CloseTime (short t)
{
	long		hightotal,lowtotal,highunused,lowunused;
	long		del;

	if ( t == 0 )  return;

	N_getsytime(&hightotal, &lowtotal, &highunused, &lowunused);

	del = t - (lowtotal-atime)/(20000/128);
	if ( del>0 && del<50 )
	{
		N_delms(del);	/* attente... */
	}
	else
	{
		N_delms(1);		/* attente minimale */
	}
}




/* ======== */
/* PosMouse */
/* ======== */

/*
	Dplace la souris.
 */

void PosMouse (Pt pos)
{
	Point		p;

	p.x = pos.x + origine.x;
	p.y = pos.y + origine.y;

	asm("moveml d4/d7,sp@-");			/* PUSH */
	asm("movel %0,d4" : : "g" (p));		/* D4 <-- p */
	asm(".word 0x4E46");				/* LIB */
	asm(".word 145");					/* ?PMOUSE */
	asm("moveml sp@+,d4/d7");			/* POP */
}



/* ======= */
/* IfMouse */
/* ======= */

/*
	Indique si la souris existe sur cette machine.
	Ceci est utile pour le PC qui n'a pas toujours une souris !
	Si oui, retourne != 0.
 */

short IfMouse (void)
{
	return 1;				/* sur smaky, y'a toujours une souris */
}


/* ----------- */
/* getmousepos */
/* ----------- */

/*
	Donne la position de la souris, aprs avoir reu
	un KEYCLIC depuis GetEvent par exemple.
 */

static Pt getmousepos (void)
{
	int		err;
	Point	pos;
	Pt		ret;
	u_char	boutons;

	err = L_ifmouse(&pos, &boutons);	/* lecture de la souris */
	if (err)  return (Pt){0,0};

	ret.x = pos.x - origine.x;
	ret.y = pos.y - origine.y;
	return ret;
}


/* ========= */
/* HideMouse */
/* ========= */

/*
	Enlve la souris.
 */

void HideMouse(void)
{
	if ( visumouse == 1 )
	{
		visumouse = 0;
		printf("%c",AFCCMO);		/* cache la souris */
	}
}


/* ========= */
/* ShowMouse */
/* ========= */

/*
	Remet la souris.
 */

void ShowMouse(void)
{
	if ( visumouse == 0 )
	{
		visumouse = 1;
		printf("%c",AFSCMO);		/* montre la souris */
	}
}



/* ----------- */
/* IfHideMouse */
/* ----------- */

/*
	Cache la souris si elle est dans un rectangle.
 */

static void IfHideMouse (Rectangle r)
{
	Pt		mouse;

	mouse = getmousepos();		/* lit la position de la souris */

	if ( mouse.x-10 > r.p2.x )  return;
	if ( mouse.x+20 < r.p1.x )  return;
	if ( mouse.y-10 > r.p2.y )  return;
	if ( mouse.y+20 < r.p1.y )  return;

	HideMouse();				/* cache la souris */
}



/* ========= */
/* ClrEvents */
/* ========= */

/*
	Vide le fifo du clavier.
 */

void ClrEvents (void)
{
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
	long		key, maj;

	*ppos = lastmouse;					/* rend la dernire position de la souris */

	N_settim(0);						/* met un timeout nul */
	key = L_getkey();					/* lecture du clavier sans attendre */
	N_settim(0xFFFF);					/* remet un timeout infini */

	lastmouse = getmousepos();			/* lit la position de la souris */

	if ( errno )			return 0;	/* retourne si timeout */

	if ( key>>16 == 'D' )  keystatus = STLEFT;
	if ( key>>16 == 'F' )  keystatus = STRIGHT;
	if ( key>>16 == 'R' )  keystatus = STUP;
	if ( key>>16 == 'C' )  keystatus = STDOWN;

	if ( key>>16 == 0x74 )  keystatus = STLEFT;
	if ( key>>16 == 0x76 )  keystatus = STRIGHT;
	if ( key>>16 == 0x78 )  keystatus = STUP;
	if ( key>>16 == 0x72 )  keystatus = STDOWN;

	if ( typejeu == 1 && modetelecom == 0 && pause == 0 )	/* tlcommand en gauche/droite ? */
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

	if ( typetext == 0 )
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

	return (short)key;
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
	unsigned long	nocoul = color;
	RGBColor		coulmod;

	if ( colormode == 0 )  return;

	coulmod.r = (red<<8)+red;
	coulmod.g = (green<<8)+green;
	coulmod.b = (blue<<8)+blue;

	tcol_putclut(tcolcanal, TCFORCE, &nocoul, &coulmod);
}


/* ======== */
/* GetColor */
/* ======== */

/*
	Donne les composantes rouge/vert/bleu d'une couleur.
	Une composante est comprise entre 0 et 255.
 */

void GetColor (short color, short *pred, short *pgreen, short *pblue)
{
	unsigned long	nocoul = color;
	RGBColor		coulcherche;
	RGBColor		coultrouve;

	if ( colormode == 0 )
	{
		*pred   = 0;
		*pgreen = 0;
		*pblue  = 0;
	}
	else
	{
		tcol_srcclut(tcolcanal, TCFORCE+TCGIVE, &nocoul, &coulcherche, &coultrouve);

		*pred   = coultrouve.r>>8;
		*pgreen = coultrouve.g>>8;
		*pblue  = coultrouve.b>>8;
	}
}



/* ========= */
/* DuplPixel */
/* ========= */

/*
	Duplique entirement un pixmap dans un autre.
 */

void DuplPixel(Pixmap *ppms, Pixmap *ppmd)
{
	Pt		p;

	CopyPixel
	(
		ppms, (p.y=0, p.x=0, p),
		ppmd, (p.y=0, p.x=0, p),
		(p.y=ppmd->dy, p.x=ppmd->dx, p), MODELOAD
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
	Pt		p;

	(*pzone).p1.x = pos.x;
	(*pzone).p1.y = pos.y;
	(*pzone).p2.x = pos.x+dim.x;
	(*pzone).p2.y = pos.y+dim.y;

	if ( shift.x == 0 && shift.y == 0 )  goto fill;
	if ( shift.x != 0 && shift.y != 0 )  goto fill;

	if ( shift.x < 0 && shift.x > -dim.x )
	{
		CopyPixel
		(
			ppm, (p.y=pos.y, p.x=pos.x, p),
			ppm, (p.y=pos.y, p.x=pos.x-shift.x, p),
			(p.y=dim.y, p.x=dim.x+shift.x, p), MODELOAD
		);
		(*pzone).p2.x = pos.x - shift.x;
	}
	if ( shift.x > 0 && shift.x < dim.x )
	{
		CopyPixel
		(
			ppm, (p.y=pos.y, p.x=pos.x+shift.x, p),
			ppm, (p.y=pos.y, p.x=pos.x, p),
			(p.y=dim.y, p.x=dim.x-shift.x, p), MODELOAD
		);
		(*pzone).p1.x = pos.x + dim.x - shift.x;
	}
	if ( shift.y < 0 && shift.y > -dim.y )
	{
		CopyPixel
		(
			ppm, (p.y=pos.y, p.x=pos.x, p),
			ppm, (p.y=pos.y-shift.y, p.x=pos.x, p),
			(p.y=dim.y+shift.y, p.x=dim.x, p), MODELOAD
		);
		(*pzone).p2.y = pos.y - shift.y;
	}
	if ( shift.y > 0 && shift.y < dim.y )
	{
		CopyPixel
		(
			ppm, (p.y=pos.y+shift.y, p.x=pos.x, p),
			ppm, (p.y=pos.y, p.x=pos.x, p),
			(p.y=dim.y-shift.y, p.x=dim.x, p), MODELOAD
		);
		(*pzone).p1.y = pos.y + dim.y - shift.y;
	}

	fill:
	if ( color == -1 )  return;
	DrawFillRect(ppm, *pzone, MODELOAD, color);		/* init la zone  mettre  jour */
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
		mode	mode de transfert (MODELOAD/MODEOR/MODEAND)
	Retourne 1 si rien n'a t dessin.
 */

short CopyPixel(Pixmap *ppms, Pt os, Pixmap *ppmd, Pt od, Pt dim, ShowMode mode)
{
	long		m;					/* mode pour gra2 */
	u_long		csf,ccf;			/* sauvetage des couleurs */
	char		*ps,*pd;			/* ^source,^destination */
	short		is,id;				/* Iy source, Iy destination */
	Rectangle	rect;				/* rectangle pour la souris */

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

	switch (mode)
	{
		case MODEOR:	m = SETDOT; break;
		case MODEAND:	m = CLRDOT; break;
		case MODEXOR:	m = INVDOT; break;
		default:		m = LOADDOT;
	}

	if ( ppms == 0 )				/* source dans l'cran ? */
	{
		rect.p1.x = os.x;
		rect.p1.y = os.y;
		rect.p2.x = os.x + dim.x;
		rect.p2.y = os.y + dim.y;
		IfHideMouse(rect);			/* cache la souris */
		os.x += origine.x;
		os.y += origine.y;
		ps = 0;
		is = 0;
	}
	else
	{
		ps = ppms->data;
		is = ppms->dxb;
	}

	if ( ppmd == 0 )				/* destination dans l'cran ? */
	{
		rect.p1.x = od.x;
		rect.p1.y = od.y;
		rect.p2.x = od.x + dim.x;
		rect.p2.y = od.y + dim.y;
		IfHideMouse(rect);			/* cache la souris */
		od.x += origine.x;
		od.y += origine.y;
		pd = 0;
		id = 0;
	}
	else
	{
		pd = ppmd->data;
		id = ppmd->dxb;
	}

	if ( pgradesc->dfcnp <= 1 )
	{										/* noir-blanc */
		if ( (pgradesc->dfmod&0x80) == 0 )	/* cran  fond blanc ? */
		{
			if ( ppms == 0 || ppmd == 0 )
			{
				switch ( m )
				{
					case LOADDOT: m += CPLDOT; break;
					case SETDOT:  m  = CLRDOT; break;
					case CLRDOT:  m  = SETDOT; break;
				}
			}
		}
		gra2_raster
		(
			pgradesc,						/* ^descripteur de fentre    */
			(Point) {os.y, os.x}, ps, is,	/* source: origine, ^data, Iy */
			(Point) {od.y, od.x}, pd, id,	/* dest:   origine, ^data, Iy */
			(Point) {dim.y, dim.x}, m		/* dimensions, mode           */
		);
	}
	else
	{										/* couleur */
		if ( ppms == 0 || ppms->nbp > 1 )  m += SRCCOUL;
		if ( ppmd == 0 || ppmd->nbp > 1 )  m += DSTCOUL;

		csf = pgradesc->dfcsf;
		ccf = pgradesc->dfccf;				/* sauve les couleurs */

		if ( (m&SRCCOUL) == 0 && (m&DSTCOUL) != 0 )
		{									/* si source n-b et destination couleur */
			pgradesc->dfcsf = ppms->scolor;
			pgradesc->dfccf = ppms->ccolor;
		}

		gra2_craster
		(
			pgradesc,						/* ^descripteur de fentre    */
			(Point) {os.y, os.x}, ps, is,	/* source: origine, ^data, Iy */
			(Point) {od.y, od.x}, pd, id,	/* dest:   origine, ^data, Iy */
			(Point) {dim.y, dim.x}, m		/* dimensions, mode           */
		);

		pgradesc->dfcsf = csf;
		pgradesc->dfccf = ccf;				/* restitue les couleurs */
	}

	ShowMouse();							/* remet la souris si ncessaire */
	return 0;
}


/* ----------- */
/* OpenGraDesc */
/* ----------- */

/*
	Prpare le descripteur de fentre pgradesc pour pouvoir dessiner
	dans un pixmap en mmoire.
 */

void OpenGraDesc (Pixmap *ppm)
{
	if ( ppm == 0 )
	{
		HideMouse();
		return;
	}

	savedf    = pgradesc->df;
	savedfd   = pgradesc->dfd;
	savedff   = pgradesc->dff;
	savedffd  = pgradesc->dffd;
	savedfwb  = pgradesc->dfwb;
	savedfwe  = pgradesc->dfwe;
	savedfiix = pgradesc->dfiix;
	savedfiiy = pgradesc->dfiiy;
	savedfcnp = pgradesc->dfcnp;
	savedfcnb = pgradesc->dfcnb;
	savedfcmd = pgradesc->dfcmd;
	savedfabs = pgradesc->dfabs;

	pgradesc->df.x   = 0;
	pgradesc->df.y   = 0;
	pgradesc->dfd.x  = ppm->dx;
	pgradesc->dfd.y  = ppm->dy;
	pgradesc->dff.x  = 0;
	pgradesc->dff.y  = 0;
	pgradesc->dffd.x = ppm->dx;
	pgradesc->dffd.y = ppm->dy;
	pgradesc->dfwb.x = 0;
	pgradesc->dfwb.y = 0;
	pgradesc->dfwe.x = ppm->dx;
	pgradesc->dfwe.y = ppm->dy;

	pgradesc->dfiix = 1;
	pgradesc->dfiiy = ppm->dxb;

	pgradesc->dfcnp = ppm->nbp;
	pgradesc->dfcnb = 1<<ppm->nbp;
	if ( ppm->nbp == 1 )  pgradesc->dfcmd = 0;
	else                  pgradesc->dfcmd = 2;

	pgradesc->dfabs = ppm->data;
}

/* ------------ */
/* CloseGraDesc */
/* ------------ */

/*
	Restitue le descripteur de fentre pgradesc pour pouvoir dessiner
	comme avant.
 */

void CloseGraDesc (Pixmap *ppm)
{
	if ( ppm == 0 )
	{
		ShowMouse();
		return;
	}

	pgradesc->df    = savedf;
	pgradesc->dfd   = savedfd;
	pgradesc->dff   = savedff;
	pgradesc->dffd  = savedffd;
	pgradesc->dfwb  = savedfwb;
	pgradesc->dfwe  = savedfwe;
	pgradesc->dfiix = savedfiix;
	pgradesc->dfiiy = savedfiiy;
	pgradesc->dfcnp = savedfcnp;
	pgradesc->dfcnb = savedfcnb;
	pgradesc->dfcmd = savedfcmd;
	pgradesc->dfabs = savedfabs;
}


/* ======== */
/* DrawLine */
/* ======== */

/*
	Dessine un segment de droite d'un pixel d'paisseur.
		*ppm		->	pixmap o dessiner (0 = cran)
		p1			->	dpart
		p2			->	arrive
		mode		->	mode de dessin
		color		->	0 = blanc .. 15 = noir
 */

void DrawLine (Pixmap *ppm, Pt p1, Pt p2, ShowMode mode, char color)
{
	u_long		csf,ccf;			/* sauvetage des couleurs */
	Pt			o;
	Point		p, d;
	char		m;

	OpenGraDesc(ppm);

	if ( ppm == 0 )
	{
		o = origine;
	}
	else
	{
		o.x = 0;
		o.y = 0;
	}

	switch (mode)
	{
		case MODEOR:	m = SETDOT; break;
		case MODEAND:	m = CLRDOT; break;
		case MODEXOR:	m = INVDOT; break;
		default:		m = SETDOT;
	}
	if ( ppm == 0 &&					/* dans l'cran ? */
		 pgradesc->dfcnp <= 1 &&		/* cran mono-chrome ? */
		 (pgradesc->dfmod&0x80) == 0 )	/* cran  fond blanc ? */
	{
		switch ( m )
		{
			case SETDOT:  m  = CLRDOT; break;
			case CLRDOT:  m  = SETDOT; break;
		}
	}

	csf = pgradesc->dfcsf;
	ccf = pgradesc->dfccf;				/* sauve les couleurs */

	pgradesc->dfcsf = color;
	pgradesc->dfccf = ~color;

	gra2_line							/* dessine une ligne */
	(
		pgradesc,
		(p.y=o.y+p1.y, p.x=o.x+p1.x, p),
		(d.y=p2.y-p1.y, d.x=p2.x-p1.x, d),
		m
	);
	pgradesc->dfcsf = csf;
	pgradesc->dfccf = ccf;				/* restitue les couleurs */

	CloseGraDesc(ppm);
}


/* ======== */
/* DrawRect */
/* ======== */

/*
	Dessine un rectangle d'un pixel d'paisseur.
		*ppm		->	pixmap o dessiner (0 = cran)
		rect.p1		->	coin sup/gauche
		rect.p2		->	coin inf/droite
		mode		->	mode de dessin
		color		->	0 = blanc .. 15 = noir
 */

void DrawRect (Pixmap *ppm, Rectangle rect, ShowMode mode, char color)
{
	u_long		csf,ccf;			/* sauvetage des couleurs */
	Pt			o;
	Point		p, d;
	char		m;

	OpenGraDesc(ppm);

	if ( ppm == 0 )
	{
		o = origine;
	}
	else
	{
		o.x = 0;
		o.y = 0;
	}

	switch (mode)
	{
		case MODEOR:	m = SETDOT; break;
		case MODEAND:	m = CLRDOT; break;
		case MODEXOR:	m = INVDOT; break;
		default:		m = SETDOT;
	}
	if ( ppm == 0 &&					/* dans l'cran ? */
		 pgradesc->dfcnp <= 1 &&		/* cran mono-chrome ? */
		 (pgradesc->dfmod&0x80) == 0 )	/* cran  fond blanc ? */
	{
		switch ( m )
		{
			case SETDOT:  m  = CLRDOT; break;
			case CLRDOT:  m  = SETDOT; break;
		}
	}

	csf = pgradesc->dfcsf;
	ccf = pgradesc->dfccf;				/* sauve les couleurs */

	pgradesc->dfcsf = color;
	pgradesc->dfccf = ~color;

	gra2_line							/* dessine une ligne */
	(
		pgradesc,
		(p.y=o.y+rect.p1.y, p.x=o.x+rect.p1.x, p),
		(d.y=0, d.x=rect.p2.x-rect.p1.x, d),
		m
	);

	gra2_line							/* dessine une ligne */
	(
		pgradesc,
		(p.y=o.y+rect.p1.y, p.x=o.x+rect.p2.x, p),
		(d.y=rect.p2.y-rect.p1.y, d.x=0, d),
		m
	);

	gra2_line							/* dessine une ligne */
	(
		pgradesc,
		(p.y=o.y+rect.p2.y, p.x=o.x+rect.p2.x, p),
		(d.y=0, d.x=rect.p1.x-rect.p2.x, d),
		m
	);

	gra2_line							/* dessine une ligne */
	(
		pgradesc,
		(p.y=o.y+rect.p2.y, p.x=o.x+rect.p1.x, p),
		(d.y=rect.p1.y-rect.p2.y, d.x=0, d),
		m
	);

	pgradesc->dfcsf = csf;
	pgradesc->dfccf = ccf;				/* restitue les couleurs */

	CloseGraDesc(ppm);
}


/* ============ */
/* DrawFillRect */
/* ============ */

/*
	Dessine une surface rectangulaire remplie avec une couleur donne
	dans un pixmap.
		*ppm		->	pixmap o dessiner (0 = cran)
		rect.p1		->	coin sup/gauche
		rect.p2		->	coin inf/droite
		mode		->	mode de dessin
		color		->	0 = blanc .. 15 = noir
 */

void DrawFillRect (Pixmap *ppm, Rectangle rect, ShowMode mode, char color)
{
	u_long		csf,ccf;			/* sauvetage des couleurs */
	Pt			o;
	Point		p, d;
	Trame88		trame1 = {-1,-1,-1,-1,-1,-1,-1,-1};
	Trame88		trame0 = {0,0,0,0,0,0,0,0};

	OpenGraDesc(ppm);

	if ( ppm == 0 )
	{
		o = origine;
	}
	else
	{
		o.x = 0;
		o.y = 0;
	}

	if ( mode == MODEXOR )
	{
		gra2_trame							/* inverse la surface */
		(
			pgradesc,
			(p.y=o.y+rect.p1.y, p.x=o.x+rect.p1.x, p),
			(d.y=rect.p2.y-rect.p1.y, d.x=rect.p2.x-rect.p1.x, d),
			INVDOT,
			trame1
		);
		goto close;
	}

	if ( pgradesc->dfcmd == 0 )
	{
		if ( ppm == 0 &&					/* dans l'cran ? */
			 (pgradesc->dfmod&0x80) == 0 )	/* cran  fond blanc ? */
		{
			if ( color )  color = 0;
			else          color = 1;
		}
		if ( color == 0 )
		{
			gra2_trame						/* rempli la surface */
			(
				pgradesc,
				(p.y=o.y+rect.p1.y, p.x=o.x+rect.p1.x, p),
				(d.y=rect.p2.y-rect.p1.y, d.x=rect.p2.x-rect.p1.x, d),
				CLRDOT,
				trame0
			);
		}
		else
		{
			gra2_trame						/* rempli la surface */
			(
				pgradesc,
				(p.y=o.y+rect.p1.y, p.x=o.x+rect.p1.x, p),
				(d.y=rect.p2.y-rect.p1.y, d.x=rect.p2.x-rect.p1.x, d),
				SETDOT,
				trame1
			);
		}
	}
	else
	{
		csf = pgradesc->dfcsf;
		ccf = pgradesc->dfccf;				/* sauve les couleurs */

		pgradesc->dfcsf = color;
		pgradesc->dfccf = color;

		gra2_trame							/* rempli la surface */
	    (
			pgradesc,
			(p.y=o.y+rect.p1.y, p.x=o.x+rect.p1.x, p),
			(d.y=rect.p2.y-rect.p1.y, d.x=rect.p2.x-rect.p1.x, d),
			LOADDOT,
			trame1
	    );
		pgradesc->dfcsf = csf;
		pgradesc->dfccf = ccf;				/* restitue les couleurs */
	}

	close:
	CloseGraDesc(ppm);
}



/* ======== */
/* GetPixel */
/* ======== */

/*
	Retourne la couleur d'un pixel contenu dans un pixmap.
		0  = blanc
		15 = noir
		-1 = en dehors du pixmap
 */

char GetPixel (Pixmap *ppm, Pt pos)
{
	char		*pt;

	if ( pos.x < 0 || pos.x >= ppm->dx ||
		 pos.y < 0 || pos.y >= ppm->dy )  return -1;

	pt = ppm->data;
	if ( ppm->nbp == 1 )
	{
		pt += ppm->dxb * pos.y;
		pt += pos.x / 8;
		if ( *pt & 1<<(7-pos.x%8) )  return 15;
		return 0;
	}
	else
	{
		return -1;		/*  faire ! */
	}
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

	pmsave.data = malloc( ((u_long)pmsave.dxb)*((u_long)pmsave.dy) );
	if ( pmsave.data == NULL )  return 1;

	memcpy(pmsave.data, ppm->data, ((u_long)pmsave.dxb)*((u_long)pmsave.dy));
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
	char	*data;

	if ( pmsave.data == 0       ||
		 ppm->data   == 0       ||
		 ppm->dxb != pmsave.dxb ||
		 ppm->dx  != pmsave.dx  ||
		 ppm->dy  != pmsave.dy  ||
		 ppm->nbp != pmsave.nbp )  return 1;

	memcpy(ppm->data, pmsave.data, ((u_long)pmsave.dxb)*((u_long)pmsave.dy));

	data = ppm->data;
	*ppm = pmsave;
	ppm->data = data;

	free(pmsave.data);
	pmsave.data = 0;

	return 0;
}



/* ========= */
/* TestHLine */
/* ========= */

/*
	Teste si une ligne horizontale est entirement vide.
	Ne fonctionne que dans un pixmap d'icne noir/blanc !
	Si oui, retourne 1 (true).
 */

short TestHLine (Pixmap *ppm, short y)
{
	char		*pt;
	short		i;

	pt = ppm->data + ppm->dxb*y;
	for ( i=0 ; i<LXICO/8 ; i++ )
	{
		if ( *pt++ )  return 0;
	}
	return 1;
}


/* ========= */
/* TestVLine */
/* ========= */

/*
	Teste si une ligne verticale est entirement vide.
	Ne fonctionne que dans un pixmap d'icne noir/blanc !
	Si oui, retourne 1 (true).
 */

short TestVLine (Pixmap *ppm, short x)
{
	char		*pt;
	short		i;
	short		mask;

	pt = ppm->data + x/8;
	mask = 1<<(7-x%8);
	for ( i=0 ; i<LYICO ; i++ )
	{
		if ( *pt & mask )  return 0;
		pt += ppm->dxb;
	}
	return 1;
}




/* --------- */
/* lib_pdump */
/* --------- */

/*
	Appel LIB,?PDUMP, pour imprimer une copie de l'cran.
 */

static int lib_pdump (int mode)
{
	asm("moveml d3/d7,sp@-");			/* PUSH */
	asm("movel %0,d3" : : "g" (mode));	/* D3 <-- mode */
	asm(".word 0x4E46");				/* LIB */
	asm(".word 0x003C");				/* ?PDUMP */
	asm("moveml sp@+,d3/d7");			/* POP */
	return 0;							/* ? */
}


/* =========== */
/* PrintScreen */
/* =========== */

/*
	Imprime une copie de l'cran.
 */

short PrintScreen (Pt p1, Pt p2)
{
	Point		coin, dim;
	int			nowdo, err;

	coin.x = p1.x;
	coin.y = p1.y;
	dim.x  = p2.x - p1.x;
	dim.y  = p2.y - p1.y;
	L_crewdo(coin, dim, &nowdo);		/* cre une sous-fentre */
	L_usewdo(nowdo);					/* utilise la sous-fentre cre */

	HideMouse();						/* cache la souris */
	err = lib_pdump(0);					/* imprime la sous-fentre */
	ShowMouse();						/* remet la souris */

	L_usewdo(0);						/* utilise la sous-fentre initiale */
	L_kilwdo(nowdo);					/* dtruit la sous-fentre cre */

	return err;
}




/* ----------- */
/* UnloadImage */
/* ----------- */

/*
	Libre un fichier image cod, si ncessaire.
 */

static int UnloadImage(ImageSmaky *pim)
{
	if ( pim->data != 0 )
	{
		free(pim->data);			/* libre l'image code */
		pim->data = 0;
	}

	if ( pim->clut != 0 )
	{
		free(pim->clut);			/* libre la clut */
		pim->clut = 0;
	}

	if ( pim->head != 0 )
	{
		free(pim->head);			/* libre l'en-tte */
		pim->head = 0;
	}
	return 0;						/* retour toujours ok */
}



/* -------- */
/* LoadCLUT */
/* -------- */

/*
	Modifie la clut en fonction de la table d'une image.
 */

static void LoadCLUT (void *ptable)
{
	if ( colormode )
	{
		tcol_loadclut(tcolcanal, TCTABLE, 0, ptable, 0);
	}
}


/* -------- */
/* ClearMem */
/* -------- */

/*
	Met une zone mmoire  zro (blanc) ou  un (noir).
 */

static void ClearMem (char *pt, int lg, int fill)
{
	if ( fill != 0 )  fill = -1;
	memset(pt, fill, lg);
}


/* --------- */
/* LoadImage */
/* --------- */

/*
	Charge un fichier image cod, si ncessaire.
 */

static int LoadImage(int numero, ImageSmaky *pim)
{
	int			err = 1;
	FILE		*channel;				/* descripteur du fichier */
	char		name[24];				/* nom de l'image BLUPIXnn.IMAGE */

	pim->head = NULL;
	pim->clut = NULL;
	pim->data = NULL;

	if ( colormode && (numero < IMAMASK || numero >= 20) )
	{
		if ( numero == 36 )  sprintf(name, "(:,#:)BLUPI_X.COLOR");
		else                 sprintf(name, "(:,#:)BLUPIX%02d.COLOR", numero);
	}
	else
	{
		if ( numero == 36 )  sprintf(name, "(:,#:)BLUPI_X.IMAGE");
		else                 sprintf(name, "(:,#:)BLUPIX%02d.IMAGE", numero);
	}

	if ( (channel=fopen(name,"r")) == NULL )  goto error;	/* ouvre le fichier */

	pim->head = malloc(sizeof(Image));
	if ( pim->head == NULL )  goto error;
	fread( pim->head, 1, sizeof(Image), channel );			/* lit l'en-tte */

	if ( pim->head->imlgc != 0 )							/* image couleur avec clut ? */
	{
		pim->clut = malloc(pim->head->imlgc);
		if ( pim->clut == NULL )  goto error;
		fread( pim->clut, 1, pim->head->imlgc, channel );	/* lit la clut */
	}

	pim->data = malloc(pim->head->imnbb);
	if (pim->data == NULL)  goto error;
	fread( pim->data, 1, pim->head->imnbb, channel );		/* lit l'image code */

	if ( pim->head->imbip > 1 )								/* image couleur ? */
	{
		LoadCLUT(pim->clut);								/* change la clut */
	}
	err = 0;												/* chargement image ok */

	error:
	if ( err )
	{
		if ( pim->head != NULL )  free(pim->head);			/* libre l'en-tte */
		if ( pim->clut != NULL )  free(pim->clut);			/* libre la clut */
		if ( pim->data != NULL )  free(pim->data);			/* libre le data */
	}

	fclose(channel);										/* ferme le fichier */
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
	int		nbp = 1;

	if ( colormode )
	{
		if ( color >= 1 )  nbp = pgradesc->dfcnp;
	}
	else
	{
		if ( color == 2 )  nbp = pgradesc->dfcnp;
	}

	if ( ppm->data != 0 )				/* pixmap existe dj ? */
	{
		if ( ppm->dx == dim.x && ppm->dy == dim.y )
		{
			ClearMem( ppm->data, ((u_long)ppm->dxb)*((u_long)ppm->dy), fill);
			return 0;					/* pixmap existe dj avec la bonne taille */
		}
		else
		{
			free(ppm->data);			/* libre le pixmap prcdent */
			ppm->data = 0;
		}
	}

	ppm->dx     = dim.x;
	ppm->dy     = dim.y;
	ppm->nbp    = nbp;				/* nb de bits/pixel selon noir-blanc/couleur */
	ppm->dxb    = ((dim.x+15)/16)*2 * ppm->nbp;
	ppm->ccolor = COLORBLANC;
	ppm->scolor = COLORNOIR;
	ppm->data   = malloc( ((u_long)ppm->dxb)*((u_long)ppm->dy) );
	if (ppm->data == NULL)
	{
		ppm->data =0;
		return 1;
	}
	else
	{
		ClearMem( ppm->data, ((u_long)ppm->dxb)*((u_long)ppm->dy), fill);
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
	if ( ppm->data != 0 )
	{
		free(ppm->data);			/* libre le pixmap */
		ppm->data = 0;
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
	ImageSmaky	im;

	err = LoadImage(numero, &im);				/* charge l'image */
	if ( err )  goto error;

	if ( ppm->data == 0            ||
		 ppm->dx != im.head->imdlx ||
		 ppm->dy != im.head->imdly )
	{
		GivePixmap(ppm);						/* libre l'ancien pixmap si ncessaire */
		ppm->dx     = im.head->imdlx;
		ppm->dy     = im.head->imdly;
		ppm->nbp    = im.head->imbip;			/* nb de bits/pixel */
		ppm->dxb    = ((im.head->imdlx+15)/16)*2 * ppm->nbp;
		ppm->ccolor = COLORBLANC;
		ppm->scolor = COLORNOIR;
		ppm->data   = malloc(ppm->dxb*ppm->dy);
		err = 1;
		if ( ppm->data == NULL )  goto error;	/* pas assez de mmoire */
	}

	im.head->imdlx *= im.head->imbip;
	im.head->imbip  = 1;
	err = gra2_decoima(im.data, ppm->data, im.head);

	error:
	if ( err )
	{
		GivePixmap(ppm);						/* libre le pixmap */
	}

	UnloadImage(&im);							/* libre l'image smaky code */

	return err;
}




/* ========= */
/* CacheIcon */
/* ========= */

/*
	Cache en mmoire une icne en vue d'une utilisation prochaine.
	Ceci est utile pour le PC qui n'a pas assez de mmoire pour
	conserver toutes les icnes !
 */

void CacheIcon(short numero)
{
	return;
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
	if ( colormode && (numero&ICONMASK) < ICOMOFF )
	{
		ppm->nbp = pgradesc->dfcnp;		/* 4 bit/pixel car couleur */
	}
	else
	{
		ppm->nbp = 1;					/* 1 bit/pixel car noir-blanc */
	}
	ppm->dxb    = (LXICO*16/8)*ppm->nbp;

	ppm->scolor = COLORNOIR;			/* initialise la couleur chair */
	ppm->ccolor = COLORBLANC;			/* initialise la couleur fond */

	if ( (numero&ICONMASK) < ICOMOFF )
	{
		no = numero;
		if ( (numero&ICONMASK) < 128*1 )
		{
			ppm->data  = pmicon1c.data;
			goto data;
		}
		if ( (numero&ICONMASK) < 128*2 )
		{
			ppm->data  = pmicon2c.data;
			no -= 128*1;
			goto data;
		}
		if ( (numero&ICONMASK) < 128*3 )
		{
			ppm->data  = pmicon3c.data;
			no -= 128*2;
			goto data;
		}
		if ( (numero&ICONMASK) < 128*4 )
		{
			ppm->data  = pmicon4c.data;
			no -= 128*3;
			goto data;
		}
		return 1;
	}
	else
	{
		numero -= ICOMOFF;
		no = numero;
		if ( (numero&ICONMASK) < 128*1 )
		{
			ppm->data  = pmicon1m.data;
			goto data;
		}
		if ( (numero&ICONMASK) < 128*2 )
		{
			ppm->data  = pmicon2m.data;
			no -= 128*1;
			goto data;
		}
		if ( (numero&ICONMASK) < 128*3 )
		{
			ppm->data  = pmicon3m.data;
			no -= 128*2;
			goto data;
		}
		if ( (numero&ICONMASK) < 128*4 )
		{
			ppm->data  = pmicon4m.data;
			no -= 128*3;
			goto data;
		}
		return 1;
	}

	data:
	ppm->data += (LXICO/8L)*((no&ICONMASK)%16)*ppm->nbp;
	ppm->data += (160L*LYICO)*((no&ICONMASK)/16)*ppm->nbp;

	switch ( no&(~ICONMASK) )
	{
		case UL:
			ppm->dx /= 2L;
			ppm->dy /= 2L;
			break;

		case UR:
			ppm->data += (LXICO/8L/2)*ppm->nbp;
			ppm->dx /= 2L;
			ppm->dy /= 2L;
			break;

		case DL:
			ppm->data += (160L*LYICO/2)*ppm->nbp;
			ppm->dx /= 2L;
			ppm->dy /= 2L;
			break;

		case DR:
			ppm->data += (160L*LYICO/2 + LXICO/8L/2)*ppm->nbp;
			ppm->dx /= 2L;
			ppm->dy /= 2L;
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

static int LoadIcon(void)
{
	int		err;

	err = GetImage(&pmicon1c, IMAICON+0);			/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon1m, IMAICON+0+IMAMASK);	/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon2c, IMAICON+1);			/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon2m, IMAICON+1+IMAMASK);	/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon3c, IMAICON+2);			/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon3m, IMAICON+2+IMAMASK);	/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon4c, IMAICON+3);			/* charge l'image des icnes */
	if ( err )  return err;

	err = GetImage(&pmicon4m, IMAICON+3+IMAMASK);	/* charge l'image des icnes */
	if ( err )  return err;

	return 0;
}


/* ------ */
/* AfMenu */
/* ------ */

/*
	Affiche les soft-keys.
 */

static void AfMenu (void)
{
	char		*menu = "\
\0\
Fin\0\
\0\
\0\
       Bruitages \0\
        Oui  Non \0\
  Dcalage       \0\
Prog. Rapide     \0\
     Vitesse     \0\
  1     2     3  \0\
\0\
Sauve Prend Pause\0";

	L_afmenu(menu);		/* affiche les soft-keys */
}



/* =========== */
/* BlackScreen */
/* =========== */

/*
	Efface tout l'cran (noir), pendant le changement de la clut.
 */

void BlackScreen (void)
{
	int		i;
	u_char	t;
	Trame88	trame;

	if ( (pgradesc->dfmod)&0x80 )		/* cran  fond blanc ? */
		t = 0xFF;				/* trame noire */
	else
		t = 0x00;				/* trame blanche */

	for ( i=0 ; i<8 ; i++ )
	{
		trame[i] = t;			/* construit la trame 8x8 */
	}

	HideMouse();				/* cache la souris */

	gra2_trame					/* efface toute la fentre */
	(
		pgradesc,				/* ^descripteur de fentre */
		(Point) {0,0},			/* coin sup/gauche         */
		pgradesc->dfd,			/* dimensions              */
		LOADDOT,				/* mode                    */
		trame					/* trame 8x8 noire/blanche */
	);

	ShowMouse();				/* remet la souris */
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
	char		filename[] = "(:,#:)BLUPIXA.DAT";
	short		n = 0;
	short		err;

	filename[12] = file;
	if ( file >= 'A' )  n = 6;
	channel = fopen(filename+n, "rb");	/* ouvre le fichier */
	if ( channel == NULL )  return errno;

	if ( fseek(channel, pos, SEEK_SET) != 0 )
	{
		err = errno;
		goto close;
	}

	if ( fread(pdata, nb, 1, channel) != 1 )
	{
		err = errno;
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
	char		filename[] = "(:,#:)BLUPIXA.DAT";
	short		n = 0;
	short		err;

	filename[12] = file;
	if ( file >= 'A' )  n = 6;
	channel = fopen(filename+n, "ab");	/* ouvre le fichier */
	if ( channel == NULL )  return errno;

	if ( fseek(channel, pos, SEEK_SET) != 0 )
	{
		err = errno;
		goto close;
	}

	if ( fwrite(pdata, nb, 1, channel) != 1 )
	{
		err = errno;
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
	char		filename[] = "(:,#:)BLUPIXA.DAT";
	short		n = 0;
	long		lg;

	filename[12] = file;
	if ( file >= 'A' )  n = 6;
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
	char		filename[] = ":BLUPIXA.DAT";

	filename[7] = file;
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
	char		oldfilename[] = ":BLUPIXA.DAT";
	char		newfilename[] = "BLUPIXA.DAT";

	oldfilename[7] = oldfile;
	newfilename[6] = newfile;
	return rename(oldfilename, newfilename);
}



/* ========== */
/* FatalError */
/* ========== */

/*
	Affiche une erreur fatale, puis quitte le jeu.
 */

void FatalError (short err)
{
	short		dberr;
	Point		pos;
	char		text[] = "Pas assez de mmoire pendant le jeu !\n\n1) Cliquez VU ou appuyez RETURN\n2) Arrtez le SMAKY\n3) Redmarrez ...";

	dberr = dbox_terror(text, 0, pos);		/* dialogue d'erreur ... */
	if ( dberr )
	{
		printf(text);						/* affiche un texte */
		getchar();
	}

	exit(err);								/* retour dans MAIN */
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


/* -------- */
/* CheckMem */
/* -------- */

/*
	Vrifie s'il reste assez de mmoire en fonction des options
	choisies. Si oui, retourne 1 (true).
 */

short CheckMem (u_long flags)
{
	long		total, biggest;
	long		mem = 0;

	G_argmem(&total, &biggest);			/* demande la mmoire restante */
	total -= 10000;						/* petite marge de 10 kilos ! */

	if ( flags & 1<<0 )  mem += MEMBW+MEMRUNBW;
	if ( flags & 1<<1 )  mem += MEMCOLOR+MEMRUNCOLOR;

	if ( flags & (1<<2|1<<3) )  mem += MEMAUDIO;
	if ( flags & 1<<2 )         mem += MEMBRUIT;
	if ( flags & 1<<3 )         mem += MEMMUSIC;

	return ( mem < total );
}


#define KEYDBOX		0x7E00

/* ======= */
/* DboxMem */
/* ======= */

/*
	Demande quelles ressources utiliser.
	Retourne une erreur != 0 en cas d'erreur (quitter).
 */

short DboxMem (void)
{
	int			err;
	char		ligne[5] = "";
	u_long		dbcanal;
	u_long		flags = 0x0L;
	u_long		taille;
	short		touche;
	char		*pdbox;
	Point		debut, fin;
	char		*menu = "\0\0\0\0\0\0\0\0\0\0\0\0";

	err = res_get(chres, RSGETPOINTER+RSGETnoHAND, 0L, RtypDbox, DBOX_MEM, 0L, &pdbox, &taille);
	if ( err )  return err;
	err = dbox_open(pdbox, DBSAV, (Point){0,0}, flags, &dbcanal);
	if ( err )  return err;

	if ( pgradesc->dfcnp == 1 || !IfFileExist("(:,#:)BLUPIX01.COLOR") )
	{
		dbox_getco(dbcanal, 1+DBGCLABEL+DBGCGREY, &debut, &fin);	/* grise "couleur" */
		flags |= 1<<0;												/* enclenche "noir-blanc */
	}
	else
	{
		flags |= 1<<1;												/* enclenche "couleur" */
	}

	if ( admode != 0 || !IfFileExist("(:,#:)BLUPIX01.AUDIO") )
	{
		dbox_getco(dbcanal, 2+DBGCLABEL+DBGCGREY, &debut, &fin);	/* grise "bruitages" */
	}
	else
	{
		flags |= 1<<2;												/* enclenche "bruitages" */
	}

	if ( admode != 0 || !IfFileExist("(:,#:)BLUPIX02.AUDIO") )
	{
		dbox_getco(dbcanal, 3+DBGCLABEL+DBGCGREY, &debut, &fin);	/* grise "musiques" */
	}
	else
	{
		flags |= 1<<3;												/* enclenche "musiques" */
	}

	if ( !CheckMem(flags) )
	{
		flags &= ~(1<<3);											/* dclenche "musiques" */
		if ( !CheckMem(flags) )
		{
			flags &= ~(1<<2);										/* dclenche "bruitages" */
			if ( !CheckMem(flags) )
			{
				flags &= ~(1<<1);									/* dclenche "couleurs" */
				flags |=   1<<0;									/* enclenche "noir-blanc" */
			}
		}
	}

	if ( CheckMem(flags) )  dbox_getco(dbcanal, 50+DBGCLABEL+DBGCFULL, &debut, &fin);
	else                    dbox_getco(dbcanal, 50+DBGCLABEL+DBGCGREY, &debut, &fin);

	ShowMouse();							/* utilise la souris */
	do
	{
		dbox_edit (dbcanal, ligne, &flags, &touche);
		if ( touche == KEYDBOX )
		{
			if ( CheckMem(flags) )  dbox_getco(dbcanal, 50+DBGCLABEL+DBGCFULL, &debut, &fin);
			else                    dbox_getco(dbcanal, 50+DBGCLABEL+DBGCGREY, &debut, &fin);
		}
	}
	while ( touche != F1  && touche != F2  && touche != F3  &&
			touche != F13 && touche != F14 && touche != F15 );
	dbox_close (dbcanal);
	HideMouse();							/* enlve la souris */
	L_afmenu(menu);							/* affiche les soft-keys */

	if ( flags & 1<<0 )  colormode = 0;		/* noir-blanc */
	else                 colormode = 1;		/* couleur */

	if ( flags & 1<<2 )  sound1 = 1;		/* bruitages */
	if ( flags & 1<<3 )  sound2 = 1;		/* musiques */

	if ( touche == F13 || touche == F14 || touche == F15 )  return 1;
	return 0;
}


/* --------- */
/* FatalLoad */
/* --------- */

/*
	Gestion des erreurs fatales de chargement  l'initialisation.
 */

static void FatalLoad(char *name, int err)
{
	char*		errorname;

	errorname = strerror(err);
	if ( (int)errorname == 0 )
	{
		fprintf(stderr, "Erreur fatale numro %d avec %s ", err, name);
	}
	else
	{
		fprintf(stderr, "Erreur fatale %s avec %s ", errorname, name);
	}
	getchar();						/* attend une touche ... */

	CloseMachine();					/* libre les ressources */
	exit(err);						/* retour dans MAIN */
}


/* =========== */
/* OpenMachine */
/* =========== */

/*
	Ouverture gnrale, chargement des librairies, gencar, etc.
 */

void OpenMachine(void)
{
	int			err;					/* erreur rendue */
	long		pid;					/* identificateur du processus cr */
	u_short		pino;					/* numro du processus cre */
	u_int		adnombre;				/* nombre de sons audio */

	L_mouse(2);							/* souris par le clavier */
	HideMouse();						/* enlve la souris */

	printf("%c%c%c",
		NOCURS,							/* enlve le curseur */
		MODGRA,1						/* mode GRACLIP */
		);

	gra2_open(&pgradesc);				/* ouverture du module GRA2 */

	err = res_opelib();					/* ouverture du module RES */
	if ( err )  FatalLoad("res", err);

	err = dbox_opelib();				/* ouverture du module DBOX */
	if ( err )  FatalLoad("dbox", err);

	err = audio_opelib();				/* ouverture du module AUDIO */
	if ( err )  FatalLoad("audio", err);

	err = audio_open(&adcanal, &admode);	/* ouvre la librairie audio */
	if ( err )  FatalLoad("audio", err);

	err = res_open ("(:,#:)BLUPIX(_%L,).RS", RSOPCACHE, 0x10000*MAJREV+MINREV, &chres);
	if ( err )  FatalLoad("BLUPIX.RS", err);

	err = DboxMem();					/* demande comment dmarrer ... */
	if ( err )  exit(err);				/* retour dans MAIN */

	if ( colormode )
	{
		err = tcol_opelib();			/* ouverture du module TCOLOR */
		if ( err )  FatalLoad("tcolor", err);
		err = tcol_litcanal(&tcolcanal);
		if ( err )  FatalLoad("tcolor", err);
	}

	if ( sound1 )  audio_cache(&adcanal, "(:,#:)BLUPIX01.AUDIO", &adcanalson1, &adnombre);
	if ( sound2 )  audio_cache(&adcanal, "(:,#:)BLUPIX02.AUDIO", &adcanalson2, &adnombre);

	err = N_cretask(FilsSound, 200, 10, "BLUPIX_PLAY", 0, 0, &pid, &pino);

	err = LoadIcon();					/* charge l'image des icnes */
	if ( err )  FatalLoad("icnes", err);

	origine.x = ((pgradesc->dfd.x)-LXIMAGE)/2;
	origine.y = ((pgradesc->dfd.y)-LYIMAGE)/2;

	StartRandom(0, 0);					/* coup de sac du gnrateur alatoire toto */
	StartRandom(1, 1);					/* coup de sac du gnrateur alatoire dcor */

	L_repkey(-1, 0);					/* pas de repeat automatique */
	keystatus = 0;

	AfMenu();							/* affiche les soft-keys */
}


/* ============ */
/* CloseMachine */
/* ============ */

/*
	Fermeture gnrale.
 */

void CloseMachine(void)
{
	short		max;

	GivePixmap(&pmicon1c);			/* libre les pixmap des icnes */
	GivePixmap(&pmicon1m);
	GivePixmap(&pmicon2c);
	GivePixmap(&pmicon2m);
	GivePixmap(&pmicon3c);
	GivePixmap(&pmicon3m);
	GivePixmap(&pmicon4c);
	GivePixmap(&pmicon4m);

	PlaySoundLoop(0);				/* plus de rptition */
	filsabort = 1;					/* fin du processus fils */
	max = 10;
	while ( filsson != 0 && max != 0 )
	{
		N_delms(5);					/* attend la fin du fils ... */
		max --;
	}
	N_delms(5);						/* attend la fin du fils ... */

	if ( adcanalson1 != 0 )  audio_uncache(&adcanal, &adcanalson1);
	if ( adcanalson2 != 0 )  audio_uncache(&adcanal, &adcanalson2);
	audio_close(&adcanal);			/* ferme la librairie audio */
	audio_clolib();					/* fermeture du module AUDIO */

	res_close(chres);				/* ferme le fichier de ressources */

	if ( colormode )
	{
		tcol_clolib();				/* fermeture du module TCOLOR */
	}
	dbox_clolib();					/* fermeture du module DBOX */
	res_clolib();					/* fermeture du module RES */
	gra2_close();					/* fermeture du module GRA2 */
	ShowMouse();					/* remet la souris */
}




/* =============== */
/* MachinePartieLg */
/* =============== */

/*
	Retourne la longueur ncessaire pour sauver les variables de la partie en cours.
 */

long MachinePartieLg (void)
{
	return sizeof(u_long)*10;
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

	err = FileWrite(&nextrand, pos, sizeof(u_long)*10, file);
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

	err = FileRead(&nextrand, pos, sizeof(u_long)*10, file);
	return err;
}



/* ======= */
/* SetDemo */
/* ======= */

/*
	Modifie le mode "demo".
 */

void SetDemo (char bDemo)
{
	demo = bDemo;
}

/* ======= */
/* GetDemo */
/* ======= */

/*
	Retourne le mode "demo".
	0 -> normal
	1 -> demo
 */

char GetDemo (void)
{
#if 1
	return demo;
#endif
return 1;
}

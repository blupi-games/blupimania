
/* ========= */
/* bm_play.c */
/* ========= */

/*
	Version	Date		Modifications
	-------------------------------------------------------------------------------
	1.0	24.08.94	dbut des travaux ...
 */

#include <stdio.h>
#include <string.h>

#include "bm.h"
#include "bm_icon.h"
#include "actions.h"



#if 1
#define VOLUME	1			/* version avec potentiomtres pour le volume */
#endif

#if 0
#define DEMONC	1			/* si version demo -> pas de construction */
#endif



/* Fichiers sur disque */
/* ------------------- */

/*
	BLUPIX?.DAT		banque	contenu
	---------------------------------------------------------------------
	1..8			A..H	nigmes niveaux 1A, 1T, 2A, 2T, ... 4T
	E..H			I..L	nigmes niveaux 5 (priv) pour les 4 joueurs
	I..P			A..H	nigmes mode dmo
	W						parties sauves (dmo)
	X						joueurs (dmo)
	Y						parties sauves (normal)
	Z						joueurs (normal)
 */



/* ------------- */
/* Phases du jeu */
/* ------------- */

typedef enum
{
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
	PHASE_ATTENTE
}
Phase;



/* ----------------------------- */
/* Actions pour changer de phase */
/* ----------------------------- */

typedef enum
{
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
	ACTION_QUITTE
}
PhAction;



/* -------------------------- */
/* Animations dans les images */
/* -------------------------- */

typedef enum
{
	ANIM_JOUE,
	ANIM_CHOIX,
	ANIM_QUITTE
}
Anim;





/* -------------------------------- */
/* Structure du fichier des joueurs */
/* -------------------------------- */

#define MAXJOUEUR		4					/* nb max de joueurs dffrents */
#define MAXNOMJ			40					/* lg max du nom d'un joueur */

typedef struct
{
	short	majrev, minrev;					/* rvision.version du logiciel */
	short	check;							/* anti-bidouilleurs */
	short	reserve1[100];					/* rserve */
	short	joueur;							/* dernier joueur utilis */
	char	nom[MAXJOUEUR][MAXNOMJ];		/* noms des joueurs */
	short	niveau[MAXJOUEUR];				/* niveau de difficult */
	short	progres[MAXJOUEUR][9];			/* mondes atteints selon les niveaux */
	short	vitesse;						/* vitesse (0..2) */
	short	scroll;							/* scroll (0..1) */
	short	bruitage;						/* bruitages (0..1) */
	short	modetelecom;					/* mode de la tlcommande (0..1) */
	short	noisevolume;					/* volume bruitages */
	short	musicvolume;					/* volume musique */
	short	reserve2[93];					/* rserve */
}
Joueur;



/* --------------------------- */
/* Variables globales externes */
/* --------------------------- */

short		g_langue = 0;						/* numro de la langue */
short		g_monde;							/* monde actuel */
short		g_updatescreen;					/* 1 -> cran  mettre  jour */
short		g_typejeu;						/* type de jeu (0..1) */
short		g_typeedit;						/* 1 -> dition d'un monde */
short		g_typetext;						/* 1 -> dition d'un texte */
short		g_modetelecom;					/* 1 -> mode tlcommande gauche/droite */
short		g_pause;							/* 1 -> pause */
short		g_passdaniel;						/* 1 -> toujours construction */
short		g_passpower;						/* 1 -> force infinie */
short		g_passnice;						/* 1 -> toujours gentil */
short		g_passhole;						/* 1 -> ne tombe pas dans trou */
short		g_construit;						/* 1 -> construit */

SDL_Renderer * g_renderer;
SDL_Window *   g_window;
Pixmap g_screen = { 0 };

int g_rendererType   = 0;
Sint32	g_timerInterval = 25;
Sint32  g_timerSkip = 4;
Pt g_lastmouse = {0};
SDL_bool g_clearKeyEvents = SDL_FALSE;


/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Pixmap	pmimage = {0,0,0,0,0,0,0};	/* pixmap pour image */
static Pixmap	pmtemp  = {0,0,0,0,0,0,0};	/* pixmap temporaire */
static Phase	phase;						/* phase du jeu */
static SDL_bool    ignoreKeyClicUp = SDL_FALSE;                      /* Previous step */
static char		banque;						/* banque utilise */
static short	mondeinit;					/* numro du monde initial */
static short	maxmonde;					/* nb max de mondes */
static Monde	descmonde;					/* description du monde */
static Monde	savemonde;					/* sauvetage d'un monde */
static Joueur	fj;							/* fichier des joueurs */
static char		lastkey;					/* dernire touche presse */
static SDL_bool fromClic = SDL_FALSE;
static short	retry;						/* nb de tentatives */
static short	generic;					/* pas du gnrique */
static short	musique = 0;				/* musique de fond */
static short	lastaccord = -1;			/* dernier accord jou */
static char		musiquehex[10];				/* musique hazard exclusif */
static short	lastnoisevolume = 8-3;		/* dernier volume bruiutages */
static short	lastmusicvolume = 8-3;		/* dernier volume musique */

static short*	animpb;						/* animation de base en cours */
static short*	animpt;						/* animation en cours */
static short	animnext;					/* pas en cours (0..n) */
static short	animdel;					/* dlai avant la premire animation */
static Pt		animpos;					/* dernire position de la souris */

static short	passrang;					/* rang du mot de passe */
static short	passindex;					/* index de l'dition du mot de passe */

static char		randomexrecommence[30];		/* tirage exclusif texte si recommence */
static char		randomexsuivant[30];		/* tirage exclusif texte si russi */




typedef struct
{
	short		ident;						/* identificateur */
	int		lg[10];						/* longueurs */
	short		reserve[10];				/* rserve */
}
Header;

typedef struct
{
	int		check;						/* vrification */
	short		monde;						/* monde actuel */
	short		typejeu;					/* type de jeu (0..1) */
	char		banque;						/* banque utilise */
	short		reserve[10];				/* rserve */
}
Partie;



	void	AnimDrawInit	(void);
static	void	PlayRelease		(void);
	void	FatalBreak		(short err);





#define ___		ICO_SOLPAVE					/* sol pav normal */

static short tabmonde[MAXCELY][MAXCELX] =
{
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
	{___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___}
};


/* Palette d'icnes pendant la construction */
/* ---------------------------------------- */

static short tabpalette[] =
{
	ICO_OUTIL_TRACKS, 999, ICO_OUTIL_SOLPAVE, 999,
	ICO_OUTIL_SOLCARRE, 999, ICO_OUTIL_SOLDALLE1, 999, ICO_OUTIL_SOLDALLE2, 999,
	ICO_OUTIL_SOLDALLE3, 999, ICO_OUTIL_SOLDALLE4, 999, ICO_OUTIL_SOLDALLE5, 999,
	ICO_OUTIL_SOLELECTRO, 999, ICO_OUTIL_SOLOBJET, 999,
	-1,
	ICO_OUTIL_BARRIERE, 999, ICO_OUTIL_MUR, 999, ICO_OUTIL_VITRE, 999,
	ICO_OUTIL_PLANTE, 999, ICO_OUTIL_PLANTEBAS, 999, ICO_OUTIL_BOIS, 999,
	ICO_OUTIL_ELECTRO, 999, ICO_OUTIL_ELECTROBAS, 999,
	ICO_OUTIL_TECHNO, 999, ICO_OUTIL_MEUBLE, 999,
	ICO_OUTIL_OBSTACLE, 999,
	-1,
	ICO_OUTIL_VISION, 999, ICO_OUTIL_BOIT, 999,
	ICO_OUTIL_AIMANT, 999,
	ICO_OUTIL_LIVRE, 999, ICO_OUTIL_MAGIC, 999,
	ICO_OUTIL_SENSUNI, 999, ICO_OUTIL_UNSEUL, 999,
	ICO_OUTIL_CLE, 999, ICO_OUTIL_PORTE, 999,
	ICO_OUTIL_DETONATEUR, 999, ICO_OUTIL_BOMBE, 999,
	ICO_OUTIL_BAISSE, 999,
	-1,
	ICO_OUTIL_DEPART, 999, ICO_OUTIL_JOUEUR, 999,
	ICO_OUTIL_TANK, 999, ICO_OUTIL_INVINCIBLE, 999,
	ICO_OUTIL_TROU, 999, ICO_OUTIL_ACCEL, 999,
	ICO_OUTIL_GLISSE, 999, ICO_OUTIL_CAISSE, 999,
	ICO_OUTIL_ARRIVEE, 999,
	-1,
	0
};

static short tabpalette0[] =
{
	ICO_OUTIL_TRACKSBAR, 10, -1,
	ICO_OUTIL_BARRIERE, 10, -1,
	ICO_OUTIL_VISION, 10, -1,
	ICO_OUTIL_BOIT, 10, -1,
	0
};






/* ----------- */
/* PlayEvSound */
/* ----------- */

/*
	Fait entendre un bruitage seulement s'il n'y a pas déjà une musique en cours.
 */

void PlayEvSound (short sound)
{
#if __SMAKY__
	if ( musique != 0 )  return;		/* rien si musique en cours */
#endif
	PlaySound(sound);
}



/* ------ */
/* PutNum */
/* ------ */

/*
	Met un nombre entier positif en base dix dans un buffer.
	Si nbdigits = 0, met juste le nombre nécessaire de digits.
 */

void PutNum (char **ppch, short num, short nbdigits)
{
	short		i;
	short		shift = 10000;
	short		digit;
	char		put = 0;

	if ( nbdigits > 0 )
	{
		for( i=nbdigits ; i>0 ; i-- )
		{
			*(*ppch+i-1) = (num%10)+'0';
			num /= 10;
		}
		*ppch += nbdigits;
	}
	else
	{
		for( i=4 ; i>=0 ; i-- )
		{
			digit  = (num/shift)%10;
			shift /= 10;
			if ( put == 1 || digit != 0 || i == 0 )
			{
				*(*ppch)++ = digit + '0';
				put = 1;
			}
		}
	}
	**ppch = 0;
}



/* --------- */
/* MondeEdit */
/* --------- */

/*
	Prpare un monde pour pouvoir l'diter tranquillement, avec tous les outils.
 */

void MondeEdit (void)
{
	short		i = 0;

	do
	{
		descmonde.palette[i] = tabpalette[i];
	}
	while ( tabpalette[i++] != 0 );
}


/* --------- */
/* MondeVide */
/* --------- */

/*
	Initialise un monde entirement vide.
 */

void MondeVide (void)
{
	short		x, y;
	short		i = 0;

	memset(&descmonde, 0, sizeof(Monde));

	for ( y=0 ; y<MAXCELY ; y++ )
	{
		for ( x=0 ; x<MAXCELX ; x++ )
		{
			descmonde.tmonde[y][x] = tabmonde[y][x];
		}
	}

	descmonde.freq = 50;

	do
	{
		descmonde.palette[i] = tabpalette0[i];
	}
	while ( tabpalette0[i++] != 0 );
}


/* ------------ */
/* BanqueToFile */
/* ------------ */

/*
	Conversion d'une banque en fichier.
 */

char BanqueToFile (char banque)
{
	if ( banque >= 'a' && banque <= 'h' )
	{
		if ( GetDemo() == 0 )  return banque-'a'+'1';	/* 1..8 */
		else                   return banque-'a'+'i';	/* i..p */
	}
	if ( banque >= 'i' && banque <= 'l' )  return banque-'i'+'e';	/* e..h */

	return banque;
}

/* -------- */
/* MondeMax */
/* -------- */

/*
	Cherche le nombre maximum de mondes possibles.
 */

void MondeMax (char banque)
{
	short	max = 2;

	maxmonde = FileGetLength(BanqueToFile(banque))/sizeof(Monde);

	if ( banque <= 'b' )  max = 5;
	if ( banque == 'd' ||
		 banque == 'f' ||
		 banque == 'h' )  max = 3;

	if ( GetDemo() == 1 && maxmonde > max )  maxmonde = max;

	if ( g_construit )  maxmonde ++;			/* si construit -> toujours un monde vide  la fin */
	if ( GetDemo() == 1 && g_construit )  maxmonde = 1;
}



#ifdef __MSDOS__
void convshort (unsigned short *s)
{
	char	t;
	char 	*p = (char*)s;

	t    = p[0];
	p[0] = p[1];
	p[1] = t;
}

void ConvMonde (Monde *m)
{
	int		i, j;

	for ( i=0 ; i<MAXCELX ; i++ )
		for ( j=0 ; j<MAXCELY ; j++ )
			convshort(&m->tmonde[j][i]);

	for ( i=0 ; i<MAXPALETTE ; i++ )
		convshort(&m->palette[i]);

	convshort(&m->freq);
	convshort(&m->color);
}
#endif

/* --------- */
/* MondeRead */
/* --------- */

/*
	Lit un nouveau monde sur le disque.
	Retourne 0 si la lecture est ok.
 */

short MondeRead (short monde, char banque)
{
	short           err = 0;
	short           max;

	if ( g_construit && GetDemo() == 0 )  max = maxmonde-1;
	else                                max = maxmonde;

	if ( monde >= max )  goto vide;

	err = FileRead(&descmonde, monde*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
	if ( err )
	{
		maxmonde = 0;
		goto vide;
	}
#ifdef __MSDOS__
	ConvMonde (&descmonde) ;
#endif
	return 0;

	vide:
	MondeVide();            /* retourne un monde vide */
	return err;
}


/* ---------- */
/* MondeWrite */
/* ---------- */

/*
	Ecrit un monde sur le disque.
	Retourne 0 si l'�criture est ok.
 */

short MondeWrite (short monde, char banque)
{
	short           err;

#ifdef __MSDOS__
	ConvMonde (&descmonde) ;
#endif
	err = FileWrite(&descmonde, monde*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
#ifdef __MSDOS__
	ConvMonde (&descmonde) ;
#endif
	MondeMax(banque);

	return err;
}



/* ---------- */
/* JoueurRead */
/* ---------- */

/*
	Lit le fichier des joueurs sur le disque.
	Retourne 0 si la lecture est ok.
 */

short JoueurRead (void)
{
	short		err;

	memset(&fj, 0, sizeof(Joueur));			/* met tout  zro */
	fj.vitesse = 1;							/* vitesse normale */

	err = FileRead(&fj, 0, sizeof(Joueur), GetDemo()?'x':'z');
	if ( err )
	{
		fj.noisevolume = 10-3;
		fj.musicvolume = 10-3;
	}

	   g_modetelecom = fj.modetelecom;

	PlayNoiseVolume(fj.noisevolume);
	PlayMusicVolume(fj.musicvolume);
	return err;
}


/* ----------- */
/* JoueurWrite */
/* ----------- */

/*
	Ecrit le fichier des joueurs sur le disque.
	Retourne 0 si l'criture est ok.
 */

short JoueurWrite (void)
{
	short		err;

	err = FileWrite(&fj, 0, sizeof(Joueur), GetDemo()?'X':'Z');
	return err;
}



/* ------------------- */
/* ConvPhaseToNumImage */
/* ------------------- */

/*
	Retourne le numro d'image correspondant  une phase de jeu.
 */

short ConvPhaseToNumImage (Phase ph)
{
	switch ( ph )
	{
		case PHASE_PLAY:        return 20;
		case PHASE_INIT:        return 21;
		case PHASE_OBJECTIF:    return 22;
		case PHASE_RECOMMENCE:  return 23;
		case PHASE_SUIVANT:     return 24;
		case PHASE_PRIVE:       return 25;
		case PHASE_PARAM:       return 26;
		case PHASE_DEPLACE:     return 27;
		case PHASE_AIDE21:      return 28;
		case PHASE_ATTENTE:     return 29;
		case PHASE_OPER:        return 30;
		case PHASE_IDENT:       return 31;
		case PHASE_AIDE22:      return 32;
		case PHASE_FINI0:       return 33;
		case PHASE_FINI1:       return 33;
		case PHASE_FINI2:       return 33;
		case PHASE_FINI3:       return 33;
		case PHASE_FINI4:       return 33;
		case PHASE_FINI5:       return 33;
		case PHASE_FINI6:       return 33;
		case PHASE_FINI7:       return 33;
		case PHASE_FINI8:       return 33;
		case PHASE_AIDE23:      return 34;
		case PHASE_REGLAGE:     return 35;
		case PHASE_GENERIC:     return 36;
		case PHASE_AIDE31:      return 40;
		case PHASE_AIDE32:      return 41;
		case PHASE_AIDE33:      return 42;
		case PHASE_AIDE34:      return 43;
		case PHASE_AIDE35:      return 44;
		case PHASE_AIDE36:      return 45;
		case PHASE_AIDE:        return 46;
		case PHASE_AIDE41:      return 47;
		case PHASE_AIDE42:      return 48;
		case PHASE_AIDE24:      return 49;
		case PHASE_AIDE11:      return 50;
		case PHASE_AIDE12:      return 51;
		case PHASE_AIDE13:      return 52;
	}
	return -1;
}



/* Codage des lettres accentues */
/* ----------------------------- */

/*	272		"a" aigu		*/
/*	271		"a" grave		*/
/*	270		"a" circonflxe	*/
/*	267		"a" trma		*/
/*	266		"e" aigu		*/
/*	265		"e" grave		*/
/*	264		"e" circonflxe	*/
/*	263		"e" trma		*/
/*	262		"i" aigu		*/
/*	261		"i" grave		*/
/*	260		"i" circonflxe	*/
/*	257		"i" trma		*/
/*	256		"o" aigu		*/
/*	255		"o" grave		*/
/*	254		"o" circonflxe	*/
/*	253		"o" trma		*/
/*	252		"u" aigu		*/
/*	251		"u" grave		*/
/*	250		"u" circonflxe	*/
/*	247		"u" trma		*/
/*	246		"c" cdille		*/
/*	245		"C" cdille		*/


/* --------- */
/* ShowImage */
/* --------- */

/*
	Affiche une image de base dans la fentre.
 */

void ShowImage (void)
{
	Pt			p;
	Rectangle	rect;
	char		*ptx;
	short		image, err, nbessai, max;

	static char *txrecommence[19] =
	{
#if FRENCH
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
		"\013\144Empoigne le probl\265me par un autre bout ?"
#endif
	};

	static char *txsuivant[20] =
	{
#if FRENCH
		"\001\001Super extra chouette, c'est r\266ussi du premier coup !",
		"\001\001Bravo champion, z\266ro faute ...",
		"\001\001Extra, la perfection quoi !",
		"\001\001Super extra hyper m\266ga,\neuh ... tr\265s bien, quoi !",
		"\001\001En un mot comme en mille:\nB-R-A-V-O, bravo, bravo ...",
		"\002\003Bravo, super, c'est r\266ussi !",
		"\002\003Champion, c'est juste !",
		"\002\003Parfait, tu peux passer \271 l'\266nigme suivante ...",
		"\002\003Tr\265s bien, mais la prochaine \266nigme sera peut-\264tre \
beaucoup plus difficile !",
		"\002\003Youpie, c'est tout juste !",
		"\004\006Ouaip, c'est dans la poche !",
		"\004\006\245a va, tu peux passer \271 l'\266nigme suivante ...",
		"\004\006Correct, passe plus loin.",
		"\004\006OK. (point \271 la ligne)",
		"\007\012\245a ira pour cette fois, mais c'\266tait dur dur, non ?",
		"\007\012Bon, \246a passe pour cette fois, mais t\270che d'y arriver \
plus vite la prochaine fois !",
		"\007\012Sans commentaire, \246a vaut mieux ...",
		"\013\144Ouf, c'est enfin r\266ussi. Bel effort ...",
		"\013\144Bravo, que d'efforts pour en arriver l\271 !",
		"\013\144C'est le moment ...\nEsp\266rons que l'\266nigme suivante sera plus facile ... \
mais rien n'est moins s\250r !"
#endif
	};

	static char *txfini[9] =
	{
#if FRENCH
		"Bravo, tu as termin\266 la premi\265re partie du niveau\0011.\n\
Essaye maintenant la deuxi\265me partie, en t\266l\266commandant BLUPI\001...",
		"Bravo, le niveau 1 est termin\266.\nEssaye maintenant le niveau\0012\001...",

		"Bravo, tu as termin\266 le niveau\0012, lorsque BLUPI est autonome.\n\
T\266l\266commande maintenant BLUPI dans la deuxi\265me partie\001...",
		"Bravo, tu as termin\266 le niveau\0012.\n\
Penses-tu pouvoir r\266soudre le niveau\0013 (c'est dur dur)\001?",

		"Formidable, tu as termin\266 la premi\265re partie du niveau\0013\001!\n\
Attaque maintenant la deuxi\265me partie de ce niveau\001...",
		"Formidable, tu as termin\266 le niveau\0013\001!\n\
Il reste le niveau\0014, mais attention, c'est du b\266ton\001...",

		"Hyper extra m\266ga chouette !\n\
Mais attention, la deuxi\265me partie (avec BLUPI t\266l\266command\266) \
n'est pas franchement facile\001...",
		"Hyper extra m\266ga chouette !\nCe jeu n'a plus de secrets pour toi. \
Heureusement, tu peux encore dessiner tes propres \266nigmes, \
pour tes copains (niveau\0015)\001...",

		"Tr\265s bien, tu as termin\266 le niveau\0015.\n\
Essaye encore de dessiner d'autres \266nigmes plus difficiles\001..."
#endif
	};

	if ( phase != PHASE_GENERIC )
	{
		BlackScreen();							/* efface tout l'cran */
	}

	image = ConvPhaseToNumImage(phase);

#ifdef VOLUME
	if ( image == 35 )  image = 37;				/* image avec potentiomtres pour le volume */
#endif

	if ( image == 33 &&
		 GetDemo() == 1 )  image = 38;			/* image avec avertissement "demo" */

	if ( image == 25 &&
		 GetDemo() == 1 )  image = 39;			/* image sans choix de l'nigme */

	err = GetImage(&pmimage, image);
	if ( err )  FatalBreak(err);				/* erreur fatale */

	nbessai = retry+1;
	if ( nbessai > 100 )  nbessai = 100;

	if ( phase == PHASE_RECOMMENCE )
	{
		max = 0;
		do
		{
			ptx = txrecommence[GetRandomEx(1,0,19,randomexrecommence)];
			max ++;
		}
		while ( (nbessai < ptx[0] || nbessai > ptx[1]) && max < 100 );
		rect.p1.x = 113;
		rect.p1.y = LYIMAGE()-319;
		rect.p2.x = 113+446;
		rect.p2.y = LYIMAGE()-319+72;
		DrawParagraph(&pmimage, rect, ptx+2, TEXTSIZEMID, MODEOR);
	}

	if ( phase == PHASE_SUIVANT )
	{
		max = 0;
		do
		{
			ptx = txsuivant[GetRandomEx(1,0,20,randomexsuivant)];
			max ++;
		}
		while ( (nbessai < ptx[0] || nbessai > ptx[1]) && max < 100 );
		rect.p1.x = 85;
		rect.p1.y = LYIMAGE()-275;
		rect.p2.x = 85+470;
		rect.p2.y = LYIMAGE()-275+163;
		DrawParagraph(&pmimage, rect, ptx+2, TEXTSIZEMID, MODEOR);
	}

	if ( phase >= PHASE_FINI0 && phase <= PHASE_FINI8 )
	{
		ptx = txfini[phase-PHASE_FINI0];
		rect.p1.x = 85;
		rect.p1.y = LYIMAGE()-266;
		rect.p2.x = 85+470;
		rect.p2.y = LYIMAGE()-266+190;
		DrawParagraph(&pmimage, rect, ptx, TEXTSIZEMID, MODEOR);
	}

	Pt dim = {pmimage.dy, pmimage.dx};
        Pt orig = {0, 0};
	CopyPixel									/* affiche l'image de base */
	(
		&pmimage, orig,
		0, orig,
		dim, MODELOAD
	);

	if ( phase == PHASE_PLAY )
	{
		GivePixmap(&pmimage);					/* libre l'image si jeu */
	}
	else
	{
		AnimDrawInit();							/* affiche les animations au dpart */
	}
}


/* Table des couleurs pendant le jeu */
/* --------------------------------- */

static short tcolor[] =
{
	0,	0,	0xFF,0xFF,0xFF,
	0,	1,	0xFF,0xFF,0x00,
	0,	2,	0xFF,0xCC,0x40,
	0,	3,	0xFF,0x00,0x00,
	0,	4,	0xDC,0xDC,0xDC,
	0,	5,	0xBE,0xBE,0xBE,
	0,	6,	0x00,0xFF,0xFF,
	0,	7,	0x00,0x00,0xFF,
	0,	8,	0x00,0xFF,0x00,
	0,	9,	0x00,0xCD,0x00,
	0,	10,	0xE0,0xA1,0xFF,
	0,	11,	0xFF,0x00,0xFF,
	0,	12,	0xDB,0x95,0x61,
	0,	13,	0xB9,0x6B,0x34,
	0,	14,	0xA9,0xD8,0xFF,

	1,	0,	0xFF,0xFF,0xFF,
	1,	1,	0xFF,0xFF,0x69,
	1,	2,	0xFF,0xCE,0x49,
	1,	3,	0xFF,0x91,0x91,
	1,	4,	0xCD,0xCD,0xCD,
	1,	5,	0xB4,0xB4,0xB4,
	1,	6,	0x96,0xFF,0xFF,
	1,	7,	0xB9,0xB7,0xFF,
	1,	8,	0xAC,0xFF,0xAC,
	1,	9,	0x8D,0xCD,0x8D,
	1,	10,	0xE9,0xB2,0xFF,
	1,	11,	0xFF,0x98,0xFA,
	1,	12,	0xF2,0xAE,0x8C,
	1,	13,	0xCD,0x85,0x85,
	1,	14,	0xA7,0xD3,0xFF,

	2,	0,	0xD0,0xD0,0xD0,
	2,	1,	0xD3,0xD3,0x00,
	2,	2,	0xCB,0xA2,0x33,
	2,	3,	0xB9,0x00,0x00,
	2,	4,	0xA0,0xA0,0xA0,
	2,	5,	0x78,0x78,0x78,
	2,	6,	0x00,0xB7,0xB7,
	2,	7,	0x03,0x00,0xCB,
	2,	8,	0x00,0xD5,0x00,
	2,	9,	0x00,0xA3,0x00,
	2,	10,	0xBA,0x85,0xD3,
	2,	11,	0xBB,0x00,0xB8,
	2,	12,	0xB1,0x6B,0x36,
	2,	13,	0x8D,0x3B,0x00,
	2,	14,	0x8D,0xB4,0xD5,

	3,	0,	0xFF,0xFF,0xFF,
	3,	1,	0xFF,0xFF,0x00,
	3,	2,	0xFF,0xD8,0x6E,
	3,	3,	0xFF,0x00,0xB7,
	3,	4,	0xDC,0xC7,0xC7,
	3,	5,	0xBE,0xAD,0xAD,
	3,	6,	0xFF,0xEE,0xFF,
	3,	7,	0xEB,0x85,0xFF,
	3,	8,	0x00,0xFF,0x00,
	3,	9,	0x00,0xCD,0x00,
	3,	10,	0xF4,0xDA,0xFF,
	3,	11,	0xFF,0xA6,0xFE,
	3,	12,	0xD3,0x97,0x69,
	3,	13,	0xD1,0x6F,0x0D,
	3,	14,	0xD8,0xB1,0xFF,

	4,	0,	0xFF,0xFF,0xFF,
	4,	1,	0xE9,0xFC,0x3F,
	4,	2,	0xED,0xC7,0x5E,
	4,	3,	0xED,0x00,0x7B,
	4,	4,	0xC6,0xC6,0xDC,
	4,	5,	0xA8,0xA5,0xBE,
	4,	6,	0xB9,0xDD,0xFF,
	4,	7,	0x88,0x83,0xD4,
	4,	8,	0x00,0xFF,0xD5,
	4,	9,	0x00,0xCD,0xB2,
	4,	10,	0xD0,0xA1,0xFF,
	4,	11,	0xB6,0x00,0xED,
	4,	12,	0xBE,0x9A,0x72,
	4,	13,	0xC5,0x5A,0x1C,
	4,	14,	0xAB,0xAF,0xFF,

	-1
};

/* ------------- */
/* ChangeCouleur */
/* ------------- */

/*
	Change les couleurs de la palette pendant le jeu.
 */

void ChangeCouleur (void)
{
	short	i = 0;

	if ( !IfColor() )  return;

	while (1)
	{
		if ( tcolor[i] == -1 )  break;

		if ( tcolor[i] == descmonde.color )
		{
			ModColor(tcolor[i+1], tcolor[i+2],tcolor[i+3],tcolor[i+4]);
		}

		i += 5;
	}
}



/* --------------- */
/* DrawRadioButton */
/* --------------- */

/*
	Dessine un bouton rond relch ou enfonc.
 */

void DrawRadioButton (Pt pos, short state)
{
	Pixmap		pm;
	short		icon;
	Pt			src, dim;

	src.x = 0;
	src.y = 0;

	dim.x = 31;
	dim.y = 31;

	if ( state )  icon = ICO_BUTTON_ROND1;
	else          icon = ICO_BUTTON_ROND0;

	GetIcon(&pm, icon, 1);
	CopyPixel(&pm, src, 0, pos, dim, MODELOAD);		/* dessine le bouton */
}


/* ---------- */
/* DrawJoueur */
/* ---------- */

/*
	Dessine le numro du joueur en enfonant un bouton rond.
 */

void DrawJoueur (void)
{
	short		i;
	Pt			pos;

	pos.x = 241;
	pos.y = LYIMAGE()-297-1;

	for ( i=0 ; i<MAXJOUEUR ; i++ )
	{
		if ( fj.joueur == i )  DrawRadioButton(pos, 1);
		else                   DrawRadioButton(pos, 0);
		pos.y += 40;
	}
}


/* ----------- */
/* DrawVitesse */
/* ----------- */

/*
	Dessine la vitesse en enfonant un bouton rond.
 */

void DrawVitesse (void)
{
	short		i;
	Pt			pos;

	pos.x = 31;
	pos.y = LYIMAGE()-292-1;

	for ( i=0 ; i<3 ; i++ )
	{
		if ( fj.vitesse == i )  DrawRadioButton(pos, 1);
		else                    DrawRadioButton(pos, 0);
		pos.y += 32;
	}
}


/* ---------- */
/* DrawScroll */
/* ---------- */

/*
	Dessine le mode de scroll en enfonant un bouton rond.
 */

void DrawScroll (void)
{
	short		i;
	Pt			pos;

	pos.x = 272;
	pos.y = LYIMAGE()-292-1;

	for ( i=0 ; i<2 ; i++ )
	{
		if ( fj.scroll == i )  DrawRadioButton(pos, 1);
		else                   DrawRadioButton(pos, 0);
		pos.y += 32;
	}
}


#ifdef VOLUME
/* ---------- */
/* DrawVolume */
/* ---------- */

/*
	Dessine le contenu d'un potentiomtre pour bruitage.
 */

void DrawVolume (short pot, short volume)
{
	Rectangle	rect;

	if ( pot == 0 )
	{
		rect.p1.x = 21+26;
		rect.p2.x = 21+26+4;
	}
	else
	{
		rect.p1.x = 21+40+16+10;
		rect.p2.x = 21+40+16+10+4;
	}

	rect.p1.y = LYIMAGE()-135-1+3;
	rect.p2.y = LYIMAGE()-135-1+3+((10-volume)*50/10);

	DrawFillRect(0, rect, MODELOAD, COLORBLANC);

	rect.p1.y = rect.p2.y;
	rect.p2.y = LYIMAGE()-135-1+3+50;

	DrawFillRect(0, rect, MODELOAD, COLORROUGE);
}
#endif

/* ------------ */
/* DrawBruitage */
/* ------------ */

/*
	Dessine le mode de bruitage en enfonant un bouton rond.
 */

void DrawBruitage ()
{
#ifdef VOLUME
	DrawVolume(0, fj.noisevolume);
	DrawVolume(1, fj.musicvolume);
#else
	Pt			pos;

	pos.x = 31;
	pos.y = LYIMAGE-140-1;

	DrawRadioButton(pos, fj.noisevolume==0 ? 0:1);
	pos.y += 32;
	DrawRadioButton(pos, fj.noisevolume==0 ? 1:0);
#endif
}


/* ----------- */
/* DrawTelecom */
/* ----------- */

/*
	Dessine le mode de tlcommande en enfonant un bouton rond.
 */

void DrawTelecom (void)
{
	short		i;
	Pt			pos;

	pos.x = 272;
	pos.y = LYIMAGE()-172-1;

	for ( i=0 ; i<2 ; i++ )
	{
		if ( fj.modetelecom == i )  DrawRadioButton(pos, 1);
		else                        DrawRadioButton(pos, 0);
		pos.y += 32;
	}
}


/* ----------- */
/* DrawCouleur */
/* ----------- */

/*
	Dessine le mode de couleur en enfonant un bouton rond.
 */

void DrawCouleur (void)
{
	short		i;
	Pt			pos;

	pos.x = 146;
	pos.y = LYIMAGE()-101-1;

	for ( i=0 ; i<5 ; i++ )
	{
		if ( descmonde.color == i )  DrawRadioButton(pos, 1);
		else                         DrawRadioButton(pos, 0);
		pos.x += 16*6;
	}
}


/* ---------- */
/* DrawArrows */
/* ---------- */

/*
	Affiche les 4 flches ou la tlcommande.
 */

void DrawArrows (char mode)
{
	Pixmap		pm;
	short		icon;
	Pt			src, dst, dim;
	Rectangle	rect;

	if ( g_typejeu == 0 || g_pause )
	{
		icon = ICO_ARROWS;
	}
	else
	{
		icon = ICO_TELECOM;
	}

	src.x = 0;
	src.y = 0;

	dst.x = 7;
	dst.y = LYIMAGE()-92-1;

	dim.x = 54;
	dim.y = 52;

	GetIcon(&pm, icon, 1);
	CopyPixel(&pm, src, 0, dst, dim, MODELOAD);		/* dessine flches ou tlcommande */

	if ( icon == ICO_TELECOM )
	{
		dim.x = 16;
		dim.y = 16;

		dst.x = 7+9;
		dst.y = LYIMAGE()-92-1+26;
		src.x = 0;
		src.y = 52;
		if ( mode == KEYGOFRONT )  src.x = 15;
		if ( mode == KEYGOBACK  )  src.x = 30;
		CopyPixel(&pm, src, 0, dst, dim, MODELOAD);	/* dessine la manette avant/arrire */

		dst.x = 7+29;
		dst.y = LYIMAGE()-92-1+26;
		src.x = 54;
		src.y = 0;
		if ( mode == KEYGOLEFT  )  src.y = 15;
		if ( mode == KEYGORIGHT )  src.y = 30;
		CopyPixel(&pm, src, 0, dst, dim, MODELOAD);	/* dessine la manette gauche/droite */
	}

	if ( g_typeedit )
	{
		rect.p1.x = 26;
		rect.p1.y = LYIMAGE()-1-28;
		rect.p2.x = 26+36;
		rect.p2.y = LYIMAGE()-1-28+18;
		DrawFillRect(0, rect, MODELOAD, COLORBLANC);	/* efface pause + disquette */
	}
}


/* --------- */
/* DrawPause */
/* --------- */

/*
	Affiche le bouton pause.
 */

void DrawPause (void)
{
	Pixmap		pm;
	Pt			src, dst, dim;

	if ( g_typeedit )  return;

	if ( g_pause )  src.x = 20;
	else          src.x = 0;
	src.y = 0;

	dst.x = 26;
	dst.y = LYIMAGE()-1-28;

	dim.x = 18;
	dim.y = 18;

	GetIcon(&pm, ICO_BUTTON_PAUSE+ICOMOFF, 1);
	CopyPixel(&pm, src, 0, dst, dim, MODEAND);

	GetIcon(&pm, ICO_BUTTON_PAUSE, 1);
	CopyPixel(&pm, src, 0, dst, dim, MODEOR);
}


/* ------------ */
/* DrawBigDigit */
/* ------------ */

/*
	Affiche un gros chiffre sur l'cran.
		pos		->	coin sup/gauche
		num		->	chiffre 0..9
 */

void DrawBigDigit (Pt pos, short num)
{
	Pixmap		pm;
	Pt			src, dim;

	GetIcon(&pm, ICO_CHAR_BIG, 1);

	src.x = (num%4)*20;
	src.y = (num/4)*26;

	dim.x = 20;
	dim.y = 26;

	CopyPixel(&pm, src, 0, pos, dim, MODELOAD);
}

/* ---------- */
/* DrawDigNum */
/* ---------- */

/*
	Affiche un gros nombre compris entre 0 et 99.
 */

void DrawBigNum (Pt pos, short num)
{
	DrawBigDigit(pos, 10);				/* efface les dizaines */
	pos.x += 20;
	DrawBigDigit(pos, 10);				/* efface les units */
	pos.x -= 20;

	if ( num > 99 )  num = 99;

	if ( num < 10 )
	{
		pos.x += 20/2;
		DrawBigDigit(pos, num);			/* affiche les units au milieu */
	}
	else
	{
		DrawBigDigit(pos, num/10);		/* affiche les dizaines */
		pos.x += 20;
		DrawBigDigit(pos, num%10);		/* affiche les units */
	}
}


/* ------------ */
/* DrawObjectif */
/* ------------ */

/*
	Affiche l'objectif du jeu.
 */

void DrawObjectif (void)
{
	Rectangle	rect;
	char		*ptext = descmonde.text;
#if FRENCH
	char		tomake[] = "Enigme \271 construire ...";
#endif
#if ENGLISH
	char		tomake[] = "Puzzle to build ...";
#endif
#if GERMAN
	char		tomake[] = "Ratsel zum bauen ...";
#endif

	switch ( phase )
	{
		case PHASE_RECOMMENCE:
			rect.p1.x = 130;
			rect.p1.y = LYIMAGE()-230;
			rect.p2.x = 130+419;
			rect.p2.y = LYIMAGE()-230+102;
			break;
		case PHASE_DEPLACE:
			rect.p1.x = 415;
			rect.p1.y = LYIMAGE()-160;
			rect.p2.x = 415+181;
			rect.p2.y = LYIMAGE()-160+63;
			break;
		default:
			rect.p1.x = 49;
			rect.p1.y = LYIMAGE()-254;
			rect.p2.x = 49+343;
			rect.p2.y = LYIMAGE()-254+130;
	}

	if ( g_construit && g_monde == maxmonde-1 )
	{
		if ( GetDemo() == 0 || *ptext == 0 )
		{
			ptext = tomake;
		}
	}

	DrawFillRect(0, rect, MODELOAD, COLORBLANC);			/* efface le rectangle */
	DrawParagraph(0, rect, ptext, TEXTSIZELIT, MODELOAD);	/* affiche la consigne */
}



/* ------------- */
/* RectStatusBar */
/* ------------- */

/*
	Retourne le rectangle  utiliser pour la barre d'avance.
 */

void RectStatusBar (Rectangle *prect)
{
	switch ( phase )
	{
		case PHASE_OBJECTIF:
		case PHASE_PRIVE:
			prect->p1.x = 488;
			prect->p1.y = LYIMAGE()-168;
			prect->p2.x = 488+113;
			prect->p2.y = LYIMAGE()-168+12;
			break;
		case PHASE_ATTENTE:
			prect->p1.x = 170;
			prect->p1.y = LYIMAGE()-113;
			prect->p2.x = 170+309;
			prect->p2.y = LYIMAGE()-113+12;
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

void DrawStatusBar (short avance, short max)
{
	short			pos;
	Rectangle		rect, part;
	Pt				pgra;
	char			lcolor, rcolor;
	char			chaine[6];
	ShowMode		mode;

	if ( max != 0 )  pos = (avance*100)/max;
	else             pos = 0;

	if ( pos < 0   )  pos = 0;
	if ( pos > 100 )  pos = 100;

	RectStatusBar(&rect);

	if ( IfColor() )
	{
		lcolor = COLORVERTC;
		rcolor = COLORROUGE;
		mode   = MODEOR;
	}
	else
	{
		lcolor = COLORNOIR;
		rcolor = COLORBLANC;
		mode   = MODEXOR;
	}

	part = rect;
	part.p2.x = part.p1.x + ((part.p2.x-part.p1.x)*pos)/100;
	DrawFillRect(0, part, MODELOAD, lcolor);	/* dessine le rectangle gauche */

	part.p1.x = part.p2.x;
	part.p2.x = rect.p2.x;
	DrawFillRect(0, part, MODELOAD, rcolor);	/* dessine le rectangle droite */

	pgra.x = (rect.p2.x+rect.p1.x)/2;
	pgra.y = rect.p1.y+TEXTSIZELIT+1;

	if ( pos < 10 )
	{
		pgra.x -= 7;
		chaine[0] = pos+'0';			/* units */
		chaine[1] = '%';
		chaine[2] = 0;
	}
	else
	{
		if ( pos < 100 )
		{
			pgra.x -= 10;
			chaine[0] = pos/10+'0';		/* dizaines */
			chaine[1] = pos%10+'0';		/* units */
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

	DrawText(0, pgra, chaine, TEXTSIZELIT, mode);
}


/* --------------- */
/* DetectStatusBar */
/* --------------- */

/*
	Dtecte le monde  atteindre selon la position de la souris.
 */

short DetectStatusBar (Pt pos, short max, Rectangle *prect)
{
	short		monde, progres;

	if ( max == 0 )  return 0;

	monde = ((pos.x-prect->p1.x)*max) / (prect->p2.x-prect->p1.x);
	if ( monde < 0     )  monde = 0;
	if ( monde > max-1 )  monde = max-1;

	progres = fj.progres[fj.joueur][fj.niveau[fj.joueur]];
	if ( !g_construit && monde > progres )  monde = progres;

	return monde;
}


/* ------------ */
/* DrawNumMonde */
/* ------------ */

/*
	Affiche le numro du monde actuel.
 */

void DrawNumMonde (void)
{
	Pixmap		pm;
	Pt			pos, src, dim;

	if ( g_construit && GetDemo() == 1 )  return;

	pos.x = 557;
	pos.y = LYIMAGE()-249;

	DrawBigNum(pos, g_monde+1);						/* dessine le numro du monde */

	src.x = 0;
	src.y = 0;

	dim.x = 58;
	dim.y = 50;

	pos.x = 478,
	pos.y = LYIMAGE()-283-1;

	if ( g_monde < maxmonde-1 &&
		 (g_construit || g_monde < fj.progres[fj.joueur][fj.niveau[fj.joueur]]) )
	{
		GetIcon(&pm, ICO_ARROWUP+1, 1);
	}
	else
	{
		GetIcon(&pm, ICO_ARROWUP, 1);
	}
	CopyPixel(&pm, src, 0, pos, dim, MODELOAD);		/* dessine la flche suprieure (+) */

	pos.y = LYIMAGE()-230-1;

	if ( g_monde > 0 )
	{
		GetIcon(&pm, ICO_ARROWDOWN+1, 1);
	}
	else
	{
		GetIcon(&pm, ICO_ARROWDOWN, 1);
	}
	CopyPixel(&pm, src, 0, pos, dim, MODELOAD);		/* dessine la flche infrieure (-) */

	if ( phase == PHASE_DEPLACE )  return;
	DrawStatusBar(g_monde, maxmonde-1);				/* dessine la barre d'avance */
}


/* ----------------- */
/* TrackingStatusBar */
/* ----------------- */

/*
	Choix d'un monde tant que la souris est presse.
 */

void TrackingStatusBar (Pt pos)
{
	Rectangle	rect;
	short		newmonde = g_monde;
	short		key;

	RectStatusBar(&rect);

	   g_monde = DetectStatusBar(pos, maxmonde, &rect);
	DrawNumMonde();							/* affiche le numro du monde */
	MondeRead(g_monde, banque);				/* lit le nouveau monde sur disque */
	DrawObjectif();							/* affiche l'objectif */

	while ( 1 )
	{
		key = GetEvent(&pos);
		if ( key == KEYCLICREL )  break;

		newmonde = DetectStatusBar(pos, maxmonde, &rect);
		if ( g_monde != newmonde )
		{
			         g_monde = newmonde;
			DrawNumMonde();					/* affiche le numro du monde */
			MondeRead(g_monde, banque);		/* lit le nouveau monde sur disque */
			DrawObjectif();					/* affiche l'objectif */
		}
	}
}



/* ------------ */
/* MondeDeplace */
/* ------------ */

/*
	Dplace un monde dans un autre.
	Retourne 0 si tout est ok.
 */

short MondeDeplace (short src, short dst)
{
	short		i;
	Monde		first, temp;

	if ( src == dst || src == dst-1 )  return 1;

	if ( src < dst )
	{
		FileRead(&first, src*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));

		for ( i=src+1 ; i<dst ; i++ )
		{
			DrawStatusBar(i-(src+1), dst-(src+1));
			FileRead(&temp, i*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
			FileWrite(&temp, (i-1)*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
		}
		DrawStatusBar(100, 100);

		FileWrite(&first, (dst-1)*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
	}

	if ( src > dst )
	{
		FileRead(&first, src*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));

		for ( i=src-1 ; i>=dst ; i-- )
		{
			DrawStatusBar(i-(src-1), dst-src);
			FileRead(&temp, i*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
			FileWrite(&temp, (i+1)*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
		}
		DrawStatusBar(100, 100);

		FileWrite(&first, dst*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
	}

	return 0;
}


/* ------------- */
/* MondeDuplique */
/* ------------- */

/*
	Duplique un monde juste aprs.
	Retourne 0 si tout est ok.
 */

short MondeDuplique (short m)
{
	short		max, i;
	Monde		temp;

	max = maxmonde;
	if ( g_construit )  max --;

	if ( m >= max )  return 1;

	for ( i=max-1 ; i>=m ; i-- )
	{
		DrawStatusBar(i-(max-1), m-max);
		FileRead(&temp, i*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
		FileWrite(&temp, (i+1)*sizeof(Monde), sizeof(Monde), BanqueToFile(banque));
	}
	DrawStatusBar(100, 100);

	   g_monde ++;
	maxmonde ++;
	return 0;
}


/* ------------ */
/* MondeDetruit */
/* ------------ */

/*
	Dtruit un monde.
	Retourne 0 si tout est ok.
 */

short MondeDetruit (short m)
{
	short		max, i, j;
	Monde		temp;

	max = maxmonde;
	if ( g_construit )  max --;

	if ( m >= max )  return 1;

	FileDelete('-');						/* dtruit le fichier temporaire (v.) */

	j = 0;
	for ( i=0 ; i<max ; i++ )
	{
		if ( i != m )
		{
			DrawStatusBar(j, max-1);
			if ( FileRead(&temp, i*sizeof(Monde), sizeof(Monde), BanqueToFile(banque)) )  goto error;
			if ( FileWrite(&temp, j*sizeof(Monde), sizeof(Monde), '-') )  goto error;
			j ++;
		}
	}
	DrawStatusBar(100, 100);

	FileDelete(BanqueToFile(banque));		/* dtruit l'ancien fichier dfinitif */
	FileRename('-', BanqueToFile(banque));	/* renomme le fichier temporaire -> dfinitif */

	maxmonde --;
	return 0;

	error:
	FileDelete('-');
	return 1;
}




/* ------------ */
/* PlayPartieLg */
/* ------------ */

/*
	Retourne la longueur ncessaire pour sauver les variables de la partie en cours.
 */

int PlayPartieLg (void)
{
	return
		sizeof(Monde) +
		sizeof(Partie);
}

/* --------------- */
/* PlayPartieWrite */
/* --------------- */

/*
	Sauve les variables de la partie en cours.
 */

short PlayPartieWrite (int pos, char file)
{
	short		err;
	Partie		partie;

	memset(&partie, 0, sizeof(Partie));

	partie.check = 123456;
	partie.monde = g_monde;
	partie.typejeu = g_typejeu;
	partie.banque = banque;

	err = FileWrite(&partie, pos, sizeof(Partie), file);
	if ( err )  return err;
	pos += sizeof(Partie);

	err = FileWrite(&descmonde, pos, sizeof(Monde), file);
	return err;
}

/* -------------- */
/* PlayPartieRead */
/* -------------- */

/*
	Lit les variables de la partie en cours.
 */

short PlayPartieRead (int pos, char file)
{
	short		err;
	Partie		partie;

	err = FileRead(&partie, pos, sizeof(Partie), file);
	if ( err )  return err;
	pos += sizeof(Partie);

	if ( partie.check != 123456 )  return 1;

	   g_monde = partie.monde;
	   g_typejeu = partie.typejeu;
	banque = partie.banque;

	if ( banque < 'I' )
	{
		fj.niveau[fj.joueur] = banque-'A';
	}
	else
	{
		fj.niveau[fj.joueur] = 8;
	}
	MondeMax(banque);

	err = FileRead(&descmonde, pos, sizeof(Monde), file);
	return err;
}


/* --------------- */
/* PartieCheckFile */
/* --------------- */

/*
	Vérifie si le fichier de sauvegarde de la partie est correct,
	c'est-à-dire s'il correspond à cette version de soft !
 */

short PartieCheckFile ()
{
	short		err;
	Header		header;

	err = FileRead(&header, 0, sizeof(Header), GetDemo()?'w':'y');
	if ( err == 0 )
	{
		if ( header.ident == 1 &&
			 header.lg[0] == PlayPartieLg() &&
			 header.lg[1] == MovePartieLg() &&
			 header.lg[2] == DecorPartieLg() &&
			 header.lg[3] == PalPartieLg() &&
			 header.lg[4] == MachinePartieLg() &&
			 header.lg[5] == 0 )  return 0;		/* fichier ok */
	}
	FileDelete(GetDemo()?'w':'y');

	memset(&header, 0, sizeof(Header));

	header.ident = 1;
	header.lg[0] = PlayPartieLg();
	header.lg[1] = MovePartieLg();
	header.lg[2] = DecorPartieLg();
	header.lg[3] = PalPartieLg();
	header.lg[4] = MachinePartieLg();

	FileWrite(&header, 0, sizeof(Header), GetDemo()?'w':'y');

	return 1;		/* le fichier n'était pas correct */
}


#define MAXPARTIE	4					/* nb max de parties sauvables par joueur */

/* ----------- */
/* PartieSauve */
/* ----------- */

/*
	Sauve la partie en cours.
 */

short PartieSauve (short rang)
{
	int		pos;
	short		err;

	PartieCheckFile();					/* adapte le fichier si ncessaire */

	pos = sizeof(Header) +
		  (PlayPartieLg()+
		   MovePartieLg()+
		   DecorPartieLg()+
		   PalPartieLg()+
		   MachinePartieLg())*
		  (fj.joueur*MAXPARTIE+rang);

	err = PlayPartieWrite(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += PlayPartieLg();

	err = MovePartieWrite(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += MovePartieLg();

	err = DecorPartieWrite(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += DecorPartieLg();

	err = PalPartieWrite(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += PalPartieLg();

	err = MachinePartieWrite(pos, GetDemo()?'W':'Y');
	return err;
}

/* ----------- */
/* PartiePrend */
/* ----------- */

/*
	Reprend la partie en cours.
 */

short PartiePrend (short rang)
{
	int		pos;
	short		err;

	err = PartieCheckFile();			/* fichier ok ? */
	if ( err )  return err;

	pos = sizeof(Header) +
		  (PlayPartieLg()+
		   MovePartieLg()+
		   DecorPartieLg()+
		   PalPartieLg()+
		   MachinePartieLg())*
		  (fj.joueur*MAXPARTIE+rang);

	err = PlayPartieRead(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += PlayPartieLg();

	err = MovePartieRead(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += MovePartieLg();

	err = DecorPartieRead(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += DecorPartieLg();

	err = PalPartieRead(pos, GetDemo()?'W':'Y');
	if ( err )  return err;
	pos += PalPartieLg();

	err = MachinePartieRead(pos, GetDemo()?'W':'Y');
	if ( err )  return err;

	IconDrawOpen();
	MoveRedraw();						/* redessine sans changement */
	IconDrawClose(1);

	ChangeCouleur();					/* change les couleurs */
	MusicStart(4+g_monde);

	return 0;
}



/* -------------- */
/* PartieDrawIcon */
/* -------------- */

/*
	Dessine l'icne prend ou sauve au milieu de la fentre.
 */

void PartieDrawIcon (short key)
{
	Pixmap		pmicon;						/* pixmap de l'icne  dessiner */
	Pt			pos, p, zero = {0, 0}, dim = {LYICO, LXICO};

	pos.x = POSXDRAW+20;
	pos.y = POSYDRAW+DIMYDRAW-LYICO-20;

	if ( key == KEYLOAD || key == -KEYLOAD )
	{
		pos.x += LXICO+20;
	}

	if ( key ==  KEYSAVE )  GetIcon(&pmicon, ICO_SAUVE+ICOMOFF, 1);
	if ( key ==  KEYLOAD )  GetIcon(&pmicon, ICO_PREND+ICOMOFF, 1);
	if ( key == -KEYSAVE )  GetIcon(&pmicon, ICO_ATTENTE+0+ICOMOFF, 1);
	if ( key == -KEYLOAD )  GetIcon(&pmicon, ICO_ATTENTE+1+ICOMOFF, 1);

	CopyPixel								/* masque le fond */
	(
		&pmicon, zero,
		0, pos,
		dim, MODEAND
	);

	if ( key ==  KEYSAVE )  GetIcon(&pmicon, ICO_SAUVE, 1);
	if ( key ==  KEYLOAD )  GetIcon(&pmicon, ICO_PREND, 1);
	if ( key == -KEYSAVE )  GetIcon(&pmicon, ICO_ATTENTE+0, 1);
	if ( key == -KEYLOAD )  GetIcon(&pmicon, ICO_ATTENTE+1, 1);

	CopyPixel								/* dessine la chair */
	(
		&pmicon, zero,
		0, pos,
		dim, MODEOR
	);
}

/* ----------------- */
/* PartieClicToEvent */
/* ----------------- */

/*
	Conversion d'un clic  une position donne en un vnement clavier.
 */

short PartieClicToEvent (Pt pos)
{
	short		*ptable;

	static short table[] =
	{
		2,77,19,19,		KEYUNDO,		/* case de fermeture */
		7,55,31,23,		KEYF1,			/* partie #1 */
		41,55,31,23,	KEYF2,			/* partie #2 */
		7,29,31,23,		KEYF3,			/* partie #3 */
		41,29,31,23,	KEYF4,			/* partie #4 */

		102,77,19,19,	KEYUNDO,		/* case de fermeture */
		107,55,31,23,	'1',			/* partie #1 */
		141,55,31,23,	'2',			/* partie #2 */
		107,29,31,23,	'3',			/* partie #3 */
		141,29,31,23,	'4',			/* partie #4 */

		-1
	};

	pos.x -= POSXDRAW+20;
	pos.y -= POSYDRAW+DIMYDRAW-LYICO-20;

	ptable = table;
	while ( ptable[0] != -1 )
	{
		if ( pos.x >= ptable[0] &&
			 pos.x <= ptable[0] + ptable[2] &&
			 pos.y >= LYICO-1- ptable[1] &&
			 pos.y <= LYICO-1- ptable[1] + ptable[3] )
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

void PartieDisque (short mode)
{
	short		key;
	Pt			pos;

	if ( mode != KEYLOAD )  PartieDrawIcon(KEYSAVE);	/* dessine l'icne */
	if ( mode != KEYSAVE )  PartieDrawIcon(KEYLOAD);	/* dessine l'icne */

	while (1)
	{
		key = GetEvent(&pos);
		if ( key == KEYCLIC || key == KEYCLICR )
		{
			key = PartieClicToEvent(pos);
		}

		if ( key == KEYUNDO || key == KEYQUIT || key == KEYHOME ||
			 key == KEYF1 || key == KEYF2 || key == KEYF3 || key == KEYF4 ||
			 key == '1'   || key == '2'   || key == '3'   || key == '4'   )  break;
	}
	PlayEvSound(SOUND_CLIC);

	if ( mode != KEYLOAD && key <= KEYF1 && key >= KEYF4 )
	{
		PartieDrawIcon(-KEYSAVE);					/* dessine l'icne d'attente */
		PartieSauve(-key+KEYF1);					/* sauve la partie */
	}

	if ( mode == KEYSAVE && key >= '1' && key <= '4' )
	{
		PartieDrawIcon(-KEYSAVE);					/* dessine l'icne d'attente */
		PartieSauve(key-'1');						/* sauve la partie */
	}

	if ( mode != KEYSAVE && key >= '1' && key <= '4' )
	{
		PartieDrawIcon(-KEYLOAD);					/* dessine l'icne d'attente */
		PartiePrend(key-'1');						/* reprend une partie */
	}

	IconDrawAll();									/* faudra tout redessiner */
}


/* ------------ */
/* StopDrawIcon */
/* ------------ */

/*
	Dessine les icnes stoppe oui/non au milieu de la fentre.
 */

void StopDrawIcon (void)
{
	Pixmap		pmicon;						/* pixmap de l'icne  dessiner */
	Pt			pos, p = {0, 0}, dim = {LYICO, LXICO};

	pos.x = POSXDRAW+20;
	pos.y = POSYDRAW+DIMYDRAW-LYICO-20;

	GetIcon(&pmicon, ICO_STOPOUI+ICOMOFF, 1);
	CopyPixel								/* masque le fond */
	(
		&pmicon, p,
		0, pos,
		dim, MODEAND
	);

	GetIcon(&pmicon, ICO_STOPOUI, 1);
	CopyPixel								/* dessine la chair */
	(
		&pmicon, p,
		0, pos,
		dim, MODEOR
	);

	pos.x += LXICO+20;

	GetIcon(&pmicon, ICO_STOPNON+ICOMOFF, 1);
	CopyPixel								/* masque le fond */
	(
		&pmicon, p,
		0, pos,
		dim, MODEAND
	);

	GetIcon(&pmicon, ICO_STOPNON, 1);
	CopyPixel								/* dessine la chair */
	(
		&pmicon, p,
		0, pos,
		dim, MODEOR
	);
}

/* --------------- */
/* StopClicToEvent */
/* --------------- */

/*
	Conversion d'un clic  une position donne en un vnement clavier.
 */

short StopClicToEvent (Pt pos)
{
	pos.x -= POSXDRAW+20;
	pos.y -= POSYDRAW+DIMYDRAW-LYICO-20;

	if ( pos.y < 0 ||
		 pos.y > LYICO )  return KEYUNDO;

	if ( pos.x >= 0 &&
		 pos.x <= LXICO )  return KEYHOME;

	return KEYUNDO;
}

/* ---------- */
/* StopPartie */
/* ---------- */

/*
	Demande s'il faut stopper la partie en cours.
 */

short StopPartie (void)
{
	Pixmap		pmsave = {0,0,0,0,0,0,0};
	short		key;
	Pt			pos;
	Pt			spos, sdim;
	Pt			p;

	PlayEvSound(SOUND_CLIC);

	spos.x = POSXDRAW+20;
	spos.y = POSYDRAW+DIMYDRAW-LYICO-20;
	sdim.x = LXICO+20+LXICO;
	sdim.y = LYICO;

	if ( GetPixmap(&pmsave, sdim, 0, 2) != 0 )  return KEYHOME;

        p.y = 0;
        p.x = 0;
	CopyPixel(0, spos, &pmsave, p, sdim, MODELOAD);	/* sauve l'cran */

	StopDrawIcon();									/* dessine les icnes */

	while (1)
	{
		key = GetEvent(&pos);
		if ( key == KEYCLIC || key == KEYCLICR )
		{
			key = StopClicToEvent(pos);
		}

		if ( key != 0 && key != KEYCLICREL )  break;
	}
	PlayEvSound(SOUND_CLIC);

        p.y = 0;
        p.x = 0;
	CopyPixel(&pmsave, p, 0, spos, sdim, MODELOAD);	/* restitue l'cran */
	GivePixmap(&pmsave);

	return key;
}




/* -------------- */
/* JoueurEditOpen */
/* -------------- */

/*
	Prpare l'dition du nom des joueurs.
 */

void JoueurEditOpen (void)
{
	Rectangle	rect;

	rect.p1.x = 299;
	rect.p1.y = LYIMAGE()-297+fj.joueur*40;
	rect.p2.x = 299+180;
	rect.p2.y = rect.p1.y+23;
	EditOpen(fj.nom[fj.joueur], MAXNOMJ, rect);

	   g_typetext = 1;
}


/* --------------- */
/* JoueurEditClose */
/* --------------- */

/*
	Fin de l'dition du nom des joueurs.
 */

void JoueurEditClose (void)
{
	EditClose();
	   g_typetext = 0;
}


/* --------- */
/* DrawIdent */
/* --------- */

/*
	Affiche tous les noms des joueurs.
 */

void DrawIdent (void)
{
	short	joueur;
	char	chaine[20];
	char	*p;
	Pt		pos;

	joueur = fj.joueur;
	for ( fj.joueur=0 ; fj.joueur<MAXJOUEUR ; fj.joueur++ )
	{
		JoueurEditOpen();				/* affiche le nom du joueur */
		JoueurEditClose();
	}
	fj.joueur = joueur;

	pos.x = 500;
	pos.y = LYIMAGE()-286;
	for ( joueur=0 ; joueur<MAXJOUEUR ; joueur++ )
	{
		if ( fj.nom[joueur][0] != 0 )
		{
			p = chaine;
			*p++ = 'A';
			*p++ = ':';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][0]+1, 0);
			*p++ = ' ';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][2]+1, 0);
			*p++ = ' ';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][4]+1, 0);
			*p++ = ' ';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][6]+1, 0);
			DrawText(0, pos, chaine, TEXTSIZELIT, MODEOR);	/* affiche la progression */
		}
		pos.y += 15;

		if ( fj.nom[joueur][0] != 0 )
		{
			p = chaine;
			*p++ = 'T';
			*p++ = ':';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][1]+1, 0);
			*p++ = ' ';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][3]+1, 0);
			*p++ = ' ';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][5]+1, 0);
			*p++ = ' ';
			*p++ = ' ';
			PutNum(&p, fj.progres[joueur][7]+1, 0);
			DrawText(0, pos, chaine, TEXTSIZELIT, MODEOR);	/* affiche la progression */
		}
		pos.y += 40-15;
	}
}



/* ------------- */
/* PhaseEditOpen */
/* ------------- */

/*
	Prpare le monde dans descmonde pour pouvoir l'diter.
 */

void PhaseEditOpen (void)
{
	MondeRead(g_monde, banque);		/* lit le monde  diter sur disque */
	savemonde = descmonde;			/* sauve le monde (palette, etc.) */
	MondeEdit();					/* modifie le monde pour pouvoir l'diter */
}


/* -------------- */
/* PhaseEditClose */
/* -------------- */

/*
	Fin de l'dition du monde dans descmonde.
 */

void PhaseEditClose (void)
{
	short		i;

	for ( i=0 ; i<MAXPALETTE ; i++ )
	{
		descmonde.palette[i] = savemonde.palette[i];	/* remet la palette initiale */
	}

	MondeWrite(g_monde, banque);

	   g_typeedit = 0;		/* fin de l'dition */
}



/* ----------- */
/* ChangePhase */
/* ----------- */

/*
	Change la phase du jeu.
	Retourne !=0 en cas d'erreur.
 */

short ChangePhase (Phase newphase)
{
	short		err, type;
	Rectangle	rect;

	/*	Ferme la phase de jeu en cours. */

	MusicStop();
	ClrEvents();

	switch ( phase )
	{
		case PHASE_IDENT:
			JoueurEditClose();			/* fin de l'dition du nom */
			BlackScreen();
			JoueurWrite();				/* crit le fichier des joueurs */
			break;

		case PHASE_REGLAGE:
			BlackScreen();
			JoueurWrite();				/* crit le fichier des joueurs */
			break;

		case PHASE_PARAM:
			PaletteEditClose(descmonde.palette);
			EditClose();
			BlackScreen();
			MondeWrite(g_monde, banque);
			     g_typetext = 0;
			break;

		case PHASE_DEPLACE:
			     g_monde = mondeinit;
			break;

		case PHASE_PLAY:
			if ( g_typeedit )
			{
				BlackScreen();
				PhaseEditClose();
			}
			DecorClose();				/* fermeture des dcors */
			MoveClose();				/* fermeture des objets en mouvement */
			IconClose();				/* fermeture des icnes */
			break;

		case PHASE_FINI0:
			fj.niveau[fj.joueur] = 1;	/* 1A -> 1T */
			break;

		case PHASE_FINI1:
			fj.niveau[fj.joueur] = 2;	/* 1T -> 2A */
			break;

		case PHASE_FINI2:
			fj.niveau[fj.joueur] = 3;	/* 2A -> 2T */
			break;

		case PHASE_FINI3:
			fj.niveau[fj.joueur] = 4;	/* 2T -> 3A */
			break;

		case PHASE_FINI4:
			fj.niveau[fj.joueur] = 5;	/* 3A -> 3T */
			break;

		case PHASE_FINI5:
			fj.niveau[fj.joueur] = 6;	/* 3T -> 4A */
			break;

		case PHASE_FINI6:
			fj.niveau[fj.joueur] = 7;	/* 4A -> 4T */
			break;

		case PHASE_FINI7:
			fj.niveau[fj.joueur] = 8;	/* 4T -> 5 */
			break;

		case PHASE_FINI8:
			     g_monde = maxmonde-1;			/*  construire */
			break;

		default:
			break;
	}

	/*	Change la phase de jeu. */

        ignoreKeyClicUp = SDL_TRUE;
	phase = newphase;					/* change la phase */
	ShowImage();						/* affiche l'image de base */

	/*	Ouvre la nouvelle phase de jeu. */

	musique = 0;						/* pas de musique de fond */

	switch ( phase )
	{
		case PHASE_GENERIC:
			PlayNoiseVolume(10-3);
			PlayMusicVolume(10-3);
			MusicStart(0);
			musique = 1;
			lastaccord = -1;
			break;

		case PHASE_SUIVANT:
			MusicStart(2);
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
			MusicStart(1);
			musique = 2;
			lastaccord = -1;
			break;

		case PHASE_IDENT:
			JoueurRead();				/* lit le fichier des joueurs sur disque */
			DrawJoueur();				/* affiche le joueur */
			DrawIdent();				/* affiche tous les noms */
			JoueurEditOpen();			/* prpare l'dition du nom */
			break;

		case PHASE_REGLAGE:
			DrawVitesse();				/* affiche la vitesse */
			DrawScroll();				/* affiche le scroll */
			DrawBruitage();				/* affiche le mode de bruitages */
			DrawTelecom();				/* affiche le mode de tlcommande */
			MusicStart(3);
			break;

		case PHASE_PARAM:
			MondeRead(g_monde, banque);	/* lit le monde  modifier sur disque */
			PaletteEditOpen(descmonde.palette);
			rect.p1.x = 218;
			rect.p1.y = LYIMAGE()-47;
			rect.p2.x = 218+180;
			rect.p2.y = LYIMAGE()-47+23;
			EditOpen(descmonde.text, MAXTEXT, rect);
			     g_typetext = 1;
			DrawCouleur();				/* affiche le mode de couleur */
			break;

		case PHASE_PRIVE:
			MondeRead(g_monde, banque);	/* lit le nouveau monde sur disque */
			DrawNumMonde();				/* affiche le numro du monde */
			DrawObjectif();				/* affiche l'objectif */
			retry = 0;
			break;

		case PHASE_DEPLACE:
			mondeinit = g_monde;
			DrawNumMonde();				/* affiche le numro du monde */
			DrawObjectif();				/* affiche l'objectif */
			break;

		case PHASE_OBJECTIF:
			MondeRead(g_monde, banque);	/* lit le nouveau monde sur disque */
			DrawNumMonde();				/* affiche le numro du monde */
			DrawObjectif();				/* affiche l'objectif */
			retry = 0;
			break;

		case PHASE_RECOMMENCE:
			PlayEvSound(SOUND_NON);
			MondeRead(g_monde, banque);	/* relit le monde sur disque */
			DrawObjectif();				/* affiche l'objectif */
			retry ++;
			break;

		case PHASE_PLAY:
			ChangeCouleur();			/* change les couleurs */

			if ( g_typeedit )  PhaseEditOpen();

			err = IconOpen();			/* ouverture des icnes */
			if ( err )  FatalBreak(err);

			err = MoveOpen();			/* ouverture des objets en mouvement */
			if ( err )  FatalBreak(err);

			err = DecorOpen();			/* ouverture des dcors */
			if ( err )  FatalBreak(err);

			IconDrawFlush();			/* vide tous les buffers internes */
			DecorNewMonde(&descmonde);	/* initialise le monde */

			type = 0;
			if ( g_typejeu == 0 || g_typeedit )  type = 1;
			PaletteNew(descmonde.palette, type);

			DecorMake(1);				/* fabrique le dcor */
			IconDrawAll();				/* redessine toute la fentre */

			     g_pause = 0;
			DrawArrows(0);				/* dessine les flches */
			DrawPause();				/* dessine le bouton pause */
			if ( g_typeedit == 0 )  MusicStart(4+g_monde);
			break;

		default:
			break;
	}

	passindex = 0;

	ClrEvents();
	return 0;							/* nouvelle phase ok */
}





/* Tables dcrivants les zones cliquables dans les images */
/* ------------------------------------------------------ */

static short timage21[] =				/* initial */
{
	331,110,56,70,		0,			ACTION_NIVEAU8,
	-1,-1,-1,-1,		'5',		ACTION_NIVEAUK5,

	123,279,40,46,		0,			ACTION_NIVEAU0,
	160,302,41,53,		0,			ACTION_NIVEAU1,
	298,284,50,59,		0,			ACTION_NIVEAU2,
	348,280,50,66,		0,			ACTION_NIVEAU3,
	150,150,48,65,		0,			ACTION_NIVEAU4,
	193,180,51,77,		0,			ACTION_NIVEAU5,
	376,174,58,65,		0,			ACTION_NIVEAU6,
	436,165,56,67,		0,			ACTION_NIVEAU7,
	-1,-1,-1,-1,		'1',		ACTION_NIVEAUK1,
	-1,-1,-1,-1,		'2',		ACTION_NIVEAUK2,
	-1,-1,-1,-1,		'3',		ACTION_NIVEAUK3,
	-1,-1,-1,-1,		'4',		ACTION_NIVEAUK4,
	-1,-1,-1,-1,		KEYRETURN,	ACTION_NIVEAUGO,
	543,150,71,78,		KEYDEF,		ACTION_REGLAGE,
	482,71,68,60,		0,			ACTION_AIDE,
	554,82,67,65,		0,			ACTION_IDENT,
	526,287,67,82,		KEYUNDO,	ACTION_QUITTE,
	0
};

static short timage22[] =				/* objectif */
{
	22,73,208,56,		KEYRETURN,	ACTION_JOUE,
	40,272,361,161,		KEYRETURN,	ACTION_JOUE,
	425,73,190,56,		KEYUNDO,	ACTION_ANNULE,
	473,284,63,53,		KEYUP,		ACTION_MONDESUIV,
	473,233,63,53,		KEYDOWN,	ACTION_MONDEPREC,
	472,174,136,31,		0,			ACTION_MONDEBAR,
	0
};

static short timage23[] =				/* recommence */
{
	21,72,292,56,		KEYRETURN,	ACTION_JOUE,
	121,240,435,120,	KEYRETURN,	ACTION_JOUE,
	400,72,209,56,		KEYUNDO,	ACTION_STOPPEKO,
	0
};

static short timage24[] =				/* suivant */
{
	27,76,207,56,		KEYRETURN,	ACTION_SUIVANT,
	419,76,185,56,		KEYUNDO,	ACTION_STOPPEOK,
	0
};

static short timage25[] =				/* priv */
{
	22,73,208,56,		KEYRETURN,	ACTION_JOUE,
	40,272,361,161,		KEYRETURN,	ACTION_JOUE,
	248,72,132,26,		'E',		ACTION_EDIT,
	248,45,132,26,		'T',		ACTION_PARAM,
	425,73,190,56,		KEYUNDO,	ACTION_DEBUT,
	473,284,63,53,		KEYUP,		ACTION_MONDESUIV,
	473,233,63,53,		KEYDOWN,	ACTION_MONDEPREC,
	472,174,136,31,		0,			ACTION_MONDEBAR,
	470,142,138,31,		'M',		ACTION_OPER,
	0
};

static short timage26[] =				/* paramtres */
{
	146,101,88,32,		0,			ACTION_COULEUR0,
	242,101,88,32,		0,			ACTION_COULEUR1,
	338,101,88,32,		0,			ACTION_COULEUR2,
	434,101,88,32,		0,			ACTION_COULEUR3,
	530,101,88,32,		0,			ACTION_COULEUR4,
	414,64,210,56,		KEYUNDO,	ACTION_OBJECTIF,
	0
};

static short timage27[] =				/* dplace */
{
	33,73,207,56,		KEYRETURN,	ACTION_ORDRE,
	395,73,207,56,		KEYUNDO,	ACTION_OBJECTIF,
	473,284,63,53,		KEYUP,		ACTION_MONDESUIV,
	473,233,63,53,		KEYDOWN,	ACTION_MONDEPREC,
	0
};

static short timage28[] =				/* aide 2.1 */
{
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE22,
	0
};

static short timage30[] =				/* opration */
{
	41,294,237,56,		'R',		ACTION_DETRUIT,
	41,227,237,56,		'M',		ACTION_DEPLACE,
	41,160,237,56,		'C',		ACTION_DUPLIQUE,
	417,81,190,56,		KEYUNDO,	ACTION_OBJECTIF,
	0
};

static short timage31[] =				/* identification */
{
	230,299,250,34,		KEYF1,		ACTION_JOUEUR0,
	230,259,250,34,		KEYF2,		ACTION_JOUEUR1,
	230,219,250,34,		KEYF3,		ACTION_JOUEUR2,
	230,179,250,34,		KEYF4,		ACTION_JOUEUR3,
	20,73,195,57,		KEYRETURN,	ACTION_DEBUT,
	222,73,183,57,		'H',		ACTION_AIDE,
	461,73,160,57,		KEYUNDO,	ACTION_QUITTE,
	0
};

static short timage32[] =				/* aide 2.2 */
{
	21,71,77,55,		0,			ACTION_AIDE21,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE23,
	0
};

static short timage33[] =				/* fini niveau */
{
	22,72,139,56,		KEYRETURN,	ACTION_FINI,
	0
};

static short timage34[] =				/* aide 2.3 */
{
	21,71,77,55,		0,			ACTION_AIDE22,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE24,
	0
};

static short timage35[] =				/* rglages */
{
	31,292,160,32,		'S',		ACTION_VITESSE0,
	31,260,160,32,		'N',		ACTION_VITESSE1,
	31,228,160,32,		'Q',		ACTION_VITESSE2,
	272,292,270,32,		0,			ACTION_SCROLL0,
	272,260,270,32,		0,			ACTION_SCROLL1,
#ifdef VOLUME
	19,137,21,21,		0,			ACTION_NOISEVOLP,
	19,99,21,21,		0,			ACTION_NOISEVOLM,
	97,137,21,21,		0,			ACTION_MUSICVOLP,
	97,99,21,21,		0,			ACTION_MUSICVOLM,
#else
	31,140,85,32,		'1',		ACTION_BRUIT0,
	31,108,85,32,		'0',		ACTION_BRUIT1,
#endif
	272,172,270,32,		0,			ACTION_TELECOM0,
	272,140,270,32,		0,			ACTION_TELECOM1,
	40,72,260,56,		KEYRETURN,	ACTION_DEBUT,
	311,72,384,56,		KEYDEF,		ACTION_IDENT,
	0
};

static short timage36[] =				/* gnrique */
{
	0,339,640,340,		0,			ACTION_IDENT,
	0
};

static short timage40[] =				/* aide 3.1 */
{
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE32,
	0
};

static short timage41[] =				/* aide 3.2 */
{
	21,71,77,55,		0,			ACTION_AIDE31,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE33,
	0
};

static short timage42[] =				/* aide 3.3 */
{
	21,71,77,55,		0,			ACTION_AIDE32,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE34,
	0
};

static short timage43[] =				/* aide 3.4 */
{
	21,71,77,55,		0,			ACTION_AIDE33,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE35,
	0
};

static short timage44[] =				/* aide 3.5 */
{
	21,71,77,55,		0,			ACTION_AIDE34,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE36,
	0
};

static short timage45[] =				/* aide 3.6 */
{
	21,71,77,55,		0,			ACTION_AIDE35,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	0
};

static short timage46[] =				/* aide */
{
	15,262,184,53,		0,			ACTION_AIDE11,
	15,200,184,53,		0,			ACTION_AIDE21,
	15,135,184,53,		0,			ACTION_AIDE31,
	15,72,184,53,		0,			ACTION_AIDE41,
	446,72,173,53,		0,			ACTION_DEBUT,
	0
};

static short timage47[] =				/* aide 4.1 */
{
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE42,
	0
};

static short timage48[] =				/* aide 4.2 */
{
	21,71,77,55,		0,			ACTION_AIDE41,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	0
};

static short timage49[] =				/* aide 2.4 */
{
	21,71,77,55,		0,			ACTION_AIDE23,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	0
};

static short timage50[] =				/* aide 1.1 */
{
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE12,
	0
};

static short timage51[] =				/* aide 1.2 */
{
	21,71,77,55,		0,			ACTION_AIDE11,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	196,71,77,55,		KEYRETURN,	ACTION_AIDE13,
	0
};

static short timage52[] =				/* aide 1.3 */
{
	21,71,77,55,		0,			ACTION_AIDE12,
	109,71,77,55,		KEYUNDO,	ACTION_AIDE,
	0
};


/* --------- */
/* GetTimage */
/* --------- */

/*
	Retourne le pointeur  la table timage??[].
 */

short* GetTimage (void)
{
	short	*pt;

	switch ( ConvPhaseToNumImage(phase) )
	{
		case 21:
			pt = timage21;
#ifdef DEMONC
			if ( GetDemo() == 1 )  pt += 6*2;
#endif
			return pt;

		case 22:  return timage22;
		case 23:  return timage23;
		case 24:  return timage24;
		case 25:  return timage25;
		case 26:  return timage26;
		case 27:  return timage27;
		case 28:  return timage28;
		case 30:  return timage30;
		case 31:  return timage31;
		case 32:  return timage32;
		case 33:  return timage33;
		case 34:  return timage34;
		case 35:  return timage35;
		case 36:  return timage36;
		case 40:  return timage40;
		case 41:  return timage41;
		case 42:  return timage42;
		case 43:  return timage43;
		case 44:  return timage44;
		case 45:  return timage45;
		case 46:  return timage46;
		case 47:  return timage47;
		case 48:  return timage48;
		case 49:  return timage49;
		case 50:  return timage50;
		case 51:  return timage51;
		case 52:  return timage52;
	}
	return 0;
}

/* ------------ */
/* ClicToAction */
/* ------------ */

/*
	Retourne l'action correspondant  la position d'un clic souris.
	Retourne -1 en cas d'erreur.
 */

PhAction ClicToAction (Pt pos)
{
	short	*pt;

	pt = GetTimage();
	if ( pt == 0 )  return -1;

	while ( pt[0] != 0 )
	{
		if ( pos.x >= pt[0] &&
			 pos.x <= pt[0]+pt[2] &&
			 pos.y >= LYIMAGE()-pt[1] &&
			 pos.y <= LYIMAGE()-pt[1]+pt[3] )
		{
			return pt[5];			/* retourne l'action clique */
		}
		pt += 6;
	}

	return -1;
}


/* ------------- */
/* EventToAction */
/* ------------- */

/*
	Conversion d'un vnement en une action, selon la phase en cours.
	Retourne -1 en cas d'erreur.
 */

PhAction EventToAction (char event)
{
	short	*pt;

	if ( event == 0 || event == KEYCLICREL )  return -1;

	pt = GetTimage();
	if ( pt == 0 )  return -1;

	if ( pt[6] == 0 )				/* une seule action ? */
	{
		return pt[5];				/* retourne la seule action possible */
	}

	while ( pt[0] != 0 )
	{
		if ( event == pt[4] )
		{
			return pt[5];			/* retourne l'action clique */
		}
		pt += 6;
	}

	return -1;
}



/* Tables dcrivants les animations dans les images */
/* ------------------------------------------------ */

static short tanim21[] =				/* initial */
{
	ACTION_NIVEAU8,		320,122,	DELNORM,6,	128+88,128+89,128+90,128+89,128+90,128+89,

	ACTION_NIVEAU1,		141,327,	DELNORM,12,	128+97,128+96,128+97,128+96,128+97,128+96,
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

static short tanim25[] =				/* priv */
{
	ACTION_JOUE,		89,137,		DELNORM,4,	2,1,3,1,
	ACTION_DEBUT,		482,136,	DELSLOW,15,	68,68,68,64,64,64,33,33,33,64,64,64,68,68,68,
	-1
};

static short tanim26[] =				/* paramtres */
{
	-1
};

static short tanim27[] =				/* dplace */
{
	-1
};

static short tanim30[] =				/* opration */
{
	ACTION_DETRUIT,		118,98,		DELNORM,8,	58,59,58,58,59,58,59,18,
	ACTION_DEPLACE,		118,98,		DELNORM,8,	58,59,58,58,59,58,59,18,
	ACTION_DUPLIQUE,	118,98,		DELNORM,8,	58,59,58,58,59,58,59,18,
	ACTION_OBJECTIF,	475,142,	DELSLOW,15,	68,68,68,64,64,64,33,33,33,64,64,64,68,68,68,
	-1
};

static short tanim31[] =				/* identification */
{
	ACTION_DEBUT,		79,136,		DELNORM,4,	2,1,3,1,
	ACTION_AIDE,		281,139,	DELNORM,8,	105,106,106,107,107,104,104,105,
	ACTION_QUITTE,		503,138,	DELNORM,4,	50,33,50,36,
	-1
};

static short tanim33[] =				/* fini niveau */
{
	ACTION_FINI,		53,138,		DELNORM,4,	46,45,18,45,
	-1
};

static short tanim35[] =				/* rglages */
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


/* ------------ */
/* AnimGetTable */
/* ------------ */

/*
	Cherche une table selon la phase de jeu.
 */

short* AnimGetTable (void)
{
	short	*pt;

	switch ( ConvPhaseToNumImage(phase) )
	{
		case 21:
			pt = tanim21;
#ifdef DEMONC
			if ( GetDemo() == 1 )  pt += 5+pt[4];
#endif
			return pt;

		case 22:  return tanim22;
		case 23:  return tanim23;
		case 24:  return tanim24;
		case 25:  return tanim25;
		case 26:  return tanim26;
		case 27:  return tanim27;
		case 30:  return tanim30;
		case 31:  return tanim31;
		case 33:  return tanim33;
		case 35:  return tanim35;
	}
	return 0;
}

/* ---------- */
/* AnimSearch */
/* ---------- */

/*
	Cherche une animation dans la table.
 */

short* AnimSearch (PhAction ac)
{
	short	*pt = AnimGetTable();

	if ( pt == 0 )  return 0;

	while ( pt[0] != -1 )
	{
		if ( ac == pt[0] )  return pt;
		pt += 5+pt[4];
	}
	return 0;
}


/* --------------- */
/* AnimIconAddBack */
/* --------------- */

/*
	Ajoute dans pmtemp toutes les icnes places derrire ou devant.
 */

void AnimIconAddBack (Pt pos, char bFront)
{
	short	*pt = animpb;
	Pt		ipos, p;
	Pixmap	pmicon;						/* pixmap de l'icne  dessiner */
	Pt orig = {0, 0};
        Pt dim = {LYICO, LXICO};

	if ( phase != PHASE_INIT || pt == 0 )  return;

	while ( pt[0] != -1 )
	{
		if ( bFront == 0 && pt >= animpt )  return;
		if ( bFront == 0 || pt > animpt )
		{
			ipos.x = pt[1];
			ipos.y = LYIMAGE()-pt[2]-1;
			if ( ipos.x < pos.x+LXICO && ipos.x+LXICO > pos.x &&
				 ipos.y < pos.y+LYICO && ipos.y+LYICO > pos.y )
			{
#if 0
				GetIcon(&pmicon, pt[5]+ICOMOFF, 1);		/* cherche le pixmap du fond */
				CopyPixel								/* masque le fond */
				(
					&pmicon, (p.y=0, p.x=0, p),
					&pmtemp, (p.y=ipos.y-pos.y, p.x=ipos.x-pos.x, p),
					(p.y=LYICO, p.x=LXICO, p), MODEAND
				);
#endif
                                Pt _pos = {ipos.y - pos.y, ipos.x - pos.x};
				GetIcon(&pmicon, pt[5], 1);				/* cherche le pixmap de la chair */
				CopyPixel								/* dessine la chair */
				(
					&pmicon, orig,
					&pmtemp, _pos,
					dim, MODEOR
				);
			}
		}
		pt += 5+pt[4];
	}
}

/* ------------ */
/* AnimDrawIcon */
/* ------------ */

/*
	Dessine une icne en conservant l'image de fond.
 */

void AnimDrawIcon (Pixmap *ppm, short icon, Pt pos, char bOther)
{
	Pixmap		pmicon;						/* pixmap de l'icne  dessiner */
	Pt			p;

        Pt orig = {0, 0};
        Pt dim = {LYICO, LXICO};
	CopyPixel								/* copie l'image originale */
	(
		&pmimage, pos,
		&pmtemp, orig,
		dim, MODELOAD
	);

	if ( bOther )  AnimIconAddBack(pos, 0);	/* ajoute les autres icnes derrire */

#if 0
	GetIcon(&pmicon, icon+ICOMOFF, 1);		/* cherche le pixmap du fond */
	CopyPixel								/* masque le fond */
	(
		&pmicon, orig,
		&pmtemp, orig,
		dim, MODEAND
	);
#endif

	GetIcon(&pmicon, icon, 1);				/* cherche le pixmap de la chair */
	CopyPixel								/* dessine la chair */
	(
		&pmicon, orig,
		&pmtemp, orig,
		dim, MODEOR
	);

	if ( bOther )  AnimIconAddBack(pos, 1);	/* ajoute les autres icnes devant */

	CopyPixel								/* met dans l'cran */
	(
		&pmtemp, orig,
		ppm, pos,
		dim, MODELOAD
	);
}

/* -------- */
/* AnimDraw */
/* -------- */

/*
	Dessine l'animation en cours.
 */

short AnimDraw (void)
{
	short		icon;
	Pt			pos;

	if ( animpt == 0 )  return DELNORM;

	icon = animpt[5+animnext%animpt[4]];

	pos.x = animpt[1];
	pos.y = LYIMAGE()-animpt[2]-1;

	AnimDrawIcon(0, icon, pos, 1);			/* dessine l'icne */

	return animpt[3];						/* retourne le dlai */
}

/* ------------ */
/* AnimDrawInit */
/* ------------ */

/*
	Dessine toutes les animations en position initiale.
 */

void AnimDrawInit (void)
{
	short	*pt = AnimGetTable();

	if ( pt == 0 )  return;

	animpb = pt;
	while ( pt[0] != -1 )
	{
		if ( phase == PHASE_REGLAGE &&
			 (pt[0] == ACTION_BRUIT0    ||
			  pt[0] == ACTION_NOISEVOLP ||
			  pt[0] == ACTION_MUSICVOLP ) &&
			 fj.noisevolume == 0 )  goto next;
		if ( phase == PHASE_REGLAGE &&
			 (pt[0] == ACTION_BRUIT1    ||
			  pt[0] == ACTION_NOISEVOLM ||
			  pt[0] == ACTION_MUSICVOLM ) &&
			 fj.noisevolume != 0 )  goto next;

		animpt = pt;
		animnext = 0;
		AnimDraw();

		next:
		pt += 5+pt[4];
	}

	if ( phase == PHASE_INIT )
	{
		pt = animpb;
		while ( pt[0] != -1 )
		{
			if ( fj.niveau[fj.joueur] == pt[0]-ACTION_NIVEAU0 )
			{
				animpt    = pt;
				animnext  = 0;
				animdel   = 5;
				animpos.x = 0;
				animpos.y = 0;
				return;
			}
			pt += 5+pt[4];
		}
	}

	animpt  = 0;
	animdel = 0;
}


/* ------------ */
/* AnimTracking */
/* ------------ */

/*
	Initialise une nouvelle animation (si ncessaire).
 */

void AnimTracking (Pt pos)
{
	short		*pt;
	short		delai;

	pt = AnimGetTable();
	if ( pt == 0 )  return;

	if ( animdel != 0 )
	{
		if ( animpos.x != pos.x || animpos.y != pos.y )
		{
			animpos = pos;
			animdel --;
		}
		goto anim;
	}

	pt = AnimSearch(ClicToAction(pos));		/* dtecte l'animation  effectuer */

	if ( pt != animpt )
	{
		if ( animpt != 0 )
		{
			animnext = 0;
			AnimDraw();						/* remet l'animation initiale */
		}
		animpb   = AnimGetTable();
		animpt   = pt;
		animnext = 0;
	}

	anim:
	if ( animpt == 0 )  return;

	OpenTime();
	delai = AnimDraw();
	CloseTime(delai);

	animnext ++;
}




/* Table du gnrique */
/* ------------------ */

static short tgeneric[] =
{
	1,	45,128,		1,
	1,	45,128,		1,
	1,	45,128,		1,
	1,	45,128,		1,
	1,	45,128,		3,
	1,	45,128,		1,
	1,	45,128,		2,
	1,	45,128,		1,
	1,	45,128,		3,
	1,	45,128,		1,
	1,	45,128,		2,
	1,	45,128,		1,
	1,	45,128,		3,
	1,	45,128,		1,
	1,	45,128,		2,
	1,	45,128,		1,
	1,	45,128,		3,
	1,	45,128,		1,
	1,	45,128,		2,
	2,	45,128,		2,


	1,	45,128,		2,
	1,	45,128,		2,
	1,	45,128,		2,
	1,	45,128,		2,
	1,	53,128,		2,
	1,	61,128,		2,
	1,	69,128,		2,
	1,	77,128,		2,
	1,	85,128,		2,
	1,	93,128,		2,
	1,	101,128,	2,
	1,	109,128,	2,
	1,	116,128,	2,
	1,	116,128,	2,
	1,	116,128,	2,
	1,	116,128,	2,

	1,	116,128,	1,
	1,	116,128,	1,
	1,	116,128,	1,
	1,	116,128,	20,
	1,	116,132,	21,
	1,	116,136,	21,
	1,	116,140,	21,
	1,	116,142,	21,
	1,	116,140,	21,
	1,	116,136,	21,
	1,	116,132,	21,
	1,	116,128,	20,
	1,	116,128,	1,
	1,	116,128,	1,
	1,	116,128,	18,
	1,	116,128,	4,
	1,	116,128,	4,
	1,	116,128,	22,
	1,	116,132,	23,
	1,	116,136,	23,
	1,	116,140,	23,
	1,	116,142,	23,
	1,	116,140,	23,
	1,	116,136,	23,
	1,	116,132,	23,
	1,	116,128,	22,
	1,	116,128,	23,
	1,	116,128,	23,
	2,	116,128,	23,


	1,	116,128,	4,
	1,	116,128,	4,
	1,	116,128,	4,
	1,	116,128,	4,
	1,	124,128,	4,
	1,	132,128,	4,
	1,	140,128,	4,
	1,	148,128,	4,
	1,	156,128,	4,
	1,	172,128,	4,
	1,	180,128,	4,
	1,	186,128,	4,
	1,	186,128,	4,
	1,	186,128,	4,
	1,	186,128,	4,

	1,	186,128,	18,
	1,	186,128,	1,
	1,	186,128,	33,
	1,	186,128,	81,
	1,	186,128,	83,
	1,	186,128,	81,
	1,	186,128,	82,
	1,	186,128,	81,
	1,	186,128,	83,
	1,	186,128,	81,
	1,	186,128,	82,
	1,	186,128,	81,
	1,	186,128,	83,
	1,	186,128,	81,
	1,	186,128,	82,
	1,	186,128,	81,
	1,	186,128,	83,
	1,	186,128,	81,
	1,	186,128,	82,
	2,	186,128,	82,


	1,	186,128,	82,
	1,	186,128,	82,
	1,	186,128,	82,
	1,	186,128,	82,
	1,	193,128,	82,
	1,	200,128,	82,
	1,	207,128,	82,
	1,	214,128,	82,
	1,	221,128,	82,
	1,	228,128,	82,
	1,	235,128,	82,
	1,	242,128,	82,
	1,	249,128,	82,
	1,	253,128,	82,
	1,	253,128,	82,
	1,	253,128,	82,
	1,	253,128,	82,

	1,	253,128,	33,
	1,	253,128,	1,
	1,	253,128,	18,
	1,	253,128,	4,
	1,	253,128,	17,
	1,	253,128,	7,
	1,	253,128,	19,
	1,	253,128,	10,
	1,	253,128,	16,
	1,	253,128,	1,
	1,	253,128,	18,
	1,	253,128,	4,
	1,	253,128,	17,
	1,	253,128,	7,
	1,	253,128,	19,
	1,	253,128,	10,
	1,	253,128,	16,
	1,	253,128,	1,
	1,	253,128,	18,
	1,	253,128,	18,
	1,	253,128,	58,
	1,	253,129,	59,
	1,	253,128,	58,
	1,	253,129,	59,
	1,	253,128,	58,
	1,	253,129,	59,
	1,	253,128,	58,
	1,	253,128,	18,
	1,	253,128,	18,
	2,	253,128,	18,


	1,	253,128,	18,
	1,	253,128,	18,
	1,	253,128,	18,
	1,	253,128,	18,
	1,	261,128,	18,
	1,	269,128,	18,
	1,	277,128,	18,
	1,	285,128,	18,
	1,	293,128,	18,
	1,	301,128,	18,
	1,	309,128,	18,
	1,	317,128,	18,
	1,	324,128,	18,
	1,	324,128,	18,
	1,	324,128,	18,
	1,	324,128,	18,

	1,	324,128,	4,
	1,	324,128,	5,
	1,	324,128,	4,
	1,	324,128,	6,
	1,	324,128,	4,
	1,	324,128,	5,
	1,	324,128,	4,
	1,	324,128,	6,
	1,	324,128,	4,
	1,	324,128,	5,
	1,	324,128,	4,
	1,	324,128,	6,
	1,	324,128,	4,
	1,	324,128,	18,
	1,	324,128,	45,
	1,	324,128,	46,
	1,	324,128,	45,
	1,	324,128,	18,
	1,	324,128,	45,
	1,	324,128,	46,
	1,	324,128,	45,
	1,	324,128,	18,
	1,	324,128,	4,
	1,	324,128,	5,
	1,	324,128,	4,
	1,	324,128,	6,
	2,	324,128,	6,


	1,	324,128,	6,
	1,	324,128,	6,
	1,	324,128,	6,
	1,	324,128,	6,
	1,	332,128,	6,
	1,	340,128,	6,
	1,	348,128,	6,
	1,	356,128,	6,
	1,	364,128,	6,
	1,	372,128,	6,
	1,	380,128,	6,
	1,	388,128,	6,
	1,	388,128,	6,
	1,	388,128,	6,
	1,	388,128,	6,

	1,	388,128,	4,
	1,	388,128,	18,
	1,	388,128,	1,
	1,	388,128,	113,
	1,	388,128,	113,
	1,	388,128,	113,
	1,	388,128,	114,
	1,	388,128,	113,
	1,	388,128,	115,
	1,	388,128,	113,
	1,	388,128,	114,
	1,	388,128,	113,
	1,	388,128,	115,
	1,	388,128,	113,
	1,	388,128,	114,
	1,	388,128,	113,
	1,	388,128,	100,
	1,	388,128,	100,
	1,	388,128,	113,
	1,	388,128,	100,
	1,	388,128,	100,
	1,	388,128,	113,
	1,	388,128,	115,
	1,	388,128,	113,
	1,	388,128,	114,
	1,	388,128,	113,
	2,	388,128,	113,


	1,	388,128,	113,
	1,	388,128,	113,
	1,	388,128,	113,
	1,	388,128,	113,
	1,	396,128,	113,
	1,	404,128,	113,
	1,	412,128,	113,
	1,	420,128,	113,
	1,	428,128,	113,
	1,	436,128,	113,
	1,	444,128,	113,
	1,	449,128,	113,
	1,	449,128,	113,
	1,	449,128,	113,
	1,	449,128,	113,

	1,	449,128,	33,
	1,	449,128,	50,
	1,	449,128,	36,
	1,	449,128,	36,
	1,	449,128,	36,
	1,	449,128,	36,
	1,	449,128,	65,
	1,	449,128,	65,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	69,
	1,	449,128,	65,
	1,	449,128,	65,
	1,	449,128,	65,
	2,	449,128,	65,


	1,	449,128,	65,
	1,	449,128,	65,
	1,	449,128,	65,
	1,	449,128,	65,
	1,	457,128,	65,
	1,	465,128,	65,
	1,	473,128,	65,
	1,	481,128,	65,
	1,	489,128,	65,
	1,	497,128,	65,
	1,	505,128,	65,
	1,	512,128,	65,
	1,	512,128,	65,
	1,	512,128,	65,
	1,	512,128,	65,

	1,	512,128,	36,
	1,	512,128,	36,
	1,	512,128,	29,
	1,	512,128,	29,
	1,	512,128,	36,
	1,	512,128,	29,
	1,	512,128,	36,
	1,	512,128,	36,
	1,	512,128,	33,
	1,	512,128,	50,
	1,	512,128,	36,
	1,	512,128,	50,
	1,	512,128,	33,
	1,	512,128,	50,
	1,	512,128,	36,
	1,	512,128,	50,
	1,	512,128,	33,
	1,	512,128,	50,
	1,	512,128,	36,
	1,	512,128,	50,
	2,	512,128,	50,

	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,
	1,	512,128,	50,

	0
};


/* ----------- */
/* GenericNext */
/* ----------- */

/*
	Effectue l'animation du gnrique.
	Retourne 1 si c'est fini !
 */

void GenericNext (void)
{
	Pixmap	*ppm;
	Pt		pos;

	if ( tgeneric[generic*4] == 0 )
	{
		ShowImage();					/* raffiche l'image gnrique */
		generic = 0;
	}

	if ( tgeneric[generic*4] == 1 )  ppm = 0;
	else                             ppm = &pmimage;

	pos.x = tgeneric[generic*4+1];
	pos.y = LYIMAGE()-tgeneric[generic*4+2];

	OpenTime();
	AnimDrawIcon(ppm, tgeneric[generic*4+3], pos, 0);
	CloseTime(DELNORM);

	generic ++;
}



/* ------------- */
/* ExecuteAction */
/* ------------- */

/*
	Excute une action (change de phase).
	Retourne 2 s'il faut quitter.
 */

short ExecuteAction (char event, Pt pos)
{
	PhAction	action;
	short		dstmonde;
	short		lastmusique;

	if ( event == KEYCLIC )
	{
		action = ClicToAction(pos);				/* action selon la position vise */
	}
	else
	{
		action = EventToAction(event);			/* action selon la touche presse */
	}

	if ( action != -1 )
	{
		PlayEvSound(SOUND_CLIC);
	}

	if ( action == ACTION_IDENT )
	{
		ChangePhase(PHASE_IDENT);
		return 0;
	}

	if ( action == ACTION_REGLAGE )
	{
		ChangePhase(PHASE_REGLAGE);
		return 0;
	}

	if ( action == ACTION_DEBUT )
	{
		if ( fj.nom[fj.joueur][0] != 0 )		/* nom du joueur existe ? */
		{
			ChangePhase(PHASE_INIT);
		}
		else
		{
			ChangePhase(PHASE_IDENT);
		}
		return 0;
	}

	if ( action == ACTION_FINI )
	{
		lastmusique = musique;
		if ( fj.niveau[fj.joueur] == 8 )		/* priv ? */
		{
			         g_monde = maxmonde-1;
			ChangePhase(PHASE_PRIVE);
			musique = lastmusique;
			return 0;
		}
		if ( fj.nom[fj.joueur][0] != 0 )		/* nom du joueur existe ? */
		{
			ChangePhase(PHASE_INIT);
		}
		else
		{
			ChangePhase(PHASE_IDENT);
		}
		musique = lastmusique;
		return 0;
	}

	if ( action >= ACTION_AIDE &&
		 action <= ACTION_AIDE42 )
	{
		ChangePhase(action-ACTION_AIDE+PHASE_AIDE);
		return 0;
	}

	if ( action == ACTION_OBJECTIF )
	{
		if ( g_construit )  ChangePhase(PHASE_PRIVE);
		else              ChangePhase(PHASE_OBJECTIF);
		return 0;
	}

	if ( action == ACTION_JOUE )
	{
		ChangePhase(PHASE_PLAY);
		return 0;
	}

	if ( action == ACTION_SUIVANT )
	{
		      g_monde ++;
		if ( fj.progres[fj.joueur][fj.niveau[fj.joueur]] < g_monde )
		{
			fj.progres[fj.joueur][fj.niveau[fj.joueur]] = g_monde;
		}
		JoueurWrite();							/* crit le fichier des joueurs */
		if ( g_monde >= maxmonde )  g_monde = 0;
		lastmusique = musique;
		if ( g_construit )  ChangePhase(PHASE_PRIVE);
		else              ChangePhase(PHASE_OBJECTIF);
		musique = lastmusique;
		return 0;
	}

	if ( action == ACTION_ANNULE )
	{
		if ( g_construit )  ChangePhase(PHASE_PRIVE);
		else              ChangePhase(PHASE_INIT);
		return 0;
	}

	if ( action == ACTION_STOPPEOK )
	{
		if ( fj.progres[fj.joueur][fj.niveau[fj.joueur]] < g_monde+1 )
		{
			fj.progres[fj.joueur][fj.niveau[fj.joueur]] = g_monde+1;
		}
		JoueurWrite();							/* crit le fichier des joueurs */
		if ( g_construit )  ChangePhase(PHASE_PRIVE);
		else              ChangePhase(PHASE_OBJECTIF);
		return 0;
	}

	if ( action == ACTION_STOPPEKO )
	{
		if ( g_construit )  ChangePhase(PHASE_PRIVE);
		else              ChangePhase(PHASE_OBJECTIF);
		return 0;
	}

	if ( action == ACTION_PARAM )
	{
		ChangePhase(PHASE_PARAM);
		return 0;
	}

	if ( action == ACTION_MONDEPREC &&
		 (GetDemo() == 0 || !g_construit) )
	{
		if ( g_monde > 0 )
		{
			         g_monde --;
			DrawNumMonde();					/* affiche le numro du monde */
			MondeRead(g_monde, banque);		/* lit le nouveau monde sur disque */
			DrawObjectif();					/* affiche l'objectif */
			return 0;
		}
		return 1;
	}

	if ( action == ACTION_MONDESUIV &&
		 (GetDemo() == 0 || !g_construit) )
	{
		if ( g_monde < maxmonde-1 &&
			 (g_construit || g_monde < fj.progres[fj.joueur][fj.niveau[fj.joueur]]) )
		{
			         g_monde ++;
			DrawNumMonde();					/* affiche le numro du monde */
			MondeRead(g_monde, banque);		/* lit le nouveau monde sur disque */
			DrawObjectif();					/* affiche l'objectif */
			return 0;
		}
		return 1;
	}

	if ( action == ACTION_MONDEBAR &&
		 (GetDemo() == 0 || !g_construit) )
	{
		TrackingStatusBar(pos);
		return 0;
	}

	if ( action == ACTION_EDIT )
	{
		      g_typeedit = 1;
		ChangePhase(PHASE_PLAY);
		return 0;
	}

	if ( action == ACTION_OPER &&
		 GetDemo() == 0 )
	{
		ChangePhase(PHASE_OPER);
		return 0;
	}

	if ( action == ACTION_DEPLACE )
	{
		ChangePhase(PHASE_DEPLACE);
		return 0;
	}

	if ( action == ACTION_ORDRE )
	{
		dstmonde = g_monde;
		ChangePhase(PHASE_ATTENTE);			/* affiche "attendez-un instant ..." */
		MondeDeplace(mondeinit, dstmonde);
		ChangePhase(PHASE_PRIVE);
		return 0;
	}

	if ( action == ACTION_DUPLIQUE )
	{
		ChangePhase(PHASE_ATTENTE);			/* affiche "attendez-un instant ..." */
		MondeDuplique(g_monde);
		ChangePhase(PHASE_PRIVE);
		return 0;
	}

	if ( action == ACTION_DETRUIT )
	{
		ChangePhase(PHASE_ATTENTE);			/* affiche "attendez-un instant ..." */
		MondeDetruit(g_monde);
		ChangePhase(PHASE_PRIVE);
		return 0;
	}

	if ( action >= ACTION_JOUEUR0 &&
		 action <= ACTION_JOUEUR3 )
	{
		JoueurEditClose();					/* fin de l'dition du nom en cours */
		fj.joueur = action - ACTION_JOUEUR0;
		DrawJoueur();
		JoueurEditOpen();					/* prpare l'dition du nouveau nom */
		return 0;
	}

	if ( (action >= ACTION_NIVEAU0 &&
		  action <= ACTION_NIVEAU8) ||
		 action == ACTION_NIVEAUGO )
	{
#ifdef DEMONC
		if ( action == ACTION_NIVEAU8 && GetDemo() == 1 )  return 0;
#endif

		if ( action != ACTION_NIVEAUGO )
		{
			fj.niveau[fj.joueur] = action - ACTION_NIVEAU0;
		}

		if ( fj.niveau[fj.joueur] < 8 )		/* fastoche/costaud/durdur/mga ? */
		{
			banque = fj.niveau[fj.joueur]+'a';
			if ( g_passdaniel )  g_construit = 1;
			else               g_construit = 0;
			MondeMax(banque);
			         g_monde = fj.progres[fj.joueur][fj.niveau[fj.joueur]];
		}
		else								/* priv ? */
		{
			banque = fj.joueur+'i';
			         g_construit = 1;
			MondeMax(banque);
			         g_monde = 0;
		}
		if ( g_construit )  ChangePhase(PHASE_PRIVE);
		else              ChangePhase(PHASE_OBJECTIF);
		return 0;
	}

	if ( action == ACTION_NIVEAUK1 )
	{
		if ( fj.niveau[fj.joueur] == 0 )  fj.niveau[fj.joueur] = 1;
		else                              fj.niveau[fj.joueur] = 0;
		AnimDrawInit();
		return 0;
	}
	if ( action == ACTION_NIVEAUK2 )
	{
		if ( fj.niveau[fj.joueur] == 2 )  fj.niveau[fj.joueur] = 3;
		else                              fj.niveau[fj.joueur] = 2;
		AnimDrawInit();
		return 0;
	}
	if ( action == ACTION_NIVEAUK3 )
	{
		if ( fj.niveau[fj.joueur] == 4 )  fj.niveau[fj.joueur] = 5;
		else                              fj.niveau[fj.joueur] = 4;
		AnimDrawInit();
		return 0;
	}
	if ( action == ACTION_NIVEAUK4 )
	{
		if ( fj.niveau[fj.joueur] == 6 )  fj.niveau[fj.joueur] = 7;
		else                              fj.niveau[fj.joueur] = 6;
		AnimDrawInit();
		return 0;
	}
#ifdef DEMONC
	if ( action == ACTION_NIVEAUK5 && GetDemo() == 0 )
#else
	if ( action == ACTION_NIVEAUK5 )
#endif
	{
		fj.niveau[fj.joueur] = 8;
		AnimDrawInit();
		return 0;
	}

	if ( action >= ACTION_VITESSE0 &&
		 action <= ACTION_VITESSE2 )
	{
		fj.vitesse = action - ACTION_VITESSE0;
		DrawVitesse();
		return 0;
	}

	if ( action >= ACTION_SCROLL0 &&
		 action <= ACTION_SCROLL1 )
	{
		fj.scroll = action - ACTION_SCROLL0;
		DrawScroll();
		return 0;
	}

	if ( action >= ACTION_BRUIT0 &&
		 action <= ACTION_BRUIT1 )
	{
		if ( action == ACTION_BRUIT0 )
		{
			fj.noisevolume = 10;
			fj.musicvolume = 10;
		}
		else
		{
			fj.noisevolume = 0;
			fj.musicvolume = 0;
		}
		PlayNoiseVolume(fj.noisevolume);
		PlayMusicVolume(fj.musicvolume);
		DrawBruitage();
		return 0;
	}

	if ( action == ACTION_NOISEVOLP &&
		 fj.noisevolume < 10 )
	{
		fj.noisevolume ++;
		PlayNoiseVolume(fj.noisevolume);
		DrawBruitage();
		PlaySound(SOUND_MAGIE);
		return 0;
	}

	if ( action == ACTION_NOISEVOLM &&
		 fj.noisevolume > 0 )
	{
		fj.noisevolume --;
		PlayNoiseVolume(fj.noisevolume);
		DrawBruitage();
		PlaySound(SOUND_MAGIE);
		return 0;
	}

	if ( action == ACTION_MUSICVOLP &&
		 fj.musicvolume < 10 )
	{
		fj.musicvolume ++;
		PlayMusicVolume(fj.musicvolume);
		DrawBruitage();
		return 0;
	}

	if ( action == ACTION_MUSICVOLM &&
		 fj.musicvolume > 0 )
	{
		fj.musicvolume --;
		PlayMusicVolume(fj.musicvolume);
		DrawBruitage();
		return 0;
	}

	if ( action >= ACTION_TELECOM0 &&
		 action <= ACTION_TELECOM1 )
	{
		fj.modetelecom = action - ACTION_TELECOM0;
		      g_modetelecom = fj.modetelecom;
		DrawTelecom();
		return 0;
	}

	if ( action >= ACTION_COULEUR0 &&
		 action <= ACTION_COULEUR4 )
	{
		descmonde.color = action - ACTION_COULEUR0;
		DrawCouleur();
		return 0;
	}

	if ( action == ACTION_QUITTE )
	{
		return 2;
	}

	return 1;
}




#if __SMAKY__
/* Table pour les musiques de fond */
/* ------------------------------- */

static short tmusic[] =
{
	1,	SOUND_MUSIC11,
	2,	SOUND_MUSIC21,
	3,	SOUND_MUSIC31,
	4,	SOUND_MUSIC41,
	0
};


/* --------------- */
/* MusicBackground */
/* --------------- */

/*
	Gère les musiques de fond, selon la phase.
 */

void MusicBackground (void)
{
	short	*ptable = tmusic;
	short	n, sound;

	if ( musique == 0 || !IfPlayReady() )  return;

	while ( ptable[0] != 0 )
	{
		if ( musique == ptable[0] )
		{
			if ( lastaccord == -1 )
			{
				sound = 0;									/* accord de base */
			}
			else
			{
				n = 0;
				do
				{
					#if 1
					sound = GetRandom(1, 0, 8+1) / 2;		/* sound <- 0..4 */
					#else
					sound = GetRandom(1, 0, 12+1) / 3;		/* sound <- 0..4 */
					#endif
					n ++;
				}
				while ( sound == lastaccord && n < 2 );
			}
			lastaccord = sound;
			if ( sound != 0 )  sound --;					/* accord de base plus souvent */

			PlaySound(ptable[1]+sound);
			return;
		}
		ptable += 2;
	}
}
#endif


/* ======== */
/* PlayInit */
/* ======== */

/*
	Initialise le jeu.
	Retourne != 0 en cas d'erreur.
 */

static short PlayInit (void)
{
	short		err;
	Pt			p;

	OpenMachine();						/* ouverture générale */

	//IconInit();							/* calcule bbox des icônes */

        g_screen.texture = SDL_CreateTexture (g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXIMAGE(), LYIMAGE());
        SDL_SetTextureBlendMode(g_screen.texture, SDL_BLENDMODE_BLEND);
        g_screen.dx = LXIMAGE();
        g_screen.dy = LYIMAGE();

        pmtemp.texture = SDL_CreateTexture (
          g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXICO, LYICO);
        SDL_SetTextureBlendMode(pmtemp.texture, SDL_BLENDMODE_BLEND);
        pmtemp.dx = LXICO;
        pmtemp.dy = LYICO;
        //SDL_SetTextureBlendMode (pmtemp.texture, SDL_BLENDMODE_BLEND);
        /*pmimage.texture = SDL_CreateTexture (
          g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXIMAGE(), LYIMAGE());*/
	//err = GetPixmap(&pmtemp, (p.y=LYICO, p.x=LXICO, p), 0, 1);
	//if ( err )  FatalBreak(err);

	   g_monde       = 0;					/* premier monde */
	banque      = 'A';					/* banque de base */
	phase       = -1;					/* pas de phase connue */
	   g_pause       = 0;
	   g_construit   = 0;
	   g_typejeu     = 0;					/* jeu sans télécommande */
	   g_typeedit    = 0;					/* pas d'édition en cours */
	   g_typetext    = 0;					/* pas d'édition de ligne */
	   g_modetelecom = 0;
	lastkey     = 0;
	   g_passdaniel  = 0;					/* mode normal (sans passe-droit) */
	   g_passpower   = 0;					/* mode normal (sans passe-droit) */
	   g_passnice    = 0;					/* mode normal (sans passe-droit) */
	   g_passhole    = 0;					/* mode normal (sans passe-droit) */
	animpb      = 0;					/* pas d'animation en cours */
	animpt      = 0;					/* pas d'animation en cours */
	generic     = 0;					/* pas du générique */

	InitRandomEx(1, 1, 4+1, musiquehex);/* init hazard musique exclusive */

	MondeVide();

	BlackScreen();						/* efface l'écran */

	return ChangePhase(PHASE_GENERIC);	/* première phase du jeu */
}



/* ========= */
/* PlayEvent */
/* ========= */

/*
	Donne un événement.
	Retourne 1 si l'action est terminée.
	Retourne 2 si le jeu est terminé.
 */

static short PlayEvent (const SDL_Event * event, int key, Pt pos, SDL_bool next)
{
	char		ev;
	Pt			ovisu;
	short		term, max, delai, last;
	KeyStatus	keystatus;
	Rectangle	rect;

	static char *pass[] = {"petitblupi", "enigmeblupi", "totalblupi",
						   "gentilblupi", "sauteblupi", "megablupi"};

	if ( phase == PHASE_GENERIC )
	{
#if __SMAKY__
		MusicBackground();						/* gère la musique de fond */
#endif
                if (next)
                  GenericNext();							/* anime le générique */
		if ( key != 0 )
		{
			ChangePhase(PHASE_IDENT);
		}
		return 1;
	}

	if ( phase != PHASE_PLAY )
	{
		if ( phase == PHASE_INIT && GetDemo() == 0 &&
			 ((key >= 'a' && key <= 'z') || (key >= '0' && key <= '9')) )
		{
			if ( passindex == 0 )
			{
				for ( passrang=0 ; passrang<6 ; passrang++ )
				{
					if ( key == pass[passrang][0] )  break;
				}
			}
			if ( passrang < 6 && key == pass[passrang][passindex] )
			{
				passindex ++;
				if ( pass[passrang][passindex] == 0 )	/* fin du mot de passe ? */
				{
					passindex = 0;
					switch ( passrang )
					{
						case 1:
							                 g_passdaniel = 1;				/* passe-droit spécial */
							break;
						case 2:
							                 g_passpower  = 1;				/* passe-droit spécial */
							break;
						case 3:
							                 g_passnice   = 1;				/* passe-droit spécial */
							break;
						case 4:
							                 g_passhole   = 1;				/* passe-droit spécial */
							break;
						case 5:
							                 g_passdaniel = 1;				/* passe-droit spécial */
							                 g_passpower  = 1;				/* passe-droit spécial */
							                 g_passnice   = 1;				/* passe-droit spécial */
							                 g_passhole   = 1;				/* passe-droit spécial */
							break;
						default:
							                 g_passdaniel = 0;				/* plus de passe-droit */
							                 g_passpower  = 0;
							                 g_passnice   = 0;
							                 g_passhole   = 0;
					}
					PlaySound(SOUND_MAGIE);
					for ( max=0 ; max<10 ; max++ )
					{
						rect.p1.x = 0;
						rect.p1.y = 0;
						rect.p2.x = LXIMAGE();
						rect.p2.y = LYIMAGE();
						DrawFillRect(0, rect, MODEXOR, COLORNOIR);	/* flash */
						for ( delai=0 ; delai<20000 ; delai++ );
					}
				}
			}
			else
			{
				passindex = 0;
			}
		}

		if ( phase == PHASE_INIT && key == KEYQUIT &&
			 StopPartie() == KEYHOME )
		{
			return 2;
		}

		if ( key == KEYQUIT || key == KEYHOME || key == KEYUNDO )
		{
			ChangePhase(PHASE_INIT);
			return 1;
		}

		if ( phase == PHASE_IDENT && key != KEYRETURN )
		{
			if ( EditEvent(key, pos) >= 0 )  return 1;
		}

		if ( phase == PHASE_PARAM )
		{
			if ( PaletteEditEvent(descmonde.palette, key, pos) == 0 )  return 1;
			if ( EditEvent(key, pos) >= 0 )  return 1;
		}

		term = ExecuteAction(key, pos);

		if ( term == 2 &&
			 StopPartie() == KEYHOME )  return 2;

		if ( term != 0 )
		{
#ifdef __SMAKY__
			MusicBackground();				/* gre la musique de fond */
#endif
                        if (next)
                          AnimTracking(pos);				/* tracking de l'animation */
		}

		OpenTime();
		CloseTime(3);						/* pour ne pas trop occuper le CPU ! */
		return 1;
	}
	else
	{
                if (ignoreKeyClicUp == SDL_TRUE)
                {
                      /* Prevent key up just when entering in the play phase */
                      if (key == KEYCLICREL) {
                        ignoreKeyClicUp = SDL_FALSE;
                        key = 0;
                      }
                }
		if ( key == KEYCLIC || key == KEYCLICREL || (key >= KEYF4 && key <= KEYF1) )
		{
			ev = PaletteEvent(key, pos);
			if ( g_typejeu == 0 || g_typeedit )
			{
				if ( ev < 0 )  key = ev;
				if ( ev == 1 )
				{
					DecorEvent(pos, 0, PaletteGetPress(), key);
				}
			}
			else
			{
				if ( ev < 0 )  key = ev;
				if ( ev == 0 )
				{
					MoveBuild(PaletteGetPress(), key);
				}
			}

                        lastkey = key;
                        fromClic = SDL_TRUE;
		}

		if ( g_typeedit == 0 &&
			 (key == KEYSAVE || key == KEYLOAD || key == KEYIO) )
		{
			PlayEvSound(SOUND_CLIC);
			PartieDisque(key);						/* prend/sauve la partie en cours ... */
			return 1;
		}

		if ( key == KEYF5 )							/* bruitages oui/non */
		{
			if ( fj.noisevolume == 0 )
			{
				if ( lastnoisevolume == 0 )  lastnoisevolume = 10-3;
				if ( lastmusicvolume == 0 )  lastmusicvolume = 10-3;
				fj.noisevolume = lastnoisevolume;
				fj.musicvolume = lastmusicvolume;
				PlayNoiseVolume(fj.noisevolume);
				PlayMusicVolume(fj.musicvolume);
			}
			else
			{
				lastnoisevolume = fj.noisevolume;
				lastmusicvolume = fj.musicvolume;
				fj.noisevolume  = 0;
				fj.musicvolume  = 0;
				PlayNoiseVolume(0);
				PlayMusicVolume(0);
			}
			return 1;
		}

		if ( key == KEYF6 )							/* dcalage progressif/rapide */
		{
			if ( fj.scroll )  fj.scroll = 0;
			else              fj.scroll = 1;
			return 1;
		}

		if ( key == KEYF7 )							/* vitesse = tortue */
		{
			fj.vitesse = 0;
			return 1;
		}
		if ( key == KEYF8 )							/* vitesse = normal */
		{
			fj.vitesse = 1;
			return 1;
		}
		if ( key == KEYF9 )							/* vitesse = gupard */
		{
			fj.vitesse = 2;
			return 1;
		}

		if ( g_typejeu == 1 &&
			 fj.modetelecom == 1 &&
			             g_pause == 0
			             && !fromClic )
		{
			//if ( key == KEYLEFT   )  key = KEYGOLEFT;
			//if ( key == KEYRIGHT  )  key = KEYGORIGHT;

			keystatus = GetKeyStatus();
			if ( keystatus != 0 )
			{
				if ( keystatus == STLEFT  )  key = KEYGOLEFT;
				if ( keystatus == STRIGHT )  key = KEYGORIGHT;
				if ( keystatus == STUP    )  key = KEYGOFRONT;
				if ( keystatus == STDOWN  )  key = KEYGOBACK;
                                lastkey = key;
			}
			else
			{
				if ( lastkey != 0 )  key = KEYCLICREL;
				lastkey = 0;
			}
		}

		if ( key == KEYGOFRONT || key == KEYGOBACK || key == KEYGOLEFT || key == KEYGORIGHT )
		{
			DrawArrows(key);				/* dessine les manettes de la télécommande */
		}

		if ( key == KEYCLICREL )
		{
			DrawArrows(0);						/* dessine les manettes de la télécommande */
                        lastkey = 0;
                        fromClic = SDL_FALSE;
		}

                if ( g_typejeu == 1 && g_pause == 0 )
		{
                  if (lastkey)
                    key = lastkey;
                }

		if ( key == KEYQUIT || key == KEYHOME || key == KEYUNDO )
		{
			if ( g_typeedit == 1 ||
				 StopPartie() == KEYHOME )
			{
				if ( g_typeedit )  ChangePhase(PHASE_PRIVE);
				else             ChangePhase(PHASE_RECOMMENCE);
				return 1;
			}
		}

		if ( g_typejeu == 0 || g_typeedit || g_pause )
		{
			ovisu = DecorGetOrigine();
			if ( key == KEYRIGHT && ovisu.x > -8 )
			{
				PlayEvSound(SOUND_CLIC);
				ovisu.x -= 4;
				DecorSetOrigine(ovisu, fj.scroll);
			}
			if ( key == KEYLEFT && ovisu.x < 16 )
			{
				PlayEvSound(SOUND_CLIC);
				ovisu.x += 4;
				DecorSetOrigine(ovisu, fj.scroll);
			}
			if ( key == KEYDOWN && ovisu.y > -20 )
			{
				PlayEvSound(SOUND_CLIC);
				ovisu.y -= 5;
				DecorSetOrigine(ovisu, fj.scroll);
			}
			if ( key == KEYUP && ovisu.y < 0 )
			{
				PlayEvSound(SOUND_CLIC);
				ovisu.y += 5;
				DecorSetOrigine(ovisu, fj.scroll);
			}
			if ( key == KEYCENTER )
			{
				PlayEvSound(SOUND_CLIC);
				ovisu.x = 4;
				DecorSetOrigine(ovisu, fj.scroll);
				ovisu.y = -10;
				DecorSetOrigine(ovisu, fj.scroll);
			}
		}
		else
		{
			last = g_typejeu;
			MoveScroll(fj.scroll);			/* dcale v. selon le toto du joueur */
			if ( last != g_typejeu )			/* type de jeu chang ? */
			{
				DrawArrows(0);				/* oui -> remet les flches/tlcommande */
			}
		}

		if ( key == KEYPAUSE && g_typeedit == 0 )
		{
			PlayEvSound(SOUND_CLIC);
			         g_pause ^= 1;						/* met/enlve la pause */
			DrawPause();					/* dessine le bouton pause */
			DrawArrows(0);					/* dessine les flches */
		}

		switch ( fj.vitesse )
		{
			case 0:
                          delai = DELSLOW;
                          g_timerSkip = 5;
                          break;
			case 2:
                          delai = DELQUICK;
                          g_timerSkip = 3;
                          break;
			default:
                          delai = DELNORM;
                          g_timerSkip = 4;
                          break;
		}

		if ( g_pause == 0 && next )
		{
			OpenTime();
			IconDrawOpen();
			DecorSuperCel(pos);				/* indique la cellule vise par la souris */
			term = MoveNext(key, pos);		/* anime jusqu'au pas suivant */
			IconDrawClose(1);
			CloseTime(delai);

			if ( term == 1 )				/* termin gagn ? */
			{
				if ( g_construit )  max = maxmonde-1;
				else              max = maxmonde;
				if ( g_monde >= max-1 )
				{
					ChangePhase(PHASE_FINI0 + fj.niveau[fj.joueur]);
					return 1;
				}
				ChangePhase(PHASE_SUIVANT);
				musique = GetRandomEx(1, 1, 4+1, musiquehex);
				lastaccord = -1;
				return 1;
			}
			if ( term == 2 )				/* termin perdu ? */
			{
				max = 0;
				do
				{
					OpenTime();
					IconDrawOpen();
					MoveNext(key, pos);		/* anime jusqu'au pas suivant */
					IconDrawClose(1);
					CloseTime(delai);
					max += delai;
				}
				while ( max < 100 );		/* attend une seconde ... */

				if ( g_typeedit )  ChangePhase(PHASE_PRIVE);
				else             ChangePhase(PHASE_RECOMMENCE);
				return 1;
			}
		}
		if (g_pause != 0 || (g_pause == 0 && !next))
		{
			OpenTime();
			IconDrawOpen();
			DecorSuperCel(pos);				/* indique la cellule visée par la souris */
			MoveRedraw();					/* redessine sans changement */
			IconDrawClose(1);
			CloseTime(delai);
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

void FatalBreak (short err)
{
	PlayRelease();						/* libère tout */
	FatalError(err);					/* quitte */
}


/* =========== */
/* PlayRelease */
/* =========== */

/*
	Fermeture générale.
 */

static void PlayRelease (void)
{
	StartRandom(0, 1);
	PlaySound(GetRandom(0, SOUND_SAUT1, SOUND_CAISSEG+1));
	HideMouse();
	OpenTime();
	CloseTime(130);

	BlackScreen();			/* efface tout l'écran */

	GivePixmap(&pmimage);
	GivePixmap(&pmtemp);

	DecorClose();			/* fermeture des décors */
	IconClose();			/* fermeture des icônes */
	MoveClose();			/* fermeture des objets en mouvement */
	CloseMachine();			/* fermeture générale */
}

static void
PushUserEvent (Sint32 code, void * data)
{
  SDL_Event event;

  event.type       = SDL_USEREVENT;
  event.user.code  = code;
  event.user.data1 = data;
  event.user.data2 = NULL;

  SDL_PushEvent (&event);
}

static Uint32 MainLoop (Uint32 interval, void * param)
{
  static int skip;
  if (!(skip % g_timerSkip))
      PushUserEvent (1548 /*EV_UPDATE*/, NULL);
  ++skip;
  return interval;
}



/* =================== */
/* Programme principal */
/* =================== */

int main (int argc, char *argv[])
{
	int			err;						/* condition de sortie */
	short		key;						/* touche pressée  */
	Pt			pos;						/* position de la souris */

	PlayInit();								/* initialise le jeu */

	if ( argc == 2 && strcmp(argv[1], "-d") == 0 )
	{
		SetDemo(1);
	}

        SDL_TimerID updateTimer = SDL_AddTimer (
          g_timerInterval,
          &MainLoop,
          NULL);

        SDL_Event event;
        SDL_bool next = SDL_FALSE;
        int nextKeys[4] = {0};
        while (SDL_WaitEvent (&event))
        {
          next = SDL_FALSE;

          if (event.type == SDL_MOUSEMOTION)
          {
            SDL_MouseMotionEvent * _event = (SDL_MouseMotionEvent *) &event;
            g_lastmouse.x = _event->x;
            g_lastmouse.y = _event->y;
            continue;
          }

          if (event.user.code == 1548)
          {
            next = SDL_TRUE;
            key = nextKeys[0];
            nextKeys[0] = nextKeys[1];
            nextKeys[1] = nextKeys[2];
            nextKeys[2] = nextKeys[3];
            nextKeys[3] = 0;
          }
          else
          {
            key = SDLEventToSmakyKey(&event);
            /* Ensure that the action is done on the next (game) frame */
            if (phase == PHASE_PLAY
              && (key == KEYCLIC || key == KEYCLICR || key == KEYCLICREL
                || key == KEYLEFT || key == KEYRIGHT || key == KEYUP || key == KEYDOWN))
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

          SDL_RenderCopy(g_renderer, g_screen.texture, NULL, NULL);
          SDL_RenderPresent(g_renderer);

          err = PlayEvent(&event, key, g_lastmouse, next);			/* fait évoluer le jeu */
          if ( err == 2 )  break;				/* quitte si terminé */
          if (event.type == SDL_QUIT)
            break;

          if (g_clearKeyEvents)
          {
            memset(nextKeys, 0, sizeof(nextKeys));
            g_clearKeyEvents = SDL_FALSE;
          }
        }

        SDL_RemoveTimer (updateTimer);
	PlayRelease();							/* fermeture générale */
	return 0;
}


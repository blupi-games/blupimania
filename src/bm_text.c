
/* ========= */
/* bm_text.c */
/* ========= */

#include <string.h>


#include "bm.h"
#include "bm_icon.h"






/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Pixmap	pmchar1;			/* pixmap des caractres */
static Pixmap	pmchar2;			/* pixmap des caractres */

static char		*pchaine;			/* pointe chane dite */
static short	lgchaine;			/* longueur de la chane */
static short	lgmax;				/* longueur maximale */
static short	curseur;			/* position du curseur (0..lgchaine) */
static char		ifaccent;			/* accent flottant en cours */
static Rectangle chrect;			/* position et dimensions */
static char		ifcx;				/* 1 -> curseur "|" prsent */
static short	cx;					/* position de curseur "|" en x */
static short	begin;				/* dbut o afficher [car] */
static short	charsize;			/* taille des caractres */







static char tchasselit[128-32] =
{
	0x04,0x04,0x07,0x0A,0x08,0x0A,0x0A,0x04,
	0x05,0x05,0x0A,0x09,0x03,0x07,0x04,0x07,
	0x07,0x07,0x08,0x08,0x07,0x08,0x07,0x07,
	0x08,0x07,0x05,0x05,0x08,0x07,0x08,0x09,
	0x0A,0x07,0x08,0x08,0x08,0x06,0x06,0x08,
	0x08,0x03,0x07,0x08,0x06,0x09,0x08,0x08,
	0x07,0x08,0x07,0x07,0x08,0x07,0x08,0x09,
	0x08,0x07,0x09,0x03,0x03,0x05,0x03,0x0A,
	0x07,0x07,0x07,0x07,0x07,0x06,0x05,0x07,
	0x07,0x03,0x05,0x07,0x03,0x09,0x07,0x07,
	0x07,0x07,0x06,0x05,0x05,0x07,0x07,0x0A,
	0x07,0x07,0x06,0x05,0x03,0x09,0x09,0x07
};

static char tchassemid[128-32] =
{
	0x09,0x08,0x0D,0x0B,0x13,0x18,0x18,0x07,
	0x0C,0x0C,0x0E,0x0E,0x08,0x0E,0x08,0x0C,
	0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
	0x11,0x11,0x08,0x08,0x0E,0x0E,0x0E,0x10,
	0x16,0x17,0x14,0x15,0x16,0x14,0x13,0x17,
	0x18,0x0C,0x11,0x18,0x14,0x1A,0x17,0x15,
	0x13,0x15,0x16,0x12,0x17,0x17,0x17,0x1A,
	0x18,0x17,0x13,0x07,0x07,0x0A,0x08,0x11,
	0x0D,0x11,0x13,0x10,0x13,0x11,0x0E,0x11,
	0x15,0x0B,0x0A,0x15,0x0B,0x1A,0x15,0x12,
	0x13,0x13,0x10,0x0E,0x0D,0x15,0x12,0x1A,
	0x12,0x12,0x10,0x07,0x0B,0x19,0x19,0x0D
};

static char tchacc[32] =
{
	91, 92, 93, 94,
	0, 0, 0, 0, 0, 0, 
	'a', 'a', 'a', 'a',
	'e', 'e', 'e', 'e',
	'i', 'i', 'i', 'i',
	'o', 'o', 'o', 'o',
	'u', 'u', 'u', 'u',
	'c', 'C'
};


/* ------ */
/* LgChar */
/* ------ */

/*
	Retourne la largeur d'un caractre.
 */

short LgChar (char c)
{
	if ( c == 1 )  c = ' ';							/* cadratin ? */
	
	if ( c >= 0 )
	{
		if ( c == '\n' )  c = 127;					/* petit triangle ">" */
		if ( c < 32 )  return 0;
		if ( charsize == TEXTSIZELIT )  return tchasselit[c-32];
		else                            return tchassemid[c-32];
	}
	else
	{
		if ( c > KEYAIGU || c < KEYcCEDILLE )  return 0;
		c = tchacc[-c+KEYAIGU];
		if ( c == 0 )  return 0;
		if ( charsize == TEXTSIZELIT )  return tchasselit[c-32];
		else                            return tchassemid[c-32];
	}
}


static char taccent[] =
{
	KEYAAIGU,   'a', 91, 2,6,
	KEYAGRAVE,  'a', 92, 2,4,
	KEYACIRCON, 'a', 93, 1,3,
	KEYATREMA,  'a', 94, 2,4,
	
	KEYEAIGU,   'e', 91, 2,6,
	KEYEGRAVE,  'e', 92, 2,5,
	KEYECIRCON, 'e', 93, 1,4,
	KEYETREMA,  'e', 94, 2,5,
	
	KEYIAIGU,   124, 91, 0,3,
	KEYIGRAVE,  124, 92, 0,1,
	KEYICIRCON, 124, 93,-2,0,
	KEYITREMA,  124, 94, 0,1,
	
	KEYOAIGU,   'o', 91, 2,6,
	KEYOGRAVE,  'o', 92, 2,5,
	KEYOCIRCON, 'o', 93, 1,4,
	KEYOTREMA,  'o', 94, 2,5,
	
	KEYUAIGU,   'u', 91, 2,8,
	KEYUGRAVE,  'u', 92, 2,6,
	KEYUCIRCON, 'u', 93, 1,5,
	KEYUTREMA,  'u', 94, 2,6,
	
	KEYCCEDILLE,'c', 123, 1,5,
	KEYcCEDILLE,'C', 123, 2,7,
	
	0
};




/* -------- */
/* DrawChar */
/* -------- */

/*
	Dessine un caractre et avance la position pour le suivant.
 */

void DrawChar (Pixmap *ppm, Pt *ppos, char c, ShowMode mode)
{
	Pixmap		*ppmchar;
	Pixmap		pmchar;
	Pt			src, dst, dim;
	short		icon;
	
	if ( c < 32 )  c = 32;
	c -= 32;
	
	dst.x = (*ppos).x;
	dst.y = (*ppos).y - charsize;
	
	if ( charsize == TEXTSIZELIT )
	{
		if ( c%16 < 8 )  ppmchar = &pmchar1;
		else             ppmchar = &pmchar2;
		
		src.x = (c%8)*10;
		src.y = (c/16)*(TEXTSIZELIT+3);
		
		dim.x = LgChar(c+32);
		dim.y = TEXTSIZELIT+3;
	}
	else
	{
		dim.x = LgChar(c+32);
		dim.y = TEXTSIZEMID+5;
		
		if ( c >= 0 && c < 'A'-33 )
		{
			icon = c/3;
			
			src.x = (c%3)*26;
			src.y = 26*0;
		}
		
		if ( c >= 'A'-33 && c < 'a'-33 )
		{
			c -= 'A'-33;
			icon = c/3;
			
			src.x = (c%3)*26;
			src.y = 26*1;
		}
		
		if ( c >= 'a'-33 )
		{
			c -= 'a'-33;
			icon = c/3;
			
			src.x = (c%3)*26;
			src.y = 26*2;
		}
		
		GetIcon(&pmchar, ICO_CHAR_MID+icon, 1);
		ppmchar = &pmchar;
	}
	CopyPixel(ppmchar, src, ppm, dst, dim, mode);
	
	(*ppos).x += dim.x;
}


/* ---------- */
/* DrawAccent */
/* ---------- */

/*
	Dessine un caractre accentu ou autre.
 */

void DrawAccent (Pixmap *ppm, Pt *ppos, char c, ShowMode mode)
{
	char		*paccent;
	ShowMode	m;
	Pt			pnext, pacc;
	
	if ( c < 0 )				/* lettre accentue ? */
	{
		paccent = taccent;
		while ( *paccent != 0 )
		{
			if ( paccent[0] == c )
			{
				pnext = *ppos;
				pacc  = *ppos;
				DrawChar(ppm, &pnext, paccent[1], mode);	/* dessine la lettre sous l'accent */
				m = mode;
				if ( mode == MODELOAD )  m = MODEOR;
				if ( charsize == TEXTSIZELIT )  pacc.x += paccent[3];
				else                            pacc.x += paccent[4];
				DrawChar(ppm, &pacc, paccent[2], m);		/* dessine l'accent flottant */
				*ppos = pnext;
				break;
			}
			paccent += 5;
		}
	}
	else
	{
		if ( c == '\n' )  c = 127;					/* petit triangle ">" */
		DrawChar(ppm, ppos, c, mode);				/* dessine le caractre */
	}
}


/* ======== */
/* DrawText */
/* ======== */

/*
	Dessine une chane de caractres termine par zro.
	Retourne la position suivante, sur la ligne de base.
		*ppm		->	pixmap o dessiner (0 = cran)
		pos			->	dpart sur la ligne de base
		pstring		->	chane termine par zro
		size		->	taille des caractres
		ShowMode	->	mode de dessin
 */

Pt DrawText (Pixmap *ppm, Pt pos, char *pstring, short size, ShowMode mode)
{
	char		c;
	
	charsize = size;
	
	if ( size == TEXTSIZELIT )
	{
		GetIcon(&pmchar1, ICO_CHAR_LIT+0, 1);
		GetIcon(&pmchar2, ICO_CHAR_LIT+1, 1);
	}
	
	while ( (c = *pstring++, c != 0) )
	{
		DrawAccent(ppm, &pos, c, mode);				/* dessine le caractre */
	}
	
	return pos;
}


/* =========== */
/* GetRectText */
/* =========== */

/*
	Retourne le rectangle englobant une chane de caractres.
	N'affiche rien du tout.
		pos			->	dpart sur la ligne de base
		pstring		->	chane termine par zro
		size		->	taille des caractres
 */

Rectangle GetRectText (Pt pos, char *pstring, short size)
{
	Rectangle	rect;
	char		c;
	short		lg = 0;
	
	charsize = size;
	
	while ( (c = *pstring++, c != 0) )
	{
		lg += LgChar(c);
	}
	
	rect.p1.x = pos.x;
	rect.p1.y = pos.y - charsize;
	rect.p2.x = pos.x + lg;
	if ( size == TEXTSIZELIT )  rect.p2.y = pos.y + 3;
	else                        rect.p2.y = pos.y + 5;
	
	return rect;
}



/* ------- */
/* GetWord */
/* ------- */

/*
	Donne les caractristiques du mot suivant.
	Retourne la longueur  utiliser.
 */

short GetWord (char **ppnext, char *pword)
{
	char		*pt;
	Pt			pos;
	Rectangle	rect;
	
	pt = pword;
	while ( **ppnext != 0 && **ppnext != '\n' && **ppnext != ' ' && **ppnext != '-' )
	{
		*pt++ = *(*ppnext)++;
	}
	if ( **ppnext == ' ' || **ppnext == '-' )
	{
		*pt++ = *(*ppnext)++;
	}
	*pt = 0;
	
	pos.x = 0;
	pos.y = 0;
	rect = GetRectText(pos, pword, charsize);
	return rect.p2.x;
}


/* ============= */
/* DrawParagraph */
/* ============= */

/*
	Dessine un paragraphe de texte dans un rectangle, en drapeau  droite.
 */

void DrawParagraph (Pixmap *ppm, Rectangle rect, char *pstring, short size, ShowMode mode)
{
	Pt		pos;
	char	word[50];
	char	*pnext;
	short	lg, under;
	
	charsize = size;
	
	if ( size == TEXTSIZELIT )  under = 3;
	else                        under = 5;
	
	pos.y = rect.p1.y + size;
	do
	{
		pos.x = rect.p1.x;
		do
		{
			pnext = pstring;
			lg = GetWord(&pnext, word);
			if ( pos.x+lg <= rect.p2.x )
			{
				DrawText(ppm, pos, word, size, mode);		/* affiche un mot */
				pstring = pnext;
			}
			pos.x += lg;
		}
		while ( pos.x <= rect.p2.x && *pstring != 0 && *pstring != '\n' );
		if ( *pstring == '\n' )  pstring ++;
		if ( size == TEXTSIZELIT )  pos.y += size*2;
		else                        pos.y += size+10;
	}
	while ( pos.y < rect.p2.y-under && *pstring != 0 );
	
	if ( *pstring != 0 )						/* texte pas entirement affich ? */
	{
		pos.x = rect.p2.x - LgChar(126);
		pos.y = rect.p2.y - under;
		DrawChar(ppm, &pos, 126, MODELOAD);		/* affiche petit triangle v */
	}
}





/* ---------- */
/* CalcJustif */
/* ---------- */

/*
	Effectue quelques calculs pour afficher une ligne en dition.
 */

void CalcJustif (void)
{
	short		dim;			/* largeur pour la ligne */
	short		i;				/* offset temporaire */
	short		length;
	
	dim = chrect.p2.x - chrect.p1.x - 10;
	cx  = chrect.p1.x;						/* curseur "|" au dbut */
	
	/*	Cherche dans la chane le premier caractre  afficher. */
	
	i = curseur;
	length = 0;
	while ( i < lgchaine && length < dim/2 )
	{
		length += LgChar(pchaine[i]);
		i ++;								/* avance de dim/2 */
	}
	if ( length > dim/2 )  i --;
	
	length = 0;
	while ( i > 0 && length < dim-1 )
	{
		i --;								/* recule de dim-1 */
		length += LgChar(pchaine[i]);
	}
	if ( length > dim-1 )  i ++;
	begin = i;
	
	/*	Cherche le nombre de caractres et la longueur  afficher. */
	
	length = 0;
	while ( i < lgchaine && length < dim )
	{
		if ( i == curseur )  cx = chrect.p1.x + length;		/* curseur "|" ici */
		length += LgChar(pchaine[i]);
		i ++;
	}
	if ( length > dim )
	{
		length -= LgChar(pchaine[i-1]);
		i --;
	}
	if ( i == curseur )  cx = chrect.p1.x + length;			/* curseur "|" ici */
}


/* ------- */
/* EditAff */
/* ------- */

/*
	Affiche un texte pendant l'dition d'une ligne.
 */

void EditAff (void)
{
	short		i;
	Pt			pos;
	Rectangle	rect;
	
	GetIcon(&pmchar1, ICO_CHAR_LIT+0, 1);
	GetIcon(&pmchar2, ICO_CHAR_LIT+1, 1);
	
	pos.x = chrect.p1.x;
	pos.y = chrect.p1.y + (chrect.p2.y-chrect.p1.y)/2 + charsize/2 - 1;
	
	for ( i=begin ; i<lgchaine ; i++ )
	{
		if ( pos.x >= chrect.p2.x-10 )  break;
		DrawAccent(0, &pos, pchaine[i], MODELOAD);		/* affiche un caractre */
	}
	
	if ( pos.x < chrect.p2.x )
	{
		rect = chrect;
		rect.p1.x = pos.x;
		DrawFillRect(0, rect, MODELOAD, COLORBLANC);	/* efface la fin de la ligne */
	}
	
	if ( begin > 0 )
	{
		pos.x = chrect.p1.x;
		DrawChar(0, &pos, 96, MODELOAD);				/* met le triangle < */
	}
	
	if ( pchaine[i] != 0 )
	{
		pos.x = chrect.p2.x - LgChar(127);
		DrawChar(0, &pos, 127, MODELOAD);				/* met le triangle > */
	}
}


/* ------- */
/* InsChar */
/* ------- */

/*
	Insre un caractre dans la ligne.
 */

short InsChar (char car)
{
	char		*s, *d;
	short		i;
	
	if ( lgchaine >= lgmax-1 )  return -1;	/* chane pleine */
	
	s = pchaine+lgchaine;
	d = pchaine+lgchaine+1;
	for( i=curseur ; i<lgchaine+1 ; i++ )
	{
		*d-- = *s--;						/* creuse le trou */
	}
	
	*(pchaine+curseur) = car;				/* met le caractre dans la chane */
	lgchaine ++;
	curseur ++;								/* avance le curseur */
	
	return 0;
}


/* ------- */
/* DelChar */
/* ------- */

/*
	Dtruit un caractre dans la ligne.
 */

short DelChar (void)
{
	char		*s, *d;
	short		i;
	
	if ( lgchaine == 0 )  return -1;		/* chane vide */
	if ( curseur == 0 )  return -1;			/* curseur au dbut */
	
	s = pchaine+curseur;
	d = pchaine+curseur-1;
	for( i=curseur ; i<lgchaine+1 ; i++ )
	{
		*d++ = *s++;						/* bouche le trou */
	}
	
	lgchaine --;
	curseur --;								/* recule le curseur */
	
	return 0;
}


/* ----------- */
/* AccentFirst */
/* ----------- */

/*
	Cherche si la touche est un accent flottant.
	Si oui, retourne l'accent. Si non, retourne zro.
 */

short AccentFirst (char key)
{
	short		i = 0;
	
	static char table[] =
	{
		KEYAIGU,	91,
		KEYGRAVE,	92,
		KEYCIRCON,	93,
		KEYTREMA,	94,
		0
	};
	
	while ( table[i] != 0 )
	{
		if ( table[i] == key )  return table[i+1];
		i += 2;
	}
	
	return 0;
}


/* ----------- */
/* AccentUnder */
/* ----------- */

/*
	Cherche si une touche peut se placer sous un accent.
	Si oui, retourne le nouveau caractre. Si non, retourne zro.
 */

char AccentUnder (char key, char accent)
{
	short		i = 0;
	
	static char table[] =
	{
		91,	'a',	KEYAAIGU,
		92,	'a',	KEYAGRAVE,
		93,	'a',	KEYACIRCON,
		94,	'a',	KEYATREMA,
		91,	'e',	KEYEAIGU,
		92,	'e',	KEYEGRAVE,
		93,	'e',	KEYECIRCON,
		94,	'e',	KEYETREMA,
		91,	'i',	KEYIAIGU,
		92,	'i',	KEYIGRAVE,
		93,	'i',	KEYICIRCON,
		94,	'i',	KEYITREMA,
		91,	'o',	KEYOAIGU,
		92,	'o',	KEYOGRAVE,
		93,	'o',	KEYOCIRCON,
		94,	'o',	KEYOTREMA,
		91,	'u',	KEYUAIGU,
		92,	'u',	KEYUGRAVE,
		93,	'u',	KEYUCIRCON,
		94,	'u',	KEYUTREMA,
		0
	};
	
	while ( table[i] != 0 )
	{
		if ( accent == table[i] && key == table[i+1] )  return table[i+2];
		i += 3;
	}
	
	return 0;
}



/* ------- */
/* InvCurs */
/* ------- */

/*
	Inverse le curseur "|".
 */

void InvCurs (void)
{
	Pt		p1, p2;
	
	if ( cx == 0 )  return;
	
	p1.x = cx - 1;
	p1.y = chrect.p1.y;
	p2.x = cx - 1;
	p2.y = chrect.p2.y;
	
	DrawLine(0, p1, p2, MODEXOR, COLORNOIR);	/* inverse la droite verticale */
}

/* ------- */
/* ClrCurs */
/* ------- */

/*
	Enlve le curseur "|" si ncessaire.
 */

void ClrCurs (void)
{
	if ( ifcx == 0 )  return;
	
	InvCurs();					/* efface le curseur */
	ifcx = 0;
}

/* ------- */
/* SetCurs */
/* ------- */

/*
	Met le curseur "|" si ncessaire.
 */

void SetCurs (void)
{
	if ( ifcx != 0 )  return;
	
	InvCurs();					/* allume le curseur */
	ifcx = 1;
}



/* -------- */
/* EditDraw */
/* -------- */

/*
	Affiche la chane dite.
 */

void EditDraw (void)
{
	if ( pchaine == 0 )  return;
	
	ClrCurs();					/* enlve le curseur "|" */
	CalcJustif();				/* fait qq calculs */
	EditAff();					/* affiche la ligne */
	SetCurs();					/* met le curseur "|" */
}



/* ========= */
/* EditEvent */
/* ========= */

/*
	Modifie la chane en dition selon un vnement clavier/souris.
	Retourne -1 en cas d'erreur.
	Retourne 1 si l'vnement ne modifie pas le contenu.
	Retourne 0 si l'vnement modifie le contenu.
 */

short EditEvent (short key, Pt pos)
{
	short		err = -1;
	
	if ( pchaine == 0 )  return 1;
	
	if ( key == 0 )  return -1;
	
	if ( key == KEYLEFT )
	{
		if ( curseur > 0 )
		{
			curseur --;
			err = 1;
		}
	}
	
	if ( key == KEYRIGHT )
	{
		if ( curseur < lgchaine )
		{
			curseur ++;
			err = 1;
		}
	}
	
	if ( key == KEYUP )
	{
		curseur = 0;
		err = 1;
	}
	
	if ( key == KEYDOWN )
	{
		curseur = lgchaine;
		err = 1;
	}
	
	if ( key == KEYDEL )				/* touche DEL normale */
	{
		err = DelChar();				/* dtruit un caractre */
	}
	
	if ( ifaccent )						/* accent flottant en cours ? */
	{
		ifaccent = AccentUnder(key, ifaccent);
		if ( ifaccent )
		{
			DelChar();					/* supprime l'accent flottant */
			key = ifaccent;				/* remplace par lettre accentue */
			goto ins;
		}
	}
	ifaccent = 	AccentFirst(key);
	if ( ifaccent )
	{
		key = ifaccent;					/* insre l'accent flottant */
	}
	
	ins:
	if ( key == KEYRETURN )  key = '\n';
	
	if ( key >= 32 && key <= 127 ||
		 key <= KEYAAIGU && key >= KEYcCEDILLE ||
		 key == '\n' )
	{
		err = InsChar((char)key);		/* insre le caractre frapp */
	}
	
	if ( err >= 0 )
	{
		EditDraw();						/* affiche la ligne */
	}
	
	return err;
}


/* ======== */
/* EditOpen */
/* ======== */

/*
	Initialise la chane  diter, et place le curseur  la fin.
 */

short EditOpen (char *p, short max, Rectangle rect)
{
    pchaine  = p;
	lgchaine = strlen(p);
	lgmax    = max;
	chrect   = rect;
	ifaccent = 0;
	ifcx     = 0;
	charsize = TEXTSIZELIT;
	
	curseur = lgchaine;
	
	EditDraw();				/* affiche la chane */
	return 0;
}


/* ========= */
/* EditClose */
/* ========= */

/*
	Termine la chane  diter, et efface le curseur.
 */

short EditClose (void)
{
	if ( pchaine == 0 )  return 1;
	
	curseur = 0;					/* met le curseur au dbut */
	
	EditDraw();						/* affiche la chane */
	ClrCurs();						/* enlve le curseur "|" */
	
	pchaine = 0;					/* y'a plus de chane en dition */
	
	return 0;
}


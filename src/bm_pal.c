
/* ======== */
/* bm_pal.c */
/* ======== */

#include <stdio.h>
#include <string.h>

#include "bm.h"
#include "bm_icon.h"
#include "actions.h"




#define MAXICONY	4							/* nb max d'icnes dans la palette */
#define MAXICONX	(15+1)						/* nb max par sous-palette */

#define MAXEDITY	4
#define MAXEDITX	12



/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static short	ticon[MAXICONY][MAXICONX];		/* icnes des boutons */
static short	trest[MAXICONY][MAXICONX];		/* nb disponible */
static short	tspal[MAXICONY];				/* tats des sous-palettes (0..MAXICONX-1) */
static short	press;							/* bouton press (0..MAXICONY-1) */
static short	typepress;						/* 1 -> palette avec bouton press */

static short	tediticon[MAXEDITY][MAXEDITX];	/* dition palette: icnes */
static short	teditetat[MAXEDITY][MAXEDITX];	/* dition palette: tats */
static short	teditrest[MAXEDITY][MAXEDITX];	/* dition palette: restes */

static short	laststatus;
static short	lastforce;
static short	lastvision;
static short	lastmechant;
static short	lastmagic;
static short	lastcles;



typedef struct
{
	short	typepress;						/* 1 -> palette avec bouton press */
	short	reserve[10];					/* rserve */
}
Partie;





/* ------------ */
/* GetButtonPos */
/* ------------ */

/*
	Donne la position d'un bouton.
 */

Pt GetButtonPos (short rang)
{
	Pt		pos;

	if ( rang < 0 || rang >= MAXICONY )
	{
		pos.x = -1;
		pos.y = -1;
		return pos;
	}

	pos.x = POSXPALETTE;
	pos.y = POSYPALETTE + rang*(LYICO/2+4);
	return pos;
}


/* ------------- */
/* GetButtonRang */
/* ------------- */

/*
	Donne le rang du bouton cliqu par la souris.
	Retourne -1 si la souris n'est pas sur un bouton.
 */

short GetButtonRang (Pt pos)
{
	short		i;
	Pt			pb;

	if ( pos.x < POSXPALETTE ||
		 pos.x > POSXPALETTE+DIMXPALETTE )  return -1;

	for ( i=0 ; i<MAXICONY ; i++ )
	{
		if ( ticon[i][0] == 0 )  continue;

		pb = GetButtonPos(i);
		if ( pos.y >= pb.y &&
			 pos.y <= pb.y+LYICO/2 )  return i;
	}
	return -1;
}


/* ---------- */
/* DrawButton */
/* ---------- */

/*
	Dessine un bouton.
	state = 0	->	bouton relch normal
	state = 1	->	bouton press
	state = 2	->	flche ">" pour sous-palette
	state = 3	->	bouton relch pour action joueur
	state = 4	->	bouton press pour action joueur
	state = 5	->	flche ">" pour sous-palette action joueur
 */

void DrawButton (Pt pos, short icon, short state)
{
	Pixmap		pm;
	short		iconbutton;
	Pt			src, dim, p;

	if ( pos.x < 0 )  return;

	src.x = 0;
	src.y = 0;
	dim.x = LXICO/2;
	dim.y = LYICO/2;

	if ( state == 0 )  iconbutton = ICO_BUTTON_REL;
	if ( state == 1 )  iconbutton = ICO_BUTTON_PRESS;
	if ( state == 2 )  iconbutton = ICO_BUTTON_SPAL;

	if ( state == 3 || state == 4 )
	{
		iconbutton = ICO_BUTTON_BUILD;
		dim.x = 52;
		if ( state == 4 )  src.y = LYICO/2;
	}

	if ( state == 5 )
	{
		iconbutton = ICO_BUTTON_BUILDSP;
		dim.x = 52;
	}

	GetIcon(&pm, iconbutton+ICOMOFF, 1);
	CopyPixel							/* efface le fond du bouton */
	(
		&pm, src,
		0, pos,
		dim, MODEAND
	);

	if ( icon == 0 )  return;

	GetIcon(&pm, iconbutton, 1);
	CopyPixel							/* dessine le cadre du bouton */
	(
		&pm, src,
		0, pos,
		dim, MODEOR
	);

	if ( icon == 1 )  return;

	if ( state == 0 || state == 3 )
	{
		pos.x += 3;						/* dcalage si bouton relch */
		pos.y += 7;
	}
	else
	{
		pos.x += 7;						/* dcalage si bouton press */
		pos.y += 2;
	}

	GetIcon(&pm, icon+ICOMOFF, 1);
	CopyPixel							/* efface le contenu du bouton */
	(
		&pm, (p.y=1, p.x=1, p),
		0, pos,
		(p.y=30, p.x=30, p), MODEAND
	);

	GetIcon(&pm, icon, 1);
	CopyPixel							/* dessine le contenu du bouton */
	(
		&pm, (p.y=1, p.x=1, p),
		0, pos,
		(p.y=30, p.x=30, p), MODEOR
	);
}


/* ---------- */
/* DrawF1toF4 */
/* ---------- */

/*
	Dessine le nom de la touche raccourci.
 */

void DrawF1toF4 (short rang)
{
	Pixmap		pm;
	Pt			src, dst, dim;

	if ( typepress != 0 )  return;
	if ( rang < 0 || rang >= MAXICONY )  return;

	dst = GetButtonPos(rang);
	dst.x += 5;
	dst.y += LYICO/2 - 7-1;

	src.x = 8*rang;
	src.y = LYICO/2 - 7;

	dim.x = 8;
	dim.y = 7;

	GetIcon(&pm, ICO_BUTTON_PAUSE+ICOMOFF, 1);
	CopyPixel(&pm, src, 0, dst, dim, MODEAND);

	GetIcon(&pm, ICO_BUTTON_PAUSE, 1);
	CopyPixel(&pm, src, 0, dst, dim, MODEOR);
}


/* --------- */
/* DrawReste */
/* --------- */

/*
	Affiche le nb restant  ct d'un bouton.
 */

void DrawReste (Pt pos, short rest)
{
	Rectangle	rect;
	char		chaine[4];

	if ( pos.x < 0 )  return;
	pos.x += LXICO/2+1;
	pos.y += 1;

	rect.p1.x = pos.x;
	rect.p1.y = pos.y;
	rect.p2.x = pos.x+14;
	rect.p2.y = pos.y+13;
	DrawFillRect(0, rect, MODELOAD, COLORBLANC);	/* efface l'emplacement */

	if ( rest >= 999 )  return;

	if ( rest > 99 )  rest = 99;		/* 2 digits au maximum */
	if ( rest < 10 )
	{
		pos.x += 4;
		chaine[0] = rest+'0';			/* units */
		chaine[1] = 0;
	}
	else
	{
		chaine[0] = rest/10+'0';		/* dizaines */
		chaine[1] = rest%10+'0';		/* units */
		chaine[2] = 0;
	}

	pos.y += TEXTSIZELIT;
	DrawText(0, pos, chaine, TEXTSIZELIT, MODEOR);
}


/* ------------ */
/* DrawTriangle */
/* ------------ */

/*
	Dessine le petit triangle en bas  ct de l'icne, pour indiquer
	la prsence d'une sous-palette.
 */

void DrawTriangle (short rang)
{
	Pt			pos;
	Rectangle	rect;
	char		chaine[2];

	pos = GetButtonPos(rang);
	if ( pos.x < 0 )  return;

	pos.x += LXICO/2+4;
	pos.y += LYICO/2-15;

	rect.p1.x = pos.x;
	rect.p1.y = pos.y;
	rect.p2.x = pos.x+TEXTSIZELIT;
	rect.p2.y = pos.y+TEXTSIZELIT+3;
	DrawFillRect(0, rect, MODELOAD, COLORBLANC);	/* efface l'emplacement */

	if ( ticon[rang][1] == 0 )  return;				/* pas de sous-palette */

	chaine[0] = 127;								/* code du petit triangle */
	chaine[1] = 0;
	pos.y += TEXTSIZELIT;
	DrawText(0, pos, chaine, TEXTSIZELIT, MODEOR);	/* dessine le petit triangle */
}


/* ----------- */
/* PaletteDraw */
/* ----------- */

/*
	Dessine la palette d'icnes.
 */

void PaletteDraw (void)
{
	Rectangle	rect;
	short		rang, state;

	rect.p1.x = 6;
	rect.p1.y = LYIMAGE-1-332;
	rect.p2.x = 6+55;
	rect.p2.y = LYIMAGE-1-332+173;
	DrawFillRect(0, rect, MODELOAD, COLORBLANC);	/* efface l'emplacement */

	for ( rang=0 ; rang<MAXICONY ; rang++ )
	{
		if ( ticon[rang][tspal[rang]] == 0 )
		{
			state = 0;
			if ( typepress == 0 )  state = 3;	/* bouton pour le joueur */
			DrawButton(GetButtonPos(rang), 0, state);
			DrawReste(GetButtonPos(rang), 999);
		}
		else
		{
			state = 0;
			if ( rang == press  )  state = 1;	/* bouton enfonc */
			if ( typepress == 0 )  state = 3;	/* bouton pour le joueur */
			DrawButton(GetButtonPos(rang), ticon[rang][tspal[rang]], state);
			DrawF1toF4(rang);
			DrawReste(GetButtonPos(rang), trest[rang][tspal[rang]]);
			DrawTriangle(rang);
		}
	}
}



/* ------------- */
/* GetSButtonPos */
/* ------------- */

/*
	Donne la position d'un bouton d'une sous-palette.
 */

Pt GetSButtonPos (short rang, short srang)
{
	Pt		pos;

	if ( srang < 0 || srang > MAXICONX ||
		 ticon[rang][srang] == 0 )
	{
		pos.x = -1;
		pos.y = -1;
		return pos;
	}

	pos = GetButtonPos(rang);
	pos.x += LXICO/2+12+4 + srang*(LXICO/2+4);
	return pos;
}


/* -------------- */
/* GetSButtonRang */
/* -------------- */

/*
	Donne le sous-rang du bouton cliqu par la souris.
	Retourne -1 si la souris n'est pas sur un sous-bouton.
 */

short GetSButtonRang (short rang, Pt pos)
{
	short		i;
	Pt			pb;

	pb = GetButtonPos(rang);
	if ( pos.y < pb.y ||
		 pos.y > pb.y+LYICO/2 )  return -1;

	for ( i=0 ; i<MAXICONX ; i++ )
	{
		if ( ticon[rang][i] == 0 )  continue;

		pb = GetSButtonPos(rang, i);
		if ( pos.x >= pb.x &&
			 pos.x <= pb.x+LXICO/2 )  return i;
	}
	return -1;
}


/* ----------------- */
/* SPaletteGetPosDim */
/* ----------------- */

/*
	Donne la position et les dimensions d'une sous-palette
 */

void SPaletteGetPosDim (short rang, Pt *ppos, Pt *pdim)
{
	short		i;

	*ppos = GetButtonPos(rang);
	(*ppos).x += LXICO/2+12;
	(*ppos).y -= 4;

	(*pdim).x = 4;
	for ( i=0 ; i<MAXICONX ; i++ )
	{
		if ( ticon[rang][i] == 0 )  break;
		(*pdim).x += LXICO/2+4;
	}
	(*pdim).y = 4+LYICO/2+4;
}


/* ------------ */
/* SPaletteOpen */
/* ------------ */

/*
	Dessine une sous-palette.
 */

short SPaletteOpen (short rang, Pixmap *ppm)
{
	Pt			pos, dim;
	Pt			p;
	Rectangle	r;
	short		i, state;

	SPaletteGetPosDim(rang, &pos, &dim);

	if ( GetPixmap(ppm, dim, 0, 2) != 0 )  return 1;

	CopyPixel(0, pos, ppm, (p.y=0, p.x=0, p), dim, MODELOAD);	/* sauve l'cran */

	DrawFillRect
	(
		0, (r.p1.x=pos.x, r.p1.y=pos.y, r.p2.x=pos.x+dim.x, r.p2.y=pos.y+dim.y, r),
		MODELOAD, COLORBLANC
	);
	DrawRect
	(
		0, (r.p1.x=pos.x, r.p1.y=pos.y, r.p2.x=pos.x+dim.x-1, r.p2.y=pos.y+dim.y-1, r),
		MODELOAD, COLORNOIR
	);

	for ( i=0 ; i<MAXICONX ; i++ )
	{
		if ( ticon[rang][i] == 0 )  break;

		state = 0;
		if ( i == tspal[rang] )  state = 1;
		DrawButton(GetSButtonPos(rang, i), ticon[rang][i], state);
	}
	return 0;
}


/* ------------- */
/* SPaletteClose */
/* ------------- */

/*
	Ferme une sous-palette.
 */

void SPaletteClose (short rang, Pixmap *ppm)
{
	Pt		pos, dim;
	Pt		p;

	SPaletteGetPosDim(rang, &pos, &dim);

	CopyPixel(ppm, (p.y=0, p.x=0, p), 0, pos, dim, MODELOAD);	/* restitue l'cran */
	GivePixmap(ppm);
}


/* ---------------- */
/* SPaletteTracking */
/* ---------------- */

/*
	Ouvre la sous-palette, choix d'un bouton, puis ferme-la.
	Retourne 1 si le bouton a t relch.
	Retourne 0 si la souris s'est dplace vers la gauche.
 */

short SPaletteTracking (short rang)
{
	Pixmap		pmsave = {0,0,0,0,0,0,0};
	Pt			pos, limit;
	short		old, new;
	short		key;
	short		type;

	if ( SPaletteOpen(rang, &pmsave) != 0 )  return 0;

	if ( typepress )  type = 2;
	else              type = 5;
	DrawButton(GetButtonPos(rang), 1, type);		/* met la flche -> */

	limit = GetButtonPos(rang);
	limit.x += LXICO/2 + 12;

	old = tspal[rang];
	while ( 1 )
	{
		key = GetEvent(&pos);
		if ( key == KEYCLICREL )  break;

		if ( pos.y >= limit.y && pos.y <= limit.y+LYICO/2 )
		{
			if ( pos.x < limit.x - 12 )  break;
		}
		else
		{
			if ( pos.x < limit.x )  break;
		}

		new = GetSButtonRang(rang, pos);
		if ( new != old && new >= 0 )
		{
			DrawButton(GetSButtonPos(rang, old), ticon[rang][old], 0);
			old = new;
			DrawButton(GetSButtonPos(rang, old), ticon[rang][old], 1);
		}
	}
	if ( old >= 0 )  tspal[rang] = old;

	SPaletteClose(rang, &pmsave);

	if ( typepress )  type = 1;
	else              type = 3;
	DrawButton(GetButtonPos(rang), ticon[rang][tspal[rang]], type);
	DrawF1toF4(rang);
	DrawReste(GetButtonPos(rang), trest[rang][tspal[rang]]);	/* affiche le nouveau reste */

	if ( key == KEYCLICREL )  return 1;
	return 0;
}



/* -------------- */
/* SupprimeBouton */
/* -------------- */

/*
	Supprime un bouton dont le compteur d'utilisation est arriv  zro.
 */

void SupprimeBouton (short rang, short spal)
{
	short		i;

	for ( i=spal ; i<MAXICONX ; i++ )
	{
		ticon[rang][i] = ticon[rang][i+1];
		trest[rang][i] = trest[rang][i+1];
	}
	ticon[rang][MAXICONX-1] = 0;
	trest[rang][MAXICONX-1] = 0;
}


/* ============= */
/* PaletteUseObj */
/* ============= */

/*
	Dcrmente le compteur d'utilisation d'un objet.
	Si le bouton correspondant  l'objet tait enfonc et que le compteur
	arrive  zro, enfonce un autre bouton automatiquement.
 */

void PaletteUseObj (short icon)
{
	short		rang, spal, type;

	for ( rang=0 ; rang<MAXICONY ; rang++ )
	{
		for ( spal=0 ; spal<MAXICONX ; spal++ )
		{
			if ( icon == ticon[rang][spal] )  goto dec;
		}
	}
	return;

	dec:
	if ( trest[rang][spal] == 0 || trest[rang][spal] >= 999 )  return;

	trest[rang][spal] --;
	DrawReste(GetButtonPos(rang), trest[rang][tspal[rang]]);	/* affiche le nouveau reste */

	if ( trest[rang][spal] > 0 )  return;

	/*	Si le bouton ne peut plus tre utilis et qu'il tait visible
		et press, cherche un autre en remplacement. */

	if ( typepress )  type = 1;
	else              type = 3;

	if ( spal == tspal[rang] )
	{
		DrawButton(GetButtonPos(rang), 0, type);	/* efface le bouton */
	}
	SupprimeBouton(rang, spal);						/* supprime le bouton dans les tables */
	DrawTriangle(rang);								/* enlve v. le petit triangle */

	if ( spal != tspal[rang] )  return;
	if ( rang != press )        return;

	for ( spal=0 ; spal<MAXICONX ; spal++ )
	{
		if ( trest[rang][spal] > 0 )
		{
			tspal[rang] = spal;
			DrawButton(GetButtonPos(rang), ticon[rang][tspal[rang]], type);
			DrawF1toF4(rang);
			DrawReste(GetButtonPos(rang), trest[rang][tspal[rang]]);
			return;
		}
	}
	for ( rang=0 ; rang<MAXICONY ; rang++ )
	{
		for ( spal=0 ; spal<MAXICONX ; spal++ )
		{
			if ( trest[rang][spal] > 0 )
			{
				press = rang;
				tspal[rang] = spal;
				DrawButton(GetButtonPos(rang), ticon[rang][tspal[rang]], type);
				DrawF1toF4(rang);
				DrawReste(GetButtonPos(rang), trest[rang][tspal[rang]]);
				return;
			}
		}
	}
	press = 0;
}


/* ============= */
/* PaletteStatus */
/* ============= */

/*
	Donne l'tat d'un bouton de la palette.
 */

short PaletteStatus (short rang)
{
	return tspal[rang];
}


/* =============== */
/* PaletteGetPress */
/* =============== */

/*
	Donne l'icne presse dans la palette.
 */

short PaletteGetPress (void)
{
	if ( trest[press][tspal[press]] == 0 )  return -1;
	return ticon[press][tspal[press]];
}


/* ---------- */
/* SpecButton */
/* ---------- */

/*
	Retourne un bouton spcial cliqu.
 */

short SpecButton (Pt pos)
{
	short		*pt;

	static short table0[] =				/* table si jeu avec flches */
	{
		25,92,17,17,	KEYUP,
		7,75,17,17,		KEYLEFT,
		43,75,17,17,	KEYRIGHT,
		25,58,17,17,	KEYDOWN,
		29,71,9,9,		KEYCENTER,

		6,28,18,18,		KEYHOME,
		26,28,18,18,	KEYPAUSE,
		46,27,16,16,	KEYIO,
		0
	};

	static short table1[] =				/* table si jeu avec tlcommande */
	{
		7,68,24,9,		KEYGOFRONT,
		7,59,24,9,		KEYGOBACK,
		36,76,8,26,		KEYGOLEFT,
		44,76,8,26,		KEYGORIGHT,

		6,28,18,18,		KEYHOME,
		26,28,18,18,	KEYPAUSE,
		46,27,16,16,	KEYIO,
		0
	};

	if ( typejeu == 0 || pause )  pt = table0;
	else                          pt = table1;

	while ( *pt != 0 )
	{
		if ( pos.x >= pt[0] &&
			 pos.x <= pt[0]+pt[2] &&
			 pos.y >= LYIMAGE-pt[1] &&
			 pos.y <= LYIMAGE-pt[1]+pt[3] )  return pt[4];
		pt += 5;
	}

	return 0;
}

/* ============ */
/* PaletteEvent */
/* ============ */

/*
	Ventilation d'un vnement pour la palette.
	Retourne un vnement (KEY*) lors d'une action directe.
	Retourne 1 si l'vnement n'tait pas pour la palette.
	Retourne 0 si l'vnement a t trait.
 */

short PaletteEvent (short event, Pt pos)
{
	short		init, rang, xlimit;
	Pt			pb;
	short		key;
	short		typep, typer;

	if ( typepress == 0 && event >= KEYF4 && event <= KEYF1 )
	{
		rang = -event+KEYF1;
		if ( ticon[rang][tspal[rang]] )
		{
			press = rang;
			return 0;
		}
	}

	if ( event != KEYCLIC )  return 1;

	rang = SpecButton(pos);
	if ( rang != 0 )  return rang;

	rang = GetButtonRang(pos);
	if ( rang == -1 )  return 1;

	PlaySound(SOUND_CLIC);

	if ( typepress )
	{
		typep = 1;		/* bouton press */
		typer = 0;		/* bouton relch */
	}
	else
	{
		typep = 4;		/* bouton press */
		typer = 3;		/* bouton relch */
	}

	init = press;
	DrawButton(GetButtonPos(press), ticon[press][tspal[press]], typer);
	DrawF1toF4(rang);
	press = rang;
	DrawButton(GetButtonPos(press), ticon[press][tspal[press]], typep);

	pb = GetButtonPos(0);
	xlimit = pb.x + LXICO/2;

	while ( 1 )
	{
		key = GetEvent(&pos);
		if ( key == KEYCLICREL )  break;

		if ( pos.x > xlimit &&
			 rang != -1 &&
			 rang < MAXICONY &&
			 ticon[rang][1] != 0 )
		{
			press = rang;
			if ( SPaletteTracking(rang) )			/* action dans sous-palette ... */
			{
				if ( typepress )  return 0;
				return 1;
			}
			key = GetEvent(&pos);
			if ( key == KEYCLICREL )  break;
		}

		rang = GetButtonRang(pos);
		if ( rang != press && rang < MAXICONY )
		{
			DrawButton(GetButtonPos(press), ticon[press][tspal[press]], typer);
			press = rang;
			DrawButton(GetButtonPos(press), ticon[press][tspal[press]], typep);
		}
	}

	if ( typepress == 0 )
	{
		DrawButton(GetButtonPos(press), ticon[press][tspal[press]], typer);
		DrawF1toF4(rang);
	}
	return 0;
}


/* ========== */
/* PaletteNew */
/* ========== */

/*
	Initialise et dessine une nouvelle palette d'icnes.
 */

void PaletteNew (short *pdesc, short type)
{
	short		rang, spal;

	typepress = type;

	for ( rang=0 ; rang<MAXICONY ; rang++ )
	{
		for ( spal=0 ; spal<MAXICONX ; spal++ )
		{
			if ( *pdesc == 0 || *pdesc == -1 )
			{
				ticon[rang][spal] = 0;
				trest[rang][spal] = 0;
			}
			else
			{
				ticon[rang][spal] = *pdesc++;
				trest[rang][spal] = *pdesc++;
			}
		}
		if ( *pdesc == -1 )  pdesc++;
		tspal[rang] = 0;
	}

	press = 0;
	PaletteDraw();

	laststatus = -1;
}






/* Gestion de l'dition d'une palette (PHASE_PARAM) */
/* ------------------------------------------------ */



/* Palette pour le choix des outils disponibles */
/* -------------------------------------------- */

static short tabpalette[] =
{
	ICO_OUTIL_TRACKSBAR, 999, ICO_OUTIL_TRACKS, 999,
	-1,
	ICO_OUTIL_BARRIERE, 999, ICO_OUTIL_MUR, 999,
	ICO_OUTIL_PLANTE, 999, ICO_OUTIL_PLANTEBAS, 999,
	ICO_OUTIL_ELECTRO, 999, ICO_OUTIL_ELECTROBAS, 999,
	ICO_OUTIL_TECHNO, 999, ICO_OUTIL_MEUBLE, 999, ICO_OUTIL_OBSTACLE, 999,
	-1,
	ICO_OUTIL_VISION, 999, ICO_OUTIL_BOIT, 999,
	ICO_OUTIL_AIMANT, 999,
	ICO_OUTIL_LIVRE, 999, ICO_OUTIL_MAGIC, 999,
	ICO_OUTIL_UNSEUL, 999,
	-1,
	ICO_OUTIL_TROU, 999, ICO_OUTIL_GLISSE, 999,
	-1,
	0
};


/* ------------ */
/* DrawEditRest */
/* ----------- */

/*
	Affiche l'indicateur de restes.
 */

void DrawEditRest (Pt pos, short reste, short etat)
{
	Pt			p;
	char		chaine[4];
	Rectangle	rect;

	rect.p1.x = pos.x + LXICO/2;
	rect.p1.y = pos.y;
	rect.p2.x = pos.x + LXICO/2 + 16;
	rect.p2.y = pos.y + LYICO/2;
	DrawFillRect(0, rect, MODELOAD, COLORBLANC);	/* efface l'indicateur prcdent */

	if ( etat == 0 )  return;						/* si bouton relch -> pas d'indicateur */

	p.x = pos.x + LXICO/2 + 3;
	p.y = pos.y + TEXTSIZELIT + 2;
	chaine[0] = 125;
	chaine[1] = 0;
	DrawText(0, p, chaine, TEXTSIZELIT, MODELOAD);	/* dessine la flche ^ */

	p.x = pos.x + LXICO/2 + 3;
	p.y = pos.y + LYICO/2 - 2;
	chaine[0] = 126;
	chaine[1] = 0;
	DrawText(0, p, chaine, TEXTSIZELIT, MODELOAD);	/* dessine la flche v */

	p.x = pos.x + LXICO/2 + 1;
	p.y = pos.y + LYICO/4 + 5;
	if ( reste > 99 )  reste = 99;
	if ( reste < 10 )
	{
		p.x += 4;
		chaine[0] = '0' + reste;
		chaine[1] = 0;
	}
	else
	{
		chaine[0] = '0' + reste/10;
		chaine[1] = '0' + reste%10;
		chaine[2] = 0;
	}

	DrawText(0, p, chaine, TEXTSIZELIT, MODELOAD);	/* dessine le chiffre */
}


/* ---------- */
/* GetEditPos */
/* ---------- */

/*
	Donne la position d'un bouton.
 */

Pt GetEditPos (short x, short y)
{
	Pt		pos;

	if ( tediticon[y][x] == 0 )
	{
		pos.x = -1;
		pos.y = -1;
		return pos;
	}

	pos.x = 20 + (LXICO/2+18)*x;
	pos.y = 38 + (LYICO/2+10)*y;

	return pos;
}


/* ----------- */
/* GetEditRang */
/* ----------- */

/*
	Donne les rangs (rx;ry) du bouton cliqu par la souris.
	Retourne 0 (false) si la souris n'est pas sur un bouton.
	Le type vaut 0 si icne ou +/-1 si flche.
 */

short GetEditRang (Pt pos, short *px, short *py, short *ptype)
{
	short		x, y;
	Pt			pb;

	for ( y=0 ; y<MAXEDITY ; y++ )
	{
		for ( x=0 ; x<MAXEDITX ; x++ )
		{
			pb = GetEditPos(x, y);
			if ( pb.x < 0 )  continue;

			if ( pos.x >= pb.x && pos.x <= pb.x+LXICO/2+14 &&
				 pos.y >= pb.y && pos.y <= pb.y+LYICO/2 )
			{
				*px = x;
				*py = y;
				if ( pos.x <= pb.x+LXICO/2 )
				{
					*ptype = 0;			/* dans l'icne */
				}
				else
				{
					if ( teditetat[y][x] == 0 )  return 0;
					if ( pos.y < pb.y+LYICO/4 )  *ptype = +1;	/* dans la flche suprieure */
					else                         *ptype = -1;	/* dans la flche infrieure */
				}
				return 1;				/* trouv */
			}
		}
	}
	return 0;
}


/* -------------- */
/* EditSearchIcon */
/* -------------- */

/*
	Cherche les rangs (ry;rx) d'une icne.
	Si l'icne n'est pas trouve, retourne 0 (false).
 */

short EditSearchIcon (short icon, short *px, short *py)
{
	short		x, y;

	for ( y=0 ; y<MAXEDITY ; y++ )
	{
		for ( x=0 ; x<MAXEDITX ; x++ )
		{
			if ( icon == tediticon[y][x] )
			{
				*px = x;
				*py = y;
				return 1;		/* trouv */
			}
		}
	}
	return 0;					/* pas trouv */
}


/* =============== */
/* PaletteEditOpen */
/* =============== */

/*
	Dessine la palette d'icnes  diter.
 */

void PaletteEditOpen (short palette[])
{
	short		i;
	short		x, y;
	short		*pdesc;
	Pt			pos;

	/*	Met dans les tables tedit*[] les icnes de tabpalette dans
		l'ordre standard. */

	pdesc = tabpalette;
	for ( y=0 ; y<MAXEDITY ; y++ )
	{
		for ( x=0 ; x<MAXEDITX ; x++ )
		{
			if ( *pdesc == 0 || *pdesc == -1 )
			{
				tediticon[y][x] = 0;
			}
			else
			{
				tediticon[y][x] = *pdesc;
				pdesc += 2;
			}
			teditetat[y][x] = 0;				/* bouton relch */
			teditrest[y][x] = 0;
		}
		if ( *pdesc == -1 )  pdesc++;
	}

	/*	Met dans les tables tedit*[] les informations contenue dans
		la table donne en entre. */

	i = 0;
	while ( palette[i] != 0 )
	{
		if ( palette[i] == -1 )
		{
			i ++;
			continue;
		}

		if ( EditSearchIcon(palette[i], &x, &y) )
		{
			teditetat[y][x] = 1;				/* bouton press */
			teditrest[y][x] = palette[i+1];
		}
		i += 2;
	}

	/*	Affiche toutes les icnes. */

	for ( y=0 ; y<MAXEDITY ; y++ )
	{
		for ( x=0 ; x<MAXEDITX ; x++ )
		{
			if ( tediticon[y][x] != 0 )
			{
				pos = GetEditPos(x, y);
				if ( pos.x >= 0 )
				{
					DrawButton(pos, tediticon[y][x], teditetat[y][x]);
					DrawEditRest(pos, teditrest[y][x], teditetat[y][x]);
				}
			}
		}
	}
}


/* ================ */
/* PaletteEditEvent */
/* ================ */

/*
	Gestion d'un vnement pour diter la palette d'icnes.
	Retourne 0 si l'vnement a t trait.
 */

short PaletteEditEvent (short palette[], short event, Pt pos)
{
	short		x, y, type;
	Pt			pb;

	if ( event != KEYCLIC )  return 1;

	if ( !GetEditRang(pos, &x, &y, &type) )  return 1;

	pb = GetEditPos(x, y);

	if ( type == 0 )							/* clic dans l'icne ? */
	{
		PlaySound(SOUND_CLIC);

		teditetat[y][x] ^= 1;					/* inverse l'tat du bouton */
		DrawButton(pb, tediticon[y][x], teditetat[y][x]);
		if ( teditetat[y][x] == 1 && teditrest[y][x] == 0 )
		{
			teditrest[y][x] = 1;
		}
		DrawEditRest(pb, teditrest[y][x], teditetat[y][x]);
	}
	else										/* clic dans une flche ? */
	{
		teditrest[y][x] += type;				/* modifie le nombre restant */
		if ( teditrest[y][x] <  1 )  teditrest[y][x] =  1;
		if ( teditrest[y][x] > 99 )  teditrest[y][x] = 99;
		DrawEditRest(pb, teditrest[y][x], teditetat[y][x]);		/* affiche le nouveau nombre */
	}

	return 0;
}


/* ================ */
/* PaletteEditClose */
/* ================ */

/*
	Fin de l'dition d'une palette d'icnes.
	Met  jour la palette de l'utilisateur.
 */

void PaletteEditClose (short palette[])
{
	short		x, y;
	short		total, nb, rest;
	short		i = 0;

	total = 0;
	for ( y=0 ; y<MAXEDITY ; y++ )
	{
		for ( x=0 ; x<MAXEDITX ; x++ )
		{
			if ( tediticon[y][x] != 0 && teditetat[y][x] == 1 )  total ++;
		}
	}

	for ( y=0 ; y<MAXEDITY ; y++ )
	{
		nb = 0;
		for ( x=0 ; x<MAXEDITX ; x++ )
		{
			if ( tediticon[y][x] != 0 && teditetat[y][x] == 1 )
			{
				palette[i++] = tediticon[y][x];
				rest = teditrest[y][x];
				if ( rest >= 99 )  rest = 999;	/* infini */
				palette[i++] = rest;

				if ( total <= MAXICONY )
				{
					palette[i++] = -1;			/* passe toujours  la ligne si peu d'icnes */
				}
				else
				{
					nb ++;
				}
			}
		}
		if ( nb > 0 )							/* au moins une icne dans cette ligne ? */
		{
			palette[i++] = -1;					/* passe  la ligne suivante */
		}
	}
	palette[i] = 0;
}





/* Gestion des informations dans la palette */
/* ---------------------------------------- */


#define INFOPOSX		0
#define INFOPOSY		(LYIMAGE-155)
#define INFODIMX		67
#define INFODIMY		58



/* ======== */
/* InfoDraw */
/* ======== */

/*
	Dessine les informations sur un toto.
 */

void InfoDraw (short status, short force, short vision, short mechant, short magic, short cles)
{
	Pixmap		pm;
	Pixmap		pminfo = {0,0,0,0,0,0,0};
	Rectangle	rect;
	Pt			p;

	if ( status  == laststatus  &&
		 force   == lastforce   &&
		 vision  == lastvision  &&
		 mechant == lastmechant &&
		 magic   == lastmagic   &&
		 cles    == lastcles    )   return;

	laststatus  = status;
	lastforce   = force;
	lastvision  = vision;
	lastmechant = mechant;
	lastmagic   = magic;
	lastcles    = cles;

	if ( status == 0 )
	{
		rect.p1.x = INFOPOSX;
		rect.p1.y = INFOPOSY;
		rect.p2.x = INFOPOSX+INFODIMX;
		rect.p2.y = INFOPOSY+INFODIMY;
		DrawFillRect(0, rect, MODELOAD, COLORBLANC);	/* efface l'emplacement */
		return;
	}

	if ( mechant )
	{
		GetIcon(&pm, ICO_INFO+1, 1);
		CopyPixel
		(
			&pm, (p.y=1, p.x=1, p),
			0, (p.y=INFOPOSY, p.x=INFOPOSX, p),
			(p.y=INFODIMY, p.x=INFODIMX, p), MODELOAD
		);
		return;
	}

	GetPixmap(&pminfo, (p.y=LYICO, p.x=LXICO, p), 0, 1);

	GetIcon(&pm, ICO_INFO, 1);
	DuplPixel(&pm, &pminfo);			/* copie l'icne dans pminfo pour pouvoir modifier */

	rect.p1.y = 10;
	rect.p2.y = 10+4;
	if ( force < 30 )
	{
		rect.p1.x = 7+(force*8)/5;
		rect.p2.x = 55;
		DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface l'ascenseur */
	}
	if ( force <= 30 )
	{
		rect.p1.x = 61;
		rect.p2.x = 61+4;
		DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface la lumire overflow */
	}

	rect.p1.y = 23;
	rect.p2.y = 40;

	if ( vision == 0 )
	{
		CopyPixel									/* met les yeux ferms */
		(
			&pminfo, (p.y=62, p.x=1, p),
			&pminfo, (p.y=23, p.x=1, p),
			(p.y=40-23, p.x=26-1, p), MODELOAD
		);
	}

	if ( force < 5 ||
		 vision == 0 )
	{
		rect.p1.x = 26;
		rect.p2.x = 49;
		DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface le saut */
	}

	if ( magic == 0 )
	{
		rect.p1.x = 49;
		rect.p2.x = 67;
		DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface le passe-muraille */
	}

	rect.p1.y = 42;
	rect.p2.y = 54;
	if ( cles == 0 )
	{
		rect.p1.x = 1;
		rect.p2.x = 54;
		DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface la cl + ABC */
	}
	else
	{
		if ( (cles & (1<<0)) == 0 )
		{
			rect.p1.x = 28;
			rect.p2.x = 38;
			DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface la cl A */
		}
		if ( (cles & (1<<1)) == 0 )
		{
			rect.p1.x = 38;
			rect.p2.x = 46;
			DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface la cl B */
		}
		if ( (cles & (1<<2)) == 0 )
		{
			rect.p1.x = 46;
			rect.p2.x = 54;
			DrawFillRect(&pminfo, rect, MODELOAD, COLORBLANC);	/* efface la cl C */
		}
	}

	CopyPixel								/* affiche les informations */
	(
		&pminfo, (p.y=1, p.x=1, p),
		0, (p.y=INFOPOSY, p.x=INFOPOSX, p),
		(p.y=INFODIMY, p.x=INFODIMX, p), MODELOAD
	);

	GivePixmap(&pminfo);
}




/* =========== */
/* PalPartieLg */
/* =========== */

/*
	Retourne la longueur ncessaire pour sauver les variables de la partie en cours.
 */

long PalPartieLg (void)
{
	return
		sizeof(short)*MAXICONY*MAXICONX +
		sizeof(short)*MAXICONY*MAXICONX +
		sizeof(short)*MAXICONY +
		sizeof(Partie);
}


/* ============== */
/* PalPartieWrite */
/* ============== */

/*
	Sauve les variables de la partie en cours.
 */

short PalPartieWrite (long pos, char file)
{
	short		err;
	Partie		partie;

	err = FileWrite(&ticon, pos, sizeof(short)*MAXICONY*MAXICONX, file);
	if ( err )  return err;
	pos += sizeof(short)*MAXICONY*MAXICONX;

	err = FileWrite(&trest, pos, sizeof(short)*MAXICONY*MAXICONX, file);
	if ( err )  return err;
	pos += sizeof(short)*MAXICONY*MAXICONX;

	err = FileWrite(&tspal, pos, sizeof(short)*MAXICONY, file);
	if ( err )  return err;
	pos += sizeof(short)*MAXICONY;

	memset(&partie, 0, sizeof(Partie));
	partie.typepress = typepress;

	err = FileWrite(&partie, pos, sizeof(Partie), file);
	return err;
}


/* ============= */
/* PalPartieRead */
/* ============= */

/*
	Lit les variables de la partie en cours.
 */

short PalPartieRead (long pos, char file)
{
	short		err;
	Partie		partie;

	err = FileRead(&ticon, pos, sizeof(short)*MAXICONY*MAXICONX, file);
	if ( err )  return err;
	pos += sizeof(short)*MAXICONY*MAXICONX;

	err = FileRead(&trest, pos, sizeof(short)*MAXICONY*MAXICONX, file);
	if ( err )  return err;
	pos += sizeof(short)*MAXICONY*MAXICONX;

	err = FileRead(&tspal, pos, sizeof(short)*MAXICONY, file);
	if ( err )  return err;
	pos += sizeof(short)*MAXICONY;

	err = FileRead(&partie, pos, sizeof(Partie), file);
	if ( err )  return err;

	typepress = partie.typepress;

	press = 0;
	PaletteDraw();					/* redessine la palette d'icnes */

	laststatus = -1;

	return 0;
}



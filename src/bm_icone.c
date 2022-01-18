
/* ========== */
/* bm_icone.c */
/* ========== */

#include <stdio.h>

#include "bm.h"




/* ---------------------------------- */
/* Descripteur d'une icne  dessiner */
/* ---------------------------------- */

typedef struct
{
	short	icon;			/* icne */
	short	btransp;		/* 1 -> transparent */
	Pt		pos;			/* coin sup/gauche de l'icne */
	short	posz;			/* position verticale (selon l'axe z) */
	Pt		cel;			/* cellule charnire pour devant/derrire */
	Reg		bbox;			/* bounding box */
	Reg		clip;			/* clipping */
}
icondraw;



/* ---------------------------------------- */
/* Descripteur d'une rgion  mettre  jour */
/* ---------------------------------------- */

typedef struct
{
	Reg		reg;			/* rgion */
	short	update;			/* 1 => toujours redessiner */
}
listreg;



/* -------------------------------------- */
/* Descripteur d'une bounding box d'icne */
/* -------------------------------------- */

typedef struct
{
	char	left;
	char	right;
	char	up;
	char	down;
}
bbox;





/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

#define MAXICONDRAW	20						/* nb max d'icnes dans une image */
#define MAXREGION	40						/* nb max de rgions dans une image */

static Pixmap		pmwork  = {0,0,0,0,0,0,0};		/* pixmap temporaire de travail */
static Pixmap		pmmask  = {0,0,0,0,0,0,0};		/* pixmap du masque selon le dcor devant */
static Pixmap		pmcopy  = {0,0,0,0,0,0,0};		/* copie modifiable d'une icne */

static icondraw		ListIconDrawNew[MAXICONDRAW];	/* liste des icnes  dessnier */
static icondraw		ListIconDrawOld[MAXICONDRAW];	/* liste des icnes  dessnier */
static listreg		ListRegNew[MAXREGION];			/* liste des rgions */
static listreg		ListRegOld[MAXREGION];			/* liste des rgions */

static bbox			IconBBox[128+64];				/* bbox des icnes 0..191 */




/* =========== */
/* IfNilRegion */
/* =========== */

/*
	Vrifie si une rgion est vide (p1=p2=0).
	Si oui retourne 1 (true), si non retourne 0 (false).
 */

short IfNilRegion (Reg rg)
{
	return ( rg.r.p1.x==0 && rg.r.p1.y==0 &&
			 rg.r.p2.x==0 && rg.r.p2.y==0 );
}


/* ============ */
/* IfSectRegion */
/* ============ */

/*
	Vrifie si deux rgions ont une zone d'intersection.
	Si oui retourne 1 (true), si non retourne 0 (false).
 */

short IfSectRegion (Reg r1, Reg r2)
{
  return   ( r1.r.p2.x >= r2.r.p1.x )
      &&   ( r1.r.p1.x <  r2.r.p2.x )
      &&   ( r1.r.p2.y >= r2.r.p1.y )
      &&   ( r1.r.p1.y <  r2.r.p2.y );
}


/* ======== */
/* OrRegion */
/* ======== */

/*
	Effectue l'union de deux rgions.
	La nouvelle rgion englobe les deux rgions initiales.
 */

Reg OrRegion (Reg r1, Reg r2)
{
	Reg	rg;

	rg.r.p1.x = ( r1.r.p1.x < r2.r.p1.x ) ? r1.r.p1.x : r2.r.p1.x ;  /* min */
	rg.r.p2.x = ( r1.r.p2.x > r2.r.p2.x ) ? r1.r.p2.x : r2.r.p2.x ;  /* max */
	rg.r.p1.y = ( r1.r.p1.y < r2.r.p1.y ) ? r1.r.p1.y : r2.r.p1.y ;  /* min */
	rg.r.p2.y = ( r1.r.p2.y > r2.r.p2.y ) ? r1.r.p2.y : r2.r.p2.y ;  /* max */
	return rg;
}


/* ========= */
/* AndRegion */
/* ========= */

/*
	Effectue l'intersection de deux rgions.
	La nouvelle rgion peut tre nulle (p1=p2=0).
 */

Reg AndRegion (Reg r1, Reg r2)
{
	Reg	rg;

	rg.r.p1.x = ( r1.r.p1.x > r2.r.p1.x ) ? r1.r.p1.x : r2.r.p1.x ;  /* max */
	rg.r.p2.x = ( r1.r.p2.x < r2.r.p2.x ) ? r1.r.p2.x : r2.r.p2.x ;  /* min */
	rg.r.p1.y = ( r1.r.p1.y > r2.r.p1.y ) ? r1.r.p1.y : r2.r.p1.y ;  /* max */
	rg.r.p2.y = ( r1.r.p2.y < r2.r.p2.y ) ? r1.r.p2.y : r2.r.p2.y ;  /* min */

	if ( rg.r.p1.x >= rg.r.p2.x || rg.r.p1.y >= rg.r.p2.y )
		return (rg.r.p1.y=0,rg.r.p1.x=0,
				rg.r.p2.y=0,rg.r.p2.x=0,rg);	/* pas d'intersection */
	else
		return rg;								/* retourne l'intersection */
}



/* --------- */
/* PutRegion */
/* --------- */

/*
	Ajoute une rgion rectangulaire dans une liste ListReg.
		*listregion	->	liste dans laquelle il faut ajouter
		region		->	rgion  ajouter
		update		->	0 = redessine seulement si icne en mouvement dedans
						1 = redessine toujours (dcor chang)
 */

static short PutRegion (listreg *listregion, Reg region, short update)
{
	short		i;
	Reg			*pr;
	Reg			clip;
	Reg			r;

	clip = AndRegion( region, (r.r.p1.y=0, r.r.p1.x=0, r.r.p2.y=DIMYDRAW ,r.r.p2.x=DIMXDRAW, r) );

	for ( i=0 ; i<MAXREGION ; i++ )
	{
		pr = &listregion[i].reg;
		if ( IfNilRegion(*pr) )
		{
			*pr = clip;				/* met une nouvelle rgion */
			listregion[i].update = update;
			return 0;
		}
		else
		{
			if ( IfSectRegion(*pr, clip) )
			{
				*pr = OrRegion(*pr, clip);	/* agrandit la rgion existante */
				listregion[i].update |= update;
				return 0;
			}
		}
	}
	return 1;					/* erreur, plus de place libre */
}



/* ---------- */
/* IconRegion */
/* ---------- */

/*
	Donne la rgion occupe par une icne et son ombre ventuelle.
 */

static Reg IconRegion (short i, Pt pos)
{
	Pixmap		pm;
	Reg			rg;

	GetIcon(&pm, i, 0);				/* donne juste les dimensions */

	rg.r.p1.x = pos.x;
	rg.r.p1.y = pos.y;				/* coin sup/gauche */
	rg.r.p2.x = pos.x + pm.dx;
	rg.r.p2.y = pos.y + pm.dy;		/* coin inf/droite */

	i &= 0x01FF;					/* icne 0..512-1 */
	if ( i < 128+64 )
	{
		rg.r.p1.x += IconBBox[i].left;
		rg.r.p2.x -= IconBBox[i].right;
		rg.r.p1.y += IconBBox[i].up;
		rg.r.p2.y -= IconBBox[i].down;
	}

	return rg;						/* retourne la rgion */
}



/* ----------- */
/* IconDrawOne */
/* ----------- */

/*
	Dessine une icne (chair+fond+ombre) dans un pixmap quelconque.
	Ne dpasse pas de la rgion de clipping donne.
 */

static void IconDrawOne(short i, short m, Pt pos, short posz, Pt cel, Reg clip, Pixmap *ppm)
{
	Pixmap		pmicon;						/* pixmap de l'icne  dessiner */
	Reg			use;						/* rgion  utiliser */
	Pt			p;

	use = AndRegion(clip, IconRegion(i, pos) );
	if ( IfNilRegion(use) ) return;			/* retour si rien  dessiner */

	DecorIconMask(&pmmask, pos, posz, cel);	/* fabrique le masque */

	if ( m == 0 )
	{
		GetIcon(&pmicon, i+ICOMOFF, 1);			/* cherche le pixmap du fond */
		DuplPixel(&pmicon, &pmcopy);			/* copie l'icne */
		CopyPixel								/* rogne l'icne de masque */
		(
			&pmmask, (p.y=0, p.x=0, p),
			&pmcopy, (p.y=0, p.x=0, p),
			(p.y=LYICO, p.x=LXICO, p), MODEAND
		);
		CopyPixel								/* dessine le fond */
		(
			&pmcopy,							/* source */
			(p.y = use.r.p1.y - pos.y ,
			p.x = use.r.p1.x - pos.x , p),
			ppm,								/* destination */
			use.r.p1,
			(p.y = use.r.p2.y - use.r.p1.y ,	/* dimensions */
			p.x = use.r.p2.x - use.r.p1.x , p),
			MODEAND								/* mode */
		);
	}

	GetIcon(&pmicon, i, 1);					/* cherche le pixmap de la chair */
	DuplPixel(&pmicon, &pmcopy);			/* copie l'icne */
	CopyPixel							/* rogne l'icne de la chair */
	(
		&pmmask, (p.y=0, p.x=0, p),
		&pmcopy, (p.y=0, p.x=0, p),
		(p.y=LYICO, p.x=LXICO, p), MODEAND
	);
	CopyPixel								/* dessine la chair */
	(
		&pmcopy,							/* source */
		(p.y = use.r.p1.y - pos.y ,
		p.x = use.r.p1.x - pos.x , p),
		ppm,								/* destination */
		use.r.p1,
		(p.y = use.r.p2.y - use.r.p1.y ,	/* dimensions */
		p.x = use.r.p2.x - use.r.p1.x , p),
		MODEOR								/* mode */
	);
}



/* =========== */
/* IconDrawAll */
/* =========== */

/*
	Indique qu'il faudra tout redessiner, avant un IconDrawOpen/Put(s)/Close.
 */

void IconDrawAll (void)
{
	short		i;
	Reg			r;

	for ( i=0 ; i<MAXREGION ; i++ )
	{
		ListRegNew[i].reg = (r.r.p1.y=0,r.r.p1.x=0,
							 r.r.p2.y=0,r.r.p2.x=0,r);	/* libre toute la table */
		ListRegNew[i].update = 0;
	}

	ListRegNew[0].reg = (r.r.p1.y=0, r.r.p1.x=0, r.r.p2.y=DIMYDRAW ,r.r.p2.x=DIMXDRAW, r);
	ListRegNew[0].update = 1;
}



/* ============= */
/* IconDrawFlush */
/* ============= */

/*
	Vide tous les buffers internes.
 */

void IconDrawFlush (void)
{
	short		i;

	for ( i=0 ; i<MAXICONDRAW ; i++ )
	{
		ListIconDrawNew[i].icon = 0;		/* libre toute la table */
	}
}



/* ============ */
/* IconDrawOpen */
/* ============ */

/*
	Prpare le dessin cach en mmoire de plusieurs icnes.
	Transfert la liste ListIconDrawNew dans ListIconDrawOld puis vide la premire.
	Transfert la liste ListRegNew dans ListRegOld puis vide la premire.
 */

void IconDrawOpen (void)
{
	short		i;
	Reg			r;

	for ( i=0 ; i<MAXICONDRAW ; i++ )
	{
		ListIconDrawOld[i] = ListIconDrawNew[i];	/* table old <-- new */
		ListIconDrawNew[i].icon = 0;				/* libre toute la table */
	}

	for ( i=0 ; i<MAXREGION ; i++ )
	{
		ListRegOld[i] = ListRegNew[i];				/* table old <-- new */
		ListRegNew[i].reg = (r.r.p1.y=0,r.r.p1.x=0,
							 r.r.p2.y=0,r.r.p2.x=0,r);	/* libre toute la table */
		ListRegNew[i].update = 0;
	}
}



/* =========== */
/* IconDrawPut */
/* =========== */

/*
	Met une icne  dessiner dans ListIconDrawNew,
	et met  jour les tables ListRegNew et ListRegOld.
	Chaque icne possde une rgion de clipping de laquelle elle ne
	doit pas dpasser.
	Si btransp == 1, ne dessine pas le fond (transparent).
 */

short IconDrawPut (short ico, short btransp, Pt pos, short posz, Pt cel, Reg clip)
{
	short		i;
	Reg			rg;

	for ( i=0 ; i<MAXICONDRAW ; i++ )
	{
		if ( ListIconDrawNew[i].icon == 0 )
		{
			ListIconDrawNew[i].icon    = ico;
			ListIconDrawNew[i].btransp = btransp;
			ListIconDrawNew[i].pos     = pos;
			ListIconDrawNew[i].posz    = posz;
			ListIconDrawNew[i].cel     = cel;
			ListIconDrawNew[i].clip    = clip;

			rg = IconRegion (ico, pos);
			ListIconDrawNew[i].bbox  = rg;
			PutRegion(ListRegNew, rg, 0);
			PutRegion(ListRegOld, rg, 0);

			return 0;			/* retour ok */
		}
	}
	return 1;				/* erreur, table pleine */
}



/* -------------- */
/* IconDrawIfMove */
/* -------------- */

/*
	Regarde si une icne a chang de place, ou si elle n'est pas seule dans sa rgion.
	Si oui retourne 1 (true) car il faut la dessiner, si non retourne 0 (false).
 */

static short IconDrawIfMove (listreg lrg)
{
	short		i,j,n;

	/* Regarde s'il s'agit d'une rgion  redessiner de toute faon */

	if ( lrg.update != 0 )  return 1;

	/* Cherche l'icne dans la rgion, seulement s'il a en a une seule. */

	n = 0;
	for ( i=0 ; i<MAXICONDRAW ; i++ )
	{
		if ( ListIconDrawNew[i].icon != 0 )
		{
			if ( IfSectRegion(ListIconDrawNew[i].bbox, lrg.reg) )
			{
				n++; if (n>1)  return 1;		/* dessin car plusieurs icnes dans rgion */
				j = i;
			}
		}
	}

	if (n==0)  return 1;				/* dessin si rien trouv */

	/* Cherche si l'icne tait dj l lors du dessin prcdent. */

	for ( i=0 ; i<MAXICONDRAW ; i++ )
	{
		if ( ListIconDrawOld[i].icon  != 0                        &&
			 ListIconDrawOld[i].icon  == ListIconDrawNew[j].icon  &&
			 ListIconDrawOld[i].pos.x == ListIconDrawNew[j].pos.x &&
			 ListIconDrawOld[i].pos.y == ListIconDrawNew[j].pos.y )
		{
			goto next;
		}
	}
	return 1;					/* dessin car icne n'tait pas l avant */

	/* Cherche s'il y avait plus d'une icne avant. */

	next:
	n = 0;
	for ( i=0 ; i<MAXICONDRAW ; i++ )
	{
		if ( ListIconDrawOld[i].icon != 0 )
		{
			if ( IfSectRegion(ListIconDrawOld[i].bbox, lrg.reg) )
			{
				n++; if (n>1)  return 1;		/* dessin car plusieurs icnes avant */
			}
		}
	}

	return 0;					/* dessin superflu */
}



/* ============== */
/* IconDrawUpdate */
/* ============== */

/*
	Indique une zone rectangulaire qui devra tre raffiche,
	par exemple parce qu'une partie du dcor a chang.
 */

void IconDrawUpdate (Reg rg)
{
	PutRegion(ListRegOld, rg, 1);
	PutRegion(ListRegNew, rg, 1);
}



/* ============= */
/* IconDrawClose */
/* ============= */

/*
	Dessine effectivement toutes les icnes donnes avec IconDrawPut,
	comprises dans une rgion de clipping.
 */

void IconDrawClose (short bdraw)
{
	short		i,j;
	Pt			p;
	Reg			r,ro;
	Pixmap		*ppmdecor;

	ppmdecor = DecorGetPixmap();

	for ( i=0 ; i<MAXREGION ; i++ )
	{
		if ( IfNilRegion(ListRegOld[i].reg) )  continue;
		if ( !IconDrawIfMove(ListRegOld[i]) )  continue;

		CopyPixel					/* met le dcor de fond */
		(
			ppmdecor, ListRegOld[i].reg.r.p1,
			&pmwork, ListRegOld[i].reg.r.p1,
			(p.y = ListRegOld[i].reg.r.p2.y - ListRegOld[i].reg.r.p1.y ,
			 p.x = ListRegOld[i].reg.r.p2.x - ListRegOld[i].reg.r.p1.x , p),
			MODELOAD
		);

		for ( j=0 ; j<MAXICONDRAW ; j++ )
		{
			if ( ListIconDrawNew[j].icon != 0 )
			{
				if ( IfSectRegion(ListRegOld[i].reg, ListIconDrawNew[j].bbox) )
				{
					IconDrawOne			/* dessine l'icne */
					(
						ListIconDrawNew[j].icon,
						ListIconDrawNew[j].btransp,
						ListIconDrawNew[j].pos,
						ListIconDrawNew[j].posz,
						ListIconDrawNew[j].cel,
						ListIconDrawNew[j].clip,
						&pmwork
					);
				}
			}
		}

		ro = AndRegion(ListRegOld[i].reg, (r.r.p1.y=0, r.r.p1.x=0,
										   r.r.p2.y=DIMYDRAW, r.r.p2.x=DIMXDRAW, r) );
		if ( bdraw && !(IfNilRegion(ro)) )
		{
			CopyPixel				/* met l'image dans l'cran */
			(
				&pmwork, (p.y = ro.r.p1.y, p.x = ro.r.p1.x, p),
				0,       (p.y = POSYDRAW+ro.r.p1.y, p.x = POSXDRAW+ro.r.p1.x, p),
				(p.y = ro.r.p2.y - ro.r.p1.y,
				p.x = ro.r.p2.x - ro.r.p1.x, p),
				MODELOAD
			);
		}
	}

	   g_updatescreen = 0;				/* l'cran est  jour */
}




/* ======== */
/* IconOpen */
/* ======== */

/*
	Ouverture gnrale.
 */

short IconOpen (void)
{
	short		err;
	Pt			p;

	/* Ouvre le pixmap de travail. */

	err = GetPixmap(&pmwork, (p.y=DIMYDRAW, p.x=DIMXDRAW ,p), 0, 1);
	if (err)  return err;

	/* Ouvre le pixmap pour copier une icne. */

	err = GetPixmap(&pmcopy, (p.y=LYICO, p.x=LXICO, p), 0, 1);
	if (err)  return err;

	return 0;
}



/* ========= */
/* IconClose */
/* ========= */

/*
	Fermeture gnrale.
 */

void IconClose (void)
{
	GivePixmap(&pmwork);
	GivePixmap(&pmmask);
	GivePixmap(&pmcopy);
}



/* ============== */
/* IconGetPixmap */
/* ============== */

/*
	Donne le pointeur au pixmap contenant les icnes.
 */

Pixmap* IconGetPixmap (void)
{
	return &pmwork;
}



#if 0
/* --------- */
/* TestHLine */
/* --------- */

/*
	Teste si une ligne horizontale est entirement vide.
	Si oui, retourne 1 (true).
 */

short TestHLine (Pixmap *ppm, short y)
{
	Pt		pos;

	pos.y = y;
	for ( pos.x=0 ; pos.x<LXICO ; pos.x++ )
	{
		if ( GetPixel(ppm, pos) != 0 )  return 0;
	}
	return 1;
}

/* --------- */
/* TestVLine */
/* --------- */

/*
	Teste si une ligne verticale est entirement vide.
	Si oui, retourne 1 (true).
 */

short TestVLine (Pixmap *ppm, short x)
{
	Pt		pos;

	pos.x = x;
	for ( pos.y=0 ; pos.y<LYICO ; pos.y++ )
	{
		if ( GetPixel(ppm, pos) != 0 )  return 0;
	}
	return 1;
}
#endif

/* ======== */
/* IconInit */
/* ======== */

/*
	Calcule les bbox des icnes.
 */

void IconInit (void)
{
	Pixmap		pm;
	short		i;
	short		x, y;

	for ( i=0 ; i<128+64 ; i++ )
	{
		GetIcon(&pm, i+ICOMOFF, 1);

		for ( x=0 ; x<LXICO/2 ; x++ )
		{
			if ( !TestVLine(&pm, x) )  break;
		}
		IconBBox[i].left = x;

		for ( x=LXICO-1 ; x>LXICO/2 ; x-- )
		{
			if ( !TestVLine(&pm, x) )  break;
		}
		IconBBox[i].right = LXICO-1-x;

		for ( y=0 ; y<LYICO/2 ; y++ )
		{
			if ( !TestHLine(&pm, y) )  break;
		}
		IconBBox[i].up = y;

		for ( y=LYICO-1 ; y>LYICO/2 ; y-- )
		{
			if ( !TestHLine(&pm, y) )  break;
		}
		IconBBox[i].down = LYICO-1-y;
	}
}



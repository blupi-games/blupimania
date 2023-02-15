
/* ========== */
/* bm_icone.c */
/* ========== */

#include <stdio.h>

#include "bm.h"
#include "bm_icon.h"



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

static Pixmap		pmwork  = {0};		/* pixmap temporaire de travail */
static Pixmap		pmmask  = {0};		/* pixmap du masque selon le dcor devant */
static Pixmap		pmcopy  = {0};		/* copie modifiable d'une icne */

static icondraw		ListIconDrawNew[MAXICONDRAW];	/* liste des icnes  dessnier */
static icondraw		ListIconDrawOld[MAXICONDRAW];	/* liste des icnes  dessnier */
static listreg		ListRegNew[MAXREGION];			/* liste des rgions */
static listreg		ListRegOld[MAXREGION];			/* liste des rgions */


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
        {
          rg.r.p1.y=0;
          rg.r.p1.x=0;
          rg.r.p2.y=0;
          rg.r.p2.x=0;
		return rg;	/* pas d'intersection */
        }
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

	return rg;						/* retourne la rgion */
}



/* ----------- */
/* IconDrawOne */
/* ----------- */

typedef struct {
  Pt p1;
  Pt p2;
  Pt dim;
  int icon;
} SuperCelHover;

/*
	Dessine une icne (chair+fond+ombre) dans un pixmap quelconque.
	Ne dpasse pas de la rgion de clipping donne.
 */

static SuperCelHover IconDrawOne(short i, short m, Pt pos, short posz, Pt cel, Reg clip, Pixmap *ppm)
{
	Pixmap		pmicon;						/* pixmap de l'icne  dessiner */
	Reg			use;						/* rgion  utiliser */
	Pt			p1, dim;
        SuperCelHover hover = {0};

	use = AndRegion(clip, IconRegion(i, pos) );
	if ( IfNilRegion(use) ) return hover;			/* retour si rien  dessiner */

        /* Récupère la liste des élément de décor à redessiner */
	const ImageStack * list = DecorIconMask(pos, posz, cel);	/* fabrique le masque */

	GetIcon(&pmicon, i, 1);					/* cherche le pixmap de la chair */
        if (m == 1)
        {
          SDL_SetTextureAlphaMod(pmicon.texture, 128);
        }


        p1.y = use.r.p1.y - pos.y;
        p1.x = use.r.p1.x - pos.x;
        dim.y = use.r.p2.y - use.r.p1.y;
        dim.x = use.r.p2.x - use.r.p1.x;

        /* crop blupi (or tank, etc) when it falls
         * 15..15 blupi drinks too much
         * 28..31 blupi is falling
         * 47 baloon
         * 63 blupi baloon
         */
        if (((i >= 14 && i <= 15) || (i >= 28 && i <= 31) || i == 47 || i == 63 || i == ICO_CAISSE || i == ICO_CAISSEV || i == ICO_CAISSEO || i == ICO_CAISSEV || i == ICO_CAISSEG || (i >= ICO_TANK_E && i <= ICO_TANK_S) || i == ICO_TANK_X || i == ICO_TANK_EO || i == ICO_TANK_NS) && posz > 0)
        {
          //dim.y -= posz;
          Pixmap mask;
          Pt p0 = {0, 0}, p2 = {0, 0};

          Pixmap pmtemp;
          pmtemp.texture = SDL_CreateTexture (
          g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXICO, LYICO);
          SDL_SetTextureBlendMode(pmtemp.texture, SDL_BLENDMODE_BLEND);
          pmtemp.dx = LXICO;
          pmtemp.dy = LYICO;
          pmtemp.orig.x = 0;
          pmtemp.orig.y = 0;

          Pt maskDim = {LYICO, LXICO};
          GetIcon(&mask, ICO_SOLMASK, 1);
          CopyPixel(&mask, p0, &pmtemp, p0, maskDim, 0);

          /* Calcul la position du "toto" sans la chute */
          Pt absPos = {pos.y + POSYDRAW + LYICO - posz, pos.x + POSXDRAW + LXICO};
          /* Calcul la cellule où se situe le trou */
          Pt holeCel = GraToCel(absPos);
          // Récupère les coordonnées du trou
          Pt holeCoords = CelToGra2(holeCel, SDL_TRUE);

          holeCoords.x -= POSXDRAW;
          holeCoords.y -= POSYDRAW;

          /* Evite de dessiner en dessus du masque */
          Pt cropDim = {LYICO, LXICO};
          cropDim.y -= use.r.p1.y - holeCoords.y;

          /* copie le fond dans le "masque" */
          SDL_SetTextureBlendMode(ppm->texture, SDL_BLENDMODE_MOD);
          if (holeCoords.y < 0)
          {
            maskDim.y += holeCoords.y;
            p2.y = -holeCoords.y;
            holeCoords.y = 0;
          }
          if (holeCoords.x < 0)
          {
            maskDim.x += holeCoords.x;
            p2.x = -holeCoords.x;
            holeCoords.x = 0;
          }
          CopyPixel(ppm, holeCoords, &pmtemp, p2, maskDim, 0);
          SDL_SetTextureBlendMode(ppm->texture, SDL_BLENDMODE_BLEND);

          /* Dessine le "toto" */
          CopyPixel								/* dessine la chair */
          (
                  &pmicon,							/* source */
                  p1,
                  ppm,								/* destination */
                  use.r.p1,
                  cropDim,
                  MODEOR								/* mode */
          );

          /* Utilise le "masque" par dessus */
          CopyPixel(&pmtemp, p2, ppm, holeCoords, maskDim, 0);

          SDL_DestroyTexture(pmtemp.texture);
        }
        else {
        /* Dessine le "toto" */
	CopyPixel								/* dessine la chair */
	(
		&pmicon,							/* source */
		p1,
		ppm,								/* destination */
		use.r.p1,
		dim,
		MODEOR								/* mode */
	);}

        if (m == 1)
        {
          SDL_SetTextureAlphaMod(pmicon.texture, 255);
        }

        /* Redessine des éléments de décors qui doivent apparaître devant les "toto" */
        for (int j = 0; j < 21*22; ++j)
        {
          if (!list[j].icon) continue;

          Pixmap		pmicon2;
          GetIcon(&pmicon2, list[j].icon, 1);

          SDL_bool blupiBaloonStart = (((i == 63 || i == 47) && posz < 20) || (i != 63 && i != 47));

          if (list[j].super && blupiBaloonStart)
          {
              dim.y = pmicon2.dy;
              dim.x = pmicon2.dx;

              hover.p1.y = 0;
              hover.p1.x = 0;
              hover.p2 = list[j].off;
              hover.dim = list[j].dim;
              hover.icon = list[j].icon;

              //if (g_typejeu == 0 && !g_superInvalid)
              //  continue;

              /* Special case where a "toto" is on the ground */
              SDL_bool isGround = (hover.icon >= ICO_SOL && hover.icon < ICO_SOLMAX) || hover.icon == ICO_SOLDALLE3 || hover.icon == ICO_SOLDALLE4 || hover.icon == ICO_SOLDALLE5 || hover.icon == ICO_TROU || hover.icon == ICO_TROUBOUCHE || hover.icon == ICO_SENSUNI_E || hover.icon == ICO_SENSUNI_O || hover.icon == ICO_SENSUNI_N || hover.icon == ICO_SENSUNI_S || hover.icon == ICO_UNSEUL /*|| hover.icon == ICO_ARRIVEEVIDE || hover.icon == ICO_ARRIVEEPRIS*/;
              // FIXME: ajouter la trappe fermée
              /* We need the original image (no redraw) */
              if (isGround)
                continue;
          }

          Pt p1 = {0, 0};
          Pt p2 = list[j].off;
          dim = list[j].dim;
          CopyPixel								/* dessine la chair */
          (
                  &pmicon2,							/* source */
                  p1,
                  ppm,								/* destination */
                  p2,
                  dim,
                  MODEOR								/* mode */
          );
        }

        return hover;
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
#if 0
	for ( i=0 ; i<MAXREGION ; i++ )
	{
		ListRegNew[i].reg = (r.r.p1.y=0,r.r.p1.x=0,
                                      r.r.p2.y=0,r.r.p2.x=0,r);	/* libre toute la table */
		ListRegNew[i].update = 0;
	}

	ListRegNew[0].reg = (r.r.p1.y=0, r.r.p1.x=0, r.r.p2.y=DIMYDRAW ,r.r.p2.x=DIMXDRAW, r);
	ListRegNew[0].update = 1;
#endif

	for ( i=0 ; i<MAXREGION ; i++ )
	{
		ListRegNew[i].reg.r.p1.y = 0;
                ListRegNew[i].reg.r.p1.x = 0;
                ListRegNew[i].reg.r.p2.y = 0;
                ListRegNew[i].reg.r.p2.x = 0;
                                                          /* libre toute la table */
		ListRegNew[i].update = 0;
	}

        ListRegNew[0].reg.r.p1.y = 0;
        ListRegNew[0].reg.r.p1.x = 0;
        ListRegNew[0].reg.r.p2.y = DIMYDRAW;
        ListRegNew[0].reg.r.p2.x = DIMXDRAW;
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
		ListRegNew[i].reg.r.p1.y = 0;
                ListRegNew[i].reg.r.p1.x=0;
                ListRegNew[i].reg.r.p2.y=0;
                ListRegNew[i].reg.r.p2.x=0;	/* libre toute la table */
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
	Dessine effectivement toutes les icônes données avec IconDrawPut,
	comprises dans une région de clipping.
 */

void IconDrawClose (short bdraw)
{
	short		i,j;
	Pt			p1, p2, dim;
	Reg			r,ro;
	Pixmap		*ppmdecor;
        SuperCelHover hover = {0};

	ppmdecor = DecorGetPixmap();

	for ( i=0 ; i<MAXREGION ; i++ )
	{
		if ( IfNilRegion(ListRegOld[i].reg) )  continue;
		if ( !IconDrawIfMove(ListRegOld[i]) )  continue;

                p1.y = ListRegOld[i].reg.r.p2.y - ListRegOld[i].reg.r.p1.y;
                p1.x = ListRegOld[i].reg.r.p2.x - ListRegOld[i].reg.r.p1.x;
		CopyPixel					/* met le décor de fond */
		(
			ppmdecor, ListRegOld[i].reg.r.p1,
			&pmwork, ListRegOld[i].reg.r.p1,
			p1,
			MODELOAD
		);


		for ( j=0 ; j<MAXICONDRAW ; j++ )
		{
			if ( ListIconDrawNew[j].icon > 0 )
			{
				if ( IfSectRegion(ListRegOld[i].reg, ListIconDrawNew[j].bbox) )
				{
					SuperCelHover _hover = IconDrawOne			/* dessine l'icône */
					(
						ListIconDrawNew[j].icon,
						ListIconDrawNew[j].btransp,
						ListIconDrawNew[j].pos,
						ListIconDrawNew[j].posz,
						ListIconDrawNew[j].cel,
						ListIconDrawNew[j].clip,
						&pmwork
					);
                                        if (_hover.icon)
                                          hover = _hover;
				}
			}
		}

		if (hover.icon)
                {
                        Pixmap tmp = {0};
                        Pixmap pmicon = {0};

                        if (g_typejeu == 1)
                        {
                          hover.dim.y = LYICO;
                          hover.dim.x = LXICO;
                          GetPixmap(&tmp, hover.dim, 2, 0);
                          GetIcon(&pmicon, ICO_CELARROWS, 1);
                          DuplPixel(&pmicon, &tmp);
                          SDL_SetTextureAlphaMod(tmp.texture, 128);
                        }
                        else
                        {
                          hover.dim.y = LYICO;
                          hover.dim.x = LXICO;
                          GetPixmap(&tmp, hover.dim, 2, 0);
                          GetIcon(&pmicon, g_superInvalid ? ICO_CROIX : hover.icon, 1);
                          DuplPixel(&pmicon, &tmp);
                          SDL_SetTextureAlphaMod(tmp.texture, 128);
                          SDL_SetTextureColorMod(tmp.texture, 32, 32, 32);
                        }

                        CopyPixel								/* dessine la chair */
                        (
                                &tmp,							/* source */
                                hover.p1,
                                &pmwork,								/* destination */
                                hover.p2,
                                hover.dim,
                                MODEOR								/* mode */
                        );

                        GivePixmap(&tmp);
                }

		r.r.p1.y=0;
                r.r.p1.x=0;
                r.r.p2.y=DIMYDRAW;
                r.r.p2.x=DIMXDRAW;
		ro = AndRegion(ListRegOld[i].reg, r );
		if ( bdraw && !(IfNilRegion(ro)) )
		{
                        p1.y = ro.r.p1.y;
                        p1.x = ro.r.p1.x;
                        p2.y = POSYDRAW+ro.r.p1.y;
                        p2.x = POSXDRAW+ro.r.p1.x;
                        dim.y = ro.r.p2.y - ro.r.p1.y;
                        dim.x = ro.r.p2.x - ro.r.p1.x;
			CopyPixel				/* met l'image dans l'écran */
			(
				&pmwork, p1,
				0,       p2,
				dim,
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

        p.y = DIMYDRAW;
        p.x = DIMXDRAW;
	err = GetPixmap(&pmwork, p, 0, 1);
	if (err)  return err;

	/* Ouvre le pixmap pour copier une icne. */

        p.y = LYICO;
        p.x = LXICO;
	err = GetPixmap(&pmcopy, p, 0, 1);
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

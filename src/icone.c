
/* ========== */
/* bm_icone.c */
/* ========== */

#include <stdio.h>

#include "blupimania.h"
#include "icon.h"

/* ---------------------------------- */
/* Descripteur d'une icône à dessiner */
/* ---------------------------------- */

typedef struct {
  short icon;    /* icône */
  short btransp; /* 1 -> transparent, 2 -> invincible */
  Pt    pos;     /* coin sup/gauche de l'icône */
  short posz;    /* position verticale (selon l'axe z) */
  Pt    cel;     /* cellule charnière pour devant/derrière */
  Reg   bbox;    /* bounding box */
  Reg   clip;    /* clipping */
} icondraw;

/* ---------------------------------------- */
/* Descripteur d'une région à mettre à jour */
/* ---------------------------------------- */

typedef struct {
  Reg   reg;    /* région */
  short update; /* 1 => toujours redessiner */
} listreg;

/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

#define MAXICONDRAW 30 /* nb max d'icônes dans une image */
#define MAXREGION 40   /* nb max de régions dans une image */

static Pixmap pmwork = {0}; /* pixmap temporaire de travail */
static Pixmap pmcopy = {0}; /* copie modifiable d'une icône */

static icondraw ListIconDrawNew[MAXICONDRAW]; /* liste des icônes à dessnier */
static icondraw ListIconDrawOld[MAXICONDRAW]; /* liste des icônes à dessnier */
static listreg  ListRegNew[MAXREGION];        /* liste des régions */
static listreg  ListRegOld[MAXREGION];        /* liste des régions */

/* =========== */
/* IfNilRegion */
/* =========== */

/*
    Vérifie si une région est vide (p1=p2=0).
    Si oui retourne 1 (true), si non retourne 0 (false).
 */

static short
IfNilRegion (Reg rg)
{
  return (rg.r.p1.x == 0 && rg.r.p1.y == 0 && rg.r.p2.x == 0 && rg.r.p2.y == 0);
}

/* ============ */
/* IfSectRegion */
/* ============ */

/*
    Vérifie si deux régions ont une zone d'intersection.
    Si oui retourne 1 (true), si non retourne 0 (false).
 */

static short
IfSectRegion (Reg r1, Reg r2)
{
  return (r1.r.p2.x >= r2.r.p1.x) && (r1.r.p1.x < r2.r.p2.x) &&
         (r1.r.p2.y >= r2.r.p1.y) && (r1.r.p1.y < r2.r.p2.y);
}

/* ======== */
/* OrRegion */
/* ======== */

/*
    Effectue l'union de deux régions.
    La nouvelle région englobe les deux régions initiales.
 */

static Reg
OrRegion (Reg r1, Reg r2)
{
  Reg rg;

  rg.r.p1.x = (r1.r.p1.x < r2.r.p1.x) ? r1.r.p1.x : r2.r.p1.x; /* min */
  rg.r.p2.x = (r1.r.p2.x > r2.r.p2.x) ? r1.r.p2.x : r2.r.p2.x; /* max */
  rg.r.p1.y = (r1.r.p1.y < r2.r.p1.y) ? r1.r.p1.y : r2.r.p1.y; /* min */
  rg.r.p2.y = (r1.r.p2.y > r2.r.p2.y) ? r1.r.p2.y : r2.r.p2.y; /* max */
  return rg;
}

/* ========= */
/* AndRegion */
/* ========= */

/*
    Effectue l'intersection de deux régions.
    La nouvelle région peut être nulle (p1=p2=0).
 */

static Reg
AndRegion (Reg r1, Reg r2)
{
  Reg rg;

  rg.r.p1.x = (r1.r.p1.x > r2.r.p1.x) ? r1.r.p1.x : r2.r.p1.x; /* max */
  rg.r.p2.x = (r1.r.p2.x < r2.r.p2.x) ? r1.r.p2.x : r2.r.p2.x; /* min */
  rg.r.p1.y = (r1.r.p1.y > r2.r.p1.y) ? r1.r.p1.y : r2.r.p1.y; /* max */
  rg.r.p2.y = (r1.r.p2.y < r2.r.p2.y) ? r1.r.p2.y : r2.r.p2.y; /* min */

  if (rg.r.p1.x >= rg.r.p2.x || rg.r.p1.y >= rg.r.p2.y)
  {
    /* pas d'intersection */
    rg.r.p1.y = 0;
    rg.r.p1.x = 0;
    rg.r.p2.y = 0;
    rg.r.p2.x = 0;
  }

  return rg;
}

/* --------- */
/* PutRegion */
/* --------- */

/*
    Ajoute une région rectangulaire dans une liste ListReg.
        *listregion	->	liste dans laquelle il faut ajouter
        region		->	région à ajouter
        update		->	0 = redessine seulement si icône en mouvement dedans
                        1 = redessine toujours (dàcor changé)
 */

static short
PutRegion (listreg * listregion, Reg region, short update)
{
  short i;
  Reg * pr;
  Reg   clip;
  Reg   r;

  clip = AndRegion (
    region,
    (r.r.p1.y = 0, r.r.p1.x = 0, r.r.p2.y = DIMYDRAW, r.r.p2.x = DIMXDRAW, r));

  for (i = 0; i < MAXREGION; i++)
  {
    pr = &listregion[i].reg;
    if (IfNilRegion (*pr))
    {
      *pr                  = clip; /* met une nouvelle région */
      listregion[i].update = update;
      return 0;
    }

    if (IfSectRegion (*pr, clip))
    {
      *pr = OrRegion (*pr, clip); /* agrandit la région existante */
      listregion[i].update |= update;
      return 0;
    }
  }
  return 1; /* erreur, plus de place libre */
}

/* ---------- */
/* IconRegion */
/* ---------- */

/*
    Donne la région occupée par une icône et son ombre éventuelle.
 */

static Reg
IconRegion (short i, Pt pos)
{
  Pixmap pm;
  Reg    rg;

  GetSprite (&pm, i, 0); /* donne juste les dimensions */

  rg.r.p1.x = pos.x;
  rg.r.p1.y = pos.y; /* coin sup/gauche */
  rg.r.p2.x = pos.x + pm.dx;
  rg.r.p2.y = pos.y + pm.dy; /* coin inf/droite */

  return rg; /* retourne la région */
}

/* ----------- */
/* IconDrawOne */
/* ----------- */

typedef struct {
  Pt  p1;
  Pt  p2;
  Pt  dim;
  int icon;
} SuperCelHover;

/*
    Dessine une icône (chair+fond+ombre) dans un pixmap quelconque.
    Ne dépasse pas de la région de clipping donnée.
 */

static SuperCelHover
IconDrawOne (
  short i, short super, Pt pos, short posz, Pt cel, Reg clip, Pixmap * ppm)
{
  Pixmap        pmicon; /* pixmap de l'icône à dessiner */
  Reg           use;    /* région à utiliser */
  Pt            p1, dim;
  SuperCelHover hover = {0};

  use = AndRegion (clip, IconRegion (i, pos));
  if (IfNilRegion (use))
    return hover; /* retour si rien à dessiner */

  /* Récupère la liste des élément de décor à redessiner */
  const ImageStack * list =
    DecorIconMask (pos, posz, cel); /* fabrique le masque */

  GetSprite (&pmicon, i, 1); /* cherche le pixmap de la chair */
  if (super == 1)
    SDL_SetTextureAlphaMod (pmicon.texture, 150);
  else if (super == 2)
  {
    SDL_SetTextureAlphaMod (pmicon.texture, 200);
    if (g_theme == 0)
      SDL_SetTextureColorMod (pmicon.texture, 0x22, 0xE2, 0x58);
    else
      SDL_SetTextureColorMod (pmicon.texture, 0xFF, 0xAA, 0xFF);
  }
  else
    SDL_SetTextureAlphaMod (pmicon.texture, 255);

  p1.y  = use.r.p1.y - pos.y;
  p1.x  = use.r.p1.x - pos.x;
  dim.y = use.r.p2.y - use.r.p1.y;
  dim.x = use.r.p2.x - use.r.p1.x;

  /* crop blupi (or tank, etc) when it falls
   * 15..15 blupi drinks too much
   * 28..31 blupi is falling
   * 47 baloon
   * 63 blupi baloon
   */
  if (
    ((i >= 14 && i <= 15) || (i >= 28 && i <= 31) || i == 47 || i == 63 ||
     i == ICO_CAISSE || i == ICO_CAISSEV || i == ICO_CAISSEO ||
     i == ICO_CAISSEV || i == ICO_CAISSEG ||
     (i >= ICO_TANK_E && i <= ICO_TANK_S) || i == ICO_TANK_X ||
     i == ICO_TANK_EO || i == ICO_TANK_NS) &&
    posz > 0)
  {
    // dim.y -= posz;
    Pixmap mask;
    Pt     p0 = {0, 0}, p2 = {0, 0};

    Pixmap pmtemp;
    pmtemp.texture = SDL_CreateTexture (
      g_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, LXICO,
      LYICO);
    SDL_SetTextureBlendMode (pmtemp.texture, SDL_BLENDMODE_BLEND);
    pmtemp.dx     = LXICO;
    pmtemp.dy     = LYICO;
    pmtemp.orig.x = 0;
    pmtemp.orig.y = 0;

    Pt maskDim = {LYICO, LXICO};
    GetSprite (&mask, ICO_SOLMASK, 1);
    CopyPixel (&mask, p0, &pmtemp, p0, maskDim);

    /* Calcul la position du "toto" sans la chute */
    Pt absPos = {pos.y + POSYDRAW + LYICO - posz, pos.x + POSXDRAW + LXICO};
    /* Calcul la cellule où se situe le trou */
    Pt holeCel = GraToCel (absPos);
    /* Récupère les coordonnées du trou */
    Pt holeCoords = CelToGra2 (holeCel, SDL_TRUE);

    holeCoords.x -= POSXDRAW;
    holeCoords.y -= POSYDRAW;

    /* Evite de dessiner en dessus du masque */
    Pt cropDim = {LYICO, LXICO};
    cropDim.y -= use.r.p1.y - holeCoords.y;
    cropDim.x -= use.r.p1.x - holeCoords.x;

    /* copie le fond dans le "masque" */
    SDL_SetTextureBlendMode (ppm->texture, SDL_BLENDMODE_MOD);
    if (holeCoords.y < 0)
    {
      maskDim.y += holeCoords.y;
      p2.y         = -holeCoords.y;
      holeCoords.y = 0;
    }
    if (holeCoords.x < 0)
    {
      maskDim.x += holeCoords.x;
      p2.x         = -holeCoords.x;
      holeCoords.x = 0;
    }
    if (holeCoords.y + LYICO >= DIMYDRAW)
      maskDim.y -= holeCoords.y + LYICO - DIMYDRAW;
    if (holeCoords.x + LXICO >= DIMXDRAW)
      maskDim.x -= holeCoords.x + LXICO - DIMXDRAW;
    CopyPixel (ppm, holeCoords, &pmtemp, p2, maskDim);
    SDL_SetTextureBlendMode (ppm->texture, SDL_BLENDMODE_BLEND);

    /* Dessine le "toto" */
    CopyPixel (&pmicon, p1, ppm, use.r.p1, cropDim);

    /* Utilise le "masque" par dessus */
    CopyPixel (&pmtemp, p2, ppm, holeCoords, maskDim);
    SDL_DestroyTexture (pmtemp.texture);
  }
  else
  {
    /* Dessine le "toto" */
    CopyPixel (&pmicon, p1, ppm, use.r.p1, dim);
  }

  if (super > 0)
  {
    SDL_SetTextureAlphaMod (pmicon.texture, 255);
    SDL_SetTextureColorMod (pmicon.texture, 255, 255, 255);
  }

  /* Redessine des éléments de décors qui doivent apparaître devant les "toto"
   */
  for (int j = 0; list[j].icon; ++j)
  {
    Pixmap pmicon2;
    GetSprite (&pmicon2, list[j].icon, 1);

    SDL_bool blupiBaloonStart =
      (((i == 63 || i == 47) && posz < 20) || (i != 63 && i != 47));

    if (list[j].super && blupiBaloonStart)
    {
      dim.y = pmicon2.dy;
      dim.x = pmicon2.dx;

      hover.p1.y = 0;
      hover.p1.x = 0;
      hover.p2   = list[j].off;
      hover.dim  = list[j].dim;
      hover.icon = list[j].icon;

      /* Special case where a "toto" is on the ground */
      SDL_bool isGround =
        (hover.icon >= ICO_SOL && hover.icon < ICO_SOLMAX) ||
        hover.icon == ICO_SOLDALLE3 || hover.icon == ICO_SOLDALLE4 ||
        hover.icon == ICO_SOLDALLE5 || hover.icon == ICO_TROU ||
        hover.icon == ICO_TROUBOUCHE || hover.icon == ICO_SENSUNI_E ||
        hover.icon == ICO_SENSUNI_O || hover.icon == ICO_SENSUNI_N ||
        hover.icon == ICO_SENSUNI_S || hover.icon == ICO_UNSEUL;

      /* We need the original image (no redraw) */
      if (isGround)
        continue;
    }

    Pt p1 = {0, 0};
    Pt p2 = list[j].off;
    dim   = list[j].dim;
    CopyPixel    /* dessine la chair */
      (&pmicon2, /* source */
       p1, ppm,  /* destination */
       p2, dim);
  }

  return hover;
}

/* =========== */
/* IconDrawAll */
/* =========== */

/*
    Indique qu'il faudra tout redessiner, avant un IconDrawOpen/Put(s)/Close.
 */

void
IconDrawAll (void)
{
  /* libère toute la table */
  memset (ListRegNew, 0, sizeof (ListRegNew));

  ListRegNew[0].reg.r.p1.y = 0;
  ListRegNew[0].reg.r.p1.x = 0;
  ListRegNew[0].reg.r.p2.y = DIMYDRAW;
  ListRegNew[0].reg.r.p2.x = DIMXDRAW;
  ListRegNew[0].update     = 1;
}

/* ============= */
/* IconDrawFlush */
/* ============= */

/*
    Vide tous les buffers internes.
 */

void
IconDrawFlush (void)
{
  memset (
    ListIconDrawNew, 0, sizeof (ListIconDrawNew)); /* libère toute la table */
}

/* ============ */
/* IconDrawOpen */
/* ============ */

/*
    Prépare le dessin caché en mémoire de plusieurs icônes.
    Transfert la liste ListIconDrawNew dans ListIconDrawOld puis vide la
   première. Transfert la liste ListRegNew dans ListRegOld puis vide la
   première.
 */

void
IconDrawOpen (void)
{
  /* table old <-- new */
  memcpy (ListIconDrawOld, ListIconDrawNew, sizeof (ListIconDrawOld));
  /* libère toute la table */
  memset (ListIconDrawNew, 0, sizeof (ListIconDrawNew));

  /* table old <-- new */
  memcpy (ListRegOld, ListRegNew, sizeof (ListRegOld));
  /* libère toute la table */
  memset (ListRegNew, 0, sizeof (ListRegNew));
}

/* =========== */
/* IconDrawPut */
/* =========== */

/*
    Met une icône à dessiner dans ListIconDrawNew,
    et met à jour les tables ListRegNew et ListRegOld.
    Chaque icône possède une région de clipping de laquelle elle ne
    doit pas dépasser.
    Si btransp == 1, ne dessine pas le fond (transparent).
 */

short
IconDrawPut (short ico, short btransp, Pt pos, short posz, Pt cel, Reg clip)
{
  short i;
  Reg   rg;

  for (i = 0; i < MAXICONDRAW; i++)
  {
    if (ListIconDrawNew[i].icon == 0)
    {
      ListIconDrawNew[i].icon    = ico;
      ListIconDrawNew[i].btransp = btransp;
      ListIconDrawNew[i].pos     = pos;
      ListIconDrawNew[i].posz    = posz;
      ListIconDrawNew[i].cel     = cel;
      ListIconDrawNew[i].clip    = clip;

      rg                      = IconRegion (ico, pos);
      ListIconDrawNew[i].bbox = rg;
      PutRegion (ListRegNew, rg, 0);
      PutRegion (ListRegOld, rg, 0);

      return 0; /* retour ok */
    }
  }

  return 1; /* erreur, table pleine */
}

/* -------------- */
/* IconDrawIfMove */
/* -------------- */

/*
    Regarde si une icône a changé de place, ou si elle n'est pas seule dans sa
   région. Si oui retourne 1 (true) car il faut la dessiner, si non retourne 0
   (false).
 */

static short
IconDrawIfMove (listreg lrg)
{
  short i, j, n;

  /* Regarde s'il s'agit d'une région à redessiner de toute façon */

  if (lrg.update != 0)
    return 1;

  /* Cherche l'icône dans la région, seulement s'il a en a une seule. */

  n = 0;
  for (i = 0; i < MAXICONDRAW && ListIconDrawNew[i].icon; i++)
    if (IfSectRegion (ListIconDrawNew[i].bbox, lrg.reg))
    {
      n++;
      if (n > 1)
        return 1; /* dessin car plusieurs icônes dans région */
      j = i;
    }

  if (n == 0)
    return 1; /* dessin si rien trouvé */

  /* Cherche si l'icône était déjà là lors du dessin précédent. */

  for (i = 0; i < MAXICONDRAW && ListIconDrawOld[i].icon; i++)
    if (
      ListIconDrawOld[i].icon == ListIconDrawNew[j].icon &&
      ListIconDrawOld[i].pos.x == ListIconDrawNew[j].pos.x &&
      ListIconDrawOld[i].pos.y == ListIconDrawNew[j].pos.y)
      goto next;

  return 1; /* dessin car icône n'était pas là avant */

  /* Cherche s'il y avait plus d'une icône avant. */

next:
  n = 0;
  for (i = 0; i < MAXICONDRAW && ListIconDrawOld[i].icon; i++)
    if (IfSectRegion (ListIconDrawOld[i].bbox, lrg.reg))
    {
      n++;
      if (n > 1)
        return 1; /* dessin car plusieurs icônes avant */
    }

  return 0; /* dessin superflu */
}

/* ============== */
/* IconDrawUpdate */
/* ============== */

/*
    Indique une zone rectangulaire qui devra être réaffichée,
    par exemple parce qu'une partie du dàcor é changé.
 */

void
IconDrawUpdate (Reg rg)
{
  PutRegion (ListRegOld, rg, 1);
  PutRegion (ListRegNew, rg, 1);
}

/* ============= */
/* IconDrawClose */
/* ============= */

/*
    Dessine effectivement toutes les icônes données avec IconDrawPut,
    comprises dans une région de clipping.
 */

void
IconDrawClose (short bdraw)
{
  short         i, j;
  Pt            p1, p2, dim;
  Reg           r, ro;
  Pixmap *      ppmdecor;
  SuperCelHover hover = {0};

  ppmdecor = DecorGetPixmap ();

  for (i = 0; i < MAXREGION; i++)
  {
    if (IfNilRegion (ListRegOld[i].reg))
      continue;
    if (!IconDrawIfMove (ListRegOld[i]))
      continue;

    p1.y = ListRegOld[i].reg.r.p2.y - ListRegOld[i].reg.r.p1.y;
    p1.x = ListRegOld[i].reg.r.p2.x - ListRegOld[i].reg.r.p1.x;
    CopyPixel /* met le décor de fond */
      (ppmdecor, ListRegOld[i].reg.r.p1, &pmwork, ListRegOld[i].reg.r.p1, p1);

    for (j = 0; j < MAXICONDRAW && ListIconDrawNew[j].icon; j++)
      if (IfSectRegion (ListRegOld[i].reg, ListIconDrawNew[j].bbox))
      {
        SuperCelHover _hover = IconDrawOne /* dessine l'icône */
          (ListIconDrawNew[j].icon, ListIconDrawNew[j].btransp,
           ListIconDrawNew[j].pos, ListIconDrawNew[j].posz,
           ListIconDrawNew[j].cel, ListIconDrawNew[j].clip, &pmwork);
        if (_hover.icon)
          hover = _hover;
      }

    if (hover.icon)
    {
      Pixmap tmp    = {0};
      Pixmap pmicon = {0};

      if (g_typejeu == 1)
      {
        hover.dim.y = LYICO;
        hover.dim.x = LXICO;
        GetPixmap (&tmp, hover.dim, 2, 0);
        GetSprite (&pmicon, ICO_CELARROWS, 1);
        DuplPixel (&pmicon, &tmp);
        SDL_SetTextureAlphaMod (tmp.texture, 128);
      }
      else
      {
        hover.dim.y = LYICO;
        hover.dim.x = LXICO;
        GetPixmap (&tmp, hover.dim, 2, 0);
        GetSprite (&pmicon, g_superInvalid ? ICO_CROIX : hover.icon, 1);
        DuplPixel (&pmicon, &tmp);
        SDL_SetTextureAlphaMod (tmp.texture, 128);
        SDL_SetTextureColorMod (tmp.texture, 32, 32, 32);
      }

      CopyPixel             /* dessine la chair */
        (&tmp,              /* source */
         hover.p1, &pmwork, /* destination */
         hover.p2, hover.dim);

      GivePixmap (&tmp);
    }

    r.r.p1.y = 0;
    r.r.p1.x = 0;
    r.r.p2.y = DIMYDRAW;
    r.r.p2.x = DIMXDRAW;
    ro       = AndRegion (ListRegOld[i].reg, r);
    if (bdraw && !(IfNilRegion (ro)))
    {
      p1.y  = ro.r.p1.y;
      p1.x  = ro.r.p1.x;
      p2.y  = POSYDRAW + ro.r.p1.y;
      p2.x  = POSXDRAW + ro.r.p1.x;
      dim.y = ro.r.p2.y - ro.r.p1.y;
      dim.x = ro.r.p2.x - ro.r.p1.x;
      CopyPixel /* met l'image dans l'écran */
        (&pmwork, p1, 0, p2, dim);
    }
  }

  g_updatescreen = 0; /* l'écran est à jour */
}

/* ======== */
/* IconOpen */
/* ======== */

/*
    Ouverture générale.
 */

short
IconOpen (void)
{
  short err;
  Pt    p;

  /* Ouvre le pixmap de travail. */

  p.y = DIMYDRAW;
  p.x = DIMXDRAW;
  err = GetPixmap (&pmwork, p, 0, 1);
  if (err)
    return err;

  /* Ouvre le pixmap pour copier une icône. */

  p.y = LYICO;
  p.x = LXICO;
  err = GetPixmap (&pmcopy, p, 0, 1);
  if (err)
    return err;

  return 0;
}

/* ========= */
/* IconClose */
/* ========= */

/*
    Fermeture générale.
 */

void
IconClose (void)
{
  GivePixmap (&pmwork);
  GivePixmap (&pmcopy);
}

/* ============== */
/* IconGetPixmap */
/* ============== */

/*
    Donne le pointeur au pixmap contenant les icônes.
 */

Pixmap *
IconGetPixmap (void)
{
  return &pmwork;
}

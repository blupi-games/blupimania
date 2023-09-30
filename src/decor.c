
/* ========== */
/* bm_decor.c */
/* ========== */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "actions.h"
#include "blupimania.h"
#include "icon.h"

/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Monde * pmonde; /* pointe la description du monde */

static short  imonde[MAXCELY][MAXCELX]; /* cellules du monde initial */
static Pixmap pmdecor = {0};            /* pixmap du décor de fond */
static Pixmap pmsuper = {0};            /* pixmap de la super cellule */
static Pixmap pmsback = {0};            /* pixmap de la super cellule sauvée */
static short  superinv;                 /* 1 -> super cellule allumée  */
static Pt     supercel;                 /* super cellule visée par la souris */
static Pt     superpos;                 /* position graphique super cellule */
static Pt     ovisu;                    /* origine 1ère cellule (0;0) */
static Pt     lastovisu;                /* dernière origine partie visible */
static Pt     lastpmouse;               /* dernière position de la souris */
static short  lastsensuni;              /* dernier sens-unique posé */
static short  lastaccel;                /* dernier accélérateur posé */
static short  lastcaisse;               /* dernière caisse posée */
static short  lasttank;                 /* dernier tank posé */

SDL_bool g_superInvalid;

typedef struct {
  Pt    ovisu;       /* origine 1ère cellule (0;0) */
  short reserve[10]; /* réserve */
} Partie;

void         DecorShift (Pt oldpos, Pt newpos, short bDraw);
static short IfHideIcon (Pt pos, Rectangle zone);

/* ======== */
/* GraToCel */
/* ======== */

/*
    Conversion d'une position graphique dans l'écran
    en une coordonnée dans une cellule.
        Pt  ->	[gra]
        cel <-	[monde]
 */

Pt
GraToCel (Pt gra)
{
  Pt cel;

  gra.x -= POSXDRAW + PLXICO * ovisu.x + LXICO / 2 - 5;
  gra.y -= POSYDRAW + PRYICO * ovisu.y + LYICO / 2 + 11;

  cel.x =
    (PRXICO * gra.y + PRYICO * gra.x) / (PRYICO * PLXICO + PRXICO * PLYICO);
  cel.y =
    (PLXICO * gra.y - PLYICO * gra.x) / (PRYICO * PLXICO + PRXICO * PLYICO);

  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    goto error;

  return cel;

error:
  cel.x = -1;
  cel.y = -1;
  return cel;
}

/* ======== */
/* CelToGra */
/* ======== */

/*
    Conversion d'une coordonnée dans une cellule
    en une position graphique dans l'écran.
        cel ->	[monde]
        Pt  <-	[gra]
 */

Pt
CelToGra (Pt cel)
{
  Pt gra;

  gra.x = PLXICO * cel.x - PRXICO * cel.y;
  gra.y = PRYICO * cel.y + PLYICO * cel.x;

  return gra;
}

Pt
CelToGra2 (Pt cel, SDL_bool shift)
{
  Pt gra;

  gra.x = PLXICO * cel.x - PRXICO * cel.y;
  gra.y = PRYICO * cel.y + PLYICO * cel.x;

  if (shift)
  {
    gra.x += POSXDRAW + PLXICO * ovisu.x - PLXICO;
    gra.y += POSYDRAW + PRYICO * ovisu.y - PLYICO;
  }

  return gra;
}

/* =========== */
/* DecorGetCel */
/* =========== */

/*
    Retourne l'icône occupant une cellule donnée.
    Retourne -1 si les coordonnéés sont hors du monde !
 */

short
DecorGetCel (Pt cel)
{
  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    return -1; /* sort du monde */

  return pmonde->tmonde[cel.y][cel.x];
}

/* =========== */
/* DecorPutCel */
/* =========== */

/*
    Modifie l'icône occupant une cellule donnée.
 */

void
DecorPutCel (Pt cel, short icon)
{
  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    return; /* sort du monde */

  pmonde->tmonde[cel.y][cel.x] = icon;
}

/* ------ */
/* GetSol */
/* ------ */

/*
    Retourne le sol à mettre à un endroit donné.
    Cherche autour comment est le sol.
    spec = 0	->	ne cherche que les sols sur lesquels on peut poser
    spec = 1	->	cherche tous les sols
 */

short
GetSol (Pt cel, short spec)
{
  short i = 0;
  Pt    pos;
  short max, icon;

  static char table[] = {
    +1, +1, //
    0, +1,  //
    -1, +1, //
    -1, 0,  //
    -1, -1, //
    0, -1,  //
    +1, -1, //
    +1, 0,  //
            //
    +2, +2, //
    +1, +2, //
    0, +2,  //
    -1, +2, //
    -2, +2, //
    -2, +1, //
    -2, 0,  //
    -2, -1, //
    -2, -2, //
    -1, -2, //
    0, -2,  //
    +1, -2, //
    +2, -2, //
    +2, -1, //
    +2, 0,  //
    +2, +1, //
            //
    +3, +3, //
    +2, +3, //
    +1, +3, //
    0, +3,  //
    -1, +3, //
    -2, +3, //
    -3, +3, //
    -3, +2, //
    -3, +1, //
    -3, 0,  //
    -3, -1, //
    -3, -2, //
    -3, -3, //
    -2, -3, //
    -1, -3, //
    0, -3,  //
    +1, -3, //
    +2, -3, //
    +3, -3, //
    +3, -2, //
    +3, -1, //
    +3, 0,  //
    +3, +1, //
    +3, +2, //
            //
    -100    //
  };

  if (spec)
    max = ICO_SOLMAX;
  else
    max = ICO_SOLOBJET;

  while (table[i] != -100)
  {
    pos.x = cel.x + table[i + 0];
    pos.y = cel.y + table[i + 1];
    icon  = DecorGetCel (pos);
    if (icon != -1 && icon < max)
      return icon;
    i += 2;
  }

  return ICO_SOLCARRE;
}

/* =============== */
/* DecorGetInitCel */
/* =============== */

/*
    Retourne l'icône de sol occupant une cellule donnée lorsque le monde
    a été initialisé.
    Retourne -1 si les coordonnéés sont hors du monde !
 */

short
DecorGetInitCel (Pt cel)
{
  short icon;

  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    return -1; /* sort du monde */

  icon = imonde[cel.y][cel.x];

  if (
    icon < ICO_SOLMAX || icon == ICO_BAISSEBAS || icon == ICO_UNSEUL ||
    (icon >= ICO_SENSUNI_S && icon <= ICO_SENSUNI_O) ||
    (icon >= ICO_ACCEL_S && icon <= ICO_ACCEL_O))
    return icon;

  return GetSol (cel, 0);
}

/* =============== */
/* DecorPutInitCel */
/* =============== */

/*
    Modifie l'icône de sol occupant une cellule donnée lorsque le monde
    a été initialisé.
 */

void
DecorPutInitCel (Pt cel, short icon)
{
  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    return; /* sort du monde */

  imonde[cel.y][cel.x] = icon;
}

/* ============= */
/* DecorIconMask */
/* ============= */

/*
    Fabrique le masque permettant de dessiner une icône sur une cellule
    tout en étant masquée par les éléments du décor placés devant.
    La table[] donne les coordonnées relatives des cellules succeptibles
    de masquer l'objet placé dans la cellule (0;0).
    Voir l'explication de cette table[] dans IMASK.IMAGE !
        ppm ->	pixmap ayant les dimensions d'une icône (80x80)
        pos ->	coordonnées exactes de l'icône à masquer [gra]
        cel ->	coordonnées de la cellule charnière [monde]
 */

const ImageStack *
DecorIconMask (Pt pos, short posz, Pt cel)
{
  static ImageStack list[(MAXCELY + 1) * (MAXCELX + 2)];
  memset (list, 0, sizeof (list));

  short icon;

  Rectangle zone;
  zone.p1.x = 0;
  zone.p1.y = 0;
  zone.p2.x = DIMXDRAW;
  zone.p2.y = DIMYDRAW;

  int k        = 0;
  Pt  pv       = pos;
  Pt  superCel = GetSuperCel ();

  int i0 = cel.y > 0 ? cel.y - 1 : 0;
  int j0 = cel.x > 0 ? cel.x - 1 : 0;

  for (int i = i0; i <= MAXCELY; i++)
  {
    Pt ph = pv;
    for (int j = j0; j <= MAXCELX; j++)
    {
      int superSet = -1;

      if (i < cel.y && j - cel.x < 1)
        continue;
      if (j < cel.x && i - cel.y < 1)
        continue;
      if (IfHideIcon (ph, zone))
        continue;

      Pt c   = {i, j};
      Pt dim = {LYICO, LXICO};

      if (c.x < MAXCELX && c.y < MAXCELY)
        icon = pmonde->tmonde[c.y][c.x];
      else
        continue; // icon = 0;

      if (superCel.x > -1 && c.x == superCel.x && c.y == superCel.y)
      {
        list[k].super = SDL_TRUE;
        list[k].icon  = icon;
        list[k].cel   = c;

        list[k].off = CelToGra (c);
        list[k].off.x += PLXICO * (ovisu.x);
        list[k].off.y += PRYICO * (ovisu.y);
        list[k].dim = dim;
        superSet    = k;
        k++;
      }

      if (
        i == cel.y && j == cel.x && icon >= ICO_PORTEO_EO &&
        icon < ICO_PORTEO_EO + 6)
        continue;

      /* MS: prevent redraw when Blupi is on this sort of case */
      if (icon >= ICO_SENSUNI_S && icon <= ICO_SENSUNI_O)
        continue;
      if (icon >= ICO_ACCEL_S && icon <= ICO_ACCEL_O)
        continue;
      if (icon == ICO_UNSEUL)
        continue;
      if (
        icon == ICO_ARRIVEE || icon == ICO_ARRIVEEPRIS ||
        icon == ICO_ARRIVEEBOUM || icon == ICO_ARRIVEEVIDE)
      {
        /* Consider to redraw only the ballon part (not the ground)
         * Note that the sprites are a bit special here. At the top
         * a black line seems missing and it's not an error. it's
         * necessary here in orer to redraw properly the ballon part.
         */
        dim.y = 51;
        if (superSet >= 0)
          list[superSet].dim = dim;
      }

      if (
        c.x < MAXCELX && c.y < MAXCELY &&
        (icon >= ICO_BLOQUE || icon == ICO_DEPART ||
         icon == ICO_ARRIVEEVIDE)) /* icône en hauteur ? */
      {
        list[k].icon = icon;
        list[k].cel  = c;

        list[k].off = CelToGra (c);
        list[k].off.x += PLXICO * (ovisu.x);
        list[k].off.y += PRYICO * (ovisu.y);
        list[k].dim = dim;
        k++;
      }
      else if (
        posz > 0 && /* icône en dessous du sol ? */
        (i > cel.y || j > cel.x ||
         (icon != ICO_DEPART && icon != ICO_TROU && icon != ICO_TROUBOUCHE)))
      {
        list[k].icon = icon;
        list[k].cel  = c;

        list[k].off = CelToGra (c);
        list[k].off.x += PLXICO * (ovisu.x);
        list[k].off.y += PRYICO * (ovisu.y);
        list[k].dim = dim;
        k++;
      }
    }
  }

  return list;
}

/* ------------ */
/* MurGetConnex */
/* ------------ */

/*
    Retourne les directions autour d'une cellule occupées par des murs.
    Ces directions devront être connectées.
 */

short
MurGetConnex (Pt cel)
{
  short icon;
  short connex = 0;

  cel.x++;
  if (cel.x < MAXCELX)
  {
    icon = DecorGetCel (cel);
    if (
      (icon >= ICO_MURHAUT && icon <= ICO_MURHAUT_D) || /* est-ce un mur ? */
      (icon >= ICO_MURBAS && icon <= ICO_MURBAS_D) ||
      (icon >= ICO_BARRIERE && icon <= ICO_BARRIERE_D) ||
      (icon >= ICO_VITRE && icon <= ICO_VITRE_D) ||
      (icon >= ICO_PORTEF_EO && icon < ICO_PORTEF_EO + 6))
      connex |= 1 << 0; /* est */
  }

  cel.x--;
  cel.y++;
  if (cel.y < MAXCELY)
  {
    icon = DecorGetCel (cel);
    if (
      (icon >= ICO_MURHAUT && icon <= ICO_MURHAUT_D) || /* est-ce un mur ? */
      (icon >= ICO_MURBAS && icon <= ICO_MURBAS_D) ||
      (icon >= ICO_BARRIERE && icon <= ICO_BARRIERE_D) ||
      (icon >= ICO_VITRE && icon <= ICO_VITRE_D) ||
      (icon >= ICO_PORTEF_EO && icon < ICO_PORTEF_EO + 6))
      connex |= 1 << 1; /* sud */
  }

  cel.x--;
  cel.y--;
  if (cel.x >= 0)
  {
    icon = DecorGetCel (cel);
    if (
      (icon >= ICO_MURHAUT && icon <= ICO_MURHAUT_D) || /* est-ce un mur ? */
      (icon >= ICO_MURBAS && icon <= ICO_MURBAS_D) ||
      (icon >= ICO_BARRIERE && icon <= ICO_BARRIERE_D) ||
      (icon >= ICO_VITRE && icon <= ICO_VITRE_D) ||
      (icon >= ICO_PORTEF_EO && icon < ICO_PORTEF_EO + 6))
      connex |= 1 << 2; /* ouest */
  }

  cel.x++;
  cel.y--;
  if (cel.y >= 0)
  {
    icon = DecorGetCel (cel);
    if (
      (icon >= ICO_MURHAUT && icon <= ICO_MURHAUT_D) || /* est-ce un mur ? */
      (icon >= ICO_MURBAS && icon <= ICO_MURBAS_D) ||
      (icon >= ICO_BARRIERE && icon <= ICO_BARRIERE_D) ||
      (icon >= ICO_VITRE && icon <= ICO_VITRE_D) ||
      (icon >= ICO_PORTEF_EO && icon < ICO_PORTEF_EO + 6))
      connex |= 1 << 3; /* nord */
  }

  return connex;
}

/* -------- */
/* MurBuild */
/* -------- */

/*
    Met un mur dans une cellule, et raccorde les cellules voisines.
 */

void
MurBuild (Pt cel, short type)
{
  short icon, oldicon, newicon;
  Pt    celinit = cel;
  short i;

  static short tmurs[] = {
    ICO_MURBAS + MUR_NSEO, /* ---- */
    ICO_MURBAS + MUR_EO,   /* ---E */
    ICO_MURBAS + MUR_NS,   /* --S- */
    ICO_MURBAS + MUR_SE,   /* --SE */
    ICO_MURBAS + MUR_EO,   /* -O-- */
    ICO_MURBAS + MUR_EO,   /* -O-E */
    ICO_MURBAS + MUR_SO,   /* -OS- */
    ICO_MURBAS + MUR_SEO,  /* -OSE */
    ICO_MURBAS + MUR_NS,   /* N--- */
    ICO_MURBAS + MUR_NE,   /* N--E */
    ICO_MURBAS + MUR_NS,   /* N-S- */
    ICO_MURBAS + MUR_ENS,  /* N-SE */
    ICO_MURBAS + MUR_NO,   /* NO-- */
    ICO_MURBAS + MUR_NEO,  /* NO-E */
    ICO_MURBAS + MUR_ONS,  /* NOS- */
    ICO_MURBAS + MUR_NSEO  /* NOSE */
  };

  static short tpos[4 * 2] = {+1, 0, 0, +1, -1, 0, 0, -1};

  icon = tmurs[MurGetConnex (cel)];
  if (type == 0)
  {
    if (GetRandom (1, 0, 2) == 0)
      icon -= 16;
    oldicon = DecorGetCel (cel);
    if (oldicon >= ICO_MURHAUT && oldicon <= ICO_MURHAUT_D)
      icon = oldicon + 16;
    if (oldicon >= ICO_MURBAS && oldicon <= ICO_MURBAS_D)
      icon = oldicon - 16;
  }
  if (type == 1)
    icon += ICO_BARRIERE - ICO_MURBAS;
  if (type == 2)
    icon += ICO_VITRE - ICO_MURBAS;
  DecorModif (cel, icon);

  for (i = 0; i < 4; i++)
  {
    cel = celinit;
    cel.x += tpos[i * 2 + 0];
    cel.y += tpos[i * 2 + 1];

    icon = DecorGetCel (cel);
    if (type == 0)
    {
      if (
        (icon >= ICO_MURHAUT && icon <= ICO_MURHAUT_D) || /* est-ce un mur ? */
        (icon >= ICO_MURBAS && icon <= ICO_MURBAS_D))
      {
        newicon = tmurs[MurGetConnex (cel)];
        if (icon <= ICO_MURHAUT_D)
          newicon -= 16; /* est-un mur haut ? */
        DecorModif (cel, newicon);
      }
    }
    if (type == 1)
    {
      if (icon >= ICO_BARRIERE && icon <= ICO_BARRIERE_D) /* est-ce une barrière
                                                             ? */
        DecorModif (cel, tmurs[MurGetConnex (cel)] + ICO_BARRIERE - ICO_MURBAS);
    }
    if (type == 2)
    {
      if (icon >= ICO_VITRE && icon <= ICO_VITRE_D) /* est-ce une vitre ? */
        DecorModif (cel, tmurs[MurGetConnex (cel)] + ICO_VITRE - ICO_MURBAS);
    }
  }
}

/* --------- */
/* BoisBuild */
/* --------- */

/*
    Met un tas de bois sur une cellule.
 */

void
BoisBuild (Pt cel)
{
  short icon, newb, flag;
  Pt    voisin;

  /* Cherche l'orientation du nouveau tas de bois à poser. */

  icon = DecorGetCel (cel);
  if (icon >= ICO_BOIS1_NS && icon <= ICO_BOIS3_EO)
  {
    if (icon >= ICO_BOIS1_NS && icon <= ICO_BOIS3_NS)
      newb = ICO_BOIS2_EO;
    else
      newb = ICO_BOIS2_NS;
  }
  else
  {
    newb = ICO_BOIS2_NS;

    voisin.x = cel.x + 1;
    voisin.y = cel.y;
    icon     = DecorGetCel (voisin);
    if (icon >= ICO_BOIS1_EO && icon <= ICO_BOIS3_EO)
      newb = ICO_BOIS2_EO;

    voisin.x -= 2;
    icon = DecorGetCel (voisin);
    if (icon >= ICO_BOIS1_EO && icon <= ICO_BOIS3_EO)
      newb = ICO_BOIS2_EO;
  }

  /* Modifie les extrémités des tas de bois. */

  if (newb == ICO_BOIS2_EO)
  {
    flag = 0;

    voisin.x = cel.x + 1;
    voisin.y = cel.y;
    icon     = DecorGetCel (voisin);
    if (icon >= ICO_BOIS1_EO && icon <= ICO_BOIS3_EO)
    {
      flag |= 1 << 0;
      voisin.x++;
      icon = DecorGetCel (voisin);
      voisin.x--;
      if (icon >= ICO_BOIS1_EO && icon <= ICO_BOIS3_EO)
      {
        DecorModif (voisin, ICO_BOIS2_EO);
      }
      else
      {
        DecorModif (voisin, ICO_BOIS3_EO);
      }
    }

    voisin.x = cel.x - 1;
    voisin.y = cel.y;
    icon     = DecorGetCel (voisin);
    if (icon >= ICO_BOIS1_EO && icon <= ICO_BOIS3_EO)
    {
      flag |= 1 << 1;
      voisin.x--;
      icon = DecorGetCel (voisin);
      voisin.x++;
      if (icon >= ICO_BOIS1_EO && icon <= ICO_BOIS3_EO)
      {
        DecorModif (voisin, ICO_BOIS2_EO);
      }
      else
      {
        DecorModif (voisin, ICO_BOIS1_EO);
      }
    }

    if (flag == (1 << 0))
      newb = ICO_BOIS1_EO;
    if (flag == (1 << 1))
      newb = ICO_BOIS3_EO;
    DecorModif (cel, newb);
  }
  else
  {
    flag = 0;

    voisin.x = cel.x;
    voisin.y = cel.y + 1;
    icon     = DecorGetCel (voisin);
    if (icon >= ICO_BOIS1_NS && icon <= ICO_BOIS3_NS)
    {
      flag |= 1 << 0;
      voisin.y++;
      icon = DecorGetCel (voisin);
      voisin.y--;
      if (icon >= ICO_BOIS1_NS && icon <= ICO_BOIS3_NS)
      {
        DecorModif (voisin, ICO_BOIS2_NS);
      }
      else
      {
        DecorModif (voisin, ICO_BOIS1_NS);
      }
    }

    voisin.x = cel.x;
    voisin.y = cel.y - 1;
    icon     = DecorGetCel (voisin);
    if (icon >= ICO_BOIS1_NS && icon <= ICO_BOIS3_NS)
    {
      flag |= 1 << 1;
      voisin.y--;
      icon = DecorGetCel (voisin);
      voisin.y++;
      if (icon >= ICO_BOIS1_NS && icon <= ICO_BOIS3_NS)
      {
        DecorModif (voisin, ICO_BOIS2_NS);
      }
      else
      {
        DecorModif (voisin, ICO_BOIS3_NS);
      }
    }

    if (flag == (1 << 0))
      newb = ICO_BOIS3_NS;
    if (flag == (1 << 1))
      newb = ICO_BOIS1_NS;
    DecorModif (cel, newb);
  }
}

/* ----------- */
/* IfCelValide */
/* ----------- */

/*
    Vérifie s'il est possible d'agir dans une cellule donnée avec
    un outil donné.
    Retourne 1 si c'est possible (cellule valide).
 */

short
IfCelValide (Pt cel, short outil)
{
  short obstacle, obnext, solmax;

  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    return 0;

  if (g_typejeu == 1) /* jeu avec toto télécommandé par le joueur ? */
  {
    if (DecorGetCel (cel) == -1)
      return 0;
    return 1;
  }

  obstacle = MoveGetCel (cel);
  if (obstacle != 0 && obstacle != 2)
    return 0; /* retourne si y'a un toto ici */

  obstacle = DecorGetCel (cel);

  if (g_typeedit)
    solmax = ICO_SOLMAX;
  else
    solmax = ICO_SOLOBJET;

  if (outil == ICO_OUTIL_TRACKS) /* tracks ? */
  {
    if (g_typeedit)
      return 1; /* en édition, le tracks peut tout détruire */
    if (obstacle < ICO_BLOQUE || obstacle == ICO_ARRIVEE)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_TRACKSBAR) /* tracks barrière ? */
  {
    if (obstacle >= ICO_BARRIERE && obstacle <= ICO_BARRIERE_D)
      return 1;
    return 0;
  }

  if (
    outil == ICO_OUTIL_SOLCARRE || /* sol pendant l'édition ? */
    outil == ICO_OUTIL_SOLPAVE || outil == ICO_OUTIL_SOLDALLE1 ||
    outil == ICO_OUTIL_SOLDALLE2 || outil == ICO_OUTIL_SOLDALLE3 ||
    outil == ICO_OUTIL_SOLDALLE4 || outil == ICO_OUTIL_SOLDALLE5 ||
    outil == ICO_OUTIL_SOLELECTRO || outil == ICO_OUTIL_SOLOBJET ||
    outil == ICO_OUTIL_INVINCIBLE)
    return 1;

  /* Refuse de faire autre chose si on est juste sur la case
      d'arrivée à côté de l'ascenseur. */

  if (outil != ICO_OUTIL_UNSEUL)
  {
    cel.x--;
    obnext = DecorGetCel (cel);
    cel.x++;
    if (
      obnext == ICO_DEPART || obnext == ICO_DEPARTOUV + 0 ||
      obnext == ICO_DEPARTOUV + 1 || obnext == ICO_DEPARTOUV + 2)
      return 0;
  }

  if (
    outil == ICO_OUTIL_ARRIVEE ||  /* ballon ? */
    outil == ICO_OUTIL_JOUEUR ||   /* toto pour joueur ? */
    outil == ICO_OUTIL_AIMANT ||   /* aimant ? */
    outil == ICO_OUTIL_TROU ||     /* trou ? */
    outil == ICO_OUTIL_GLISSE ||   /* glisse ? */
    outil == ICO_OUTIL_BARRIERE || /* barrière ? */
    outil == ICO_OUTIL_VITRE ||    /* vitre ? */
    outil == ICO_OUTIL_VISION ||   /* lunettes ? */
    outil == ICO_OUTIL_LIVRE ||    /* livre ? */
    outil == ICO_OUTIL_BAISSE ||   /* porte électronique ? */
    outil == ICO_OUTIL_UNSEUL ||   /* un seul toto ? */
    outil == ICO_OUTIL_MAGIC)      /* chapeau de magicien ? */
  {
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_DEPART)
  {
    if (obstacle >= solmax)
      return 0;
    cel.x++;
    obnext = DecorGetCel (cel);
    cel.x--;
    if (obnext < 0 || obnext >= solmax)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_BOIT) /* table avec boisson ? */
  {
    if (
      g_typeedit && (obstacle == ICO_TABLEBOIT || obstacle == ICO_TABLEPOISON))
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_MUR) /* mur ? */
  {
    if (g_typeedit && obstacle >= ICO_MURHAUT && obstacle <= ICO_MURHAUT_D)
      return 1;
    if (g_typeedit && obstacle >= ICO_MURBAS && obstacle <= ICO_MURBAS_D)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_BOIS) /* tas de bois ? */
  {
    if (obstacle >= ICO_BOIS1_NS && obstacle <= ICO_BOIS3_EO)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (
    outil == ICO_OUTIL_PLANTE ||  /* fleur ? */
    outil == ICO_OUTIL_PLANTEBAS) /* fleur basse ? */
  {
    if (g_typeedit && obstacle >= ICO_PLANTEBAS && obstacle <= ICO_PLANTEHAUT_D)
      return 1;
    if (obstacle >= solmax)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_TANK) /* tank ? */
  {
    if (
      (obstacle >= ICO_TANK_E && obstacle <= ICO_TANK_S) ||
      obstacle == ICO_TANK_X || obstacle == ICO_TANK_EO ||
      obstacle == ICO_TANK_NS)
      return 1;
    if (obstacle >= solmax)
      return 0;
    return 1;
  }

  if (
    outil == ICO_OUTIL_ELECTRO ||  /* électronique ? */
    outil == ICO_OUTIL_ELECTROBAS) /* électronique basse ? */
  {
    if (
      g_typeedit && obstacle >= ICO_ELECTROBAS && obstacle <= ICO_ELECTROHAUT_D)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_TECHNO) /* techno ? */
  {
    if (g_typeedit && obstacle >= ICO_TECHNO1 && obstacle <= ICO_TECHNO1_D)
      return 1;
    if (g_typeedit && obstacle >= ICO_TECHNO2 && obstacle <= ICO_TECHNO2_D)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_OBSTACLE) /* obstacle ? */
  {
    if (g_typeedit && obstacle >= ICO_OBSTACLE && obstacle <= ICO_OBSTACLE_D)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_MEUBLE) /* meuble ? */
  {
    if (g_typeedit && obstacle >= ICO_MEUBLE && obstacle <= ICO_MEUBLE_D)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_SENSUNI) /* sens-unique ? */
  {
    if (obstacle >= ICO_SENSUNI_S && obstacle <= ICO_SENSUNI_O)
      return 1;
    if (obstacle >= solmax)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_CAISSE) /* caisse ? */
  {
    if (
      obstacle == ICO_CAISSE || obstacle == ICO_CAISSEV ||
      obstacle == ICO_CAISSEO || obstacle == ICO_CAISSEG)
      return 1;
    if (obstacle >= solmax)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_ACCEL) /* accélérateur ? */
  {
    if (obstacle >= ICO_ACCEL_S && obstacle <= ICO_ACCEL_O)
      return 1;
    if (obstacle >= solmax)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_PORTE) /* porte ? */
  {
    if (obstacle >= ICO_PORTEF_EO && obstacle < ICO_PORTEF_EO + 6)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_CLE) /* clé ? */
  {
    if (obstacle >= ICO_CLE_A && obstacle <= ICO_CLE_C)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_DETONATEUR) /* détonateur ? */
  {
    if (obstacle >= ICO_DETONATEUR_A && obstacle <= ICO_DETONATEUR_C)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  if (outil == ICO_OUTIL_BOMBE) /* bombe ? */
  {
    if (obstacle >= ICO_BOMBE_A && obstacle <= ICO_BOMBE_C)
      return 1;
    if (
      obstacle >= solmax && obstacle != ICO_BAISSEBAS &&
      obstacle != ICO_CAISSEBAS && obstacle != ICO_CAISSEVBAS &&
      obstacle != ICO_CAISSEOBAS && obstacle != ICO_CAISSEGBAS)
      return 0;
    return 1;
  }

  return 0;
}

Pt
GetSuperCel ()
{
  return supercel;
}

/* =========== */
/* DecorDetCel */
/* =========== */

/*
    Détecte la cellule contenant l'objet du décor montré par la souris.
    Cherche si le point montré vise une partie haute du décor placée devant
    la base cliquée, en construisant les masques des cellules pouvant
    masquer le sol de la cellule de base.
    La table[] donne les coordonnéés relatives des cellules succeptibles
    de masquer l'objet placé dans la cellule (0;0), depuis la plus en
    avant jusqu'à la plus en arrière.
    Voir l'explication de cette table[] dans IMASK.IMAGE !
 */

Pt
DecorDetCel (Pt pos)
{
  if (
    pos.x < POSXDRAW || pos.y < POSYDRAW || pos.x > POSXDRAW + DIMXDRAW ||
    pos.y > POSYDRAW + DIMYDRAW)
    return (Pt){-1, -1};

  return GraToCel (pos); /* détection "transparente" */
}

/* ------ */
/* InvCel */
/* ------ */

/*
    Inverse rapidement une cellule.
 */

void
InvCel (Pt cel, short outil)
{
  Pt src, dst, dim;

  if (IfCelValide (cel, outil))
    return;

  src.x = 0;
  src.y = 0;
  dst   = CelToGra (cel);
  dst.x += POSXDRAW + PLXICO * ovisu.x;
  dst.y += POSYDRAW + PRYICO * ovisu.y;
  dim.x = LXICO;
  dim.y = LYICO;

  if (dst.x < POSXDRAW) /* dépasse à gauche ? */
  {
    dim.x -= POSXDRAW - dst.x;
    if (dim.x <= 0)
      return;
    src.x += POSXDRAW - dst.x;
    dst.x = POSXDRAW;
  }
  if (dst.x + dim.x > POSXDRAW + DIMXDRAW) /* dépasse à droite ? */
  {
    dim.x -= dst.x + dim.x - (POSXDRAW + DIMXDRAW);
    if (dim.x <= 0)
      return;
  }
  if (dst.y < POSYDRAW) /* dépasse en haut ? */
  {
    dim.y -= POSYDRAW - dst.y;
    if (dim.y <= 0)
      return;
    src.y += POSYDRAW - dst.y;
    dst.y = POSYDRAW;
  }
  if (dst.y + dim.y > POSYDRAW + DIMYDRAW) /* dépasse en bas ? */
  {
    dim.y -= dst.y + dim.y - (POSYDRAW + DIMYDRAW);
    if (dim.y <= 0)
      return;
  }

  DrawIcon (ICO_CROIX, src, dst, dim);
}

/* ----------- */
/* PutNewDecor */
/* ----------- */

/*
    Modifie une cellule du décor, en mettant un nouvel objet
    d'un type donné.
 */

short
PutNewDecor (Pt cel, short min, short max, short first, short limit, short add)
{
  short objet;

  objet = DecorGetCel (cel);
  if (objet >= limit + add)
    objet -= add;

  if (objet >= min && objet < max) /* y a-t-il déjà un objet de ce type ici ? */
  {
    objet++; /* oui -> met le suivant */
    if (objet >= max)
      objet = min;
  }
  else
  {
    if (first == 0)
      objet = GetRandom (1, min, max); /* non -> choix zé zazarre */
    else
      objet = first;
  }

  if (objet >= limit)
    objet += add;

  DecorModif (cel, objet); /* met un autre objet */

  return objet;
}

/* ------------- */
/* SuperCelFlush */
/* ------------- */

/*
    Faudra recalculer la super cellule.
 */

void
SuperCelFlush (void)
{
  superinv   = 0;
  supercel.x = -1; /* pas de super cellule */
  supercel.y = -1;
  superpos.x = -1; /* pas de super cellule */
  superpos.y = -1;

  lastpmouse.x = -1;
}

/* ------------ */
/* SuperCelClip */
/* ------------ */

/*
    Copie une icône dans le pixmap du décor, avec clipping selon la zone.
    Retourne 0 (false) s'il ne faut rien dessiner (clipping total).
 */

short
SuperCelClip (Pt * ppos, Pt * pdim)
{
  if (ppos->x < 0)
  {
    pdim->x += ppos->x;
    ppos->x = 0;
  }
  if (ppos->x + pdim->x > pmdecor.dx)
  {
    pdim->x -= ppos->x + pdim->x - pmdecor.dx;
  }
  if (pdim->x <= 0)
    return 0;

  if (ppos->y < 0)
  {
    pdim->y += ppos->y;
    ppos->y = 0;
  }
  if (ppos->y + pdim->y > pmdecor.dy)
  {
    pdim->y -= ppos->y + pdim->y - pmdecor.dy;
  }
  if (pdim->y <= 0)
    return 0;

  return 1;
}

/* ----------- */
/* SuperCelSet */
/* ----------- */

/*
    Allume la super cellule dans le pixmap du décor (si nécessaire).
 */

void
SuperCelSet (SDL_bool invalid)
{
  Pt    src, dst, dim;
  Reg   rg;
  short icon;

  g_superInvalid = invalid;

  if (superpos.x == -1 && superpos.y == -1)
    return;

  src   = superpos;
  dst.x = 0;
  dst.y = 0;
  dim.x = LXICO;
  dim.y = LYICO;
  if (SuperCelClip (&src, &dim))
    /* sauve la zone dans pmsback */
    CopyPixel (&pmdecor, src, &pmsback, dst, dim);

  dim.x = LXICO;
  dim.y = LYICO;

  rg.r.p1.x = superpos.x;
  rg.r.p1.y = superpos.y;
  rg.r.p2.x = superpos.x + LXICO;
  rg.r.p2.y = superpos.y + LYICO;

  icon = DecorGetCel (supercel);
  if (icon >= 0)
  {
    IconDrawPut (icon, 0, superpos, 0, supercel, rg);
    IconDrawUpdate (rg); /* faudra redessiner cette partie */
  }
}

/* ------------- */
/* SuperCelClear */
/* ------------- */

/*
    Efface la super cellule dans le pixmap du décor (si nécessaire).
 */

void
SuperCelClear (void)
{
  Pt    dst, dim;
  Reg   rg;
  short icon;

  if (superpos.x == -1 && superpos.y == -1)
    return;

  dst   = superpos;
  dim.x = LXICO;
  dim.y = LYICO;
  SuperCelClip (&dst, &dim); // FIXME: useless ?

  rg.r.p1.x = superpos.x;
  rg.r.p1.y = superpos.y;
  rg.r.p2.x = superpos.x + LXICO;
  rg.r.p2.y = superpos.y + LYICO;

  icon = DecorGetCel (supercel);
  if (icon >= 0)
  {
    IconDrawPut (icon, 0, superpos, 0, supercel, rg);
    IconDrawUpdate (rg); /* faudra redessiner cette partie */
  }
}

/* ============= */
/* DecorSuperCel */
/* ============= */

/*
    Indique la super cellule visée par la souris.
 */

void
DecorSuperCel (Pt pmouse)
{
  Pt       cel;
  SDL_bool invalid = SDL_FALSE;

  lastpmouse = pmouse;

  cel = DecorDetCel (pmouse); /* calcule la cellule montrée par la souris */

  if (!IfCelValide (cel, PaletteGetPress ()))
    invalid = SDL_TRUE;

  SuperCelClear (); /* efface l'ancienne super cellule */

  supercel = cel;
  superpos = CelToGra (cel);
  superpos.x += PLXICO * ovisu.x;
  superpos.y += PRYICO * ovisu.y;

  SuperCelSet (invalid); /* allume la nouvelle super cellule */
}

/* ========== */
/* DecorEvent */
/* ========== */

/*
    Modifie le décor en fonction d'un événement.
    Si poscel == 0  ->	tracking selon souris
    Si poscel != 0  ->	action directe sur la cellule pos
    Retourne 0 si l'événement a été traité.
 */

short
DecorEvent (Pt pos, short poscel, short outil, int key)
{
  Pt    cel;
  short init, con, first;

  if (outil < 0)
    return 1;

  if (poscel)
    cel = pos;
  else
    cel = DecorDetCel (pos); /* calcule la cellule montrée par la souris */

  if (poscel == 0 && key != KEYCLICREL) /* si l'on a pas relàché tout de suite
                                         */
    return 0;

  if (!IfCelValide (cel, outil))
    return 1;

  PaletteUseObj (outil); /* décrémente le reste à disposition */
  MoveBack (cel);        /* fait év. un pas en arrière */

  if (
    outil == ICO_OUTIL_TRACKS || /* tracks ? */
    outil == ICO_OUTIL_TRACKSBAR)
  {
    DecorModif (cel, GetSol (cel, 1));
    if (!g_typeedit)
      PlaySound (SOUND_ACTION, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLCARRE) /* sol carré ? */
  {
    DecorModif (cel, ICO_SOLCARRE);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLPAVE) /* sol pavé ? */
  {
    DecorModif (cel, ICO_SOLPAVE);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLDALLE1) /* sol dallé ? */
  {
    DecorModif (cel, ICO_SOLDALLE1);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLDALLE2) /* sol dallé ? */
  {
    DecorModif (cel, ICO_SOLDALLE2);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLDALLE3) /* sol dallé ? */
  {
    DecorModif (cel, ICO_SOLDALLE3);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLDALLE4) /* sol dallé ? */
  {
    DecorModif (cel, ICO_SOLDALLE4);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLDALLE5) /* sol dallé ? */
  {
    DecorModif (cel, ICO_SOLDALLE5);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLELECTRO) /* sol électronique ? */
  {
    PutNewDecor (cel, ICO_SOLELECTRO, ICO_SOLELECTRO_D + 1, 0, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_SOLOBJET) /* sol objet ? */
  {
    PutNewDecor (cel, ICO_SOLOBJET, ICO_SOLOBJET_D + 1, 0, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_INVINCIBLE) /* sol invincible ? */
  {
    DecorModif (cel, ICO_INVINCIBLE);
    goto termine;
  }

  if (outil == ICO_OUTIL_DEPART) /* ascenseur ? */
  {
    DecorModif (cel, ICO_DEPART);
    cel.x++;
    DecorModif (cel, ICO_SOLOBJET + 1);
    goto termine;
  }

  if (outil == ICO_OUTIL_JOUEUR) /* toto pour joueur ? */
  {
    DecorModif (cel, ICO_JOUEUR);
    goto termine;
  }

  if (outil == ICO_OUTIL_ARRIVEE) /* ballon ? */
  {
    DecorModif (cel, ICO_ARRIVEE);
    goto termine;
  }

  if (outil == ICO_OUTIL_BAISSE) /* porte électronique ? */
  {
    DecorModif (cel, ICO_BAISSE);
    goto termine;
  }

  if (outil == ICO_OUTIL_UNSEUL) /* un seul toto ? */
  {
    DecorModif (cel, ICO_UNSEUL);
    if (!g_typeedit)
      PlaySound (SOUND_UNSEUL, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_AIMANT) /* aimant ? */
  {
    DecorModif (cel, ICO_AIMANT);
    if (!g_typeedit)
      PlaySound (SOUND_AIMANT, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_TROU) /* trou ? */
  {
    DecorModif (cel, ICO_TROU);
    if (!g_typeedit)
      PlaySound (SOUND_CLIC, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_GLISSE) /* peau de banane ? */
  {
    DecorModif (cel, ICO_GLISSE);
    if (!g_typeedit)
      PlaySound (SOUND_CLIC, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_MUR) /* brique ? */
  {
    MurBuild (cel, 0); /* met un mur */
    if (!g_typeedit)
      PlaySound (SOUND_CAISSE, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_BARRIERE) /* barrière ? */
  {
    MurBuild (cel, 1); /* met une barrière */
    if (!g_typeedit)
      PlaySound (SOUND_SAUT2, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_VITRE) /* vitre ? */
  {
    MurBuild (cel, 2); /* met une vitre */
    goto termine;
  }

  if (outil == ICO_OUTIL_BOIS) /* tas de bois ? */
  {
    BoisBuild (cel); /* met un tas de bois */
    goto termine;
  }

  if (outil == ICO_OUTIL_PLANTEBAS) /* fleur basse ? */
  {
    PutNewDecor (cel, ICO_PLANTEBAS, ICO_PLANTEBAS_D + 1, 0, 0, 0);
    if (!g_typeedit)
      PlaySound (SOUND_CLIC, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_PLANTE) /* fleur haute ? */
  {
    PutNewDecor (cel, ICO_PLANTEHAUT, ICO_PLANTEHAUT_D + 1, 0, 0, 0);
    if (!g_typeedit)
      PlaySound (SOUND_CLIC, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_TANK) /* tank ? */
  {
    init = DecorGetCel (cel);
    switch (init)
    {
    case ICO_TANK_E:
      init = ICO_TANK_N;
      break;
    case ICO_TANK_N:
      init = ICO_TANK_O;
      break;
    case ICO_TANK_O:
      init = ICO_TANK_S;
      break;
    case ICO_TANK_S:
      init = ICO_TANK_EO;
      break;
    case ICO_TANK_EO:
      init = ICO_TANK_NS;
      break;
    case ICO_TANK_NS:
      init = ICO_TANK_X;
      break;
    case ICO_TANK_X:
      init = ICO_TANK_E;
      break;
    default:
      init = lasttank;
    }
    DecorModif (cel, init); /* met un tank */
    lasttank = init;
    goto termine;
  }

  if (outil == ICO_OUTIL_ELECTROBAS) /* électronique basse ? */
  {
    PutNewDecor (cel, ICO_ELECTROBAS, ICO_ELECTROBAS_D + 1, 0, 0, 0);
    if (!g_typeedit)
      PlaySound (SOUND_CAISSEO, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_ELECTRO) /* électronique haute ? */
  {
    PutNewDecor (cel, ICO_ELECTROHAUT, ICO_ELECTROHAUT_D + 1, 0, 0, 0);
    if (!g_typeedit)
      PlaySound (SOUND_CAISSEO, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_TECHNO) /* techno ? */
  {
    PutNewDecor (
      cel, ICO_TECHNO1, ICO_TECHNO1 + 10, 0, ICO_TECHNO1 + 5, 16 - 5);
    if (!g_typeedit)
      PlaySound (SOUND_CAISSEV, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_OBSTACLE) /* obstacle ? */
  {
    PutNewDecor (cel, ICO_OBSTACLE, ICO_OBSTACLE_D + 1, 0, 0, 0);
    if (!g_typeedit)
      PlaySound (SOUND_SAUT2, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_MEUBLE) /* meuble ? */
  {
    PutNewDecor (cel, ICO_MEUBLE, ICO_MEUBLE_D + 1, 0, 0, 0);
    if (!g_typeedit)
      PlaySound (SOUND_SAUT2, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_SENSUNI) /* sens-unique ? */
  {
    lastsensuni =
      PutNewDecor (cel, ICO_SENSUNI_S, ICO_SENSUNI_O + 1, lastsensuni, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_ACCEL) /* accélérateur ? */
  {
    lastaccel =
      PutNewDecor (cel, ICO_ACCEL_S, ICO_ACCEL_O + 1, lastaccel, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_VISION) /* lunettes ? */
  {
    DecorModif (cel, ICO_LUNETTES); /* met des lunettes */
    if (!g_typeedit)
      PlaySound (SOUND_CLIC, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_BOIT) /* bouteille ? */
  {
    init = DecorGetCel (cel);
    if (init == ICO_TABLEBOIT)
      init = ICO_TABLEPOISON;
    else
      init = ICO_TABLEBOIT;
    if (!g_typeedit)
      init = ICO_TABLEBOIT;
    DecorModif (cel, init); /* met une table avec bouteille */
    if (!g_typeedit)
      PlaySound (SOUND_SAUT2, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_LIVRE) /* livre ? */
  {
    DecorModif (cel, ICO_LIVRE); /* met un livre */
    if (!g_typeedit)
      PlaySound (SOUND_SAUT2, &cel);
    goto termine;
  }

  if (outil == ICO_OUTIL_CAISSE) /* caisse ? */
  {
    init = DecorGetCel (cel);
    switch (init)
    {
    case ICO_CAISSE:
      init = ICO_CAISSEV;
      break;
    case ICO_CAISSEV:
      init = ICO_CAISSEO;
      break;
    case ICO_CAISSEO:
      init = ICO_CAISSEG;
      break;
    case ICO_CAISSEG:
      init = ICO_CAISSE;
      break;
    default:
      init = lastcaisse;
    }
    DecorModif (cel, init); /* met une caisse */
    lastcaisse = init;
    goto termine;
  }

  if (outil == ICO_OUTIL_PORTE) /* porte ? */
  {
    init = DecorGetCel (cel);
    if (init < 0)
      goto termine;

    MurBuild (cel, 1); /* met une barrière (modifie murs alentours) */
    DecorModif (cel, init);
    con   = MurGetConnex (cel);
    first = ICO_PORTEF_EO;
    if (con & (1 << 0) && con & (1 << 2))
      first = ICO_PORTEF_NS;
    PutNewDecor (cel, ICO_PORTEF_EO, ICO_PORTEF_EO + 6, first, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_CLE) /* clé ? */
  {
    PutNewDecor (cel, ICO_CLE_A, ICO_CLE_C + 1, ICO_CLE_A, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_DETONATEUR) /* détonateur ? */
  {
    PutNewDecor (
      cel, ICO_DETONATEUR_A, ICO_DETONATEUR_C + 1, ICO_DETONATEUR_A, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_BOMBE) /* bombe ? */
  {
    PutNewDecor (cel, ICO_BOMBE_A, ICO_BOMBE_C + 1, ICO_BOMBE_A, 0, 0);
    goto termine;
  }

  if (outil == ICO_OUTIL_MAGIC) /* baguette magique ? */
  {
    DecorModif (cel, ICO_MAGIC); /* met un chapeau de magicien */
    if (!g_typeedit)
      PlaySound (SOUND_CLIC, &cel);
    goto termine;
  }

  return 1;

termine:
  DecorSuperCel (pos);
  cel = supercel;
  SuperCelClear (); /* éteint la super cellule */
  SuperCelFlush (); /* super cellule plus valable */
  supercel = cel;   /* ne redessine pas tout de suite ! */
  return 0;
}

/* ----------------- */
/* GetIconCaisseSSol */
/* ----------------- */

/*
    Dessine le sol sous une boule fixe en fonction du sol contenu
    dans le monde initial ou dans les cellules voisines du décor.
 */

short
GetIconCaisseSSol (Pt cel)
{
  short icon;

  icon = imonde[cel.y][cel.x];
  if (
    icon > ICO_SOLMAX && (icon < ICO_SENSUNI_S || icon > ICO_SENSUNI_O) &&
    (icon < ICO_PORTEO_EO || icon > ICO_PORTEO_EO + 5) &&
    (icon < ICO_ACCEL_S || icon > ICO_ACCEL_O) && icon != ICO_UNSEUL)
    icon = GetSol (cel, 0);

  return icon;
}

/* ========== */
/* DecorModif */
/* ========== */

/*
    Modifie une cellule dans l'image de fond pour le décor.
    La table[] donne les coordonnéés relatives des cellules ayant une
    intersection avec la cellule du décor modifiée.
 */

void
DecorModif (Pt cel, short newicon)
{
  short  icon;
  Pixmap pmnewdecor = {0};
  Pixmap pmmask     = {0};
  Pixmap pmissol;
  Pixmap pm;
  Pt     dst, zero = {0, 0}, dim = {LYICO, LXICO};
  Reg    rg;

  if (newicon == pmonde->tmonde[cel.y][cel.x])
    return;
  pmonde->tmonde[cel.y][cel.x] = newicon; /* modifie une cellule du monde */

  MoveModifCel (cel); /* indique cellule modifiée à move */

  /* Génère dans pmnewdecor l'image de la nouvelle partie du décor,
      en redessinant toutes les cellules placées derrière. */

  GetPixmap (&pmnewdecor, dim, 1, 1); /* noirci le pixmap du décor */

  int k = 0;
  for (int i = cel.y - 3; i <= MAXCELY; i++, k++)
  {
    for (int j = cel.x - 3; j <= MAXCELX; j++, k++)
    {
      Pt c = {i, j};

      if (c.x >= 0 && c.y >= 0)
      {
        if (c.x == MAXCELX)
          icon = ICO_BORDD; /* bord droite du plateau */
        if (c.y == MAXCELY)
          icon = ICO_BORDG; /* bord gauche du plateau */
        if (c.x < MAXCELX && c.y < MAXCELY)
          icon = pmonde->tmonde[c.y][c.x];

        dst.x = PLXICO * (j - cel.x) - PRXICO * (i - cel.y);
        dst.y = PRYICO * (i - cel.y) + PLYICO * (j - cel.x);

        if (
          icon == ICO_LUNETTES || icon == ICO_MAGIC || icon == ICO_AIMANT ||
          icon == ICO_LIVRE || icon == ICO_OBSTACLE + 8 || icon == ICO_GLISSE ||
          icon == ICO_CAISSE || icon == ICO_CAISSEV || icon == ICO_CAISSEO ||
          icon == ICO_CAISSEG || icon == ICO_TABLEVIDE ||
          icon == ICO_TABLEBOIT || icon == ICO_TABLEPOISON ||
          (icon >= ICO_MEUBLE && icon < ICO_MEUBLE + 16) ||
          (icon >= ICO_TANK_E && icon <= ICO_TANK_S) || icon == ICO_TANK_X ||
          icon == ICO_TANK_EO || icon == ICO_TANK_NS || icon == ICO_JOUEUR ||
          (icon >= ICO_DETONATEUR_A && icon <= ICO_BOMBE_EX))
        {
          GetIcon (&pmissol, GetIconCaisseSSol (c), 1);
          CopyPixel /* dessine le sol sous la boule */
            (&pmissol, zero, &pmnewdecor, dst, dim);
        }

        GetIcon (&pm, icon, 1);
        /* dessine la cellule */
        CopyPixel (&pm, zero, &pmnewdecor, dst, dim);
      }
    }
  }

  /* Copie la nouvelle partie du décor dans pmdecor, mais en la masquant
      au préalable par les objets pouvant être placés devant. */

  dst = CelToGra (cel);
  dst.x += PLXICO * ovisu.x;
  dst.y += PRYICO * ovisu.y;

  pmonde->tmonde[cel.y][cel.x] = ICO_SOL;
  pmonde->tmonde[cel.y][cel.x] = newicon;

  CopyPixel (&pmmask, zero, &pmnewdecor, zero, dim);
  CopyPixel /* dessine l'emplacement changé */
    (&pmnewdecor, zero, &pmdecor, dst, dim);

  rg.r.p1.x = dst.x;
  rg.r.p1.y = dst.y;
  rg.r.p2.x = dst.x + LXICO;
  rg.r.p2.y = dst.y + LYICO;
  IconDrawUpdate (rg); /* faudra redessiner cette partie */

  GivePixmap (&pmnewdecor);
  GivePixmap (&pmmask);

  DecorSuperCel (lastpmouse); /* remet la super cellule */
}

/* ============== */
/* DecorGetPixmap */
/* ============== */

/*
    Donne le pointeur au pixmap contenant le décor.
 */

Pixmap *
DecorGetPixmap (void)
{
  return &pmdecor;
}

/* =============== */
/* DecorGetOrigine */
/* =============== */

/*
    Donne l'origine de la cellule visible.
 */

Pt
DecorGetOrigine (void)
{
  return ovisu;
}

#define STEPDEL 3 /* délai d'un pas (unité 20ms) */
#define STEPX 24  /* pas horizontal (déplacement total = 176) */
#define STEPY 18  /* pas vertical   (déplacement total =  90) */

/* ---------- */
/* DecorMixPx */
/* ---------- */

/*
    Fabrique l'image de fond pour le décor en mixant l'ancien pixmap
    avec le nouveau lors d'un décalage horizontal de gauche à droite.
 */

void
DecorMixPx (Pixmap * ppmold, Pixmap * ppmnew, short total, short part)
{
  Pt p1, p2, dim;

  p1.y  = 0;
  p1.x  = part;
  p2.y  = POSYDRAW;
  p2.x  = POSXDRAW;
  dim.y = DIMYDRAW;
  dim.x = DIMXDRAW - part;
  CopyPixel (ppmold, p1, 0, p2, dim);

  p1.y  = 0;
  p1.x  = DIMXDRAW - total;
  p2.y  = POSYDRAW;
  p2.x  = POSXDRAW + DIMXDRAW - part;
  dim.y = DIMYDRAW;
  dim.x = part;
  CopyPixel (ppmnew, p1, 0, p2, dim);

  SDL_Delay (20);
}

/* ---------- */
/* DecorMixMx */
/* ---------- */

/*
    Fabrique l'image de fond pour le décor en mixant l'ancien pixmap
    avec le nouveau lors d'un décalage horizontal de droite à gauche.
 */

void
DecorMixMx (Pixmap * ppmold, Pixmap * ppmnew, short total, short part)
{
  Pt p, p2, zero = {0, 0}, dim;

  p.y   = POSYDRAW;
  p.x   = POSXDRAW + part;
  dim.y = DIMYDRAW;
  dim.x = DIMXDRAW - part;
  CopyPixel (ppmold, zero, 0, p, dim);

  p.y   = POSYDRAW;
  p.x   = POSXDRAW;
  p2.y  = 0;
  p2.x  = total - part;
  dim.y = DIMYDRAW;
  dim.x = part;
  CopyPixel (ppmnew, p2, 0, p, dim);

  SDL_Delay (20);
}

/* ---------- */
/* DecorMixPy */
/* ---------- */

/*
    Fabrique l'image de fond pour le décor en mixant l'ancien pixmap
    avec le nouveau lors d'un décalage vertical de haut en bas.
 */

void
DecorMixPy (Pixmap * ppmold, Pixmap * ppmnew, short total, short part)
{
  Pt p1, p2, dim;

  p1.x  = 0;
  p1.y  = part;
  p2.x  = POSXDRAW;
  p2.y  = POSYDRAW;
  dim.x = DIMXDRAW;
  dim.y = DIMYDRAW - part;
  CopyPixel (ppmold, p1, 0, p2, dim);

  p1.x  = 0;
  p1.y  = DIMYDRAW - total;
  p2.x  = POSXDRAW;
  p2.y  = POSYDRAW + DIMYDRAW - part;
  dim.x = DIMXDRAW;
  dim.y = part;
  CopyPixel (ppmnew, p1, 0, p2, dim);

  SDL_Delay (20);
}

/* ---------- */
/* DecorMixMy */
/* ---------- */

/*
    Fabrique l'image de fond pour le décor en mixant l'ancien pixmap
    avec le nouveau lors d'un décalage vertical de bas en haut.
 */

void
DecorMixMy (Pixmap * ppmold, Pixmap * ppmnew, short total, short part)
{
  Pt p1, p2, dim;

  p1.x  = 0;
  p1.y  = 0;
  p2.x  = POSXDRAW;
  p2.y  = POSYDRAW + part;
  dim.x = DIMXDRAW;
  dim.y = DIMYDRAW - part;
  CopyPixel (ppmold, p1, 0, p2, dim);

  p1.x  = 0;
  p1.y  = total - part;
  p2.x  = POSXDRAW;
  p2.y  = POSYDRAW;
  dim.x = DIMXDRAW;
  dim.y = part;
  CopyPixel (ppmnew, p1, 0, p2, dim);

  SDL_Delay (20);
}

/* =============== */
/* DecorSetOrigine */
/* =============== */

/*
    Donne l'origine de la cellule visible.
    quick = 0	->	décalage progressif
    quick = 1	->	décalage rapide
 */

void
DecorSetOrigine (Pt origine, short quick)
{
  Pt  p, oldpos, newpos, termpos, zero = {0, 0}, dim = {DIMYDRAW, DIMXDRAW};
  Reg rg;
  Pixmap * ppmicon;
  short    err;

  if (quick || lastovisu.x >= 10000 || g_updatescreen)
  {
    ovisu = origine;

    DecorMake (1);  /* adapte le décor */
    IconDrawAll (); /* redessine toute la fenêtre */
    return;
  }

  IconDrawOpen ();
  SuperCelClear ();  /* éteint la super cellule */
  SuperCelFlush ();  /* super cellule plus valable */
  MoveRedraw ();     /* redessine sans changement */
  IconDrawClose (1); /* enlève la super cellule de l'écran */

  oldpos.x = PLXICO * lastovisu.x;
  oldpos.y = PRYICO * lastovisu.y;
  newpos   = oldpos;

  ovisu = origine;
  DecorMake (0); /* pmdecor <-- adapte le décor */

  termpos.x = PLXICO * lastovisu.x;
  termpos.y = PRYICO * lastovisu.y;

  IconDrawOpen ();
  rg.r.p1.x = 0;
  rg.r.p1.y = 0;
  rg.r.p2.x = DIMXDRAW;
  rg.r.p2.y = DIMYDRAW;
  if (newpos.x < termpos.x)
    rg.r.p2.x = termpos.x - newpos.x;
  if (newpos.x > termpos.x)
    rg.r.p1.x = DIMXDRAW - (newpos.x - termpos.x);
  if (newpos.y < termpos.y)
    rg.r.p2.y = termpos.y - newpos.y;
  if (newpos.y > termpos.y)
    rg.r.p1.y = DIMYDRAW - (newpos.y - termpos.y);
  IconDrawUpdate (rg);
  MoveRedraw ();              /* redessine sans changement */
  IconDrawClose (0);          /* pmwork <-- icônes (nouveau) */
  ppmicon = IconGetPixmap (); /* ppmicon <-- pmwork */

  err = SavePixmap (&pmdecor); /* sauve le nouveau décor */
  if (err != 0)
    return;
  p.y = POSYDRAW;
  p.x = POSXDRAW;
  CopyPixel /* pmdecor <-- écran (actuel) */
    (0, p, &pmdecor, zero, dim);

  while (newpos.x < termpos.x)
  {
    newpos.x += STEPX;
    if (newpos.x > termpos.x)
      newpos.x = termpos.x;
    DecorMixMx (
      &pmdecor, ppmicon, termpos.x - oldpos.x,
      newpos.x - oldpos.x); /* adapte le décor "<" */
    Render ();
  }

  while (newpos.x > termpos.x)
  {
    newpos.x -= STEPX;
    if (newpos.x < termpos.x)
      newpos.x = termpos.x;
    DecorMixPx (
      &pmdecor, ppmicon, oldpos.x - termpos.x,
      oldpos.x - newpos.x); /* adapte le décor ">" */
    Render ();
  }

  while (newpos.y < termpos.y)
  {
    newpos.y += STEPY;
    if (newpos.y > termpos.y)
      newpos.y = termpos.y;
    DecorMixMy (
      &pmdecor, ppmicon, termpos.y - oldpos.y,
      newpos.y - oldpos.y); /* adapte le décor "^" */
    Render ();
  }

  while (newpos.y > termpos.y)
  {
    newpos.y -= STEPY;
    if (newpos.y < termpos.y)
      newpos.y = termpos.y;
    DecorMixPy (
      &pmdecor, ppmicon, oldpos.y - termpos.y,
      oldpos.y - newpos.y); /* adapte le décor "v" */
    Render ();
  }

  RestorePixmap (&pmdecor);   /* restitue le nouveau décor */
  DecorSuperCel (lastpmouse); /* remet la super cellule */
}

/* ---------- */
/* IfHideIcon */
/* ---------- */

/*
    Teste si une icône est complètement cachée, c'est-à-dire
    hors de la zone rectangulaire.
    Si oui (cachée), retourne TRUE.
    Si non (visible), retourne FALSE;
 */

static short
IfHideIcon (Pt pos, Rectangle zone)
{
  return (
    pos.x + LXICO < zone.p1.x || pos.x > zone.p2.x ||
    pos.y + LYICO < zone.p1.y || pos.y > zone.p2.y);
}

/* ------------- */
/* CopyIconDecor */
/* ------------- */

/*
    Copie une icône dans le pixmap du décor, avec clipping selon la zone.
 */

void
CopyIconDecor (Pixmap * ppmicon, Pt pos, Rectangle zone)
{
  Pt src, dst, dim;

  src.x = 0;
  src.y = 0;
  dst   = pos;
  dim.x = LXICO;
  dim.y = LYICO;

  if (dst.x < zone.p1.x) /* dépasse à gauche ? */
  {
    dim.x -= zone.p1.x - dst.x;
    if (dim.x <= 0)
      return;
    src.x += zone.p1.x - dst.x;
    dst.x = zone.p1.x;
  }
  if (dst.x + dim.x > zone.p2.x) /* dépasse à droite ? */
  {
    dim.x -= dst.x + dim.x - zone.p2.x;
    if (dim.x <= 0)
      return;
  }
  if (dst.y < zone.p1.y) /* dépasse en haut ? */
  {
    dim.y -= zone.p1.y - dst.y;
    if (dim.y <= 0)
      return;
    src.y += zone.p1.y - dst.y;
    dst.y = zone.p1.y;
  }
  if (dst.y + dim.y > zone.p2.y) /* dépasse en bas ? */
  {
    dim.y -= dst.y + dim.y - zone.p2.y;
    if (dim.y <= 0)
      return;
  }

  CopyPixel (ppmicon, src, &pmdecor, dst, dim);
}

/* ---------- */
/* DecorShift */
/* ---------- */

/*
    Fabrique l'image de fond pour le décor.
 */

void
DecorShift (Pt oldpos, Pt newpos, short bDraw)
{
  Pixmap    pmissol, pmichair;
  Rectangle zone;
  Pt        shift;
  Pt        pv, ph;
  Pt        cel;
  short     i, j, icon;

  /* Si c'est possible, décale une partie du contenu actuel de pmdecor
      pour n'avoir à redessiner plus que la partie effectivement
      changée, c'est-à-dire découverte. */

  zone.p1.x = 0;
  zone.p1.y = 0;
  zone.p2.x = DIMXDRAW;
  zone.p2.y = DIMYDRAW;

  if (oldpos.x < 10000)
  {
    shift.x = oldpos.x - newpos.x;
    shift.y = oldpos.y - newpos.y;
    ScrollPixel (&pmdecor, shift, COLORNOIR, &zone);
  }

  /* Met à jour le décor dans pmdecor correspondant à la zone découverte. */

  pv = newpos;
  for (i = 0; i <= MAXCELY; i++)
  {
    ph = pv;
    for (j = 0; j <= MAXCELX; j++)
    {
      if (j == MAXCELX && i == MAXCELY)
        goto term;
      if (j == MAXCELX)
        icon = ICO_BORDD; /* bord droite du plateau */
      if (i == MAXCELY)
        icon = ICO_BORDG; /* bord gauche du plateau */
      if (j < MAXCELX && i < MAXCELY)
        icon = pmonde->tmonde[i][j];

      if (!IfHideIcon (ph, zone))
      {
        GetIcon (&pmichair, icon, 1);

        if (
          icon == ICO_LUNETTES || icon == ICO_MAGIC || icon == ICO_AIMANT ||
          icon == ICO_LIVRE || icon == ICO_OBSTACLE + 8 || icon == ICO_GLISSE ||
          icon == ICO_CAISSE || icon == ICO_CAISSEV || icon == ICO_CAISSEO ||
          icon == ICO_CAISSEG || icon == ICO_TABLEVIDE ||
          icon == ICO_TABLEBOIT || icon == ICO_TABLEPOISON ||
          (icon >= ICO_MEUBLE && icon < ICO_MEUBLE + 16) ||
          (icon >= ICO_TANK_E && icon <= ICO_TANK_S) || icon == ICO_TANK_X ||
          icon == ICO_TANK_EO || icon == ICO_TANK_NS || icon == ICO_JOUEUR ||
          (icon >= ICO_DETONATEUR_A && icon <= ICO_BOMBE_EX))
        {
          cel.x = j;
          cel.y = i;
          GetIcon (&pmissol, GetIconCaisseSSol (cel), 1);
          CopyIconDecor (&pmissol, ph, zone); /* dessine le sol sous la boule */
        }

        CopyIconDecor (&pmichair, ph, zone); /* dessine la cellule */
      }
      ph.x += PLXICO;
      ph.y += PLYICO;
      if (ph.x > zone.p2.x)
        break;
    }
    pv.x -= PRXICO;
    pv.y += PRYICO;
    if (pv.y > zone.p2.y)
      break;
  }

term:
  if (bDraw)
  {
    Pt        src, dst, dim;
    Rectangle szone;

    dst.x = POSXDRAW;
    dst.y = POSYDRAW;
    dim.x = DIMXDRAW;
    dim.y = DIMYDRAW;
    ScrollPixelRect (0, dst, dim, shift, -1, &szone);

    src.x = zone.p1.x;
    src.y = zone.p1.y;
    dst.x = szone.p1.x;
    dst.y = szone.p1.y;
    dim.x = zone.p2.x - zone.p1.x;
    dim.y = zone.p2.y - zone.p1.y;
    CopyPixel (&pmdecor, src, 0, dst, dim);
  }
}

/* ========= */
/* DecorMake */
/* ========= */

/*
    Fabrique l'image de fond pour le décor.
 */

void
DecorMake (short bSuperCel)
{
  Pt oldpos, newpos;

  SuperCelClear (); /* éteint la super cellule */
  SuperCelFlush (); /* super cellule plus valable */

  if (lastovisu.x < 10000)
  {
    oldpos.x = PLXICO * lastovisu.x;
    oldpos.y = PRYICO * lastovisu.y;
  }
  else
    oldpos.x = 10000;

  newpos.x = PLXICO * ovisu.x;
  newpos.y = PRYICO * ovisu.y;

  DecorShift (oldpos, newpos, 0); /* fabrique le décor */
  lastovisu = ovisu;              /* mémorise l'origine actuelle */

  if (bSuperCel)
    DecorSuperCel (lastpmouse); /* remet la super cellule */
}

/* ============= */
/* DecorNewMonde */
/* ============= */

/*
    Initialise un nouveau monde.
    Retourne 0 si tout est en ordre.
 */

short
DecorNewMonde (Monde * pm)
{
  short x, y;

  pmonde = pm;

  for (y = 0; y < MAXCELY; y++)
    for (x = 0; x < MAXCELX; x++)
      imonde[y][x] = pmonde->tmonde[y][x];

  ovisu.x        = 4;
  ovisu.y        = -10;
  lastovisu.x    = 10000; /* le contenu de pmdecor est vide */
  lastpmouse.x   = -1;
  g_updatescreen = 1; /* il faut mettre l'écran à jour */

  SuperCelFlush ();
  MoveNewMonde (pmonde->freq); /* qq initialisations dans move */
  return 0;
}

int
LoadDecor ()
{
  Pt  p;
  int err;

  p.y = DIMYDRAW;
  p.x = DIMXDRAW;
  err = GetPixmap (&pmdecor, p, 1, 1);
  if (err)
    return err;

  p.y = LYICO;
  p.x = LXICO;
  err = GetPixmap (&pmsuper, p, -1, 1);
  if (err)
    return err;

  p.y = LYICO;
  p.x = LXICO;
  err = GetPixmap (&pmsback, p, -1, 1);

  return err;
}

void
UnloadDecor ()
{
  GivePixmap (&pmdecor);
  GivePixmap (&pmsuper);
  GivePixmap (&pmsback);
}

/* ========= */
/* DecorOpen */
/* ========= */

/*
    Ouverture des décors.
 */

short
DecorOpen (void)
{
  int err = LoadDecor ();

  if (err)
    return err;

  lastsensuni = 0;
  lastaccel   = 0;
  lastcaisse  = ICO_CAISSE;
  lasttank    = ICO_TANK_E;

  return 0;
}

/* ========== */
/* DecorClose */
/* ========== */

/*
    Fermeture des décors.
 */

void
DecorClose (void)
{
  UnloadDecor ();
}

/* ============= */
/* DecorPartieLg */
/* ============= */

/*
    Retourne la longueur nécessaire pour sauver les variables de la partie en
   cours.
 */

long
DecorPartieLg (void)
{
  return sizeof (short) * MAXCELY * MAXCELX + sizeof (Partie);
}

/* ================ */
/* DecorPartieWrite */
/* ================ */

/*
    Sauve les variables de la partie en cours.
 */

short
DecorPartieWrite (long pos, char file)
{
  short  err;
  Partie partie;

  err = FileWrite (&imonde, pos, sizeof (short) * MAXCELY * MAXCELX, file);
  if (err)
    return err;
  pos += sizeof (short) * MAXCELY * MAXCELX;

  partie.ovisu = ovisu;

  err = FileWrite (&partie, pos, sizeof (Partie), file);
  return err;
}

/* =============== */
/* DecorPartieRead */
/* =============== */

/*
    Lit les variables de la partie en cours.
 */

short
DecorPartieRead (long pos, char file)
{
  short  err;
  Partie partie;
  Pt     p;

  err = FileRead (&imonde, pos, sizeof (short) * MAXCELY * MAXCELX, file);
  if (err)
    return err;
  pos += sizeof (short) * MAXCELY * MAXCELX;

  err = FileRead (&partie, pos, sizeof (Partie), file);
  if (err)
    return err;

  ovisu = partie.ovisu;

  p.y = DIMYDRAW;
  p.x = DIMXDRAW;
  err = GetPixmap (&pmdecor, p, 1, 1);
  if (err)
    return err;

  lastovisu.x = 10000; /* le contenu de pmdecor est vide */
  SuperCelFlush ();    /* plus de super cellule valide */
  DecorMake (1);       /* refabrique le décor */
  IconDrawAll ();      /* redessine toute la fenêtre */
  return 0;
}

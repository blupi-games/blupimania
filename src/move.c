
/* ========= */
/* bm_move.c */
/* ========= */

#include <stdio.h>
#include <string.h>

#include "actions.h"
#include "blupimania.h"
#include "icon.h"

/* ------------------------- */
/* Descripteur d'un automate */
/* ------------------------- */

typedef struct {
  short   idata;    /* permet de régénérer le pointeur pdata (*) */
  short * pdata;    /* pointeur au data (fixe) */
  short   offset;   /* offset à partir de pdata */
  short   size;     /* taille du data (1..3) */
  short   loop;     /* nb de boucles à effectuer */
  short   index;    /* index */
  short   indexmax; /* index maximal */
} Auto;

/* (*)	Chaque fois que pdata est initialisé, idata l'est aussi, afin de
   permettre de régénérer pdata avec une procédure "Conv*ToTab*". Ceci est
   nécessaire lorsque la structure de l'automate est sauvée puis reprise sur
   disque !
 */

/* --------------------------- */
/* Descripteur d'un toto animé */
/* --------------------------- */

#define OFFZTOTO 6 /* décalage en Z pour toto */

typedef enum {
  STVIDE, /* pas de toto */
  STNORM  /* marche normalement */
} Status;

typedef struct {
  Status status;       /* état général */
  short  joueur;       /* 1 si c'est le joueur */
  short  force;        /* force (60..0) */
  short  vitesse;      /* vitesse (-2..+2) */
  short  vision;       /* vision (0..1) */
  short  mechant;      /* méchant (0..1) */
  short  tank;         /* tank (0..4) */
  short  magic;        /* passe-muraille (100..0) */
  short  invincible;   /* invincible (100..0) */
  short  cles;         /* clés trouvées */
  Pt     poscel;       /* position cellule dans le monde */
  Pt     poscela;      /* position cellule d'arrivée */
  Pt     poscelb;      /* position cellule d'arrivée lointaine (si saute) */
  Action action;       /* action en cours */
  Pt     lastpos;      /* dernière cellule atteinte */
  Pt     lastobjet;    /* coordonnées du dernier objet utilisé */
  short  cntnotuse;    /* durée pour ne pas utiliser */
  short  lastrot;      /* dernier sens de rotation utilisé */
  Pt     rstdecorcel;  /* cellule du décor à restaurer */
  short  rstdecoricon; /* icône du décor à restaurer */
  short  nextrepos;    /* prochain repos possible */
  short  sequence;     /* numéro de la séquence d'animation */
  short  energie;      /* énergie d'une boule qui roule */

  Auto  autoicon; /* automate pour les icônes */
  Auto  automove; /* automate pour les mouvements */
  short moverang; /* mouvement linéaire: rang */
  short movemax;  /* mouvement linéaire: maximum */
  Pt    movelg;   /* mouvement linéaire: longueur */

  short offz;   /* élévation verticale en z */
  Pt    posgra; /* position graphique de l'icône */
  short icon;   /* icône du moment */
} TotoMove;

/* ------------------------------------- */
/* Descripteur d'un objet du décor animé */
/* ------------------------------------- */

#define MAXOBJET 10

typedef struct {
  Status status;   /* état général */
  Pt     cel;      /* cellule de l'objet */
  Auto   autoicon; /* automate pour les icônes */
} ObjetMove;

/* -------------------------- */
/* Descripteur d'un ascenseur */
/* -------------------------- */

#define MAXDEPART 10

typedef struct {
  Pt    cel;   /* cellule où est l'ascenseur */
  short freq;  /* fréquence des départs */
  short count; /* décompteur */
} DepartMove;

/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static TotoMove   toto[MAXTOTO];     /* totos animés */
static TotoMove   back[MAXTOTO];     /* états précédents */
static DepartMove depart[MAXDEPART]; /* ascenseurs */
static ObjetMove  objet[MAXOBJET];   /* objets du décor animés */
static short      nbtoto;           /* nb de toto en mouvement simultanément */
static short      nbout;            /* nb de toto à sortir */
static short      nbin;             /* nb de toto à rentrer */
static short      perdu;            /* 1 -> c'est perdu */
static short      redraw;           /* 1 -> redessine tout */
static short      lastdetect;       /* dernier toto détecté pour les infos */
static short      lasttelecom;      /* dernière action de la télécommande */
static Pt         celcap1, celcap2; /* cellule de cap pour le toto du joueur */
static short      vitessepousse;    /* vitesse à laquelle pousser une caisse */
static Action     caisseodir;       /* boule qui roule: direction */
static Action     caisseoddir;      /* boule qui roule: dernière direction */
static Pt         caisseocel;       /* boule qui roule: départ */
static short      gendecor;         /* numéro de génération du décor */
static short genstarttank; /* numéro de génération dernier départ tank */

typedef struct {
  short  nbtoto;           /* nb de toto en mouvement simultanément */
  short  nbout;            /* nb de toto à sortir */
  short  nbin;             /* nb de toto à rentrer */
  short  lastdetect;       /* dernier toto détecté pour les infos */
  Pt     celcap1, celcap2; /* cellule de cap pour le toto du joueur */
  Action caisseodir;       /* boule qui roule: direction */
  Action caisseoddir;      /* boule qui roule: dernière direction */
  Pt     caisseocel;       /* boule qui roule: départ */
  short  reserve[10];      /* réserve */
} Partie;

void  NewAction (short i, Action action, short posz);
short ObjetPut (Pt cel, Objet obj);

/* -------- */
/* AutoInit */
/* -------- */

/*
    Initialise un automate.
 */

void
AutoInit (Auto * p, short idata, short * pdata, short size)
{
  p->idata  = idata;
  p->pdata  = pdata;
  p->offset = 0;
  p->size   = size;
  p->loop   = 0;
}

/* -------- */
/* AutoNext */
/* -------- */

/*
    Avance un automate.
    Retourne 1 (true) si c'est fini.
 */

short
AutoNext (Auto * p, short result[], const Pt * cel)
{
  short c;
  short i;

  if (p->loop == 0) /* opration suivante ? */
  {
    c = p->pdata[p->offset];

    if (c == OPSOUND)
    {
      p->offset++;
      PlaySound (p->pdata[p->offset++], cel);
      c = p->pdata[p->offset];
    }

    if (c == OPTERM)
    {
      return 1;
    }

    if (c == OPLIST)
    {
      p->offset++;
      p->loop     = 1;
      p->index    = 0;
      p->indexmax = p->pdata[p->offset++];
    }

    if (c == OPREPEAT)
    {
      p->offset++;
      p->loop     = p->pdata[p->offset++];
      p->index    = 0;
      p->indexmax = p->pdata[p->offset++];
    }
  }

  for (i = 0; i < p->size; i++)
  {
    result[i] = p->pdata[p->index * p->size + p->offset + i];
  }
  p->index++;
  if (p->index == p->indexmax)
  {
    p->index = 0;
    if (p->loop < 999)
      p->loop--;
    if (p->loop == 0)
    {
      p->offset += p->indexmax * p->size;
    }
  }

  return 0;
}

/* ============ */
/* MoveModifCel */
/* ============ */

/*
    Cette procédure est appelés chaque fois qu'une cellule du décor
    a été modifiée.
 */

void
MoveModifCel (Pt cel)
{
  short i;

  i = DecorGetCel (cel);
  if (
    i == ICO_TELE || i == ICO_TELE + 3 || i == ICO_TELE + 4 ||
    i == ICO_TABLEBOIT || i == ICO_TABLEVIDE)
    return;

  for (i = 0; i < MAXTOTO; i++)
  {
    if (toto[i].lastobjet.x == cel.x && toto[i].lastobjet.y == cel.y)
    {
      toto[i].lastobjet.x = -1;
      toto[i].lastobjet.y = -1;
    }
  }

  gendecor++;
}

/* ========== */
/* MoveGetCel */
/* ========== */

/*
    Retourne 1 si le toto est sur la cellule de départ.
    Retourne 2 si le toto est sur la cellule d'arrivée.
    Retourne 0 s'il n'y a rien (pas de toto ici).
    Retourne -1 si les coordonnées sont hors du monde !
 */

short
MoveGetCel (Pt cel)
{
  short i;

  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    return -1; /* sort du monde */

  for (i = 0; i < MAXTOTO; i++)
  {
    if (toto[i].status == STVIDE)
      continue;
    if (toto[i].poscel.x == cel.x && toto[i].poscel.y == cel.y)
      return 1;
    if (toto[i].poscela.x == cel.x && toto[i].poscela.y == cel.y)
      return 2;
    if (toto[i].poscelb.x == cel.x && toto[i].poscelb.y == cel.y)
      return 2;
  }

  return 0; /* pas de toto ici */
}

/* ----------- */
/* GetObstacle */
/* ----------- */

/*
    Détermine l'obstacle éventuel placé sur une cellule.
    Il peut s'agit d'un objet du décor ou d'un autre toto !
    Retourne 0 s'il n'y a pas d'obstacle.
        toto = 1	->	tient compte des toto
        toto = 0	->	ne tient compte que du décor
 */

short
GetObstacle (Pt cel, short toto)
{
  short i;

  i = DecorGetCel (cel);

  if (i >= ICO_PORTEO_EO && i < ICO_PORTEO_EO + 6)
    goto move;

  if (i == ICO_UNSEUL)
    goto move;

  if (
    i == -1 ||             /* sort du monde */
    i >= ICO_BLOQUE ||     /* obstacle du décor */
    i == ICO_ARRIVEE ||    /* arrivée */
    i == ICO_DEPART ||     /* départ */
    i == ICO_TROU ||       /* trou */
    i == ICO_TROUBOUCHE || /* trou bouché */
    i == ICO_GLISSE)       /* peau de banane */
    return i;

move:
  if (toto == 0)
    return 0;
  return MoveGetCel (cel); /* tient compte des toto */
}

/* -------------- */
/* GetOrientation */
/* -------------- */

/*
    Donne l'orientation correspondant à une action.
    Retourne une valeur ACMARCHE?? selon le sens.
 */

Action
GetOrientation (Action action)
{
  switch (action)
  {
  case AC_STOP_E:
  case AC_MARCHE_E:
  case AC_RECULE_E:
  case AC_SAUTE1_E:
  case AC_SAUTE2_E:
  case AC_SAUTEDET_E:
  case AC_TOURNE_NE:
  case AC_TOURNE_SE:
  case AC_TOMBE_E:
  case AC_TOMBE_TANK_E:
  case AC_TOMBE_TANKB_E:
  case AC_BOIT_E:
  case AC_BOITX_E:
  case AC_TOURTE_E:
  case AC_REPOS_E:
  case AC_DORT_E:
  case AC_REFLEXION_E:
  case AC_HAUSSE_E:
  case AC_YOYO_E:
  case AC_YOUPIE_E:
  case AC_NON_E:
  case AC_VISION_E:
  case AC_POUSSE_E:
  case AC_NPOUSSE_E:
  case AC_CAISSE_E:
  case AC_CAISSEV_E:
  case AC_CAISSEO_E:
  case AC_CAISSEOD_E:
  case AC_CAISSEG_E:
  case AC_DEPART_E:
  case AC_MAGIC_E:
  case AC_MECHANT_E:
  case AC_GLISSE_E:
  case AC_LIVRE_E:
  case AC_MUSIQUE_E:
  case AC_CLE_E:
  case AC_PORTE_E:
  case AC_EXPLOSE_E:
  case AC_START_E:
    return AC_MARCHE_E;

  case AC_STOP_O:
  case AC_MARCHE_O:
  case AC_RECULE_O:
  case AC_SAUTE1_O:
  case AC_SAUTE2_O:
  case AC_SAUTEDET_O:
  case AC_TOURNE_SO:
  case AC_TOURNE_NO:
  case AC_TOMBE_O:
  case AC_TOMBE_TANK_O:
  case AC_TOMBE_TANKB_O:
  case AC_BOIT_O:
  case AC_BOITX_O:
  case AC_TOURTE_O:
  case AC_REPOS_O:
  case AC_DORT_O:
  case AC_REFLEXION_O:
  case AC_HAUSSE_O:
  case AC_YOYO_O:
  case AC_YOUPIE_O:
  case AC_NON_O:
  case AC_VISION_O:
  case AC_POUSSE_O:
  case AC_NPOUSSE_O:
  case AC_CAISSE_O:
  case AC_CAISSEV_O:
  case AC_CAISSEO_O:
  case AC_CAISSEOD_O:
  case AC_CAISSEG_O:
  case AC_PIANO_O:
  case AC_MAGIC_O:
  case AC_MECHANT_O:
  case AC_GLISSE_O:
  case AC_LIVRE_O:
  case AC_MUSIQUE_O:
  case AC_CLE_O:
  case AC_PORTE_O:
  case AC_BAISSE_O:
  case AC_ELECTRO_O:
  case AC_EXPLOSE_O:
  case AC_START_O:
    return AC_MARCHE_O;

  case AC_STOP_S:
  case AC_MARCHE_S:
  case AC_RECULE_S:
  case AC_SAUTE1_S:
  case AC_SAUTE2_S:
  case AC_SAUTEDET_S:
  case AC_TOURNE_ES:
  case AC_TOURNE_OS:
  case AC_TOMBE_S:
  case AC_TOMBE_TANK_S:
  case AC_TOMBE_TANKB_S:
  case AC_BOIT_S:
  case AC_BOITX_S:
  case AC_TOURTE_S:
  case AC_REPOS_S:
  case AC_DORT_S:
  case AC_REFLEXION_S:
  case AC_HAUSSE_S:
  case AC_YOYO_S:
  case AC_YOUPIE_S:
  case AC_NON_S:
  case AC_VISION_S:
  case AC_POUSSE_S:
  case AC_NPOUSSE_S:
  case AC_CAISSE_S:
  case AC_CAISSEV_S:
  case AC_CAISSEO_S:
  case AC_CAISSEOD_S:
  case AC_CAISSEG_S:
  case AC_MAGIC_S:
  case AC_MECHANT_S:
  case AC_GLISSE_S:
  case AC_LIVRE_S:
  case AC_MUSIQUE_S:
  case AC_CLE_S:
  case AC_PORTE_S:
  case AC_EXPLOSE_S:
  case AC_START_S:
    return AC_MARCHE_S;

  case AC_STOP_N:
  case AC_MARCHE_N:
  case AC_RECULE_N:
  case AC_SAUTE1_N:
  case AC_SAUTE2_N:
  case AC_SAUTEDET_N:
  case AC_TOURNE_ON:
  case AC_TOURNE_EN:
  case AC_TOMBE_N:
  case AC_TOMBE_TANK_N:
  case AC_TOMBE_TANKB_N:
  case AC_BOIT_N:
  case AC_BOITX_N:
  case AC_TOURTE_N:
  case AC_REPOS_N:
  case AC_DORT_N:
  case AC_REFLEXION_N:
  case AC_HAUSSE_N:
  case AC_YOYO_N:
  case AC_YOUPIE_N:
  case AC_NON_N:
  case AC_VISION_N:
  case AC_POUSSE_N:
  case AC_NPOUSSE_N:
  case AC_CAISSE_N:
  case AC_CAISSEV_N:
  case AC_CAISSEO_N:
  case AC_CAISSEOD_N:
  case AC_CAISSEG_N:
  case AC_MAGIC_N:
  case AC_MECHANT_N:
  case AC_GLISSE_N:
  case AC_LIVRE_N:
  case AC_MUSIQUE_N:
  case AC_CLE_N:
  case AC_PORTE_N:
  case AC_TELE_N:
  case AC_EXPLOSE_N:
  case AC_START_N:
    return AC_MARCHE_N;

  default:
    break;
  }
  return -1;
}

/* ----------- */
/* GetRotation */
/* ----------- */

/*
    Donne le sens de la rotation correspondant à une action.
    Retourne +1 si c'est dans le sens des aiguilles d'une montre.
    Retourne -1 si c'est dans le sens contraire.
    Retourne 0 si c'est une action qui ne tourne pas !
 */

short
GetRotation (Action action)
{
  switch (action)
  {
  case AC_TOURNE_NE:
  case AC_TOURNE_ES:
  case AC_TOURNE_SO:
  case AC_TOURNE_ON:
    return +1;
  case AC_TOURNE_NO:
  case AC_TOURNE_OS:
  case AC_TOURNE_SE:
  case AC_TOURNE_EN:
    return -1;
  default:
    return 0;
  }
  return 0;
}

/* -------- */
/* MoveInit */
/* -------- */

/*
    Initialise un toto animé sur une cellule, prêt à partir avec une action.
    Retourne le rang du toto initialisé (-1 si erreur).
 */

short
MoveInit (Pt poscel, short action, short posz)
{
  short i;

  for (i = 0; 1; i++)
  {
    if (i == MAXTOTO)
      return -1;
    if (toto[i].status == STVIDE)
      break;
  }

  nbtoto++; /* un objet animé de plus */

  toto[i].poscel       = poscel;
  toto[i].poscela.x    = -1;
  toto[i].poscela.y    = -1;
  toto[i].poscelb.x    = -1;
  toto[i].poscelb.y    = -1;
  toto[i].icon         = 0;
  toto[i].action       = -1;
  toto[i].lastpos.x    = -1;
  toto[i].lastpos.y    = -1;
  toto[i].lastobjet.x  = -1;
  toto[i].lastobjet.y  = -1;
  toto[i].joueur       = 0;
  toto[i].force        = 15;
  toto[i].vitesse      = 0;
  toto[i].vision       = 0;
  toto[i].mechant      = 0;
  toto[i].tank         = 0;
  toto[i].magic        = 0;
  toto[i].invincible   = 0;
  toto[i].cles         = 0;
  toto[i].rstdecoricon = 0;
  toto[i].nextrepos    = 0;
  toto[i].sequence     = 0;
  toto[i].energie      = 0;

  NewAction (i, action, posz); /* démarre une action */

  return i;
}

/* ---------- */
/* DepartNext */
/* ---------- */

/*
    Fait éventuellement partir les ascenseurs.
 */

void
DepartNext (void)
{
  short d;
  Pt    cel;

  if (nbout == 0 || nbtoto > MAXTOTO - 2)
    return;

  for (d = 0; d < MAXDEPART; d++)
  {
    if (depart[d].cel.x < 0)
      continue; /* continue si ascenseur inutilisé */

    cel = depart[d].cel;
    cel.x++;
    if (GetObstacle (cel, 1) != 0)
      continue; /* continue si cellule occupée */

    depart[d].count--;
    if (depart[d].count == 0)
    {
      depart[d].count = depart[d].freq;
      ObjetPut (depart[d].cel, OB_DEPART); /* ouverture-fermeture */
      MoveInit (
        depart[d].cel, AC_BALLON_E, 150 - LYICO); /* un ballon s'envole .. */
      MoveInit (depart[d].cel, AC_DEPART_E, 150); /* .. avec toto accroché */
      nbout--;
      if (nbout == 0 || nbtoto > MAXTOTO - 2)
        return;
    }
  }
}

/* ------------ */
/* ObjetNextOne */
/* ------------ */

/*
    Bouge si nécessaire un objet animé du décor.
 */

void
ObjetNextOne (short i)
{
  short result[1];
  short icon;

  if (objet[i].status == STVIDE)
    return;

  if (AutoNext (&objet[i].autoicon, result, &objet[i].cel))
  {
    objet[i].status = STVIDE;
  }
  else
  {
    icon = result[0];
    if (icon == -1)
    {
      icon = DecorGetInitCel (objet[i].cel);
    }
    if (icon != 0)
    {
      DecorModif (objet[i].cel, icon); /* modifie le décor */
    }
  }
}

/* --------- */
/* ObjetNext */
/* --------- */

/*
    Bouge si nécessaire les objets animés du décor.
 */

void
ObjetNext (void)
{
  short i;

  for (i = 0; i < MAXOBJET; i++)
  {
    ObjetNextOne (i);
  }
}

/* -------- */
/* ObjetPut */
/* -------- */

/*
    Met un objet animé dans la liste.
    Retourne 0 (false) si la liste est pleine.
 */

short
ObjetPut (Pt cel, Objet obj)
{
  short i;

  for (i = 0; i < MAXOBJET; i++)
  {
    if (objet[i].status == STVIDE)
    {
      objet[i].status = STNORM;
      objet[i].cel    = cel;
      AutoInit (&objet[i].autoicon, obj, ConvObjetToTabIcon (obj), 1);

      if (obj >= OB_SENSUNIO && obj <= OB_SENSUNIS)
      {
        ObjetNextOne (i); /* modifie le décor immédiatement */
      }

      return 1;
    }
  }
  return 0;
}

/* ---------- */
/* TableMovie */
/* ---------- */

/*
    Génère un mouvement quelconque selon une table.
 */

void
TableMovie (short i, Action action, short * ptable)
{
  AutoInit (&toto[i].automove, action, ptable, 3);
  toto[i].movemax = 0; /* indique mouvement selon une table */
}

/* --------- */
/* InitMovie */
/* --------- */

/*
    Initialise un mouvement linéaire régulier, dans le plan (z=0).
        nbmove	->	nb de pas à effectuer
        lg		->	déplacement à effectuer (+/- dx;dy)
 */

void
InitMovie (short i, short nbmove, Pt lg)
{
  if (nbmove < 1)
    nbmove = 1; /* vitesse maximale */
  if (nbmove > 100)
    nbmove = 100; /* vitesse minimale */

  toto[i].moverang = 0;
  toto[i].movemax  = nbmove;
  toto[i].movelg   = lg;
}

/* --------- */
/* CalcMovie */
/* --------- */

/*
    Calcule un mouvement linéaire régulier, dans le plan (z=0).
    Retourne 1 (true) si le mouvement est terminé.
 */

short
CalcMovie (short i, short result[])
{
  Pt last, new;

  if (toto[i].movemax == 0) /* mouvement selon table ? */
  {
    return AutoNext (
      &toto[i].automove, result,
      &toto[i].poscel); /* oui -> mouvement selon automate */
  }

  if (toto[i].moverang >= toto[i].movemax)
    return 1;

  if (toto[i].moverang == 0)
  {
    last.x = 0;
    last.y = 0;
  }
  else
  {
    last.x =
      ((toto[i].movelg.x * (toto[i].moverang - 1)) + toto[i].movemax / 2) /
      toto[i].movemax;
    last.y =
      ((toto[i].movelg.y * (toto[i].moverang - 1)) + toto[i].movemax / 2) /
      toto[i].movemax;
  }

  new.x = ((toto[i].movelg.x * toto[i].moverang) + toto[i].movemax / 2) /
          toto[i].movemax;
  new.y = ((toto[i].movelg.y * toto[i].moverang) + toto[i].movemax / 2) /
          toto[i].movemax;

  toto[i].moverang++;

  result[0] = new.x - last.x; /* déplacement selon x */
  result[1] = new.y - last.y; /* déplacement selon y */
  result[2] = 0;              /* déplacement selon z */

  return 0;
}

/* ------------- */
/* GetTypeMarche */
/* ------------- */

/*
    Donne le type de marche à utiliser pour un toto donné.
        0 -> marche rapide en 4 mouvements (1,2,1,3)
        1 -> marche plus jolie
 */

short
GetTypeMarche (short i)
{
  if (
    toto[i].force >= 5 && toto[i].vision == 1 && toto[i].mechant == 0 &&
    toto[i].tank == 0)
    return 1;

  return 0;
}

/* --------- */
/* NewAction */
/* --------- */

/*
    Démarre une nouvelle action pour un toto.
 */

void
NewAction (short i, Action action, short posz)
{
  short   nbmove;
  short * ptable;
  Pt      p;
  Action  lastaction;

  lastaction = toto[i].action;

  if (
    action >= AC_MARCHE_E && /* toto doit marcher ? */
    action <= AC_MARCHE_S &&
    ((toto[i].action >= AC_STOP_E && /* toto est stoppé ? */
      toto[i].action <= AC_STOP_S) ||
     (toto[i].action >= AC_VISION_E && /* toto a reçu des lunettes ? */
      toto[i].action <= AC_VISION_S) ||
     (toto[i].action >= AC_REPOS_E && /* toto s'est reposé ? */
      toto[i].action <= AC_REPOS_S) ||
     (toto[i].action >= AC_REFLEXION_E && /* toto s'est gratté la tête ? */
      toto[i].action <= AC_REFLEXION_S) ||
     (toto[i].action >= AC_YOYO_E && /* toto a joué au yoyo ? */
      toto[i].action <= AC_YOYO_S) ||
     (toto[i].action >= AC_HAUSSE_E && /* toto a haussé les épaules ? */
      toto[i].action <= AC_HAUSSE_S)) &&
    toto[i].force >= 5 && /* toto a de la force ? */
    toto[i].tank == 0)
  {
    action +=
      AC_START_E -
      AC_MARCHE_E; /* oui -> effectue l'action "start" avant de marcher */
  }

  toto[i].lastrot = GetRotation (action);

  toto[i].status = STNORM;
  toto[i].action = action;

  p = CelToGra (toto[i].poscel);
  p.y -= OFFZTOTO;
  toto[i].posgra = p;
  toto[i].offz   = posz;

  if (
    toto[i].poscela.x != toto[i].poscel.x ||
    toto[i].poscela.y != toto[i].poscel.y ||
    toto[i].poscelb.x != toto[i].poscel.x ||
    toto[i].poscelb.y != toto[i].poscel.y)
  {
    gendecor++;
  }

  toto[i].poscela = toto[i].poscel;
  toto[i].poscelb = toto[i].poscel;

  if (toto[i].vision == 0 || toto[i].force < 5)
    nbmove = 14;
  else
    nbmove = 8;
  if (toto[i].mechant == 1)
    nbmove = 6;

  if (toto[i].joueur)
    nbmove -= 2;

  if (toto[i].tank == 1)
    nbmove = 12;
  if (toto[i].tank == 2)
    nbmove = 5;
  if (toto[i].tank == 3)
    nbmove = 15;

  if (toto[i].vitesse > 0)
    nbmove /= 3;
  if (toto[i].vitesse < 0)
    nbmove *= 3;

  if (
    action < AC_MARCHE_E || action > AC_MARCHE_S || action != lastaction ||
    nbmove != toto[i].movemax)
  {
    ptable = ConvActionToTabIcon (action, GetTypeMarche (i));
    AutoInit (&toto[i].autoicon, action, ptable, 1); /* init les icônes */
  }

  ptable = ConvActionToTabMove (action);
  if (ptable != NULL)
  {
    TableMovie (i, action, ptable); /* init le mouvement selon table */
  }

  switch (action)
  {
  case AC_STOP_E:
  case AC_STOP_O:
  case AC_STOP_S:
  case AC_STOP_N:
    InitMovie (i, 1, (p.y = 0, p.x = 0, p));
    break;

  case AC_START_E:
    InitMovie (i, 2, (p.y = 0, p.x = 0, p));
    toto[i].poscelb.x++; /* comme si cellule déjà occupée ! */
    break;
  case AC_START_O:
    InitMovie (i, 2, (p.y = 0, p.x = 0, p));
    toto[i].poscelb.x--; /* comme si cellule déjà occupée ! */
    break;
  case AC_START_S:
    InitMovie (i, 2, (p.y = 0, p.x = 0, p));
    toto[i].poscelb.y++; /* comme si cellule déjà occupée ! */
    break;
  case AC_START_N:
    InitMovie (i, 2, (p.y = 0, p.x = 0, p));
    toto[i].poscelb.y--; /* comme si cellule déjà occupée ! */
    break;

  case AC_MARCHE_E:
  case AC_RECULE_O:
    InitMovie (i, nbmove, (p.y = PLYICO, p.x = PLXICO, p));
    toto[i].poscela.x++;
    break;
  case AC_MARCHE_O:
  case AC_RECULE_E:
    InitMovie (i, nbmove, (p.y = -PLYICO, p.x = -PLXICO, p));
    toto[i].poscela.x--;
    break;
  case AC_MARCHE_S:
  case AC_RECULE_N:
    InitMovie (i, nbmove, (p.y = PRYICO, p.x = -PRXICO, p));
    toto[i].poscela.y++;
    break;
  case AC_MARCHE_N:
  case AC_RECULE_S:
    InitMovie (i, nbmove, (p.y = -PRYICO, p.x = PRXICO, p));
    toto[i].poscela.y--;
    break;

  case AC_SAUTE1_E:
    toto[i].poscela.x++;
    toto[i].poscelb.x += 2;
    break;
  case AC_SAUTE1_O:
    toto[i].poscela.x--;
    toto[i].poscelb.x -= 2;
    break;
  case AC_SAUTE1_S:
    toto[i].poscela.y++;
    toto[i].poscelb.y += 2;
    break;
  case AC_SAUTE1_N:
    toto[i].poscela.y--;
    toto[i].poscelb.y -= 2;
    break;

  case AC_PIANO_O:
    InitMovie (i, 30, (p.y = 0, p.x = 0, p));
    break;

  case AC_REPOS_E:
  case AC_REPOS_O:
  case AC_REPOS_S:
  case AC_REPOS_N:
    InitMovie (i, 30, (p.y = 0, p.x = 0, p));
    break;

  case AC_DORT_E:
  case AC_DORT_O:
  case AC_DORT_S:
  case AC_DORT_N:
    InitMovie (i, 50, (p.y = 0, p.x = 0, p));
    break;

  case AC_REFLEXION_E:
  case AC_REFLEXION_O:
  case AC_REFLEXION_S:
  case AC_REFLEXION_N:
    InitMovie (i, 4, (p.y = 0, p.x = 0, p));
    break;

  case AC_HAUSSE_E:
  case AC_HAUSSE_O:
  case AC_HAUSSE_S:
  case AC_HAUSSE_N:
    InitMovie (i, 2, (p.y = 0, p.x = 0, p));
    break;

  case AC_YOYO_E:
  case AC_YOYO_O:
  case AC_YOYO_S:
  case AC_YOYO_N:
    InitMovie (i, 60, (p.y = 0, p.x = 0, p));
    break;

  case AC_NON_E:
  case AC_NON_O:
  case AC_NON_S:
  case AC_NON_N:
    InitMovie (i, 8, (p.y = 0, p.x = 0, p));
    break;

  case AC_POUSSE_E:
    InitMovie (i, vitessepousse, (p.y = PLYICO, p.x = PLXICO, p));
    toto[i].poscela.x++;
    break;
  case AC_POUSSE_O:
    InitMovie (i, vitessepousse, (p.y = -PLYICO, p.x = -PLXICO, p));
    toto[i].poscela.x--;
    break;
  case AC_POUSSE_S:
    InitMovie (i, vitessepousse, (p.y = PRYICO, p.x = -PRXICO, p));
    toto[i].poscela.y++;
    break;
  case AC_POUSSE_N:
    InitMovie (i, vitessepousse, (p.y = -PRYICO, p.x = PRXICO, p));
    toto[i].poscela.y--;
    break;

  case AC_NPOUSSE_E:
  case AC_NPOUSSE_O:
  case AC_NPOUSSE_S:
  case AC_NPOUSSE_N:
    InitMovie (i, 8, (p.y = 0, p.x = 0, p));
    break;

  case AC_CAISSE_E:
  case AC_CAISSEV_E:
  case AC_CAISSEO_E:
  case AC_CAISSEG_E:
    InitMovie (i, vitessepousse, (p.y = PLYICO, p.x = PLXICO, p));
    toto[i].poscela.x++;
    break;
  case AC_CAISSE_O:
  case AC_CAISSEV_O:
  case AC_CAISSEO_O:
  case AC_CAISSEG_O:
    InitMovie (i, vitessepousse, (p.y = -PLYICO, p.x = -PLXICO, p));
    toto[i].poscela.x--;
    break;
  case AC_CAISSE_S:
  case AC_CAISSEV_S:
  case AC_CAISSEO_S:
  case AC_CAISSEG_S:
    InitMovie (i, vitessepousse, (p.y = PRYICO, p.x = -PRXICO, p));
    toto[i].poscela.y++;
    break;
  case AC_CAISSE_N:
  case AC_CAISSEV_N:
  case AC_CAISSEO_N:
  case AC_CAISSEG_N:
    InitMovie (i, vitessepousse, (p.y = -PRYICO, p.x = PRXICO, p));
    toto[i].poscela.y--;
    break;

  case AC_ELECTRO_O:
    InitMovie (i, 16, (p.y = 0, p.x = 0, p));
    break;

  case AC_TELE_N:
    InitMovie (i, 20, (p.y = 0, p.x = 0, p));
    break;

  case AC_TOURTE_E:
  case AC_TOURTE_O:
  case AC_TOURTE_S:
  case AC_TOURTE_N:
    InitMovie (i, 35, (p.y = 0, p.x = 0, p));
    break;

  case AC_SAUTE2_E:
  case AC_TOMBE_E:
  case AC_TOMBE_TANK_E:
  case AC_TOMBE_TANKB_E:
  case AC_CAISSEOD_E:
  case AC_BALLON_E:
  case AC_DEPART_E:
  case AC_ARRIVEE_E:
  case AC_GLISSE_E:
    toto[i].poscela.x++;
    break;
  case AC_SAUTE2_O:
  case AC_TOMBE_O:
  case AC_TOMBE_TANK_O:
  case AC_TOMBE_TANKB_O:
  case AC_CAISSEOD_O:
  case AC_ARRIVEE_O:
  case AC_GLISSE_O:
    toto[i].poscela.x--;
    break;
  case AC_SAUTE2_S:
  case AC_TOMBE_S:
  case AC_TOMBE_TANK_S:
  case AC_TOMBE_TANKB_S:
  case AC_CAISSEOD_S:
  case AC_ARRIVEE_S:
  case AC_GLISSE_S:
    toto[i].poscela.y++;
    break;
  case AC_SAUTE2_N:
  case AC_TOMBE_N:
  case AC_TOMBE_TANK_N:
  case AC_TOMBE_TANKB_N:
  case AC_CAISSEOD_N:
  case AC_ARRIVEE_N:
  case AC_GLISSE_N:
    toto[i].poscela.y--;
    break;

  case AC_BAISSE_O:
    InitMovie (i, 40, (p.y = 0, p.x = 0, p));
    break;

  case AC_TANK:
    InitMovie (i, 80, (p.y = 0, p.x = 0, p));
    break;

  default:
    break;
  }

  if (
    toto[i].poscela.x != toto[i].poscel.x ||
    toto[i].poscela.y != toto[i].poscel.y ||
    toto[i].poscelb.x != toto[i].poscel.x ||
    toto[i].poscelb.y != toto[i].poscel.y)
  {
    gendecor++;
  }
}

/* ------ */
/* GetMur */
/* ------ */

/*
    Retourne l'obstacle présent dans une cellule, comme GetObstacle.
    Retourne 0 (rien) si l'obstacle n'est pas un mur (sens-unique).
 */

short
GetMur (Pt cel, short toto)
{
  short icon;

  icon = GetObstacle (cel, toto);

  if (icon >= ICO_SENSUNI_S && icon <= ICO_SENSUNI_O)
    return 0;
  if (icon >= ICO_ACCEL_S && icon <= ICO_ACCEL_O)
    return 0;
  return icon;
}

/* ------------ */
/* TourneAction */
/* ------------ */

/*
    Tourne toto lorsqu'il bute sur un obstacle quelconque
    (mur du décor, autre toto, etc.).
    Essaye de le tourner dans le même sens que le précédent utilisé !
 */

void
TourneAction (short i)
{
  Action nextaction = toto[i].action;
  Action orientation;
  short  n = 0;
  Pt     left, right;
  short  leftob, rightob;

  orientation = GetOrientation (toto[i].action);

  if (toto[i].vision == 0) /* est-ce que toto est aveugle ? */
  {
    do
    {
      switch (orientation)
      {
      case AC_MARCHE_E:
        nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_ES : AC_TOURNE_EN;
        break;
      case AC_MARCHE_O:
        nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_OS : AC_TOURNE_ON;
        break;
      case AC_MARCHE_S:
        nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_SO : AC_TOURNE_SE;
        break;
      case AC_MARCHE_N:
        nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_NO : AC_TOURNE_NE;
        break;
      default:
        break;
      }
      n++;
      if (toto[i].lastrot == 0)
        break;
      if (toto[i].lastrot == GetRotation (nextaction))
        break;
      if (n > 10)
        break;
    } while (1);
  }
  else /* si toto n'est pas aveugle */
  {
    left  = toto[i].poscel;
    right = toto[i].poscel;

    switch (orientation)
    {
    case AC_MARCHE_E:
      left.y--;
      leftob = GetMur (left, 1);
      right.y++;
      rightob    = GetMur (right, 1);
      nextaction = -1;
      if (
        (left.x != toto[i].lastpos.x || left.y != toto[i].lastpos.y) &&
        leftob == 0)
        nextaction = AC_TOURNE_EN;
      if (
        (right.x != toto[i].lastpos.x || right.y != toto[i].lastpos.y) &&
        rightob == 0)
        nextaction = AC_TOURNE_ES;
      if (nextaction != -1)
        break;

      if (leftob == 0)
        nextaction = AC_TOURNE_EN;
      if (rightob == 0)
        nextaction = AC_TOURNE_ES;
      if (nextaction != -1)
        break;

      if (toto[i].lastrot == +1)
        nextaction = AC_TOURNE_ES;
      if (toto[i].lastrot == -1)
        nextaction = AC_TOURNE_EN;
      if (nextaction != -1)
        break;

      nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_ES : AC_TOURNE_EN;
      break;

    case AC_MARCHE_O:
      left.y++;
      leftob = GetMur (left, 1);
      right.y--;
      rightob    = GetMur (right, 1);
      nextaction = -1;
      if (
        (left.x != toto[i].lastpos.x || left.y != toto[i].lastpos.y) &&
        leftob == 0)
        nextaction = AC_TOURNE_OS;
      if (
        (right.x != toto[i].lastpos.x || right.y != toto[i].lastpos.y) &&
        rightob == 0)
        nextaction = AC_TOURNE_ON;
      if (nextaction != -1)
        break;

      if (leftob == 0)
        nextaction = AC_TOURNE_OS;
      if (rightob == 0)
        nextaction = AC_TOURNE_ON;
      if (nextaction != -1)
        break;

      if (toto[i].lastrot == +1)
        nextaction = AC_TOURNE_ON;
      if (toto[i].lastrot == -1)
        nextaction = AC_TOURNE_OS;
      if (nextaction != -1)
        break;

      nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_OS : AC_TOURNE_ON;
      break;

    case AC_MARCHE_S:
      left.x++;
      leftob = GetMur (left, 1);
      right.x--;
      rightob    = GetMur (right, 1);
      nextaction = -1;
      if (
        (left.x != toto[i].lastpos.x || left.y != toto[i].lastpos.y) &&
        leftob == 0)
        nextaction = AC_TOURNE_SE;
      if (
        (right.x != toto[i].lastpos.x || right.y != toto[i].lastpos.y) &&
        rightob == 0)
        nextaction = AC_TOURNE_SO;
      if (nextaction != -1)
        break;

      if (leftob == 0)
        nextaction = AC_TOURNE_SE;
      if (rightob == 0)
        nextaction = AC_TOURNE_SO;
      if (nextaction != -1)
        break;

      if (toto[i].lastrot == +1)
        nextaction = AC_TOURNE_SO;
      if (toto[i].lastrot == -1)
        nextaction = AC_TOURNE_SE;
      if (nextaction != -1)
        break;

      nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_SO : AC_TOURNE_SE;
      break;

    case AC_MARCHE_N:
      left.x--;
      leftob = GetMur (left, 1);
      right.x++;
      rightob    = GetMur (right, 1);
      nextaction = -1;
      if (
        (left.x != toto[i].lastpos.x || left.y != toto[i].lastpos.y) &&
        leftob == 0)
        nextaction = AC_TOURNE_NO;
      if (
        (right.x != toto[i].lastpos.x || right.y != toto[i].lastpos.y) &&
        rightob == 0)
        nextaction = AC_TOURNE_NE;
      if (nextaction != -1)
        break;

      if (leftob == 0)
        nextaction = AC_TOURNE_NO;
      if (rightob == 0)
        nextaction = AC_TOURNE_NE;
      if (nextaction != -1)
        break;

      if (toto[i].lastrot == +1)
        nextaction = AC_TOURNE_NE;
      if (toto[i].lastrot == -1)
        nextaction = AC_TOURNE_NO;
      if (nextaction != -1)
        break;

      nextaction = (GetRandom (0, 0, 2) == 0) ? AC_TOURNE_NO : AC_TOURNE_NE;
      break;

    default:
      break;
    }
  }

  NewAction (i, nextaction, 0);
}

/* ---------- */
/* BombeBaoum */
/* ---------- */

/*
    Fait péter toutes les bombes correspondant à un détonateur.
 */

void
BombeBaoum (short detonateur)
{
  short bombe;
  Pt    cel;

  if (detonateur == ICO_DETONATEUR_A)
    bombe = ICO_BOMBE_A;
  if (detonateur == ICO_DETONATEUR_B)
    bombe = ICO_BOMBE_B;
  if (detonateur == ICO_DETONATEUR_C)
    bombe = ICO_BOMBE_C;

  for (cel.y = 0; cel.y < MAXCELY; cel.y++)
  {
    for (cel.x = 0; cel.x < MAXCELX; cel.x++)
    {
      if (bombe == DecorGetCel (cel))
      {
        if (bombe == ICO_BOMBE_A)
          ObjetPut (cel, OB_BOMBEA);
        if (bombe == ICO_BOMBE_B)
          ObjetPut (cel, OB_BOMBEB);
        if (bombe == ICO_BOMBE_C)
          ObjetPut (cel, OB_BOMBEC);
      }
    }
  }
}

/* ----------- */
/* SauteAction */
/* ----------- */

/*
    Regarde s'il est possible de sauter par dessus un objet du décor
    pas trop haut.
    Retourne 0 si c'est possible.
 */

short
SauteAction (short i, Pt celsaut)
{
  short icon, obstacle;
  Pt    celarr;

  if (
    toto[i].force < 5 || toto[i].vision == 0 || toto[i].mechant == 1 ||
    toto[i].tank != 0)
    return 1; /* impossible */

  celarr.x = 2 * celsaut.x - toto[i].poscel.x;
  celarr.y = 2 * celsaut.y - toto[i].poscel.y;
  obstacle = GetObstacle (celarr, 1);
  if (obstacle != 0 && obstacle != ICO_AIMANT)
    return 1;

  icon = GetObstacle (celsaut, 1);
  if (
    (icon >= ICO_PLANTEBAS &&
     icon <= ICO_PLANTEBAS_D) || /* obstacle pas trop haut ? */
    (icon >= ICO_ELECTROBAS &&
     icon <= ICO_ELECTROBAS_D) || /* obstacle pas trop haut ? */
    icon == ICO_LUNETTES ||       /* lunettes ? */
    icon == ICO_LIVRE ||          /* livre ? */
    icon == ICO_MEUBLE + 8 ||     /* tabouret ? */
    icon == ICO_TROU)             /* trou ? */
  {
    if (!g_passpower)
      toto[i].force -= 3;

    switch (GetOrientation (toto[i].action))
    {
    case AC_MARCHE_E:
      NewAction (i, AC_SAUTE1_E, 0);
      return 0;
    case AC_MARCHE_O:
      NewAction (i, AC_SAUTE1_O, 0);
      return 0;
    case AC_MARCHE_S:
      NewAction (i, AC_SAUTE1_S, 0);
      return 0;
    case AC_MARCHE_N:
      NewAction (i, AC_SAUTE1_N, 0);
      return 0;
    default:
      return 1;
    }
  }

  return 1; /* impossible */
}

/* -------------- */
/* IfPousseCaisse */
/* -------------- */

/*
    Vérifie s'il est possible de pousser une caisse.
    Démarre éventuellement les actions associées.
    Si oui, retourne 1 (vrai).
 */

short
IfPousseCaisse (Pt celarr, short caisse, Action orientation)
{
  short icon;

  icon = GetObstacle (celarr, 1);

  if (
    icon == 0 || icon == ICO_TROU || icon == ICO_BAISSEBAS ||
    (icon >= ICO_SENSUNI_S && icon <= ICO_SENSUNI_O) ||
    (icon >= ICO_ACCEL_S && icon <= ICO_ACCEL_O) ||
    ((icon == ICO_VITRE + 4 || icon == ICO_VITRE + 5) && caisse == ICO_CAISSEO))
  {
    if (icon == ICO_TROU)
    {
      DecorModif (celarr, ICO_TROUBOUCHE); /* met un trou bouché */
    }
    if (icon == ICO_SENSUNI_O && orientation == AC_MARCHE_E)
    {
      if (MoveGetCel (celarr) == 0)
        ObjetPut (celarr, OB_SENSUNIO); /* monte puis redescend la flèche */
      return 0;                         /* impossible */
    }
    if (icon == ICO_SENSUNI_E && orientation == AC_MARCHE_O)
    {
      if (MoveGetCel (celarr) == 0)
        ObjetPut (celarr, OB_SENSUNIE); /* monte puis redescend la flèche */
      return 0;                         /* impossible */
    }
    if (icon == ICO_SENSUNI_N && orientation == AC_MARCHE_S)
    {
      if (MoveGetCel (celarr) == 0)
        ObjetPut (celarr, OB_SENSUNIN); /* monte puis redescend la flèche */
      return 0;                         /* impossible */
    }
    if (icon == ICO_SENSUNI_S && orientation == AC_MARCHE_N)
    {
      if (MoveGetCel (celarr) == 0)
        ObjetPut (celarr, OB_SENSUNIS); /* monte puis redescend la flèche */
      return 0;                         /* impossible */
    }
    if (icon == ICO_VITRE + 4)
    {
      ObjetPut (celarr, OB_VITRENS); /* casse la vitre */
    }
    if (icon == ICO_VITRE + 5)
    {
      ObjetPut (celarr, OB_VITREEO); /* casse la vitre */
    }
    return 1; /* toto peut pousser */
  }

  return 0; /* impossible */
}

/* ----------------- */
/* SearchTotoForTank */
/* ----------------- */

/*
    Retourne -1 s'il n'y a rien (pas de toto ici).
    Autrement, retourne le rang du toto.
 */

short
SearchTotoForTank (Pt cel)
{
  short i;

  if (cel.x < 0 || cel.x >= MAXCELX || cel.y < 0 || cel.y >= MAXCELY)
    return -1; /* sort du monde */

  for (i = 0; i < MAXTOTO; i++)
  {
    if (
      toto[i].status == STVIDE || toto[i].tank != 0 ||
      toto[i].invincible != 0 || toto[i].action == AC_BALLON_E ||
      toto[i].action == AC_DEPART_E ||
      (toto[i].action >= AC_ARRIVEE_E && toto[i].action <= AC_ARRIVEE_S) ||
      toto[i].action == AC_ARRIVEE_M || toto[i].action == AC_BALLON_M ||
      (toto[i].action >= AC_CAISSE_E && toto[i].action <= AC_CAISSEG_T))
      continue;

    if (toto[i].poscel.x == cel.x && toto[i].poscel.y == cel.y)
      return i;
    if (toto[i].poscela.x == cel.x && toto[i].poscela.y == cel.y)
      return i;
    if (toto[i].poscelb.x == cel.x && toto[i].poscelb.y == cel.y)
      return i;
  }

  return -1; /* pas de toto ici */
}

/* ---------- */
/* SpecAction */
/* ---------- */

/*
    Effectue une action spéciale, en fonction de l'obstacle rencontré.
    Retourne 1 si une action est entreprise.
 */

short
SpecAction (short i, short obstacle, Pt testcel)
{
  Action orientation, nextaction;
  Pt     cel, cel2;
  short  icon, rang;

  orientation = GetOrientation (toto[i].action);

  if (obstacle == ICO_TROU && !g_passhole) /* tombe dans un trou ? */
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    if (toto[i].tank == 0)
    {
      NewAction (i, orientation + AC_TOMBE_E - AC_MARCHE_E, 0);
    }
    if (toto[i].tank == 1 || toto[i].tank == 2)
    {
      NewAction (i, orientation + AC_TOMBE_TANK_E - AC_MARCHE_E, 0);
    }
    if (toto[i].tank == 3)
    {
      NewAction (i, orientation + AC_TOMBE_TANKB_E - AC_MARCHE_E, 0);
    }
    return 1;
  }

  if (toto[i].tank != 0)
  {
    if (
      obstacle == 1 || obstacle == 2 || /* tank arrive sur un toto ? */
      (obstacle >= ICO_SENSUNI_S && obstacle <= ICO_SENSUNI_O))
    {
      rang = SearchTotoForTank (testcel);
      if (rang >= 0)
      {
        if (toto[rang].action == AC_TANK)
        {
          NewAction (i, orientation - AC_MARCHE_E + AC_STOP_E, 0);
          return 1;
        }
        else
        {
          NewAction (rang, AC_TANK, 0); /* toto se fait bouziller */
          return 0;
        }
      }
    }
  }

  if (
    obstacle == ICO_SENSUNI_S && /* sens unique vers le sud ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_N)
    {
      if (toto[i].joueur)
        celcap1.x = -1;
      TourneAction (i);
      ObjetPut (testcel, OB_SENSUNIS); /* monte puis redescend la flèche */
    }
    else
    {
      NewAction (i, orientation, 0); /* passe sur la flèche */
    }
    return 1;
  }

  if (
    obstacle == ICO_SENSUNI_N && /* sens unique vers le nord ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_S)
    {
      if (toto[i].joueur)
        celcap1.x = -1;
      TourneAction (i);
      ObjetPut (testcel, OB_SENSUNIN); /* monte puis redescend la flèche */
    }
    else
    {
      NewAction (i, orientation, 0); /* passe sur la flèche */
    }
    return 1;
  }

  if (
    obstacle == ICO_SENSUNI_E && /* sens unique vers l'est ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_O)
    {
      if (toto[i].joueur)
        celcap1.x = -1;
      TourneAction (i);
      ObjetPut (testcel, OB_SENSUNIE); /* monte puis redescend la flèche */
    }
    else
    {
      NewAction (i, orientation, 0); /* passe sur la flèche */
    }
    return 1;
  }

  if (
    obstacle == ICO_SENSUNI_O && /* sens unique vers l'ouest ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_E)
    {
      if (toto[i].joueur)
        celcap1.x = -1;
      TourneAction (i);
      ObjetPut (testcel, OB_SENSUNIO); /* monte puis redescend la flèche */
    }
    else
    {
      NewAction (i, orientation, 0); /* passe sur la flèche */
    }
    return 1;
  }

  if (
    obstacle == ICO_ACCEL_S && /* accélérateur vers le sud ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_S)
      toto[i].vitesse = +2;
    if (orientation == AC_MARCHE_N)
      toto[i].vitesse = -2;
    NewAction (i, orientation, 0);
    return 1;
  }

  if (
    obstacle == ICO_ACCEL_E && /* accélérateur vers l'est ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_E)
      toto[i].vitesse = +2;
    if (orientation == AC_MARCHE_O)
      toto[i].vitesse = -2;
    NewAction (i, orientation, 0);
    return 1;
  }

  if (
    obstacle == ICO_ACCEL_N && /* accélérateur vers le nord ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_N)
      toto[i].vitesse = +2;
    if (orientation == AC_MARCHE_S)
      toto[i].vitesse = -2;
    NewAction (i, orientation, 0);
    return 1;
  }

  if (
    obstacle == ICO_ACCEL_O && /* accélérateur vers l'ouest ? */
    MoveGetCel (testcel) == 0)
  {
    if (orientation == AC_MARCHE_O)
      toto[i].vitesse = +2;
    if (orientation == AC_MARCHE_E)
      toto[i].vitesse = -2;
    NewAction (i, orientation, 0);
    return 1;
  }

  if (toto[i].tank == 3)
  {
    if (obstacle == ICO_VITRE + 4)
    {
      ObjetPut (testcel, OB_VITRENS); /* casse la vitre */
      NewAction (i, orientation, 0);
      return 1;
    }
    if (obstacle == ICO_VITRE + 5)
    {
      ObjetPut (testcel, OB_VITREEO); /* casse la vitre */
      NewAction (i, orientation, 0);
      return 1;
    }
  }

  if (toto[i].tank != 0)
    return 0; /* le tank ne fait rien d'autre */

  if (obstacle == ICO_ARRIVEE) /* est-on sur l'arrivée ? */
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    if (toto[i].mechant == 0)
    {
      toto[i].vision = 1;
      toto[i].force  = 30;
      NewAction (i, orientation + AC_ARRIVEE_E - AC_MARCHE_E, 0);
      DecorModif (
        testcel, ICO_ARRIVEEPRIS); /* le ballon est pris (si autre toto) */
    }
    else
    {
      NewAction (i, orientation + AC_MECHANT_E - AC_MARCHE_E, 0);
      ObjetPut (testcel, OB_BALLONEX); /* fait exploser le ballon */
    }
    return 1;
  }

  if (obstacle == ICO_AIMANT) /* arrive à un aimant ? */
  {
    DecorModif (
      testcel, DecorGetInitCel (testcel)); /* enlève l'aimant du décor */
    PlaySound (SOUND_AIMANT, &toto[i].poscel);
    NewAction (i, orientation, 0);
    return 1;
  }

  if (
    obstacle == ICO_LUNETTES && /* arrive aux lunettes ? */
    toto[i].vision == 0)
  {
    toto[i].vision = 1;

    DecorModif (
      testcel, DecorGetInitCel (testcel)); /* enlève les lunettes du décor */

    NewAction (i, orientation + AC_VISION_E - AC_MARCHE_E, 0);
    return 1;
  }

  if (
    (obstacle == ICO_TABLEBOIT ||    /* arrive à une table ? */
     obstacle == ICO_TABLEPOISON) && /* arrive à une table ? */
    toto[i].mechant == 0 &&
    (toto[i].lastobjet.x != testcel.x || toto[i].lastobjet.y != testcel.y))
  {
    if (toto[i].joueur)
      celcap1.x = -1;

    nextaction = orientation + AC_BOIT_E - AC_MARCHE_E;
    if (!g_passpower)
    {
      if (obstacle == ICO_TABLEPOISON)
      {
        toto[i].vision = 0;
        toto[i].force  = 0;
        toto[i].joueur = 0;
        nextaction     = orientation + AC_BOITX_E - AC_MARCHE_E;
      }
      else
      {
        toto[i].force += 30;
      }
    }

    DecorModif (testcel, ICO_TABLEVIDE); /* enlève la bouteille sur la table */

    if (toto[i].joueur == 0)
      toto[i].lastobjet = testcel;
    toto[i].cntnotuse = 30;

    NewAction (i, nextaction, 0);

    toto[i].rstdecorcel  = testcel;
    toto[i].rstdecoricon = obstacle; /* faudra remettre la bouteille */
    return 1;
  }

  if (
    obstacle == ICO_TOURTE && /* arrive à une tourte ? */
    toto[i].mechant == 0)
  {
    if (toto[i].joueur)
      celcap1.x = -1;

    DecorModif (testcel, ICO_TOURTE + 1); /* enlève la tourte sur la table */
    NewAction (i, orientation + AC_TOURTE_E - AC_MARCHE_E, 0);
    return 1;
  }

  if (
    obstacle == ICO_PIANO && /* arrive au piano ? */
    toto[i].mechant == 0 &&
    (toto[i].lastobjet.x != testcel.x || toto[i].lastobjet.y != testcel.y))
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    if (orientation == AC_MARCHE_O)
    {
      if (toto[i].joueur == 0)
        toto[i].lastobjet = testcel;
      toto[i].cntnotuse = 10;
      NewAction (i, AC_PIANO_O, 0);
      return 1;
    }
  }

  if (
    obstacle == ICO_MAGIC && /* arrive à la magie ? */
    toto[i].mechant == 0)
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    NewAction (i, orientation + AC_MAGIC_E - AC_MARCHE_E, 0);

    toto[i].rstdecorcel  = testcel; /* faudra enlever la magie */
    toto[i].rstdecoricon = DecorGetInitCel (testcel);
    return 1;
  }

  if (
    obstacle == ICO_LIVRE && /* arrive à un livre ? */
    toto[i].vision == 1 && toto[i].mechant == 0 &&
    (toto[i].lastobjet.x != testcel.x || toto[i].lastobjet.y != testcel.y))
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    DecorModif (testcel, DecorGetInitCel (testcel)); /* enlève le livre */

    toto[i].lastobjet = testcel;
    toto[i].cntnotuse = 30;

    NewAction (i, orientation + AC_LIVRE_E - AC_MARCHE_E, 0);

    toto[i].rstdecorcel  = testcel;
    toto[i].rstdecoricon = ICO_LIVRE; /* faudra remettre le livre */
    return 1;
  }

  if (
    ((obstacle >= ICO_MEUBLE + 0 && /* arrive à une commode ? */
      obstacle <= ICO_MEUBLE + 3) ||
     (obstacle >= ICO_MEUBLE + 6 && obstacle <= ICO_MEUBLE + 7)) &&
    toto[i].vision == 1 && toto[i].mechant == 0 &&
    (toto[i].lastobjet.x != testcel.x || toto[i].lastobjet.y != testcel.y))
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    toto[i].lastobjet = testcel;
    toto[i].cntnotuse = 30;

    NewAction (i, orientation + AC_MUSIQUE_E - AC_MARCHE_E, 0);
    return 1;
  }

  if (
    obstacle >= ICO_CLE_A && /* arrive à une clé ? */
    obstacle <= ICO_CLE_C &&
    (toto[i].cles & (1 << (obstacle - ICO_CLE_A))) == 0 && toto[i].mechant == 0)
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    toto[i].cles |= 1 << (obstacle - ICO_CLE_A);
    DecorModif (testcel, ICO_CLE_VIDE); /* décroche la clé */
    NewAction (i, orientation + AC_CLE_E - AC_MARCHE_E, 0);
    return 1;
  }

  if (
    obstacle >= ICO_PORTEF_EO && /* arrive à une porte est-ouest fermée ? */
    obstacle < ICO_PORTEF_EO + 3 &&
    (orientation == AC_MARCHE_E || orientation == AC_MARCHE_O) &&
    toto[i].cles & (1 << (obstacle - ICO_PORTEF_EO)) && toto[i].mechant == 0)
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    toto[i].cles &= ~(1 << (obstacle - ICO_PORTEF_EO));
    DecorModif (
      testcel, obstacle + (ICO_PORTEO_EO - ICO_PORTEF_EO)); /* ouvre la porte */
    NewAction (i, orientation + AC_PORTE_E - AC_MARCHE_E, 0);
    return 1;
  }

  if (
    obstacle >= ICO_PORTEF_NS && /* arrive à une porte nord-sud fermée ? */
    obstacle < ICO_PORTEF_NS + 3 &&
    (orientation == AC_MARCHE_N || orientation == AC_MARCHE_S) &&
    toto[i].cles & (1 << (obstacle - ICO_PORTEF_NS)) && toto[i].mechant == 0)
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    toto[i].cles &= ~(1 << (obstacle - ICO_PORTEF_NS));
    DecorModif (
      testcel, obstacle + (ICO_PORTEO_NS - ICO_PORTEF_NS)); /* ouvre la porte */
    NewAction (i, orientation + AC_PORTE_E - AC_MARCHE_E, 0);
    return 1;
  }

  if (
    obstacle == ICO_BAISSE && /* arrive à une porte électronique ouest ? */
    orientation == AC_MARCHE_O && toto[i].mechant == 0)
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    ObjetPut (testcel, OB_BAISSE); /* baisse la porte */
    NewAction (i, AC_BAISSE_O, 0);
    return 1;
  }

  if (obstacle == ICO_GLISSE) /* glisse sur une peau de banane ? */
  {
    if (toto[i].mechant || g_passnice)
    {
      NewAction (i, orientation, 0); /* passe par-dessus la peau de banane */
      DecorModif (testcel, DecorGetInitCel (testcel));
      return 1;
    }
    if (toto[i].joueur)
      celcap1.x = -1;
    toto[i].mechant = 1;  /* devient méchant */
    toto[i].vision  = 1;  /* voyant */
    toto[i].joueur  = 0;  /* ce n'est plus le joueur */
    toto[i].force   = 30; /* force constante */
    nbout++;              /* un autre pourra ressortir */
    NewAction (i, orientation + AC_GLISSE_E - AC_MARCHE_E, 0);

    toto[i].rstdecorcel  = testcel; /* faudra enlever la peau de banane */
    toto[i].rstdecoricon = DecorGetInitCel (testcel);
    return 1;
  }

  if (
    (obstacle == ICO_CAISSE || /* arrive derrière une caisse ? */
     obstacle == ICO_CAISSEV || obstacle == ICO_CAISSEO ||
     obstacle == ICO_CAISSEG) &&
    toto[i].mechant == 0 && toto[i].vision == 1)
  {
    if (toto[i].force > 5) /* assez de force pour pousser ? */
    {
      if (obstacle == ICO_CAISSE)
        vitessepousse = 14;
      if (obstacle == ICO_CAISSEV)
        vitessepousse = 8;
      if (obstacle == ICO_CAISSEO)
        vitessepousse = 10;
      if (obstacle == ICO_CAISSEG)
        vitessepousse = 10;
      cel.x  = 2 * testcel.x - toto[i].poscel.x;
      cel.y  = 2 * testcel.y - toto[i].poscel.y;
      cel2.x = -1;
      icon   = GetObstacle (cel, 1);
      if (obstacle == ICO_CAISSEV && icon == ICO_CAISSEV) /* 2 caisses ? */
      {
        vitessepousse = 20;
        cel2          = cel;
        cel.x         = 3 * testcel.x - 2 * toto[i].poscel.x;
        cel.y         = 3 * testcel.y - 2 * toto[i].poscel.y;
        icon          = GetObstacle (cel, 1);
      }
      if (IfPousseCaisse (cel, obstacle, orientation))
      {
        NewAction (i, orientation + AC_POUSSE_E - AC_MARCHE_E, 0);

        if (cel2.x != -1) /* 2 caisses à pousser ? */
        {
          DecorModif (cel2, DecorGetInitCel (cel2)); /* enlève la caisse fixe */
          rang = MoveInit (cel2, orientation + AC_CAISSEV_E - AC_MARCHE_E, 0);
          if (rang >= 0)
          {
            toto[rang].icon = ICO_CAISSEV;
            redraw          = 1; /* faudra tout redessiner */
          }
          if (GetRandom (1, 0, 5) == 0)
            PlaySound (SOUND_POUSSE, &toto[i].poscel);
        }
        else
        {
          if (obstacle == ICO_CAISSE && GetRandom (1, 0, 5) == 0)
            PlaySound (SOUND_POUSSE, &toto[i].poscel);
        }

        if (caisseocel.x == testcel.x && caisseocel.y == testcel.y)
        {
          caisseodir  = 0; /* ne roule pas toute seule */
          caisseoddir = 0; /* ne roule pas toute seule */
        }

        DecorModif (
          testcel, DecorGetInitCel (testcel)); /* enlève la caisse fixe */
        if (obstacle == ICO_CAISSE)
        {
          rang = MoveInit (testcel, orientation + AC_CAISSE_E - AC_MARCHE_E, 0);
        }
        if (obstacle == ICO_CAISSEV)
        {
          rang =
            MoveInit (testcel, orientation + AC_CAISSEV_E - AC_MARCHE_E, 0);
        }
        if (obstacle == ICO_CAISSEO)
        {
          rang =
            MoveInit (testcel, orientation + AC_CAISSEO_E - AC_MARCHE_E, 0);
        }
        if (obstacle == ICO_CAISSEG)
        {
          rang =
            MoveInit (testcel, orientation + AC_CAISSEG_E - AC_MARCHE_E, 0);
        }
        if (rang >= 0)
        {
          toto[rang].icon = obstacle;
          if (obstacle == ICO_CAISSEO || obstacle == ICO_CAISSEG)
          {
            toto[rang].energie = 2; /* énergie d'une boule poussée */
          }
          redraw = 1; /* faudra tout redessiner */
        }
        return 1;
      }
      else
      {
        goto paspousse;
      }
    }
    else
    {
      goto nonpousse;
    }
  paspousse:
    if (toto[i].joueur)
    {
      NewAction (i, orientation + AC_NPOUSSE_E - AC_MARCHE_E, 0);
      PlaySound (SOUND_POUSSE, &toto[i].poscel);
      return 1;
    }
  nonpousse:
    if (toto[i].joueur)
    {
      NewAction (i, orientation + AC_NON_E - AC_MARCHE_E, 0);
      return 1;
    }
  }

  if (
    obstacle == ICO_TECHNO1 && /* est-on sur un techno HT ? */
    orientation == AC_MARCHE_O && toto[i].mechant == 0 &&
    (toto[i].lastobjet.x != testcel.x || toto[i].lastobjet.y != testcel.y))
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    if (toto[i].joueur == 0)
      toto[i].lastobjet = testcel;
    if (!g_passpower)
    {
      toto[i].force  = 0;
      toto[i].vision = 0;
      toto[i].joueur = 0;
    }
    NewAction (i, AC_ELECTRO_O, 0); /* toto s'électrocute */
    return 1;
  }

  if (
    obstacle == ICO_TELE && /* est-on devant une télévision ? */
    orientation == AC_MARCHE_N && toto[i].vision == 1 && toto[i].mechant == 0 &&
    (toto[i].lastobjet.x != testcel.x || toto[i].lastobjet.y != testcel.y))
  {
    if (toto[i].joueur)
      celcap1.x = -1;
    if (toto[i].joueur == 0)
      toto[i].lastobjet = testcel;

    NewAction (i, AC_TELE_N, 0); /* toto regarde la télé */
    ObjetPut (testcel, OB_TELE); /* allume la télé */
    return 1;
  }

  if (
    obstacle >= ICO_DETONATEUR_A && obstacle <= ICO_DETONATEUR_C &&
    toto[i].mechant == 1)
  {
    NewAction (i, orientation + AC_SAUTEDET_E - AC_MARCHE_E, 0);
    if (obstacle == ICO_DETONATEUR_A)
      ObjetPut (testcel, OB_DETONATEURA);
    if (obstacle == ICO_DETONATEUR_B)
      ObjetPut (testcel, OB_DETONATEURB);
    if (obstacle == ICO_DETONATEUR_C)
      ObjetPut (testcel, OB_DETONATEURC);
    BombeBaoum (obstacle);
    return 1;
  }

  if (SauteAction (i, testcel) == 0)
    return 1;

  if (
    toto[i].magic > 0 &&
    ((obstacle >= ICO_MURHAUT && obstacle <= ICO_MURHAUT_D) ||
     (obstacle >= ICO_MURBAS && obstacle <= ICO_MURBAS_D)))
  {
    PlaySound (SOUND_PASSEMUR, &toto[i].poscel);
    NewAction (i, orientation, 0); /* toto traverse les murs */
    return 1;
  }

  return 0;
}

/* ------------- */
/* IfSuperAction */
/* ------------- */

/*
    Teste si une cellule contient une chouette action pour toto.
    Si back = 1, cela signifie qu'on teste une cellule située derrière
    toto (demi-tour pour y aller).
    Si oui, retourne 1 (true).
 */

short
IfSuperAction (short i, Pt cel, Action direction, short back)
{
  short icon;

  /* Si l'objet placé dans cette cellule est le dernier utilisé
      par ce toto, ne le considère pas comme chouette. */

  if (toto[i].lastobjet.x == cel.x && toto[i].lastobjet.y == cel.y)
    return 0;

  if (toto[i].tank != 0)
  {
    if (back == 0 && SearchTotoForTank (cel) >= 0)
      return 1; /* un toto */
    return 0;   /* ne s'intéresse à rien d'autre si tank */
  }

  icon = GetObstacle (cel, 0);

  if (icon == ICO_AIMANT)
    return 1; /* aimant */

  if (
    toto[i].vision == 0 || /* ne s'intéresse à rien d'autre si aveugle */
    back)
    return 0; /* ne s'intéresse à rien d'autre si dans le dos */

  if (icon == ICO_ARRIVEE)
    return 1; /* arrivée */

  if (
    toto[i].mechant == 1 && icon >= ICO_DETONATEUR_A &&
    icon <= ICO_DETONATEUR_C)
    return 1;

  if (toto[i].mechant == 1)
    return 0; /* ne s'intéresse à rien d'autre si méchant */

  if (
    icon == ICO_TABLEBOIT ||   /* table avec boisson ? */
    icon == ICO_TABLEPOISON || /* table avec poison ? */
    icon == ICO_MAGIC ||       /* magie ? */
    icon == ICO_LIVRE)
    return 1; /* livre ? */

  if (
    icon >= ICO_CLE_A && /* clé A à C ? */
    icon <= ICO_CLE_C)
  {
    if ((toto[i].cles & (1 << (icon - ICO_CLE_A))) == 0)
      return 1;
  }

  if (
    (icon == ICO_PORTEF_EO + 0 || icon == ICO_PORTEF_NS + 0) &&
    toto[i].cles & (1 << 0))
    return 1;

  if (
    (icon == ICO_PORTEF_EO + 1 || icon == ICO_PORTEF_NS + 1) &&
    toto[i].cles & (1 << 1))
    return 1;

  if (
    (icon == ICO_PORTEF_EO + 2 || icon == ICO_PORTEF_NS + 2) &&
    toto[i].cles & (1 << 2))
    return 1;

  if (
    icon == ICO_BAISSE && /* porte électronique */
    direction == AC_MARCHE_O)
    return 1;

  return 0;
}

/* ------------ */
/* IfLineAction */
/* ------------ */

/*
    Regarde s'il y a une cellule intéressante dans une direction donnée.
    Si oui, retourne 1 (true).
 */

short
IfLineAction (short i, Pt cel, Action direction, short back)
{
  short nb, icon, obstacle;
  Pt    dir, pos;

  dir.x = 0;
  dir.y = 0;
  switch (direction)
  {
  case AC_MARCHE_E:
    dir.x++;
    break;
  case AC_MARCHE_O:
    dir.x--;
    break;
  case AC_MARCHE_S:
    dir.y++;
    break;
  case AC_MARCHE_N:
    dir.y--;
    break;
  default:
    return 0;
  }

  for (nb = 0; nb < 10; nb++)
  {
    cel.x += dir.x;
    cel.y += dir.y;

    /* retourne true si objet intéressant */
    if (IfSuperAction (i, cel, direction, back))
      return 1;

    if (toto[i].tank == 0)
    {
      icon = GetObstacle (cel, 0); /* ne tient pas compte des toto */
    }
    else
    {
      icon = GetObstacle (cel, 1); /* tient compte des toto */
    }

    if (
      toto[i].force >= 5 && /* force pour sauter ? */
      toto[i].vision == 1 && toto[i].mechant == 0 && toto[i].tank == 0)
    {
      if (
        (icon >= ICO_PLANTEBAS &&
         icon <= ICO_PLANTEBAS_D) || /* obstacle pas trop haut ? */
        (icon >= ICO_ELECTROBAS &&
         icon <= ICO_ELECTROBAS_D) || /* obstacle pas trop haut ? */
        icon == ICO_LUNETTES)         /* lunettes ? */
      {
        pos.x    = cel.x + dir.x;
        pos.y    = cel.y + dir.y;
        obstacle = GetObstacle (pos, 0);
        if (
          obstacle == 0 || /* libre après l'obstacle bas ? */
          obstacle == ICO_AIMANT)
        {
          pos.x    = cel.x - dir.x;
          pos.y    = cel.y - dir.y;
          obstacle = GetObstacle (pos, 0);
          if (
            obstacle == 0 || /* libre avant l'obstacle bas ? */
            obstacle == ICO_AIMANT)
          {
            continue;
          }
        }
      }
    }

    if (
      icon == ICO_GLISSE || /* peau de banane ? */
      icon == ICO_TROU ||   /* trou ? */
      (icon >= ICO_SENSUNI_S &&
       icon <= ICO_SENSUNI_O) || /* le regard passe par-dessus un sens-unique */
      (icon >= ICO_ACCEL_S &&
       icon <= ICO_ACCEL_O)) /* le regard passe par-dessus un accélérateur */
    {
      continue;
    }

    if (icon != 0)
      return 0; /* retourne false si obstacle */
  }
  return 0;
}

/* -------- */
/* TurnCoin */
/* -------- */

/*
    Teste si toto dépasse un mur en coin, et s'il faut éventuellement
    tourner plutôt que de continuer toujours tout droit.
    Retourne 1 (true) si toto decide de tourner.
 */

short
TurnCoin (short i, Action * pnextaction)
{
  Pt     left, right, back, bleft, bright;
  Action acleft, acright;

  if (toto[i].vision == 0)
    return 0; /* si toto aveugle -> ne cherche pas */
  if (GetRandom (0, 0, 2) == 0)
    return 0; /* seulement une fois sur deux ! */

  left   = toto[i].poscel;
  right  = toto[i].poscel;
  back   = toto[i].poscel;
  bleft  = toto[i].poscel;
  bright = toto[i].poscel;

  switch (*pnextaction)
  {
  case AC_MARCHE_E:
    left.y--;
    right.y++;
    back.x--;
    bleft.x--;
    bleft.y--;
    bright.x--;
    bright.y++;
    acleft  = AC_TOURNE_EN;
    acright = AC_TOURNE_ES;
    break;
  case AC_MARCHE_O:
    left.y++;
    right.y--;
    back.x++;
    bleft.x++;
    bleft.y++;
    bright.x++;
    bright.y--;
    acleft  = AC_TOURNE_OS;
    acright = AC_TOURNE_ON;
    break;
  case AC_MARCHE_S:
    left.x++;
    right.x--;
    back.y--;
    bleft.x++;
    bleft.y--;
    bright.x--;
    bright.y--;
    acleft  = AC_TOURNE_SE;
    acright = AC_TOURNE_SO;
    break;
  case AC_MARCHE_N:
    left.x--;
    right.x++;
    back.y++;
    bleft.x--;
    bleft.y++;
    bright.x++;
    bright.y++;
    acleft  = AC_TOURNE_NO;
    acright = AC_TOURNE_NE;
    break;
  default:
    return 0;
  }

  if (GetMur (back, 0) >= ICO_BLOQUE)
    return 0; /* mur dans le dos ? */

  if (
    GetMur (bleft, 0) >= ICO_BLOQUE && /* mur à l'arrière gauche ? */
    GetMur (left, 1) == 0 &&           /* passage à gauche ? */
    (left.x != toto[i].lastpos.x ||
     left.y != toto[i].lastpos.y)) /* ne vient-on pas déjà de là ? */
  {
    *pnextaction = acleft;
    return 1;
  }

  if (
    GetMur (bright, 0) >= ICO_BLOQUE && /* mur à l'arrière droite ? */
    GetMur (right, 1) == 0 &&           /* passage à droite ? */
    (right.x != toto[i].lastpos.x ||
     right.y != toto[i].lastpos.y)) /* ne vient-on pas déjà de là ? */
  {
    *pnextaction = acright;
    return 1;
  }

  return 0;
}

/* ------------ */
/* VisionAction */
/* ------------ */

/*
    Si toto est doté de la vision, regarde s'il n'y a pas mieux à faire
    que de marcher tout droit.
    Retourne 1 (true) si y'a qq chose de mieux à faire.
 */

short
VisionAction (short i, Action * pnextaction)
{
  /* S'il s'agit d'un tank immobile, on regarde dans les 4 directions. */

  if (toto[i].tank == 4) /* tank immobile ? */
  {
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_S, 0))
    {
      *pnextaction = AC_MARCHE_S;
      return 1;
    }
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_N, 0))
    {
      *pnextaction = AC_MARCHE_N;
      return 1;
    }
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_E, 0))
    {
      *pnextaction = AC_MARCHE_E;
      return 1;
    }
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_O, 0))
    {
      *pnextaction = AC_MARCHE_O;
      return 1;
    }
    return 0;
  }

  /* Si toto s'est déjà fixé un but, ne change pas d'avis ! */

  if (IfLineAction (i, toto[i].poscel, *pnextaction, 0))
    return 1;

  if (*pnextaction == AC_MARCHE_E)
  {
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_S, 0))
    {
      *pnextaction = AC_TOURNE_ES;
      return 1;
    }
    if (
      IfLineAction (i, toto[i].poscel, AC_MARCHE_N, 0) ||
      IfLineAction (i, toto[i].poscel, AC_MARCHE_O, 1))
    {
      *pnextaction = AC_TOURNE_EN;
      return 1;
    }

    if (TurnCoin (i, pnextaction))
      return 1;
  }

  if (*pnextaction == AC_MARCHE_S)
  {
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_E, 0))
    {
      *pnextaction = AC_TOURNE_SE;
      return 1;
    }
    if (
      IfLineAction (i, toto[i].poscel, AC_MARCHE_O, 0) ||
      IfLineAction (i, toto[i].poscel, AC_MARCHE_N, 1))
    {
      *pnextaction = AC_TOURNE_SO;
      return 1;
    }

    if (TurnCoin (i, pnextaction))
      return 1;
  }

  if (*pnextaction == AC_MARCHE_O)
  {
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_S, 0))
    {
      *pnextaction = AC_TOURNE_OS;
      return 1;
    }
    if (
      IfLineAction (i, toto[i].poscel, AC_MARCHE_N, 0) ||
      IfLineAction (i, toto[i].poscel, AC_MARCHE_E, 1))
    {
      *pnextaction = AC_TOURNE_ON;
      return 1;
    }

    if (TurnCoin (i, pnextaction))
      return 1;
  }

  if (*pnextaction == AC_MARCHE_N)
  {
    if (IfLineAction (i, toto[i].poscel, AC_MARCHE_E, 0))
    {
      *pnextaction = AC_TOURNE_NE;
      return 1;
    }
    if (
      IfLineAction (i, toto[i].poscel, AC_MARCHE_O, 0) ||
      IfLineAction (i, toto[i].poscel, AC_MARCHE_S, 1))
    {
      *pnextaction = AC_TOURNE_NO;
      return 1;
    }

    if (TurnCoin (i, pnextaction))
      return 1;
  }

  return 0;
}

/* ------------- */
/* SoundAmbiance */
/* ------------- */

/*
    Fait éventuellement entendre un son d'ambiance selon
    un obstacle rencontré.
 */

void
SoundAmbiance (short obstacle, const Pt * cel)
{
  if (
    obstacle == ICO_TECHNO1 + 1 || obstacle == ICO_TECHNO1 + 2 ||
    obstacle == ICO_TECHNO1 + 3 || obstacle == ICO_TECHNO2 + 1 ||
    obstacle == ICO_TECHNO2 + 2)
  {
    PlaySound (SOUND_MACHINE, cel);
  }

  if (obstacle >= ICO_PLANTEHAUT && obstacle <= ICO_PLANTEHAUT_D)
  {
    PlaySound (SOUND_OISEAUX, cel);
  }
}

/* ------------ */
/* JoueurAction */
/* ------------ */

/*
    Cherche une action pour le toto piloté par le joueur.
 */

void
JoueurAction (short i, char event, Action orientation, Pt testcel)
{
  short     obstacle;
  short     retry = 0;
  KeyStatus keystatus;
  Pt        backcel;
  short     nextrepos;
  short     eventcont;

  obstacle  = GetObstacle (testcel, 1);
  keystatus = GetKeyStatus ();

  if (g_typejeu == 1 && g_modetelecom == 1 && g_pause == 0)
  {
    keystatus &= ~STLEFT;
    keystatus &= ~STRIGHT;
  }

  nextrepos         = toto[i].nextrepos;
  toto[i].nextrepos = 50; /* pas de repos avant longtemps si toto actif */

  /* Déplace le toto du joueur selon les actions de la télécommande. */

  eventcont = event;
  if (event == 0 && lasttelecom != 0)
  {
    eventcont = lasttelecom; /* continue si bouton toujours pressé */
  }

  if (eventcont == KEYGOFRONT) /* avance */
  {
    // lasttelecom = eventcont;
    celcap1.x = -1;
    if (SpecAction (i, obstacle, testcel))
      return;
    if (obstacle == 0)
    {
      NewAction (i, orientation, 0); /* continue d'avancer */
      return;
    }
    SoundAmbiance (obstacle, &toto[i].poscel);
    NewAction (i, orientation + AC_STOP_E - AC_MARCHE_E, 0);
    return;
  }

  if (eventcont == KEYGOBACK) /* recule */
  {
    // lasttelecom = eventcont;
    celcap1.x = -1;
    backcel.x = 2 * toto[i].poscel.x - testcel.x;
    backcel.y = 2 * toto[i].poscel.y - testcel.y;
    if (GetObstacle (backcel, 1) == 0)
    {
      NewAction (i, orientation + AC_RECULE_E - AC_MARCHE_E, 0);
      return;
    }
    NewAction (i, orientation + AC_STOP_E - AC_MARCHE_E, 0);
    return;
  }

  if (event == KEYGOLEFT) /* tourne à gauche */
  {
    celcap1.x = -1;
    if (orientation == AC_MARCHE_E)
    {
      NewAction (i, AC_TOURNE_EN, 0);
      return;
    }
    if (orientation == AC_MARCHE_S)
    {
      NewAction (i, AC_TOURNE_SE, 0);
      return;
    }
    if (orientation == AC_MARCHE_O)
    {
      NewAction (i, AC_TOURNE_OS, 0);
      return;
    }
    if (orientation == AC_MARCHE_N)
    {
      NewAction (i, AC_TOURNE_NO, 0);
      return;
    }
  }

  if (event == KEYGORIGHT) /* tourne à droite */
  {
    celcap1.x = -1;
    if (orientation == AC_MARCHE_E)
    {
      NewAction (i, AC_TOURNE_ES, 0);
      return;
    }
    if (orientation == AC_MARCHE_S)
    {
      NewAction (i, AC_TOURNE_SO, 0);
      return;
    }
    if (orientation == AC_MARCHE_O)
    {
      NewAction (i, AC_TOURNE_ON, 0);
      return;
    }
    if (orientation == AC_MARCHE_N)
    {
      NewAction (i, AC_TOURNE_NE, 0);
      return;
    }
  }

  /* Déplace le toto du joueur selon les touches flèches. */

rekey:
  if (keystatus == STRIGHT) /* va à droite */
  {
    if (retry == 0)
      celcap1.x = -1;
    if (orientation == AC_MARCHE_E)
    {
      if (SpecAction (i, obstacle, testcel))
        return;
    }
    if (orientation == AC_MARCHE_S)
    {
      NewAction (i, AC_TOURNE_SE, 0);
      return;
    }
    if (orientation == AC_MARCHE_N)
    {
      NewAction (i, AC_TOURNE_NE, 0);
      return;
    }
    if (orientation == AC_MARCHE_O)
    {
      NewAction (i, AC_TOURNE_ON, 0);
      return;
    }
    SoundAmbiance (obstacle, &toto[i].poscel);
  }

  if (keystatus == STDOWN) /* va en bas */
  {
    if (retry == 0)
      celcap1.x = -1;
    if (orientation == AC_MARCHE_S)
    {
      if (SpecAction (i, obstacle, testcel))
        return;
    }
    if (orientation == AC_MARCHE_E)
    {
      NewAction (i, AC_TOURNE_ES, 0);
      return;
    }
    if (orientation == AC_MARCHE_O)
    {
      NewAction (i, AC_TOURNE_OS, 0);
      return;
    }
    if (orientation == AC_MARCHE_N)
    {
      NewAction (i, AC_TOURNE_NE, 0);
      return;
    }
    SoundAmbiance (obstacle, &toto[i].poscel);
  }

  if (keystatus == STLEFT) /* va à gauche */
  {
    if (retry == 0)
      celcap1.x = -1;
    if (orientation == AC_MARCHE_O)
    {
      if (SpecAction (i, obstacle, testcel))
        return;
    }
    if (orientation == AC_MARCHE_S)
    {
      NewAction (i, AC_TOURNE_SO, 0);
      return;
    }
    if (orientation == AC_MARCHE_N)
    {
      NewAction (i, AC_TOURNE_NO, 0);
      return;
    }
    if (orientation == AC_MARCHE_E)
    {
      NewAction (i, AC_TOURNE_ES, 0);
      return;
    }
    SoundAmbiance (obstacle, &toto[i].poscel);
  }

  if (keystatus == STUP) /* va en haut */
  {
    if (retry == 0)
      celcap1.x = -1;
    if (orientation == AC_MARCHE_N)
    {
      if (SpecAction (i, obstacle, testcel))
        return;
    }
    if (orientation == AC_MARCHE_O)
    {
      NewAction (i, AC_TOURNE_ON, 0);
      return;
    }
    if (orientation == AC_MARCHE_E)
    {
      NewAction (i, AC_TOURNE_EN, 0);
      return;
    }
    if (orientation == AC_MARCHE_S)
    {
      NewAction (i, AC_TOURNE_SO, 0);
      return;
    }
    SoundAmbiance (obstacle, &toto[i].poscel);
  }

  if (obstacle == 0 && keystatus != 0)
  {
    if (retry == 0)
      celcap1.x = -1;
    NewAction (i, orientation, 0); /* continue d'avancer */
    return;
  }

  /* Déplace le toto du joueur selon le cap choisi. */

  if (
    celcap1.x == toto[i].poscel.x && celcap1.y == toto[i].poscel.y &&
    celcap2.x != -1)
  {
    celcap1   = celcap2; /* va sur le deuxième cap */
    celcap2.x = -1;
  }

  if (celcap1.x != -1 && retry == 0)
  {
    if (celcap1.y == toto[i].poscel.y && celcap1.x > toto[i].poscel.x)
    {
      keystatus = STRIGHT;
      retry++;
      goto rekey;
    }
    if (celcap1.y == toto[i].poscel.y && celcap1.x < toto[i].poscel.x)
    {
      keystatus = STLEFT;
      retry++;
      goto rekey;
    }
    if (celcap1.x == toto[i].poscel.x && celcap1.y > toto[i].poscel.y)
    {
      keystatus = STDOWN;
      retry++;
      goto rekey;
    }
    if (celcap1.x == toto[i].poscel.x && celcap1.y < toto[i].poscel.y)
    {
      keystatus = STUP;
      retry++;
      goto rekey;
    }
  }

  /* Si y'a vraiment rien d'autre à faire, stoppe. */

  celcap1.x = -1;

  toto[i].nextrepos = nextrepos;
  if (toto[i].nextrepos > 0)
    toto[i].nextrepos--;

  if (toto[i].force < 5 && toto[i].nextrepos == 0 && GetRandom (0, 0, 10) == 0)
  {
    NewAction (i, orientation + AC_DORT_E - AC_MARCHE_E, 0);
    toto[i].nextrepos = 50;
    return;
  }

  if (toto[i].nextrepos == 0 && GetRandom (0, 0, 20) == 0)
  {
    NewAction (i, orientation + AC_REPOS_E - AC_MARCHE_E, 0);
    toto[i].nextrepos = 20;
    return;
  }

  if (toto[i].force >= 5 && GetRandom (1, 0, 23) == 0)
  {
    NewAction (i, orientation + AC_REFLEXION_E - AC_MARCHE_E, 0);
    return;
  }

  if (toto[i].force >= 5 && GetRandom (1, 0, 5) == 0)
  {
    NewAction (i, orientation + AC_HAUSSE_E - AC_MARCHE_E, 0);
    return;
  }

  if (toto[i].nextrepos == 0 && GetRandom (1, 0, 30) == 0)
  {
    NewAction (i, orientation + AC_YOYO_E - AC_MARCHE_E, 0);
    toto[i].nextrepos = 25;
    return;
  }

  NewAction (i, orientation + AC_STOP_E - AC_MARCHE_E, 0);
}

/* ---------- */
/* IfFreeLine */
/* ---------- */

/*
    Vérifie si le trajet sur une ligne droite est libre d'obstacles.
    Si oui, retourne 1 (true).
    Ne teste pas la cellule d'arrivée !
 */

short
IfFreeLine (Pt depart, Pt arrivee)
{
  Pt    dir;
  short obstacle;

  dir.x = 0;
  dir.y = 0;

  if (depart.x == arrivee.x)
  {
    if (depart.y < arrivee.y)
      dir.y = +1;
    if (depart.y > arrivee.y)
      dir.y = -1;
  }
  if (depart.y == arrivee.y)
  {
    if (depart.x < arrivee.x)
      dir.x = +1;
    if (depart.x > arrivee.x)
      dir.x = -1;
  }

  if (dir.x == 0 && dir.y == 0)
    return 0;

  while (depart.x != arrivee.x || depart.y != arrivee.y)
  {
    obstacle = GetObstacle (depart, 0);
    depart.x += dir.x;
    depart.y += dir.y;

    if (
      (obstacle >= ICO_SENSUNI_S && obstacle <= ICO_SENSUNI_O) ||
      (obstacle >= ICO_ACCEL_S && obstacle <= ICO_ACCEL_O) ||
      (obstacle >= ICO_PLANTEBAS && obstacle <= ICO_PLANTEBAS_D) ||
      obstacle == ICO_ARRIVEE)
      continue;

    if (obstacle >= ICO_BLOQUE || obstacle == ICO_DEPART)
      return 0;
  }

  return 1; /* le trajet est libre */
}

/* --------- */
/* GetJoueur */
/* --------- */

/*
    Retourne le numéro du toto du joueur.
    Retourne -1 si y'en a pas !
 */

short
GetJoueur (void)
{
  short i;

  for (i = 0; i < MAXTOTO; i++)
  {
    if (toto[i].status != STVIDE && toto[i].joueur)
      return i;
  }
  return -1;
}

/* --------- */
/* JoueurCap */
/* --------- */

/*
    Cherche un nouveau cap à atteindre pour le toto du joueur en fonction
    de la cellule cliquée dans le décor.
 */

void
JoueurCap (char event, Pt pmouse)
{
  Pt    cel, inter;
  short i;

  cel = DecorDetCel (pmouse); /* cherche la cellule cliquée */
  if (cel.x < 0)
    goto error;

  i = GetJoueur ();
  if (i < 0)
    goto error;

  if (
    cel.x == toto[i].poscel.x && cel.y == toto[i].poscel.y &&
    toto[i].action <= AC_STOP_S && event == KEYCLIC)
  {
    switch (GetOrientation (toto[i].action))
    {
    case AC_MARCHE_E:
      NewAction (i, AC_TOURNE_EN, 0); /* effectue 1/4 de tour anti-horaire */
      break;
    case AC_MARCHE_O:
      NewAction (i, AC_TOURNE_OS, 0);
      break;
    case AC_MARCHE_S:
      NewAction (i, AC_TOURNE_SE, 0);
      break;
    case AC_MARCHE_N:
      NewAction (i, AC_TOURNE_NO, 0);
      break;
    default:
      break;
    }
    return;
  }

  if (
    cel.x == toto[i].poscel.x && cel.y == toto[i].poscel.y &&
    toto[i].action <= AC_STOP_S && event == KEYCLICR)
  {
    switch (GetOrientation (toto[i].action))
    {
    case AC_MARCHE_E:
      NewAction (i, AC_TOURNE_ES, 0); /* effectue 1/4 de tour horaire */
      break;
    case AC_MARCHE_O:
      NewAction (i, AC_TOURNE_ON, 0);
      break;
    case AC_MARCHE_S:
      NewAction (i, AC_TOURNE_SO, 0);
      break;
    case AC_MARCHE_N:
      NewAction (i, AC_TOURNE_NE, 0);
      break;
    default:
      break;
    }
    return;
  }

  if (cel.x == toto[i].poscel.x || cel.y == toto[i].poscel.y)
  {
    if (IfFreeLine (toto[i].poscel, cel)) /* accessible en ligne droite ? */
    {
      celcap1   = cel;
      celcap2.x = -1;
      return;
    }
    goto error;
  }

  inter.x = cel.x;
  inter.y = toto[i].poscel.y;
  if (
    IfFreeLine (toto[i].poscel, inter) && /* accessible en "L" ? */
    IfFreeLine (inter, cel))
  {
    celcap1 = inter;
    celcap2 = cel;
    return;
  }

  inter.x = toto[i].poscel.x;
  inter.y = cel.y;
  if (
    IfFreeLine (toto[i].poscel, inter) && /* accessible en "L" inversé ? */
    IfFreeLine (inter, cel))
  {
    celcap1 = inter;
    celcap2 = cel;
    return;
  }

error:
  celcap1.x = -1; /* aucun cap */
}

/* ---------- */
/* NextAction */
/* ---------- */

/*
    Cherche une action suivante pour un toto.
    Cette procédure est appelée chaque fois qu'une action quelconque
    est terminée, pour déterminer ce que toto fera ensuite.
 */

void
NextAction (char event, short i)
{
  Pt     testcel;
  Pt     p;
  Action nextaction;
  short  obstacle;
  short  orientation;

  if (
    toto[i].action >= AC_SAUTE1_E && /* saut commencé ? */
    toto[i].action <= AC_SAUTE1_S)
  {
    toto[i].poscel = toto[i].poscela;
    NewAction (
      i, toto[i].action + AC_SAUTE2_E - AC_SAUTE1_E,
      toto[i].offz); /* termine le saut */
    return;
  }

  if (
    toto[i].action >= AC_START_E && /* marche commencée ? */
    toto[i].action <= AC_START_S)
  {
    NewAction (i, toto[i].action - AC_START_E + AC_MARCHE_E, toto[i].offz);
    return;
  }

  back[i] = toto[i]; /* conserve l'état pour pouvoir faire un pas en arrière */

  if (
    (DecorGetCel (toto[i].poscel) == ICO_INVINCIBLE ||
     DecorGetCel (toto[i].poscela) == ICO_INVINCIBLE) &&
    toto[i].mechant == 0 && toto[i].tank == 0 &&
    toto[i].action != AC_BALLON_E && toto[i].action != AC_DEPART_E &&
    (toto[i].action < AC_ARRIVEE_E || toto[i].action > AC_ARRIVEE_S) &&
    toto[i].action != AC_ARRIVEE_M && toto[i].action != AC_BALLON_M &&
    (toto[i].action < AC_CAISSE_E || toto[i].action > AC_CAISSEG_T))
  {
    toto[i].invincible = 100; /* toto est invincible pour un temps */
  }

  if (toto[i].rstdecoricon != 0)
  {
    p = toto[i].lastobjet;
    DecorModif (
      toto[i].rstdecorcel, toto[i].rstdecoricon); /* restaure le décor */
    toto[i].rstdecoricon = 0;
    toto[i].lastobjet    = p;
  }

  if (
    toto[i].force > 60 && toto[i].action >= AC_BOIT_E &&
    toto[i].action <= AC_BOIT_S)
  {
    NewAction (
      i, AC_EXPLOSE_E + toto[i].action - AC_BOIT_E,
      0); /* toto à trop mangé ! */
    toto[i].force = 0;

    ObjetPut (toto[i].poscel, OB_TROPBU); /* faudra ouvrir le sol */
    toto[i].rstdecorcel = toto[i].poscel;
    toto[i].rstdecoricon =
      DecorGetCel (toto[i].poscel); /* faudra remettre le sol initial */
    return;
  }

  if (
    toto[i].force > 0 && toto[i].mechant == 0 && toto[i].tank == 0 &&
    toto[i].action > AC_STOP_S &&
    (toto[i].action < AC_REPOS_E || toto[i].action > AC_REPOS_S) &&
    (toto[i].action < AC_DORT_E || toto[i].action > AC_DORT_S) &&
    (toto[i].action < AC_REFLEXION_E || toto[i].action > AC_REFLEXION_S) &&
    (toto[i].action < AC_YOYO_E || toto[i].action > AC_YOYO_S) &&
    (toto[i].action < AC_HAUSSE_E || toto[i].action > AC_HAUSSE_S))
  {
    if ((toto[i].joueur == 0 || (toto[i].sequence++) % 2 == 0) && !g_passpower)
    {
      toto[i].force--; /* la force diminue un peu */
    }
  }

  toto[i].vitesse /= 2; /* diminue la vitesse */

  if (
    toto[i].magic > 0 && toto[i].action > AC_STOP_S &&
    (toto[i].action < AC_REPOS_E || toto[i].action > AC_REPOS_S) &&
    (toto[i].action < AC_DORT_E || toto[i].action > AC_DORT_S) &&
    (toto[i].action < AC_REFLEXION_E || toto[i].action > AC_REFLEXION_S) &&
    (toto[i].action < AC_YOYO_E || toto[i].action > AC_YOYO_S) &&
    (toto[i].action < AC_HAUSSE_E || toto[i].action > AC_HAUSSE_S))
  {
    toto[i].magic--; /* les pouvoirs magiques diminuent un peu */
  }

  if (toto[i].cntnotuse > 0)
  {
    toto[i].cntnotuse--;
    if (toto[i].cntnotuse == 0)
    {
      toto[i].lastobjet.x = -1;
      toto[i].lastobjet.y = -1; /* on peut de nouveau utiliser l'objet */
    }
  }

  if (toto[i].joueur)
  {
    testcel   = toto[i].poscel;
    testcel.x = 2 * toto[i].poscela.x - toto[i].poscel.x;
    testcel.y = 2 * toto[i].poscela.y - toto[i].poscel.y;
    obstacle  = GetObstacle (testcel, 0);
    SoundAmbiance (obstacle, &toto[i].poscel); /* év. son d'ambiance */
  }

  if (
    (toto[i].poscela.x != toto[i].poscel.x ||
     toto[i].poscela.y != toto[i].poscel.y) &&
    toto[i].icon != ICO_CAISSE && toto[i].icon != ICO_CAISSEV &&
    toto[i].icon != ICO_CAISSEO && toto[i].icon != ICO_CAISSEG &&
    DecorGetCel (toto[i].poscel) == ICO_UNSEUL)
  {
    ObjetPut (toto[i].poscel, OB_UNSEUL); /* ouvre la trappe */
  }

  if (
    toto[i].poscela.x != toto[i].poscel.x ||
    toto[i].poscela.y != toto[i].poscel.y)
  {
    toto[i].lastpos = toto[i].poscel;
    gendecor++;
  }

  toto[i].poscel = toto[i].poscela;

  testcel     = toto[i].poscel;
  nextaction  = toto[i].action;
  orientation = GetOrientation (toto[i].action);

  if (toto[i].action >= AC_MAGIC_E && toto[i].action <= AC_MAGIC_S)
  {
    toto[i].magic += 30;
  }

  if (toto[i].action >= AC_ARRIVEE_E && toto[i].action <= AC_ARRIVEE_S)
  {
    NewAction (i, AC_ARRIVEE_M, 0);          /* toto monte avec son ballon */
    MoveInit (testcel, AC_BALLON_M, -LYICO); /* départ du ballon */
    DecorModif (testcel, ICO_ARRIVEEVIDE);   /* détache le ballon */
    return;
  }

  if (toto[i].action == AC_ARRIVEE_M)
  {
    toto[i].status = STVIDE; /* ce toto n'existe plus */
    nbtoto--;
    nbin--; /* y'a un toto kè rentré */
    return;
  }

  if (
    (toto[i].action >= AC_TOMBE_E && toto[i].action <= AC_TOMBE_TANKB_S) ||
    (toto[i].action >= AC_EXPLOSE_E && toto[i].action <= AC_EXPLOSE_S) ||
    toto[i].action == AC_TANK)
  {
    toto[i].status = STVIDE; /* ce toto n'existe plus */
    nbtoto--;
    if (toto[i].mechant == 0 && toto[i].tank == 0)
      nbout++; /* il pourra ressortir */
    if (toto[i].joueur)
    {
      toto[i].joueur = 0;
      perdu          = 1;
    }
    if (toto[i].tank == 3)
    {
      DecorModif (
        toto[i].poscel, ICO_TANKBAS); /* met un sol bouché par le tank */
      DecorPutInitCel (
        toto[i].poscel, ICO_TANKBAS); /* modification permanante */
    }
    return;
  }

  if (
    (toto[i].action >= AC_GLISSE_E && toto[i].action <= AC_GLISSE_S) ||
    toto[i].action == AC_ELECTRO_O)
  {
    if (toto[i].joueur)
    {
      toto[i].joueur = 0;
      perdu          = 1;
    }
  }

  if (toto[i].action == AC_BALLON_E || toto[i].action == AC_BALLON_M)
  {
    toto[i].status = STVIDE; /* ce toto n'existe plus */
    nbtoto--;
    return;
  }

  if (toto[i].action >= AC_CAISSE_E && toto[i].action <= AC_CAISSE_S)
  {
    if (DecorGetCel (toto[i].poscel) == ICO_TROUBOUCHE)
    {
      NewAction (i, AC_CAISSE_T, 0); /* la caisse tombe dans le trou */
    }
    else
    {
      DecorModif (toto[i].poscel, ICO_CAISSE); /* remet la caisse fixe */
      toto[i].status = STVIDE;                 /* ce toto n'existe plus */
      nbtoto--;
    }
    return;
  }

  if (toto[i].action >= AC_CAISSEV_E && toto[i].action <= AC_CAISSEV_S)
  {
    if (DecorGetCel (toto[i].poscel) == ICO_TROUBOUCHE)
    {
      NewAction (i, AC_CAISSEV_T, 0); /* la caisse tombe dans le trou */
    }
    else
    {
      DecorModif (toto[i].poscel, ICO_CAISSEV); /* remet la caisse fixe */
      toto[i].status = STVIDE;                  /* ce toto n'existe plus */
      nbtoto--;
    }
    return;
  }

  if (toto[i].action >= AC_CAISSEO_E && toto[i].action <= AC_CAISSEOD_S)
  {
    if (DecorGetCel (toto[i].poscel) == ICO_TROUBOUCHE)
    {
      NewAction (i, AC_CAISSEO_T, 0); /* la caisse tombe dans le trou */
    }
    else
    {
      if (toto[i].energie == 2) /* énergie d'une boule poussée ? */
      {
        caisseodir = toto[i].action;
        caisseocel = toto[i].poscel;
      }
      DecorModif (toto[i].poscel, ICO_CAISSEO); /* remet la caisse fixe */
      toto[i].status = STVIDE;                  /* ce toto n'existe plus */
      nbtoto--;
    }
    return;
  }

  if (toto[i].action >= AC_CAISSEG_E && toto[i].action <= AC_CAISSEG_S)
  {
    if (DecorGetCel (toto[i].poscel) == ICO_TROUBOUCHE)
    {
      NewAction (i, AC_CAISSEG_T, 0); /* la caisse tombe dans le trou */
    }
    else
    {
      if (toto[i].energie == 2) /* énergie d'une boule poussée ? */
      {
        if (orientation == AC_MARCHE_E)
          testcel.x++;
        if (orientation == AC_MARCHE_O)
          testcel.x--;
        if (orientation == AC_MARCHE_S)
          testcel.y++;
        if (orientation == AC_MARCHE_N)
          testcel.y--;
        if (IfPousseCaisse (testcel, ICO_CAISSEG, orientation))
        {
          NewAction (i, orientation + AC_CAISSEG_E - AC_MARCHE_E, 0);
          return; /* la machine continue */
        }
      }
      PlaySound (SOUND_CAISSEV, &toto[i].poscel);
      DecorModif (toto[i].poscel, ICO_CAISSEG); /* remet la caisse fixe */
      toto[i].status = STVIDE;                  /* ce toto n'existe plus */
      nbtoto--;
    }
    return;
  }

  if (toto[i].action == AC_CAISSE_T)
  {
    DecorModif (
      toto[i].poscel, ICO_CAISSEBAS); /* met un sol bouché par la caisse */
    DecorPutInitCel (
      toto[i].poscel, ICO_CAISSEBAS); /* modification permanante */
    toto[i].status = STVIDE;          /* ce toto n'existe plus */
    nbtoto--;
    return;
  }

  if (toto[i].action == AC_CAISSEV_T)
  {
    DecorModif (
      toto[i].poscel, ICO_CAISSEVBAS); /* met un sol bouché par la caisse */
    DecorPutInitCel (
      toto[i].poscel, ICO_CAISSEVBAS); /* modification permanante */
    toto[i].status = STVIDE;           /* ce toto n'existe plus */
    nbtoto--;
    return;
  }

  if (toto[i].action == AC_CAISSEO_T)
  {
    DecorModif (
      toto[i].poscel, ICO_CAISSEOBAS); /* met un sol bouché par la caisse */
    DecorPutInitCel (
      toto[i].poscel, ICO_CAISSEOBAS); /* modification permanante */
    toto[i].status = STVIDE;           /* ce toto n'existe plus */
    nbtoto--;
    return;
  }

  if (toto[i].action == AC_CAISSEG_T)
  {
    DecorModif (
      toto[i].poscel, ICO_CAISSEGBAS); /* met un sol bouché par la caisse */
    DecorPutInitCel (
      toto[i].poscel, ICO_CAISSEGBAS); /* modification permanante */
    toto[i].status = STVIDE;           /* ce toto n'existe plus */
    nbtoto--;
    return;
  }

  if (
    toto[i].action >= AC_SAUTE2_E && toto[i].action <= AC_SAUTE2_S &&
    DecorGetCel (toto[i].poscel) == ICO_AIMANT)
  {
    DecorModif (
      testcel, DecorGetInitCel (testcel)); /* enlève l'aimant du décor */
    PlaySound (SOUND_AIMANT, &toto[i].poscel);
  }

  switch (GetOrientation (toto[i].action))
  {
  case AC_MARCHE_E:
    nextaction = AC_MARCHE_E;
    testcel.x++;
    break;
  case AC_MARCHE_O:
    nextaction = AC_MARCHE_O;
    testcel.x--;
    break;
  case AC_MARCHE_S:
    nextaction = AC_MARCHE_S;
    testcel.y++;
    break;
  case AC_MARCHE_N:
    nextaction = AC_MARCHE_N;
    testcel.y--;
    break;

  default:
    break;
  }

  if (toto[i].joueur) /* est-ce le toto du joueur ? */
  {
    JoueurAction (i, event, nextaction, testcel);
    return;
  }

  VisionAction (i, &nextaction); /* regarde si toto peut faire mieux */

  if (toto[i].nextrepos > 0)
    toto[i].nextrepos--;

  if (
    nextaction >= AC_MARCHE_E && nextaction <= AC_MARCHE_S &&
    toto[i].nextrepos == 0 && toto[i].force < 5 && toto[i].vision > 0 &&
    toto[i].tank == 0 && GetRandom (0, 0, 20) == 0)
  {
    NewAction (i, nextaction + AC_DORT_E - AC_MARCHE_E, 0);
    toto[i].nextrepos = 8;
    return;
  }

  if (
    nextaction >= AC_MARCHE_E && nextaction <= AC_MARCHE_S &&
    toto[i].nextrepos == 0 && toto[i].force < 10 && toto[i].vision > 0 &&
    toto[i].tank == 0 && GetRandom (0, 0, 15) == 0)
  {
    NewAction (i, nextaction + AC_REPOS_E - AC_MARCHE_E, 0);
    toto[i].nextrepos = 5;
    return;
  }

  if (
    nextaction >= AC_MARCHE_E && nextaction <= AC_MARCHE_S &&
    toto[i].force > 40 && toto[i].vision > 0 && toto[i].tank == 0 &&
    GetRandom (0, 0, 10) == 0)
  {
    NewAction (i, nextaction + AC_YOUPIE_E - AC_MARCHE_E, 0);
    return;
  }

  if (nextaction < AC_MARCHE_E || nextaction > AC_MARCHE_S) /* action autre que
                                                               de marcher ? */
  {
    NewAction (i, nextaction, 0); /* oui -> démarre la nouvelle action */
    return;
  }

  obstacle = GetObstacle (testcel, 1);
  if (obstacle == 0) /* mouvement projeté possible ? */
  {
    NewAction (
      i, nextaction, 0); /* si c'est possible -> démarre la nouvelle action */
  }
  else
  {
    if (!SpecAction (i, obstacle, testcel)) /* effectue une action spéciale */
    {
      if (toto[i].mechant == 0 && toto[i].tank == 0)
      {
        SoundAmbiance (obstacle, &toto[i].poscel); /* év. son d'ambiance */
      }
      TourneAction (i); /* tourne si y'a rien d'autre à faire */
    }
  }
}

/* ======== */
/* MoveBack */
/* ======== */

/*
    Fait faire un pas en arrière au toto placé sur une cellule donnée.
 */

void
MoveBack (Pt cel)
{
  short i;

  for (i = 0; i < MAXTOTO; i++)
  {
    if (toto[i].status == STVIDE)
      continue;
    if (
      (toto[i].poscela.x == cel.x && toto[i].poscela.y == cel.y) ||
      (toto[i].poscelb.x == cel.x && toto[i].poscelb.y == cel.y))
    {
      if (back[i].status != STVIDE)
      {
        toto[i] = back[i]; /* reprend l'état précédent */
      }
    }
  }
}

/* -------- */
/* TrieToto */
/* -------- */

/*
    Génère une table triée contenant les indices dans toto[] avec
    en premier les totos placés les plus au fond, et en dernier ceux
    qui sont tout devant. Ceci est nécessaire, pour dessiner avec
    IconDrawPut à partir du fond jusque devant.
    Trie avec un "bubble-sort" pas fameux du tout, mais cela ne gène
    pas trop, vu le petit nombre de toto à trier (<10) !
 */

void
TrieToto (char * ptable)
{
  short i, ii, j;
  short v, vv;
  short job;

  for (i = 0; i < MAXTOTO; i++)
  {
    ptable[i] = i; /* génère une table par défaut */
  }

  do
  {
    job = 0;
    for (j = 0; j < MAXTOTO - 1; j++)
    {
      i  = ptable[j];
      ii = ptable[j + 1];

      v =
        toto[i].posgra.x +
        ((int) toto[i].posgra.y * (int) (PLXICO + PRXICO)) / (PRYICO - PLYICO);
      vv =
        toto[ii].posgra.x +
        ((int) toto[ii].posgra.y * (int) (PLXICO + PRXICO)) / (PRYICO - PLYICO);

      if (v > vv)
      {
        i             = ptable[j];
        ptable[j]     = ptable[j + 1];
        ptable[j + 1] = i;

        job++; /* on a fait qq chose */
      }
    }
  } while (job != 0); /* répète tant que qq a été fait */
}

/* ---------- */
/* DetectToto */
/* ---------- */

/*
    Détecte le toto visé par la souris.
    Retourne -1 si y'en a aucun !
 */

short
DetectToto (Pt pmouse, Pt ovisu, char ordre[])
{
  short i, j;
  Pt    pos;

  pmouse.x -= POSXDRAW;
  pmouse.y -= POSYDRAW;
  if (
    pmouse.x >= 0 && pmouse.x <= DIMXDRAW && pmouse.y >= 0 &&
    pmouse.y <= DIMYDRAW)
  {
    for (j = MAXTOTO - 1; j >= 0; j--) /* du plus en avant au plus en arrière */
    {
      i = ordre[j];

      if (
        toto[i].status == STVIDE || toto[i].tank != 0 ||
        toto[i].action == AC_BALLON_E || toto[i].action == AC_BALLON_M ||
        (toto[i].action >= AC_CAISSE_E && toto[i].action <= AC_CAISSE_S) ||
        (toto[i].action >= AC_CAISSEV_E && toto[i].action <= AC_CAISSEV_S) ||
        (toto[i].action >= AC_CAISSEG_E && toto[i].action <= AC_CAISSEG_S) ||
        (toto[i].action >= AC_CAISSEO_E && toto[i].action <= AC_CAISSEOD_S))
        continue;

      pos.x = toto[i].posgra.x + PLXICO * ovisu.x;
      pos.y = toto[i].posgra.y + PRYICO * ovisu.y + toto[i].offz;

      if (
        pmouse.x >= pos.x + 15 && pmouse.x <= pos.x + LXICO - 15 &&
        pmouse.y >= pos.y + 15 && pmouse.y <= pos.y + LYICO - 5)
      {
        lastdetect = i;
        return i; /* retourne le toto trouvé */
      }
    }
  }

  if (lastdetect != -1)
  {
    if (
      toto[lastdetect].status == STVIDE || toto[lastdetect].tank != 0 ||
      toto[lastdetect].action == AC_BALLON_E ||
      toto[lastdetect].action == AC_BALLON_M ||
      (toto[lastdetect].action >= AC_CAISSE_E &&
       toto[lastdetect].action <= AC_CAISSE_S) ||
      (toto[lastdetect].action >= AC_CAISSEV_E &&
       toto[lastdetect].action <= AC_CAISSEV_S) ||
      (toto[lastdetect].action >= AC_CAISSEG_E &&
       toto[lastdetect].action <= AC_CAISSEG_S) ||
      (toto[lastdetect].action >= AC_CAISSEO_E &&
       toto[lastdetect].action <= AC_CAISSEOD_S))
    {
      lastdetect = -1;
    }
  }
  if (lastdetect != -1)
    return lastdetect;

  for (i = 0; i < MAXTOTO; i++)
  {
    if (toto[i].status != STVIDE && toto[i].tank == 0)
    {
      lastdetect = i;
      return i; /* retourne le premier toto trouvé dans les tables */
    }
  }

  return -1; /* rien trouvé */
}

/* ----------- */
/* IfInterrupt */
/* ----------- */

/*
    Retourne 1 (true) si l'action en cours d'un toto
    est interruptible, suite à un événement prioritaire.
 */

short
IfInterrupt (short i, short event)
{
  if (toto[i].joueur == 0)
    return 0;

  if (
    event != KEYGOFRONT && event != KEYGOBACK && event != KEYGOLEFT &&
    event != KEYGORIGHT && event != KEYCLIC && event != KEYCLICR &&
    GetKeyStatus () == 0)
    return 0;

  if (
    (toto[i].action >= AC_STOP_E && toto[i].action <= AC_STOP_S) ||
    (toto[i].action >= AC_REPOS_E && toto[i].action <= AC_REPOS_S) ||
    (toto[i].action >= AC_REFLEXION_E && toto[i].action <= AC_REFLEXION_S) ||
    (toto[i].action >= AC_HAUSSE_E && toto[i].action <= AC_HAUSSE_S) ||
    (toto[i].action >= AC_YOYO_E && toto[i].action <= AC_YOYO_S) ||
    (toto[i].action >= AC_DORT_E && toto[i].action <= AC_DORT_S))
    return 1;

  return 0;
}

/* ------- */
/* DrawOne */
/* ------- */

/*
    Dessine un toto animé.
 */

void
DrawOne (short i, Pt ovisu)
{
  short btransp;
  Reg   r;
  Pt    pos;
  Pt    celdd; /* cellule charnière pour devant/derrière */

  pos = toto[i].posgra;
  pos.x += PLXICO * ovisu.x;
  pos.y += PRYICO * ovisu.y + toto[i].offz;
  if (
    toto[i].icon == ICO_CAISSE || toto[i].icon == ICO_CAISSEV ||
    toto[i].icon == ICO_CAISSEO || toto[i].icon == ICO_CAISSEG ||
    (toto[i].icon >= ICO_TANK_E && toto[i].icon <= ICO_TANK_S) ||
    toto[i].icon == ICO_TANK_X || toto[i].icon == ICO_TANK_EO ||
    toto[i].icon == ICO_TANK_NS)
    pos.y += OFFZTOTO;

  celdd.x = (toto[i].poscel.x > toto[i].poscela.x) ? toto[i].poscel.x
                                                   : toto[i].poscela.x;
  celdd.y = (toto[i].poscel.y > toto[i].poscela.y) ? toto[i].poscel.y
                                                   : toto[i].poscela.y;

  btransp = 0;
  if (
    toto[i].magic > 0 || /* toto transparent */
    toto[i].invincible & 1)
    btransp = 1; /* toto invisible un pas sur deux */

  if (toto[i].offz < LYICO + OFFZTOTO) /* si trop bas -> ne dessine pas */
  {
    r.r.p1.y = 0;
    r.r.p1.x = 0;
    r.r.p2.y = DIMYDRAW;
    r.r.p2.x = DIMXDRAW;
    IconDrawPut (toto[i].icon, btransp, pos, toto[i].offz, celdd, r);
  }
}

/* --------- */
/* StartTank */
/* --------- */

/*
    Fait éventuellement partir les tanks stoppés si y'a un toto qui s'pointe.
 */

void
StartTank (void)
{
  Pt     cel;
  short  icon, i, decor;
  Action nextaction;

  if (g_typeedit != 0)
    return; /* pas de départ si on est en train d'éditer */

  if (genstarttank == gendecor)
    return; /* rien à faire si toujours le même décor */

  for (cel.y = 0; cel.y <= MAXCELY; cel.y++)
  {
    for (cel.x = 0; cel.x <= MAXCELX; cel.x++)
    {
      icon = DecorGetCel (cel);

      if (
        icon == ICO_TANK_X || /* est-ce un tank immobile (dans le décor) ? */
        icon == ICO_TANK_EO || icon == ICO_TANK_NS)
      {
        i = MoveInit (cel, AC_STOP_E, 0); /* prépare le tank */
        if (i >= 0)
        {
          toto[i].tank = 4; /* tank immobile */
          if (VisionAction (i, &nextaction) == 0)
          {
            toto[i].status = STVIDE; /* le tank ne part pas */
            nbtoto--;
          }
          else
          {
            decor = DecorGetInitCel (cel);
            DecorModif (cel, decor);      /* met un sol normal */
            DecorPutInitCel (cel, decor); /* modification permanante */
            if (icon == ICO_TANK_X)
              toto[i].tank = 2;
            else
              toto[i].tank = 3;
            toto[i].action = nextaction - AC_MARCHE_E + AC_STOP_E;
          }
        }
      }
    }
  }

  genstarttank = gendecor;
}

/* ======== */
/* MoveNext */
/* ======== */

/*
    Avance les totos animés d'un "pas".
    Retourne 1 si le monde est terminé (tous les toto rentrés).
    Retourne 2 si le monde est terminé (perdu).
 */

short
MoveNext (char event, Pt pmouse)
{
  short i, j;
  short try;
  Pt    ovisu;
  char  ordre[MAXTOTO];
  short result[3];
  Pt    celarr;
  short orientation;

  perdu  = 0;
  redraw = 0;

  if (event == KEYCLICREL)
  {
    lasttelecom = 0; /* stoppe si bouton souris relâché */
  }

  if ((event == KEYCLIC || event == KEYCLICR) && g_typejeu == 1)
  {
    JoueurCap (event, pmouse); /* assigne év. un nouveau cap à atteindre */
  }

  DepartNext (); /* gère les ascenseurs */
  ObjetNext ();  /* gère les objets du décor */

  TrieToto (ordre); /* génère la table ordre[] */

  ovisu = DecorGetOrigine ();

  i =
    DetectToto (pmouse, ovisu, ordre); /* détecte le toto visé par la souris */
  if (i < 0)
    InfoDraw (0, 0, 0, 0, 0, 0);
  else
    InfoDraw (
      1, toto[i].force, toto[i].vision, toto[i].mechant, toto[i].magic,
      toto[i].cles);

  caisseoddir = caisseodir;
  caisseodir  = 0; /* pas de boule qui roule */

  StartTank (); /* fait év. partir les tanks stoppés */

  for (j = 0; j < MAXTOTO; j++)
  {
    i   = ordre[j];
    try = 0;

  next:
    if (toto[i].status == STVIDE)
      continue;

    if (
      CalcMovie (i, result) || /* action terminée ? */
      (try < 1 && IfInterrupt (i, event)))
    {
      NextAction (event, i); /* oui -> affecte une autre action à toto */
      try++;
      goto next;
    }

    toto[i].posgra.x += result[0];
    toto[i].posgra.y += result[1];
    toto[i].offz += result[2];

    if (toto[i].invincible > 0)
      toto[i].invincible--; /* l'invincibilité diminue un peu */

    AutoNext (
      &toto[i].autoicon, result,
      &toto[i].poscel); /* cherche l'icône suivante */
    if (toto[i].mechant == 0 || result[0] < 16 || result[0] > 19)
      toto[i].icon = result[0];

    if (
      toto[i].vision == 0 && ((toto[i].icon >= 1 && toto[i].icon <= 12) ||
                              (toto[i].icon >= 16 && toto[i].icon <= 19)))
      toto[i].icon += 80; /* icône de toto aveugle */

    if (
      toto[i].force < 5 && ((toto[i].icon >= 1 && toto[i].icon <= 12) ||
                            (toto[i].icon >= 16 && toto[i].icon <= 19)))
      toto[i].icon += 32; /* icône de mauvaise humeur */

    if (toto[i].mechant == 1 && (toto[i].icon >= 1 && toto[i].icon <= 12))
      toto[i].icon += 112; /* icône méchant */

    if (
      toto[i].joueur && ((toto[i].icon >= 1 && toto[i].icon <= 12) ||
                         (toto[i].icon >= 16 && toto[i].icon <= 19) ||
                         (toto[i].icon >= 33 && toto[i].icon <= 44)))
      toto[i].icon += 128; /* icône avec antenne */

    if (toto[i].tank != 0)
    {
      orientation = GetOrientation (toto[i].action);
      if (orientation == AC_MARCHE_E)
        toto[i].icon = ICO_TANK_E;
      if (orientation == AC_MARCHE_N)
        toto[i].icon = ICO_TANK_N;
      if (orientation == AC_MARCHE_O)
        toto[i].icon = ICO_TANK_O;
      if (orientation == AC_MARCHE_S)
        toto[i].icon = ICO_TANK_S;
      if (toto[i].tank == 2)
        toto[i].icon = ICO_TANK_X;
      if (toto[i].tank == 3)
      {
        if (orientation == AC_MARCHE_E || orientation == AC_MARCHE_O)
          toto[i].icon = ICO_TANK_EO;
        if (orientation == AC_MARCHE_N || orientation == AC_MARCHE_S)
          toto[i].icon = ICO_TANK_NS;
      }
    }

    DrawOne (i, ovisu); /* dessine le toto */
  }

  /* Regarde s'il y a une boule qui roule qui doit continuer de rouler
      sur son élan ... */

  if (caisseoddir != 0)
  {
    celarr = caisseocel;
    if (caisseoddir == AC_CAISSEO_E)
      celarr.x++;
    if (caisseoddir == AC_CAISSEO_O)
      celarr.x--;
    if (caisseoddir == AC_CAISSEO_S)
      celarr.y++;
    if (caisseoddir == AC_CAISSEO_N)
      celarr.y--;
    if (IfPousseCaisse (
          celarr, ICO_CAISSEO, caisseoddir - AC_CAISSEO_E + AC_MARCHE_E))
    {
      DecorModif (
        caisseocel, DecorGetInitCel (caisseocel)); /* enlève la boule fixe */
      i = MoveInit (caisseocel, caisseoddir + AC_CAISSEOD_E - AC_CAISSEO_E, 0);
      if (i >= 0)
      {
        toto[i].icon    = ICO_CAISSEO;
        toto[i].energie = 1; /* énergie d'une boule sur son élan */
        redraw          = 1; /* faudra tout redessiner */
      }
    }
  }

  if (redraw)
    MoveRedraw (); /* redessine tout */

  if (perdu)
    return 2; /* snif, c'est perdu */
  if (nbin <= 0)
    return 1; /* tous les toto sont rentrés */
  return 0;
}

/* ========== */
/* MoveRedraw */
/* ========== */

/*
    Redessine tous les toto comme lors du dernier ModeNext.
 */

void
MoveRedraw (void)
{
  short i, j;
  Pt    ovisu;
  char  ordre[MAXTOTO];

  TrieToto (ordre); /* génère la table ordre[] */

  ovisu = DecorGetOrigine ();

  for (j = 0; j < MAXTOTO; j++)
  {
    i = ordre[j];
    if (toto[i].status == STVIDE)
      continue;

    DrawOne (i, ovisu); /* dessine le toto */
  }
}

/* ========= */
/* MoveBuild */
/* ========= */

/*
    Construit devant le toto du joueur, selon l'outil actionné.
    Retourne 0 si l'événement a été traité.
 */

short
MoveBuild (short outil, int key)
{
  short i, temp, err;
  short orientation;
  Pt    pos;

  i = GetJoueur ();
  if (i < 0)
    return 1;

  pos = toto[i].poscel;

  orientation = GetOrientation (toto[i].action);
  switch (orientation)
  {
  case AC_MARCHE_E:
    pos.x++;
    break;
  case AC_MARCHE_O:
    pos.x--;
    break;
  case AC_MARCHE_S:
    pos.y++;
    break;
  case AC_MARCHE_N:
    pos.y--;
    break;
  default:
    break;
  }

  temp      = g_typejeu;
  g_typejeu = 0; /* comme si jeu sans toto télécommandé */
  err       = DecorEvent (pos, 1, outil, key); /* modifie le décor */
  g_typejeu = temp;

  if (err)
  {
    NewAction (i, orientation + AC_NON_E - AC_MARCHE_E, 0);
  }

  return err;
}

/* ========== */
/* MoveScroll */
/* ========== */

/*
    Décale le décor si le toto du joueur dépasse de la partie visible.
 */

void
MoveScroll (short quick)
{
  short i, set;
  Pt    ovisu, pos;
  short palette;

  i = GetJoueur ();

  if (g_typeedit == 0 && i < 0 && g_typejeu == 1)
  {
    palette = 0;
    PaletteNew (&palette, 0); /* supprime tous les outils ! */
    g_typejeu = 0;            /* jeu sans toto télécommandé */
  }

  if (
    i < 0 || toto[i].action == AC_ARRIVEE_M || /* toto monte en ballon */
    (toto[i].action >= AC_TOMBE_E &&           /* toto tombe dans un trou */
     toto[i].action <= AC_TOMBE_S))
    return;

  do
  {
    set   = 0;
    ovisu = DecorGetOrigine ();

    pos.x = toto[i].posgra.x + PLXICO * ovisu.x;
    pos.y = toto[i].posgra.y + PRYICO * ovisu.y + toto[i].offz;

    if (pos.x < 20)
    {
      ovisu.x += 4;
      DecorSetOrigine (ovisu, quick);
      set = 1;
    }

    if (pos.x + LXICO > DIMXDRAW - 20)
    {
      ovisu.x -= 4;
      DecorSetOrigine (ovisu, quick);
      set = 1;
    }

    if (pos.y < 0)
    {
      ovisu.y += 5;
      DecorSetOrigine (ovisu, quick);
      set = 1;
    }

    if (pos.y + LYICO > DIMYDRAW - 10)
    {
      ovisu.y -= 5;
      DecorSetOrigine (ovisu, quick);
      set = 1;
    }
  } while (set == 1);
}

/* ============ */
/* MoveNewMonde */
/* ============ */

/*
    Cherche les positions des ascenseurs.
 */

void
MoveNewMonde (short freq)
{
  short i, d;
  Pt    cel;
  short icon;
  short nbjoueurs;
  short nbdepart;

  StartRandom (0, 0); /* aléatoire répétitif pour les toto */
  StartRandom (1, 1); /* aléatoire total pour les décors */

  g_typejeu   = 0;
  nbout       = 0;
  nbjoueurs   = 0;
  nbdepart    = 0;
  lasttelecom = 0;
  celcap1.x   = -1;

  for (i = 0; i < MAXTOTO; i++)
  {
    toto[i].status         = STVIDE;
    toto[i].autoicon.pdata = 0;
    back[i].status         = STVIDE;
    back[i].autoicon.pdata = 0;
  }

  for (d = 0; d < MAXDEPART; d++)
  {
    depart[d].cel.x = -1;
    depart[d].cel.y = -1;
  }

  for (i = 0; i < MAXOBJET; i++)
  {
    objet[i].status = STVIDE;
  }

  d = 0;
  for (cel.y = 0; cel.y <= MAXCELY; cel.y++)
  {
    for (cel.x = 0; cel.x <= MAXCELX; cel.x++)
    {
      icon = DecorGetCel (cel);
      if (
        icon == ICO_DEPART && /* est-ce un ascenseur ? */
        d < MAXDEPART)
      {
        nbdepart++;
        depart[d].cel   = cel;
        depart[d].freq  = freq;
        depart[d].count = 1;
        d++;
      }
      if (icon == ICO_ARRIVEE) /* est-ce une arrivée (ballon) ? */
      {
        nbout++;
      }
      if (
        icon == ICO_JOUEUR && /* est-ce le toto du joueur ? */
        g_typeedit == 0)      /* n'est-on pas en train d'éditer un monde ? */
      {
        g_typejeu = 1;
        DecorPutCel (cel, DecorGetInitCel (cel)); /* remet un sol normal */
        if (nbjoueurs == 0)
        {
          i = MoveInit (cel, AC_STOP_E, 0); /* prépare le toto du joueur */
          if (i >= 0)
          {
            toto[i].joueur = 1; /* c'est le toto du joueur */
            toto[i].vision = 1;
            nbout--;
          }
        }
        nbjoueurs++;
      }
      if (
        icon >= ICO_TANK_E && /* est-ce un tank ? */
        icon <= ICO_TANK_S &&
        g_typeedit == 0) /* n'est-on pas en train d'éditer un monde ? */
      {
        i = MoveInit (
          cel, icon - ICO_TANK_E + AC_STOP_E, 0); /* prépare le tank */
        if (i >= 0)
        {
          DecorPutCel (cel, DecorGetInitCel (cel)); /* remet un sol normal */
          toto[i].tank = 1;                         /* c'est un tank */
        }
      }
    }
  }

  if (nbout < 0)
    nbout = 0;

  nbin = nbout;
  if (g_typejeu == 1)
    nbin++; /* rentre aussi le toto du joueur */

  if (g_typeedit || nbout == 0) /* édition d'un monde en cours ? */
  {
    nbout = 0; /* oui -> aucun toto */
    nbin  = 1;
  }

  nbtoto = 0;

  gendecor     = 0;
  genstarttank = gendecor - 1;
}

/* ======== */
/* MoveOpen */
/* ======== */

/*
    Ouverture générale.
 */

short
MoveOpen (void)
{
  lastdetect = -1;
  return 0;
}

/* ========= */
/* MoveClose */
/* ========= */

/*
    Fermeture générale.
 */

void
MoveClose (void)
{
}

/* ============ */
/* MovePartieLg */
/* ============ */

/*
    Retourne la longueur nécessaire pour sauver les variables de la partie en
   cours.
 */

int
MovePartieLg (void)
{
  return sizeof (TotoMove) * MAXTOTO + sizeof (TotoMove) * MAXTOTO +
         sizeof (DepartMove) * MAXDEPART + sizeof (ObjetMove) * MAXOBJET +
         sizeof (Partie);
}

/* =============== */
/* MovePartieWrite */
/* =============== */

/*
    Sauve les variables de la partie en cours.
 */

short
MovePartieWrite (int pos, char file)
{
  short  err;
  Partie partie;

  err = FileWrite (&toto, pos, sizeof (TotoMove) * MAXTOTO, file);
  if (err)
    return err;
  pos += sizeof (TotoMove) * MAXTOTO;

  err = FileWrite (&back, pos, sizeof (TotoMove) * MAXTOTO, file);
  if (err)
    return err;
  pos += sizeof (TotoMove) * MAXTOTO;

  err = FileWrite (&depart, pos, sizeof (DepartMove) * MAXDEPART, file);
  if (err)
    return err;
  pos += sizeof (DepartMove) * MAXDEPART;

  err = FileWrite (&objet, pos, sizeof (ObjetMove) * MAXOBJET, file);
  if (err)
    return err;
  pos += sizeof (ObjetMove) * MAXOBJET;

  memset (&partie, 0, sizeof (Partie));
  partie.nbtoto      = nbtoto;
  partie.nbout       = nbout;
  partie.nbin        = nbin;
  partie.lastdetect  = lastdetect;
  partie.celcap1     = celcap1;
  partie.celcap2     = celcap2;
  partie.caisseodir  = caisseodir;
  partie.caisseoddir = caisseoddir;
  partie.caisseocel  = caisseocel;

  err = FileWrite (&partie, pos, sizeof (Partie), file);
  return err;
}

/* ============== */
/* MovePartieRead */
/* ============== */

/*
    Lit les variables de la partie en cours.
    Les pointeurs contenus dans les automates (pdata) doivent être
    régénérés en fonction de l'action (idata) !
 */

short
MovePartieRead (int pos, char file)
{
  short  err, i;
  Partie partie;

  err = FileRead (&toto, pos, sizeof (TotoMove) * MAXTOTO, file);
  if (err)
    return err;
  pos += sizeof (TotoMove) * MAXTOTO;

  for (i = 0; i < MAXTOTO; i++)
  {
    toto[i].autoicon.pdata =
      ConvActionToTabIcon (toto[i].autoicon.idata, GetTypeMarche (i));
    toto[i].automove.pdata = ConvActionToTabMove (toto[i].automove.idata);
  }

  err = FileRead (&back, pos, sizeof (TotoMove) * MAXTOTO, file);
  if (err)
    return err;
  pos += sizeof (TotoMove) * MAXTOTO;

  for (i = 0; i < MAXTOTO; i++)
  {
    back[i].autoicon.pdata =
      ConvActionToTabIcon (back[i].autoicon.idata, GetTypeMarche (i));
    back[i].automove.pdata = ConvActionToTabMove (back[i].automove.idata);
  }

  err = FileRead (&depart, pos, sizeof (DepartMove) * MAXDEPART, file);
  if (err)
    return err;
  pos += sizeof (DepartMove) * MAXDEPART;

  err = FileRead (&objet, pos, sizeof (ObjetMove) * MAXOBJET, file);
  if (err)
    return err;
  pos += sizeof (ObjetMove) * MAXOBJET;

  for (i = 0; i < MAXOBJET; i++)
  {
    objet[i].autoicon.pdata = ConvObjetToTabIcon (objet[i].autoicon.idata);
  }

  err = FileRead (&partie, pos, sizeof (Partie), file);
  if (err)
    return err;

  nbtoto      = partie.nbtoto;
  nbout       = partie.nbout;
  nbin        = partie.nbin;
  lastdetect  = partie.lastdetect;
  celcap1     = partie.celcap1;
  celcap2     = partie.celcap2;
  caisseodir  = partie.caisseodir;
  caisseoddir = partie.caisseoddir;
  caisseocel  = partie.caisseocel;

  genstarttank = gendecor - 1;

  return 0;
}


/* ========= */
/* actions.h */
/* ========= */

/* Définitions pour les automates */
/* ------------------------------ */

#pragma once

#define OPTERM 0   /* fin */
#define OPLIST 1   /* OPLIST i, D(i)..D(i-1) */
#define OPREPEAT 2 /* OPREPEAT, n, i, D(i)..D(i-1) */
#define OPSOUND 3  /* OPSOUND s */

/* Actions pour les objets mobiles (toto) */
/* -------------------------------------- */

typedef enum {
  AC_STOP_E,        /* stoppé vers l'est */
  AC_STOP_N,        /* stoppé vers le nord */
  AC_STOP_O,        /* stoppé vers l'ouest */
  AC_STOP_S,        /* stoppé vers le sud */
  AC_MARCHE_E,      /* marche vers l'est */
  AC_MARCHE_N,      /* marche vers le nord */
  AC_MARCHE_O,      /* marche vers l'ouest */
  AC_MARCHE_S,      /* marche vers le sud */
  AC_RECULE_E,      /* recule vers l'est */
  AC_RECULE_N,      /* recule vers le nord */
  AC_RECULE_O,      /* recule vers l'ouest */
  AC_RECULE_S,      /* recule vers le sud */
  AC_TOURNE_NE,     /* tourne du nord à l'est */
  AC_TOURNE_ON,     /* tourne de l'ouest au nord */
  AC_TOURNE_SO,     /* tourne du sud à l'ouest */
  AC_TOURNE_ES,     /* tourne de l'est au sud */
  AC_TOURNE_SE,     /* tourne du sud à l'est */
  AC_TOURNE_EN,     /* tourne de l'est au nord */
  AC_TOURNE_NO,     /* tourne du nord à l'ouest */
  AC_TOURNE_OS,     /* tourne de l'ouest au sud */
  AC_SAUTE1_E,      /* saute vers l'est */
  AC_SAUTE1_N,      /* saute vers le nord */
  AC_SAUTE1_O,      /* saute vers l'ouest */
  AC_SAUTE1_S,      /* saute vers le sud */
  AC_SAUTE2_E,      /* saute vers l'est */
  AC_SAUTE2_N,      /* saute vers le nord */
  AC_SAUTE2_O,      /* saute vers l'ouest */
  AC_SAUTE2_S,      /* saute vers le sud */
  AC_SAUTEDET_E,    /* saute sur détonateur vers l'est */
  AC_SAUTEDET_N,    /* saute sur détonateur vers le nord */
  AC_SAUTEDET_O,    /* saute sur détonateur vers l'ouest */
  AC_SAUTEDET_S,    /* saute sur détonateur vers le sud */
  AC_TOMBE_E,       /* tombe vers l'est */
  AC_TOMBE_N,       /* tombe vers le nord */
  AC_TOMBE_O,       /* tombe vers l'ouest */
  AC_TOMBE_S,       /* tombe vers le sud */
  AC_TOMBE_TANK_E,  /* tank tombe dans trou */
  AC_TOMBE_TANK_N,  /* tank tombe dans trou */
  AC_TOMBE_TANK_O,  /* tank tombe dans trou */
  AC_TOMBE_TANK_S,  /* tank tombe dans trou */
  AC_TOMBE_TANKB_E, /* tank tombe dans trou */
  AC_TOMBE_TANKB_N, /* tank tombe dans trou */
  AC_TOMBE_TANKB_O, /* tank tombe dans trou */
  AC_TOMBE_TANKB_S, /* tank tombe dans trou */
  AC_BOIT_E,        /* boit vers l'est */
  AC_BOIT_N,        /* boit vers le nord */
  AC_BOIT_O,        /* boit vers l'ouest */
  AC_BOIT_S,        /* boit vers le sud */
  AC_BOITX_E,       /* boit mauvais vers l'est */
  AC_BOITX_N,       /* boit mauvais vers le nord */
  AC_BOITX_O,       /* boit mauvais vers l'ouest */
  AC_BOITX_S,       /* boit mauvais vers le sud */
  AC_TOURTE_E,      /* tourte vers l'est */
  AC_TOURTE_N,      /* tourte vers le nord */
  AC_TOURTE_O,      /* tourte vers l'ouest */
  AC_TOURTE_S,      /* tourte vers le sud */
  AC_REPOS_E,       /* repos vers l'est */
  AC_REPOS_N,       /* repos vers le nord */
  AC_REPOS_O,       /* repos vers l'ouest */
  AC_REPOS_S,       /* repos vers le sud */
  AC_DORT_E,        /* dort vers l'est */
  AC_DORT_N,        /* dort vers le nord */
  AC_DORT_O,        /* dort vers l'ouest */
  AC_DORT_S,        /* dort vers le sud */
  AC_YOUPIE_E,      /* youpie vers l'est */
  AC_YOUPIE_N,      /* youpie vers le nord */
  AC_YOUPIE_O,      /* youpie vers l'ouest */
  AC_YOUPIE_S,      /* youpie vers le sud */
  AC_NON_E,         /* non vers l'est */
  AC_NON_N,         /* non vers le nord */
  AC_NON_O,         /* non vers l'ouest */
  AC_NON_S,         /* non vers le sud */
  AC_POUSSE_E,      /* pousse vers l'est */
  AC_POUSSE_N,      /* pousse vers le nord */
  AC_POUSSE_O,      /* pousse vers l'ouest */
  AC_POUSSE_S,      /* pousse vers le sud */
  AC_NPOUSSE_E,     /* pousse pas vers l'est */
  AC_NPOUSSE_N,     /* pousse pas vers le nord */
  AC_NPOUSSE_O,     /* pousse pas vers l'ouest */
  AC_NPOUSSE_S,     /* pousse pas vers le sud */
  AC_CAISSE_E,      /* caisse vers l'est */
  AC_CAISSE_N,      /* caisse vers le nord */
  AC_CAISSE_O,      /* caisse vers l'ouest */
  AC_CAISSE_S,      /* caisse vers le sud */
  AC_CAISSEV_E,     /* caisse vers l'est */
  AC_CAISSEV_N,     /* caisse vers le nord */
  AC_CAISSEV_O,     /* caisse vers l'ouest */
  AC_CAISSEV_S,     /* caisse vers le sud */
  AC_CAISSEO_E,     /* caisse vers l'est */
  AC_CAISSEO_N,     /* caisse vers le nord */
  AC_CAISSEO_O,     /* caisse vers l'ouest */
  AC_CAISSEO_S,     /* caisse vers le sud */
  AC_CAISSEOD_E,    /* caisse décélérée vers l'est */
  AC_CAISSEOD_N,    /* caisse décélérée vers le nord */
  AC_CAISSEOD_O,    /* caisse décélérée vers l'ouest */
  AC_CAISSEOD_S,    /* caisse décélérée vers le sud */
  AC_CAISSEG_E,     /* caisse vers l'est */
  AC_CAISSEG_N,     /* caisse vers le nord */
  AC_CAISSEG_O,     /* caisse vers l'ouest */
  AC_CAISSEG_S,     /* caisse vers le sud */
  AC_CAISSE_T,      /* caisse tombe dans trou */
  AC_CAISSEV_T,     /* caisse tombe dans trou */
  AC_CAISSEO_T,     /* caisse tombe dans trou */
  AC_CAISSEG_T,     /* caisse tombe dans trou */
  AC_VISION_E,      /* vision vers l'est */
  AC_VISION_N,      /* vision vers le nord */
  AC_VISION_O,      /* vision vers l'ouest */
  AC_VISION_S,      /* vision vers le sud */
  AC_DEPART_E,      /* départ vers l'est */
  AC_BALLON_E,      /* ballon vers l'est */
  AC_ARRIVEE_E,     /* arrivée vers l'est */
  AC_ARRIVEE_N,     /* arrivée vers le nord */
  AC_ARRIVEE_O,     /* arrivée vers l'ouest */
  AC_ARRIVEE_S,     /* arrivée vers le sud */
  AC_ARRIVEE_M,     /* arrivée avec le ballon */
  AC_BALLON_M,      /* arrivée ballon monte */
  AC_PIANO_O,       /* joue du piano */
  AC_MAGIC_E,       /* magique vers l'est */
  AC_MAGIC_N,       /* magique vers le nord */
  AC_MAGIC_O,       /* magique vers l'ouest */
  AC_MAGIC_S,       /* magique vers le sud */
  AC_ELECTRO_O,     /* électrocuté vers l'ouest */
  AC_TELE_N,        /* télévision vers le nord */
  AC_MECHANT_E,     /* méchant vers l'est */
  AC_MECHANT_N,     /* méchant vers le nord */
  AC_MECHANT_O,     /* méchant vers l'ouest */
  AC_MECHANT_S,     /* méchant vers le sud */
  AC_GLISSE_E,      /* glisse vers l'est */
  AC_GLISSE_N,      /* glisse vers le nord */
  AC_GLISSE_O,      /* glisse vers l'ouest */
  AC_GLISSE_S,      /* glisse vers le sud */
  AC_LIVRE_E,       /* livre vers l'est */
  AC_LIVRE_N,       /* livre vers le nord */
  AC_LIVRE_O,       /* livre vers l'ouest */
  AC_LIVRE_S,       /* livre vers le sud */
  AC_MUSIQUE_E,     /* musique vers l'est */
  AC_MUSIQUE_N,     /* musique vers le nord */
  AC_MUSIQUE_O,     /* musique vers l'ouest */
  AC_MUSIQUE_S,     /* musique vers le sud */
  AC_CLE_E,         /* clé vers l'est */
  AC_CLE_N,         /* clé vers le nord */
  AC_CLE_O,         /* clé vers l'ouest */
  AC_CLE_S,         /* clé vers le sud */
  AC_PORTE_E,       /* porte vers l'est */
  AC_PORTE_N,       /* porte vers le nord */
  AC_PORTE_O,       /* porte vers l'ouest */
  AC_PORTE_S,       /* porte vers le sud */
  AC_BAISSE_O,      /* baisse vers l'ouest */
  AC_EXPLOSE_E,     /* trop mangé vers l'est */
  AC_EXPLOSE_N,     /* trop mangé vers le nord */
  AC_EXPLOSE_O,     /* trop mangé vers l'ouest */
  AC_EXPLOSE_S,     /* trop mangé vers le sud */
  AC_START_E,       /* départ vers l'est */
  AC_START_N,       /* départ vers le nord */
  AC_START_O,       /* départ vers l'ouest */
  AC_START_S,       /* départ vers le sud */
  AC_REFLEXION_E,   /* réflexion vers l'est */
  AC_REFLEXION_N,   /* réflexion vers le nord */
  AC_REFLEXION_O,   /* réflexion vers l'ouest */
  AC_REFLEXION_S,   /* réflexion vers le sud */
  AC_HAUSSE_E,      /* hausse vers l'est */
  AC_HAUSSE_N,      /* hausse vers le nord */
  AC_HAUSSE_O,      /* hausse vers l'ouest */
  AC_HAUSSE_S,      /* hausse vers le sud */
  AC_YOYO_E,        /* yoyo vers l'est */
  AC_YOYO_N,        /* yoyo vers le nord */
  AC_YOYO_O,        /* yoyo vers l'ouest */
  AC_YOYO_S,        /* yoyo vers le sud */
  AC_TANK           /* bouzillé par le tank */
} Action;

/* Actions pour les objets animés (décor) */
/* -------------------------------------- */

typedef enum {
  OB_DETONATEURA, /* détonateur qui s'abaisse */
  OB_DETONATEURB,
  OB_DETONATEURC,
  OB_BOMBEA, /* bombe qui explose */
  OB_BOMBEB,
  OB_BOMBEC,
  OB_SENSUNIO, /* sens-unique qui monte */
  OB_SENSUNIE,
  OB_SENSUNIN,
  OB_SENSUNIS,
  OB_VITRENS, /* vitre qui casse */
  OB_VITREEO,
  OB_BALLONEX, /* ballon qui explose */
  OB_BAISSE,   /* porte électronique qui se baisse */
  OB_TELE,     /* télé en marche */
  OB_UNSEUL,   /* trappe qui s'ouvre */
  OB_DEPART,   /* ouverture-fermeture du départ */
  OB_TROPBU    /* le sol s'ouvre */
} Objet;

/* Bruitages divers */
/* ---------------- */

typedef enum {
  SOUND_SAUT1        = 1, /* toto va sauter */
  SOUND_SAUT2        = 2, /* toto retombe après un saut */
  SOUND_TROPBU       = 3, /* toto a trop bu */
  SOUND_TOMBE        = 4, /* toto tombe dans un trou */
  SOUND_TROUVEBALLON = 5, /* toto à trouvé le ballon */
  SOUND_TROUVECLE    = 6, /* toto à trouvé une clé */
  SOUND_BOIT         = 7, /* toto boit à la bouteille */
  SOUND_MAGIE        = 8, /* toto va devenir transparent */
  SOUND_ELECTRO      = 9, /* toto s'électrocute */
  SOUND_ARRIVE      = 10, /* toto arrive de sous-terre accroché à un ballon */
  SOUND_REPOS       = 11, /* toto se repose */
  SOUND_DORT        = 12, /* toto s'endort */
  SOUND_GLISSE      = 13, /* toto glisse sur une peau de banane */
  SOUND_TOURTE      = 14, /* toto écrase la tourte */
  SOUND_LUNETTES    = 15, /* toto met ses lunettes */
  SOUND_CREVE       = 16, /* toto méchant crève un ballon */
  SOUND_LIVRE       = 17, /* toto lit un livre */
  SOUND_MALADE      = 18, /* toto tombe malade */
  SOUND_POUSSE      = 19, /* toto pousse une caisse */
  SOUND_SENSUNI     = 20, /* seus-unique se lève */
  SOUND_PORTEOUVRE  = 21, /* porte qui s'ouvre */
  SOUND_PORTEBAISSE = 22, /* porte blindée qui se baisse */
  SOUND_UNSEUL      = 23, /* main qui sort */
  SOUND_VITRECASSE  = 24, /* vitre qui se casse */
  SOUND_BOMBE       = 25, /* bombe qui explose */
  SOUND_NON         = 26, /* toto fait non-non */
  SOUND_CLIC        = 27, /* clic sur un bouton */
  SOUND_CAISSE      = 28, /* caisse qui tombe */
  SOUND_CAISSEV     = 29, /* structure légère qui tombe */
  SOUND_CAISSEO     = 30, /* boule qui tombe */
  SOUND_ACTION      = 31, /* action dans le décor */
  SOUND_MECHANT     = 32, /* toto devient méchant */
  SOUND_PASSEMUR    = 33, /* toto traverse un mur */
  SOUND_AIMANT      = 34, /* arrive sur un aimant */
  SOUND_PORTEBAISSE2 = 35, /* porte blindée ouverte */
  SOUND_MACHINE      = 36, /* machine électronique */
  SOUND_OISEAUX      = 37, /* oiseaux */
  SOUND_BURP         = 38, /* toto a trop bu */
  SOUND_CAISSEG      = 39, /* machine qui tombe */

  SOUND_MUSIC11 = 40, /* musique 1-1 */
  SOUND_MUSIC12 = 41, /* musique 1-2 */
  SOUND_MUSIC13 = 42, /* musique 1-3 */
  SOUND_MUSIC14 = 43, /* musique 1-4 */
  SOUND_MUSIC21 = 44, /* musique 2-1 */
  SOUND_MUSIC22 = 45, /* musique 2-2 */
  SOUND_MUSIC23 = 46, /* musique 2-3 */
  SOUND_MUSIC24 = 47, /* musique 2-4 */
  SOUND_MUSIC31 = 48, /* musique 3-1 */
  SOUND_MUSIC32 = 49, /* musique 3-2 */
  SOUND_MUSIC33 = 50, /* musique 3-3 */
  SOUND_MUSIC34 = 51, /* musique 3-4 */
  SOUND_MUSIC41 = 52, /* musique 4-1 */
  SOUND_MUSIC42 = 53, /* musique 4-2 */
  SOUND_MUSIC43 = 54, /* musique 4-3 */
  SOUND_MUSIC44 = 55, /* musique 4-4 */

  SOUND_MAX = 56,
} Sound;

/* Procédures communes */
/* ------------------- */

extern short * ConvActionToTabIcon (Action action, short typemarche);
extern short * ConvActionToTabMove (Action action);
extern short * ConvObjetToTabIcon (Objet objet);


/* ========== */
/* bm_table.c */
/* ========== */

#include "actions.h"
#include "blupimania.h"
#include "icon.h"

/* ----------------------- */
/* Tables pour les actions */
/* ----------------------- */
/* clang-format off */

/* toto est stoppé */
/* --------------- */

static short tabiconstope[] = {OPREPEAT, 999, 1,  1, OPTERM};
static short tabiconstopo[] = {OPREPEAT, 999, 1,  7, OPTERM};
static short tabiconstopn[] = {OPREPEAT, 999, 1, 10, OPTERM};
static short tabiconstops[] = {OPREPEAT, 999, 1,  4, OPTERM};


/* toto part (avant la marche) */
/* --------------------------- */

static short tabiconstarte[] = {OPREPEAT, 999, 1, 128 + 64, OPTERM};
static short tabiconstarto[] = {OPREPEAT, 999, 1, 128 + 66, OPTERM};
static short tabiconstartn[] = {OPREPEAT, 999, 1, 128 + 67, OPTERM};
static short tabiconstarts[] = {OPREPEAT, 999, 1, 128 + 65, OPTERM};


/* toto marche */
/* ----------- */

static short tabiconmarchee[]   = {OPREPEAT, 999, 4,  1,  2,  1,  3, OPTERM};
static short tabiconmarcheo[]   = {OPREPEAT, 999, 4,  7,  8,  7,  9, OPTERM};
static short tabiconmarchen[]   = {OPREPEAT, 999, 4, 10, 11, 10, 12, OPTERM};
static short tabiconmarches[]   = {OPREPEAT, 999, 4,  4,  5,  4,  6, OPTERM};

static short tabiconmarchefe[]  = {OPREPEAT, 999, 6,  2,  1, 128 + 69,  3,  1, 128 + 68, OPTERM};
static short tabiconmarchefo[]  = {OPREPEAT, 999, 6,  9,  7, 128 + 72,  8,  7, 128 + 73, OPTERM};
static short tabiconmarchefn[]  = {OPREPEAT, 999, 6, 11, 10, 128 + 75, 12, 10, 128 + 74, OPTERM};
static short tabiconmarchefs[]  = {OPREPEAT, 999, 6,  6,  4, 128 + 70,  5,  4, 128 + 71, OPTERM};

static short tabiconmarchefre[] = {OPREPEAT, 999, 6,  2, 128 + 68,  1,  3, 128 + 69,  1, OPTERM};
static short tabiconmarchefro[] = {OPREPEAT, 999, 6,  9, 128 + 73,  7,  8, 128 + 72,  7, OPTERM};
static short tabiconmarchefrn[] = {OPREPEAT, 999, 6, 11, 128 + 74, 10, 12, 128 + 75, 10, OPTERM};
static short tabiconmarchefrs[] = {OPREPEAT, 999, 6,  6, 128 + 71,  4,  5, 128 + 70,  4, OPTERM};


/* toto tourne d'un quart de tour */
/* ------------------------------ */

static short tabmovetourne[] = {OPREPEAT, 2, 1, 0, 0, 0, OPTERM};

static short tabicontournene[] = {OPLIST, 2, 16,  1, OPTERM};
static short tabicontourneso[] = {OPLIST, 2, 17,  7, OPTERM};
static short tabicontourneon[] = {OPLIST, 2, 19, 10, OPTERM};
static short tabicontournees[] = {OPLIST, 2, 18,  4, OPTERM};
static short tabicontournese[] = {OPLIST, 2, 18,  1, OPTERM};
static short tabicontourneno[] = {OPLIST, 2, 19,  7, OPTERM};
static short tabicontourneen[] = {OPLIST, 2, 16, 10, OPTERM};
static short tabicontourneos[] = {OPLIST, 2, 17,  4, OPTERM};


/* toto saute un obstacle */
/* ---------------------- */

static short tabmovesaute1e[] = {OPLIST, 3 + 4,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 12, 3, -30, 14, 3, -8, 12, 3, -5, 12, 4, -2,
                                 OPTERM};
static short tabmovesaute1o[] = {OPLIST, 3 + 4,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 -12, -3, -30, -14, -3, -8, -12, -3, -5, -12, -4, -2,
                                 OPTERM};
static short tabmovesaute1s[] = {OPLIST, 3 + 4,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 -10, 6, -30, -10, 4, -8, -10, 6, -5, -12, 4, -2,
                                 OPTERM};
static short tabmovesaute1n[] = {OPLIST, 3 + 4,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 10, -6, -30, 10, -4, -8, 10, -6, -5, 12, -4, -2,
                                 OPTERM};

static short tabiconsaute1e[] = {OPLIST, 3, 20, 20, 20,
                                 OPSOUND, SOUND_SAUT1,
                                 OPLIST, 4, 21, 21, 21, 21,
                                 OPTERM};
static short tabiconsaute1o[] = {OPLIST, 3, 24, 24, 24,
                                 OPSOUND, SOUND_SAUT1,
                                 OPLIST, 4, 25, 25, 25, 25,
                                 OPTERM};
static short tabiconsaute1s[] = {OPLIST, 3, 22, 22, 22,
                                 OPSOUND, SOUND_SAUT1,
                                 OPLIST, 4, 23, 23, 23, 23,
                                 OPTERM};
static short tabiconsaute1n[] = {OPLIST, 3, 26, 26, 26,
                                 OPSOUND, SOUND_SAUT1,
                                 OPLIST, 4, 27, 27, 27, 27,
                                 OPTERM};


static short tabmovesaute2e[] = {OPLIST, 3 + 2,
                                 12, 3, 2, 14, 3, 5, 12, 3, 8,
                                 0, 0, 30, 0, 0, 0,
                                 OPTERM};
static short tabmovesaute2o[] = {OPLIST, 3 + 2,
                                 -12, -3, 2, -14, -3, 5, -12, -3, 8,
                                 0, 0, 30, 0, 0, 0,
                                 OPTERM};
static short tabmovesaute2s[] = {OPLIST, 3 + 2,
                                 -10, 6, 2, -10, 4, 5, -10, 6, 8,
                                 0, 0, 30, 0, 0, 0,
                                 OPTERM};
static short tabmovesaute2n[] = {OPLIST, 3 + 2,
                                 10, -6, 2, 10, -4, 5, 10, -6, 8,
                                 0, 0, 30, 0, 0, 0,
                                 OPTERM};

static short tabiconsaute2e[] = {OPLIST, 3, 21, 21, 21,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 2, 20, 1,
                                 OPTERM};
static short tabiconsaute2o[] = {OPLIST, 3, 25, 25, 25,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 2, 24, 7,
                                 OPTERM};
static short tabiconsaute2s[] = {OPLIST, 3, 23, 23, 23,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 2, 22, 4,
                                 OPTERM};
static short tabiconsaute2n[] = {OPLIST, 3, 27, 27, 27,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 2, 26, 10,
                                 OPTERM};


/* toto tombe dans un trou */
/* ----------------------- */

static short tabmovetombee[] = {OPLIST, 9, 6, 2, 0, 7, 1, 0, 6, 2, 0, 6, 1, 0, 6, 2, 0, 7, 1, 5,
                                           6, 2, 10, 0, 0, 15, 0, 0, 20, OPTERM};
static short tabmovetombeo[] = {OPLIST, 9, -6, -2, 0, -7, -1, 0, -6, -2, 0, -6, -1, 0, -6, -2, 0, -7, -1, 5,
                                           -6, -2, 10, 0, 0, 15, 0, 0, 20, OPTERM};
static short tabmovetomben[] = {OPLIST, 9, 5, -3, 0, 5, -2, 0, 5, -3, 0, 6, -2, 0, 5, -3, 0, 5, -2, 5,
                                           5, -3, 10, 0, 0, 15, 0, 0, 20, OPTERM};
static short tabmovetombes[] = {OPLIST, 9, -5, 3, 0, -5, 2, 0, -5, 3, 0, -6, 2, 0, -5, 3, 0, -5, 2, 5,
                                           -5, 3, 10, 0, 0, 15, 0, 0, 20, OPTERM};

static short tabicontombee[] = {OPLIST, 5, 1, 2, 1, 3 + 32, 1 + 32,
                                OPSOUND,  SOUND_TOMBE,
                                OPLIST, 4, 28, 28, 28, 28,
                                OPTERM};
static short tabicontombeo[] = {OPLIST, 5, 7, 8, 7, 9 + 32, 7 + 32,
                                OPSOUND,  SOUND_TOMBE,
                                OPLIST, 4, 30, 30, 30, 30,
                                OPTERM};
static short tabicontombes[] = {OPLIST, 5, 4, 5, 4, 6 + 32, 4 + 32,
                                OPSOUND,  SOUND_TOMBE,
                                OPLIST, 4, 29, 29, 29, 29,
                                OPTERM};
static short tabicontomben[] = {OPLIST, 5, 10, 11, 10, 12 + 32, 10 + 32,
                                OPSOUND,  SOUND_TOMBE,
                                OPLIST, 4, 31, 31, 31, 31,
                                OPTERM};

static short tabicontombetanke[] = {OPREPEAT, 9, 1, 1,  OPTERM};
static short tabicontombetanko[] = {OPREPEAT, 9, 1, 7,  OPTERM};
static short tabicontombetanks[] = {OPREPEAT, 9, 1, 4,  OPTERM};
static short tabicontombetankn[] = {OPREPEAT, 9, 1, 10, OPTERM};

static short tabmovetombetankbe[] = {OPLIST, 9,  6, 2, 0, 7, 1, 0, 6, 2, 0, 6, 1, 0, 6, 2, 0, 7, 1, 5,
                                                 6, 2, 10, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 4,  0, 0, -20, 0, 0, -15, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 2,  0, 0, -20, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPTERM};
static short tabmovetombetankbo[] = {OPLIST, 9,  -6, -2, 0, -7, -1, 0, -6, -2, 0, -6, -1, 0, -6, -2, 0, -7, -1, 5,
                                                 -6, -2, 10, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 4,  0, 0, -20, 0, 0, -15, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 2,  0, 0, -20, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPTERM};
static short tabmovetombetankbn[] = {OPLIST, 9,  5, -3, 0, 5, -2, 0, 5, -3, 0, 6, -2, 0, 5, -3, 0, 5, -2, 5,
                                                 5, -3, 10, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 4,  0, 0, -20, 0, 0, -15, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 2,  0, 0, -20, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPTERM};
static short tabmovetombetankbs[] = {OPLIST, 9,  -5, 3, 0, -5, 2, 0, -5, 3, 0, -6, 2, 0, -5, 3, 0, -5, 2, 5,
                                                 -5, 3, 10, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 4,  0, 0, -20, 0, 0, -15, 0, 0, 15, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPLIST, 2,  0, 0, -20, 0, 0, 20,
                                     OPSOUND,    SOUND_CAISSE,
                                     OPTERM};

static short tabicontombetankbe[] = {OPREPEAT, 15, 1, 1,  OPTERM};
static short tabicontombetankbo[] = {OPREPEAT, 15, 1, 7,  OPTERM};
static short tabicontombetankbs[] = {OPREPEAT, 15, 1, 4,  OPTERM};
static short tabicontombetankbn[] = {OPREPEAT, 15, 1, 10, OPTERM};


/* toto boit à la bouteille */
/* ------------------------ */

static short tabmoveboit[] = {OPREPEAT, 5, 2, 0, 0, -2, 0, 0, +2, OPTERM};

static short tabiconboite[] = {OPSOUND, SOUND_BOIT, OPREPEAT, 10, 1, 52, OPTERM};
static short tabiconboito[] = {OPSOUND, SOUND_BOIT, OPREPEAT, 10, 1, 54, OPTERM};
static short tabiconboits[] = {OPSOUND, SOUND_BOIT, OPREPEAT, 10, 1, 53, OPTERM};
static short tabiconboitn[] = {OPSOUND, SOUND_BOIT, OPREPEAT, 10, 1, 55, OPTERM};


/* toto boit à la mauvaise bouteille */
/* --------------------------------- */

static short tabmoveboitxe[] = {OPREPEAT, 5, 2,  0, 0, -2, 0, 0, +2,
                                OPREPEAT, 70, 1, 0, 0, 0,
                                OPTERM};
static short tabmoveboitxo[] = {OPREPEAT, 5, 2,  0, 0, -2, 0, 0, +2,
                                OPREPEAT, 72, 1, 0, 0, 0,
                                OPTERM};
static short tabmoveboitxs[] = {OPREPEAT, 5, 2,  0, 0, -2, 0, 0, +2,
                                OPREPEAT, 70, 1, 0, 0, 0,
                                OPTERM};
static short tabmoveboitxn[] = {OPREPEAT, 5, 2,  0, 0, -2, 0, 0, +2,
                                OPREPEAT, 72, 1, 0, 0, 0,
                                OPTERM};

static short tabiconboitxe[] = {OPSOUND,       SOUND_BOIT,
                                OPREPEAT, 10, 1, 52,
                                OPSOUND,       SOUND_MALADE,
                                OPLIST, 30,    128 + 20, 128 + 21, 128 + 20, 128 + 21, 128 + 20,
                                               128 + 21, 128 + 22, 128 + 21, 128 + 22, 128 + 22,
                                               128 + 22, 128 + 22, 128 + 23, 128 + 24, 128 + 23,
                                               128 + 24, 128 + 23, 128 + 24, 128 + 24, 128 + 24,
                                               128 + 24, 128 + 24, 128 + 24, 128 + 25, 128 + 25,
                                               128 + 26, 128 + 26, 128 + 25, 128 + 26, 128 + 26,
                                OPREPEAT, 10, 1, 128 + 26,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 20, 1, 128 + 27,
                                OPREPEAT, 8, 1,  128 + 28,
                                OPLIST, 2,      50, 33,
                                OPTERM};
static short tabiconboitxo[] = {OPSOUND,       SOUND_BOIT,
                                OPREPEAT, 10, 1, 54,
                                OPSOUND,       SOUND_MALADE,
                                OPLIST, 30,    128 + 20, 128 + 21, 128 + 20, 128 + 21, 128 + 20,
                                               128 + 21, 128 + 22, 128 + 21, 128 + 22, 128 + 22,
                                               128 + 22, 128 + 22, 128 + 23, 128 + 24, 128 + 23,
                                               128 + 24, 128 + 23, 128 + 24, 128 + 24, 128 + 24,
                                               128 + 24, 128 + 24, 128 + 24, 128 + 25, 128 + 25,
                                               128 + 26, 128 + 26, 128 + 25, 128 + 26, 128 + 26,
                                OPREPEAT, 10, 1, 128 + 26,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 20, 1, 128 + 27,
                                OPREPEAT, 8, 1,  128 + 28,
                                OPLIST, 2,      50, 36, 49, 39,
                                OPTERM};
static short tabiconboitxs[] = {OPSOUND,       SOUND_BOIT,
                                OPREPEAT, 10, 1, 53,
                                OPSOUND,       SOUND_MALADE,
                                OPLIST, 30,    128 + 20, 128 + 21, 128 + 20, 128 + 21, 128 + 20,
                                               128 + 21, 128 + 22, 128 + 21, 128 + 22, 128 + 22,
                                               128 + 22, 128 + 22, 128 + 23, 128 + 24, 128 + 23,
                                               128 + 24, 128 + 23, 128 + 24, 128 + 24, 128 + 24,
                                               128 + 24, 128 + 24, 128 + 24, 128 + 25, 128 + 25,
                                               128 + 26, 128 + 26, 128 + 25, 128 + 26, 128 + 26,
                                OPREPEAT, 10, 1, 128 + 26,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 20, 1, 128 + 27,
                                OPREPEAT, 8, 1,  128 + 28,
                                OPLIST, 2,      50, 36,
                                OPTERM};
static short tabiconboitxn[] = {OPSOUND,       SOUND_BOIT,
                                OPREPEAT, 10, 1, 55,
                                OPSOUND,       SOUND_MALADE,
                                OPLIST, 30,    128 + 20, 128 + 21, 128 + 20, 128 + 21, 128 + 20,
                                               128 + 21, 128 + 22, 128 + 21, 128 + 22, 128 + 22,
                                               128 + 22, 128 + 22, 128 + 23, 128 + 24, 128 + 23,
                                               128 + 24, 128 + 23, 128 + 24, 128 + 24, 128 + 24,
                                               128 + 24, 128 + 24, 128 + 24, 128 + 25, 128 + 25,
                                               128 + 26, 128 + 26, 128 + 25, 128 + 26, 128 + 26,
                                OPREPEAT, 10, 1, 128 + 26,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 20, 1, 128 + 27,
                                OPREPEAT, 8, 1,  128 + 28,
                                OPLIST, 2,      50, 33, 48, 42,
                                OPTERM};


/* toto jour du piano */
/* ------------------ */

static short tabiconpianoo[] = {OPSOUND,    SOUND_MUSIC31,
                                OPLIST, 30, 56, 57, 56, 56, 57, 56, 56, 57, 56, 56, 57, 57, 56, 57, 56,
                                            57, 56, 57, 56, 56, 57, 56, 56, 57, 57, 56, 56, 57, 56, 56,
                                OPTERM};


/* toto se repose */
/* -------------- */

static short tabiconrepose[] = {OPREPEAT, 3, 1,  33,
                                OPSOUND,       SOUND_REPOS,
                                OPREPEAT, 25, 1, 64,
                                OPREPEAT, 2, 1,  33,
                                OPTERM};
static short tabiconreposo[] = {OPREPEAT, 3, 1,  39,
                                OPSOUND,       SOUND_REPOS,
                                OPREPEAT, 25, 1, 66,
                                OPREPEAT, 2, 1,  39,
                                OPTERM};
static short tabiconreposs[] = {OPREPEAT, 3, 1,  36,
                                OPSOUND,       SOUND_REPOS,
                                OPREPEAT, 25, 1, 65,
                                OPREPEAT, 2, 1,  36,
                                OPTERM};
static short tabiconreposn[] = {OPREPEAT, 3, 1,  42,
                                OPSOUND,       SOUND_REPOS,
                                OPREPEAT, 25, 1, 67,
                                OPREPEAT, 2, 1,  42,
                                OPTERM};


/* toto dort */
/* --------- */

static short tabicondorte[] =  {OPREPEAT, 3, 1,  33,
                                OPREPEAT, 7, 1,  64,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 30, 1, 68,
                                OPREPEAT, 8, 1,  64,
                                OPREPEAT, 2, 1,  33,
                                OPTERM};
static short tabicondorto[] =  {OPREPEAT, 3, 1,  39,
                                OPREPEAT, 7, 1,  66,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 30, 1, 70,
                                OPREPEAT, 8, 1,  66,
                                OPREPEAT, 2, 1,  39,
                                OPTERM};
static short tabicondorts[] =  {OPREPEAT, 3, 1,  36,
                                OPREPEAT, 7, 1,  65,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 30, 1, 69,
                                OPREPEAT, 8, 1,  65,
                                OPREPEAT, 2, 1,  36,
                                OPTERM};
static short tabicondortn[] =  {OPREPEAT, 3, 1,  42,
                                OPREPEAT, 7, 1,  67,
                                OPSOUND,  SOUND_DORT,
                                OPREPEAT, 30, 1, 71,
                                OPREPEAT, 8, 1,  67,
                                OPREPEAT, 2, 1,  42,
                                OPTERM};


/* toto se gratte la tête */
/* ---------------------- */

static short tabiconreflexione[] = {OPREPEAT, 2, 2, 128 + 13, 128 + 14, OPTERM};
static short tabiconreflexiono[] = {OPREPEAT, 2, 2, 128 + 45, 128 + 46, OPTERM};
static short tabiconreflexions[] = {OPREPEAT, 2, 2, 128 + 29, 128 + 30, OPTERM};
static short tabiconreflexionn[] = {OPREPEAT, 2, 2, 128 + 61, 128 + 62, OPTERM};


/* toto hausse les épaules */
/* ----------------------- */

static short tabiconhaussee[] = {OPREPEAT, 4, 1, 128 + 15, OPTERM};
static short tabiconhausseo[] = {OPREPEAT, 4, 1, 128 + 47, OPTERM};
static short tabiconhausses[] = {OPREPEAT, 4, 1, 128 + 31, OPTERM};
static short tabiconhaussen[] = {OPREPEAT, 4, 1, 128 + 63, OPTERM};


/* toto joue au yoyo */
/* ----------------- */

static short tabiconyoyoe[] = {OPREPEAT, 15, 4, 128 + 114, 128 + 113, 128 + 112, 128 + 113, OPTERM};
static short tabiconyoyoo[] = {OPREPEAT, 15, 4, 128 + 120, 128 + 119, 128 + 118, 128 + 119, OPTERM};
static short tabiconyoyos[] = {OPREPEAT, 15, 4, 128 + 117, 128 + 116, 128 + 115, 128 + 116, OPTERM};
static short tabiconyoyon[] = {OPREPEAT, 15, 4, 128 + 123, 128 + 122, 128 + 121, 128 + 122, OPTERM};


/* toto saute de joie */
/* ------------------ */

static short tabmoveyoupie[] = {OPLIST, 9, 0, 0, -20, 0, 0, -10, 0, 0, -5, 0, 0, -2,
                                          0, 0, +2, 0, 0, +5, 0, 0, +10, 0, 0, +20, 0, 0, 0,
                                OPTERM};

static short tabiconyoupiee[] = {OPLIST, 8, 18, 4, 17, 7, 19, 10, 16, 1,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 1, 1,
                                 OPTERM};

static short tabiconyoupieo[] = {OPLIST, 8, 19, 10, 16, 1, 18, 4, 17, 7,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 1, 7,
                                 OPTERM};

static short tabiconyoupies[] = {OPLIST, 8, 17, 7, 19, 10, 16, 1, 18, 4,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 1, 4,
                                 OPTERM};

static short tabiconyoupien[] = {OPLIST, 8, 16, 1, 18, 4, 17, 7, 19, 10,
                                 OPSOUND, SOUND_SAUT2,
                                 OPLIST, 1, 10,
                                 OPTERM};


/* toto fait non-non */
/* ----------------- */

static short tabiconnone[] = {OPSOUND, SOUND_NON, OPREPEAT, 2, 4, 33, 48, 33, 50, OPTERM};
static short tabiconnono[] = {OPSOUND, SOUND_NON, OPREPEAT, 2, 4, 39, 49, 39, 51, OPTERM};
static short tabiconnons[] = {OPSOUND, SOUND_NON, OPREPEAT, 2, 4, 36, 49, 36, 50, OPTERM};
static short tabiconnonn[] = {OPSOUND, SOUND_NON, OPREPEAT, 2, 4, 42, 48, 42, 51, OPTERM};


/* toto a trop bu */
/* -------------- */

static short tabmoveexplose[] = {OPREPEAT, 15, 1, 0, 0, 0,
                                 OPREPEAT, 5, 2,  0, 0, -2, 0, 0, +2,
                                 OPLIST, 6, 0, 0, 5, 0, 0, 10, 0, 0, 15, 0, 0, 20, 0, 0, 25, 0, 0, 25,
                                 OPREPEAT, 8, 1,  0, 0, 0,
                                 OPTERM};

static short tabiconexplosee[] = {OPLIST, 3, 50, 50, 50,
                                  OPSOUND,  SOUND_BURP,
                                  OPREPEAT, 22, 1, 14,
                                  OPSOUND,  SOUND_TROPBU,
                                  OPREPEAT, 14, 1, 15,
                                  OPTERM};

static short tabiconexploseo[] = {OPLIST, 5, 49, 36, 50, 50, 50,
                                  OPSOUND,  SOUND_BURP,
                                  OPREPEAT, 20, 1, 14,
                                  OPSOUND,  SOUND_TROPBU,
                                  OPREPEAT, 14, 1, 15,
                                  OPTERM};

static short tabiconexploses[] = {OPLIST, 3, 50, 50, 50,
                                  OPSOUND,  SOUND_BURP,
                                  OPREPEAT, 22, 1, 14,
                                  OPSOUND,  SOUND_TROPBU,
                                  OPREPEAT, 14, 1, 15,
                                  OPTERM};

static short tabiconexplosen[] = {OPLIST, 5, 48, 33, 50, 50, 50,
                                  OPSOUND,  SOUND_BURP,
                                  OPREPEAT, 20, 1, 14,
                                  OPSOUND,  SOUND_TROPBU,
                                  OPREPEAT, 14, 1, 15,
                                  OPTERM};

static short tabicontropbu[] = {OPREPEAT, 24, 1, 0,
                                OPLIST, 1,      ICO_TROU,
                                OPTERM};


/* toto prend les lunettes */
/* ----------------------- */

static short tabmovevisione[] = {OPLIST, 12 + 6 + 1,
                                 0, 0, -18, 0, 0, 18, 0, 0, -7, 0, 0, 7, 0, 0, -4, 0, 0, 4, 0, 0, -2, 0, 0, 2,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, -14, 0, 0, -5, 0, 0, -2, 0, 0, +2, 0, 0, +5, 0, 0, +14,
                                 0, 0, 0, OPTERM};
static short tabmovevisiono[] = {OPLIST, 12 + 6 + 5,
                                 0, 0, -18, 0, 0, 18, 0, 0, -7, 0, 0, 7, 0, 0, -4, 0, 0, 4, 0, 0, -2, 0, 0, 2,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, -14, 0, 0, -5, 0, 0, -2, 0, 0, +2, 0, 0, +5, 0, 0, +14,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, OPTERM};
static short tabmovevisions[] = {OPLIST, 12 + 6 + 3,
                                 0, 0, -18, 0, 0, 18, 0, 0, -7, 0, 0, 7, 0, 0, -4, 0, 0, 4, 0, 0, -2, 0, 0, 2,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, -14, 0, 0, -5, 0, 0, -2, 0, 0, +2, 0, 0, +5, 0, 0, +14,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, OPTERM};
static short tabmovevisionn[] = {OPLIST, 12 + 6 + 7,
                                 0, 0, -18, 0, 0, 18, 0, 0, -7, 0, 0, 7, 0, 0, -4, 0, 0, 4, 0, 0, -2, 0, 0, 2,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                 0, 0, -14, 0, 0, -5, 0, 0, -2, 0, 0, +2, 0, 0, +5, 0, 0, +14,
                                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, OPTERM};

static short tabiconvisione[] = {OPSOUND,       SOUND_LUNETTES,
                                 OPREPEAT, 12, 1, 80,
                                 OPLIST, 6,     4, 17, 7, 19, 10, 16,
                                 OPSOUND,       SOUND_SAUT2,
                                 OPLIST, 1,     1,
                                 OPTERM};
static short tabiconvisiono[] = {OPSOUND,       SOUND_LUNETTES,
                                 OPREPEAT, 12, 1, 80,
                                 OPLIST, 6,     4, 17, 7, 19, 10, 16,
                                 OPSOUND,       SOUND_SAUT2,
                                 OPLIST, 5,     1, 18, 4, 17, 7,
                                 OPTERM};
static short tabiconvisions[] = {OPSOUND,       SOUND_LUNETTES,
                                 OPREPEAT, 12, 1, 80,
                                 OPLIST, 6,     4, 17, 7, 19, 10, 16,
                                 OPSOUND,       SOUND_SAUT2,
                                 OPLIST, 3,     1, 18, 4,
                                 OPTERM};
static short tabiconvisionn[] = {OPSOUND,       SOUND_LUNETTES,
                                 OPREPEAT, 12, 1, 80,
                                 OPLIST, 6,     4, 17, 7, 19, 10, 16,
                                 OPSOUND,       SOUND_SAUT2,
                                 OPLIST, 7,     1, 18, 4, 17, 7, 19, 10,
                                 OPTERM};


/* toto arrive en montant l'ascenseur */
/* ---------------------------------- */

static short tabicondepartob[] = {OPSOUND,       SOUND_PORTEOUVRE,
                                  OPLIST, 4,     ICO_DEPARTOUV + 0, ICO_DEPARTOUV + 0,
                                                 ICO_DEPARTOUV + 1, ICO_DEPARTOUV + 1,
                                  OPREPEAT, 36, 1, ICO_DEPARTOUV + 2,
                                  OPSOUND,       SOUND_PORTEOUVRE,
                                  OPLIST, 5,     ICO_DEPARTOUV + 1, ICO_DEPARTOUV + 1,
                                                 ICO_DEPARTOUV + 0, ICO_DEPARTOUV + 0,
                                                 ICO_DEPART,
                                  OPTERM};

static short tabmovedeparte[] = {OPREPEAT, 10, 1, 0, 0, 0,
                                 OPREPEAT, 15, 1, 0, 0, -10,
                                 OPLIST, 4,      11, 3, -10, 11, 3, -5, 11, 3, +5, 11, 2, +10,
                                 OPREPEAT, 6, 1,  0, 0, 0,
                                 OPTERM};

static short tabicondeparte[] = {OPREPEAT, 10, 1, 63,
                                 OPSOUND,       SOUND_ARRIVE,
                                 OPREPEAT, 19, 1, 63,
                                 OPSOUND,       SOUND_SAUT2,
                                 OPREPEAT, 2, 1,  63,
                                 OPREPEAT, 4, 1,  81,
                                 OPTERM};


static short tabmoveballone[] = {OPREPEAT, 10, 1, 0, 0, 0,
                                 OPREPEAT, 15, 1, 0, 0, -10,
                                 OPLIST, 4,      11, 3, -10, 11, 3, -5, 11, 3, +5, 11, 2, +10,
                                 OPREPEAT, 2, 1,  0, 0, 0,
                                 OPREPEAT, 20, 1, 0, 0, -20,
                                 OPTERM};

static short tabiconballone[] = {OPREPEAT, 999, 1, 47, OPTERM};


/* toto saute sur l'arrivée et monte au ciel */
/* ----------------------------------------- */

static short tabmovearriveee[] = {OPREPEAT, 3, 1,  8, 2, 0,
                                  OPREPEAT, 10, 1, 0, 0, 0,
                                  OPTERM};
static short tabmovearriveeo[] = {OPREPEAT, 3, 1,  -8, -2, 0,
                                  OPREPEAT, 12, 1, 0, 0, 0,
                                  OPTERM};
static short tabmovearrivees[] = {OPREPEAT, 3, 1,  -6, 3, 0,
                                  OPREPEAT, 10, 1, 0, 0, 0,
                                  OPTERM};
static short tabmovearriveen[] = {OPREPEAT, 3, 1,  6, -3, 0,
                                  OPREPEAT, 12, 1, 0, 0, 0,
                                  OPTERM};

static short tabiconarriveee[] = {OPLIST, 3,  2, 1, 3,
                                  OPSOUND,   SOUND_TROUVEBALLON,
                                  OPLIST, 10, 1, 18, 45, 46, 45, 18, 45, 46, 18, 1,
                                  OPTERM};
static short tabiconarriveeo[] = {OPLIST, 3,  8, 7, 9,
                                  OPSOUND,   SOUND_TROUVEBALLON,
                                  OPLIST, 12, 7, 17, 4, 18, 45, 46, 45, 18, 45, 46, 18, 1,
                                  OPTERM};
static short tabiconarrivees[] = {OPLIST, 3,  5, 4, 6,
                                  OPSOUND,   SOUND_TROUVEBALLON,
                                  OPLIST, 10, 4, 18, 45, 46, 45, 18, 45, 46, 18, 1,
                                  OPTERM};
static short tabiconarriveen[] = {OPLIST, 3,  11, 10, 12,
                                  OPSOUND,   SOUND_TROUVEBALLON,
                                  OPLIST, 12, 10, 16, 1, 18, 45, 46, 45, 18, 45, 46, 18, 1,
                                  OPTERM};

static short tabmovearriveem[] = {OPREPEAT, 27, 1, 0, 0, -15, OPTERM};
static short tabiconarriveem[] = {OPREPEAT, 27, 1, 79, OPTERM};

static short tabmoveballonm[] = {OPREPEAT, 27, 1, 0, 0, -15, OPTERM};
static short tabiconballonm[] = {OPREPEAT, 27, 1, 47, OPTERM};


/* toto fait de la magie */
/* --------------------- */

static short tabmovemagice[] = {OPREPEAT, 5, 1, 0, 0, 0,
                                OPREPEAT, 4, 2, 0, 0, -12, 0, 0, +12,
                                OPREPEAT, 2, 1, 0, 0, 0,
                                OPTERM};
static short tabmovemagico[] = {OPREPEAT, 7, 1, 0, 0, 0,
                                OPREPEAT, 4, 2, 0, 0, -12, 0, 0, +12,
                                OPREPEAT, 4, 1, 0, 0, 0,
                                OPTERM};
static short tabmovemagics[] = {OPREPEAT, 5, 1, 0, 0, 0,
                                OPREPEAT, 4, 2, 0, 0, -12, 0, 0, +12,
                                OPREPEAT, 2, 1, 0, 0, 0,
                                OPTERM};
static short tabmovemagicn[] = {OPREPEAT, 7, 1, 0, 0, 0,
                                OPREPEAT, 4, 2, 0, 0, -12, 0, 0, +12,
                                OPREPEAT, 4, 1, 0, 0, 0,
                                OPTERM};

static short tabiconmagice[] = {OPLIST, 5,     1, 18, 58, 58, 58,
                                OPSOUND,      SOUND_MAGIE,
                                OPREPEAT, 4, 2, 59, 58,
                                OPLIST, 2,     18, 1,
                                OPTERM};
static short tabiconmagico[] = {OPLIST, 7,     7, 17, 4, 18, 58, 58, 58,
                                OPSOUND,      SOUND_MAGIE,
                                OPREPEAT, 4, 2, 59, 58,
                                OPLIST, 4,     18, 4, 17, 7,
                                OPTERM};
static short tabiconmagics[] = {OPLIST, 5,     4, 18, 58, 58, 58,
                                OPSOUND,      SOUND_MAGIE,
                                OPREPEAT, 4, 2, 59, 58,
                                OPLIST, 2,     18, 4,
                                OPTERM};
static short tabiconmagicn[] = {OPLIST, 7,     10, 16, 1, 18, 58, 58, 58,
                                OPSOUND,      SOUND_MAGIE,
                                OPREPEAT, 4, 2, 59, 58,
                                OPLIST, 4,     18, 1, 16, 10,
                                OPTERM};


/* toto s'électrocute */
/* ------------------ */

static short tabiconelectroo[] = {OPLIST, 4,     7, 7, 60, 60,
                                  OPSOUND,      SOUND_ELECTRO,
                                  OPREPEAT, 5, 2, 61, 62,
                                  OPLIST, 2,     61, 39,
                                  OPTERM};


/* sens-unique qui monte puis redescend */
/* ------------------------------------ */

static short tabiconsensunis[] = {OPSOUND,       SOUND_SENSUNI,
                                  OPLIST, 8,      ICO_SENSUNI_S + 4, ICO_SENSUNI_E + 4,
                                                 ICO_SENSUNI_N + 4, ICO_SENSUNI_O + 4,
                                                 ICO_SENSUNI_S + 8, ICO_SENSUNI_E + 8,
                                                 ICO_SENSUNI_N + 8, ICO_SENSUNI_O + 8,
                                  OPREPEAT, 25, 1, ICO_SENSUNI_S + 8,
                                  OPLIST, 4,      ICO_SENSUNI_S + 4, ICO_SENSUNI_S + 8,
                                                 ICO_SENSUNI_S + 4, ICO_SENSUNI_S,
                                  OPTERM};
static short tabiconsensunie[] = {OPSOUND,       SOUND_SENSUNI,
                                  OPLIST, 8,      ICO_SENSUNI_E + 4, ICO_SENSUNI_N + 4,
                                                 ICO_SENSUNI_O + 4, ICO_SENSUNI_S + 4,
                                                 ICO_SENSUNI_E + 8, ICO_SENSUNI_N + 8,
                                                 ICO_SENSUNI_O + 8, ICO_SENSUNI_S + 8,
                                  OPREPEAT, 25, 1, ICO_SENSUNI_E + 8,
                                  OPLIST, 4,      ICO_SENSUNI_E + 4, ICO_SENSUNI_E + 8,
                                                 ICO_SENSUNI_E + 4, ICO_SENSUNI_E,
                                  OPTERM};
static short tabiconsensunin[] = {OPSOUND,       SOUND_SENSUNI,
                                  OPLIST, 8,      ICO_SENSUNI_N + 4, ICO_SENSUNI_O + 4,
                                                 ICO_SENSUNI_S + 4, ICO_SENSUNI_E + 4,
                                                 ICO_SENSUNI_N + 8, ICO_SENSUNI_O + 8,
                                                 ICO_SENSUNI_S + 8, ICO_SENSUNI_E + 8,
                                  OPREPEAT, 25, 1, ICO_SENSUNI_N + 8,
                                  OPLIST, 4,      ICO_SENSUNI_N + 4, ICO_SENSUNI_N + 8,
                                                 ICO_SENSUNI_N + 4, ICO_SENSUNI_N,
                                  OPTERM};
static short tabiconsensunio[] = {OPSOUND,       SOUND_SENSUNI,
                                  OPLIST, 8,      ICO_SENSUNI_O + 4, ICO_SENSUNI_S + 4,
                                                 ICO_SENSUNI_E + 4, ICO_SENSUNI_N + 4,
                                                 ICO_SENSUNI_O + 8, ICO_SENSUNI_S + 8,
                                                 ICO_SENSUNI_E + 8, ICO_SENSUNI_N + 8,
                                  OPREPEAT, 25, 1, ICO_SENSUNI_O + 8,
                                  OPLIST, 4,      ICO_SENSUNI_O + 4, ICO_SENSUNI_O + 8,
                                                 ICO_SENSUNI_O + 4, ICO_SENSUNI_O,
                                  OPTERM};


/* toto fait éclater un ballon */
/* --------------------------- */

static short tabmovemechant[] = {OPREPEAT, 5, 1, 0, 0, 0,
                                 OPREPEAT, 4, 2, 0, 0, -4, 0, 0, +4,
                                 OPTERM};

static short tabiconmechante[] = {OPSOUND,      SOUND_CREVE,
                                  OPREPEAT, 5, 1, 100,
                                  OPREPEAT, 8, 1, 113,
                                  OPTERM};
static short tabiconmechanto[] = {OPSOUND,      SOUND_CREVE,
                                  OPREPEAT, 5, 1, 102,
                                  OPREPEAT, 8, 1, 119,
                                  OPTERM};
static short tabiconmechants[] = {OPSOUND,      SOUND_CREVE,
                                  OPREPEAT, 5, 1, 101,
                                  OPREPEAT, 8, 1, 116,
                                  OPTERM};
static short tabiconmechantn[] = {OPSOUND,      SOUND_CREVE,
                                  OPREPEAT, 5, 1, 103,
                                  OPREPEAT, 8, 1, 122,
                                  OPTERM};

static short tabiconballonex[] = {OPLIST, 6, ICO_ARRIVEEPRIS, ICO_ARRIVEEPRIS,
                                            ICO_ARRIVEEBOUM, ICO_ARRIVEEPRIS,
                                            ICO_ARRIVEEBOUM, ICO_ARRIVEEVIDE,
                                  OPTERM};


/* toto glisse sur une peau de banane */
/* ---------------------------------- */

static short tabmoveglissee[] = {OPREPEAT, 4, 1, +8, +2, 0,
                                 OPLIST, 2,     +6, +1, -8, +6, +1, +8,
                                 OPREPEAT, 16, 1, 0, 0, 0,
                                 OPTERM};
static short tabmoveglisseo[] = {OPREPEAT, 4, 1, -8, -2, 0,
                                 OPLIST, 2,     -6, -1, -8, -6, -1, +8,
                                 OPREPEAT, 16, 1, 0, 0, 0,
                                 OPTERM};
static short tabmoveglisses[] = {OPREPEAT, 4, 1, -6, +3, 0,
                                 OPLIST, 2,     -6, +3, -8, -6, +3, +8,
                                 OPREPEAT, 16, 1, 0, 0, 0,
                                 OPTERM};
static short tabmoveglissen[] = {OPREPEAT, 4, 1, +6, -3, 0,
                                 OPLIST, 2,     +6, -3, -8, +6, -3, +8,
                                 OPREPEAT, 16, 1, 0, 0, 0,
                                 OPTERM};

static short tabiconglissee[] = {OPREPEAT, 1, 4, 1 + 32, 2 + 32, 1 + 32, 3 + 32,
                                 OPSOUND,      SOUND_GLISSE,
                                 OPREPEAT, 2, 1, 72,
                                 OPREPEAT, 8, 1, 64,
                                 OPSOUND,      SOUND_MECHANT,
                                 OPREPEAT, 8, 1, 113,
                                 OPTERM};
static short tabiconglisseo[] = {OPREPEAT, 1, 4, 7 + 32, 8 + 32, 7 + 32, 9 + 32,
                                 OPSOUND,      SOUND_GLISSE,
                                 OPREPEAT, 2, 1, 74,
                                 OPREPEAT, 8, 1, 66,
                                 OPSOUND,      SOUND_MECHANT,
                                 OPREPEAT, 8, 1, 119,
                                 OPTERM};
static short tabiconglisses[] = {OPREPEAT, 1, 4, 4 + 32, 5 + 32, 4 + 32, 6 + 32,
                                 OPSOUND,      SOUND_GLISSE,
                                 OPREPEAT, 2, 1, 73,
                                 OPREPEAT, 8, 1, 65,
                                 OPSOUND,      SOUND_MECHANT,
                                 OPREPEAT, 8, 1, 116,
                                 OPTERM};
static short tabiconglissen[] = {OPREPEAT, 1, 4, 10 + 32, 11 + 32, 10 + 32, 12 + 32,
                                 OPSOUND,      SOUND_GLISSE,
                                 OPREPEAT, 2, 1, 75,
                                 OPREPEAT, 8, 1, 67,
                                 OPSOUND,      SOUND_MECHANT,
                                 OPREPEAT, 8, 1, 122,
                                 OPTERM};


/* toto lit un livre */
/* ----------------- */

static short tabmovelivre[] = {OPREPEAT, 10, 1, 0, 0, 0,
                               OPREPEAT, 5, 2,  0, 0, -3, 0, 0, +3,
                               OPREPEAT, 10, 1, 0, 0, 0,
                               OPTERM};

static short tabiconlivree[] = {OPSOUND, SOUND_LIVRE, OPREPEAT, 30, 1, 104, OPTERM};
static short tabiconlivreo[] = {OPSOUND, SOUND_LIVRE, OPREPEAT, 30, 1, 106, OPTERM};
static short tabiconlivres[] = {OPSOUND, SOUND_LIVRE, OPREPEAT, 30, 1, 105, OPTERM};
static short tabiconlivren[] = {OPSOUND, SOUND_LIVRE, OPREPEAT, 30, 1, 107, OPTERM};


/* toto écoute de la musique */
/* ------------------------- */

static short tabmovemusiquee[] = {OPREPEAT, 44, 1, 0, 0, 0, OPTERM};
static short tabmovemusiqueo[] = {OPREPEAT, 48, 1, 0, 0, 0, OPTERM};
static short tabmovemusiques[] = {OPREPEAT, 44, 1, 0, 0, 0, OPTERM};
static short tabmovemusiquen[] = {OPREPEAT, 48, 1, 0, 0, 0, OPTERM};

static short tabiconmusiquee[] = {OPLIST, 2,      1, 18,
                                  OPSOUND,       SOUND_MUSIC21,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPSOUND,       SOUND_MUSIC22,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPLIST, 2,      18, 1,
                                  OPTERM};
static short tabiconmusiqueo[] = {OPLIST, 4,      7, 17, 4, 18,
                                  OPSOUND,       SOUND_MUSIC21,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPSOUND,       SOUND_MUSIC22,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPLIST, 4,      18, 4, 17, 7,
                                  OPTERM};
static short tabiconmusiques[] = {OPLIST, 2,      4, 18,
                                  OPSOUND,       SOUND_MUSIC21,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPSOUND,       SOUND_MUSIC22,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPLIST, 2,      18, 4,
                                  OPTERM};
static short tabiconmusiquen[] = {OPLIST, 4,      10, 16, 1, 18,
                                  OPSOUND,       SOUND_MUSIC21,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPSOUND,       SOUND_MUSIC22,
                                  OPREPEAT, 10, 2, 77, 78,
                                  OPLIST, 4,      18, 1, 16, 10,
                                  OPTERM};


/* toto prend une clé */
/* ------------------ */

static short tabmoveclee[] = {OPREPEAT, 10, 1, 0, 0, 0, OPTERM};
static short tabmovecleo[] = {OPREPEAT, 14, 1, 0, 0, 0, OPTERM};
static short tabmovecles[] = {OPREPEAT, 10, 1, 0, 0, 0, OPTERM};
static short tabmoveclen[] = {OPREPEAT, 14, 1, 0, 0, 0, OPTERM};

static short tabiconclee[] = {OPLIST, 1,     1,
                              OPSOUND,      SOUND_TROUVECLE,
                              OPREPEAT, 2, 4, 18, 45, 46, 45,
                              OPLIST, 1,     1,
                              OPTERM};
static short tabiconcleo[] = {OPLIST, 3,     7, 17, 4,
                              OPSOUND,      SOUND_TROUVECLE,
                              OPREPEAT, 2, 4, 18, 45, 46, 45,
                              OPLIST, 3,     4, 17, 7,
                              OPTERM};
static short tabiconcles[] = {OPLIST, 1,     4,
                              OPSOUND,      SOUND_TROUVECLE,
                              OPREPEAT, 2, 4, 18, 45, 46, 45,
                              OPLIST, 1,     4,
                              OPTERM};
static short tabiconclen[] = {OPLIST, 3,     10, 16, 1,
                              OPSOUND,      SOUND_TROUVECLE,
                              OPREPEAT, 2, 4, 18, 45, 46, 45,
                              OPLIST, 3,     1, 16, 10,
                              OPTERM};


/* toto ouvre une porte */
/* -------------------- */

static short tabmoveporte[] = {OPREPEAT, 4, 2, 0, 0, -5, 0, 0, +5, OPTERM};

static short tabiconportee[] = {OPSOUND, SOUND_PORTEOUVRE, OPREPEAT, 8, 1, 1, OPTERM};
static short tabiconporteo[] = {OPSOUND, SOUND_PORTEOUVRE, OPREPEAT, 8, 1, 7, OPTERM};
static short tabiconportes[] = {OPSOUND, SOUND_PORTEOUVRE, OPREPEAT, 8, 1, 4, OPTERM};
static short tabiconporten[] = {OPSOUND, SOUND_PORTEOUVRE, OPREPEAT, 8, 1, 10, OPTERM};



/* toto baisse une porte électronique */
/* ---------------------------------- */

static short tabiconbaisseo[] = {OPLIST, 3,      7, 60, 60,
                                 OPREPEAT, 25, 1, 7,
                                 OPREPEAT, 3, 4,  25, 30, 25, 7,
                                 OPTERM};

static short tabiconbaisse[] = {OPSOUND,      SOUND_PORTEBAISSE,
                                OPREPEAT,  6, 2, ICO_BAISSE + 1, ICO_BAISSE + 2,
                                OPLIST,   15,    ICO_BAISSE + 3, ICO_BAISSE + 3, ICO_BAISSE + 3,
                                                 ICO_BAISSE + 4, ICO_BAISSE + 4, ICO_BAISSE + 4,
                                                 ICO_BAISSE + 5, ICO_BAISSE + 5, ICO_BAISSE + 5,
                                                 ICO_BAISSE + 6, ICO_BAISSE + 6, ICO_BAISSE + 6,
                                                 ICO_BAISSE + 7, ICO_BAISSE + 7, ICO_BAISSE + 7,
                                OPSOUND,         SOUND_PORTEBAISSE2,
                                OPLIST,    1,    ICO_BAISSEBAS,
                                OPTERM};


/* ouvre la trappe "un seul" */
/* ------------------------- */

static short tabiconunseul[] = {OPLIST, 2,  ICO_UNSEUL + 1, ICO_UNSEUL + 1,
                                OPSOUND,    SOUND_UNSEUL,
                                OPLIST, 21, ICO_UNSEUL + 2, ICO_UNSEUL + 2,
                                            ICO_UNSEUL + 2, ICO_UNSEUL + 2, ICO_UNSEUL + 3, ICO_UNSEUL + 4,
                                            ICO_UNSEUL + 3, ICO_UNSEUL + 4, ICO_UNSEUL + 5, ICO_UNSEUL + 4,
                                            ICO_UNSEUL + 3, ICO_UNSEUL + 4, ICO_UNSEUL + 5, ICO_UNSEUL + 6,
                                            ICO_UNSEUL + 5, ICO_UNSEUL + 4, ICO_UNSEUL + 3, ICO_UNSEUL + 4,
                                            ICO_UNSEUL + 5, ICO_UNSEUL + 6, ICO_UNSEUL + 7,
                                OPTERM};


/* toto pousse une caisse */
/* ---------------------- */

static short tabiconpoussee[] = {OPREPEAT, 999, 4, 128 + 48 +  1, 128 + 48 +  2, 128 + 48 +  1, 128 + 48 +  3,
                                 OPTERM};
static short tabiconpousseo[] = {OPREPEAT, 999, 4, 128 + 48 +  7, 128 + 48 +  8, 128 + 48 +  7, 128 + 48 +  9,
                                 OPTERM};
static short tabiconpoussen[] = {OPREPEAT, 999, 4, 128 + 48 + 10, 128 + 48 + 11, 128 + 48 + 10, 128 + 48 + 12,
                                 OPTERM};
static short tabiconpousses[] = {OPREPEAT, 999, 4, 128 + 48 +  4, 128 + 48 +  5, 128 + 48 +  4, 128 + 48 +  6,
                                 OPTERM};

static short tabiconcaisse[]  = {OPREPEAT, 999, 1, ICO_CAISSE,  OPTERM};
static short tabiconcaissev[] = {OPREPEAT, 999, 1, ICO_CAISSEV, OPTERM};
static short tabiconcaisseo[] = {OPREPEAT, 999, 1, ICO_CAISSEO, OPTERM};
static short tabiconcaisseg[] = {OPREPEAT, 999, 1, ICO_CAISSEG, OPTERM};

static short tabmovecaissee[] = {OPLIST, 15, +5, +1, 0, +5, +1, 0, +4, +1, 0, +4, +1, 0, +4, +1, 0,
                                             +4, +1, 0, +3, +1, 0, +3, +0, 0, +3, +1, 0, +2, +1, 0,
                                             +2, +1, 0, +2, +0, 0, +1, +1, 0, +1, +0, 0, +1, +0, 0,
                                 OPTERM};
static short tabmovecaisseo[] = {OPLIST, 15, -5, -1, 0, -5, -1, 0, -4, -1, 0, -4, -1, 0, -4, -1, 0,
                                             -4, -1, 0, -3, -1, 0, -3, -0, 0, -3, -1, 0, -2, -1, 0,
                                             -2, -1, 0, -2, -0, 0, -1, -1, 0, -1, -0, 0, -1, -0, 0,
                                 OPTERM};
static short tabmovecaisses[] = {OPLIST, 15, -4, +2, 0, -4, +2, 0, -3, +2, 0, -3, +2, 0, -3, +2, 0,
                                             -3, +2, 0, -3, +1, 0, -2, +1, 0, -2, +1, 0, -2, +1, 0,
                                             -2, +1, 0, -2, +0, 0, -1, +1, 0, -1, +0, 0, -1, +0, 0,
                                 OPTERM};
static short tabmovecaissen[] = {OPLIST, 15, +4, -2, 0, +4, -2, 0, +3, -2, 0, +3, -2, 0, +3, -2, 0,
                                             +3, -2, 0, +3, -1, 0, +2, -1, 0, +2, -1, 0, +2, -1, 0,
                                             +2, -1, 0, +2, -0, 0, +1, -1, 0, +1, -0, 0, +1, -0, 0,
                                 OPTERM};

static short tabmovecaisset[]  = {OPREPEAT, 5, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSE,
                                  OPREPEAT, 2, 1, 0, 0, -10,
                                  OPREPEAT, 2, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSE,
                                  OPREPEAT, 1, 1, 0, 0, -10,
                                  OPREPEAT, 1, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSE,
                                  OPTERM};
static short tabmovecaissevt[] = {OPREPEAT, 5, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEV,
                                  OPREPEAT, 1, 1, 0, 0, -10,
                                  OPREPEAT, 1, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEV,
                                  OPTERM};
static short tabmovecaisseot[] = {OPREPEAT, 5, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEO,
                                  OPREPEAT, 3, 1, 0, 0, -10,
                                  OPREPEAT, 3, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEO,
                                  OPREPEAT, 2, 1, 0, 0, -10,
                                  OPREPEAT, 2, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEO,
                                  OPREPEAT, 1, 1, 0, 0, -10,
                                  OPREPEAT, 1, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEO,
                                  OPTERM};
static short tabmovecaissegt[] = {OPREPEAT, 5, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEG,
                                  OPREPEAT, 2, 1, 0, 0, +3,
                                  OPREPEAT, 1, 1, 0, 0, -6,
                                  OPREPEAT, 2, 1, 0, 0, -10,
                                  OPREPEAT, 2, 1, 0, 0, +10,
                                  OPSOUND,  SOUND_CAISSEG,
                                  OPREPEAT, 2, 1, 0, 0, +3,
                                  OPREPEAT, 6, 1, 0, 0, -1,
                                  OPTERM};


/* le méchant toto saute sur un détonateur */
/* --------------------------------------- */

static short tabmovesautedete[] = {OPSOUND,  SOUND_SAUT1,
                                   OPLIST, 8, +11, +3, -20, +11, +3, -20, +11, +3, -10, +11, +3, +10,
                                              -11, -3, -10, -11, -3, +10, -11, -3, +20, -11, -3, +20,
                                   OPTERM};
static short tabmovesautedeto[] = {OPSOUND,  SOUND_SAUT1,
                                   OPLIST, 8, -11, -3, -20, -11, -3, -20, -11, -3, -10, -11, -3, +10,
                                              +11, +3, -10, +11, +3, +10, +11, +3, +20, +11, +3, +20,
                                   OPTERM};
static short tabmovesautedetn[] = {OPSOUND,  SOUND_SAUT1,
                                   OPLIST, 8, +9, -4, -20, +9, -4, -20, +9, -4, -10, +9, -4, +10,
                                              -9, +4, -10, -9, +4, +10, -9, +4, +20, -9, +4, +20,
                                   OPTERM};
static short tabmovesautedets[] = {OPSOUND,  SOUND_SAUT1,
                                   OPLIST, 8, -9, +4, -20, -9, +4, -20, -9, +4, -10, -9, +4, +10,
                                              +9, -4, -10, +9, -4, +10, +9, -4, +20, +9, -4, +20,
                                   OPTERM};

static short tabiconsautedete[] = {OPREPEAT, 8, 1, 113, OPTERM};
static short tabiconsautedeto[] = {OPREPEAT, 8, 1, 119, OPTERM};
static short tabiconsautedetn[] = {OPREPEAT, 8, 1, 122, OPTERM};
static short tabiconsautedets[] = {OPREPEAT, 8, 1, 116, OPTERM};

static short tabicondetonateura[] = {OPREPEAT, 2, 1, ICO_DETONATEUR_A,
                                     OPSOUND,      SOUND_CLIC,
                                     OPREPEAT, 1, 1, ICO_DETONATEUR_A + 3,
                                     OPTERM};
static short tabicondetonateurb[] = {OPREPEAT, 2, 1, ICO_DETONATEUR_B,
                                     OPSOUND,      SOUND_CLIC,
                                     OPREPEAT, 1, 1, ICO_DETONATEUR_B + 3,
                                     OPTERM};
static short tabicondetonateurc[] = {OPREPEAT, 2, 1, ICO_DETONATEUR_C,
                                     OPSOUND,      SOUND_CLIC,
                                     OPREPEAT, 1, 1, ICO_DETONATEUR_C + 3,
                                     OPTERM};

static short tabiconbombea[] = {OPREPEAT, 10, 1, ICO_BOMBE_A,
                                OPSOUND,       SOUND_BOMBE,
                                OPREPEAT, 5, 2,  ICO_BOMBE_EX, ICO_BOMBE_A,
                                OPLIST, 1,      -1,
                                OPTERM};
static short tabiconbombeb[] = {OPREPEAT, 10, 1, ICO_BOMBE_B,
                                OPSOUND,       SOUND_BOMBE,
                                OPREPEAT, 5, 2,  ICO_BOMBE_EX, ICO_BOMBE_B,
                                OPLIST, 1,      -1,
                                OPTERM};
static short tabiconbombec[] = {OPREPEAT, 10, 1, ICO_BOMBE_C,
                                OPSOUND,       SOUND_BOMBE,
                                OPREPEAT, 5, 2,  ICO_BOMBE_EX, ICO_BOMBE_C,
                                OPLIST, 1,      -1,
                                OPTERM};

/* toto regarde la télévision */
/* -------------------------- */

static short tabicontelen[] = {OPLIST, 1,      10,
                               OPREPEAT, 18, 1, 67,
                               OPLIST, 1,      10,
                               OPTERM};

static short tabicontele[] = {OPREPEAT, 2, 1, ICO_TELE,
                              OPSOUND,      SOUND_MUSIC11,
                              OPREPEAT, 8, 2, ICO_TELE + 3, ICO_TELE + 4,
                              OPREPEAT, 2, 1, ICO_TELE,
                              OPTERM};


/* toto joue avec la tourte */
/* ------------------------ */

static short tabicontourte[] = {OPSOUND,       SOUND_TOURTE,
                                OPREPEAT, 2, 3,  76, 95, 95,
                                OPREPEAT, 5, 2,  76, 95,
                                OPSOUND,       SOUND_TOURTE,
                                OPREPEAT, 5, 2,  76, 95,
                                OPREPEAT, 3, 3,  76, 95, 76,
                                OPTERM};


/* vitre qui se casse */
/* ------------------ */

static short tabiconvitrens[] = {OPSOUND,      SOUND_VITRECASSE,
                                 OPREPEAT, 2, 2, ICO_VITREC_NS, ICO_VITREC_NS + 1,
                                 OPLIST, 1,     -1,
                                 OPTERM};
static short tabiconvitreeo[] = {OPSOUND,      SOUND_VITRECASSE,
                                 OPREPEAT, 2, 2, ICO_VITREC_EO, ICO_VITREC_EO + 1,
                                 OPLIST, 1,     -1,
                                 OPTERM};


/* toto détruit par un tank */
/* ------------------------ */

static short tabicontank[] = {OPSOUND,       SOUND_ELECTRO,
                              OPREPEAT, 10, 2, 128 + 91, 128 + 92,
                              OPREPEAT, 10, 1, 128 + 93,
                              OPLIST, 3,      128 + 94, 128 + 95, 128 + 94,
                              OPREPEAT, 5, 1,  128 + 93,
                              OPLIST, 3,      128 + 94, 128 + 95, 128 + 94,
                              OPREPEAT, 6, 1,  128 + 93,
                              OPLIST, 9,      128 + 107, 128 + 107, 128 + 107,
                                               128 + 108, 128 + 108, 128 + 108,
                                             128 + 109, 128 + 109, 128 + 109,
                              OPREPEAT, 10, 2, 128 + 110, 128 + 111,
                              OPTERM};

/* clang-format on */

/* ------------------- */
/* ConvActionToTabIcon */
/* ------------------- */

/*
    Cherche la table d'icônes correspondant à une action.
 */

short *
ConvActionToTabIcon (Action action, short typemarche)
{
  switch (action)
  {
  case AC_STOP_E:
    return tabiconstope;
  case AC_STOP_O:
    return tabiconstopo;
  case AC_STOP_S:
    return tabiconstops;
  case AC_STOP_N:
    return tabiconstopn;

  case AC_MARCHE_E:
    if (typemarche == 1)
      return tabiconmarchefe;
    else
      return tabiconmarchee;
  case AC_MARCHE_O:
    if (typemarche == 1)
      return tabiconmarchefo;
    else
      return tabiconmarcheo;
  case AC_MARCHE_S:
    if (typemarche == 1)
      return tabiconmarchefs;
    else
      return tabiconmarches;
  case AC_MARCHE_N:
    if (typemarche == 1)
      return tabiconmarchefn;
    else
      return tabiconmarchen;

  case AC_RECULE_E:
    if (typemarche == 1)
      return tabiconmarchefre;
    else
      return tabiconmarchee;
  case AC_RECULE_O:
    if (typemarche == 1)
      return tabiconmarchefro;
    else
      return tabiconmarcheo;
  case AC_RECULE_S:
    if (typemarche == 1)
      return tabiconmarchefrs;
    else
      return tabiconmarches;
  case AC_RECULE_N:
    if (typemarche == 1)
      return tabiconmarchefrn;
    else
      return tabiconmarchen;

  case AC_TOURNE_NE:
    return tabicontournene;
  case AC_TOURNE_SO:
    return tabicontourneso;
  case AC_TOURNE_ES:
    return tabicontournees;
  case AC_TOURNE_ON:
    return tabicontourneon;

  case AC_TOURNE_SE:
    return tabicontournese;
  case AC_TOURNE_NO:
    return tabicontourneno;
  case AC_TOURNE_OS:
    return tabicontourneos;
  case AC_TOURNE_EN:
    return tabicontourneen;

  case AC_SAUTE1_E:
    return tabiconsaute1e;
  case AC_SAUTE1_O:
    return tabiconsaute1o;
  case AC_SAUTE1_S:
    return tabiconsaute1s;
  case AC_SAUTE1_N:
    return tabiconsaute1n;

  case AC_SAUTE2_E:
    return tabiconsaute2e;
  case AC_SAUTE2_O:
    return tabiconsaute2o;
  case AC_SAUTE2_S:
    return tabiconsaute2s;
  case AC_SAUTE2_N:
    return tabiconsaute2n;

  case AC_SAUTEDET_E:
    return tabiconsautedete;
  case AC_SAUTEDET_O:
    return tabiconsautedeto;
  case AC_SAUTEDET_S:
    return tabiconsautedets;
  case AC_SAUTEDET_N:
    return tabiconsautedetn;

  case AC_TOMBE_E:
    return tabicontombee;
  case AC_TOMBE_O:
    return tabicontombeo;
  case AC_TOMBE_S:
    return tabicontombes;
  case AC_TOMBE_N:
    return tabicontomben;

  case AC_TOMBE_TANK_E:
    return tabicontombetanke;
  case AC_TOMBE_TANK_O:
    return tabicontombetanko;
  case AC_TOMBE_TANK_S:
    return tabicontombetanks;
  case AC_TOMBE_TANK_N:
    return tabicontombetankn;

  case AC_TOMBE_TANKB_E:
    return tabicontombetankbe;
  case AC_TOMBE_TANKB_O:
    return tabicontombetankbo;
  case AC_TOMBE_TANKB_S:
    return tabicontombetankbs;
  case AC_TOMBE_TANKB_N:
    return tabicontombetankbn;

  case AC_BOIT_E:
    return tabiconboite;
  case AC_BOIT_O:
    return tabiconboito;
  case AC_BOIT_S:
    return tabiconboits;
  case AC_BOIT_N:
    return tabiconboitn;

  case AC_BOITX_E:
    return tabiconboitxe;
  case AC_BOITX_O:
    return tabiconboitxo;
  case AC_BOITX_S:
    return tabiconboitxs;
  case AC_BOITX_N:
    return tabiconboitxn;

  case AC_TOURTE_E:
  case AC_TOURTE_O:
  case AC_TOURTE_S:
  case AC_TOURTE_N:
    return tabicontourte;

  case AC_PIANO_O:
    return tabiconpianoo;

  case AC_EXPLOSE_E:
    return tabiconexplosee;
  case AC_EXPLOSE_O:
    return tabiconexploseo;
  case AC_EXPLOSE_S:
    return tabiconexploses;
  case AC_EXPLOSE_N:
    return tabiconexplosen;

  case AC_START_E:
    return tabiconstarte;
  case AC_START_O:
    return tabiconstarto;
  case AC_START_S:
    return tabiconstarts;
  case AC_START_N:
    return tabiconstartn;

  case AC_REPOS_E:
    return tabiconrepose;
  case AC_REPOS_O:
    return tabiconreposo;
  case AC_REPOS_S:
    return tabiconreposs;
  case AC_REPOS_N:
    return tabiconreposn;

  case AC_DORT_E:
    return tabicondorte;
  case AC_DORT_O:
    return tabicondorto;
  case AC_DORT_S:
    return tabicondorts;
  case AC_DORT_N:
    return tabicondortn;

  case AC_YOUPIE_E:
    return tabiconyoupiee;
  case AC_YOUPIE_O:
    return tabiconyoupieo;
  case AC_YOUPIE_S:
    return tabiconyoupies;
  case AC_YOUPIE_N:
    return tabiconyoupien;

  case AC_NON_E:
    return tabiconnone;
  case AC_NON_O:
    return tabiconnono;
  case AC_NON_S:
    return tabiconnons;
  case AC_NON_N:
    return tabiconnonn;

  case AC_POUSSE_E:
    return tabiconpoussee;
  case AC_POUSSE_O:
    return tabiconpousseo;
  case AC_POUSSE_S:
    return tabiconpousses;
  case AC_POUSSE_N:
    return tabiconpoussen;

  case AC_NPOUSSE_E:
    return tabiconpoussee;
  case AC_NPOUSSE_O:
    return tabiconpousseo;
  case AC_NPOUSSE_S:
    return tabiconpousses;
  case AC_NPOUSSE_N:
    return tabiconpoussen;

  case AC_CAISSE_E:
  case AC_CAISSE_O:
  case AC_CAISSE_S:
  case AC_CAISSE_N:
  case AC_CAISSE_T:
    return tabiconcaisse;

  case AC_CAISSEV_E:
  case AC_CAISSEV_O:
  case AC_CAISSEV_S:
  case AC_CAISSEV_N:
  case AC_CAISSEV_T:
    return tabiconcaissev;

  case AC_CAISSEO_E:
  case AC_CAISSEO_O:
  case AC_CAISSEO_S:
  case AC_CAISSEO_N:
  case AC_CAISSEOD_E:
  case AC_CAISSEOD_O:
  case AC_CAISSEOD_S:
  case AC_CAISSEOD_N:
  case AC_CAISSEO_T:
    return tabiconcaisseo;

  case AC_CAISSEG_E:
  case AC_CAISSEG_O:
  case AC_CAISSEG_S:
  case AC_CAISSEG_N:
  case AC_CAISSEG_T:
    return tabiconcaisseg;

  case AC_VISION_E:
    return tabiconvisione;
  case AC_VISION_O:
    return tabiconvisiono;
  case AC_VISION_S:
    return tabiconvisions;
  case AC_VISION_N:
    return tabiconvisionn;

  case AC_DEPART_E:
    return tabicondeparte;
  case AC_BALLON_E:
    return tabiconballone;

  case AC_ARRIVEE_E:
    return tabiconarriveee;
  case AC_ARRIVEE_O:
    return tabiconarriveeo;
  case AC_ARRIVEE_S:
    return tabiconarrivees;
  case AC_ARRIVEE_N:
    return tabiconarriveen;

  case AC_ARRIVEE_M:
    return tabiconarriveem;
  case AC_BALLON_M:
    return tabiconballonm;

  case AC_MAGIC_E:
    return tabiconmagice;
  case AC_MAGIC_O:
    return tabiconmagico;
  case AC_MAGIC_S:
    return tabiconmagics;
  case AC_MAGIC_N:
    return tabiconmagicn;

  case AC_ELECTRO_O:
    return tabiconelectroo;

  case AC_TELE_N:
    return tabicontelen;

  case AC_MECHANT_E:
    return tabiconmechante;
  case AC_MECHANT_O:
    return tabiconmechanto;
  case AC_MECHANT_S:
    return tabiconmechants;
  case AC_MECHANT_N:
    return tabiconmechantn;

  case AC_GLISSE_E:
    return tabiconglissee;
  case AC_GLISSE_O:
    return tabiconglisseo;
  case AC_GLISSE_S:
    return tabiconglisses;
  case AC_GLISSE_N:
    return tabiconglissen;

  case AC_LIVRE_E:
    return tabiconlivree;
  case AC_LIVRE_O:
    return tabiconlivreo;
  case AC_LIVRE_S:
    return tabiconlivres;
  case AC_LIVRE_N:
    return tabiconlivren;

  case AC_MUSIQUE_E:
    return tabiconmusiquee;
  case AC_MUSIQUE_O:
    return tabiconmusiqueo;
  case AC_MUSIQUE_S:
    return tabiconmusiques;
  case AC_MUSIQUE_N:
    return tabiconmusiquen;

  case AC_CLE_E:
    return tabiconclee;
  case AC_CLE_O:
    return tabiconcleo;
  case AC_CLE_S:
    return tabiconcles;
  case AC_CLE_N:
    return tabiconclen;

  case AC_PORTE_E:
    return tabiconportee;
  case AC_PORTE_O:
    return tabiconporteo;
  case AC_PORTE_S:
    return tabiconportes;
  case AC_PORTE_N:
    return tabiconporten;

  case AC_REFLEXION_E:
    return tabiconreflexione;
  case AC_REFLEXION_O:
    return tabiconreflexiono;
  case AC_REFLEXION_S:
    return tabiconreflexions;
  case AC_REFLEXION_N:
    return tabiconreflexionn;

  case AC_HAUSSE_E:
    return tabiconhaussee;
  case AC_HAUSSE_O:
    return tabiconhausseo;
  case AC_HAUSSE_S:
    return tabiconhausses;
  case AC_HAUSSE_N:
    return tabiconhaussen;

  case AC_YOYO_E:
    return tabiconyoyoe;
  case AC_YOYO_O:
    return tabiconyoyoo;
  case AC_YOYO_S:
    return tabiconyoyos;
  case AC_YOYO_N:
    return tabiconyoyon;

  case AC_BAISSE_O:
    return tabiconbaisseo;

  case AC_TANK:
    return tabicontank;

  default:
    break;
  }

  return NULL;
}

/* ------------------- */
/* ConvActionToTabMove */
/* ------------------- */

/*
    Cherche la table de mouvement correspondant à une action.
 */

short *
ConvActionToTabMove (Action action)
{
  switch (action)
  {
  case AC_TOURNE_NE:
  case AC_TOURNE_SO:
  case AC_TOURNE_ES:
  case AC_TOURNE_ON:
  case AC_TOURNE_SE:
  case AC_TOURNE_NO:
  case AC_TOURNE_OS:
  case AC_TOURNE_EN:
    return tabmovetourne;

  case AC_SAUTE1_E:
    return tabmovesaute1e;
  case AC_SAUTE1_O:
    return tabmovesaute1o;
  case AC_SAUTE1_S:
    return tabmovesaute1s;
  case AC_SAUTE1_N:
    return tabmovesaute1n;

  case AC_SAUTE2_E:
    return tabmovesaute2e;
  case AC_SAUTE2_O:
    return tabmovesaute2o;
  case AC_SAUTE2_S:
    return tabmovesaute2s;
  case AC_SAUTE2_N:
    return tabmovesaute2n;

  case AC_SAUTEDET_E:
    return tabmovesautedete;
  case AC_SAUTEDET_O:
    return tabmovesautedeto;
  case AC_SAUTEDET_S:
    return tabmovesautedets;
  case AC_SAUTEDET_N:
    return tabmovesautedetn;

  case AC_TOMBE_E:
  case AC_TOMBE_TANK_E:
    return tabmovetombee;
  case AC_TOMBE_O:
  case AC_TOMBE_TANK_O:
    return tabmovetombeo;
  case AC_TOMBE_S:
  case AC_TOMBE_TANK_S:
    return tabmovetombes;
  case AC_TOMBE_N:
  case AC_TOMBE_TANK_N:
    return tabmovetomben;

  case AC_TOMBE_TANKB_E:
    return tabmovetombetankbe;
  case AC_TOMBE_TANKB_O:
    return tabmovetombetankbo;
  case AC_TOMBE_TANKB_S:
    return tabmovetombetankbs;
  case AC_TOMBE_TANKB_N:
    return tabmovetombetankbn;

  case AC_BOIT_E:
  case AC_BOIT_O:
  case AC_BOIT_S:
  case AC_BOIT_N:
    return tabmoveboit;

  case AC_BOITX_E:
    return tabmoveboitxe;
  case AC_BOITX_O:
    return tabmoveboitxo;
  case AC_BOITX_S:
    return tabmoveboitxs;
  case AC_BOITX_N:
    return tabmoveboitxn;

  case AC_EXPLOSE_E:
  case AC_EXPLOSE_O:
  case AC_EXPLOSE_S:
  case AC_EXPLOSE_N:
    return tabmoveexplose;

  case AC_YOUPIE_E:
  case AC_YOUPIE_O:
  case AC_YOUPIE_S:
  case AC_YOUPIE_N:
    return tabmoveyoupie;

  case AC_CAISSEOD_E:
    return tabmovecaissee;
  case AC_CAISSEOD_O:
    return tabmovecaisseo;
  case AC_CAISSEOD_S:
    return tabmovecaisses;
  case AC_CAISSEOD_N:
    return tabmovecaissen;

  case AC_CAISSE_T:
    return tabmovecaisset;
  case AC_CAISSEV_T:
    return tabmovecaissevt;
  case AC_CAISSEO_T:
    return tabmovecaisseot;
  case AC_CAISSEG_T:
    return tabmovecaissegt;

  case AC_VISION_E:
    return tabmovevisione;
  case AC_VISION_O:
    return tabmovevisiono;
  case AC_VISION_S:
    return tabmovevisions;
  case AC_VISION_N:
    return tabmovevisionn;

  case AC_DEPART_E:
    return tabmovedeparte;
  case AC_BALLON_E:
    return tabmoveballone;

  case AC_ARRIVEE_E:
    return tabmovearriveee;
  case AC_ARRIVEE_O:
    return tabmovearriveeo;
  case AC_ARRIVEE_S:
    return tabmovearrivees;
  case AC_ARRIVEE_N:
    return tabmovearriveen;

  case AC_ARRIVEE_M:
    return tabmovearriveem;
  case AC_BALLON_M:
    return tabmoveballonm;

  case AC_MAGIC_E:
    return tabmovemagice;
  case AC_MAGIC_O:
    return tabmovemagico;
  case AC_MAGIC_S:
    return tabmovemagics;
  case AC_MAGIC_N:
    return tabmovemagicn;

  case AC_MECHANT_E:
  case AC_MECHANT_O:
  case AC_MECHANT_S:
  case AC_MECHANT_N:
    return tabmovemechant;

  case AC_GLISSE_E:
    return tabmoveglissee;
  case AC_GLISSE_O:
    return tabmoveglisseo;
  case AC_GLISSE_S:
    return tabmoveglisses;
  case AC_GLISSE_N:
    return tabmoveglissen;

  case AC_LIVRE_E:
  case AC_LIVRE_O:
  case AC_LIVRE_S:
  case AC_LIVRE_N:
    return tabmovelivre;

  case AC_MUSIQUE_E:
    return tabmovemusiquee;
  case AC_MUSIQUE_O:
    return tabmovemusiqueo;
  case AC_MUSIQUE_S:
    return tabmovemusiques;
  case AC_MUSIQUE_N:
    return tabmovemusiquen;

  case AC_CLE_E:
    return tabmoveclee;
  case AC_CLE_O:
    return tabmovecleo;
  case AC_CLE_S:
    return tabmovecles;
  case AC_CLE_N:
    return tabmoveclen;

  case AC_PORTE_E:
  case AC_PORTE_O:
  case AC_PORTE_S:
  case AC_PORTE_N:
    return tabmoveporte;

  default:
    break;
  }

  return NULL;
}

/* ------------------ */
/* ConvObjetToTabIcon */
/* ------------------ */

/*
    Cherche la table d'icônes correspondant à un objet.
 */

short *
ConvObjetToTabIcon (Objet objet)
{
  switch (objet)
  {
  case OB_BOMBEA:
    return tabiconbombea;
  case OB_BOMBEB:
    return tabiconbombeb;
  case OB_BOMBEC:
    return tabiconbombec;
  case OB_SENSUNIO:
    return tabiconsensunio;
  case OB_SENSUNIE:
    return tabiconsensunie;
  case OB_SENSUNIN:
    return tabiconsensunin;
  case OB_SENSUNIS:
    return tabiconsensunis;
  case OB_VITRENS:
    return tabiconvitrens;
  case OB_VITREEO:
    return tabiconvitreeo;
  case OB_BALLONEX:
    return tabiconballonex;
  case OB_BAISSE:
    return tabiconbaisse;
  case OB_TELE:
    return tabicontele;
  case OB_DETONATEURA:
    return tabicondetonateura;
  case OB_DETONATEURB:
    return tabicondetonateurb;
  case OB_DETONATEURC:
    return tabicondetonateurc;
  case OB_UNSEUL:
    return tabiconunseul;
  case OB_DEPART:
    return tabicondepartob;
  case OB_TROPBU:
    return tabicontropbu;
  }

  return 0;
}


/* ========= */
/* bm_text.c */
/* ========= */

#include <string.h>

#include "blupimania.h"
#include "icon.h"

/* --------------------------- */
/* Variables globales internes */
/* --------------------------- */

static Pixmap pmchar1; /* pixmap des caractères */
static Pixmap pmchar2; /* pixmap des caractères */

static char * pchaine;  /* pointe chaîne éditée */
static short  lgchaine; /* longueur de la chaîne */
static short  lgmax;    /* longueur maximale */
static short  curseur;  /* position du curseur (0..lgchaine) */
static char   ifaccent; /* accent flottant en cours */
static Rect   chrect;   /* position et dimensions */
static char   ifcx;     /* 1 -> curseur "|" présent */
static short  cx;       /* position de curseur "|" en x */
static short  begin;    /* début où afficher [car] */
static short  charsize; /* taille des caractères */

static const char tchasselit[128 - 32] = {
  0x04, 0x04, 0x07, 0x0A, 0x08, 0x0A, 0x0A, 0x04, //
  0x05, 0x05, 0x0A, 0x09, 0x03, 0x07, 0x04, 0x07, //
  0x07, 0x07, 0x08, 0x08, 0x07, 0x08, 0x07, 0x07, //
  0x08, 0x07, 0x05, 0x05, 0x08, 0x07, 0x08, 0x09, //
  0x0A, 0x07, 0x08, 0x08, 0x08, 0x06, 0x06, 0x08, //
  0x08, 0x03, 0x07, 0x08, 0x06, 0x09, 0x08, 0x08, //
  0x07, 0x08, 0x07, 0x07, 0x08, 0x07, 0x08, 0x09, //
  0x08, 0x07, 0x09, 0x03, 0x03, 0x05, 0x03, 0x0A, //
  0x07, 0x07, 0x07, 0x07, 0x07, 0x06, 0x05, 0x07, //
  0x07, 0x03, 0x05, 0x07, 0x03, 0x09, 0x07, 0x07, //
  0x07, 0x07, 0x06, 0x05, 0x05, 0x07, 0x07, 0x0A, //
  0x07, 0x07, 0x06, 0x05, 0x03, 0x09, 0x09, 0x07  //
};

static const char tchassemid[128 - 32] = {
  0x09, 0x08, 0x0D, 0x0B, 0x13, 0x18, 0x18, 0x07, //
  0x0C, 0x0C, 0x0E, 0x0E, 0x08, 0x0E, 0x08, 0x0C, //
  0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, //
  0x11, 0x11, 0x08, 0x08, 0x0E, 0x0E, 0x0E, 0x10, //
  0x16, 0x17, 0x14, 0x15, 0x16, 0x14, 0x13, 0x17, //
  0x18, 0x0C, 0x11, 0x18, 0x14, 0x1A, 0x17, 0x15, //
  0x13, 0x15, 0x16, 0x12, 0x17, 0x17, 0x17, 0x1A, //
  0x18, 0x17, 0x13, 0x07, 0x07, 0x0A, 0x08, 0x11, //
  0x0D, 0x11, 0x13, 0x10, 0x13, 0x11, 0x0E, 0x11, //
  0x15, 0x0B, 0x0A, 0x15, 0x0B, 0x1A, 0x15, 0x12, //
  0x13, 0x13, 0x10, 0x0E, 0x0D, 0x15, 0x12, 0x1A, //
  0x12, 0x12, 0x10, 0x07, 0x0B, 0x19, 0x19, 0x0D  //
};

static const char tchacc[32] = {
  91,  92,  93,  94,        //
  0,   0,   0,   0,   0, 0, //
  'a', 'a', 'a', 'a',       //
  'e', 'e', 'e', 'e',       //
  'i', 'i', 'i', 'i',       //
  'o', 'o', 'o', 'o',       //
  'u', 'u', 'u', 'u',       //
  'c', 'C'                  //
};

/* ------ */
/* LgChar */
/* ------ */

/*
    Retourne la largeur d'un caractère.
 */

short
LgChar (char c)
{
  if (c == 1)
    c = ' '; /* cadratin ? */

  if (c >= 0)
  {
    if (c == '\n')
      c = 127; /* petit triangle ">" */
    if (c < 32)
      return 0;
    if (charsize == TEXTSIZELIT)
      return tchasselit[c - 32];
    else
      return tchassemid[c - 32];
  }
  else
  {
    if (c > KEYAIGU || c < KEYcCEDILLE)
      return 0;
    c = tchacc[-c + KEYAIGU];
    if (c == 0)
      return 0;
    if (charsize == TEXTSIZELIT)
      return tchasselit[c - 32];
    else
      return tchassemid[c - 32];
  }
}

static const char taccent[] = {
  KEYAAIGU, 'a', 91, 2, 6,    //
  KEYAGRAVE, 'a', 92, 2, 4,   //
  KEYACIRCON, 'a', 93, 1, 3,  //
  KEYATREMA, 'a', 94, 2, 4,   //
                              //
  KEYEAIGU, 'e', 91, 2, 6,    //
  KEYEGRAVE, 'e', 92, 2, 5,   //
  KEYECIRCON, 'e', 93, 1, 4,  //
  KEYETREMA, 'e', 94, 2, 5,   //
                              //
  KEYIAIGU, 124, 91, 0, 3,    //
  KEYIGRAVE, 124, 92, 0, 1,   //
  KEYICIRCON, 124, 93, -2, 0, //
  KEYITREMA, 124, 94, 0, 1,   //
                              //
  KEYOAIGU, 'o', 91, 2, 6,    //
  KEYOGRAVE, 'o', 92, 2, 5,   //
  KEYOCIRCON, 'o', 93, 1, 4,  //
  KEYOTREMA, 'o', 94, 2, 5,   //
                              //
  KEYUAIGU, 'u', 91, 2, 8,    //
  KEYUGRAVE, 'u', 92, 2, 6,   //
  KEYUCIRCON, 'u', 93, 1, 5,  //
  KEYUTREMA, 'u', 94, 2, 6,   //

  KEYCCEDILLE, 'c', 123, 1, 5, //
  KEYcCEDILLE, 'C', 123, 2, 7, //
                               //
  0                            //
};

/* -------- */
/* DrawChar */
/* -------- */

/*
    Dessine un caractère et avance la position pour le suivant.
 */

void
DrawChar (Pixmap * ppm, Pt * ppos, char c, Rect * clip)
{
  Pixmap * ppmchar;
  Pixmap   pmchar;
  Pt       src, dst, dim, oDim;
  short    icon = 0;

  if (c < 32)
    c = 32;
  c -= 32;

  dst.x = (*ppos).x;
  dst.y = (*ppos).y - charsize;

  if (charsize == TEXTSIZELIT)
  {
    if (c % 16 < 8)
      ppmchar = &pmchar1;
    else
      ppmchar = &pmchar2;

    src.x = (c % 8) * 10;
    src.y = (c / 16) * (TEXTSIZELIT + 3);

    dim.x = LgChar (c + 32);
    dim.y = TEXTSIZELIT + 3;
  }
  else
  {
    dim.x = LgChar (c + 32);
    dim.y = TEXTSIZEMID + 5;

    if (c >= 0 && c < 'A' - 33)
    {
      icon = c / 3;

      src.x = (c % 3) * 26;
      src.y = 26 * 0;
    }

    if (c >= 'A' - 33 && c < 'a' - 33)
    {
      c -= 'A' - 33;
      icon = c / 3;

      src.x = (c % 3) * 26;
      src.y = 26 * 1;
    }

    if (c >= 'a' - 33)
    {
      c -= 'a' - 33;
      icon = c / 3;

      src.x = (c % 3) * 26;
      src.y = 26 * 2;
    }

    GetSprite (&pmchar, ICO_CHAR_MID + icon);
    ppmchar = &pmchar;
  }

  oDim = dim;

  /* This clipping stuff manages only necessary cases
   * like the one for printing the percentage.
   */
  if (clip)
  {

    if (dst.x < clip->p1.x)
    {
      short diff = clip->p1.x - dst.x;
      dim.x -= diff;
      dst.x = clip->p1.x;
      src.x += diff;
    }
    else if (dst.x + dim.x > clip->p2.x)
      dim.x = clip->p2.x - dst.x;

    if (dst.y < clip->p1.y)
    {
      short diff = clip->p1.y - dst.y;
      dim.y -= diff;
      dst.y = clip->p1.y;
      src.y += diff;
    }
    else if (dst.y + dim.y > clip->p2.y)
      dim.y = clip->p2.y - dst.y;
  }

  CopyPixel (ppmchar, src, ppm, dst, dim);

  (*ppos).x += oDim.x;
}

/* ---------- */
/* DrawAccent */
/* ---------- */

/*
    Dessine un caractère accentué ou autre.
 */

static void
DrawAccent (Pixmap * ppm, Pt * ppos, char c, Rect * clip)
{
  const char * paccent;
  Pt           pnext, pacc;

  if (c < 0) /* lettre accentuée ? */
  {
    paccent = taccent;
    while (*paccent != 0)
    {
      if (paccent[0] == c)
      {
        pnext = *ppos;
        pacc  = *ppos;
        DrawChar (
          ppm, &pnext, paccent[1], clip); /* dessine la lettre sous l'accent */
        if (charsize == TEXTSIZELIT)
          pacc.x += paccent[3];
        else
          pacc.x += paccent[4];
        DrawChar (ppm, &pacc, paccent[2], clip); /* dessine l'accent flottant */
        *ppos = pnext;
        break;
      }
      paccent += 5;
    }
  }
  else
  {
    if (c == '\n')
      c = 127;                     /* petit triangle ">" */
    DrawChar (ppm, ppos, c, clip); /* dessine le caractère */
  }
}

/* ======== */
/* DrawText */
/* ======== */

/*
    Dessine une chaîne de caractères terminée par zéro.
    Retourne la position suivante, sur la ligne de base.
        *ppm		->	pixmap où dessiner (0 = écran)
        pos			->	départ sur la ligne de base
        pstring		->	chaîne terminée par zéro
        size		->	taille des caractères
 */

Pt
DrawString (
  Pixmap * ppm, Pt pos, char * pstring, short size, SDL_bool underline)
{
  char c;
  Pt   start = pos;
  charsize   = size;

  if (size == TEXTSIZELIT)
  {
    GetSprite (&pmchar1, ICO_CHAR_LIT + 0);
    GetSprite (&pmchar2, ICO_CHAR_LIT + 1);
  }

  while ((c = *pstring++, c != 0))
    DrawAccent (ppm, &pos, c, NULL); /* dessine le caractère */

  if (underline)
  {
    start.y += 2;
    pos.y += 2;
    DrawLine (ppm, start, pos, COLORNOIR);
  }

  return pos;
}

/**
 * DrawPercent
 *
 * Draw a percentage with "color" inversion.
 */

Pt
DrawPercent (
  Pixmap * ppm, Pt pos, char * pstring, Rect * clipLeft, Rect * clipRight)
{
  char   c;
  Pt     pOrig = pos;
  char * sOrig = pstring;
  short  leftColor, rightColor;

  charsize = TEXTSIZELIT;

  if (IfColor ())
  {
    leftColor  = ICO_CHAR_LIT;
    rightColor = ICO_CHAR_LITW;
  }
  else
  {
    leftColor  = ICO_CHAR_LITW;
    rightColor = ICO_CHAR_LIT;
  }

  /* Draw in white (on the left) */
  GetSprite (&pmchar1, leftColor + 0);
  GetSprite (&pmchar2, leftColor + 1);

  while ((c = *pstring++, c != 0))
    DrawAccent (ppm, &pos, c, clipLeft); /* draw one char */

  pos     = pOrig;
  pstring = sOrig;

  /* Draw in black (on the right) */
  GetSprite (&pmchar1, rightColor + 0);
  GetSprite (&pmchar2, rightColor + 1);

  while ((c = *pstring++, c != 0))
    DrawAccent (ppm, &pos, c, clipRight); /* draw one char */

  return pos;
}

/* =========== */
/* GetRectText */
/* =========== */

/*
    Retourne le rectangle englobant une chaîne de caractères.
    N'affiche rien du tout.
        pos		->	départ sur la ligne de base
        pstring		->	chaîne terminée par zéro
        size		->	taille des caractères
 */

Rect
GetRectText (Pt pos, char * pstring, short size)
{
  Rect  rect;
  char  c;
  short lg = 0;

  charsize = size;

  while ((c = *pstring++, c != 0))
    lg += LgChar (c);

  rect.p1.x = pos.x;
  rect.p1.y = pos.y - charsize;
  rect.p2.x = pos.x + lg;
  if (size == TEXTSIZELIT)
    rect.p2.y = pos.y + 3;
  else
    rect.p2.y = pos.y + 5;

  return rect;
}

/* ------- */
/* GetWord */
/* ------- */

/*
    Donne les caractéristiques du mot suivant.
    Retourne la longueur à utiliser.
 */

static short
GetWord (const char ** ppnext, char * pword)
{
  char * pt;
  Pt     pos;
  Rect   rect;

  pt = pword;
  while (**ppnext != 0 && **ppnext != '\n' && **ppnext != ' ' &&
         **ppnext != '-')
  {
    *pt++ = *(*ppnext)++;
  }
  if (**ppnext == ' ' || **ppnext == '-')
  {
    *pt++ = *(*ppnext)++;
  }
  *pt = 0;

  pos.x = 0;
  pos.y = 0;
  rect  = GetRectText (pos, pword, charsize);
  return rect.p2.x;
}

/* ============= */
/* DrawParagraph */
/* ============= */

/*
    Dessine un paragraphe de texte dans un rectangle, en drapeau à droite.
 */

void
DrawParagraph (Pixmap * ppm, Rect rect, const char * pstring, short size)
{
  Pt           pos;
  char         word[50];
  const char * pnext;
  short        lg, under;

  charsize = size;

  if (size == TEXTSIZELIT)
    under = 3;
  else
    under = 5;

  pos.y = rect.p1.y + size;
  do
  {
    pos.x = rect.p1.x;
    do
    {
      pnext = pstring;
      lg    = GetWord (&pnext, word);
      if (pos.x + lg <= rect.p2.x)
      {
        DrawString (ppm, pos, word, size, SDL_FALSE); /* affiche un mot */
        pstring = pnext;
      }
      pos.x += lg;
    } while (pos.x <= rect.p2.x && *pstring != 0 && *pstring != '\n');
    if (*pstring == '\n')
      pstring++;
    if (size == TEXTSIZELIT)
      pos.y += size * 2;
    else
      pos.y += size + 10;
  } while (pos.y < rect.p2.y - under && *pstring != 0);

  if (*pstring != 0) /* texte pas entièrement affiché ? */
  {
    pos.x = rect.p2.x - LgChar (126);
    pos.y = rect.p2.y - under;
    DrawChar (ppm, &pos, 126, NULL); /* affiche petit triangle v */
  }
}

/* ---------- */
/* CalcJustif */
/* ---------- */

/*
    Effectue quelques calculs pour afficher une ligne en édition.
 */

static void
CalcJustif (void)
{
  short dim; /* largeur pour la ligne */
  short i;   /* offset temporaire */
  short length;

  dim = chrect.p2.x - chrect.p1.x - 10;
  cx  = chrect.p1.x; /* curseur "|" au début */

  /* Cherche dans la chaîne le premier caractère à afficher. */

  i      = curseur;
  length = 0;
  while (i < lgchaine && length < dim / 2)
  {
    length += LgChar (pchaine[i]);
    i++; /* avance de dim/2 */
  }
  if (length > dim / 2)
    i--;

  length = 0;
  while (i > 0 && length < dim - 1)
  {
    i--; /* recule de dim-1 */
    length += LgChar (pchaine[i]);
  }
  if (length > dim - 1)
    i++;
  begin = i;

  /* Cherche le nombre de caractères et la longueur à afficher. */

  length = 0;
  while (i < lgchaine && length < dim)
  {
    if (i == curseur)
      cx = chrect.p1.x + length; /* curseur "|" ici */
    length += LgChar (pchaine[i]);
    i++;
  }
  if (length > dim)
  {
    length -= LgChar (pchaine[i - 1]);
    i--;
  }
  if (i == curseur)
    cx = chrect.p1.x + length; /* curseur "|" ici */
}

/* ------- */
/* EditAff */
/* ------- */

/*
    Affiche un texte pendant l'édition d'une ligne.
 */

static void
EditAff (void)
{
  short i;
  Pt    pos;
  Rect  rect;

  GetSprite (&pmchar1, ICO_CHAR_LIT + 0);
  GetSprite (&pmchar2, ICO_CHAR_LIT + 1);

  pos.x = chrect.p1.x;
  pos.y = chrect.p1.y + (chrect.p2.y - chrect.p1.y) / 2 + charsize / 2 - 1;

  DrawFillRect (0, chrect, COLORBLANC); /* efface la ligne */

  for (i = begin; i < lgchaine; i++)
  {
    if (pos.x >= chrect.p2.x - 10)
      break;
    DrawAccent (0, &pos, pchaine[i], NULL); /* affiche un caractère */
  }

  if (begin > 0)
  {
    pos.x     = chrect.p1.x;
    rect      = chrect;
    rect.p2.x = rect.p1.x + LgChar (127);
    DrawFillRect (0, rect, COLORBLANC);
    DrawChar (0, &pos, 96, NULL); /* met le triangle < */
  }

  if (pchaine[i] != 0)
  {
    pos.x = chrect.p2.x - LgChar (127);
    DrawChar (0, &pos, 127, NULL); /* met le triangle > */
  }
}

/* ------- */
/* InsChar */
/* ------- */

/*
    Insère un caractère dans la ligne.
 */

static short
InsChar (char car)
{
  char *s, *d;
  short i;

  if (lgchaine >= lgmax - 1)
    return -1; /* chaîne pleine */

  s = pchaine + lgchaine;
  d = pchaine + lgchaine + 1;
  for (i = curseur; i < lgchaine + 1; i++)
    *d-- = *s--; /* creuse le trou */

  *(pchaine + curseur) = car; /* met le caractère dans la chaîne */
  lgchaine++;
  curseur++; /* avance le curseur */

  return 0;
}

/* ------- */
/* DelChar */
/* ------- */

/*
    Détruit un caractère dans la ligne.
 */

static short
DelChar (void)
{
  char *s, *d;
  short i;

  if (lgchaine == 0)
    return -1; /* chaîne vide */
  if (curseur == 0)
    return -1; /* curseur au début */

  s = pchaine + curseur;
  d = pchaine + curseur - 1;
  for (i = curseur; i < lgchaine + 1; i++)
    *d++ = *s++; /* bouche le trou */

  lgchaine--;
  curseur--; /* recule le curseur */

  return 0;
}

/* ----------- */
/* AccentFirst */
/* ----------- */

/*
    Cherche si la touche est un accent flottant.
    Si oui, retourne l'accent. Si non, retourne zéro.
 */

static short
AccentFirst (char key)
{
  short i = 0;

  static char table[] = {
    KEYAIGU,   91, //
    KEYGRAVE,  92, //
    KEYCIRCON, 93, //
    KEYTREMA,  94, //
    0              //
  };

  while (table[i] != 0)
  {
    if (table[i] == key)
      return table[i + 1];
    i += 2;
  }

  return 0;
}

/* ----------- */
/* AccentUnder */
/* ----------- */

/*
    Cherche si une touche peut se placer sous un accent.
    Si oui, retourne le nouveau caractère. Si non, retourne zéro.
 */

static char
AccentUnder (char key, char accent)
{
  short i = 0;

  static char table[] = {
    91, 'a', KEYAAIGU,   //
    92, 'a', KEYAGRAVE,  //
    93, 'a', KEYACIRCON, //
    94, 'a', KEYATREMA,  //
    91, 'e', KEYEAIGU,   //
    92, 'e', KEYEGRAVE,  //
    93, 'e', KEYECIRCON, //
    94, 'e', KEYETREMA,  //
    91, 'i', KEYIAIGU,   //
    92, 'i', KEYIGRAVE,  //
    93, 'i', KEYICIRCON, //
    94, 'i', KEYITREMA,  //
    91, 'o', KEYOAIGU,   //
    92, 'o', KEYOGRAVE,  //
    93, 'o', KEYOCIRCON, //
    94, 'o', KEYOTREMA,  //
    91, 'u', KEYUAIGU,   //
    92, 'u', KEYUGRAVE,  //
    93, 'u', KEYUCIRCON, //
    94, 'u', KEYUTREMA,  //
    0                    //
  };

  while (table[i] != 0)
  {
    if (accent == table[i] && key == table[i + 1])
      return table[i + 2];
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

static void
InvCurs (short color)
{
  Pt p1, p2;

  if (cx == 0)
    return;

  p1.x = cx - 1;
  p1.y = chrect.p1.y + 2;
  p2.x = cx - 1;
  p2.y = chrect.p2.y - 2;

  DrawLine (0, p1, p2, color); /* inverse la droite verticale */
}

/* ------- */
/* ClrCurs */
/* ------- */

/*
    Enlève le curseur "|" si nécessaire.
 */

static void
ClrCurs (void)
{
  if (ifcx == 0)
    return;

  InvCurs (COLORBLANC); /* efface le curseur */
  ifcx = 0;
}

/* ------- */
/* SetCurs */
/* ------- */

/*
    Met le curseur "|" si nécessaire.
 */

static void
SetCurs (void)
{
  if (ifcx != 0)
    return;

  InvCurs (COLORNOIR); /* allume le curseur */
  ifcx = 1;
}

/* -------- */
/* EditDraw */
/* -------- */

/*
    Affiche la chaîne éditée.
 */

static void
EditDraw (void)
{
  if (pchaine == 0)
    return;

  ClrCurs ();    /* enlève le curseur "|" */
  CalcJustif (); /* fait qq calculs */
  EditAff ();    /* affiche la ligne */
  SetCurs ();    /* met le curseur "|" */
}

/* ========= */
/* EditEvent */
/* ========= */

/*
    Modifie la chaîne en édition selon un événement clavier/souris.
    Retourne -1 en cas d'erreur.
    Retourne 1 si l'événement ne modifie pas le contenu.
    Retourne 0 si l'événement modifie le contenu.
 */

short
EditEvent (short key)
{
  short err = -1;

  if (pchaine == 0)
    return 1;

  if (key == 0)
    return -1;

  if (key == KEYLEFT)
  {
    if (curseur > 0)
    {
      curseur--;
      err = 1;
    }
  }

  if (key == KEYRIGHT)
  {
    if (curseur < lgchaine)
    {
      curseur++;
      err = 1;
    }
  }

  if (key == KEYUP)
  {
    curseur = 0;
    err     = 1;
  }

  if (key == KEYDOWN)
  {
    curseur = lgchaine;
    err     = 1;
  }

  if (key == KEYDEL) /* touche DEL normale */
  {
    err = DelChar (); /* détruit un caractère */
  }

  if (ifaccent) /* accent flottant en cours ? */
  {
    ifaccent = AccentUnder (key, ifaccent);
    if (ifaccent)
    {
      DelChar ();     /* supprime l'accent flottant */
      key = ifaccent; /* remplace par lettre accentuée */
      goto ins;
    }
  }
  ifaccent = AccentFirst (key);
  if (ifaccent)
  {
    key = ifaccent; /* insère l'accent flottant */
  }

ins:
  if (key == KEYRETURN)
    key = '\n';

  if (
    (key >= 32 && key <= 127) || (key <= KEYAAIGU && key >= KEYcCEDILLE) ||
    key == '\n')
    err = InsChar ((char) key); /* insère le caractère frappé */

  if (err >= 0)
    EditDraw (); /* affiche la ligne */

  return err;
}

/* ======== */
/* EditOpen */
/* ======== */

/*
    Initialise la chaîne à éditer, et place le curseur à la fin.
 */

short
EditOpen (char * p, short max, Rect rect)
{
  pchaine  = p;
  lgchaine = strlen (p);
  lgmax    = max;
  chrect   = rect;
  ifaccent = 0;
  ifcx     = 0;
  charsize = TEXTSIZELIT;

  curseur = lgchaine;

  EditDraw (); /* affiche la chaîne */
  return 0;
}

/* ========= */
/* EditClose */
/* ========= */

/*
    Termine la chaîne à éditer, et efface le curseur.
 */

short
EditClose (void)
{
  if (pchaine == 0)
    return 1;

  curseur = 0; /* met le curseur au début */

  EditDraw (); /* affiche la chaîne */
  ClrCurs ();  /* enlève le curseur "|" */

  pchaine = 0; /* y'a plus de chaîne en édition */
  return 0;
}

// Copyright (C) 2000-2003, Luca Padovani <luca.padovani@cs.unibo.it>.
//
// This file is part of GtkMathView, a Gtk widget for MathML.
// 
// GtkMathView is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// GtkMathView is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GtkMathView; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// For details, see the GtkMathView World-Wide-Web page,
// http://helm.cs.unibo.it/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>

#include <cassert>

#include <gdk/gdkx.h>
#include <pango/pangox.h>

#include "Gtk_AdobeShaper.hh"
#include "Gtk_AreaFactory.hh"
#include "Gtk_RenderingContext.hh"
#include "MathVariant.hh"
#include "MathVariantMap.hh"
#include "ShaperManager.hh"
#include "MathGraphicDevice.hh"
#include "MathMLElement.hh"

static struct {
  guint8 index;
  DOM::Char16 ch;
} symbolMap[] = {
  { 0x20, 0x0020 },  // SPACE // space
  { 0x21, 0x0021 },  // EXCLAMATION MARK  // exclam
  { 0x22, 0x2200 },  // FOR ALL // universal
  { 0x23, 0x0023 },  // NUMBER SIGN // numbersign
  { 0x24, 0x2203 },  // THERE EXISTS  // existential
  { 0x25, 0x0025 },  // PERCENT SIGN  // percent
  { 0x26, 0x0026 },  // AMPERSAND // ampersand
  { 0x27, 0x220B },  // CONTAINS AS MEMBER  // suchthat
  { 0x28, 0x0028 },  // LEFT PARENTHESIS  // parenleft
  { 0x29, 0x0029 },  // RIGHT PARENTHESIS // parenright
  { 0x2A, 0x2217 },  // ASTERISK OPERATOR // asteriskmath
  { 0x2B, 0x002B },  // PLUS SIGN // plus
  { 0x2C, 0x002C },  // COMMA // comma
  { 0x2D, 0x2212 },  // MINUS SIGN  // minus
  { 0x2E, 0x002E },  // FULL STOP // period
  { 0x2F, 0x002F },  // SOLIDUS // slash

  { 0x30, 0x0030 },  // DIGIT ZERO  // zero
  { 0x31, 0x0031 },  // DIGIT ONE // one
  { 0x32, 0x0032 },  // DIGIT TWO // two
  { 0x33, 0x0033 },  // DIGIT THREE // three
  { 0x34, 0x0034 },  // DIGIT FOUR  // four
  { 0x35, 0x0035 },  // DIGIT FIVE  // five
  { 0x36, 0x0036 },  // DIGIT SIX // six
  { 0x37, 0x0037 },  // DIGIT SEVEN // seven
  { 0x38, 0x0038 },  // DIGIT EIGHT // eight
  { 0x39, 0x0039 },  // DIGIT NINE  // nine
  { 0x3A, 0x003A },  // COLON // colon
  { 0x3B, 0x003B },  // SEMICOLON // semicolon
  { 0x3C, 0x003C },  // LESS-THAN SIGN  // less
  { 0x3D, 0x003D },  // EQUALS SIGN // equal
  { 0x3E, 0x003E },  // GREATER-THAN SIGN // greater
  { 0x3F, 0x003F },  // QUESTION MARK // question

  { 0x40, 0x2245 },  // APPROXIMATELY EQUAL TO  // congruent
  { 0x41, 0x0391 },  // GREEK CAPITAL LETTER ALPHA  // Alpha
  { 0x42, 0x0392 },  // GREEK CAPITAL LETTER BETA // Beta
  { 0x43, 0x03A7 },  // GREEK CAPITAL LETTER CHI  // Chi
  { 0x44, 0x0394 },  // GREEK CAPITAL LETTER DELTA  // Delta
  { 0x44, 0x2206 },  // INCREMENT // Delta
  { 0x45, 0x0395 },  // GREEK CAPITAL LETTER EPSILON  // Epsilon
  { 0x46, 0x03A6 },  // GREEK CAPITAL LETTER PHI  // Phi
  { 0x47, 0x0393 },  // GREEK CAPITAL LETTER GAMMA  // Gamma
  { 0x48, 0x0397 },  // GREEK CAPITAL LETTER ETA  // Eta
  { 0x49, 0x0399 },  // GREEK CAPITAL LETTER IOTA // Iota
  { 0x4A, 0x03D1 },  // GREEK THETA SYMBOL  // theta1
  { 0x4B, 0x039A },  // GREEK CAPITAL LETTER KAPPA  // Kappa
  { 0x4C, 0x039B },  // GREEK CAPITAL LETTER LAMDA  // Lambda
  { 0x4D, 0x039C },  // GREEK CAPITAL LETTER MU // Mu
  { 0x4E, 0x039D },  // GREEK CAPITAL LETTER NU // Nu
  { 0x4F, 0x039F },  // GREEK CAPITAL LETTER OMICRON  // Omicron

  { 0x50, 0x03A0 },  // GREEK CAPITAL LETTER PI // Pi
  { 0x51, 0x0398 },  // GREEK CAPITAL LETTER THETA  // Theta
  { 0x52, 0x03A1 },  // GREEK CAPITAL LETTER RHO  // Rho
  { 0x53, 0x03A3 },  // GREEK CAPITAL LETTER SIGMA  // Sigma
  { 0x54, 0x03A4 },  // GREEK CAPITAL LETTER TAU  // Tau
  { 0x55, 0x03A5 },  // GREEK CAPITAL LETTER UPSILON  // Upsilon
  { 0x56, 0x03C2 },  // GREEK SMALL LETTER FINAL SIGMA  // sigma1
  { 0x57, 0x03A9 },  // GREEK CAPITAL LETTER OMEGA  // Omega
  { 0x57, 0x2126 },  // OHM SIGN  // Omega
  { 0x58, 0x039E },  // GREEK CAPITAL LETTER XI // Xi
  { 0x59, 0x03A8 },  // GREEK CAPITAL LETTER PSI  // Psi
  { 0x5A, 0x0396 },  // GREEK CAPITAL LETTER ZETA // Zeta
  { 0x5B, 0x005B },  // LEFT SQUARE BRACKET // bracketleft
  { 0x5C, 0x2234 },  // THEREFORE // therefore
  { 0x5D, 0x005D },  // RIGHT SQUARE BRACKET  // bracketright
  { 0x5E, 0x22A5 },  // UP TACK // perpendicular
  { 0x5F, 0x005F },  // LOW LINE  // underscore

  { 0x60, 0xF8E5 },  // RADICAL EXTENDER  // radicalex (CUS)
  { 0x61, 0x03B1 },  // GREEK SMALL LETTER ALPHA  // alpha
  { 0x62, 0x03B2 },  // GREEK SMALL LETTER BETA // beta
  { 0x63, 0x03C7 },  // GREEK SMALL LETTER CHI  // chi
  { 0x64, 0x03B4 },  // GREEK SMALL LETTER DELTA  // delta
  { 0x65, 0x03B5 },  // GREEK SMALL LETTER EPSILON  // epsilon
  { 0x66, 0x03C6 },  // GREEK SMALL LETTER PHI  // phi
  { 0x67, 0x03B3 },  // GREEK SMALL LETTER GAMMA  // gamma
  { 0x68, 0x03B7 },  // GREEK SMALL LETTER ETA  // eta
  { 0x69, 0x03B9 },  // GREEK SMALL LETTER IOTA // iota
  { 0x6A, 0x03D5 },  // GREEK PHI SYMBOL  // phi1
  { 0x6B, 0x03BA },  // GREEK SMALL LETTER KAPPA  // kappa
  { 0x6C, 0x03BB },  // GREEK SMALL LETTER LAMDA  // lambda
  { 0x6D, 0x00B5 },  // MICRO SIGN  // mu
  { 0x6D, 0x03BC },  // GREEK SMALL LETTER MU // mu
  { 0x6E, 0x03BD },  // GREEK SMALL LETTER NU // nu
  { 0x6F, 0x03BF },  // GREEK SMALL LETTER OMICRON  // omicron

  { 0x70, 0x03C0 },  // GREEK SMALL LETTER PI // pi
  { 0x71, 0x03B8 },  // GREEK SMALL LETTER THETA  // theta
  { 0x72, 0x03C1 },  // GREEK SMALL LETTER RHO  // rho
  { 0x73, 0x03C3 },  // GREEK SMALL LETTER SIGMA  // sigma
  { 0x74, 0x03C4 },  // GREEK SMALL LETTER TAU  // tau
  { 0x75, 0x03C5 },  // GREEK SMALL LETTER UPSILON  // upsilon
  { 0x76, 0x03D6 },  // GREEK PI SYMBOL // omega1
  { 0x77, 0x03C9 },  // GREEK SMALL LETTER OMEGA  // omega
  { 0x78, 0x03BE },  // GREEK SMALL LETTER XI // xi
  { 0x79, 0x03C8 },  // GREEK SMALL LETTER PSI  // psi
  { 0x7A, 0x03B6 },  // GREEK SMALL LETTER ZETA // zeta
  { 0x7B, 0x007B },  // LEFT CURLY BRACKET  // braceleft
  { 0x7C, 0x007C },  // VERTICAL LINE // bar
  { 0x7D, 0x007D },  // RIGHT CURLY BRACKET // braceright
  { 0x7E, 0x223C },  // TILDE OPERATOR  // similar

  { 0xA0, 0x20AC },  // EURO SIGN // Euro
  { 0xA1, 0x03D2 },  // GREEK UPSILON WITH HOOK SYMBOL  // Upsilon1
  { 0xA2, 0x2032 },  // PRIME // minute
  { 0xA3, 0x2264 },  // LESS-THAN OR EQUAL TO // lessequal
  { 0xA4, 0x2044 },  // FRACTION SLASH  // fraction
  { 0xA4, 0x2215 },  // DIVISION SLASH  // fraction
  { 0xA5, 0x221E },  // INFINITY  // infinity
  { 0xA6, 0x0192 },  // LATIN SMALL LETTER F WITH HOOK  // florin
  { 0xA7, 0x2663 },  // BLACK CLUB SUIT // club
  { 0xA8, 0x2666 },  // BLACK DIAMOND SUIT  // diamond
  { 0xA9, 0x2665 },  // BLACK HEART SUIT  // heart
  { 0xAA, 0x2660 },  // BLACK SPADE SUIT  // spade
  { 0xAB, 0x2194 },  // LEFT RIGHT ARROW  // arrowboth
  { 0xAC, 0x2190 },  // LEFTWARDS ARROW // arrowleft
  { 0xAD, 0x2191 },  // UPWARDS ARROW // arrowup
  { 0xAE, 0x2192 },  // RIGHTWARDS ARROW  // arrowright
  { 0xAF, 0x2193 },  // DOWNWARDS ARROW // arrowdown

  { 0xB0, 0x00B0 },  // DEGREE SIGN // degree
  { 0xB1, 0x00B1 },  // PLUS-MINUS SIGN // plusminus
  { 0xB2, 0x2033 },  // DOUBLE PRIME  // second
  { 0xB3, 0x2265 },  // GREATER-THAN OR EQUAL TO  // greaterequal
  { 0xB4, 0x00D7 },  // MULTIPLICATION SIGN // multiply
  { 0xB5, 0x221D },  // PROPORTIONAL TO // proportional
  { 0xB6, 0x2202 },  // PARTIAL DIFFERENTIAL  // partialdiff
  { 0xB7, 0x2022 },  // BULLET  // bullet
  { 0xB8, 0x00F7 },  // DIVISION SIGN // divide
  { 0xB9, 0x2260 },  // NOT EQUAL TO  // notequal
  { 0xBA, 0x2261 },  // IDENTICAL TO  // equivalence
  { 0xBB, 0x2248 },  // ALMOST EQUAL TO // approxequal
  { 0xBC, 0x2026 },  // HORIZONTAL ELLIPSIS // ellipsis
  { 0xBD, 0xF8E6 },  // VERTICAL ARROW EXTENDER // arrowvertex (CUS)
  { 0xBE, 0xF8E7 },  // HORIZONTAL ARROW EXTENDER // arrowhorizex (CUS)
  { 0xBF, 0x21B5 },  // DOWNWARDS ARROW WITH CORNER LEFTWARDS // carriagereturn

  { 0xC0, 0x2135 },  // ALEF SYMBOL // aleph
  { 0xC1, 0x2111 },  // BLACK-LETTER CAPITAL I  // Ifraktur
  { 0xC2, 0x211C },  // BLACK-LETTER CAPITAL R  // Rfraktur
  { 0xC3, 0x2118 },  // SCRIPT CAPITAL P  // weierstrass
  { 0xC4, 0x2297 },  // CIRCLED TIMES // circlemultiply
  { 0xC5, 0x2295 },  // CIRCLED PLUS  // circleplus
  { 0xC6, 0x2205 },  // EMPTY SET // emptyset
  { 0xC7, 0x2229 },  // INTERSECTION  // intersection
  { 0xC8, 0x222A },  // UNION // union
  { 0xC9, 0x2283 },  // SUPERSET OF // propersuperset
  { 0xCA, 0x2287 },  // SUPERSET OF OR EQUAL TO // reflexsuperset
  { 0xCB, 0x2284 },  // NOT A SUBSET OF // notsubset
  { 0xCC, 0x2282 },  // SUBSET OF // propersubset
  { 0xCD, 0x2286 },  // SUBSET OF OR EQUAL TO // reflexsubset
  { 0xCE, 0x2208 },  // ELEMENT OF  // element
  { 0xCF, 0x2209 },  // NOT AN ELEMENT OF // notelement

  { 0xD0, 0x2220 },  // ANGLE // angle
  { 0xD1, 0x2207 },  // NABLA // gradient
  { 0xD2, 0x00AE },  // REGISTERED SIGN SERIF // registerserif (CUS) CHANGED
  { 0xD3, 0x00A9 },  // COPYRIGHT SIGN SERIF  // copyrightserif (CUS) CHANGED
  { 0xD4, 0x2122 },  // TRADE MARK SIGN SERIF // trademarkserif (CUS) CHANGED
  { 0xD5, 0x220F },  // N-ARY PRODUCT // product
  { 0xD6, 0x221A },  // SQUARE ROOT // radical
  { 0xD7, 0x22C5 },  // DOT OPERATOR  // dotmath
  { 0xD8, 0x00AC },  // NOT SIGN  // logicalnot
  { 0xD9, 0x2227 },  // LOGICAL AND // logicaland
  { 0xDA, 0x2228 },  // LOGICAL OR  // logicalor
  { 0xDB, 0x21D4 },  // LEFT RIGHT DOUBLE ARROW // arrowdblboth
  { 0xDC, 0x21D0 },  // LEFTWARDS DOUBLE ARROW  // arrowdblleft
  { 0xDD, 0x21D1 },  // UPWARDS DOUBLE ARROW  // arrowdblup
  { 0xDE, 0x21D2 },  // RIGHTWARDS DOUBLE ARROW // arrowdblright
  { 0xDF, 0x21D3 },  // DOWNWARDS DOUBLE ARROW  // arrowdbldown

  { 0xE0, 0x25CA },  // LOZENGE // lozenge
  { 0xE1, 0x2329 },  // LEFT-POINTING ANGLE BRACKET // angleleft
  { 0xE2, 0xF8E8 },  // REGISTERED SIGN SANS SERIF  // registersans (CUS)
  { 0xE3, 0xF8E9 },  // COPYRIGHT SIGN SANS SERIF // copyrightsans (CUS)
  { 0xE4, 0xF8EA },  // TRADE MARK SIGN SANS SERIF  // trademarksans (CUS)
  { 0xE5, 0x2211 },  // N-ARY SUMMATION // summation

  { 0xF1, 0x232A },  // RIGHT-POINTING ANGLE BRACKET  // angleright
  { 0xF2, 0x222B },  // INTEGRAL  // integral
  { 0xF3, 0x2320 },  // TOP HALF INTEGRAL // integraltp
  { 0xF4, 0xF8F5 },  // INTEGRAL EXTENDER // integralex (CUS)
  { 0xF5, 0x2321 },  // BOTTOM HALF INTEGRAL  // integralbt
  { 0xF6, 0xF8F6 },  // RIGHT PAREN TOP // parenrighttp (CUS)
  { 0xF7, 0xF8F7 },  // RIGHT PAREN EXTENDER  // parenrightex (CUS)
  { 0xF8, 0xF8F8 },  // RIGHT PAREN BOTTOM  // parenrightbt (CUS)
  { 0xF9, 0xF8F9 },  // RIGHT SQUARE BRACKET TOP  // bracketrighttp (CUS)
  { 0xFA, 0xF8FA },  // RIGHT SQUARE BRACKET EXTENDER // bracketrightex (CUS)
  { 0xFB, 0xF8FB },  // RIGHT SQUARE BRACKET BOTTOM // bracketrightbt (CUS)
  { 0xFC, 0xF8FC },  // RIGHT CURLY BRACKET TOP // bracerighttp (CUS)
  { 0xFD, 0xF8FD },  // RIGHT CURLY BRACKET MID // bracerightmid (CUS)
  { 0xFE, 0xF8FE },  // RIGHT CURLY BRACKET BOTTOM // bracerightbt (CUS)

  { 0x00, 0x0000 }
};

struct HStretchyChar
{
  DOM::Char16 ch;
  guint8 normal;
  guint8 left;
  guint8 glue;
  guint8 right;
};

struct VStretchyChar
{
  DOM::Char16 ch;
  guint8 normal;
  guint8 top;
  guint8 glue;
  guint8 middle;
  guint8 bottom;
};

static HStretchyChar hMap[] =
  {
    //        N     L     G     R
    { 0x2190, 0xAC, 0xAC, 0xBE, 0    },
    { 0x2192, 0xAE, 0,    0xBE, 0xAE },
    { 0x2194, 0xAB, 0xAC, 0xBE, 0xAE },
    { 0,      0,    0,    0,    0    }
  };

static VStretchyChar vMap[] =
  {
    //        N     T     G     M     B
    { 0x0028, 0x28, 0xE6, 0xE7, 0,    0xE8 },
    { 0x0029, 0x29, 0xF6, 0xF7, 0,    0xF8 },
    { 0x005B, 0x5B, 0xE9, 0xEA, 0,    0xEB },
    { 0x005D, 0x5D, 0xF9, 0xFA, 0,    0xFB },
    { 0x007B, 0x7B, 0xEC, 0xEF, 0xED, 0xEE },
    { 0x007C, 0x7C, 0,    0x7C, 0,    0    },
    { 0x007D, 0x7D, 0xFC, 0xEF, 0xFD, 0xFE },
    { 0x2191, 0xAD, 0xAD, 0xBD, 0,    0    },
    { 0x2193, 0xAF, 0,    0xBD, 0,    0xAF },
    { 0x2195, 0,    0xAD, 0xBD, 0,    0xAF },
    { 0x222B, 0xF2, 0xF3, 0xF4, 0,    0xF5 },
    { 0,      0,    0,    0,    0,    0    }
  };

struct XFontDesc
{
  MathVariant variant;
  char* family;
  char* weight;
  char* slant;
  char* charset;
};

#define SYMBOL_INDEX 0
#define LATIN_BASE_INDEX 1

#define H_STRETCHY_BIT   0x100
#define V_STRETCHY_BIT   0x200
#define GLYPH_INDEX_MASK 0x0ff

static XFontDesc variantDesc[Gtk_AdobeShaper::N_FONTS] =
  {
    { NORMAL_VARIANT, "symbol", "medium", "r", "adobe-fontspecific" },
    { NORMAL_VARIANT, "times", "medium", "r", "iso8859-1" },
    { BOLD_VARIANT, "times", "bold", "r", "iso8859-1" },
    { ITALIC_VARIANT, "times", "medium", "i", "iso8859-1" },
    { BOLD_ITALIC_VARIANT, "times", "bold", "i", "iso8859-1" },
    { SANS_SERIF_VARIANT, "helvetica", "medium", "r", "iso8859-1" },
    { BOLD_SANS_SERIF_VARIANT, "helvetica", "bold", "r", "iso8859-1" },
    { SANS_SERIF_ITALIC_VARIANT, "helvetica", "medium", "i", "iso8859-1" },
    { SANS_SERIF_BOLD_ITALIC_VARIANT, "helvetica", "bold", "i", "iso8859-1" },
    { MONOSPACE_VARIANT, "courier", "medium", "r", "iso8859-1" }
  };

void
Gtk_AdobeShaper::registerShaper(const SmartPtr<ShaperManager>& sm, unsigned shaperId)
{
  assert(sm);

  for (unsigned i = LATIN_BASE_INDEX; i < N_FONTS; i++)
    {
      for (DOM::Char16 ch = 0x20; ch < 0x100; ch++)
	{
	  DOM::Char32 vch = mapMathVariant(variantDesc[i].variant, ch);
	  if (variantDesc[i].variant == NORMAL_VARIANT || vch != ch)
	    sm->registerChar(vch, GlyphSpec(shaperId, i, ch));
	}
    }

  for (unsigned i = 0; symbolMap[i].ch; i++)
    sm->registerChar(symbolMap[i].ch, GlyphSpec(shaperId, SYMBOL_INDEX, symbolMap[i].index));

  for (unsigned i = 0; vMap[i].ch != 0; i++)
    sm->registerStretchyChar(vMap[i].ch, GlyphSpec(shaperId, SYMBOL_INDEX, i | V_STRETCHY_BIT));

  for (unsigned i = 0; hMap[i].ch != 0; i++)
    sm->registerStretchyChar(hMap[i].ch, GlyphSpec(shaperId, SYMBOL_INDEX, i | H_STRETCHY_BIT));
}

void
Gtk_AdobeShaper::unregisterShaper(const SmartPtr<ShaperManager>&, unsigned)
{
  // nothing to do???
}

void
Gtk_AdobeShaper::shape(const MathFormattingContext& ctxt, ShapingResult& result) const
{
  for (unsigned n = result.chunkSize(); n > 0; n--)
    {
      AreaRef res;
      GlyphSpec spec = result.getSpec();

      if (spec.getGlyphId() & H_STRETCHY_BIT)
	res = shapeStretchyCharH(ctxt, spec, result.getHSpan());
      else if (spec.getGlyphId() & V_STRETCHY_BIT)
	res = shapeStretchyCharV(ctxt, spec, result.getVSpan());

      // If we get here then either the character was not required
      // to stretch, or one of the stretchying methods has failed,
      // hence we shape it with no stretchying
      if (!res) res = shapeChar(ctxt, spec);
      if (!res) break;

      result.pushArea(1, res);
    }
}

const char*
Gtk_AdobeShaper::getXLFD(unsigned fi, const scaled& size)
{
  assert(fi < N_FONTS);

  static char buffer[128];
  sprintf(buffer, "-adobe-%s-%s-%s-*--*-%d-75-75-*-*-%s",
	  variantDesc[fi].family, variantDesc[fi].weight, variantDesc[fi].slant,
	  static_cast<int>(size.toFloat() * 10 + 0.5f), variantDesc[fi].charset);
 
  return buffer;
}

PangoFont*
Gtk_AdobeShaper::getPangoFont(unsigned fi, const scaled& size, PangoXSubfont& subfont) const
{
  assert(fi < N_FONTS);
  CachedFontKey key(size);
  PangoFontCache::iterator p = pangoFontCache[fi].find(key);
  if (p != pangoFontCache[fi].end())
    {
      subfont = p->second.subfont;
      return p->second.font;
    }

  // Note that we use the default values for the display
  // that is the value that was specified to the
  // X server on the command line. This will work on most cases
  PangoFont* font = pango_x_load_font(gdk_x11_get_default_xdisplay(), getXLFD(fi, size));
  assert(font);

  // the following operations are needed even if the subfont is
  // always 1, dunno why :-(((
  PangoXSubfont* sf;
  int* subfont_charset;
  int n_subfonts;
  n_subfonts = pango_x_list_subfonts(font, &variantDesc[fi].charset, 1, &sf, &subfont_charset);
  assert(n_subfonts > 0);
  subfont = sf[0];
  g_free(sf);
  g_free(subfont_charset);
#if 0
  printf("found %d subfonts\n", n_subfonts);
  for (unsigned i = 0; i < n_subfonts; i++)
    printf("subfont: %d\n", subfont[i]);
#endif
  
  pangoFontCache[fi][key] = CachedPangoFontData(font, subfont);

  return font;
}

XftFont*
Gtk_AdobeShaper::getXftFont(unsigned fi, const scaled& size) const
{
  XftFont* font = XftFontOpenXlfd(gdk_x11_get_default_xdisplay(),
		                  gdk_x11_get_default_screen(),
				  getXLFD(fi, size));
  assert(font);

  return font;
}

AreaRef
Gtk_AdobeShaper::createPangoGlyphArea(const SmartPtr<Gtk_AreaFactory>& factory,
				      unsigned fi, unsigned gi,
				      const scaled& size) const
{
  PangoXSubfont subfont;
  PangoFont* font = getPangoFont(fi, size, subfont);
  assert(font);

  PangoGlyphString* gs = pango_glyph_string_new();
  pango_glyph_string_set_size(gs, 1);
  gs->glyphs[0].glyph = PANGO_X_MAKE_GLYPH(subfont, gi);
  gs->glyphs[0].geometry.x_offset = 0;
  gs->glyphs[0].geometry.y_offset = 0;
  gs->glyphs[0].geometry.width = 0;

  return factory->pangoGlyph(font, gs);
}

AreaRef
Gtk_AdobeShaper::createXftGlyphArea(const SmartPtr<Gtk_AreaFactory>& factory,
				    unsigned fi, unsigned gi,
				    const scaled& size) const
{
  XftFont* font = getXftFont(fi, size);
  assert(font);
  return factory->xftGlyph(font, gi);
}

AreaRef
Gtk_AdobeShaper::getGlyphArea(const SmartPtr<Gtk_AreaFactory>& factory,
			      unsigned fi, unsigned gi,
			      const scaled& size) const
{
  assert(fi < N_FONTS);
  CachedAreaKey key(gi, size);
  AreaCache::iterator p = areaCache[fi].find(key);
  if (p != areaCache[fi].end())
    return p->second;

  AreaRef res = createPangoGlyphArea(factory, fi, gi, size);
  // AreaRef res = createXftGlyphArea(factory, fi, gi, size);
  assert(res);
  areaCache[fi][key] = res;

  return res;
}

AreaRef
Gtk_AdobeShaper::shapeChar(const MathFormattingContext& ctxt, const GlyphSpec& spec) const
{
  SmartPtr<Gtk_AreaFactory> factory = smart_cast<Gtk_AreaFactory>(ctxt.getDevice()->getFactory());
  assert(factory);
  return getGlyphArea(factory, spec.getFontId(), spec.getGlyphId() & GLYPH_INDEX_MASK, ctxt.getSize());
}

void
Gtk_AdobeShaper::getGlyphExtents(PangoFont* font, PangoGlyphString* gs, PangoRectangle* rect) const
{
  assert(font);
  assert(gs);
  assert(rect);
  pango_glyph_string_extents(gs, font, rect, NULL);
}

AreaRef
Gtk_AdobeShaper::shapeStretchyCharH(const MathFormattingContext& ctxt, const GlyphSpec& spec, const scaled& span) const
{
  SmartPtr<Gtk_AreaFactory> factory = smart_cast<Gtk_AreaFactory>(ctxt.getDevice()->getFactory());
  assert(factory);

  const scaled size = ctxt.getSize();

  const HStretchyChar* charSpec = &hMap[spec.getGlyphId() & GLYPH_INDEX_MASK];

  AreaRef normal = (charSpec->normal != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->normal, size) : 0;
  AreaRef left = (charSpec->left != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->left, size) : 0;
  AreaRef glue = (charSpec->glue != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->glue, size) : 0;
  AreaRef right = (charSpec->right != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->right, size) : 0;

  return composeStretchyCharH(factory, normal, left, glue, right, span);
}

#include "scaledAux.hh"

AreaRef
Gtk_AdobeShaper::shapeStretchyCharV(const MathFormattingContext& ctxt, const GlyphSpec& spec, const scaled& strictSpan) const
{ 
  const scaled span = strictSpan - (1 * ctxt.getSize()) / 10;

  SmartPtr<Gtk_AreaFactory> factory = smart_cast<Gtk_AreaFactory>(ctxt.getDevice()->getFactory());
  assert(factory);

  std::cerr << "shapeStretchyCharV span = " << span << std::endl;

  const scaled size = ctxt.getSize();

  const VStretchyChar* charSpec = &vMap[spec.getGlyphId() & GLYPH_INDEX_MASK];

  AreaRef normal = (charSpec->normal != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->normal, size) : 0;
  AreaRef top = (charSpec->top != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->top, size) : 0;
  AreaRef glue = (charSpec->glue != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->glue, size) : 0;
  AreaRef middle = (charSpec->middle != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->middle, size) : 0;
  AreaRef bottom = (charSpec->bottom != 0) ? getGlyphArea(factory, SYMBOL_INDEX, charSpec->bottom, size) : 0;

  return composeStretchyCharV(factory, normal, top, glue, middle, bottom, span);
}


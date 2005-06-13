// Copyright (C) 2000-2005, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://helm.cs.unibo.it/mml-widget/, or send a mail to
// <lpadovan@cs.unibo.it>

#include <config.h>

#include <cassert>
#include <vector>
#include <sstream>

#include "Configuration.hh"
#include "ComputerModernFamily.hh"

ComputerModernFamily::ComputerModernFamily(const SmartPtr<AbstractLogger>&, const SmartPtr<Configuration>& conf)
{
  std::vector<String> fl = conf->getStringList("fonts/computer-modern/font-name");

  for (unsigned fn = FN_NIL + 1; fn < FN_NOT_VALID; fn++)
    for (unsigned fs = FS_NIL + 1; fs < FS_NOT_VALID; fs++)
      {
	const String fontName = nameOfFont(FontNameId(fn), FontSizeId(fs));
	configuredFont[fn][fs] = std::find(fl.begin(), fl.end(), fontName) != fl.end();
      }
}

ComputerModernFamily::~ComputerModernFamily()
{ }

SmartPtr<ComputerModernFamily>
ComputerModernFamily::create(const SmartPtr<AbstractLogger>& l, const SmartPtr<Configuration>& conf)
{ return new ComputerModernFamily(l, conf); }

int
ComputerModernFamily::sizeOfFontSizeId(FontSizeId id)
{
  assert(validFontSizeId(id));
  static int size[] = { 5, 6, 7, 8, 9, 10, 12, 17 };
  return size[id];
}

ComputerModernFamily::FontSizeId
ComputerModernFamily::fontSizeIdOfSize(int size)
{
  static FontSizeId fontSizeId[] = {
    FS_5, FS_6, FS_7, FS_8, FS_9, FS_10, FS_NIL, FS_12, FS_NIL, FS_NIL, FS_NIL, FS_NIL, FS_17
  };

  if (size >= 5 && size <= 17)
    return fontSizeId[size - 5];
  else
    return FS_NIL;
}

const char*
ComputerModernFamily::nameOfFontNameId(FontNameId id)
{
  assert(validFontNameId(id));
  static const char* name[] = {
    "cmr",
    "cmb",
    "cmbx",
    "cmbxti",
    "cmti",
    "cmss",
    "cmssi",
    "cmssbx",
    "cmtt",
    "cmsy",
    "cmbsy",
    "cmmi",
    "cmmib",
    "cmex"
  };
  assert(sizeof(name) / sizeof(const char*) == FN_NOT_VALID);
  return name[id];
}

String
ComputerModernFamily::nameOfFont(FontNameId id, FontSizeId designSize)
{
  std::ostringstream os;
  os << nameOfFontNameId(id) << sizeOfFontSizeId(designSize);
  return os.str();
}

String
ComputerModernFamily::nameOfFont(MathVariant variant, FontEncId encId, const scaled& size) const
{
  assert(validFontEncId(encId));
  
  scaled trueSize = size;
  FontSizeId sizeId;
  const FontNameId name = findFont(variant, encId, trueSize, sizeId);
  if (validFontNameId(name))
    return nameOfFont(name, sizeId);
  else
    return "";
}

ComputerModernFamily::FontEncId
ComputerModernFamily::encIdOfFontNameId(FontNameId id)
{
  assert(validFontNameId(id));
  static FontEncId enc[] = {
    FE_CMR,
    FE_CMR,
    FE_CMR,
    FE_CMR,
    FE_CMR,
    FE_CMR,
    FE_CMR,
    FE_CMR,
    FE_CMR,
    FE_CMSY,
    FE_CMSY,
    FE_CMMI,
    FE_CMMI, 
    FE_CMEX
  };
  assert(sizeof(enc) / sizeof(FontEncId) == FN_NOT_VALID);
  return enc[id];
}

bool
ComputerModernFamily::fontEnabled(FontNameId name, FontSizeId size) const
{
  assert(validFontNameId(name));
  assert(validFontSizeId(size));
  return configuredFont[name][size];
}

ComputerModernFamily::FontNameId
ComputerModernFamily::findBestFont(MathVariant variant, FontEncId encId, int size, FontSizeId& designSize) const
{
  assert(validFontEncId(encId));
  assert(validMathVariant(variant));

  designSize = FS_5;
  int bestDiff = std::abs(size - sizeOfFontSizeId(designSize));
  for (int i = FS_5; i < FS_NOT_VALID; i++)
    if (std::abs(size - sizeOfFontSizeId(FontSizeId(i))) < bestDiff)
      designSize = FontSizeId(i);

  static FontNameId family[FE_NOT_VALID][MONOSPACE_VARIANT - NORMAL_VARIANT + 1] =
    {
      { FN_CMR,  FN_CMB,   FN_CMTI, FN_CMBXTI, FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_CMSS, FN_CMSSBX, FN_CMSSI, FN_NIL, FN_CMTT },
      { FN_NIL,  FN_NIL,   FN_CMMI, FN_CMMIB,  FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_NIL,  FN_NIL,    FN_NIL,   FN_NIL, FN_NIL },
      { FN_CMSY, FN_CMBSY, FN_NIL,  FN_NIL,    FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_NIL,  FN_NIL,    FN_NIL,   FN_NIL, FN_NIL },
      { FN_CMEX, FN_NIL,   FN_NIL,  FN_NIL,    FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_NIL, FN_NIL,  FN_NIL,    FN_NIL,   FN_NIL, FN_NIL }
    };

  return family[encId - FE_CMR][variant - NORMAL_VARIANT];
}

ComputerModernFamily::FontNameId
ComputerModernFamily::findFont(MathVariant variant, FontEncId encId, scaled& size, FontSizeId& designSize) const
{
  size = round(size);
  FontNameId family = findBestFont(variant, encId, size.toInt(), designSize);
  if (variant == BOLD_VARIANT && encId == FE_CMR && fontEnabled(FN_CMBX, designSize))
    return FN_CMBX;
  else if (family != FN_NIL && fontEnabled(family, designSize))
    return family;

  // we prefer scaling the font rather than renouncing to the precise family
  designSize = FS_10;
  if (family != FN_NIL && fontEnabled(family, designSize))
    return family;

  switch (encId)
    {
    case FE_CMR:
      switch (variant)
	{
	case NORMAL_VARIANT:
	case BOLD_VARIANT:
	case ITALIC_VARIANT:
	case SCRIPT_VARIANT:
	case FRAKTUR_VARIANT:
	case SANS_SERIF_VARIANT:
	case MONOSPACE_VARIANT:
	case DOUBLE_STRUCK_VARIANT:
	  if (fontEnabled(FN_CMR)) return FN_CMR;
	  else return FN_NIL;
	case BOLD_ITALIC_VARIANT:
	  if (fontEnabled(FN_CMTI)) return FN_CMTI;
	  else if (fontEnabled(FN_CMB)) return FN_CMB;
	  else if (fontEnabled(FN_CMR)) return FN_CMR;
	  else return FN_NIL;
	case BOLD_FRAKTUR_VARIANT:
	case BOLD_SCRIPT_VARIANT:
	  if (fontEnabled(FN_CMB)) return FN_CMB;
	  else if (fontEnabled(FN_CMR)) return FN_CMR;
	  else return FN_NIL;
	case BOLD_SANS_SERIF_VARIANT:
	  if (fontEnabled(FN_CMSS)) return FN_CMSS;
	  else if (fontEnabled(FN_CMB)) return FN_CMB;
	  else if (fontEnabled(FN_CMR)) return FN_CMR;
	  else return FN_NIL;
	case SANS_SERIF_ITALIC_VARIANT:
	  if (fontEnabled(FN_CMSS)) return FN_CMSS;
	  else if (fontEnabled(FN_CMTI)) return FN_CMTI;
	  else if (fontEnabled(FN_CMR)) return FN_CMR;
	  else return FN_NIL;
	case SANS_SERIF_BOLD_ITALIC_VARIANT:
	  if (fontEnabled(FN_CMSSBX)) return FN_CMSSBX;
	  else if (fontEnabled(FN_CMSSI)) return FN_CMSSI;
	  else if (fontEnabled(FN_CMSS)) return FN_CMSS;
	  else if (fontEnabled(FN_CMTI)) return FN_CMTI;
	  else if (fontEnabled(FN_CMB)) return FN_CMB;
	  else if (fontEnabled(FN_CMR)) return FN_CMR;
	  else return FN_NIL;
	default:
	  assert(false);
	}
      break;
    case FE_CMMI:
      switch (variant)
	{
	case BOLD_ITALIC_VARIANT:
	case BOLD_FRAKTUR_VARIANT:
	case BOLD_SCRIPT_VARIANT:
	case BOLD_SANS_SERIF_VARIANT:
	case SANS_SERIF_BOLD_ITALIC_VARIANT:
	  if (fontEnabled(FN_CMMIB)) return FN_CMMIB;
	  else if (fontEnabled(FN_CMSY)) return FN_CMSY;
	  else return FN_NIL;
	default:
	  if (fontEnabled(FN_CMSY)) return FN_CMSY;
	  else return FN_NIL;
	}
      break;
    case FE_CMSY:
      switch (variant)
	{
	case BOLD_ITALIC_VARIANT:
	case BOLD_FRAKTUR_VARIANT:
	case BOLD_SCRIPT_VARIANT:
	case BOLD_SANS_SERIF_VARIANT:
	case SANS_SERIF_BOLD_ITALIC_VARIANT:
	  if (fontEnabled(FN_CMBSY)) return FN_CMBSY;
	  else if (fontEnabled(FN_CMSY)) return FN_CMSY;
	  else return FN_NIL;
	default:
	  if (fontEnabled(FN_CMSY)) return FN_CMSY;
	  else return FN_NIL;
	}
      break;
    case FE_CMEX:
      if (fontEnabled(FN_CMEX)) return FN_CMEX;
      else return FN_NIL;
    default:
      assert(false);
    }

  return FN_NIL;
}

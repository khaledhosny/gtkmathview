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

#ifndef __Gtk_AdobeShaper_hh__
#define __Gtk_AdobeShaper_hh__

#include <X11/Xft/Xft.h>

#include "Shaper.hh"

class Gtk_AdobeShaper : public Shaper
{
public:
  Gtk_AdobeShaper(void) { }
  virtual ~Gtk_AdobeShaper() { }

  virtual void registerChars(class ShaperManager&, unsigned shaperId) const;
  virtual unsigned shape(class ShapingResult&) const;

protected:
  XftFont* getFont(unsigned, const scaled&) const;
  void getGlyphExtents(XftFont*, FT_UInt, XGlyphInfo*) const;
  bool shapeChar(class ShapingResult&, const class GlyphSpec&) const;
  bool shapeStretchyCharV(class ShapingResult&, const class GlyphSpec&) const;
  bool shapeStretchyCharH(class ShapingResult&, const class GlyphSpec&) const;
};

#endif // __Gtk_AdobeShaper_hh__
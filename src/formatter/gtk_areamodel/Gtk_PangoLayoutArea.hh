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

#ifndef __Gtk_PangoLayoutArea_hh__
#define __Gtk_PangoLayoutArea_hh__

#include <pango/pango.h>

#include "GObjectPtr.hh"
#include "SimpleArea.hh"

class Gtk_PangoLayoutArea : public SimpleArea 
{
protected:
  Gtk_PangoLayoutArea(PangoLayout*);
  virtual ~Gtk_PangoLayoutArea();

public:
  static SmartPtr<Gtk_PangoLayoutArea> create(PangoLayout* layout)
  { return new Gtk_PangoLayoutArea(layout); }

  virtual BoundingBox box(void) const;
  virtual scaled leftEdge(void) const;
  virtual scaled rightEdge(void) const;
  virtual void render(class RenderingContext&, const scaled&, const scaled&) const;
  virtual DOM::Element dump(const DOM::Document&) const;

private:
  GObjectPtr<PangoLayout> layout;
};

#endif // __Gtk_PangoLayoutArea_hh__
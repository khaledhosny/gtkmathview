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

#include "Gtk_AreaFactory.hh"
#include "Gtk_PangoShaper.hh"
#include "Gtk_RenderingContext.hh"
#include "MathGraphicDevice.hh"
#include "MathMLElement.hh"
#include "MathVariantMap.hh"

struct HStretchyChar
{
  gunichar ch;
  gunichar left;
  gunichar glue;
  gunichar right;
};

struct VStretchyChar
{
  gunichar ch;
  gunichar top;
  gunichar glue;
  gunichar middle;
  gunichar bottom;
};

static HStretchyChar hMap[] =
  {
    { 0x2190, 0x2190, 0x23af, 0x0000 },
    { 0x2192, 0x0000, 0x23af, 0x2192 },
    { 0x2194, 0x2190, 0x23af, 0x2192 },
    { 0,      0,      0,      0      }
  };

static VStretchyChar vMap[] = 
  {
    { 0x0028, 0x239b, 0x239c, 0x0000, 0x239d }, // LEFT PAREN
    { 0x0029, 0x239e, 0x239f, 0x0000, 0x23a0 }, // RIGHT PAREN
    { 0x005b, 0x23a1, 0x23a2, 0x0000, 0x23a3 }, // LEFT SQUARE BRACKET
    { 0x005d, 0x23a4, 0x23a5, 0x0000, 0x23a6 }, // RIGHT SQUARE BRACKET
    { 0x007b, 0x23a7, 0x23aa, 0x23a8, 0x23a9 }, // LEFT CURLY BRACKET
    { 0x007d, 0x23ab, 0x23aa, 0x23ac, 0x23ad }, // RIGHT CURLY BRACKET
    { 0x2191, 0x2191, 0x23d0, 0x0000, 0x0000 }, // UP ARROW
    { 0x2193, 0x0000, 0x23d0, 0x0000, 0x2193 }, // BOTTOM ARROW
    { 0x222b, 0x2320, 0x23ae, 0x0000, 0x2321 }, // INTEGRAL
    { 0,      0,      0,      0,      0 }
  };

#define NORMAL_INDEX 0
#define H_STRETCHY_INDEX 1
#define V_STRETCHY_INDEX 2
#define MAPPED_BASE_INDEX 3

Gtk_PangoShaper::Gtk_PangoShaper()
{ }

Gtk_PangoShaper::~Gtk_PangoShaper()
{ }

void
Gtk_PangoShaper::registerShaper(const SmartPtr<class ShaperManager>& sm, unsigned shaperId)
{
  for (unsigned i = NORMAL_VARIANT; i <= MONOSPACE_VARIANT; i++)
    {
      for (Char16 ch = 0x21; ch < 0x100; ch++)
	{
	  Char32 vch = mapMathVariant(MathVariant(i), ch);
	  if (i == NORMAL_VARIANT || vch != ch)
	    sm->registerChar(vch, GlyphSpec(shaperId, MAPPED_BASE_INDEX + i - NORMAL_VARIANT, ch));
	}
    }

  for (unsigned i = 0; hMap[i].ch != 0; i++)
    sm->registerStretchyChar(hMap[i].ch, GlyphSpec(shaperId, H_STRETCHY_INDEX, i));

  for (unsigned i = 0; vMap[i].ch != 0; i++)
    sm->registerStretchyChar(vMap[i].ch, GlyphSpec(shaperId, V_STRETCHY_INDEX, i));
}

void
Gtk_PangoShaper::unregisterShaper(const SmartPtr<class ShaperManager>&, unsigned)
{

}

void
Gtk_PangoShaper::shape(const MathFormattingContext& ctxt, ShapingResult& result) const
{
  const GlyphSpec spec = result.getSpec();
  switch (spec.getFontId())
    {
    case H_STRETCHY_INDEX:
      result.pushArea(1, shapeStretchyCharH(ctxt, spec, result.getHSpan()));
      break;
    case V_STRETCHY_INDEX:
      result.pushArea(1, shapeStretchyCharV(ctxt, spec, result.getVSpan()));
      break;
    default:
      result.pushArea(1, shapeChar(ctxt, spec));
      break;
    }
}

AreaRef
Gtk_PangoShaper::shapeChar(const MathFormattingContext& ctxt, const GlyphSpec& spec) const
{
  gchar buffer[6];
  gint length = g_unichar_to_utf8(spec.getGlyphId(), buffer);

  PangoLayout* layout = createPangoLayout(buffer, length, Gtk_RenderingContext::toPangoPixels(ctxt.getSize()),
					  getTextAttributes(MathVariant(spec.getFontId() - MAPPED_BASE_INDEX + NORMAL_VARIANT)));
  SmartPtr<Gtk_AreaFactory> factory = smart_cast<Gtk_AreaFactory>(ctxt.getDevice()->getFactory());
  assert(factory);
  return factory->pangoLayoutLine(layout);
}

AreaRef
Gtk_PangoShaper::shapeStretchyCharH(const MathFormattingContext& ctxt, const GlyphSpec& spec, const scaled& span) const
{
  const HStretchyChar* charSpec = &hMap[spec.getGlyphId()];

  const AreaRef normal = (charSpec->ch != 0) ? shapeString(ctxt, &charSpec->ch, 1) : 0;
  const AreaRef left = (charSpec->left != 0) ? shapeString(ctxt, &charSpec->left, 1) : 0;
  const AreaRef glue = (charSpec->glue != 0) ? shapeString(ctxt, &charSpec->glue, 1) : 0;
  const AreaRef right = (charSpec->right != 0) ? shapeString(ctxt, &charSpec->right, 1) : 0;

  return composeStretchyCharH(ctxt.getDevice()->getFactory(), normal, left, glue, right, span);
}

AreaRef
Gtk_PangoShaper::shapeStretchyCharV(const MathFormattingContext& ctxt, const GlyphSpec& spec, const scaled& strictSpan) const
{
  const scaled span = strictSpan - (1 * ctxt.getSize()) / 10;

  const VStretchyChar* charSpec = &vMap[spec.getGlyphId()];

  AreaRef normal = (charSpec->ch != 0) ? shapeString(ctxt, &charSpec->ch, 1) : 0;
  AreaRef top = (charSpec->top != 0) ? shapeString(ctxt, &charSpec->top, 1) : 0;
  AreaRef glue = (charSpec->glue != 0) ? shapeString(ctxt, &charSpec->glue, 1) : 0;
  AreaRef middle = (charSpec->middle != 0) ? shapeString(ctxt, &charSpec->middle, 1) : 0;
  AreaRef bottom = (charSpec->bottom != 0) ? shapeString(ctxt, &charSpec->bottom, 1) : 0;

  return composeStretchyCharV(ctxt.getDevice()->getFactory(), normal, top, glue, middle, bottom, span);
}

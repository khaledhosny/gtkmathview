// Copyright (C) 2000-2002, Luca Padovani <luca.padovani@cs.unibo.it>.
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

#include "MathMLEmbellishment.hh"
#include "MathMLOperatorElement.hh"
#include "MathGraphicDevice.hh"
#include "MathFormattingContext.hh"

AreaRef
MathMLEmbellishment::formatEmbellishment(const SmartPtr<MathMLElement>& elem,
					 const MathFormattingContext& context,
					 const AreaRef& area)
{
  assert(elem);
  if (SmartPtr<MathMLOperatorElement> top = elem->getCoreOperatorTop())
    {
      assert(!top->dirtyAttribute());
      scaled leftPadding = top->GetLeftPadding();
      scaled rightPadding = top->GetRightPadding();
      if (leftPadding != scaled::zero() || rightPadding != scaled::zero())
	{
	  std::vector<AreaRef> row;
	  row.reserve(3);
	  row.push_back(context.getDevice()->getFactory()->horizontalSpace(leftPadding));
	  row.push_back(area);
	  row.push_back(context.getDevice()->getFactory()->horizontalSpace(rightPadding));
	  return context.getDevice()->getFactory()->horizontalArray(row);
	}
      else
	return area;
    }
  else
    return area;
}

void
MathMLEmbellishment::DoEmbellishmentLayout(const SmartPtr<MathMLElement>& elem, BoundingBox& box)
{
  assert(elem);
  if (SmartPtr<MathMLOperatorElement> top = elem->getCoreOperatorTop())
    {
      assert(!top->dirtyAttribute());
      box.width += top->GetLeftPadding() + top->GetRightPadding();
    }
}

void
MathMLEmbellishment::SetEmbellishmentPosition(const SmartPtr<MathMLElement>& elem, scaled& x, scaled&)
{
  assert(elem);
  if (SmartPtr<MathMLOperatorElement> top = elem->getCoreOperatorTop())
    {
      assert(!top->dirtyAttribute());
      x += top->GetLeftPadding();
    }
}

// Copyright (C) 2000-2007, Luca Padovani <padovani@sti.uniurb.it>.
//
// This file is part of GtkMathView, a flexible, high-quality rendering
// engine for MathML documents.
// 
// GtkMathView is free software; you can redistribute it and/or modify it
// either under the terms of the GNU Lesser General Public License version
// 3 as published by the Free Software Foundation (the "LGPL") or, at your
// option, under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation (the "GPL").  If you do not
// alter this notice, a recipient may use your version of this file under
// either the GPL or the LGPL.
//
// GtkMathView is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the LGPL or
// the GPL for more details.
// 
// You should have received a copy of the LGPL and of the GPL along with
// this program in the files COPYING-LGPL-3 and COPYING-GPL-2; if not, see
// <http://www.gnu.org/licenses/>.

#include <config.h>

#include <cassert>

#include "AreaFactory.hh"
#include "MathMLPhantomElement.hh"
#include "MathMLOperatorElement.hh"
#include "FormattingContext.hh"
#include "MathGraphicDevice.hh"

MathMLPhantomElement::MathMLPhantomElement(const SmartPtr<class MathMLNamespaceContext>& context)
  : MathMLNormalizingContainerElement(context)
{ }

MathMLPhantomElement::~MathMLPhantomElement()
{ }

bool
MathMLPhantomElement::IsSpaceLike() const
{
  assert(getChild());
  return getChild()->IsSpaceLike();
}

AreaRef
MathMLPhantomElement::format(FormattingContext& ctxt)
{
  if (dirtyLayout())
    {
      ctxt.push(this);
      AreaRef res = getChild() ? getChild()->format(ctxt) : 0;
      
      if (res)
	res = ctxt.MGD()->wrapper(ctxt, MathMLEmbellishment::formatEmbellishment(this, ctxt, ctxt.MGD()->getFactory()->hide(res)));
      else
	res = 0;
      setArea(res);
      ctxt.pop();

      resetDirtyLayout();
    }

  return getArea();
}

SmartPtr<MathMLOperatorElement>
MathMLPhantomElement::getCoreOperator()
{
  return getChild() ? getChild()->getCoreOperator() : 0;
}

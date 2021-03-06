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

#include "AbstractLogger.hh"
#include "MathMLAttributeSignatures.hh"
#include "MathMLActionElement.hh"
#include "MathMLOperatorElement.hh"
#include "FormattingContext.hh"
#include "MathGraphicDevice.hh"
#include "ValueConversion.hh"

MathMLActionElement::MathMLActionElement(const SmartPtr<class MathMLNamespaceContext>& context)
  : MathMLLinearContainerElement(context)
{
  selection = 0;
}

MathMLActionElement::~MathMLActionElement()
{ }

AreaRef
MathMLActionElement::format(FormattingContext& ctxt)
{
  if (dirtyLayout())
    {
      ctxt.push(this);

      if (SmartPtr<Value> value = GET_ATTRIBUTE_VALUE(MathML, Action, selection))
	selection = ToInteger(value) - 1;
      else
	selection = 0;

      if (SmartPtr<Value> vAction = GET_ATTRIBUTE_VALUE(MathML, Action, actiontype))
	{
	  String action = ToString(vAction);

	  if (action == "toggle")
	    {
	      // nothing to do, selecting the element is just fine
	      // but we want to have cycling
	      selection %= getSize();
	    }
	  else
	    getLogger()->out(LOG_WARNING, "action `%s' is not supported (ignored)", action.c_str());
	}
      else
	getLogger()->out(LOG_WARNING, "no action specified for `maction' element");

      AreaRef res;
      if (selection < getSize())
	if (SmartPtr<MathMLElement> elem = getChild(selection))
	  res = elem->format(ctxt);
      if (!res) res = ctxt.MGD()->dummy(ctxt);
      assert(res);

      setArea(ctxt.MGD()->wrapper(ctxt, res));
      ctxt.pop();

      resetDirtyLayout();
    }

  return getArea();
}

SmartPtr<MathMLOperatorElement>
MathMLActionElement::getCoreOperator()
{
  if (selection < getSize())
    if (SmartPtr<MathMLElement> elem = getChild(selection))
      return elem->getCoreOperator();
  return nullptr;
}

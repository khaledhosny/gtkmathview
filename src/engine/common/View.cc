// Copyright (C) 2000-2004, Luca Padovani <luca.padovani@cs.unibo.it>.
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

#include "Clock.hh"
#include "Globals.hh"
#include "MathMLView.hh"
#include "scaled.hh"
#include "scaledConv.hh"
#include "traverseAux.hh"
#include "MathMLElementFactory.hh"
#include "RefinementContext.hh"
#include "MathGraphicDevice.hh"
#include "MathFormattingContext.hh"
#include "SearchingContext.hh"
#include "Gtk_WrapperArea.hh"
#include "Linker.hh"

MathMLView::MathMLView(const SmartPtr<MathMLElementFactory>& f,
		       const SmartPtr<MathGraphicDevice>& d)
  : factory(f), device(d)
{
  freezeCounter = 0;
  defaultFontSize = Globals::configuration.GetFontSize();
}

MathMLView::~MathMLView()
{ }

bool
MathMLView::freeze()
{
  return freezeCounter++ == 0;
}

bool
MathMLView::thaw()
{
  assert(freezeCounter > 0);
  return --freezeCounter == 0;
}

SmartPtr<MathMLElement>
MathMLView::getRootElement() const
{
  return root ? smart_cast<MathMLElement>(factory->getElement(root)) : 0;
}

void
MathMLView::setDefaultFontSize(unsigned size)
{
  assert(size > 0);
  if (defaultFontSize != size)
    {
      defaultFontSize = size;
      if (SmartPtr<MathMLElement> elem = getRootElement())
	{
	  elem->setDirtyAttributeD();
	  elem->setDirtyLayout();	  
	}
    }
}

void
MathMLView::DOMSubtreeModifiedListener::handleEvent(const DOM::Event& ev)
{
  DOM::MutationEvent me(ev);
  assert(me);

  if (SmartPtr<MathMLElement> elem = findMathMLElement(view, DOM::Element(me.get_target())))
    {
      elem->setDirtyStructure();
      elem->setDirtyAttributeD();
    }
}

void
MathMLView::DOMAttrModifiedListener::handleEvent(const DOM::Event& ev)
{
  DOM::MutationEvent me(ev);
  assert(me);

  if (SmartPtr<MathMLElement> elem = findMathMLElement(view, DOM::Element(me.get_target())))
    elem->setDirtyAttribute();
}

void
MathMLView::setRoot(const DOM::Element& elem)
{
  if (root)
    {
      DOM::EventTarget et(root);
      assert(et);

      et.removeEventListener("DOMNodeInserted", *subtreeModifiedListener, false);
      et.removeEventListener("DOMNodeRemoved", *subtreeModifiedListener, false);
      et.removeEventListener("DOMCharacterDataModified", *subtreeModifiedListener, false);
      et.removeEventListener("DOMAttrModified", *attrModifiedListener, false);

      delete subtreeModifiedListener;
      delete attrModifiedListener;
      subtreeModifiedListener = 0;
      attrModifiedListener = 0;
    }

  if (elem)
    {
      root = elem;

      DOM::EventTarget et(root);
      assert(et);

      subtreeModifiedListener = new DOMSubtreeModifiedListener(this);
      attrModifiedListener = new DOMAttrModifiedListener(this);

      et.addEventListener("DOMNodeInserted", *subtreeModifiedListener, false);
      et.addEventListener("DOMNodeRemoved", *subtreeModifiedListener, false);
      et.addEventListener("DOMCharacterDataModified", *subtreeModifiedListener, false);
      et.addEventListener("DOMAttrModified", *attrModifiedListener, false);
    }
}

#include "Gtk_RenderingContext.hh"

SmartPtr<MathMLElement>
MathMLView::getElementAt(const scaled& x, const scaled& y) const
{
  if (AreaRef rootArea = getRootArea())
    {
      BoundingBox box = rootArea->box();
      SearchingContext context(x, y);
#if 0
      std::cerr << "searching at " << Gtk_RenderingContext::toGtkX(x) << "," << Gtk_RenderingContext::toGtkY(y) << std::endl;
#endif
      if (rootArea->find(context, -x0, -box.height - y0))
	{
	  SearchingContext::Result result = context.getResult();
#if 0
	  std::cerr << "found area at " << result.x << "," << result.y 
		    << " is wrapper? " << is_a<const Gtk_WrapperArea>(result.area)
		    << " has element? " << (smart_cast<const Gtk_WrapperArea>(result.area)->getElement() != 0) << std::endl;
#endif
	  if (SmartPtr<const Gtk_WrapperArea> area = smart_cast<const Gtk_WrapperArea>(result.area))
	    if (SmartPtr<MathMLElement> elem = smart_cast<MathMLElement>(area->getElement()))
	      return elem;
	}
    }
  
  return 0;
}

bool
MathMLView::getElementExtents(const DOM::Element& el, scaled& x, scaled& y, BoundingBox& box) const
{
  assert(el);
  if (AreaRef rootArea = getRootArea())
    if (SmartPtr<MathMLElement> elem = findMathMLElement(this, el))
      if (AreaRef elemArea = elem->getArea())
	{
	  AreaId elemId = rootArea->idOf(elemArea);

	  for (AreaId id = elemId; !id.empty(); id = id.tail())
	    std::cout << id.head() << "/";
	  std::cout << std::endl;

	  std::pair<scaled,scaled> orig = rootArea->origin(elemId);
	  x = orig.first;
	  y = orig.second;
	  box = elemArea->box();
	  return true;
	}
							    
  return false;
}

AreaRef
MathMLView::getRootArea() const
{
  if (root && !frozen())
    {
      SmartPtr<MathMLElement> elem = getRootElement();
      assert(elem);

      if (elem->dirtyStructure())
	{
	  Clock perf;
	  perf.Start();
	  elem->construct();
	  perf.Stop();
	  Globals::logger(LOG_INFO, "construction time: %dms", perf());
	}

      if (elem->dirtyAttribute() || elem->dirtyAttributeP())
	{
	  RefinementContext rc;
	  Clock perf;
	  perf.Start();
	  elem->refine(rc);
	  perf.Stop();
	  Globals::logger(LOG_INFO, "refinement time: %dms", perf());
	}

      if (elem->dirtyLayout())
	{
	  MathFormattingContext ctxt(device);
	  scaled l = device->evaluate(ctxt, Length(defaultFontSize, Length::PT_UNIT), scaled::zero());
	  //ctxt.setSize(device->evaluate(ctxt, Length(28, Length::PT_UNIT), scaled::zero()));
	  ctxt.setSize(l);
	  ctxt.setActualSize(ctxt.getSize());
	  Clock perf;
	  perf.Start();
	  elem->format(ctxt);
	  perf.Stop();
	  Globals::logger(LOG_INFO, "format time: %dms", perf());
	}

      return elem->getArea();
    }

  return 0;
}

BoundingBox
MathMLView::getBoundingBox() const
{
  if (AreaRef rootArea = getRootArea())
    return rootArea->box();
  else
    return BoundingBox();
}

Rectangle
MathMLView::getRectangle() const
{
  if (AreaRef rootArea = getRootArea())
    return Rectangle(scaled::zero(), scaled::zero(), rootArea->box());
  else
    return Rectangle();
}

void
MathMLView::render(RenderingContext& ctxt) const
{
  if (AreaRef rootArea = getRootArea())
    {
      Clock perf;
      perf.Start();
      BoundingBox box = rootArea->box();
      rootArea->render(ctxt, -x0, -box.height - y0);
      perf.Stop();
      Globals::logger(LOG_INFO, "rendering time: %dms", perf());
    }
}

SmartPtr<Linker>
MathMLView::getLinker() const
{
  return factory->getLinker();
}
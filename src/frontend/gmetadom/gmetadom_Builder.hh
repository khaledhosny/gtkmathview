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

#ifndef __gmetadom_Builder_hh__
#define __gmetadom_Builder_hh__

#include "gmetadom.hh"
#include "gmetadom_Model.hh"
#include "Builder.hh"
#include "String.hh"
#include "WeakPtr.hh"
#include "TemplateLinker.hh"

class gmetadom_Builder : public Builder
{
protected:
  gmetadom_Builder(void) { }
  virtual ~gmetadom_Builder();

public:
  static SmartPtr<gmetadom_Builder> create(void);

  void setRootModelElement(const DOM::Element&);
  DOM::Element getRootModelElement(void) const { return root; }

  SmartPtr<Element> findElement(const DOM::Element& p) const { return linker.assoc(p); }
  DOM::Element findSelfOrAncestorModelElement(const SmartPtr<Element>&) const;
  SmartPtr<Element> findSelfOrAncestorElement(const DOM::Element&) const;

  bool notifyStructureChanged(const DOM::Element&);
  bool notifyAttributeChanged(const DOM::Element&, const DOM::GdomeString&);

protected:
  // methods for accessing the linker
  SmartPtr<Element> linkerAssoc(const DOM::Element& el) const { return linker.assoc(el); }
  void linkerAdd(const DOM::Element& el, Element* elem) const;
  void linkerRemove(Element* elem) const;

  class DOMSubtreeModifiedListener : public DOM::EventListener
  {
  public:
    DOMSubtreeModifiedListener(const SmartPtr<gmetadom_Builder>& b) : builder(b) { };
    virtual ~DOMSubtreeModifiedListener() { };
    virtual void handleEvent(const DOM::Event&);

  private:
    WeakPtr<gmetadom_Builder> builder;
  };

  class DOMAttrModifiedListener : public DOM::EventListener
  {
  public:
    DOMAttrModifiedListener(const SmartPtr<gmetadom_Builder>& b) : builder(b) { };
    virtual ~DOMAttrModifiedListener() { };
    virtual void handleEvent(const DOM::Event&);

  private:
    WeakPtr<gmetadom_Builder> builder;
  };

private:
  DOMSubtreeModifiedListener* subtreeModifiedListener;
  DOMAttrModifiedListener* attrModifiedListener;
  mutable TemplateLinker<gmetadom_Model> linker;
  DOM::Element root;
};

#endif // __gmetadom_Builder_hh__

// Copyright (C) 2000, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://cs.unibo.it/~lpadovan/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#include <config.h>
#include <assert.h>

#include "keyword.hh"
#include "gdomeAux.h"
#include "Iterator.hh"
#include "stringAux.hh"
#include "MathEngine.hh"
#include "MathMLAttribute.hh"
#include "MathMLParseFile.hh"
#include "OperatorDictionary.hh"
#include "MathMLAttributeList.hh"

static void
getAttribute(GdomeElement* node, const char* attr, MathMLAttributeList* aList)
{
  assert(aList != NULL);

  if (!gdome_el_hasAttribute_c(node, attr)) return;

  GdomeDOMString* attrVal = gdome_el_getAttribute_c(node, attr);
  assert(attrVal != NULL);

  MathMLAttribute* attribute =
    new MathMLAttribute(AttributeIdOfName(attr), allocString(attrVal));

  aList->Append(attribute);
  gdome_str_unref(attrVal);
}

//

OperatorDictionary::OperatorDictionary()
{
}

OperatorDictionary::~OperatorDictionary()
{
  Unload();
}

bool
OperatorDictionary::Load(const char* fileName)
{
  GdomeDocument* doc = MathMLParseFile(fileName, true);
  if (doc == NULL) return false;

  GdomeException exc;
  GdomeElement* root = gdome_doc_documentElement(doc, &exc);
  if (root == NULL) {
    gdome_doc_unref(doc, &exc);
    MathEngine::logger(LOG_WARNING, "operator dictionary `%s': parse error", fileName);
    return false;
  }

  if (!gdome_n_name_is(GDOME_N(root), "dictionary")) {
    gdome_doc_unref(doc, &exc);
    MathEngine::logger(LOG_WARNING, "operator dictionary `%s': could not find root element", fileName);
    return false;
  }

  for (GdomeNode* p = gdome_el_firstChild(root, &exc);
       p != NULL;
       p = gdome_n_nextSibling_unref(p)) {
    if (!gdome_n_isBlank(p) && gdome_n_name_is(p, "operator")) {
      GdomeElement* op = GDOME_EL(p);
      assert(op != NULL);

      if (gdome_el_hasAttribute_c(op, "name")) {
	GdomeDOMString* opName = gdome_el_getAttribute_c(op, "name");
	const String* opString = allocString(opName);
	gdome_str_unref(opName);

	MathMLAttributeList* def = new MathMLAttributeList;

	getAttribute(op, "form", def);
	getAttribute(op, "fence", def);
	getAttribute(op, "separator", def);
	getAttribute(op, "lspace", def);
	getAttribute(op, "rspace", def);
	getAttribute(op, "stretchy", def);
	getAttribute(op, "direction", def);
	getAttribute(op, "symmetric", def);
	getAttribute(op, "maxsize", def);
	getAttribute(op, "minsize", def);
	getAttribute(op, "largeop", def);
	getAttribute(op, "movablelimits", def);
	getAttribute(op, "accent", def);

	const MathMLAttributeList* defaults = AlreadyDefined(*def);
	if (defaults == NULL) defaults = def;
	else delete def;

	OperatorDictionaryItem* item = new OperatorDictionaryItem;
	item->name     = opString;
	item->defaults = defaults;

	items.Append(item);
      } else {
	MathEngine::logger(LOG_WARNING, "operator dictionary `%s': could not find operator name", fileName);
      }
    } else if (!gdome_n_isBlank(p)) {
      GdomeDOMString* nodeName = gdome_n_nodeName(p, &exc);
      assert(nodeName != NULL);

      MathEngine::logger(LOG_WARNING, "operator dictionary `%s': unknown element `%s'", fileName,
			 gdome_str_c(nodeName));

      gdome_str_unref(nodeName);
    }
  }

  gdome_doc_unref(doc, &exc);

  return true;
}

void
OperatorDictionary::Unload()
{
  while (defaults.GetSize() > 0) {
    const MathMLAttributeList* aList = defaults.RemoveFirst();
    delete aList;
  }

  while (items.GetSize() > 0) {
    const OperatorDictionaryItem* item = items.RemoveFirst();
    delete item;
  }
}

const MathMLAttributeList*
OperatorDictionary::AlreadyDefined(const MathMLAttributeList& def) const
{
  for (Iterator<const MathMLAttributeList*> i(defaults); i.More(); i.Next()) {
    assert(i() != NULL);
    if (i()->Equal(def)) return i();
  }

  return NULL;
}

void
OperatorDictionary::Search(const String* opName,
			   const MathMLAttributeList** prefix,
			   const MathMLAttributeList** infix,
			   const MathMLAttributeList** postfix) const
{
  assert(opName != NULL);
  assert(prefix != NULL && infix != NULL && postfix != NULL);

  *prefix = *infix = *postfix = NULL;

  for (Iterator<const OperatorDictionaryItem*> p(items); p.More(); p.Next()) {
    assert(p() != NULL);
    assert(p()->name != NULL);
    assert(p()->defaults != NULL);

    if (p()->name->Equal(*opName)) {
      const MathMLAttribute* attribute = p()->defaults->GetAttribute(ATTR_FORM);
      if (attribute != NULL && attribute->GetValue() != NULL) {
	const String* form = attribute->GetValue();
	if (*prefix == NULL && form->Equal("prefix")) *prefix = p()->defaults;
	else if (*infix == NULL && form->Equal("infix")) *infix = p()->defaults;
	else if (*postfix == NULL && form->Equal("postfix")) *postfix = p()->defaults;
      } else
	MathEngine::logger(LOG_WARNING, 
			   "entry for `%s' in operator dictionary has no mandatory `form' attribute",
			   p()->name->ToStaticC());
    }
  }
}

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

#ifndef __AttributeSignature_hh__
#define __AttributeSignature_hh__

#include "String.hh"
#include "Value.hh"
#include "SmartPtr.hh"

typedef SmartPtr<Value> (*AttributeParser)(const UCS4String::const_iterator&,
					   const UCS4String::const_iterator&,
					   UCS4String::const_iterator&);

struct GMV_MathView_EXPORT AttributeSignature
{
  String name;
  String fullName;
  AttributeParser parser;
  bool fromElement;
  bool fromContext;
  bool deprecated;
  bool emptyMeaningful;
  const char* defaultUnparsedValue;
  mutable SmartPtr<Value> defaultValue;

  SmartPtr<Value> getDefaultValue(void) const;
  SmartPtr<Value> parseValue(const String&) const;
};

typedef const AttributeSignature* AttributeId;

#define ATTRIBUTE_SIGNATURE(ns,el,name) sig_##ns##_##el##_##name
#define ATTRIBUTE_ID_OF_SIGNATURE(sig) (&(sig))
#define ATTRIBUTE_ID(ns,el,name) (ATTRIBUTE_ID_OF_SIGNATURE(ATTRIBUTE_SIGNATURE(ns,el,name)))
#define ATTRIBUTE_PARSER(ns,el,name) &Parse_##ns##_##el##_##name::parse
#define ATTRIBUTE_NAME_OF_ID(id) ((id)->name)
#define ATTRIBUTE_FULL_NAME(ns,el,name) #ns"."#el"."#name
#define REFINE_ATTRIBUTE(ctxt,ns,el,name) refineAttribute(ctxt, ATTRIBUTE_SIGNATURE(ns,el,name))
#define GET_ATTRIBUTE_VALUE(ns,el,name) getAttributeValue(ATTRIBUTE_SIGNATURE(ns,el,name))
#define GET_ATTRIBUTE_DEFAULT_VALUE(ns,el,name) (ATTRIBUTE_SIGNATURE(ns,el,name).getDefaultValue())
#define DECLARE_ATTRIBUTE(ns,el,name) extern GMV_MathView_EXPORT const AttributeSignature ATTRIBUTE_SIGNATURE(ns,el,name)
#define DEFINE_ATTRIBUTE(ns,el,name,fe,fc,de,em,df) \
  const AttributeSignature ATTRIBUTE_SIGNATURE(ns,el,name) = \
  { #name, ATTRIBUTE_FULL_NAME(ns,el,name), ATTRIBUTE_PARSER(ns,el,name), fe, fc, de, em, df, 0 }

#endif // __AttributeSignature_hh__

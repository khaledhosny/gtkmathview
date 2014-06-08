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

#include <glib.h>
#include <cctype>

#include "String.hh"

String
trimSpacesLeft(const String& s)
{
  String::const_iterator i = s.begin();
  while (i != s.end() && isXmlSpace(*i)) i++;
  if (i != s.begin())
    return String(i, s.end());
  else
    return s;
}

String
trimSpacesRight(const String& s)
{
  String::const_iterator i = s.end();
  while (i != s.begin() && isXmlSpace(*(i - 1))) i--;
  if (i != s.end())
    return String(s.begin(), i);
  else
    return s;
}

String
collapseSpaces(const String& s)
{
  String res;
  res.reserve(s.length());

  String::const_iterator i = s.begin();
  while (i != s.end())
    if (isXmlSpace(*i))
      {
	res.push_back(' ');
	do i++; while (i != s.end() && isXmlSpace(*i));
      }
    else
      {
	res.push_back(*i);
	i++;
      }
  return res;
}

String
deleteSpaces(const String& s)
{
  String res;
  res.reserve(s.length());
  for (String::const_iterator i = s.begin(); i != s.end(); i++)
    if (!isXmlSpace(*i))
      res.push_back(*i);
  return res;
}

String
toLowerCase(const String& s)
{
  String res;
  res.reserve(s.length());
  for (String::const_iterator i = s.begin(); i != s.end(); i++)
    res.push_back(tolower(*i));
  return res;
}

template <typename DEST_CHAR, typename SOURCE_CHAR, typename DEST_STRING, typename SOURCE_STRING, 
	  DEST_CHAR* (*f)(const SOURCE_CHAR*, glong, glong*, glong*, GError**)>
DEST_STRING
DESTofSOURCE(const SOURCE_STRING& s)
{
  g_assert(sizeof(DEST_CHAR) == sizeof(typename DEST_STRING::value_type));
  g_assert(sizeof(SOURCE_CHAR) == sizeof(typename SOURCE_STRING::value_type));
  glong length;
  DEST_CHAR* destBuffer = f((const SOURCE_CHAR*) s.data(), s.length(), NULL, &length, NULL);
  DEST_STRING res((const typename DEST_STRING::value_type*) destBuffer, length);
  g_free(destBuffer);
  return res;
}

String
StringOfUCS4String(const UCS4String& s)
{ return DESTofSOURCE<gchar, gunichar, String, UCS4String, &g_ucs4_to_utf8>(s); }

UCS4String
UCS4StringOfString(const String& s)
{ return DESTofSOURCE<gunichar, gchar, UCS4String, String, &g_utf8_to_ucs4>(s); }

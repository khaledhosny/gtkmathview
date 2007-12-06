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

#include "TFMManager.hh"
#include "TFMFont.hh"
#include "TFMFontManager.hh"

TFMFontManager::TFMFontManager(const SmartPtr<TFMManager>& tm)
  : tfmManager(tm)
{ }

TFMFontManager::~TFMFontManager()
{ }

SmartPtr<TFMFontManager>
TFMFontManager::create(const SmartPtr<TFMManager>& tm)
{ return new TFMFontManager(tm); }

SmartPtr<TFMFont>
TFMFontManager::createFont(const SmartPtr<TFM>& tfm, const scaled& size) const
{ return TFMFont::create(tfm, size); }

SmartPtr<TFMFont>
TFMFontManager::getFont(const SmartPtr<TFM>& tfm, const scaled& size) const
{
  const CachedFontKey key(tfm->getName(), size);
  FontCache::iterator p = fontCache.find(key);
  if (p != fontCache.end())
    return p->second;
  else if (const SmartPtr<TFMFont> font = createFont(tfm, size))
    {
      fontCache[key] = font;
      return font;
    }
  else
    return 0;
}

SmartPtr<TFMFont>
TFMFontManager::getFont(const String& name, const scaled& size) const
{ return getFont(tfmManager->getTFM(name), size); }

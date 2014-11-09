// Copyright (C) 2000-2007, Luca Padovani <padovani@sti.uniurb.it>.
// Copyright (C) 2013, Khaled Hosny <khaledhosny@eglug.org>.
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

#include <cairo.h>
#include <cairo-ft.h>
#ifdef CAIRO_HAS_SVG_SURFACE
#  include <cairo-svg.h>
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
#  include <cairo-pdf.h>
#endif
#ifdef CAIRO_HAS_PS_SURFACE
#  include <cairo-ps.h>
#endif
#include <glib.h>

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "defs.h"
#include "Logger.hh"
#include "libxml2_MathView.hh"
#include "MathMLOperatorDictionary.hh"
#include "Cairo_Backend.hh"
#include "Cairo_RenderingContext.hh"
#include "MathGraphicDevice.hh"
#include "MathMLNamespaceContext.hh"
#include "FormattingContext.hh"

#define XSTR(s) STR(s)
#define STR(s) #s

#define DEF_FONT_SIZE   XSTR(DEFAULT_FONT_SIZE)
#define DEF_FONT_FAMILY DEFAULT_FONT_FAMILY

typedef libxml2_MathView MathView;

static char **remaining_args = NULL;
static char *fontname = (char*) DEFAULT_FONT_FAMILY;
static int fontsize = DEFAULT_FONT_SIZE;
static GOptionEntry entries[] = {
  { "font-family", 'f', 0, G_OPTION_ARG_STRING, &fontname, "Font name (default: " DEF_FONT_FAMILY ")",     "family" },
  { "face-size",   's', 0, G_OPTION_ARG_INT,    &fontsize, "Face size (default: " DEF_FONT_SIZE ")", "size" },
  { G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &remaining_args, NULL, "[FILE...]" },
  { NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

static cairo_surface_t*
create_surface(const String filename, double width, double height)
{
  cairo_surface_t* surface = NULL;
  std::string::size_type n = filename.find(".");
  if (n != std::string::npos && n < filename.length() - 1)
  {
    const String extension = filename.substr(n + 1);
    if (extension == "png")
      surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
#ifdef CAIRO_HAS_SVG_SURFACE
    else if (extension == "svg")
      surface = cairo_svg_surface_create(filename.c_str(), width, height);
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
    else if (extension == "pdf")
      surface = cairo_pdf_surface_create(filename.c_str(), width, height);
#endif
#ifdef CAIRO_HAS_PS_SURFACE
    else if (extension == "ps")
      surface = cairo_ps_surface_create(filename.c_str(), width, height);
    else if (extension == "eps") {
      surface = cairo_ps_surface_create(filename.c_str(), width, height);
      cairo_ps_surface_set_eps(surface, true);
    }
#endif
    else
      g_print("can't output files of format: %s\n", extension.c_str());
  } else {
      g_print("can't output files without extension\n");
  }

  return surface;
}

int
main(int argc, char *argv[])
{
  GError *error = NULL;
  GOptionContext *context = g_option_context_new(NULL);
  g_option_context_add_main_entries(context, entries, NULL);

  if (!g_option_context_parse(context, &argc, &argv, &error))
  {
    g_print("option parsing failed: %s\n", error->message);
    exit(1);
  }

  if (remaining_args == NULL || remaining_args[1] == NULL)
  {
    g_print("no input or output files specified\n");
    exit(1);
  }

  const String input_file = remaining_args[0];
  const String output_file = remaining_args[1];

  SmartPtr<AbstractLogger> logger = Logger::create();

  FcResult result;
  FcPattern* pattern = FcPatternCreate();
  FcPatternAddString(pattern, FC_FAMILY, (FcChar8 *) fontname);
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);
  FcPattern* resolved = FcFontMatch(NULL, pattern, &result);
  cairo_font_face_t* font_face = cairo_ft_font_face_create_for_pattern(resolved);

  cairo_matrix_t font_matrix, font_ctm;
  cairo_matrix_init_scale(&font_matrix, fontsize, fontsize);
  cairo_matrix_init_identity(&font_ctm);
  cairo_font_options_t* font_options = cairo_font_options_create();
  cairo_scaled_font_t* font = cairo_scaled_font_create(font_face, &font_matrix, &font_ctm, font_options);
  cairo_font_options_destroy(font_options);

  SmartPtr<Backend> backend = Cairo_Backend::create(font);
  SmartPtr<MathGraphicDevice> device = backend->getMathGraphicDevice();
  SmartPtr<MathMLOperatorDictionary> dictionary = MathMLOperatorDictionary::create();

  SmartPtr<MathView> view = MathView::create(logger);
  view->setOperatorDictionary(dictionary);
  view->setMathMLNamespaceContext(MathMLNamespaceContext::create(view, device));
  view->setDefaultFontSize(fontsize);

  view->loadURI(input_file);

  const BoundingBox box = view->getBoundingBox();
  double width = box.horizontalExtent().toDouble();
  double height = box.verticalExtent().toDouble();
  cairo_surface_t* surface = create_surface(output_file, width, height);
  if (surface)
  {
    cairo_t* cr = cairo_create(surface);
    Cairo_RenderingContext* rc = new Cairo_RenderingContext(cr);

    view->render(*rc, scaled::zero(), -box.height);

    if (cairo_surface_get_type(surface) == CAIRO_SURFACE_TYPE_IMAGE)
      cairo_surface_write_to_png(surface, output_file.c_str());

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
  }

  view->resetRootElement();

  cairo_scaled_font_destroy(font);
  cairo_font_face_destroy(font_face);

  return 0;
}

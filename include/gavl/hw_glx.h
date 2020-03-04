/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2012 Members of the Gmerlin project
 * gmerlin-general@lists.sourceforge.net
 * http://gmerlin.sourceforge.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************/

#include <gavl/gavldefs.h>
#include <X11/Xlib.h>

/**
 * @file hw_vaapi_x11.h
 * external api header.
 */

/* Structure returned by gavl_hw_get_native_handle */

#include <GL/glx.h>

typedef struct
  {
  Display * dpy;
  GLXFBConfig * fbcfg;
  
  } gavl_hw_vaapi_x11_t;

GAVL_PUBLIC gavl_hw_context_t * gavl_hw_ctx_create_glx(Display * dpy, int * attrs);

GAVL_PUBLIC Display * gavl_hw_ctx_glx_get_display(gavl_hw_context_t *);
GAVL_PUBLIC GLXFBConfig gavl_hw_ctx_glx_get_fbconfig(gavl_hw_context_t * ctx);
GAVL_PUBLIC GLXContext gavl_hw_ctx_glx_get_ctx(gavl_hw_context_t * ctx); 


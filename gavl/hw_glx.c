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

#include <stdlib.h>

#include <X11/Xlib.h>


#include <gavl/gavl.h>
#include <gavl/hw_glx.h>

#include <hw_private.h>

typedef struct
  {
  GLXFBConfig * fbcfg;

  GLXContext ctx;
  
  Display * private_display;
  Display * display;
  } glx_t;

static void destroy_native_glx(void * native)
  {
  glx_t * priv = native;

  if(priv->fbcfg)
    XFree(priv->fbcfg);

  if(priv->ctx)
    glXDestroyContext(priv->display, priv->ctx);
      
  if(priv->private_display)
    XCloseDisplay(priv->private_display);
  free(priv);

  }

static gavl_pixelformat_t * get_image_formats_glx(gavl_hw_context_t * ctx)
  {
  return NULL;
  }

static gavl_pixelformat_t * get_overlay_formats_glx(gavl_hw_context_t * ctx)
  {
  return NULL;
  }
  
static void video_format_adjust_glx(gavl_hw_context_t * ctx,
                                    gavl_video_format_t * fmt)
  {
  return;
  }

static void overlay_format_adjust_glx(gavl_hw_context_t * ctx, gavl_video_format_t * fmt)
  {
  return;
  }

  
static gavl_video_frame_t * video_frame_create_hw_glx(gavl_hw_context_t * ctx,
                                                      gavl_video_format_t * fmt)
  {
  return NULL;

  }

static gavl_video_frame_t * video_frame_create_ram_glx(gavl_hw_context_t * ctx,
                                                       gavl_video_format_t * fmt)
  {
  return NULL;

  }

static gavl_video_frame_t * video_frame_create_ovl_glx(gavl_hw_context_t * ctx,
                                                       gavl_video_format_t * fmt)
  {
  return NULL;

  }

static void video_frame_destroy_glx(gavl_video_frame_t * f)
  {
  return;
  }

static int video_frame_to_ram_glx(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  return 0;
  }

static int video_frame_to_hw_glx(const gavl_video_format_t * fmt,
                                 gavl_video_frame_t * dst,
                                 gavl_video_frame_t * src)
  {
  return 0;
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native         = destroy_native_glx,
    .get_image_formats      = get_image_formats_glx,
    .get_overlay_formats    = get_overlay_formats_glx,
    .video_frame_create_hw  = video_frame_create_hw_glx,
    .video_frame_create_ram = video_frame_create_ram_glx,
    .video_frame_create_ovl = video_frame_create_ovl_glx,
    .video_frame_destroy    = video_frame_destroy_glx,
    .video_frame_to_ram     = video_frame_to_ram_glx,
    .video_frame_to_hw      = video_frame_to_hw_glx,
    .video_format_adjust    = video_format_adjust_glx,
    .overlay_format_adjust  =  overlay_format_adjust_glx,
    
  };

gavl_hw_context_t * gavl_hw_ctx_create_glx(Display * dpy, int * attrs)
  {
  glx_t * priv;
  int n_elements = 0;
  
  priv = calloc(1, sizeof(*priv));

  if(dpy)
    {
    priv->display = dpy;
    }
  else
    {
    priv->private_display = XOpenDisplay(NULL);

    if(!priv->private_display)
      return NULL;
    
    priv->display = priv->private_display;
    }
  
  priv->fbcfg = glXChooseFBConfig(priv->display,
                                  DefaultScreen(priv->display),
                                  attrs, &n_elements);

  if(!priv->fbcfg || !n_elements)
    goto fail;

  priv->ctx =
    glXCreateNewContext(priv->display, priv->fbcfg[0], GLX_RGBA_TYPE, NULL, True);
  
  return gavl_hw_context_create_internal(priv, &funcs, GAVL_HW_GLX);
  
  fail:

  if(priv)
    destroy_native_glx(priv);

  return NULL;
  }

Display * gavl_hw_ctx_glx_get_display(gavl_hw_context_t * ctx)
  {
  glx_t * p = ctx->native;
  return p->display;
  }

GLXFBConfig gavl_hw_ctx_glx_get_fbconfig(gavl_hw_context_t * ctx)
  {
  glx_t * p = ctx->native;
  return p->fbcfg[0];
  }

GLXContext gavl_hw_ctx_glx_get_ctx(gavl_hw_context_t * ctx)
  {
  glx_t * p = ctx->native;
  return p->ctx;
  }

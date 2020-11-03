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
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>


#include <gavl/gavl.h>
#include <gavl/hw_glx.h>
#include <gavl/hw_gl.h>

#include <hw_private.h>


typedef struct
  {
  GLXFBConfig * fbcfg;

  GLXContext ctx;
  
  Display * private_display;
  Display * display;

  Window win;
  GLXWindow glx_win;

  Colormap cmap;
  
  } glx_t;

static void destroy_native_glx(void * native)
  {
  glx_t * priv = native;

  if(priv->fbcfg)
    XFree(priv->fbcfg);

  if(priv->ctx)
    glXDestroyContext(priv->display, priv->ctx);

  if(priv->glx_win != None)
    glXDestroyWindow(priv->display, priv->glx_win);
  
  if(priv->win != None)
    XDestroyWindow(priv->display, priv->win);

  if(priv->cmap != None)
    XFreeColormap(priv->display, priv->cmap);
  
  if(priv->private_display)
    XCloseDisplay(priv->private_display);
  free(priv);

  }
  

static gavl_video_frame_t * video_frame_create_hw_glx(gavl_hw_context_t * ctx,
                                                      gavl_video_format_t * fmt)
  {
  GLuint * tex;
  gavl_video_frame_t * ret;
  
  //  fprintf(stderr, "video_frame_create_hw_glx\n");
  //  gavl_video_format_dump(fmt);
  
  ret = gavl_video_frame_create(NULL);

  tex = calloc(1, sizeof(*tex));
  
  gavl_hw_glx_set_current(ctx, None);
  *tex = gavl_gl_create_texture(fmt);
  gavl_hw_glx_unset_current(ctx);

  ret->user_data = tex;
  
  return ret;
  }

static void video_frame_destroy_glx(gavl_video_frame_t * f)
  {
  if(f->user_data)
    {
    GLuint * tex = f->user_data;

    gavl_hw_glx_set_current(f->hwctx, None);
    glDeleteTextures(1, tex);
    gavl_hw_glx_unset_current(f->hwctx);

    free(tex);
    
    }
  f->hwctx = NULL;
  gavl_video_frame_destroy(f);
  return;
  }

static int video_frame_to_ram_glx(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  GLuint * tex;

  gavl_hw_glx_set_current(src->hwctx, None);

  tex = src->user_data;
  gavl_gl_frame_to_ram(fmt, dst, *tex);

  gavl_hw_glx_unset_current(src->hwctx);
  
  return 1;
  }

static int video_frame_to_hw_glx(const gavl_video_format_t * fmt,
                                 gavl_video_frame_t * dst,
                                 gavl_video_frame_t * src)
  {
  GLuint * tex;
  
  gavl_hw_glx_set_current(dst->hwctx, None);

  tex = dst->user_data;
  gavl_gl_frame_to_hw(fmt, *tex, src);
  
  gavl_hw_glx_unset_current(dst->hwctx);
  
  return 1;
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native         = destroy_native_glx,
    .get_image_formats      = gavl_gl_get_image_formats,
    .get_overlay_formats    = gavl_gl_get_overlay_formats,
    .video_frame_create_hw  = video_frame_create_hw_glx,
    .video_frame_destroy    = video_frame_destroy_glx,
    .video_frame_to_ram     = video_frame_to_ram_glx,
    .video_frame_to_hw      = video_frame_to_hw_glx,
    .video_format_adjust    = gavl_gl_adjust_video_format,
    .overlay_format_adjust  = gavl_gl_adjust_video_format,
  };

gavl_hw_context_t * gavl_hw_ctx_create_glx(Display * dpy, const int * attrs)
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

  priv->win = None;
  priv->glx_win = None;
  priv->cmap = None;
  
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

void gavl_hw_glx_set_current(gavl_hw_context_t * ctx, GLXDrawable drawable)
  {
  glx_t * p = ctx->native;

  //  fprintf(stderr, "gavl_hw_glx_set_current %p %ld\n", ctx, drawable);
  
  if(drawable == None)
    {

    if(p->win == None)
      {
      Window root;
      XSetWindowAttributes swa;
      
      XVisualInfo * vi = glXGetVisualFromFBConfig(p->display, p->fbcfg[0]);

      root = DefaultRootWindow(p->display);
      
      p->cmap = XCreateColormap(p->display, root, vi->visual, AllocNone);
      
      memset(&swa, 0, sizeof(swa));

      swa.event_mask = ExposureMask;
      swa.colormap = p->cmap;
      
      p->win = XCreateWindow(p->display, root, 0, 0, 10, 10, 0, vi->depth, InputOutput,
                             vi->visual, CWEventMask|CWColormap, &swa);

      p->glx_win = glXCreateWindow(p->display, p->fbcfg[0], p->win, NULL);
      
      XFree(vi);
      XSync(p->display, False);
      }
    
    glXMakeContextCurrent(p->display, p->glx_win, p->glx_win, p->ctx);
    }
  else
    glXMakeContextCurrent(p->display, drawable, drawable, p->ctx);
  
  }

void gavl_hw_glx_unset_current(gavl_hw_context_t * ctx)
  {
  glx_t * p = ctx->native;

  //  fprintf(stderr, "gavl_hw_glx_unset %p\n", ctx);

  glXMakeContextCurrent(p->display, None, None, NULL);
  }

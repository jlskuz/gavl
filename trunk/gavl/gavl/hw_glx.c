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
#include <string.h>

#include <X11/Xlib.h>


#include <gavl/gavl.h>
#include <gavl/hw_glx.h>

#include <hw_private.h>

static const struct
  {
  const gavl_pixelformat_t fmt;
  const GLenum format;
  const GLenum type;
  }
pixelformats[] =
  {
    { GAVL_RGB_24,     GL_RGB,  GL_BYTE },
    { GAVL_RGBA_32,    GL_RGBA, GL_BYTE },
    { GAVL_RGB_48,     GL_RGB,  GL_SHORT },
    { GAVL_RGBA_64,    GL_RGBA, GL_SHORT },
    { GAVL_RGB_FLOAT,  GL_RGB,  GL_FLOAT },
    { GAVL_RGBA_FLOAT, GL_RGBA, GL_FLOAT },
    { GAVL_PIXELFORMAT_NONE    /* End */ },
  };

#define NUM_PIXELFORMATS (sizeof(pixelformats)/sizeof(pixelformats[0]))

static int get_gl_format(gavl_pixelformat_t fmt, GLenum * format, GLenum * type)
  {
  int i = 0;

  while(pixelformats[i].fmt != GAVL_PIXELFORMAT_NONE)
    {
    if(pixelformats[i].fmt == fmt)
      {
      *format = pixelformats[i].format;
      *type   = pixelformats[i].type;
      return 1;
      }
    i++;
    }
  return 0;
  }

typedef struct
  {
  GLXFBConfig * fbcfg;

  GLXContext ctx;
  
  Display * private_display;
  Display * display;

  Window win;
  } glx_t;

static void destroy_native_glx(void * native)
  {
  glx_t * priv = native;

  if(priv->fbcfg)
    XFree(priv->fbcfg);

  if(priv->ctx)
    glXDestroyContext(priv->display, priv->ctx);

  if(priv->win != None)
    XDestroyWindow(priv->display, priv->win);
  
  if(priv->private_display)
    XCloseDisplay(priv->private_display);
  free(priv);

  }

static gavl_pixelformat_t * get_image_formats_glx(gavl_hw_context_t * ctx)
  {
  int idx = 0;
  
  gavl_pixelformat_t * ret;
  //  glx_t * priv = ctx->native;

  ret = calloc(NUM_PIXELFORMATS, sizeof(*ret));

  while(1)
    {
    ret[idx] = pixelformats[idx].fmt;
    if(pixelformats[idx].fmt == GAVL_PIXELFORMAT_NONE)
      break;
    }
  
  return ret;
  }

static gavl_pixelformat_t * get_overlay_formats_glx(gavl_hw_context_t * ctx)
  {
  int idx1 = 0;
  int idx2 = 0;
  
  gavl_pixelformat_t * ret;
  //  glx_t * priv = ctx->native;

  ret = calloc(NUM_PIXELFORMATS, sizeof(*ret));

  while(1)
    {
    if(pixelformats[idx1].fmt == GAVL_PIXELFORMAT_NONE)
      break;
    
    if(gavl_pixelformat_has_alpha(pixelformats[idx1].fmt))
      {
      ret[idx2] = pixelformats[idx1].fmt;
      idx2++;
      }
    idx1++;
    }

  ret[idx2] = GAVL_PIXELFORMAT_NONE;
  
  return ret;
  }
  
static void video_format_adjust_glx(gavl_hw_context_t * ctx,
                                    gavl_video_format_t * fmt)
  {
  gavl_video_format_set_frame_size(fmt, 8, 1);
  }

static void overlay_format_adjust_glx(gavl_hw_context_t * ctx,
                                      gavl_video_format_t * fmt)
  {
  gavl_video_format_set_frame_size(fmt, 8, 1);
  }

static gavl_video_frame_t * video_frame_create_hw_glx(gavl_hw_context_t * ctx,
                                                      gavl_video_format_t * fmt)
  {
  GLuint * tex;
  gavl_video_frame_t * ret;
  GLenum type = 0, format = 0;

  if(!get_gl_format(fmt->pixelformat, &format, &type))
    return 0;
  
  ret = gavl_video_frame_create(NULL);

  tex = calloc(1, sizeof(*tex));
  
  ret->user_data = tex;

  gavl_hw_glx_set_current(ctx, None);

  glGenTextures(1,tex);
  glBindTexture(GL_TEXTURE_2D, *tex);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA,
               fmt->image_width,
               fmt->image_height,
               0,
               format,
               type,
               NULL);
  
  gavl_hw_glx_unset_current(ctx);
  
  return NULL;
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
  gavl_video_frame_destroy(f);
  return;
  }

static int video_frame_to_ram_glx(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  GLuint * tex;
  GLenum type = 0, format = 0;

  get_gl_format(fmt->pixelformat, &format, &type);


  gavl_hw_glx_set_current(src->hwctx, None);

  tex = src->user_data;
  
  glBindTexture(GL_TEXTURE_2D, *tex);
  glPixelStorei(GL_PACK_ALIGNMENT, 8);

  glGetTexImage(GL_TEXTURE_2D,
                0, format, type, dst->planes[0]);
  
  gavl_hw_glx_unset_current(src->hwctx);
  
  return 1;
  }

static int video_frame_to_hw_glx(const gavl_video_format_t * fmt,
                                 gavl_video_frame_t * dst,
                                 gavl_video_frame_t * src)
  {
  GLuint * tex;
  GLenum type = 0, format = 0;

  get_gl_format(fmt->pixelformat, &format, &type);
  
  gavl_hw_glx_set_current(dst->hwctx, None);

  tex = dst->user_data;

  glBindTexture(GL_TEXTURE_2D, *tex);
  glPixelStorei(GL_PACK_ALIGNMENT, 8);
  
  glTexSubImage2D(GL_TEXTURE_2D,
                  0, 0, 0, fmt->image_width, fmt->image_height,
                  format, type, src->planes[0]);
  
  gavl_hw_glx_unset_current(dst->hwctx);
  
  return 1;
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native         = destroy_native_glx,
    .get_image_formats      = get_image_formats_glx,
    .get_overlay_formats    = get_overlay_formats_glx,
    .video_frame_create_hw  = video_frame_create_hw_glx,
    .video_frame_destroy    = video_frame_destroy_glx,
    .video_frame_to_ram     = video_frame_to_ram_glx,
    .video_frame_to_hw      = video_frame_to_hw_glx,
    .video_format_adjust    = video_format_adjust_glx,
    .overlay_format_adjust  =  overlay_format_adjust_glx,
    
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

  if(drawable == None)
    {

    if(p->win == None)
      {
      XSetWindowAttributes swa;

      XVisualInfo * vi = glXGetVisualFromFBConfig(p->display, p->fbcfg[0]);

      memset(&swa, 0, sizeof(swa));
      swa.event_mask = ExposureMask;
      
      p->win = XCreateWindow(p->display, DefaultRootWindow(p->display), 0, 0, 10, 10, 0, vi->depth, InputOutput,
                             vi->visual, CWEventMask, &swa);
      
      
      XFree(vi);
      }
    
    glXMakeCurrent(p->display, p->win, p->ctx);
    }
  else
    glXMakeCurrent(p->display, drawable, p->ctx);
  
  }

void gavl_hw_glx_unset_current(gavl_hw_context_t * ctx)
  {
  glx_t * p = ctx->native;
  glXMakeCurrent(p->display, None, NULL);
  }

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

// #include <X11/Xlib.h>

#include <GL/gl.h>


#include <gavl/gavl.h>
#include <gavl/hw_gl.h>
#include <gavl/hw_egl.h>

#include <hw_private.h>


typedef struct
  {
  EGLDisplay display;
  EGLConfig  config;
  EGLContext  context;

  /* If off-screen rendering is used */
  EGLSurface surf;
  
  } egl_t;

static void destroy_native_egl(void * native)
  {
  egl_t * priv = native;

  if(priv->surf != EGL_NO_SURFACE)
    eglDestroySurface(priv->display,
                      priv->surf);

  eglDestroyContext(priv->display, priv->context);
  eglTerminate(priv->display);
  
  free(priv);
  }
  

static gavl_video_frame_t * video_frame_create_hw_egl(gavl_hw_context_t * ctx,
                                                      gavl_video_format_t * fmt)
  {
  GLuint * tex;
  gavl_video_frame_t * ret;
  
  //  fprintf(stderr, "video_frame_create_hw_egl\n");
  //  gavl_video_format_dump(fmt);
  
  ret = gavl_video_frame_create(NULL);

  tex = calloc(1, sizeof(*tex));
  
  gavl_hw_egl_set_current(ctx, EGL_NO_SURFACE);
  *tex = gavl_gl_create_texture(fmt);
  gavl_hw_egl_unset_current(ctx);

  ret->user_data = tex;
  
  return ret;
  }

static void video_frame_destroy_egl(gavl_video_frame_t * f)
  {
  if(f->user_data)
    {
    GLuint * tex = f->user_data;

    gavl_hw_egl_set_current(f->hwctx, EGL_NO_SURFACE);
    glDeleteTextures(1, tex);
    gavl_hw_egl_unset_current(f->hwctx);

    free(tex);
    
    }
  f->hwctx = NULL;
  gavl_video_frame_destroy(f);
  return;
  }

static int video_frame_to_ram_egl(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  GLuint * tex;

  gavl_hw_egl_set_current(src->hwctx, EGL_NO_SURFACE);

  tex = src->user_data;
  gavl_gl_frame_to_ram(fmt, dst, *tex);

  gavl_hw_egl_unset_current(src->hwctx);
  
  return 1;
  }

static int video_frame_to_hw_egl(const gavl_video_format_t * fmt,
                                 gavl_video_frame_t * dst,
                                 gavl_video_frame_t * src)
  {
  GLuint * tex;
  
  gavl_hw_egl_set_current(dst->hwctx, EGL_NO_SURFACE);

  tex = dst->user_data;
  gavl_gl_frame_to_hw(fmt, *tex, src);
  
  gavl_hw_egl_unset_current(dst->hwctx);
  
  return 1;
  }

static const gavl_hw_funcs_t funcs =
  {
    .destroy_native         = destroy_native_egl,
    .get_image_formats      = gavl_gl_get_image_formats,
    .get_overlay_formats    = gavl_gl_get_overlay_formats,
    .video_frame_create_hw  = video_frame_create_hw_egl,
    .video_frame_destroy    = video_frame_destroy_egl,
    .video_frame_to_ram     = video_frame_to_ram_egl,
    .video_frame_to_hw      = video_frame_to_hw_egl,
    .video_format_adjust    = gavl_gl_adjust_video_format,
    .overlay_format_adjust  = gavl_gl_adjust_video_format,
  };

gavl_hw_context_t * gavl_hw_ctx_create_egl(EGLint const * attrs)
  {
  egl_t * priv;

  EGLint num_configs = 0;
  
  priv = calloc(1, sizeof(*priv));

  if((priv->display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY)
    goto fail;

  eglInitialize(priv->display, NULL, NULL);
  
  if(eglChooseConfig(priv->display, attrs, &priv->config, 1, &num_configs) == EGL_FALSE)
    goto fail;
  
  if((priv->context = eglCreateContext(priv->display,
                                       priv->config,
                                       EGL_NO_CONTEXT,
                                       NULL)) == EGL_NO_CONTEXT)
    goto fail;

  priv->surf = EGL_NO_SURFACE;
  
  return gavl_hw_context_create_internal(priv, &funcs, GAVL_HW_EGL);
  
  fail:

  if(priv)
    destroy_native_egl(priv);

  return NULL;
  }

#if 0

Display * gavl_hw_ctx_egl_get_display(gavl_hw_context_t * ctx)
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
#endif


EGLint const surface_attribs[] =
  {
    EGL_WIDTH, 10,
    EGL_HEIGHT, 10,
    EGL_NONE
  };


void gavl_hw_egl_set_current(gavl_hw_context_t * ctx, EGLSurface surf)
  {
  egl_t * p = ctx->native;

  //  fprintf(stderr, "gavl_hw_glx_set_current %p %ld\n", ctx, drawable);
  
  if(surf == EGL_NO_SURFACE)
    {

    if(p->surf == EGL_NO_SURFACE)
      {
      p->surf = eglCreatePbufferSurface(p->display,
                                        p->config,
                                        surface_attribs); 
      }
    eglMakeCurrent(p->display, p->surf, p->surf, p->context);
    
    }
  else
    eglMakeCurrent(p->display, surf, surf, p->context);
  
  }

void gavl_hw_egl_unset_current(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;

  //  fprintf(stderr, "gavl_hw_glx_unset %p\n", ctx);

  eglMakeCurrent(p->display, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE);
  }

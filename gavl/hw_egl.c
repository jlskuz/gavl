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

#include <config.h>

#ifdef HAVE_XLIB
#include <X11/Xlib.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

#define EGL_EGLEXT_PROTOTYPES

#include <EGL/egl.h>
#include <EGL/eglext.h>


#include <gavl/gavl.h>
#include <gavl/hw_gl.h>
#include <gavl/hw_egl.h>

#include <gavl/log.h>
#define LOG_DOMAIN "egl"

#include <hw_private.h>

#ifndef EGL_PLATFORM_X11_EXT
#define EGL_PLATFORM_X11_EXT 0x31D5
#endif

#ifdef HAVE_DRM_DRM_FOURCC_H 
#include <drm/drm_fourcc.h>
#define HAVE_DRM
#else // !HAVE_DRM_DRM_FOURCC_H 

#ifdef HAVE_LIBDRM_DRM_FOURCC_H 
#include <libdrm/drm_fourcc.h>
#define HAVE_DRM
#endif

#endif // !HAVE_DRM_DRM_FOURCC_H 

#ifdef HAVE_DRM
#include <gavl/hw_v4l2.h>
#endif

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

typedef struct
  {
  EGLDisplay display;
  EGLConfig  config;
  EGLContext  context;

  /* If off-screen rendering is used */
  EGLSurface surf;

  /* Current rendering target */
  EGLSurface current_surf;
  
  void * native_display;
  void * native_display_priv;

  /* Function pointers */

  /* Originally the last argument the next 2 functions  const EGLAttrib*.
   * We change it to void* to enable compilation against older GLE headers
   * (e.g. on Raspbian).
   * We don't use them anyway.
   */
  
  EGLDisplay (*eglGetPlatformDisplay)(EGLenum, void *, const void*);
  EGLSurface (*eglCreatePlatformWindowSurface)(EGLDisplay, EGLConfig, void *, const void*);

  void * (*eglCreateImage)(EGLDisplay dpy, EGLContext ctx, EGLenum target,
                           EGLClientBuffer buffer, const EGLint *attrib_list);
  
  EGLBoolean (*eglDestroyImage)(EGLDisplay dpy, void * image);

  void (*glEGLImageTargetTexture2DOES)(GLenum target, void * image);
  
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

static const EGLint
gles_attributes[] =
  {
#ifdef EGL_CONTEXT_MAJOR_VERSION
   EGL_CONTEXT_MAJOR_VERSION, 3,
#else
   EGL_CONTEXT_CLIENT_VERSION, 3,
#endif
   EGL_NONE
  };

#if 0
static void dump_dma_buf_formats(EGLDisplay  dpy)
  {
  //  EGL_EXT_image_dma_buf_import_modifiers 
  //  EGLBoolean  eglQueryDmaBufFormatsEXT (EGLDisplay  dpy, EGLint  max_formats, EGLint  *formats, EGLint  *num_formats)
  }
#endif

gavl_hw_context_t * gavl_hw_ctx_create_egl(EGLint const * attrs, gavl_hw_type_t type, void * native_display)
  {
  egl_t * priv;

  EGLint num_configs = 0;

  void * native_display_priv = NULL;
  EGLenum platform;

  const EGLint * attributes = NULL;
  
  //  
  
  switch(type)
    {
    case GAVL_HW_NONE:  // Autodetect
      break;
    case GAVL_HW_EGL_GLES_X11:  // X11
      eglBindAPI(EGL_OPENGL_ES_API);
      attributes = gles_attributes;

#ifdef HAVE_XLIB
      if(!native_display)
        {
        native_display_priv = XOpenDisplay(NULL);
        native_display = native_display_priv;
        }
      /* X11 connection failed */
      if(!native_display)
        return NULL;
      platform = EGL_PLATFORM_X11_EXT;
#else
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Compiled without XLIB support");
      return NULL;
#endif
      break;
    case GAVL_HW_EGL_GL_X11:  // X11
      eglBindAPI(EGL_OPENGL_API);
#ifdef HAVE_XLIB
      if(!native_display)
        {
        native_display_priv = XOpenDisplay(NULL);
        native_display = native_display_priv;
        }
      /* X11 connection failed */
      if(!native_display)
        return NULL;
      platform = EGL_PLATFORM_X11_EXT;
#else
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Compiled without XLIB support");
      return NULL;
#endif
      break;
    default:
      return NULL;
    }
  
  priv = calloc(1, sizeof(*priv));

  priv->native_display = native_display;
  priv->native_display_priv = native_display_priv;

  priv->eglGetPlatformDisplay          = (void*)eglGetProcAddress("eglGetPlatformDisplayEXT");
  priv->eglCreatePlatformWindowSurface = (void*)eglGetProcAddress("eglCreatePlatformWindowSurfaceEXT");

  priv->eglCreateImage = (void*)eglGetProcAddress("eglCreateImageKHR");
  priv->eglDestroyImage = (void*)eglGetProcAddress("eglDestroyImageKHR");

  priv->glEGLImageTargetTexture2DOES = (void*)eglGetProcAddress("glEGLImageTargetTexture2DOES");
  
  if((priv->display = priv->eglGetPlatformDisplay(platform, native_display, NULL)) == EGL_NO_DISPLAY)
    goto fail;

  if(eglInitialize(priv->display, NULL, NULL) == EGL_FALSE)
    goto fail;
  
  if(eglChooseConfig(priv->display, attrs, &priv->config, 1, &num_configs) == EGL_FALSE)
    goto fail;
  
  if((priv->context = eglCreateContext(priv->display,
                                       priv->config,
                                       EGL_NO_CONTEXT,
                                       attributes)) == EGL_NO_CONTEXT)
    goto fail;
  
  priv->surf = EGL_NO_SURFACE;
  
  return gavl_hw_context_create_internal(priv, &funcs, type);
  
  fail:

  if(priv)
    destroy_native_egl(priv);

  return NULL;
  }

void * gavl_hw_ctx_egl_get_native_display(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;
  
  return p->native_display;
  }

EGLSurface gavl_hw_ctx_egl_create_window_surface(gavl_hw_context_t * ctx, void * native_window)
  {
  egl_t * p = ctx->native;
  return p->eglCreatePlatformWindowSurface(p->display, p->config, native_window, NULL);
  }

void gavl_hw_ctx_egl_destroy_surface(gavl_hw_context_t * ctx, EGLSurface surf)
  {
  egl_t * p = ctx->native;
  eglDestroySurface(p->display, surf);
  }

EGLDisplay gavl_hw_ctx_egl_get_egl_display(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;
  return p->display;
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
    p->current_surf = p->surf;
    
    }
  else
    p->current_surf = surf;
  
  if(!eglMakeCurrent(p->display, p->current_surf, p->current_surf, p->context))
    fprintf(stderr, "eglMakeCurrent failed\n");
  }

void gavl_hw_egl_unset_current(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;

  //  fprintf(stderr, "gavl_hw_glx_unset %p\n", ctx);

  eglMakeCurrent(p->display, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE);
  p->current_surf = EGL_NO_SURFACE;
  }

void gavl_hw_egl_swap_buffers(gavl_hw_context_t * ctx)
  {
  egl_t * p = ctx->native;
  
  eglSwapBuffers(p->display, p->current_surf);
 
  }

int gavl_hw_egl_import_v4l2_buffer(gavl_hw_context_t * ctx,
                                   const gavl_video_format_t * fmt,
                                   GLuint texture,
                                   gavl_video_frame_t * v4l2_frame)
  {
#ifdef HAVE_DRM

  egl_t * egl;
  
  EGLImageKHR image = EGL_NO_IMAGE_KHR;
  EGLint attrs[128];
  int aidx = 0;

  gavl_v4l2_buffer_t * buf = v4l2_frame->user_data;
  
  egl = ctx->native;
  
  attrs[aidx++] = EGL_WIDTH;
  attrs[aidx++] = fmt->image_width;
  attrs[aidx++] = EGL_HEIGHT;
  attrs[aidx++] = fmt->image_height;
  
  if(fmt->pixelformat == GAVL_YUV_420_P)
    {
    /* 420_P is planar in GAVL and elsewhere but in V4L2 all planes
       are in the first plane of the buffer. Probably an ugly hack to
       support YUV420 even in the non-planat API.
    */
    if(buf->num_planes == 1)
      {
      if(v4l2_frame->planes[1] < v4l2_frame->planes[2])
        {
        attrs[aidx++] = EGL_LINUX_DRM_FOURCC_EXT;
        attrs[aidx++] = DRM_FORMAT_YUV420;
        }
      else
        {
        attrs[aidx++] = EGL_LINUX_DRM_FOURCC_EXT;
        attrs[aidx++] = DRM_FORMAT_YVU420;
        }

      /* Plane 0 */
      attrs[aidx++] = EGL_DMA_BUF_PLANE0_FD_EXT;
      attrs[aidx++] = buf->planes[0].dma_fd;

      attrs[aidx++] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
      attrs[aidx++] = 0;

      attrs[aidx++] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
      attrs[aidx++] = v4l2_frame->strides[0];

      /* Plane 1 */
      attrs[aidx++] = EGL_DMA_BUF_PLANE1_FD_EXT;
      attrs[aidx++] = buf->planes[0].dma_fd;

      attrs[aidx++] = EGL_DMA_BUF_PLANE1_OFFSET_EXT;
      attrs[aidx++] = v4l2_frame->planes[1] - v4l2_frame->planes[0];
      
      attrs[aidx++] = EGL_DMA_BUF_PLANE1_PITCH_EXT;
      attrs[aidx++] = v4l2_frame->strides[1];
      
      /* Plane 2 */
      attrs[aidx++] = EGL_DMA_BUF_PLANE2_FD_EXT;
      attrs[aidx++] = buf->planes[0].dma_fd;

      attrs[aidx++] = EGL_DMA_BUF_PLANE2_OFFSET_EXT;
      attrs[aidx++] = v4l2_frame->planes[2] - v4l2_frame->planes[0];
      
      attrs[aidx++] = EGL_DMA_BUF_PLANE2_PITCH_EXT;
      attrs[aidx++] = v4l2_frame->strides[2];
      
      }
    else
      {
      /* TODO */
      
      }
    
    }
  else if(!gavl_pixelformat_is_planar(fmt->pixelformat))
    {
    /* TODO */
    }
  else
    {
    /* TODO */
    
    }

  /* Terminate attributes */

  attrs[aidx++] = EGL_NONE;

  eglGetError();
  
  image = egl->eglCreateImage(egl->display, EGL_NO_CONTEXT,
                              EGL_LINUX_DMA_BUF_EXT,
                              NULL, attrs);
  
  if(!image)
    {
    fprintf(stderr, "Creating Image failed %08x\n", eglGetError());
    return 0;
    }
  gavl_hw_egl_set_current(ctx, EGL_NO_SURFACE);


  /* Associate the texture with the dma buffer */
  glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture);
  egl->glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
  
  /* Destroy image */

  egl->eglDestroyImage(egl->display, image);
  
  gavl_hw_egl_unset_current(ctx);
  
  return 1;
  
#else
  return 0;
#endif
  
  }



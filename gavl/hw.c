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
#include <hw_private.h>

gavl_hw_context_t * gavl_hw_context_create_internal(void * native,
                                                    const gavl_hw_funcs_t * funcs,
                                                    gavl_hw_type_t type)
  {
  gavl_hw_context_t * ret = calloc(1, sizeof(*ret));
  ret->type = type;
  ret->native = native;
  ret->funcs = funcs;
  ret->image_formats = ret->funcs->get_image_formats(ret);
  ret->overlay_formats = ret->funcs->get_overlay_formats(ret);
  return ret;
  }

void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx)
  {
  if(ctx->funcs->destroy_native)
    ctx->funcs->destroy_native(ctx->native);

  if(ctx->image_formats)
    free(ctx->image_formats);
  if(ctx->overlay_formats)
    free(ctx->overlay_formats);
  
  free(ctx);
  }

const gavl_pixelformat_t *
gavl_hw_ctx_get_image_formats(gavl_hw_context_t * ctx)
  {
  return ctx->image_formats;
  }

const gavl_pixelformat_t *
gavl_hw_ctx_get_overlay_formats(gavl_hw_context_t * ctx)
  {
  return ctx->overlay_formats;
  }


void * gavl_hw_ctx_get_native_handle(gavl_hw_context_t * ctx)
  {
  return ctx->native;
  }

void gavl_hw_video_format_adjust(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt)
  {
  fmt->pixelformat = gavl_pixelformat_get_best(fmt->pixelformat,
                                               gavl_hw_ctx_get_image_formats(ctx),
                                               NULL);

  if(ctx->funcs->video_format_adjust)
    ctx->funcs->video_format_adjust(ctx, fmt);
  }

/* Create a video frame. The frame will be a reference for a hardware surface */
gavl_video_frame_t * gavl_hw_video_frame_create_hw(gavl_hw_context_t * ctx,
                                                   gavl_video_format_t * fmt)
  {
  gavl_video_frame_t * ret;
  gavl_hw_video_format_adjust(ctx, fmt);
  if(ctx->funcs->video_frame_create_hw)
    ret = ctx->funcs->video_frame_create_hw(ctx, fmt);
  else
    ret = gavl_video_frame_create(fmt);
  return ret;
  }

GAVL_PUBLIC gavl_video_frame_t * gavl_hw_video_frame_create_ovl(gavl_hw_context_t * ctx,
                                                                gavl_video_format_t * fmt)
  {
  gavl_video_frame_t * ret;
  gavl_hw_video_format_adjust(ctx, fmt);
  if(ctx->funcs->video_frame_create_ovl)
    ret = ctx->funcs->video_frame_create_ovl(ctx, fmt);
  else
    ret = gavl_video_frame_create(fmt);
  return ret;
  }

/* Create a video frame. The frame will have data available for CPU access but is
 suitable for transfer to a hardware surface */
gavl_video_frame_t * gavl_hw_video_frame_create_ram(gavl_hw_context_t * ctx,
                                                    gavl_video_format_t * fmt)
  {
  gavl_video_frame_t * ret;
  gavl_hw_video_format_adjust(ctx, fmt);

  if(ctx->funcs->video_frame_create_ram)
    ret = ctx->funcs->video_frame_create_ram(ctx, fmt);
  else
    ret = gavl_video_frame_create(fmt);

  return ret;
  }

/* Load a video frame from RAM into the hardware */
int gavl_video_frame_ram_to_hw(const gavl_video_format_t * fmt,
                                gavl_video_frame_t * dst,
                                gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = dst->hwctx;
  gavl_video_frame_copy_metadata(dst, src);
  return ctx->funcs->video_frame_to_hw ?
    ctx->funcs->video_frame_to_hw(fmt, dst, src) : 0;
  }

/* Load a video frame from the hardware into RAM */
int gavl_video_frame_hw_to_ram(const gavl_video_format_t * fmt,
                               gavl_video_frame_t * dst,
                               gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = dst->hwctx;
  gavl_video_frame_copy_metadata(dst, src);
  return ctx->funcs->video_frame_to_ram ?
    ctx->funcs->video_frame_to_ram(fmt, dst, src) : 0;
  }


void 
gavl_hw_destroy_video_frame(gavl_hw_context_t * ctx,
                            gavl_video_frame_t * frame)
  {
  if(ctx->funcs->video_frame_destroy)
    ctx->funcs->video_frame_destroy(frame);
  else
    {
    frame->hwctx = NULL;
    gavl_video_frame_destroy(frame);
    }
  }

gavl_hw_type_t gavl_hw_ctx_get_type(gavl_hw_context_t * ctx)
  {
  return ctx->type;
  }

static const struct
  {
  gavl_hw_type_t type;
  const char * name;
  }
types[] = 
  {
    { GAVL_HW_GLX, "GLX Texture" },
    { GAVL_HW_VAAPI_X11, "vaapi through X11" },
    { /* End  */ },
  };
  
const char * gavl_hw_type_to_string(gavl_hw_type_t type)
  {
  int i = 0;

  while(types[i].name)
    {
    if(types[i].type == type)
      return types[i].name;
    i++;
    }
  return NULL;
  }

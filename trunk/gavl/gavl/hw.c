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
                                                    const gavl_hw_funcs_t * funcs)
  {
  gavl_hw_context_t * ret = calloc(1, sizeof(*ret));
  ret->native = native;
  ret->funcs = funcs;
  return ret;
  }

void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx)
  {
  if(ctx->funcs->destroy_native)
    ctx->funcs->destroy_native(ctx->native);

  if(ctx->pixelformats)
    free(ctx->pixelformats);
  
  free(ctx);
  }

const gavl_pixelformat_t * gavl_hw_ctx_get_pixelformats(gavl_hw_context_t * ctx)
  {
  if(!ctx->pixelformats)
    ctx->pixelformats = ctx->funcs->get_pixelformats(ctx);
  return ctx->pixelformats;
  }


void * gavl_hw_ctx_get_native_handle(gavl_hw_context_t * ctx)
  {
  return ctx->native;
  }

void gavl_hw_video_format_adjust(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt)
  {
  fmt->pixelformat = gavl_pixelformat_get_best(fmt->pixelformat,
                                               gavl_hw_ctx_get_pixelformats(ctx),
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
void gavl_video_frame_ram_to_hw(const gavl_video_format_t * fmt,
                                gavl_video_frame_t * dst,
                                const gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = dst->hwctx;
  
  
  }

/* Load a video frame from the hardware into RAM */
void gavl_video_frame_hw_to_ram(const gavl_video_format_t * fmt,
                                gavl_video_frame_t * dst,
                                const gavl_video_frame_t * src)
  {
  gavl_hw_context_t * ctx = dst->hwctx;
  }


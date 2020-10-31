
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


#include <gavl/gavl.h>

#include <interface/mmal/mmal.h>
#include <gavl/hw_mmal.h>

#include <hw_private.h>


#if 1
static const struct
  {
  const gavl_pixelformat_t gavl;
  uint32_t mmal;
  }
pixelformats[] =
  {
    { GAVL_RGB_24,           MMAL_ENCODING_RGB24 },
    { GAVL_RGBA_32,          MMAL_ENCODING_RGBA  },
    { GAVL_BGR_24,           MMAL_ENCODING_BGR24 },
    { GAVL_BGR_32,           MMAL_ENCODING_BGR32 },
    { GAVL_YUV_420_P,        MMAL_ENCODING_I420  },
    { GAVL_YUY2,             MMAL_ENCODING_YUYV  },
    { GAVL_UYVY,             MMAL_ENCODING_UYVY  },
    { GAVL_PIXELFORMAT_NONE    /* End */ },
  };

#define NUM_PIXELFORMATS (sizeof(pixelformats)/sizeof(pixelformats[0]))
#endif

typedef struct
  {
  gavl_video_format_t fmt;
  MMAL_COMPONENT_T * hw_to_ram;
  MMAL_COMPONENT_T * ram_to_hw;
  } format_context_t;

typedef struct
  {
  format_context_t * formats;
  int formats_alloc;
  int num_formats;
  } mmal_priv_t;

static void video_format_adjust_mmal(gavl_hw_context_t * ctx,
                                     gavl_video_format_t * fmt)
  {
  /* Alignment numbers taken from libavcodec/mmaldec.c, function ffmal_copy_frame() */
  gavl_video_format_set_frame_size(fmt, 32, 16);
  }

static format_context_t * get_format_context(gavl_hw_context_t * ctx, const gavl_video_format_t * fmt1)
  {
  int i;
  format_context_t * ret;
  mmal_priv_t * priv = ctx->native;

  gavl_video_forma_t fmt;
  gavl_video_format_copy(&fmt, fmt1);
  video_format_adjust_mmal(ctx, &fmt);
  
  for(i = 0; i < priv->num_formats; i++)
    {

    if((priv->formats[i].frame_width == fmt.frame_width) &&
       (priv->formats[i].frame_height == fmt.frame_height) &&
       (priv->formats[i].pixelformat == fmt.pixelformat))
      {
      return &priv->formats[i];
      }
    }

  if(priv->num_formats == priv->formats_alloc)
    {
    priv->formats_alloc += 16;
    priv->formats = realloc(priv->formats, sizeof(*priv->formats) * (priv->formats_alloc - priv->num_formats));
    }

  ret = &priv->formats[priv->num_formats];
  priv->num_formats++;
  gavl_video_format_copy(&ret->fmt, &fmt);
  return ret;
  }

static gavl_pixelformat_t * get_image_formats_mmal(gavl_hw_context_t * ctx)
  {
  int idx = 0;
  
  gavl_pixelformat_t * ret;
  
  ret = calloc(NUM_PIXELFORMATS, sizeof(*ret));

  while(1)
    {
    ret[idx] = pixelformats[idx].gavl;
    if(pixelformats[idx].gavl == GAVL_PIXELFORMAT_NONE)
      break;
    idx++;
    }
  
  return ret;
  }

static gavl_pixelformat_t * get_overlay_formats_mmal(gavl_hw_context_t * ctx)
  {
  int idx1 = 0;
  int idx2 = 0;
  
  gavl_pixelformat_t * ret;
  
  ret = calloc(NUM_PIXELFORMATS, sizeof(*ret));

  while(1)
    {
    if(pixelformats[idx1].gavl == GAVL_PIXELFORMAT_NONE)
      break;
    
    if(gavl_pixelformat_has_alpha(pixelformats[idx1].gavl))
      {
      ret[idx2] = pixelformats[idx1].gavl;
      idx2++;
      }
    idx1++;
    }

  ret[idx2] = GAVL_PIXELFORMAT_NONE;
  
  return ret;
  }


#if 0
void gavl_mmal_set_frame(const gavl_video_format_t * fmt,
                         MMAL_BUFFER_HEADER_T * buf,
                         gavl_video_frame_t * ret)
  {
  /* Assume adjusted format */
  gavl_video_frame_set_planes(ret, fmt, buf->data + buf->type->video.offset[0]);
  ret->user_data = buf;
  }
#endif


#if 1
static void destroy_native_mmal(void * native)
  {
  int i;
  
  mmal_priv_t * p = native;

  for(i = 0; i < p->num_formats; i++)
    {
    if(p->formats[i].hw_to_ram)
      {
      mmal_component_destroy(p->formats[i].hw_to_ram);
      }
    if(p->formats[i].ram_to_hw)
      {
      mmal_component_destroy(p->formats[i].ram_to_hw);
      }
    }
  
  }
#endif

static int video_frame_to_ram_mmal(const gavl_video_format_t * fmt,
                                   gavl_video_frame_t * dst,
                                   gavl_video_frame_t * src)
  {
  format_context_t * fmtctx = get_format_context(src->hwctx, fmt);

  if(!fmtctx->hw_to_ram)
    {
    
    }
  
  //  MMAL_COMPONENT_DEFAULT_VIDEO_CONVERTER
  
  }

static int video_frame_to_hw_mmal(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  format_context_t * fmtctx = get_format_context(src->hwctx, fmt);

  if(!fmtctx->hw_to_ram)
    {
    
    }
  
  }



static const gavl_hw_funcs_t funcs =
  {
   .destroy_native = destroy_native_mmal,
   .get_image_formats = get_image_formats_mmal,
   .get_overlay_formats = get_overlay_formats_mmal,

   .video_frame_to_ram = video_frame_to_ram_mmal,
   .video_frame_to_hw  = video_frame_to_hw_mmal,
   
#if 0
    .video_frame_create_hw = gavl_vaapi_video_frame_create_hw,
    // .video_frame_create_ram = gavl_vaapi_video_frame_create_ram,
    .video_frame_create_ovl = gavl_vaapi_video_frame_create_ovl,
    .video_frame_destroy = gavl_vaapi_video_frame_destroy,
#endif
    .video_format_adjust  = video_format_adjust_mmal,
  };

gavl_hw_context_t * gavl_hw_ctx_create_mmal(void)
  {
  mmal_priv_t * priv = calloc(1, sizeof(*priv));
  
  return gavl_hw_context_create_internal(priv, &funcs, GAVL_HW_MMAL);
  }


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
#include <inttypes.h>

#include <hw_vaapi.h>

static struct
  {
  gavl_pixelformat_t pfmt;
  uint32_t fourcc;
  unsigned int rt_format;
  }
formats[] =
  {
    { GAVL_RGBA_32, VA_FOURCC('R','G','B','A'), VA_RT_FORMAT_RGB32 },
    { GAVL_YUV_420_P,  VA_FOURCC('I','4','2','0'), VA_RT_FORMAT_YUV420 },
    { GAVL_YUV_420_P,  VA_FOURCC('Y','V','1','2'), VA_RT_FORMAT_YUV420 },
    { GAVL_PIXELFORMAT_NONE, 0, 0 }, // End
  };

static gavl_pixelformat_t fourcc_to_pixelformat(uint32_t fourcc)
  {
  int i = 0;
  while(formats[i].pfmt != GAVL_PIXELFORMAT_NONE)
    {
    if(formats[i].fourcc == fourcc)
      return formats[i].pfmt;
    i++;
    }
  return GAVL_PIXELFORMAT_NONE;
  }

static unsigned int pixelformat_to_rt_format(gavl_pixelformat_t pfmt)
  {
  int i = 0;
  while(formats[i].pfmt != GAVL_PIXELFORMAT_NONE)
    {
    if(formats[i].pfmt == pfmt)
      return formats[i].rt_format;
    i++;
    }
  return 0;
  }

static VAImageFormat * pixelformat_to_image_format(gavl_hw_vaapi_t * priv,
                                                   gavl_pixelformat_t pfmt)
  {
  int i;
  for(i = 0; i < priv->num_image_formats; i++)
    {
    if(fourcc_to_pixelformat(priv->image_formats[i].fourcc) == pfmt)
      return &priv->image_formats[i];
    }
  return NULL;
  }


static gavl_video_frame_t * create_common(gavl_hw_context_t * ctx)
  {
  gavl_video_frame_t * ret;
  ret = gavl_video_frame_create(NULL);
  ret->hwctx = ctx;
  return ret;
  }

gavl_video_frame_t * gavl_vaapi_video_frame_create_hw(gavl_hw_context_t * ctx,
                                                      gavl_video_format_t * fmt)
  {
  VAStatus result;
  gavl_video_frame_t * ret = NULL;
  unsigned int rt_format;
  VASurfaceID * surf = NULL;
  /* Create surface */
  gavl_hw_vaapi_t * priv = ctx->native;

  if(!(rt_format = pixelformat_to_rt_format(fmt->pixelformat)))
    goto fail;
  
  surf = calloc(1, sizeof(*surf));
  
  if((result = vaCreateSurfaces(priv->dpy, rt_format, fmt->image_width, fmt->image_height, surf, 1, NULL, 0)) !=
     VA_STATUS_SUCCESS)
    goto fail;
  
  ret = create_common(ctx);
  ret->user_data = surf;
  
  return ret;

  fail:

  if(ret)
    {
    ret->hwctx = NULL;
    gavl_video_frame_destroy(ret);
    }

  if(surf)
    free(surf);
  return NULL;
  }

static int map_frame(gavl_hw_vaapi_t * priv, gavl_video_frame_t * f)
  {
  void * buf;
  uint8_t * buf_i;
  VAStatus result;
  VAImage * image;

  image = f->user_data;
  
  /* Map */
  if((result = vaMapBuffer(priv->dpy, image->buf, &buf)) != VA_STATUS_SUCCESS)
    {
    return 0;
    }

  buf_i = buf;
  
  f->planes[0] = buf_i + image->offsets[0];
  f->planes[1] = buf_i + image->offsets[1];
  f->planes[2] = buf_i + image->offsets[2];

  f->strides[0] = image->pitches[0];
  f->strides[1] = image->pitches[1];
  f->strides[2] = image->pitches[2];
  return 1;
  }

gavl_video_frame_t *
gavl_vaapi_video_frame_create_ram(gavl_hw_context_t * ctx,
                                  gavl_video_format_t * fmt)
  {
  VAImage * image;
  gavl_video_frame_t * ret;
  VAImageFormat * format;
  VAStatus result;
  
  gavl_hw_vaapi_t * priv = ctx->native;

  if(!(format = pixelformat_to_image_format(priv, fmt->pixelformat)))
    return NULL;
  
  ret = create_common(ctx);
  image = calloc(1, sizeof(*image));

  /* Create image */
  if((result = vaCreateImage(priv->dpy,
                             format,
                             fmt->image_width,
                             fmt->image_height,
                             image)) != VA_STATUS_SUCCESS)
    goto fail;

  ret->user_data = image;

  fprintf(stderr, "Created Image: %dx%d Datasize: %d, planes: %d\n",
          image->width, image->height, image->data_size, image->num_planes);
  fprintf(stderr, "Pitches: %d %d %d\n",
          image->pitches[0], 
          image->pitches[1], 
          image->pitches[2]);
  fprintf(stderr, "Offsets: %d %d %d\n",
          image->offsets[0], 
          image->offsets[1], 
          image->offsets[2]);
  fprintf(stderr, "Components: %c %c %c %c\n",
          image->component_order[0],
          image->component_order[1],
          image->component_order[2],
          image->component_order[3]);

  if(!map_frame(priv, ret))
    goto fail;
  
  return ret;
  
  fail:
  if(image)
    free(image);
  if(ret)
    {
    gavl_video_frame_null(ret);
    ret->hwctx = NULL;
    gavl_video_frame_destroy(ret);
    }
  return NULL;
  }

void gavl_vaapi_video_frame_destroy(gavl_video_frame_t * f)
  {
  gavl_hw_vaapi_t * priv = f->hwctx->native;
  
  if(f->planes[0])
    {
    VAImage * image = f->user_data;
    vaDestroyImage(priv->dpy, image->image_id);
    free(image);
    }
  else
    {
    
    }
  gavl_video_frame_null(f);
  f->hwctx = NULL;
  gavl_video_frame_destroy(f);
  }

int gavl_vaapi_video_frame_to_ram(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  VASurfaceID * surf;
  VAImage * image;
  VAStatus result;
  gavl_hw_vaapi_t * priv = src->hwctx->native;

  image = dst->user_data;
  surf = src->user_data;
  
  vaUnmapBuffer(priv->dpy, image->buf);

  if((result = vaGetImage(priv->dpy,
                          *surf,
                          0,
                          0,
                          fmt->image_width,
                          fmt->image_height,
                          image->image_id)) != VA_STATUS_SUCCESS)
    {
    return 0;
    }
  map_frame(priv, dst);
  return 1;
  }

int gavl_vaapi_video_frame_to_hw(const gavl_video_format_t * fmt,
                                 gavl_video_frame_t * dst,
                                 gavl_video_frame_t * src)
  {
  VASurfaceID * surf;
  VAImage * image;
  VAStatus result;
  gavl_hw_vaapi_t * priv = src->hwctx->native;

  image = src->user_data;
  surf = dst->user_data;
  
  vaUnmapBuffer(priv->dpy, image->buf);

  if((result = vaPutImage(priv->dpy,
                          *surf,
                          image->image_id,
                          0, // int  	src_x,
                          0, // int  	src_y,
                          fmt->image_width, // unsigned int  	src_width,
                          fmt->image_height, // unsigned int  	src_height,
                          0, // int  	dest_x,
                          0, // int  	dest_y,
                          fmt->image_width, // unsigned int  	dest_width,
                          fmt->image_height)) != VA_STATUS_SUCCESS) //unsigned int  	dest_height 
    {
    return 0;
    }
  map_frame(priv, src);
  return 1;
  }

gavl_pixelformat_t *
gavl_vaapi_get_pixelformats(gavl_hw_context_t * ctx)
  {
  int num, i;
  gavl_pixelformat_t * ret;
  gavl_hw_vaapi_t * p = ctx->native;
  int num_ret = 0;
  gavl_pixelformat_t fmt;
  int j;
  int take_it;
  
  num = vaMaxNumImageFormats(p->dpy);
  p->image_formats = malloc(num * sizeof(*p->image_formats));

  vaQueryImageFormats(p->dpy, p->image_formats, &p->num_image_formats);

  ret = calloc(p->num_image_formats + 1, sizeof(*ret));
  
  for(i = 0; i < p->num_image_formats; i++)
    {

    fprintf(stderr, "Format %d\n", i);
    fprintf(stderr, "  fourcc: %c%c%c%c\n",
            (p->image_formats[i].fourcc) & 0xff,
            (p->image_formats[i].fourcc >> 8) & 0xff,
            (p->image_formats[i].fourcc >> 16) & 0xff,
            (p->image_formats[i].fourcc >> 24) & 0xff);
    fprintf(stderr, "  byte_order:     %d\n", p->image_formats[i].byte_order);
    fprintf(stderr, "  bits_per_pixel: %d\n", p->image_formats[i].bits_per_pixel);
    fprintf(stderr, "  depth:          %d\n", p->image_formats[i].depth);
    fprintf(stderr, "  red_mask:       %08x\n", p->image_formats[i].red_mask);
    fprintf(stderr, "  green_mask:     %08x\n", p->image_formats[i].green_mask);
    fprintf(stderr, "  blue_mask:      %08x\n", p->image_formats[i].blue_mask);
    fprintf(stderr, "  alpha_mask:     %08x\n", p->image_formats[i].alpha_mask);
    
    if((fmt = fourcc_to_pixelformat(p->image_formats[i].fourcc)) == GAVL_PIXELFORMAT_NONE)
      continue;

    j = 0;
    take_it = 1;
    
    while(j < num_ret)
      {
      if(ret[j] == fmt)
        {
        take_it = 0;
        break;
        }
      j++;
      }

    if(take_it)
      {
      ret[num_ret] = fmt;
      num_ret++;
      }
    }
  ret[num_ret] = GAVL_PIXELFORMAT_NONE;
  return ret;
  }

void gavl_vaapi_cleanup(void * priv)
  {
  gavl_hw_vaapi_t * p = priv;
  if(p->image_formats)
    free(p->image_formats);
  if(p->dpy)
    vaTerminate(p->dpy);
  }
  
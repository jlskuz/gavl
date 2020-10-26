
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
#include <gavl/hw_mmal.h>
#include <hw_private.h>

#if 0
static const struct
  {
  const gavl_pixelformat_t gavl;
  uint32_t mmal_fourcc;
  }
pixelformats[] =
  {
    { GAVL_RGB_24,     GL_RGB,  GL_UNSIGNED_BYTE },
    { GAVL_RGBA_32,    GL_RGBA, GL_UNSIGNED_BYTE },
    { GAVL_RGB_48,     GL_RGB,  GL_SHORT },
    { GAVL_RGBA_64,    GL_RGBA, GL_SHORT },
    { GAVL_RGB_FLOAT,  GL_RGB,  GL_FLOAT },
    { GAVL_RGBA_FLOAT, GL_RGBA, GL_FLOAT },
    { GAVL_PIXELFORMAT_NONE    /* End */ },
  };

#define NUM_PIXELFORMATS (sizeof(pixelformats)/sizeof(pixelformats[0]))
#endif


#if 0
static void destroy_native(void * native)
  {
  /* Nothing */
  }
#endif


// static void get_image_formats

static const gavl_hw_funcs_t funcs =
  {
   //    .destroy_native = destroy_native,
#if 0
    .get_image_formats = gavl_vaapi_get_image_formats,
    .get_overlay_formats = gavl_vaapi_get_overlay_formats,
    .video_frame_create_hw = gavl_vaapi_video_frame_create_hw,
    .video_frame_create_ram = gavl_vaapi_video_frame_create_ram,
    .video_frame_create_ovl = gavl_vaapi_video_frame_create_ovl,
    .video_frame_destroy = gavl_vaapi_video_frame_destroy,
    .video_frame_to_ram = gavl_vaapi_video_frame_to_ram,
    .video_frame_to_hw  = gavl_vaapi_video_frame_to_hw,
    .video_format_adjust  = gavl_vaapi_video_format_adjust,
#endif
  };

gavl_hw_context_t * gavl_hw_ctx_create_mmal(void)
  {
  return gavl_hw_context_create_internal(NULL, &funcs, GAVL_HW_MMAL);
  }


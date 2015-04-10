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


// #include <hw_private.h>
#include <hw_vaapi.h>

gavl_video_frame_t * gavl_vaapi_video_frame_create_hw(gavl_hw_context_t * ctx,
                                                      gavl_video_format_t * fmt)
  {
  /* Create surface */
  gavl_hw_vaapi_t * priv = ctx->native;
  
  }

gavl_video_frame_t * gavl_vaapi_video_frame_create_ram(gavl_hw_context_t * ctx,
                                                       gavl_video_format_t * fmt)
  {
  /* Create image */
  gavl_hw_vaapi_t * priv = ctx->native;

  }

void gavl_vaapi_video_frame_destroy(gavl_video_frame_t * f)
  {

  }

void gavl_vaapi_video_frame_to_ram(const gavl_video_format_t * fmt,
                                   gavl_video_frame_t * dst,
                                   gavl_video_frame_t * src)
  {

  }

void gavl_vaapi_video_frame_to_hw(const gavl_video_format_t * fmt,
                                  gavl_video_frame_t * dst,
                                  gavl_video_frame_t * src)
  {
  
  }

gavl_pixelformat_t *
gavl_vaapi_get_pixelformats(gavl_hw_context_t * ctx)
  {
  int num, i;
  VAImageFormat *format_list;
  gavl_hw_vaapi_t * p = ctx->native;
  
  num = vaMaxNumImageFormats(p->dpy);
  format_list = malloc(num * sizeof(*format_list));

  vaQueryImageFormats(p->dpy, format_list, &num);


  for(i = 0; i < num; i++)
    {
    fprintf(stderr, "fourcc: %c%c%c%c\n",
            (format_list[i].fourcc >> 24) & 0xff,
            (format_list[i].fourcc >> 16) & 0xff,
            (format_list[i].fourcc >> 8) & 0xff,
            (format_list[i].fourcc) & 0xff);
    }
  return NULL;
  }

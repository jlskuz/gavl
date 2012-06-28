/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2011 Members of the Gmerlin project
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

struct gavl_video_source_s
  {
  gavl_video_format_t src_format;
  gavl_video_format_t dst_format;
  int src_flags;
  int dst_flags;

  gavl_video_source_func_t func;
  gavl_video_converter_t * cnv;
  int rescale_timestamps;
  
  int do_convert;
  
  void * priv;
  int stream;

  /* FPS Conversion */
  int64_t next_pts;

  int64_t fps_pts;
  int64_t fps_duration;
  
  gavl_video_frame_t * fps_frame;
  
  gavl_video_frame_pool_t * fp_in;
  gavl_video_frame_pool_t * fp_out;

  gavl_source_status_t (*read_video)(gavl_video_source_t * s,
                                     gavl_video_frame_t ** frame);
  };

gavl_video_source_t *
gavl_video_source_create(gavl_video_source_func_t func,
                         void * priv, int stream,
                         int src_flags,
                         const gavl_video_format_t * src_format)
  {
  gavl_video_source_t * ret = calloc(1, sizeof(*ret));

  ret->func = func;
  ret->priv = priv;
  ret->stream = stream;
  ret->src_flags = src_flags;
  gavl_video_format_copy(&ret->src_format, src_format);
  ret->cnv = gavl_video_converter_create();

  
  return ret;
  }

/* Called by the destination */

const gavl_video_format_t *
gavl_video_source_get_src_format(gavl_video_source_t * s)
  {
  return &s->src_format;
  }
  
const gavl_video_format_t *
gavl_video_source_get_dst_format(gavl_video_source_t * s)
  {
  return &s->dst_format;
  }

gavl_video_options_t * gavl_video_source_get_options(gavl_video_source_t * s)
  {
  return gavl_video_converter_get_options(s->cnv);
  }

static gavl_source_status_t
read_video_simple(gavl_video_source_t * s,
                  gavl_video_frame_t ** frame)
  {
  
  }

static gavl_source_status_t
read_video_cnv(gavl_video_source_t * s,
               gavl_video_frame_t ** frame)
  {
  gavl_source_status_t st;
  gavl_video_frame_t * in_frame = NULL;
  
  if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    in_frame = gavl_video_frame_pool_get(s->fp_in);

  if((st = s->func(s->priv, &in_frame, s->stream)) != GAVL_SOURCE_OK)
    return st;

  if(!(*frame))
    {
    if(s->fp_out)
      s->fp_out = gavl_video_frame_pool_create(NULL, &s->dst_format);
    *frame = gavl_video_frame_pool_get(s->fp_out);
    }
  gavl_video_convert(s->cnv, in_frame, *frame);
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_video_fps(gavl_video_source_t * s,
               gavl_video_frame_t ** frame)
  {
  int new_frame = 0;
  int expired = 0;
  gavl_source_status_t st;
  
  /* Read frame if necessary */
  if(!s->fps_frame)
    {
    if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
      s->fps_frame = gavl_video_frame_pool_get(s->fp_in);
    if((st = s->func(s->priv, &s->fps_frame, s->stream)) != GAVL_SOURCE_OK)
      return st;
    
    s->fps_pts      = s->fps_frame->timestamp;
    s->fps_duration = s->fps_frame->duration;
    
    s->next_pts = gavl_time_rescale(s->src_format.timescale,
                                    s->dst_format.timescale,
                                    s->fps_frame->timestamp);
    new_frame = 1;
    }
  
  /* Check if frame expired */
  while(gavl_time_rescale(s->src_format.timescale,
                          s->dst_format.timescale,
                          s->fps_pts + s->fps_duration) <= s->next_pts)
    {
    if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
      s->fps_frame = gavl_video_frame_pool_get(s->fp_in);
    
    if((st = s->func(s->priv, &s->fps_frame, s->stream)) != GAVL_SOURCE_OK)
      return st;

    s->fps_pts      = s->fps_frame->timestamp;
    s->fps_duration = s->fps_frame->duration;
    new_frame = 1;
    }
  
  s->fps_frame->timestamp = s->next_pts;
  s->fps_frame->duration  = s->dst_format.frame_duration;
  
  s->next_pts += s->dst_format.frame_duration;

  /* Check if frame will be expired next time */
  if(gavl_time_rescale(s->src_format.timescale,
                       s->dst_format.timescale,
                       s->fps_pts + s->fps_duration) <= s->next_pts)
    expired = 1; // Frame won't be used another time
  
  /* Now check what to do */
  
  if(*frame)
    {
    if(new_frame)
      {
      
      }
    }
  else
    {
    
    }
  return GAVL_SOURCE_OK;
  }
  
void gavl_video_source_set_dst(gavl_video_source_t * s, int dst_flags,
                               const gavl_video_format_t * dst_format)
  {
  int convert_fps;
  
  s->dst_flags = dst_flags;
  gavl_video_format_copy(&s->dst_format, dst_format);
  
  s->do_convert =
    gavl_video_converter_init(s->cnv, &s->src_format, &s->dst_format);
  
  s->rescale_timestamps = 0;

  convert_fps = 0;
  
  if(s->dst_format.framerate_mode == GAVL_FRAMERATE_CONSTANT)
    {
    if((s->src_format.framerate_mode != GAVL_FRAMERATE_CONSTANT) ||
       (s->src_format.timescale * s->dst_format.frame_duration !=
        s->dst_format.timescale * s->src_format.frame_duration))
      {
      convert_fps = 1;
      }
    }
  
  if(convert_fps)
    s->read_video = read_video_fps;
  else if(s->do_convert)
    s->read_video = read_video_cnv;
  else
    s->read_video = read_video_simple;
  
  if(!convert_fps)
    {
    if(s->src_format.timescale != s->src_format.timescale)
      s->rescale_timestamps = 1;
    }

  if(!(s->src_flags & GAVL_SOURCE_SRC_ALLOC))
    s->fp_in = gavl_video_frame_pool_create(NULL, &s->src_format);
  
  }
  
gavl_source_status_t gavl_video_source_read_frame(void * sp, int stream,
                                                  gavl_video_frame_t ** frame)
  {
  gavl_video_source_t * s = sp;
  
  if(!frame)
    {
    /* Skip one frame as cheaply as possible */
    }
  else
    return s->read_video(sp, frame);
  }


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

#include <config.h>
#include <gavl/gavl.h>
#include <gavl/metadata.h>
#include <gavl/metatags.h>
#include <gavl/utils.h>
#include <gavl/trackinfo.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* */

static gavl_dictionary_t *
get_stream_nc(gavl_dictionary_t * d, int i, const char * tag)
  {
  gavl_array_t * arr;
  gavl_value_t * val;

  if((arr = gavl_dictionary_get_array_nc(d, tag)) &&
     (val = gavl_array_get_nc(arr, i)))
    return gavl_value_get_dictionary_nc(val);

  return NULL; 
  }

static const gavl_dictionary_t *
get_stream(const gavl_dictionary_t * d, int i, const char * tag)
  {
  const gavl_array_t * arr;
  const gavl_value_t * val;

  if((arr = gavl_dictionary_get_array(d, tag)) &&
     (val = gavl_array_get(arr, i)))
    return gavl_value_get_dictionary(val);
  
  return NULL; 
  }

static const gavl_dictionary_t * get_stream_metadata(const gavl_dictionary_t * d, int i, const char * tag)
  {
  const gavl_dictionary_t * s;

  if(!(s = get_stream(d, i, tag)))
    return NULL;
  
  return gavl_dictionary_get_dictionary(s, GAVL_META_METADATA);
  }

static gavl_dictionary_t * get_stream_metadata_nc(gavl_dictionary_t * d, int i, const char * tag)
  {
  gavl_dictionary_t * s;

  if(!(s = get_stream_nc(d, i, tag)))
    return NULL;
  return gavl_dictionary_get_dictionary_nc(s, GAVL_META_METADATA);
  }

static int
dictionary_get_num_streams(const gavl_dictionary_t * d, const char * tag)
  {
  const gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array(d, tag)))
    return 0;
  return arr->num_entries;
  }

static gavl_dictionary_t *
append_stream(gavl_dictionary_t * d, const char * tag)
  {
  gavl_value_t val;
  gavl_array_t * arr;
  gavl_dictionary_t * s;
  gavl_dictionary_t * m;
  
  if(!(arr = gavl_dictionary_get_array_nc(d, tag)))
    {
    gavl_value_init(&val);
    gavl_value_set_array(&val);
    gavl_dictionary_set_nocopy(d, tag, &val);
    arr = gavl_dictionary_get_array_nc(d, tag);
    }

  gavl_value_init(&val);
  s = gavl_value_set_dictionary(&val);
  m = gavl_dictionary_get_dictionary_create(s, GAVL_META_METADATA);
  gavl_dictionary_set_int(m, GAVL_META_IDX, arr->num_entries);
  gavl_array_splice_val_nocopy(arr, arr->num_entries, 0, &val);

  return arr->entries[arr->num_entries-1].v.dictionary;
  }

static void delete_stream(gavl_dictionary_t * d, const char * tag, int idx)
  {
  gavl_array_t * arr;

  if(!(arr = gavl_dictionary_get_array_nc(d, tag)) ||
     (idx < 0) || (idx >= arr->num_entries))
    return;
  gavl_array_splice_val(arr, idx, 1, NULL);
  }

static void init_stream(gavl_dictionary_t * dict)
  {
  gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);
  }

/* Audio */

static void init_audio_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;
  
  init_stream(dict);

  gavl_value_init(&fmt_val);
  gavl_value_set_audio_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);
  }

gavl_dictionary_t * gavl_track_get_audio_stream_nc(gavl_dictionary_t * d, int i)
  {
  return get_stream_nc(d, i, GAVL_META_AUDIO_STREAMS);
  }

const gavl_dictionary_t * gavl_track_get_audio_stream(const gavl_dictionary_t * d, int i)
  {
  return get_stream(d, i, GAVL_META_AUDIO_STREAMS);
  }

int gavl_track_get_num_audio_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_AUDIO_STREAMS);
  }
  
gavl_dictionary_t * gavl_track_append_audio_stream(gavl_dictionary_t * d)
  {
  gavl_dictionary_t * s = append_stream(d, GAVL_META_AUDIO_STREAMS);
  init_audio_stream(s);
  return s;
  }

const gavl_dictionary_t * gavl_track_get_audio_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, stream, GAVL_META_AUDIO_STREAMS);
  }

gavl_dictionary_t * gavl_track_get_audio_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, stream, GAVL_META_AUDIO_STREAMS);
  }

const gavl_audio_format_t * gavl_track_get_audio_format(const gavl_dictionary_t * d, int stream)
  {
  const gavl_dictionary_t * s;
  const gavl_value_t * val;
    
  if(!(s = get_stream(d, stream, GAVL_META_AUDIO_STREAMS)) ||
     !(val = gavl_dictionary_get(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_audio_format(val);
  }

gavl_audio_format_t * gavl_track_get_audio_format_nc(gavl_dictionary_t * d, int stream)
  {
  gavl_dictionary_t * s;
  gavl_value_t * val;
  
  if(!(s = get_stream_nc(d, stream, GAVL_META_AUDIO_STREAMS)) ||
     !(val = gavl_dictionary_get_nc(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_audio_format_nc(val);
  }

void gavl_track_delete_audio_stream(gavl_dictionary_t * d, int stream)
  {
  delete_stream(d, GAVL_META_AUDIO_STREAMS, stream);
  }

/* Video */

static void init_video_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;

  init_stream(dict);

  gavl_value_init(&fmt_val);
  gavl_value_set_video_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);

  }

gavl_dictionary_t * gavl_track_get_video_stream_nc(gavl_dictionary_t * d, int i)
  {
  return get_stream_nc(d, i, GAVL_META_VIDEO_STREAMS);
  }

const gavl_dictionary_t * gavl_track_get_video_stream(const gavl_dictionary_t * d, int i)
  {
  return get_stream(d, i, GAVL_META_VIDEO_STREAMS);
  }
  
int gavl_track_get_num_video_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_VIDEO_STREAMS);
  }
  
gavl_dictionary_t * gavl_track_append_video_stream(gavl_dictionary_t * d)
  {
  gavl_dictionary_t * s = append_stream(d, GAVL_META_VIDEO_STREAMS);
  init_video_stream(s);
  return s;
  }

const gavl_dictionary_t * gavl_track_get_video_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, stream, GAVL_META_VIDEO_STREAMS);
  }

gavl_dictionary_t * gavl_track_get_video_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, stream, GAVL_META_VIDEO_STREAMS);
  }

const gavl_video_format_t * gavl_track_get_video_format(const gavl_dictionary_t * d, int stream)
  {
  const gavl_dictionary_t * s;
  const gavl_value_t * val;
    
  if(!(s = get_stream(d, stream, GAVL_META_VIDEO_STREAMS)) ||
     !(val = gavl_dictionary_get(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format(val);
  }

gavl_video_format_t * gavl_track_get_video_format_nc(gavl_dictionary_t * d, int stream)
  {
  gavl_dictionary_t * s;
  gavl_value_t * val;
  
  if(!(s = get_stream_nc(d, stream, GAVL_META_VIDEO_STREAMS)) ||
     !(val = gavl_dictionary_get_nc(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format_nc(val);
  }

void gavl_track_delete_video_stream(gavl_dictionary_t * d, int stream)
  {
  delete_stream(d, GAVL_META_VIDEO_STREAMS, stream);

  }


/* Text */

static void init_text_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;
  
  init_stream(dict);

  gavl_value_init(&fmt_val);
  gavl_value_set_video_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);
  }

gavl_dictionary_t * gavl_track_get_text_stream_nc(gavl_dictionary_t * d, int i)
  {
  return get_stream_nc(d, i, GAVL_META_TEXT_STREAMS);
  }

const gavl_dictionary_t * gavl_track_get_text_stream(const gavl_dictionary_t * d, int i)
  {
  return get_stream(d, i, GAVL_META_TEXT_STREAMS);
  }

int gavl_track_get_num_text_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_TEXT_STREAMS);

  }
  
gavl_dictionary_t * gavl_track_append_text_stream(gavl_dictionary_t * d)
  {
  gavl_dictionary_t * s =  append_stream(d, GAVL_META_TEXT_STREAMS);
  init_text_stream(s);
  return s;
  }

const gavl_dictionary_t * gavl_track_get_text_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, stream, GAVL_META_TEXT_STREAMS);
  }

gavl_dictionary_t * gavl_track_get_text_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, stream, GAVL_META_TEXT_STREAMS);
  }

void gavl_track_delete_text_stream(gavl_dictionary_t * d, int stream)
  {
  delete_stream(d, GAVL_META_TEXT_STREAMS, stream);

  }


/* Overlay */

static void init_overlay_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;

  init_stream(dict);

  gavl_value_init(&fmt_val);
  gavl_value_set_video_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);

  }

gavl_dictionary_t * gavl_track_get_overlay_stream_nc(gavl_dictionary_t * d, int i)
  {
  return get_stream_nc(d, i, GAVL_META_OVERLAY_STREAMS);
  }

const gavl_dictionary_t * gavl_track_get_overlay_stream(const gavl_dictionary_t * d, int i)
  {
  return get_stream(d, i, GAVL_META_OVERLAY_STREAMS);
  }

int gavl_track_get_num_overlay_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_OVERLAY_STREAMS);
  }
  
gavl_dictionary_t * gavl_track_append_overlay_stream(gavl_dictionary_t * d)
  {
  gavl_dictionary_t * s =  append_stream(d, GAVL_META_OVERLAY_STREAMS);
  init_overlay_stream(s);
  return s;
  }
     
const gavl_dictionary_t * gavl_track_get_overlay_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, stream, GAVL_META_OVERLAY_STREAMS);
  }

gavl_dictionary_t * gavl_track_get_overlay_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, stream, GAVL_META_OVERLAY_STREAMS);
  }

const gavl_video_format_t * gavl_track_get_overlay_format(const gavl_dictionary_t * d, int stream)
  {
  const gavl_dictionary_t * s;
  const gavl_value_t * val;
    
  if(!(s = get_stream(d, stream, GAVL_META_OVERLAY_STREAMS)) ||
     !(val = gavl_dictionary_get(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format(val);
  }

gavl_video_format_t * gavl_track_get_overlay_format_nc(gavl_dictionary_t * d, int stream)
  {
  gavl_dictionary_t * s;
  gavl_value_t * val;
  
  if(!(s = get_stream_nc(d, stream, GAVL_META_OVERLAY_STREAMS)) ||
     !(val = gavl_dictionary_get_nc(s, GAVL_META_STREAM_FORMAT)))
    return NULL;
  
  return gavl_value_get_video_format_nc(val);
  }

void gavl_track_delete_overlay_stream(gavl_dictionary_t * d, int stream)
  {
  delete_stream(d, GAVL_META_OVERLAY_STREAMS, stream);
  }

/* Text */

static void init_msg_stream(gavl_dictionary_t * dict)
  {
  gavl_value_t fmt_val;
  
  init_stream(dict);

  gavl_value_init(&fmt_val);
  gavl_value_set_video_format(&fmt_val);
  gavl_dictionary_set_nocopy(dict, GAVL_META_STREAM_FORMAT, &fmt_val);
  }

gavl_dictionary_t * gavl_track_get_msg_stream_nc(gavl_dictionary_t * d, int i)
  {
  return get_stream_nc(d, i, GAVL_META_MSG_STREAMS);
  }

const gavl_dictionary_t * gavl_track_get_msg_stream(const gavl_dictionary_t * d, int i)
  {
  return get_stream(d, i, GAVL_META_MSG_STREAMS);
  }

int gavl_track_get_num_msg_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_MSG_STREAMS);

  }
  
gavl_dictionary_t * gavl_track_append_msg_stream(gavl_dictionary_t * d)
  {
  gavl_dictionary_t * s =  append_stream(d, GAVL_META_MSG_STREAMS);
  init_msg_stream(s);
  return s;
  }

const gavl_dictionary_t * gavl_track_get_msg_metadata(const gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata(d, stream, GAVL_META_MSG_STREAMS);
  }

gavl_dictionary_t * gavl_track_get_msg_metadata_nc(gavl_dictionary_t * d, int stream)
  {
  return get_stream_metadata_nc(d, stream, GAVL_META_MSG_STREAMS);
  }

void gavl_track_delete_msg_stream(gavl_dictionary_t * d, int stream)
  {
  delete_stream(d, GAVL_META_MSG_STREAMS, stream);

  }



/* Track */

static void track_init(gavl_dictionary_t * track, int idx)
  {
  gavl_dictionary_t * m;
  m = gavl_dictionary_get_dictionary_create(track, GAVL_META_METADATA);
  gavl_dictionary_set_int(m, GAVL_META_IDX, idx);
  }

gavl_array_t * gavl_get_tracks_nc(gavl_dictionary_t * dict)
  {
  return gavl_dictionary_get_array_create(dict, GAVL_META_TRACKS);
  }

const gavl_array_t * gavl_get_tracks(const gavl_dictionary_t * dict)
  {
  return gavl_dictionary_get_array(dict, GAVL_META_TRACKS);
  }

gavl_dictionary_t * gavl_append_track(gavl_dictionary_t * dict)
  {
  int num;
  gavl_dictionary_t * new_track;
  gavl_value_t val;
  gavl_array_t * arr = gavl_get_tracks_nc(dict);

  num = arr->num_entries;
  
  gavl_value_init(&val);
  new_track = gavl_value_set_dictionary(&val);
  
  track_init(new_track, num);
  gavl_array_push_nocopy(arr, &val);
  gavl_track_update_children(dict);
  return arr->entries[num].v.dictionary;
  }

const gavl_dictionary_t * gavl_get_track(const gavl_dictionary_t * dict, int idx)
  {
  const gavl_value_t * val;
  const gavl_array_t * tracks;

  if((idx < 0) && !gavl_dictionary_get_int(dict, GAVL_META_CURIDX, &idx))
    idx = 0;
  
  if(!(tracks = gavl_get_tracks(dict)) ||
     !(val = gavl_array_get(tracks, idx)))
    return NULL;
  return gavl_value_get_dictionary(val);
  }

gavl_dictionary_t * gavl_get_track_nc(gavl_dictionary_t * dict, int idx)
  {
  gavl_value_t * val;
  gavl_array_t * tracks;

  if((idx < 0) && !gavl_dictionary_get_int(dict, GAVL_META_CURIDX, &idx))
    idx = 0;

  if(!(tracks = gavl_get_tracks_nc(dict)) ||
     !(val = gavl_array_get_nc(tracks, idx)))
    return NULL;
  return gavl_value_get_dictionary_nc(val);
  }

void gavl_set_current_track(gavl_dictionary_t * dict, int idx)
  {
  gavl_dictionary_set_int(dict, GAVL_META_CURIDX, idx);
  }

int gavl_get_num_tracks(const gavl_dictionary_t * dict)
  {
  const gavl_array_t * tracks;
  if(!(tracks = gavl_get_tracks(dict)))
    return 0;
  return tracks->num_entries;
  }

int gavl_get_num_tracks_loaded(const gavl_dictionary_t * dict,
                               int * total)
  {
  int ret;
  int num_children = 0;

  const gavl_array_t * tracks;
  const gavl_dictionary_t * m;

  if(!(tracks = gavl_get_tracks(dict)))
    ret = 0;
  else
    ret = tracks->num_entries;
  
  if(!(m = gavl_dictionary_get_dictionary(dict, GAVL_META_METADATA)) ||
     !gavl_dictionary_get_int(m, GAVL_META_NUM_CHILDREN, &num_children))
    *total = 0;
  else
    *total = num_children;
  
  return ret;
  }

void gavl_delete_track(gavl_dictionary_t * dict, int idx)
  {
  gavl_track_splice_children(dict, idx, 1, NULL);
  }

void gavl_track_splice_children(gavl_dictionary_t * dict, int idx, int del,
                                const gavl_value_t * val)
  {
  gavl_array_t * tracks;
  if(!(tracks = gavl_get_tracks_nc(dict)))
    return;
  
  if(!val || (val->type == GAVL_TYPE_DICTIONARY) || (val->type == GAVL_TYPE_UNDEFINED))
    {
    gavl_array_splice_val(tracks, idx, del, val);
    }
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    gavl_array_splice_array(tracks, idx, del, val->v.array);
    }
  gavl_track_update_children(dict);
  }

void gavl_track_splice_children_nocopy(gavl_dictionary_t * dict, int idx, int del,
                                       gavl_value_t * val)
  {
  gavl_array_t * tracks;
  if(!(tracks = gavl_get_tracks_nc(dict)))
    return;
  
  if(!val || (val->type == GAVL_TYPE_DICTIONARY) || (val->type == GAVL_TYPE_UNDEFINED))
    {
    gavl_array_splice_val_nocopy(tracks, idx, del, val);
    }
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    gavl_array_splice_array_nocopy(tracks, idx, del, val->v.array);
    }
  gavl_track_update_children(dict);
  gavl_value_reset(val);
  }


gavl_audio_format_t * gavl_stream_get_audio_format_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_audio_format_nc(val);
  else
    return NULL;
  }

gavl_video_format_t * gavl_stream_get_video_format_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_video_format_nc(val);
  else
    return NULL;
  }

const gavl_audio_format_t * gavl_stream_get_audio_format(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_audio_format(val);
  else
    return NULL;
  }

const gavl_video_format_t * gavl_stream_get_video_format(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_STREAM_FORMAT)))
    return gavl_value_get_video_format(val);
  else
    return NULL;
  }

gavl_dictionary_t * gavl_stream_get_metadata_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary_nc(val);
  else
    return NULL;
  }


const gavl_dictionary_t * gavl_stream_get_metadata(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary(val);
  else
    return NULL;
  }

gavl_dictionary_t * gavl_track_get_metadata_nc(gavl_dictionary_t * dict)
  {
  gavl_value_t * val;
  if((val = gavl_dictionary_get_nc(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary_nc(val);
  else
    return NULL;
  }


const gavl_dictionary_t * gavl_track_get_metadata(const gavl_dictionary_t * dict)
  {
  const gavl_value_t * val;
  if((val = gavl_dictionary_get(dict, GAVL_META_METADATA)))
    return gavl_value_get_dictionary(val);
  else
    return NULL;
  }


static void get_stream_duration(void * priv, 
                                int idx,
                                const gavl_value_t * v)
  {
  gavl_time_t * t;
  gavl_time_t test_duration;
  const gavl_dictionary_t * s;
  const gavl_dictionary_t * m;
  int timescale;
  int64_t stream_duration;
  
  t = priv;
  
  if(!(s = gavl_value_get_dictionary(v)))
    return;
  
  if(!(m = gavl_dictionary_get_dictionary(s, GAVL_META_METADATA)))
    return;
    
  if(!(gavl_dictionary_get_int(m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &timescale)))
    return;
    
  if((timescale <= 0))
    return;
  
  if(!(gavl_dictionary_get_long(m, GAVL_META_STREAM_DURATION, &stream_duration)))
    return;
  
  test_duration = gavl_time_unscale(timescale, stream_duration);
  if(test_duration > *t)
    *t = test_duration;
  }


void gavl_track_compute_duration(gavl_dictionary_t * dict)
  {
  gavl_array_t * arr;  
  gavl_time_t dur = 0;
  gavl_dictionary_t * m = gavl_track_get_metadata_nc(dict);
  
  if(gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &dur) && (dur > 0))
    return;
  
  if((arr = gavl_dictionary_get_array_nc(dict, GAVL_META_AUDIO_STREAMS)))
    gavl_array_foreach(arr, get_stream_duration, &dur);
  
  if((arr = gavl_dictionary_get_array_nc(dict, GAVL_META_VIDEO_STREAMS)))
    gavl_array_foreach(arr, get_stream_duration, &dur);
  
  if((arr = gavl_dictionary_get_array_nc(dict, GAVL_META_TEXT_STREAMS)))
    gavl_array_foreach(arr, get_stream_duration, &dur);
  
  if((arr = gavl_dictionary_get_array_nc(dict, GAVL_META_OVERLAY_STREAMS)))
    gavl_array_foreach(arr, get_stream_duration, &dur);

  if(dur > 0)
    gavl_track_set_duration(dict, dur);
  
  }

/* Detect various patterns in filenames */

static int is_skip_char(char c)
  {
  return isspace(c) || (c == '_') || (c == '-');
  }

static const char * detect_date(const char * filename, gavl_dictionary_t * metadata)
  {
  int year;
  int month;
  int day;
  
  const char * pos = strrchr(filename, '(');
  if(!pos)
    return NULL;

  year = 9999;
  month = 99;
  day = 99;
  
  if(sscanf(pos, "(%d-%d-%d)", &year, &month, &day) < 3)
    {
    month = 99;
    day = 99;

    if(sscanf(pos, "(%d)", &year) < 1)
      {
      year = 9999;
      return NULL;
      }
    }
  pos--;
  while(is_skip_char(*pos) && (pos > filename))
    pos--;
  pos++;

  gavl_dictionary_set_date(metadata, GAVL_META_DATE, year, month, day);
  
  return pos;
  }

const char * gavl_detect_episode_tag(const char * filename, const char * end, 
                                     int * season_p, int * idx_p)
  {
  const char * pos;
  int season, idx;
  if(!end)
    end = filename + strlen(filename);

  pos = filename;
  
  while(pos < end)
    {
    if((sscanf(pos, "S%dE%d", &season, &idx) == 2) ||
       (sscanf(pos, "s%de%d", &season, &idx) == 2))
      {
      if(season_p)
        *season_p = season;
      if(idx_p)
        *idx_p = idx;
      return pos;
      }
    pos++;
    }
  return NULL;
  }

/* "Show - S1E2 - Episode (1990)" */

static int detect_episode(const char * filename, gavl_dictionary_t * dict)
  {
  int season = 0;
  int idx = 0;
  
  const char * tag;
  const char * pos;
  const char * end = detect_date(filename, dict);
  if(!end)
    end = filename + strlen(filename);

  tag = gavl_detect_episode_tag(filename, end, &season, &idx);
  if(!tag)
    return 0;

  pos = tag;
  while(!is_skip_char(*pos) && (pos < end))
    pos++;
  if(pos == end)
    return 0;
  while(is_skip_char(*pos) && (pos < end))
    pos++;

  if(pos == end)
    return 0;

  gavl_dictionary_set_string_nocopy(dict, GAVL_META_TITLE, gavl_strndup(pos, end));
  
  pos = tag;
  pos--;

  while(is_skip_char(*pos) && (pos > filename))
    pos--;

  if(pos == filename)
    {
    gavl_dictionary_set(dict, GAVL_META_TITLE, NULL);
    return 0;
    }
  pos++;
  
  gavl_dictionary_set_string_nocopy(dict, GAVL_META_SHOW, gavl_strndup(filename, pos));
  gavl_dictionary_set_int(dict, GAVL_META_SEASON, season);
  gavl_dictionary_set_int(dict, GAVL_META_IDX, idx-1);
  return 1;  
  }




/* "Movie title (1990)" */

static int detect_movie_singlefile(const char * filename, gavl_dictionary_t * dict)
  {
  const char * end;

  if(!(end = detect_date(filename, dict)))
    end = filename + strlen(filename);
  
  gavl_dictionary_set_string_nocopy(dict, GAVL_META_TITLE, gavl_strndup(filename, end));
  return 1;
  }

/* "Movie title (1990) CD1" */

static char * detect_multipart_tag(char * filename, int * part)
  {
  char * pos = filename + strlen(filename) - 3;

  while(pos > filename)
    {
    if(!strncasecmp(pos, "CD", 2) && isdigit(pos[2]))
      {
      if(part)
        *part = atoi(pos+2);
      return pos;
      }
    else if(!strncasecmp(pos, "part", 4) && isdigit(pos[4]))
      {
      if(part)
        *part = atoi(pos+4);
      return pos;
      }
    pos--;
    }
  return NULL;
  }

static int detect_movie_multifile(char * basename, gavl_dictionary_t * dict)
  {
  int idx = 0;
  char * pos;
  const char * end = basename + strlen(basename);
  
  pos = detect_multipart_tag(basename, &idx);
  
  if(!pos)
    return 0;
  
  end = detect_date(basename, dict);
  if(!end)
    {
    end = pos;
    end--;
 
    while(is_skip_char(*end) && (end > basename))
      end--;
    end++;
    }
  
  gavl_dictionary_set_string_nocopy(dict, GAVL_META_TITLE, gavl_strndup(basename, end));
  
  gavl_dictionary_set_string(dict, GAVL_META_LABEL, basename);
  gavl_dictionary_set_int(dict, GAVL_META_IDX, idx);

  pos--;
  while(is_skip_char(*pos) && (pos > basename))
    pos--;
  pos++;
  *pos = '\0';
  return 1;
  }

static void finalize_audio(gavl_dictionary_t * dict)
  {
  const char * var;
  gavl_dictionary_t * m;
  const gavl_dictionary_t * sm;
  const gavl_audio_format_t * fmt;
  int val_i;
  
  if(!(m = gavl_track_get_metadata_nc(dict)) ||
     !(fmt = gavl_track_get_audio_format(dict, 0)) ||
     !(sm = gavl_track_get_audio_metadata(dict, 0)))
    return;
  
  if(fmt->num_channels)
    gavl_dictionary_set_int(m, GAVL_META_AUDIO_CHANNELS, fmt->num_channels);
  if(fmt->samplerate)
    gavl_dictionary_set_int(m, GAVL_META_AUDIO_SAMPLERATE, fmt->samplerate);
  
  if(gavl_dictionary_get_int(sm, GAVL_META_BITRATE, &val_i))
    gavl_dictionary_set_int(m, GAVL_META_AUDIO_BITRATE, val_i);
  
  if((var = gavl_dictionary_get_string(sm, GAVL_META_FORMAT)))
    gavl_dictionary_set_string(m, GAVL_META_AUDIO_CODEC, var);
  
  }

static void finalize_video(gavl_dictionary_t * dict)
  {
  const char * var;
  gavl_dictionary_t * m;
  const gavl_dictionary_t * sm;
  const gavl_video_format_t * fmt;
  
  if(!(m = gavl_track_get_metadata_nc(dict)) ||
     !(fmt = gavl_track_get_video_format(dict, 0)) ||
     !(sm = gavl_track_get_video_metadata(dict, 0)))
    return;

  if(fmt->image_width && fmt->image_height)
    {
    gavl_dictionary_set_int(m, GAVL_META_WIDTH, fmt->image_width);
    gavl_dictionary_set_int(m, GAVL_META_HEIGHT, fmt->image_height);
    }

  if((var = gavl_dictionary_get_string(sm, GAVL_META_FORMAT)))
    gavl_dictionary_set_string(m, GAVL_META_VIDEO_CODEC, var);
  }

void gavl_track_finalize(gavl_dictionary_t * dict)
  {
  gavl_time_t duration = GAVL_TIME_UNDEFINED;
  
  const char * media_class = NULL;
  gavl_dictionary_t * m;
  
  int num_audio_streams;
  int num_video_streams;
  char * basename = NULL;

  const char * location = NULL;
  const char * pos1;
  const char * pos2;

  m = gavl_track_get_metadata_nc(dict);
  
  gavl_dictionary_get_src(m, GAVL_META_SRC, 0,
                          NULL, &location);
  
  if(location && (pos1 = strrchr(location, '/')) &&
     (pos2 = strrchr(location, '.')) && (pos2 > pos1))
    {
    basename = gavl_strndup(pos1+1, pos2);
    }
  
  num_audio_streams = gavl_track_get_num_audio_streams(dict);
  num_video_streams = gavl_track_get_num_video_streams(dict);
  
  /* Figure out the media type */
  if((num_audio_streams == 1) && !num_video_streams)
    {
    /* Audio file */
    media_class = GAVL_META_MEDIA_CLASS_AUDIO_FILE;
    }
  else if(num_video_streams >= 1)
    {
    const gavl_video_format_t * fmt;

    if(!num_audio_streams &&
       (num_video_streams == 1) &&
       (fmt = gavl_track_get_video_format(dict, 0)) && 
       (fmt->framerate_mode == GAVL_FRAMERATE_STILL))
      {
      /* Photo */
      media_class = GAVL_META_MEDIA_CLASS_IMAGE;
      }
    else
      {
      /* Video */
      media_class = GAVL_META_MEDIA_CLASS_VIDEO_FILE;
      }
    }

  gavl_track_compute_duration(dict);
  
  if(media_class)
    {
    if(!strcmp(media_class, GAVL_META_MEDIA_CLASS_AUDIO_FILE))
      {
      finalize_audio(dict);
      
      /* Check for audio broadcast */
      if(gavl_dictionary_get(m, GAVL_META_STATION))
        media_class = GAVL_META_MEDIA_CLASS_AUDIO_BROADCAST;

      /* Check for song */
      else if(gavl_dictionary_get(m, GAVL_META_ARTIST) &&
              gavl_dictionary_get(m, GAVL_META_TITLE) &&
              gavl_dictionary_get(m, GAVL_META_ALBUM))
        {
        media_class = GAVL_META_MEDIA_CLASS_SONG;
        }
      }
    else if(!strcmp(media_class, GAVL_META_MEDIA_CLASS_IMAGE))
      {
      
      }
    else if(!strcmp(media_class, GAVL_META_MEDIA_CLASS_VIDEO_FILE))
      {
      finalize_video(dict);
      if(num_audio_streams > 0)
        finalize_audio(dict);
      
      if((num_video_streams == 1) && gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &duration) && (duration > 0))
        {
        /* Check for episode */

        if(basename && detect_episode(basename, m))
          media_class = GAVL_META_MEDIA_CLASS_TV_EPISODE;
        /* Check for movie */
        else if(basename && detect_movie_multifile(basename, m))
          media_class = GAVL_META_MEDIA_CLASS_MOVIE_PART;
        else if(basename && detect_movie_singlefile(basename, m))
          media_class = GAVL_META_MEDIA_CLASS_MOVIE;
        }
      }
    }
  if(media_class && !gavl_dictionary_get_string(m, GAVL_META_MEDIA_CLASS))
    gavl_dictionary_set_string(m, GAVL_META_MEDIA_CLASS, media_class);
  
  if(basename)
    free(basename);
  }

gavl_time_t gavl_track_get_duration(const gavl_dictionary_t * dict)
  {
  gavl_time_t dur;
  const gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata(dict)))
    return GAVL_TIME_UNDEFINED;
  
  if(gavl_dictionary_get_long(m, GAVL_META_APPROX_DURATION, &dur) && (dur > 0))
    return dur;
  else
    return GAVL_TIME_UNDEFINED;
  }

void gavl_track_set_duration(gavl_dictionary_t * dict, gavl_time_t dur)
  {
  gavl_dictionary_t * m = gavl_track_get_metadata_nc(dict);
  gavl_dictionary_set_long(m, GAVL_META_APPROX_DURATION, dur);
  }

void gavl_track_set_num_audio_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  gavl_dictionary_set(dict, GAVL_META_AUDIO_STREAMS, NULL);

  for(i = 0; i < num; i++)
    gavl_track_append_audio_stream(dict);
 
  }

void gavl_track_set_num_video_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  gavl_dictionary_set(dict, GAVL_META_VIDEO_STREAMS, NULL);

  for(i = 0; i < num; i++)
    gavl_track_append_video_stream(dict);
  
  }

void gavl_track_set_num_text_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  gavl_dictionary_set(dict, GAVL_META_TEXT_STREAMS, NULL);

  for(i = 0; i < num; i++)
    gavl_track_append_text_stream(dict);
  
  }

void gavl_track_set_num_overlay_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  gavl_dictionary_set(dict, GAVL_META_OVERLAY_STREAMS, NULL);

  for(i = 0; i < num; i++)
    gavl_track_append_overlay_stream(dict);
  
  }

void gavl_track_set_num_msg_streams(gavl_dictionary_t * dict, int num)
  {
  int i;

  /* Delete previous streams */
  gavl_dictionary_set(dict, GAVL_META_MSG_STREAMS, NULL);

  for(i = 0; i < num; i++)
    gavl_track_append_msg_stream(dict);
  
  }


int gavl_track_can_seek(const gavl_dictionary_t * track)
  {
  int val = 0;
  const gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_CAN_SEEK, &val) ||
     !val)
    return 0;

  return 1;
  }

int gavl_track_can_pause(const gavl_dictionary_t * track)
  {
  int val = 0;
  const gavl_dictionary_t * m;

  if(!(m = gavl_track_get_metadata(track)) ||
     !gavl_dictionary_get_int(m, GAVL_META_CAN_PAUSE, &val) ||
     !val)
    return 0;

  return 1;
  
  }

#define META_GUI "gui"

void gavl_track_set_gui_state(gavl_dictionary_t * track, const char * state, int val)
  {
  int old_val = 0;
  gavl_dictionary_t * gui = gavl_dictionary_get_dictionary_create(track, META_GUI);

  gavl_dictionary_get_int(track, state, &old_val);
  
  if((val < 0) || (val > 1))
    val = !old_val;
  
  gavl_dictionary_set_int(gui, state, val);
  }

int gavl_track_get_gui_state(const gavl_dictionary_t * track, const char * state)
  {
  int ret = 0;
  const gavl_dictionary_t * gui = gavl_dictionary_get_dictionary(track, META_GUI);

  if(!gui || !gavl_dictionary_get_int(gui, state, &ret))
    return 0;
  
  return ret;
  }

void gavl_track_clear_gui_state(gavl_dictionary_t * track)
  {
  gavl_dictionary_set(track, META_GUI, NULL);
  }

void gavl_track_copy_gui_state(gavl_dictionary_t * dst, const gavl_dictionary_t * src)
  {
  gavl_dictionary_t * dst_gui;
  gavl_dictionary_t tmp;
  
  const gavl_dictionary_t * src_gui = gavl_dictionary_get_dictionary(src, META_GUI);

  if(!src_gui)
    return;
  
  dst_gui = gavl_dictionary_get_dictionary_create(dst, META_GUI);

  gavl_dictionary_init(&tmp);
  
  gavl_dictionary_merge(&tmp, src_gui, dst_gui);
  gavl_dictionary_reset(dst_gui);
  gavl_dictionary_move(dst_gui, &tmp);
  }

void gavl_track_update_children(gavl_dictionary_t * dict)
  {
  int i;
  gavl_dictionary_t * m;
  gavl_dictionary_t * track;
  gavl_array_t * arr;
  arr = gavl_get_tracks_nc(dict);
  m = gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);

  gavl_dictionary_set_int(m, GAVL_META_NUM_CHILDREN, arr->num_entries);

  for(i = 0; i < arr->num_entries; i++)
    {
    //    if(!())
    //      fprintf(stderr, "Blupp %d %p %d %p\n", i, arr, arr->num_entries, arr->entries);

    if((track = gavl_get_track_nc(dict, i)) &&
       (m = gavl_dictionary_get_dictionary_create(track, GAVL_META_METADATA)))
      gavl_dictionary_set_int(m, GAVL_META_IDX, i);
    }
  }

const char * gavl_track_get_id(const gavl_dictionary_t * dict)
  {
  const gavl_dictionary_t * m;

  if((m = gavl_track_get_metadata(dict)))
    return gavl_dictionary_get_string(m, GAVL_META_ID);
  else
    return NULL;
  }

void gavl_track_set_id_nocopy(gavl_dictionary_t * dict, char * id)
  {
  gavl_dictionary_t * m = gavl_dictionary_get_dictionary_create(dict, GAVL_META_METADATA);
  gavl_dictionary_set_string_nocopy(m, GAVL_META_ID, id);
  }

void gavl_track_set_id(gavl_dictionary_t * dict, const char * id)
  {
  gavl_track_set_id_nocopy(dict, gavl_strdup(id));
  }

int gavl_get_track_idx_by_id(const gavl_dictionary_t * dict, const char * id)
  {
  int i;
  const gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_id;

  int num = gavl_get_num_tracks(dict);
  
  for(i = 0; i < num; i++)
    {
    if((track = gavl_get_track(dict, i)) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_id = gavl_dictionary_get_string(m, GAVL_META_ID)) &&
       !strcmp(track_id, id))
      return i;
    }
  return -1;
  }

const gavl_dictionary_t * gavl_get_track_by_id(const gavl_dictionary_t * dict, const char * id)
  {
  int i;
  const gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_id;

  int num = gavl_get_num_tracks(dict);
  
  for(i = 0; i < num; i++)
    {
    if((track = gavl_get_track(dict, i)) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_id = gavl_dictionary_get_string(m, GAVL_META_ID)) &&
       !strcmp(track_id, id))
      return track;
    }
  return NULL;
  }

gavl_dictionary_t * gavl_get_track_by_id_nc(gavl_dictionary_t * dict, const char * id)
  {
  int i;
  gavl_dictionary_t * track;
  const gavl_dictionary_t * m;
  const char * track_id;
  int num = gavl_get_num_tracks(dict);
  
  for(i = 0; i < num; i++)
    {
    if((track = gavl_get_track_nc(dict, i)) &&
       (m = gavl_track_get_metadata(track)) &&
       (track_id = gavl_dictionary_get_string(m, GAVL_META_ID)) &&
       !strcmp(track_id, id))
      return track;
    }
  return NULL;
  }

static int compare_metadata_string(const void * p1, const void * p2)
  {
  const char * s1;
  const char * s2;
  
  const gavl_dictionary_t * dict1;
  const gavl_dictionary_t * dict2;

  if(!(dict1 = gavl_value_get_dictionary(p1)) ||
     !(dict2 = gavl_value_get_dictionary(p2)) ||
     !(dict1 = gavl_track_get_metadata(dict1)) ||
     !(dict2 = gavl_track_get_metadata(dict2)) ||
     !(s1 = gavl_dictionary_get_string(dict1, GAVL_META_LABEL)) ||
     !(s2 = gavl_dictionary_get_string(dict2, GAVL_META_LABEL)))
    return 0;
  
  return strcoll(s1, s2);
  }
  
void gavl_sort_tracks_by_label(gavl_array_t * arr)
  {
  gavl_array_sort(arr, compare_metadata_string);
  }

/* 
void gavl_sort_tracks_by_metadata_long(gavl_dictionary_t * dict,
                                       const char * str)
  {
  
  }

*/

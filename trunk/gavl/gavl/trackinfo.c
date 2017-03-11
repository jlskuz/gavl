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
  m = gavl_dictionary_get_child(s, GAVL_META_METADATA);
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
  gavl_dictionary_get_child(dict, GAVL_META_METADATA);
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

/* Track */

static void track_init(gavl_dictionary_t * track, int idx)
  {
  gavl_dictionary_t * m;
  m = gavl_dictionary_get_child(track, GAVL_META_METADATA);
  gavl_dictionary_set_int(m, GAVL_META_IDX, idx);
  }

static gavl_array_t * get_tracks_nc(gavl_dictionary_t * dict)
  {
  gavl_array_t * ret;

  if(!(ret = gavl_dictionary_get_array_nc(dict, GAVL_META_TRACKS)))
    {
    gavl_value_t val;
    gavl_value_init(&val);
    gavl_value_set_array(&val);
    gavl_dictionary_set_nocopy(dict, GAVL_META_TRACKS, &val);
    ret = gavl_dictionary_get_array_nc(dict, GAVL_META_TRACKS);
    }
  return ret;
  }

static const gavl_array_t * get_tracks(const gavl_dictionary_t * dict)
  {
  return gavl_dictionary_get_array(dict, GAVL_META_TRACKS);
  }

gavl_dictionary_t * gavl_append_track(gavl_dictionary_t * dict)
  {
  int num;
  gavl_dictionary_t * new_track;
  gavl_value_t val;
  gavl_array_t * arr = get_tracks_nc(dict);

  num = arr->num_entries;
  
  gavl_value_init(&val);
  new_track = gavl_value_set_dictionary(&val);
  
  track_init(new_track, num);
  gavl_array_push_nocopy(arr, &val);
  
  
  return arr->entries[num].v.dictionary;
  }

const gavl_dictionary_t * gavl_get_track(const gavl_dictionary_t * dict, int idx)
  {
  const gavl_value_t * val;
  const gavl_array_t * tracks;

  if(!(tracks = get_tracks(dict)) ||
     !(val = gavl_array_get(tracks, idx)))
    return NULL;
  return gavl_value_get_dictionary(val);
  }

gavl_dictionary_t * gavl_get_track_nc(gavl_dictionary_t * dict, int idx)
  {
  gavl_value_t * val;
  gavl_array_t * tracks;

  if(!(tracks = get_tracks_nc(dict)) ||
     !(val = gavl_array_get_nc(tracks, idx)))
    return NULL;
  return gavl_value_get_dictionary_nc(val);
  }

int gavl_get_num_tracks(const gavl_dictionary_t * dict)
  {
  const gavl_array_t * tracks;
  if(!(tracks = get_tracks(dict)))
    return 0;
  return tracks->num_entries;
  }

void gavl_delete_track(gavl_dictionary_t * dict, int idx)
  {
  gavl_array_t * tracks;
  if((tracks = get_tracks_nc(dict)))
    gavl_array_splice_val(tracks, idx, 1, NULL);
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


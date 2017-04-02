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

#ifndef GAVL_TRACKINFO_H_INCLUDED
#define GAVL_TRACKINFO_H_INCLUDED

/*
 *  Standardized info structures for media content.
 *  
 */
  
/* Audio */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_audio_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_audio_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC
int gavl_track_get_num_audio_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_audio_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_audio_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_audio_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_audio_format_t * gavl_track_get_audio_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_audio_format_t * gavl_track_get_audio_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_delete_audio_stream(gavl_dictionary_t * d, int stream);

/* Video */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_video_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_video_stream(const gavl_dictionary_t * d, int i);
  
GAVL_PUBLIC
int gavl_track_get_num_video_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_video_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_video_format_t * gavl_track_get_video_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_video_format_t * gavl_track_get_video_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_video_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_video_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_video_format_t * gavl_track_get_video_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_video_format_t * gavl_track_get_video_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_delete_video_stream(gavl_dictionary_t * d, int stream);


/* Text */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_text_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_text_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC
int gavl_track_get_num_text_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_text_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_text_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_text_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_delete_text_stream(gavl_dictionary_t * d, int stream);


/* Overlay */
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_overlay_stream_nc(gavl_dictionary_t * d, int i);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_overlay_stream(const gavl_dictionary_t * d, int i);

GAVL_PUBLIC
int gavl_track_get_num_overlay_streams(const gavl_dictionary_t * d);
  
GAVL_PUBLIC
gavl_dictionary_t * gavl_track_append_overlay_stream(gavl_dictionary_t * d);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_overlay_metadata(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_overlay_metadata_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
const gavl_video_format_t * gavl_track_get_overlay_format(const gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
gavl_video_format_t * gavl_track_get_overlay_format_nc(gavl_dictionary_t * d, int stream);

GAVL_PUBLIC
void gavl_track_delete_overlay_stream(gavl_dictionary_t * d, int stream);

/* Tracks */
GAVL_PUBLIC
gavl_dictionary_t * gavl_append_track(gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_get_track(const gavl_dictionary_t*, int idx);

GAVL_PUBLIC
gavl_dictionary_t * gavl_get_track_nc(gavl_dictionary_t*, int idx);

GAVL_PUBLIC
int gavl_get_num_tracks(const gavl_dictionary_t*);

GAVL_PUBLIC
void gavl_delete_track(gavl_dictionary_t*, int idx);

// GAVL_PUBLIC

GAVL_PUBLIC
gavl_audio_format_t * gavl_stream_get_audio_format_nc(gavl_dictionary_t*);

GAVL_PUBLIC
gavl_video_format_t * gavl_stream_get_video_format_nc(gavl_dictionary_t*);

GAVL_PUBLIC
gavl_dictionary_t * gavl_stream_get_metadata_nc(gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_audio_format_t * gavl_stream_get_audio_format(const gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_video_format_t * gavl_stream_get_video_format(const gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_stream_get_metadata(const gavl_dictionary_t*);


GAVL_PUBLIC
gavl_dictionary_t * gavl_track_get_metadata_nc(gavl_dictionary_t*);

GAVL_PUBLIC
const gavl_dictionary_t * gavl_track_get_metadata(const gavl_dictionary_t*);

GAVL_PUBLIC
void gavl_track_compute_duration(gavl_dictionary_t * dict);

GAVL_PUBLIC
gavl_time_t gavl_track_get_duration(const gavl_dictionary_t * dict);

GAVL_PUBLIC
void gavl_track_set_duration(gavl_dictionary_t * dict, gavl_time_t dur);



#endif // GAVL_TRACKINFO_H_INCLUDED

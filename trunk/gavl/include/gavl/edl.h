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


#ifndef GAVL_EDL_H_INCLUDED
#define GAVL_EDL_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/** \ingroup edl
 *  \brief Forward declaration
 */

#define GAVL_EDL_TRACK_IDX     "tidx"
#define GAVL_EDL_STREAM_IDX    "sidx"
#define GAVL_EDL_SRC_TIME      "stime"
#define GAVL_EDL_DST_TIME      "dtime"
#define GAVL_EDL_DST_DUR       "ddur"
#define GAVL_EDL_SPEED_NUM     "spnum"
#define GAVL_EDL_SPEED_DEN     "spden"
#define GAVL_EDL_SEGMENTS      "segs"
  
  
/** \brief One segment of a physical stream to appear in a logical stream
 */

#if 0  

typedef struct gavl_edl_s gavl_edl_t;

typedef struct
  {
  char * url;   //!< Location of that segment. If NULL, the "master url" in \ref gavl_edl_t is valid.

  int track;        //!<  Track index for multitrack inputs
  int stream;       //!<  Index of the A/V stream
  int timescale;    //!<  Source timescale
    
  int64_t src_time; //!< Time within the source in source timescale
  
  /* Time and duration within the destination in destination
     timescale */
  int64_t dst_time;  //!< Time  within the destination in destination timescale
  int64_t dst_duration; //!< Duration within the destination in destination timescale

  /*  */
  int32_t speed_num; //!< Playback speed numerator
  int32_t speed_den; //!< Playback speed demoninator
  
  } gavl_edl_segment_t;
#else
typedef gavl_dictionary_t gavl_edl_segment_t;
#endif

GAVL_PUBLIC 
void gavl_edl_segment_set_url(gavl_edl_segment_t * seg, const char * url); 

GAVL_PUBLIC
void gavl_edl_segment_set_speed(gavl_edl_segment_t * seg, int num, int den); 

GAVL_PUBLIC
void gavl_edl_segment_set(gavl_edl_segment_t * seg,
                          int track,
                          int stream,
                          int timescale,
                          int64_t src_time,
                          int64_t dst_time,
                          int64_t dst_duration);

GAVL_PUBLIC
const char * gavl_edl_segment_get_url(const gavl_edl_segment_t * seg); 

GAVL_PUBLIC
void gavl_edl_segment_get_speed(const gavl_edl_segment_t * seg, int * num, int * den); 

GAVL_PUBLIC
int gavl_edl_segment_get(const gavl_edl_segment_t * seg,
                          int * track,
                          int * stream,
                          int * timescale,
                          int64_t * src_time,
                          int64_t * dst_time,
                          int64_t * dst_duration);


GAVL_PUBLIC
void gavl_edl_finalize(gavl_dictionary_t * edl);
  
  
/** \brief A locical stream
 */
#if 0
typedef struct
  {
  gavl_edl_segment_t * segments; //!< Segments
  int num_segments;              //!< Number of segments 
  int timescale;                 //!< Destination timescale
  } gavl_edl_stream_t;

/** \brief A locical track
 */

typedef struct
  {
  gavl_dictionary_t   metadata;        //!< Metadata (optional)
  
  int num_audio_streams;             //!< Number of logical audio streams
  gavl_edl_stream_t * audio_streams; //!< Logical audio streams

  int num_video_streams;             //!< Number of logical video streams
  gavl_edl_stream_t * video_streams; //!< Logical video streams

  int num_text_streams;     //!< Number of logical text subtitle streams
  gavl_edl_stream_t * text_streams; //!< Logical text subtitle streams

  int num_overlay_streams;  //!< Number of logical overlay subtitle streams
  gavl_edl_stream_t * overlay_streams; //!< Logical overlay subtitle streams
  
  } gavl_edl_track_t;

/** \brief EDL structure
 */

struct gavl_edl_s
  {
  int num_tracks;             //!< Number of logical tracks
  gavl_edl_track_t * tracks;  //!< Logical tracks
  char * url;                 //!< Filename if all streams are from the same file
  };
 

/** \brief Append a track to the EDL
    \param e An EDL
 *  \returns The new track
 */

GAVL_PUBLIC
gavl_edl_track_t * gavl_edl_add_track(gavl_edl_t * e);

/** \brief Append an audio stream to an EDL track
    \param t An EDL track
 *  \returns The new stream
 */

GAVL_PUBLIC
gavl_edl_stream_t * gavl_edl_add_audio_stream(gavl_edl_track_t * t);

/** \brief Append a video stream to an EDL track
    \param t An EDL track
 *  \returns The new stream
 */

GAVL_PUBLIC
gavl_edl_stream_t * gavl_edl_add_video_stream(gavl_edl_track_t * t);

/** \brief Append a text stream to an EDL track
    \param t An EDL track
 *  \returns The new stream
 */

GAVL_PUBLIC
gavl_edl_stream_t * gavl_edl_add_text_stream(gavl_edl_track_t * t);

/** \brief Append an overlay stream to an EDL track
    \param t An EDL track
 *  \returns The new stream
 */

GAVL_PUBLIC
gavl_edl_stream_t * gavl_edl_add_overlay_stream(gavl_edl_track_t * t);

/** \brief Copy an entire EDL
    \param e An EDL
 *  \returns Copy of the EDL
 */

GAVL_PUBLIC
gavl_edl_t * gavl_edl_copy(const gavl_edl_t * e);

/** \brief Destroy an EDL and free all memory
    \param e An EDL
 */

GAVL_PUBLIC
void gavl_edl_destroy(gavl_edl_t * e);

  
#endif

/** \brief Create an empty EDL
 *  \returns A newly allocated EDL
 */

GAVL_PUBLIC
gavl_dictionary_t * gavl_edl_create(gavl_dictionary_t * parent);

  
/** \brief Append a segment to an EDL stream
    \param s An EDL stream
 *  \returns The new segment
 */
  
GAVL_PUBLIC
gavl_edl_segment_t * gavl_edl_add_segment(gavl_dictionary_t * s);

#if 0

/** \brief Dump an EDL to stderr
    \param e An EDL

    Mainly used for debugging
 */

GAVL_PUBLIC
void gavl_edl_dump(const gavl_edl_t * e);

/** \brief Convert a source time to a destination time
    \param st An EDL stream
    \param seg An EDL Segment
    \param src_time The time in source coordinates
    \returns The time in destination coordinates
 */
  
GAVL_PUBLIC
int64_t gavl_edl_src_time_to_dst(const gavl_dictionary_t * st,
                                 const gavl_edl_segment_t * seg,
                                 int64_t src_time);

/** \brief Convert a destination time to a source time
    \param t An EDL track
    \param st An EDL stream
    \param seg An EDL Segment
    \param src_time The time in source coordinates
    \returns The time in destination coordinates
 */

  
GAVL_PUBLIC
const gavl_edl_segment_t *
gavl_edl_dst_time_to_src(const gavl_dictionary_t * t,
                         const gavl_dictionary_t * st,
                         int64_t dst_time,
                         int64_t * src_time,
                         int64_t * mute_time);
                             
GAVL_PUBLIC gavl_time_t
gavl_edl_track_get_duration(const gavl_edl_track_t * t);
#endif

/* Convert to / from dictionary for easy transporting */

// GAVL_PUBLIC void gavl_edl_to_dictionary(const gavl_edl_t * edl, gavl_dictionary_t * dict);
// GAVL_PUBLIC int gavl_edl_from_dictionary(gavl_edl_t * edl, const gavl_dictionary_t * dict);
  
#ifdef __cplusplus
}
#endif

#endif // GAVL_EDL_H_INCLUDED

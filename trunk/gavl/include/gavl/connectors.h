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

/**
 * @file connector.h
 * external api header.
 */

#ifndef GAVL_CONNECTORS_H_INCLUDED
#define GAVL_CONNECTORS_H_INCLUDED

#include <gavl/gavl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*! \defgroup pipelines A/V pipeline building elements
 *
 * Starting with version 1.5.0 gavl gets support for
 * building blocks for A/V pipelines. The basic elements
 * are a source, a sink and a connector. The source works in
 * pull-mode, the sink works in push mode. A connector connects
 * a source with one or more sinks.
 */

/*! \defgroup sources A/V sources
 *
 *  A/V sources are elements, which can be used to
 *  conveniently pass audio or video frames from one
 *  program module to another. They do implicit format
 *  conversion and optimized buffer handling.
 *
 *  A module, which provides A/V frames, creates a source
 *  module and tells the native format in which it provides
 *  the frames. If you want to obtain the frames from the source,
 *  you tell the desired output format and if you intend to overwrite
 *  the frames.
 *
 *  To obtain the frames, you pass the address of the pointer of the
 *  frame. If the pointer is NULL, it will be set to an internal buffer.
 *
 *  The return value is of the type \ref gavl_source_status_t, which can
 *  have 3 states depending on whether the stream ended (EOF) or a frame
 *  is available or if no frame is available right now. The latter can be
 *  used to implement parts of the pipeline in pull mode (the default) and
 *  others in push mode.
 * 
 * @{
 */

/** \brief Return value of the source function */  

typedef enum
  {
    GAVL_SOURCE_EOF   = 0, //!< End of file, no more frames available
    GAVL_SOURCE_OK    = 1, //!< Frame available
    GAVL_SOURCE_AGAIN = 2, //!< No frame available right now, might try later
  } gavl_source_status_t;

/** \brief Prototype for onbtaining one audio frame
 *  \param priv Client data
 *  \param frame Where to store the frame
 *  \param stream Stream number to read from (usually zero)
 *
 *  If *frame is non-null, the data will be copied there,
 *  otherwise the address of an internally allocated frame is returned
 */
  
typedef gavl_source_status_t
(*gavl_audio_source_func_t)(void * priv, gavl_audio_frame_t ** frame);

/** \brief Prototype for onbtaining one video frame
 *  \param priv Client data
 *  \param frame Where to store the frame
 *  \param stream Stream number to read from (usually zero)
 *
 *  If *frame is non-null, the data will be copied there,
 *  otherwise the address of an internally allocated frame is returned.
 */
  
typedef gavl_source_status_t
(*gavl_video_source_func_t)(void * priv, gavl_video_frame_t ** frame);

/** \brief Forward declaration of the video source */
  
typedef struct gavl_video_source_s gavl_video_source_t;

/** \brief Forward declaration of the audio source */
  
typedef struct gavl_audio_source_s gavl_audio_source_t;

/** \brief Source provides a pointer to an internal structure */

#define GAVL_SOURCE_SRC_ALLOC               (1<<0)

/** \brief Source might need the frames for decoding future frames */
#define GAVL_SOURCE_SRC_REF                 (1<<1)

/** \brief Samples per frame is just an upper bound.
    Frames can have smaller sizes also. The last frame is always
    allowed to have fewer samples, even if this flag is not set */
#define GAVL_SOURCE_SRC_FRAMESIZE_MAX       (1<<2)
  
/** \brief Destination changes frame. */

#define GAVL_SOURCE_DST_OVERWRITES          (1<<0)

/* Called by the source */

/** \brief Create a video source
 *  \param func Function to get the frames from
 *  \param priv Client data to pass to func
 *  \param src_flags Flags describing the source
 *  \param src_format Native source format
 *  \returns A newly created video source
 */

GAVL_PUBLIC
gavl_video_source_t *
gavl_video_source_create(gavl_video_source_func_t func,
                         void * priv, int src_flags,
                         const gavl_video_format_t * src_format);

/** \brief Get coversion options of a video source
 *  \param s A video source
 *  \returns Conversion options
 */

  
GAVL_PUBLIC
gavl_video_options_t * gavl_video_source_get_options(gavl_video_source_t * s);

/** \brief Reset a video source
 *  \param s A video source
 *
 *  Call this after seeking to reset the internal state
 */
  
GAVL_PUBLIC
void gavl_video_source_reset(gavl_video_source_t * s);

/** \brief Destroy a video source
 *  \param s A video source
 *
 *  Destroy a video source including all video frames ever
 *  created by it.
 */

GAVL_PUBLIC
void gavl_video_source_destroy(gavl_video_source_t * s);
  
/* Called by the destination */

/** \brief Get the native format
 *  \param s A video source
 *  \returns The native video format
 */
  
GAVL_PUBLIC
const gavl_video_format_t *
gavl_video_source_get_src_format(gavl_video_source_t * s);

/** \brief Get the output format
 *  \param s A video source
 *  \returns The video format in which frames are read
 */
  
GAVL_PUBLIC
const gavl_video_format_t *
gavl_video_source_get_dst_format(gavl_video_source_t * s);

/** \brief Set the destination mode
 *  \param s A video source
 *  \param dst_flags Flags
 *  \param dst_format Format in which the frames will be read
 *
 *  If you accept the source format (as returned by
 *  \ref gavl_video_source_get_src_format) you can pass NULL for the
 *  dst_format.
 *
 *  If the destination format differs from the source format,
 *  the frames will converted. For this, we have a
 *  \ref gavl_video_converter_t and also do simple framerate conversion
 *  which repeats/drops frames.
 */
  
GAVL_PUBLIC
void
gavl_video_source_set_dst(gavl_video_source_t * s, int dst_flags,
                          const gavl_video_format_t * dst_format);

/** \brief Read a video frame
 *  \param s A video source 
 *  \param frame Address of a frame.
 *
 *  This reads one frame from the source. If *frame is NULL
 *  it will be set to an internal buffer, otherwise the data is
 *  copied to the frame you pass.
 */


GAVL_PUBLIC
gavl_source_status_t
gavl_video_source_read_frame(void * s, gavl_video_frame_t ** frame);
  
/* Called by source */ 

/** \brief Create an audio source
 *  \param func Function to get the frames from
 *  \param priv Client data to pass to func
 *  \param src_flags Flags describing the source
 *  \param src_format Native source format
 *  \returns A newly created audio source
 */
  
GAVL_PUBLIC
gavl_audio_source_t *
gavl_audio_source_create(gavl_audio_source_func_t func,
                         void * priv, int src_flags,
                         const gavl_audio_format_t * src_format);

/** \brief Get the native format
 *  \param s An audio source
 *  \returns The native audio format
 */
  
GAVL_PUBLIC
const gavl_audio_format_t *
gavl_audio_source_get_src_format(gavl_audio_source_t * s);

/** \brief Get the output format
 *  \param s An audio source
 *  \returns The format in which frames will be read
 */
  
GAVL_PUBLIC
const gavl_audio_format_t *
gavl_audio_source_get_dst_format(gavl_audio_source_t * s);

/** \brief Set the destination mode
 *  \param s An audio source
 *  \param dst_flags Flags
 *  \param dst_format Format in which the frames will be read
 *
 *  If you accept the source format (as returned by
 *  \ref gavl_video_source_get_src_format) you can pass NULL for the
 *  dst_format.
 *
 *  If the destination format differs from the source format,
 *  the frames will converted. For this, we have a
 *  \ref gavl_audio_converter_t. In addition, if the
 *  samples_per_frame members are different, the frames will
 *  be repackaged.
 */

GAVL_PUBLIC
void
gavl_audio_source_set_dst(gavl_audio_source_t * s, int dst_flags,
                          const gavl_audio_format_t * dst_format);

/** \brief Read an audio frame
 *  \param s An audio source 
 *  \param frame Address of a frame.
 *  \returns The status
 *
 *  This reads one frame from the source. If *frame is NULL
 *  it will be set to an internal buffer, otherwise the data is
 *  copied to the frame you pass.
 *
 *  If the return value is \ref GAVL_SOURCE_AGAIN, you might
 *  have an imcomplete frame. In this case you must call
 *  this function again with exactly the same frame argument.
 */
  
GAVL_PUBLIC
gavl_source_status_t
gavl_audio_source_read_frame(void*s, gavl_audio_frame_t ** frame);

/** \brief Skip audio samples at the input
 *  \param s An audio source 
 *  \param num_samples Number of samples to skip
 *
 *  This skips a number of input samples. It can be used after
 *  seeking if the sample position after a seek is no multiple of
 *  the frame size.
 */
  
GAVL_PUBLIC
void 
gavl_audio_source_skip_src(gavl_audio_source_t * s, int num_samples);
  
/** \brief Read audio samples
 *  \param s An audio source 
 *  \param frame An audio frame 
 *  \param num_samples Number of samples to read
 *
 *  This is for APIs, which pass the number of samples to each
 *  read() call and the number is not known in advance.
 */

GAVL_PUBLIC
int gavl_audio_source_read_samples(void*s, gavl_audio_frame_t * frame,
                                   int num_samples);

/** \brief Get coversion options of an audio source
 *  \param s An audio source
 *  \returns Conversion options
 */
  
GAVL_PUBLIC
gavl_audio_options_t * gavl_audio_source_get_options(gavl_audio_source_t * s);

/** \brief Reset an audio source
 *  \param s An audio source
 *
 *  This resets the internal state as if no frame was read yet.
 */
  
GAVL_PUBLIC
void gavl_audio_source_reset(gavl_audio_source_t * s);

/** \brief Destroy an audio source
 *  \param s An audio source
 *
 *  Destroy an audio source including all audio frames ever
 *  created by it.
 */
  
GAVL_PUBLIC
void gavl_audio_source_destroy(gavl_audio_source_t * s);
  
/**
 * @}
 */

/*! \defgroup sinks A/V sinks
 *
 *  This is a thin layer for a unified handling of
 *  A/V sinks. A sink can either supply a frame where
 *  the data could be copied or you pass a frame
 *  allocated by yourself to the sink. Sinks don't do
 *  format conversion. Instead you need to obtain the
 *  format and pass this to the source where you read
 *  data from.
 *
 * @{
 */

/** \brief Return status of the sink functions 
 */
  
typedef enum
  {
    GAVL_SINK_ERROR, //!< Something went wrong
    GAVL_SINK_OK,    //!< Frame was successfully processed
  } gavl_sink_status_t;

/** \brief Audio sink
 *
 *  You don't want to know what's inside
 */

typedef struct 
gavl_audio_sink_s gavl_audio_sink_t;

/** \brief Prototype for getting a frame buffer
 *  \param priv Private data
 *  \returns An audio frame where to copy the data
 *
 *  Sinks can use this to pass specially allocated buffers
 *  (e.g. shared or mmaped memory) to the client
 */

typedef gavl_audio_frame_t *
(*gavl_audio_sink_get_func)(void * priv);

/** \brief Prototype for putting a frame
 *  \param priv Private data
 *  \param f An audio frame
 *  \returns \ref GAVL_SINK_ERROR if an error happened, \ref GAVL_SINK_OK else.
 */
  
typedef gavl_sink_status_t
(*gavl_audio_sink_put_func)(void * priv, gavl_audio_frame_t * f);

/** \brief Create an audio sink
 *  \param get_func Function for getting a frame buffer or NULL
 *  \param put_func Function for outputting a frame
 *  \param priv Client data to pass to get_func and put_func
 *  \param format Format in which we accept the data
 *  \returns A newly created audio sink
 */
  
GAVL_PUBLIC gavl_audio_sink_t *
gavl_audio_sink_create(gavl_audio_sink_get_func get_func,
                       gavl_audio_sink_put_func put_func,
                       void * priv,
                       const gavl_audio_format_t * format);

/** \brief Get the format
 *  \param s An audio sink
 *  \returns format in which the sink accepts data
 */
  
GAVL_PUBLIC const gavl_audio_format_t *
gavl_audio_sink_get_format(gavl_audio_sink_t * s);

/** \brief Get a buffer for a frame
 *  \param s An audio sink
 *  \returns A frame buffer
 *
 *  This function must be called before
 *  \ref gavl_audio_sink_put_frame. If it returns non-NULL, the same
 *  frame must be passed to the next call to \ref gavl_audio_sink_put_frame.
 */
  
GAVL_PUBLIC gavl_audio_frame_t *
gavl_audio_sink_get_frame(gavl_audio_sink_t * s);

/** \brief Output a frame
 *  \param s An audio sink
 *  \param f Frame
 *  \returns \ref GAVL_SINK_ERROR if an error happened, \ref GAVL_SINK_OK else.
 *
 *  The frame must be the same as returned by the preceeding call to
 *  \ref gavl_audio_sink_get_frame if it was not NULL.
 */

GAVL_PUBLIC gavl_sink_status_t
gavl_audio_sink_put_frame(gavl_audio_sink_t * s, gavl_audio_frame_t * f);

/** \brief Destroy an audio sink
 *  \param s An audio sink
 */
  
GAVL_PUBLIC void
gavl_audio_sink_destroy(gavl_audio_sink_t * s);

  
/** \brief Video sink
 *
 *  You don't want to know what's inside
 */
  
typedef struct 
gavl_video_sink_s gavl_video_sink_t;

/** \brief Prototype for getting a frame buffer
 *  \param priv Private data
 *  \returns A video frame where to copy the data
 *
 *  Sinks can use this to pass specially allocated buffers
 *  (e.g. shared or mmaped memory) to the client
 */

typedef gavl_video_frame_t *
(*gavl_video_sink_get_func)(void * priv);

/** \brief Prototype for putting a frame
 *  \param priv Private data
 *  \param f A video frame
 *  \returns \ref GAVL_SINK_ERROR if an error happened, \ref GAVL_SINK_OK else.
 */

typedef gavl_sink_status_t
(*gavl_video_sink_put_func)(void * priv, gavl_video_frame_t * f);

/** \brief Create a video sink
 *  \param get_func Function for getting a frame buffer or NULL
 *  \param put_func Function for outputting a frame
 *  \param priv Client data to pass to get_func and put_func
 *  \param format Format in which we accept the data
 *  \returns A newly created video sink
 */
  
GAVL_PUBLIC gavl_video_sink_t *
gavl_video_sink_create(gavl_video_sink_get_func get_func,
                       gavl_video_sink_put_func put_func,
                       void * priv,
                       const gavl_video_format_t * format);

/** \brief Get the format
 *  \param s A video sink
 *  \returns format in which the sink accepts data
 */

GAVL_PUBLIC const gavl_video_format_t *
gavl_video_sink_get_format(gavl_video_sink_t * s);

/** \brief Get a buffer for a frame
 *  \param s A video sink
 *  \returns A frame buffer
 *
 *  This function must be called before
 *  \ref gavl_video_sink_put_frame. If it returns non-NULL, the same
 *  frame must be passed to the next call to \ref gavl_video_sink_put_frame.
 */
  
GAVL_PUBLIC gavl_video_frame_t *
gavl_video_sink_get_frame(gavl_video_sink_t * s);

/** \brief Output a frame
 *  \param s A video sink
 *  \param f Frame
 *  \returns \ref GAVL_SINK_ERROR if an error happened, \ref GAVL_SINK_OK else.
 *
 *  The frame must be the same as returned by the preceeding call to
 *  \ref gavl_video_sink_get_frame if it was not NULL.
 */

GAVL_PUBLIC gavl_sink_status_t
gavl_video_sink_put_frame(gavl_video_sink_t * s, gavl_video_frame_t * f);

/** \brief Destroy a video sink
 *  \param s A video sink
 */

GAVL_PUBLIC void
gavl_video_sink_destroy(gavl_video_sink_t * s);
  
/**
 * @}
 */


/*! \defgroup connectors A/V connectors
 *
 *  Connectors link one source and one or more sinks. They 
 *  do all buffer handling and format conversion.
 *
 * @{
 */

/*! \brief Opaque structure for the audio connector
 *
 * You don't want to know what's inside.
 */
  
typedef struct gavl_audio_connector_s gavl_audio_connector_t;

/*! \brief Opaque structure for the audio connector
 *
 * You don't want to know what's inside.
 */

typedef struct gavl_video_connector_s gavl_video_connector_t;

/*! \brief Callback for processing an audio frame
 *  \param priv Client data
 *  \param frame Frame
 *
 *  This function is called whenever a frame was read from the source.
 *  Use \ref gavl_audio_connector_set_process_func to set this callback.
 *  The format of the frame can be obtained with
 *  \ref gavl_audio_connector_get_process_format after
 *  \ref gavl_audio_connector_start was called.
 */
  
typedef void
(*gavl_audio_connector_process_func)(void * priv,
                                     gavl_audio_frame_t * frame);

/*! \brief Callback for processing a video frame
 *  \param priv Client data
 *  \param frame Frame
 *
 *  This function is called whenever a frame was read from the source.
 *  Use \ref gavl_video_connector_set_process_func to set this callback.
 *  The format of the frame can be obtained with
 *  \ref gavl_video_connector_get_process_format
 *  after \ref gavl_video_connector_start was called.
 */

typedef void
(*gavl_video_connector_process_func)(void * priv,
                                     gavl_video_frame_t * frame);

/*! \brief Create an audio connector
 *  \param src Source
 *  \returns A newly created audio connector
 */
  
GAVL_PUBLIC gavl_audio_connector_t *
gavl_audio_connector_create(gavl_audio_source_t * src);

/*! \brief Get conversion options
 *  \param c An audio connector
 *  \returns Conversion options
 *
 *  These options will be used for all internal format conversions.
 */
  
GAVL_PUBLIC gavl_audio_options_t *
gavl_audio_connector_get_options(gavl_audio_connector_t * c);

/*! \brief Destroy an audio connector
 *  \param c An audio connector
 */
  
GAVL_PUBLIC void
gavl_audio_connector_destroy(gavl_audio_connector_t * c);

/*! \brief Connect a sink
 *  \param c An audio connector
 *  \param sink An audio sink
 */
  
GAVL_PUBLIC void
gavl_audio_connector_connect(gavl_audio_connector_t * c,
                             gavl_audio_sink_t * sink);

/*! \brief Set process callback
 *  \param c An audio connector
 *  \param func Process callback
 *  \param priv Client data to be passed to func
 */
  
GAVL_PUBLIC void
gavl_audio_connector_set_process_func(gavl_audio_connector_t * c,
                                      gavl_audio_connector_process_func func,
                                      void * priv);

/*! \brief Start an audio connector
 *  \param c An audio connector
 *
 *  Call this function after connecting all sinks and before
 *  calling \ref gavl_audio_connector_process.
 */

GAVL_PUBLIC void
gavl_audio_connector_start(gavl_audio_connector_t * c);

/*! \brief Get process format
 *  \param c An audio connector
 *  \returns The intermediate format of the frames passed to the process callback
 */
  
GAVL_PUBLIC const gavl_audio_format_t * 
gavl_audio_connector_get_process_format(gavl_audio_connector_t * c);

/*! \brief Process one frame
 *  \param c An audio connector
 *  \returns 0 if a sink reported an error, 1 else
 *
 *  Read one frame from the source and pass it to all sinks.
 *  The sinks might output zero or more frames depending on the
 *  conversions.
 */
  
GAVL_PUBLIC int
gavl_audio_connector_process(gavl_audio_connector_t * c);

/*! \brief Reset an audio connector
 *  \param c An audio connector
 *
 *  Reset the audio connector.
 */

GAVL_PUBLIC
void gavl_audio_connector_reset(gavl_audio_connector_t * c);

  
/* */

/*! \brief Create a video connector
 *  \param src Source
 *  \returns A newly created video connector
 */
  
GAVL_PUBLIC gavl_video_connector_t *
gavl_video_connector_create(gavl_video_source_t * src);

/*! \brief Get conversion options
 *  \param c A video connector
 *  \returns Conversion options
 *
 *  These options will be used for all internal format conversions.
 */
  
GAVL_PUBLIC gavl_video_options_t *
gavl_video_connector_get_options(gavl_video_connector_t * c);

/*! \brief Destroy a video connector
 *  \param c A video connector
 */
  
GAVL_PUBLIC void
gavl_video_connector_destroy(gavl_video_connector_t * c);

/*! \brief Connect a sink
 *  \param c A video connector
 *  \param sink A video sink
 */

GAVL_PUBLIC void
gavl_video_connector_connect(gavl_video_connector_t * c,
                             gavl_video_sink_t * sink);

/*! \brief Set process callback
 *  \param c A video connector
 *  \param func Process callback
 *  \param priv Client data to be passed to func
 */

GAVL_PUBLIC void
gavl_video_connector_set_process_func(gavl_video_connector_t * c,
                                      gavl_video_connector_process_func func,
                                      void * priv);

/*! \brief Start a video connector
 *  \param c A video connector
 *
 *  Call this function after connecting all sinks and before
 *  calling \ref gavl_video_connector_process.
 */

GAVL_PUBLIC void
gavl_video_connector_start(gavl_video_connector_t * c);

/*! \brief Get process format
 *  \param c A video connector
 *  \returns The intermediate format of the frames passed to the process callback
 */

GAVL_PUBLIC const gavl_video_format_t * 
gavl_video_connector_get_process_format(gavl_video_connector_t * c);

/*! \brief Process one frame
 *  \param c A video connector
 *  \returns 0 if a sink reported an error, 1 else
 *
 *  Read one frame from the source and pass it to all sinks.
 *  The sinks might output zero or more frames depending on the
 *  conversions.
 */
  
GAVL_PUBLIC int
gavl_video_connector_process(gavl_video_connector_t * c);

/*! \brief Reset a video connector
 *  \param c A video connector
 *
 *  Reset the video connector.
 */
  
GAVL_PUBLIC
void gavl_video_connector_reset(gavl_video_connector_t * c);

  
/**
 * @}
 */
  
#ifdef __cplusplus
}
#endif

#endif // GAVL_CONNECTORS_H_INCLUDED

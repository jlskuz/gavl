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

#ifndef GAVL_MSG_H_INCLUDED
#define GAVL_MSG_H_INCLUDED

#include <gavl/gavldefs.h>
#include <gavl/value.h>

#define GAVL_MSG_NONE     -1 //!< Reserved ID for non valid message
#define GAVL_MSG_MAX_ARGS  8 //!< Maximum number of args

#define GAVL_MSG_NS_TRANSPORT 1
#define GAVL_MSG_NS_GENERIC   2
#define GAVL_MSG_NS_SRC       3
#define GAVL_MSG_NS_GUI       4

/* Message IDs */

/*
 *  GAVL_MSG_NS_TRANSPORT
 */

/** \brief 
 *  
 *  For implementing block-free duplex connections.
 */

#define GAVL_MSG_READY          1 // Initial handshake, sent by client, echoed by server
#define GAVL_MSG_EOT            2 // End of transmission, it's peer's turn
#define GAVL_MSG_ACK            3 // Acknowledge, can have application specific args
#define GAVL_MSG_BYE            4 // Quit, connection is closed IMMEDIATELY after

/*
 *  GAVL_MSG_NS_GENERIC
 */

/** \brief Generic progress callback
 *
 *  arg1: Activity (string) 
 *  arg0: Percentage (0.0..1.0)
 */

#define GAVL_MSG_PROGRESS      1

/*
 *  GAVL_MSG_NS_SRC
 */


/** \brief Global metadata changed
 *
 *  arg0: time (time)
 *  arg1: scale (int)
 *  arg2: Metadata
 */

#define GAVL_MSG_SRC_METADATA_CHANGED  1

/** \brief Pixel aspect ratio changed
 *
 *  arg0: time (time)
 *  arg1: scale (int)
 *  arg2: Stream
 *  arg3: new pixel width
 *  arg4: new pixel width
 */

#define GAVL_MSG_SRC_ASPECT_CHANGED    2

/** \brief Buffering notification
 *
 *  arg0: Percentage (float), negative means buffering finished
 */

#define GAVL_MSG_SRC_BUFFERING 3

/* GUI Events */

/*
 *  GAVL_MSG_NS_GUI
 */

/** \brief Key was pressed
 *
 *  arg0: Key  (see keycodes.h)
 *  arg1: Mask (see keycodes.h)
 */

#define GAVL_MSG_GUI_KEY_PRESS       1  // Key was pressed

/** \brief Key was released
 *
 *  arg0: Key  (see keycodes.h)
 *  arg1: Mask (see keycodes.h)
 */

#define GAVL_MSG_GUI_KEY_RELEASE     2  // Key was released

/** \brief Button was pressed
 *
 *  arg0: Button  (1..5)
 *  arg1: Mask (see keycodes.h)
 *  arg2: x (int, in screen pixels)
 *  arg3: y (int, in screen pixels)
 *  arg4: pos (position, 0..1, relative to Video viewport)
 */

#define GAVL_MSG_GUI_BUTTON_PRESS    3  // Mouse button was pressed

/** \brief Button was released
 *
 *  arg0: Button  (1..5)
 *  arg1: Mask (see keycodes.h)
 *  arg2: x (int, in screen pixels)
 *  arg3: y (int, in screen pixels)
 *  arg4: pos (position, 0..1, relative to Video viewport)
 */

#define GAVL_MSG_GUI_BUTTON_RELEASE  4  // Mouse button was released

/** \brief Motion callback
 *
 *  arg0: Mask (see keycodes.h)
 *  arg1: x (int, in video pixels)
 *  arg2: y (int, in video pixels)
 *  arg3: pos (position, 0..1, relative to Video viewport)
 */

#define GAVL_MSG_GUI_MOUSE_MOTION    5  // Mouse was moved

/** \brief Accelerator
 *
 *  arg0: id (int)
 */

#define GAVL_MSG_GUI_ACCEL         6  // Accelerator was triggered

/** \brief Window coordinates changed
 *
 *  arg0: x (int)
 *  arg1: y (int)
 *  arg2: w (int)
 *  arg3: h (int)
 */

#define GAVL_MSG_GUI_WINDOW_COORDS 7  // Window coordinates changed

/* Commands */

/** \brief Set window title
 *
 *  arg0: title (string)
 */

#define GAVL_CMD_GUI_SET_WINDOW_TITLE 100

/** \brief Accelerator
 *
 *  arg0: time (time)
 *  arg1: scale (int)
 *  arg2: id (int)
 */

#define GAVL_MSG_GUI_ACCEL         6  // Accelerator was triggered



/** \brief Message type
 */

struct gavl_msg_s
  {
  uint32_t ns;  // Namespace
  uint32_t id;
  int num_args;

  // Where to send the answer. Meaning defined at a higher level.
  void * sender; 
  
  gavl_value_t args[GAVL_MSG_MAX_ARGS];
  
  };

typedef struct gavl_msg_s gavl_msg_t;

/** \brief Create a message
 *  \returns A newly allocated message
 */

GAVL_PUBLIC
gavl_msg_t * gavl_msg_create();

/** \brief Initialize a message struct
 *  \param m A message
 */

GAVL_PUBLIC
void gavl_msg_init(gavl_msg_t * m);

/** \brief Destroy a message
 *  \param msg A message
 */

GAVL_PUBLIC
void gavl_msg_destroy(gavl_msg_t * msg);

/** \brief Free internal memory of the message
 *  \param msg A message
 *
 *  Use this, if you want to reuse the message with
 *  a different ID or args
 */

GAVL_PUBLIC
void gavl_msg_free(gavl_msg_t * msg);

/** \brief Copy message
 *  \param dst Destination
 *  \param src Source
 */

GAVL_PUBLIC
void gavl_msg_copy(gavl_msg_t * dst, const gavl_msg_t * src);


GAVL_PUBLIC
int gavl_msg_match(const gavl_msg_t * m, uint32_t id, uint32_t ns);

/* Functions for messages */

/** \brief Set the ID of a message
 *  \param msg A message
 *  \param id The ID
 */

GAVL_PUBLIC
void gavl_msg_set_id(gavl_msg_t * msg, int id);

/** \brief Set the ID of a message with namespace
 *  \param msg A message
 *  \param id The ID
 *  \param ns The Namespace
 */

GAVL_PUBLIC
void gavl_msg_set_id_ns(gavl_msg_t * msg, int id, int ns);

/** \brief Get the ID of a message
 *  \param msg A message
 *  \returns The ID
 */

GAVL_PUBLIC
int gavl_msg_get_id(gavl_msg_t * msg);

/** \brief Get the ID and namespace of a message
 *  \param msg A message
 *  \param ns If non-NULL, returns the namespace
 *  \returns The ID
 */

GAVL_PUBLIC
int gavl_msg_get_id_ns(gavl_msg_t * msg, int * ns);

GAVL_PUBLIC
int gavl_msg_set_arg(gavl_msg_t * msg, int idx, const gavl_value_t * val);

GAVL_PUBLIC
int gavl_msg_set_arg_nocopy(gavl_msg_t * msg, int idx, gavl_value_t * val);

GAVL_PUBLIC
const gavl_value_t * gavl_msg_get_arg_c(const gavl_msg_t * msg, int idx);

GAVL_PUBLIC
void gavl_msg_get_arg(gavl_msg_t * msg, int idx, gavl_value_t * val);

/** \brief Set an integer argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_int(gavl_msg_t * msg, int arg, int value);

/** \brief Get an integer argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns Value
 */

GAVL_PUBLIC
int gavl_msg_get_arg_int(const gavl_msg_t * msg, int arg);

/** \brief Set a time argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_time(gavl_msg_t * msg, int arg, gavl_time_t value);

/** \brief Get a time argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns Value
 */

GAVL_PUBLIC
gavl_time_t gavl_msg_get_arg_time(gavl_msg_t * msg, int arg);

/** \brief Set a string argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_string(gavl_msg_t * msg, int arg, const char * value);

/** \brief Get a string argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns The string
 *
 *  You can get the string value only once from each arg
 *  and must free() it, when you are done with it
 */

GAVL_PUBLIC
char * gavl_msg_get_arg_string(gavl_msg_t * msg, int arg);

/** \brief Get a string argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns The string
 *
 *  Like \ref gavl_msg_set_arg_string except that the returned string is 
 *  owned by the message and must not be freed.
 */

GAVL_PUBLIC
const char * gavl_msg_get_arg_string_c(const gavl_msg_t * msg, int arg);

/** \brief Set a float argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_float(gavl_msg_t * msg, int arg, double value);

/** \brief Get a float argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \returns Value
 */

GAVL_PUBLIC
double gavl_msg_get_arg_float(gavl_msg_t * msg, int arg);

/** \brief Set an RGB color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_color_rgb(gavl_msg_t * msg, int arg, const float * value);

/** \brief Get an RGB color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_get_arg_color_rgb(gavl_msg_t * msg, int arg, float * value);


/** \brief Set an RGBA color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_color_rgba(gavl_msg_t * msg, int arg, const float * value);

/** \brief Get an RGBA color argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_get_arg_color_rgba(gavl_msg_t * msg, int arg, float * value);

/** \brief Set a position argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_set_arg_position(gavl_msg_t * msg, int arg, const double * value);

/** \brief Get a position argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param value Value
 */

GAVL_PUBLIC
void gavl_msg_get_arg_position(gavl_msg_t * msg, int arg, double * value);

/** \brief Set an audio format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format An audio format
 */

GAVL_PUBLIC
void gavl_msg_set_arg_audio_format(gavl_msg_t * msg, int arg,
                                 const gavl_audio_format_t * format);

/** \brief Get an audio format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format Returns the audio format
 */

GAVL_PUBLIC
void gavl_msg_get_arg_audio_format(gavl_msg_t * msg, int arg,
                                 gavl_audio_format_t * format);


/** \brief Set a video format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format A video format
 */

GAVL_PUBLIC
void gavl_msg_set_arg_video_format(gavl_msg_t * msg, int arg,
                                 const gavl_video_format_t * format);

/** \brief Get a video format argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param format Returns the video format
 */

GAVL_PUBLIC
void gavl_msg_get_arg_video_format(gavl_msg_t * msg, int arg,
                                 gavl_video_format_t * format);


/** \brief Set a matadata argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param m Metadata
 */

GAVL_PUBLIC
void gavl_msg_set_arg_dictionary(gavl_msg_t * msg, int arg,
                               const gavl_dictionary_t * m);

/** \brief Get a matadata argument
 *  \param msg A message
 *  \param arg Argument index (starting with 0)
 *  \param m Returns metadata
 *
 *  Don't pass uninitalized memory as metadata.
 */

GAVL_PUBLIC
int gavl_msg_get_arg_dictionary(gavl_msg_t * msg, int arg,
                               gavl_dictionary_t * m);

GAVL_PUBLIC
int gavl_msg_get_arg_dictionary_c(const gavl_msg_t * msg, int arg,
                                gavl_dictionary_t * m);


GAVL_PUBLIC
void gavl_msg_dump(const gavl_msg_t * msg, int indent);

/*
 *  Utilities
 */

GAVL_PUBLIC void
gavl_msg_set_progress(gavl_msg_t * msg, const char * activity, float perc);

GAVL_PUBLIC void
gavl_msg_get_progress(gavl_msg_t * msg, char ** activity, float * perc);


GAVL_PUBLIC void
gavl_msg_set_src_metadata(gavl_msg_t * msg, int64_t time, int scale, const gavl_dictionary_t * m);

GAVL_PUBLIC void
gavl_msg_get_src_metadata(gavl_msg_t * msg, int64_t * time, int * scale, gavl_dictionary_t * m);

GAVL_PUBLIC void
gavl_msg_set_src_aspect(gavl_msg_t * msg, int64_t time, int scale, int stream,
                        int pixel_width, int pixel_height);

GAVL_PUBLIC void
gavl_msg_get_src_aspect(gavl_msg_t * msg,
                        int64_t * time,
                        int * scale, int * stream,
                        int * pixel_width, int * pixel_height);
GAVL_PUBLIC void
gavl_msg_set_src_buffering(gavl_msg_t * msg, float perc);

GAVL_PUBLIC void
gavl_msg_get_src_buffering(gavl_msg_t * msg, float * perc);

GAVL_PUBLIC void
gavl_msg_set_gui_button_press(gavl_msg_t * msg, int button,
                              int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_button_release(gavl_msg_t * msg, int button,
                                int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_get_gui_button(gavl_msg_t * msg, int * button,
                        int * mask, int * x, int * y, double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_key_press(gavl_msg_t * msg, int key,
                           int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_key_release(gavl_msg_t * msg, int key,
                             int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_get_gui_key(gavl_msg_t * msg, int * key,
                     int * mask, int * x, int * y, double * pos);

GAVL_PUBLIC void
gavl_msg_set_gui_motion(gavl_msg_t * msg, 
                        int mask, int x, int y, const double * pos);

GAVL_PUBLIC void
gavl_msg_get_gui_motion(gavl_msg_t * msg,
                        int * mask, int * x, int * y, double * pos);
                        

#endif // GAVL_MSG_H_INCLUDED

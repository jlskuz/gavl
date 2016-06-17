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

#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include <gavl/gavl.h>
#include <gavl/metadata.h>
#include <gavl/utils.h>
#include <gavl/msg.h>
#include <gavl/gavf.h>



static int type_has_buffer(int type)
  {
  switch(type)
    {
    case GAVL_MSG_TYPE_STRING:
    case GAVL_MSG_TYPE_AUDIO_FORMAT:
    case GAVL_MSG_TYPE_VIDEO_FORMAT:
    case GAVL_MSG_TYPE_METADATA:
      return 1;
    default:
      return 0;
    }
  }


void gavl_msg_set_id_ns(gavl_msg_t * msg, int id, int ns)
  {
  msg->id = id;
  msg->ns = ns;
  msg->num_args = 0;

  /* Zero everything */
  memset(&msg->args, 0, sizeof(msg->args));
  }

void gavl_msg_set_id(gavl_msg_t * msg, int id)
  {
  gavl_msg_set_id_ns(msg, id, 0);
  }

int gavl_msg_get_id_ns(gavl_msg_t * msg, int * ns)
  {
  *ns = msg->ns;
  return msg->id;
  }

void gavl_msg_copy(gavl_msg_t * dst, const gavl_msg_t * src)
  {
  int i;

  /* Also clears all memory */
  gavl_msg_set_id(dst, src->id);

  dst->id       = src->id;
  dst->ns       = src->ns;
  dst->num_args = src->num_args;

  for(i = 0; i < src->num_args; i++)
    {
    dst->args[i].type = src->args[i].type;

    switch(src->args[i].type)
      {
      case GAVL_MSG_TYPE_INT:
        dst->args[i].value.val_i = src->args[i].value.val_i;
        break;
      case GAVL_MSG_TYPE_FLOAT: 
        dst->args[i].value.val_f = src->args[i].value.val_f;
        break;
      case GAVL_MSG_TYPE_STRING:
      case GAVL_MSG_TYPE_AUDIO_FORMAT:
      case GAVL_MSG_TYPE_VIDEO_FORMAT:
      case GAVL_MSG_TYPE_METADATA:
        gavl_buffer_copy(&dst->args[i].value.val_buf, &src->args[i].value.val_buf);
        break;
      case GAVL_MSG_TYPE_TIME:
        dst->args[i].value.val_time = src->args[i].value.val_time;
        break;
      case GAVL_MSG_TYPE_COLOR_RGB:
        memcpy(dst->args[i].value.val_color, src->args[i].value.val_color, sizeof(src->args[i].value.val_color[0]) * 3);
        break;
      case GAVL_MSG_TYPE_COLOR_RGBA:
        memcpy(dst->args[i].value.val_color, src->args[i].value.val_color, sizeof(src->args[i].value.val_color[0]) * 4);
        break;
      case GAVL_MSG_TYPE_POSITION:
        memcpy(dst->args[i].value.val_pos, src->args[i].value.val_pos, sizeof(src->args[i].value.val_pos[0]) * 2);
        break;
      }
    
    }
  }

static int check_arg(int arg)
  {
  if(arg < 0)
    return 0;
  assert(arg < GAVL_MSG_MAX_ARGS);
  return 1;
  }

/* Set argument to a basic type */

void gavl_msg_set_arg_int(gavl_msg_t * msg, int arg, int value)
  {
  if(!check_arg(arg))
    return;
  msg->args[arg].value.val_i = value;
  msg->args[arg].type = GAVL_MSG_TYPE_INT;
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_time(gavl_msg_t * msg, int arg, gavl_time_t value)
  {
  if(!check_arg(arg))
    return;
  msg->args[arg].value.val_time = value;
  msg->args[arg].type = GAVL_MSG_TYPE_TIME;
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }



void gavl_msg_set_arg_string(gavl_msg_t * msg, int arg, const char * value)
  {
  if(value)
    {
    int length = strlen(value)+1;
    gavl_buffer_alloc(&msg->args[arg].value.val_buf, length);
    memcpy(msg->args[arg].value.val_buf.buf, value, length);
    msg->args[arg].value.val_buf.len = length;
    }
  else
    {
    gavl_buffer_free(&msg->args[arg].value.val_buf);
    gavl_buffer_init(&msg->args[arg].value.val_buf);
    }
  msg->args[arg].type = GAVL_MSG_TYPE_STRING;
  
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_float(gavl_msg_t * msg, int arg, double value)
  {
  if(!check_arg(arg))
    return;
  msg->args[arg].value.val_f = value;
  msg->args[arg].type = GAVL_MSG_TYPE_FLOAT;
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_color_rgb(gavl_msg_t * msg, int arg, const float * value)
  {
  if(!check_arg(arg))
    return;
  msg->args[arg].value.val_color[0] = value[0];
  msg->args[arg].value.val_color[1] = value[1];
  msg->args[arg].value.val_color[2] = value[2];
  msg->args[arg].type = GAVL_MSG_TYPE_COLOR_RGB;
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_color_rgba(gavl_msg_t * msg, int arg, const float * value)
  {
  if(!check_arg(arg))
    return;
  msg->args[arg].value.val_color[0] = value[0];
  msg->args[arg].value.val_color[1] = value[1];
  msg->args[arg].value.val_color[2] = value[2];
  msg->args[arg].value.val_color[3] = value[3];
  msg->args[arg].type = GAVL_MSG_TYPE_COLOR_RGBA;
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }

void gavl_msg_set_arg_position(gavl_msg_t * msg, int arg, const double * value)
  {
  if(!check_arg(arg))
    return;
  msg->args[arg].value.val_pos[0] = value[0];
  msg->args[arg].value.val_pos[1] = value[1];
  msg->args[arg].type = GAVL_MSG_TYPE_POSITION;
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;
  }


/* Get basic types */

int gavl_msg_get_id(gavl_msg_t * msg)
  {
  return msg->id;
  }

int gavl_msg_get_arg_int(gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return 0;
  return msg->args[arg].value.val_i;
  }

gavl_time_t gavl_msg_get_arg_time(gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return 0;
  return msg->args[arg].value.val_time;
  }

double gavl_msg_get_arg_float(gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return 0.0;
  return msg->args[arg].value.val_f;
  }

void gavl_msg_get_arg_color_rgb(gavl_msg_t * msg, int arg, float * val)
  {
  if(!check_arg(arg))
    return;
  val[0] = msg->args[arg].value.val_color[0];
  val[1] = msg->args[arg].value.val_color[1];
  val[2] = msg->args[arg].value.val_color[2];
  }

void gavl_msg_get_arg_color_rgba(gavl_msg_t * msg, int arg, float * val)
  {
  if(!check_arg(arg))
    return;
  val[0] = msg->args[arg].value.val_color[0];
  val[1] = msg->args[arg].value.val_color[1];
  val[2] = msg->args[arg].value.val_color[2];
  val[3] = msg->args[arg].value.val_color[3];
  }

void gavl_msg_get_arg_position(gavl_msg_t * msg, int arg, double * val)
  {
  if(!check_arg(arg))
    return;
  val[0] = msg->args[arg].value.val_pos[0];
  val[1] = msg->args[arg].value.val_pos[1];
  }


char * gavl_msg_get_arg_string(gavl_msg_t * msg, int arg)
  {
  char * ret;
  if(!check_arg(arg))
    return NULL;
  ret = (char *)msg->args[arg].value.val_buf.buf;
  msg->args[arg].value.val_buf.buf = NULL;
  msg->args[arg].value.val_buf.len = 0;
  msg->args[arg].value.val_buf.alloc = 0;
  return ret;
  }

const char * gavl_msg_get_arg_string_c(const gavl_msg_t * msg, int arg)
  {
  if(!check_arg(arg))
    return NULL;
  return (char *)msg->args[arg].value.val_buf.buf;
  }

/* Get/Set routines for structures */


static void set_arg_ptr_nocopy(gavl_msg_t * msg, int arg, void * value,
                               int len,
                               int type)
  {
  msg->args[arg].value.val_buf.buf = value;
  msg->args[arg].value.val_buf.len = len;
  msg->args[arg].value.val_buf.alloc = len;
  msg->args[arg].type = type;
  
  if(arg+1 > msg->num_args)
    msg->num_args = arg + 1;

  }

static void * get_arg_ptr(gavl_msg_t * msg, int arg, int * length)
  {
  void * ret;
  
  if(!check_arg(arg))
    return NULL;

  ret = msg->args[arg].value.val_buf.buf;
  msg->args[arg].value.val_buf.buf = NULL;
  
  if(length)
    *length = msg->args[arg].value.val_buf.len;
  msg->args[arg].value.val_buf.len = 0;
  msg->args[arg].value.val_buf.alloc = 0;
  return ret;
  }

void gavl_msg_set_arg_audio_format(gavl_msg_t * msg, int arg,
                                 const gavl_audio_format_t * format)
  {
  uint8_t * ptr1;
  int len;
  ptr1 = gavl_audio_format_to_buffer(&len, format);

  set_arg_ptr_nocopy(msg, arg, ptr1,
                     len,
                     GAVL_MSG_TYPE_AUDIO_FORMAT);
  }

void gavl_msg_get_arg_audio_format(gavl_msg_t * msg, int arg,
                                 gavl_audio_format_t * format)
  {
  uint8_t * ptr;
  int len = 0;
  ptr = get_arg_ptr(msg, arg, &len);
  gavl_audio_format_from_buffer(ptr, len, format);
  free(ptr);
  }

/* Video format */

void gavl_msg_set_arg_video_format(gavl_msg_t * msg, int arg,
                                 const gavl_video_format_t * format)
  {
  uint8_t * ptr1;
  int len = 0;
  ptr1 = gavl_video_format_to_buffer(&len, format);

  set_arg_ptr_nocopy(msg, arg, ptr1,
                     len,
                     GAVL_MSG_TYPE_VIDEO_FORMAT);
  }

void gavl_msg_get_arg_video_format(gavl_msg_t * msg, int arg,
                                 gavl_video_format_t * format)
  {
  uint8_t * ptr;
  int len = 0;
  
  ptr = get_arg_ptr(msg, arg, &len);
  gavl_video_format_from_buffer(ptr, len, format);
  free(ptr);
  }

void gavl_msg_set_arg_metadata(gavl_msg_t * msg, int arg,
                             const gavl_metadata_t * m)
  {
  int len;
  uint8_t * ptr1;
  
  ptr1 = gavl_metadata_to_buffer(&len, m);

  set_arg_ptr_nocopy(msg, arg, ptr1,
                     len,
                     GAVL_MSG_TYPE_METADATA);
  }
                            
void gavl_msg_get_arg_metadata(gavl_msg_t * msg, int arg,
                             gavl_metadata_t * m)
  {
  uint8_t * ptr;
  int len = 0;

  gavl_metadata_free(m);
  gavl_metadata_init(m);
  ptr = get_arg_ptr(msg, arg, &len);
  if(len > 0)
    {
    gavl_metadata_from_buffer(ptr, len, m);
    free(ptr);
    }
  }

void gavl_msg_init(gavl_msg_t * m)
  {
  memset(m, 0, sizeof(*m));
  m->id = -1;
  }

gavl_msg_t * gavl_msg_create()
  {
  gavl_msg_t * ret;
  ret = malloc(sizeof(*ret));
  gavl_msg_init(ret);
  return ret;
  }


void gavl_msg_free(gavl_msg_t * m)
  {
  int i;
  for(i = 0; i < m->num_args; i++)
    {
    if(type_has_buffer(m->args[i].type))
      gavl_buffer_free(&m->args[i].value.val_buf);
    }
  memset(m->args, 0, GAVL_MSG_MAX_ARGS * sizeof(m->args[0]));
  m->num_args = 0;
  m->id = -1;
  m->ns = 0;
  }

void gavl_msg_destroy(gavl_msg_t * m)
  {
  gavl_msg_free(m);
  free(m);
  }

void gavl_msg_dump(const gavl_msg_t * msg, int indent)
  {
  int i;

  gavl_dprintf("Message NS: %d ID: %d (%08x) args: %d\n", msg->ns, msg->id, msg->id, msg->num_args);
    
  for(i = 0; i < msg->num_args; i++)
    {
    switch(msg->args[i].type)
      {
      case GAVL_MSG_TYPE_INT:
        gavl_diprintf(indent + 2, "arg[%d]: Int: %d\n", i, msg->args[i].value.val_i);
        break;
      case GAVL_MSG_TYPE_FLOAT:
        gavl_diprintf(indent + 2, "arg[%d]: Float: %f\n", i, msg->args[i].value.val_f);
        break;
      case GAVL_MSG_TYPE_TIME:
        gavl_diprintf(indent + 2, "arg[%d]: Time: %"PRId64"\n",
                    i, msg->args[i].value.val_time);
        break;
      case GAVL_MSG_TYPE_COLOR_RGB:
        gavl_diprintf(indent + 2, "arg[%d]: RGB Color: %f,%f,%f\n",
                    i,
                    msg->args[i].value.val_color[0],
                    msg->args[i].value.val_color[1],
                    msg->args[i].value.val_color[2]);
        break;
      case GAVL_MSG_TYPE_COLOR_RGBA:
        gavl_diprintf(indent + 2, "arg[%d]: RGBA Color: %f,%f,%f,%f\n",
                    i,
                    msg->args[i].value.val_color[0],
                    msg->args[i].value.val_color[1],
                    msg->args[i].value.val_color[2],
                    msg->args[i].value.val_color[3]);
        break;
      case GAVL_MSG_TYPE_POSITION:
        gavl_diprintf(indent + 2, "arg[%d]: Pos: %f,%f\n",
                    i,
                    msg->args[i].value.val_pos[0],
                    msg->args[i].value.val_pos[1]);
        break;
      case GAVL_MSG_TYPE_STRING:
        gavl_diprintf(indent + 2, "arg[%d]: String: %s\n",
                    i,
                    (char*)msg->args[i].value.val_buf.buf);
        break;
      case GAVL_MSG_TYPE_AUDIO_FORMAT:
        {
        gavl_audio_format_t afmt;
        gavl_diprintf(indent + 2, "arg[%d]: Audio format\n", i);

        gavl_audio_format_from_buffer(msg->args[i].value.val_buf.buf, msg->args[i].value.val_buf.len, &afmt);
        gavl_audio_format_dumpi(&afmt, indent + 2);
        }
        break;
      case GAVL_MSG_TYPE_VIDEO_FORMAT:
        {
        gavl_video_format_t vfmt;
        gavl_diprintf(indent + 2, "arg[%d]: Video format\n", i);

        gavl_video_format_from_buffer(msg->args[i].value.val_buf.buf, msg->args[i].value.val_buf.len, &vfmt);
        gavl_video_format_dumpi(&vfmt, indent + 2);
        }
        break;
      case GAVL_MSG_TYPE_METADATA:
        {
        gavl_metadata_t m;
        gavl_metadata_init(&m);
        gavl_diprintf(indent + 2, "arg[%d]: Metadata\n", i);

        gavl_metadata_from_buffer(msg->args[i].value.val_buf.buf, msg->args[i].value.val_buf.len, &m);
        gavl_metadata_dump(&m, indent + 4);
        gavl_metadata_free(&m);
        }
        break;
      }
      
    }
  
  }

/* Set specific messages */

/** \brief Generic progress callback
 *
 *  arg0: Activity (string) 
 *  arg1: Percentage (0.0..1.0)
 */

void
gavl_msg_set_progress(gavl_msg_t * msg, const char * activity, float perc)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_PROGRESS, GAVL_MSG_NS_GENERIC);
  gavl_msg_set_arg_string(msg, 0, activity);
  gavl_msg_set_arg_float(msg, 1, perc);
  }

void
gavl_msg_get_progress(gavl_msg_t * msg, char ** activity, float * perc)
  {
  if(activity)
    *activity = gavl_msg_get_arg_string(msg, 0);
  if(perc)
    *perc = gavl_msg_get_arg_float(msg, 1);
  }



void
gavl_msg_set_src_metadata(gavl_msg_t * msg, int64_t time, int scale, const gavl_metadata_t * m)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_SRC_METADATA, GAVL_MSG_NS_SRC);
  gavl_msg_set_arg_time(msg, 0, time);
  gavl_msg_set_arg_int(msg, 1, scale);
  gavl_msg_set_arg_metadata(msg, 2, m);
  }

void
gavl_msg_get_src_metadata(gavl_msg_t * msg, int64_t * time, int * scale, gavl_metadata_t * m)
  {
  if(time)
    *time = gavl_msg_get_arg_time(msg, 0);
  if(scale)
    *scale = gavl_msg_get_arg_int(msg, 1);
  if(m)
    gavl_msg_get_arg_metadata(msg, 2, m);
  }


void
gavl_msg_set_src_aspect(gavl_msg_t * msg, int64_t time, int scale, int stream,
                        int pixel_width, int pixel_height)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_SRC_ASPECT, GAVL_MSG_NS_SRC);
  gavl_msg_set_arg_time(msg, 0, time);
  gavl_msg_set_arg_int(msg, 1, scale);
  gavl_msg_set_arg_int(msg, 2, stream);
  gavl_msg_set_arg_int(msg, 3, pixel_width);
  gavl_msg_set_arg_int(msg, 4, pixel_height);
  }

void
gavl_msg_get_src_aspect(gavl_msg_t * msg,
                        int64_t * time,
                        int * scale, int * stream,
                        int * pixel_width, int * pixel_height)
  {
  if(time)
    *time = gavl_msg_get_arg_time(msg, 0);
  if(scale)
    *scale = gavl_msg_get_arg_int(msg, 1);
  if(stream)
    *stream = gavl_msg_get_arg_int(msg, 2);
  if(pixel_width)
    *pixel_width = gavl_msg_get_arg_int(msg, 3);
  if(pixel_height)
    *pixel_height = gavl_msg_get_arg_int(msg, 4);
  }

void
gavl_msg_set_src_buffering(gavl_msg_t * msg, float perc)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_SRC_BUFFERING, GAVL_MSG_NS_SRC);
  gavl_msg_set_arg_float(msg, 0, perc);
  }

void
gavl_msg_get_src_buffering(gavl_msg_t * msg, float * perc)
  {
  if(perc)
    *perc = gavl_msg_get_arg_float(msg, 0);
  }

static void set_button_args(gavl_msg_t * msg, int64_t time, int scale, int button,
                            int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_arg_time(msg, 0, time);
  gavl_msg_set_arg_int(msg, 1, scale);
  gavl_msg_set_arg_int(msg, 2, button);
  gavl_msg_set_arg_int(msg, 3, mask);
  gavl_msg_set_arg_int(msg, 4, x);
  gavl_msg_set_arg_int(msg, 5, y);
  gavl_msg_set_arg_position(msg, 6, pos);
  }

void
gavl_msg_set_gui_button_press(gavl_msg_t * msg, int64_t time, int scale, int button,
                              int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_BUTTON_PRESS, GAVL_MSG_NS_GUI);
  set_button_args(msg, time, scale, button, mask, x, y, pos);
  }

void
gavl_msg_set_gui_button_release(gavl_msg_t * msg, int64_t time, int scale, int button,
                                int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_BUTTON_RELEASE, GAVL_MSG_NS_GUI);
  set_button_args(msg, time, scale, button, mask, x, y, pos);
  }

void
gavl_msg_get_gui_button(gavl_msg_t * msg, int64_t * time, int * scale, int * button,
                        int * mask, int * x, int * y, double * pos)
 
  {
  if(time)
    *time = gavl_msg_get_arg_time(msg, 0);
  if(scale)
    *scale = gavl_msg_get_arg_int(msg, 1);
  if(button)
    *button = gavl_msg_get_arg_int(msg, 2);
  if(mask)
    *mask = gavl_msg_get_arg_int(msg, 3);
  if(x)
    *x = gavl_msg_get_arg_int(msg, 4);
  if(y)
    *y = gavl_msg_get_arg_int(msg, 5);
  if(pos)
    gavl_msg_get_arg_position(msg, 6, pos);
  }

static void set_key_args(gavl_msg_t * msg, int64_t time, int scale, int key,
                         int mask, int x, int y, const double * pos)
  {
  gavl_msg_set_arg_time(msg, 0, time);
  gavl_msg_set_arg_int(msg, 1, scale);
  gavl_msg_set_arg_int(msg, 2, key);
  gavl_msg_set_arg_int(msg, 3, mask);
  gavl_msg_set_arg_int(msg, 4, x);
  gavl_msg_set_arg_int(msg, 5, y);
  gavl_msg_set_arg_position(msg, 6, pos);
  }

void
gavl_msg_set_gui_key_press(gavl_msg_t * msg, int64_t time, int scale, int key,
                           int mask, int x, int y, const double * pos)
  
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_KEY_PRESS, GAVL_MSG_NS_GUI);
  set_key_args(msg, time, scale, key, mask, x, y, pos);
  }

void
gavl_msg_set_gui_key_release(gavl_msg_t * msg, int64_t time, int scale, int key,
                             int mask, int x, int y, const double * pos)
  
  {
  gavl_msg_set_id_ns(msg, GAVL_MSG_GUI_KEY_RELEASE, GAVL_MSG_NS_GUI);
  set_key_args(msg, time, scale, key, mask, x, y, pos);
  }

void
gavl_msg_get_gui_key(gavl_msg_t * msg, int64_t * time, int * scale, int * key,
                     int * mask, int * x, int * y, double * pos)
  
  {
  if(time)
    *time = gavl_msg_get_arg_time(msg, 0);
  if(scale)
    *scale = gavl_msg_get_arg_int(msg, 1);
  if(key)
    *key = gavl_msg_get_arg_int(msg, 2);
  if(mask)
    *mask = gavl_msg_get_arg_int(msg, 3);
  if(x)
    *x = gavl_msg_get_arg_int(msg, 4);
  if(y)
    *y = gavl_msg_get_arg_int(msg, 5);
  if(pos)
    gavl_msg_get_arg_position(msg, 6, pos);
  }


void
gavl_msg_set_gui_motion(gavl_msg_t * msg, int64_t time, int scale, 
                        int mask, int x, int y, const double * pos)
  
  {
  gavl_msg_set_arg_time(msg, 0, time);
  gavl_msg_set_arg_int(msg, 1, scale);
  gavl_msg_set_arg_int(msg, 2, mask);
  gavl_msg_set_arg_int(msg, 3, x);
  gavl_msg_set_arg_int(msg, 4, y);
  gavl_msg_set_arg_position(msg, 5, pos);
  }

void
gavl_msg_get_gui_motion(gavl_msg_t * msg, int64_t * time, int * scale,
                        int * mask, int * x, int * y, double * pos)
  
  {
  if(time)
    *time = gavl_msg_get_arg_time(msg, 0);
  if(scale)
    *scale = gavl_msg_get_arg_int(msg, 1);
  if(mask)
    *mask = gavl_msg_get_arg_int(msg, 2);
  if(x)
    *x = gavl_msg_get_arg_int(msg, 3);
  if(y)
    *y = gavl_msg_get_arg_int(msg, 4);
  if(pos)
    gavl_msg_get_arg_position(msg, 5, pos);

  }

int gavl_msg_match(const gavl_msg_t * m, uint32_t id, uint32_t ns)
  {
  return ((m->id == id) && (m->ns == ns)) ? 1 : 0;
  }

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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Time <-> String */

void
gavl_metadata_date_to_string(int year,
                             int month,
                             int day, char * ret)
  {
  snprintf(ret, GAVL_METADATA_DATE_STRING_LEN,
           "%04d-%02d-%02d", year, month, day);
  }

void
gavl_metadata_date_time_to_string(int year,
                                  int month,
                                  int day,
                                  int hour,
                                  int minute,
                                  int second,
                                  char * ret)
  {
  snprintf(ret, GAVL_METADATA_DATE_TIME_STRING_LEN,
           "%04d-%02d-%02d %02d:%02d:%02d",
           year, month, day, hour, minute, second);
  }

void
gavl_dictionary_set_date(gavl_dictionary_t * m,
                       const char * key,
                       int year,
                       int month,
                       int day)
  {
  // YYYY-MM-DD
  char buf[GAVL_METADATA_DATE_STRING_LEN];
  gavl_metadata_date_to_string(year, month, day, buf);
  gavl_dictionary_set_string(m, key, buf);
  }

void
gavl_dictionary_set_date_time(gavl_dictionary_t * m,
                            const char * key,
                            int year,
                            int month,
                            int day,
                            int hour,
                            int minute,
                            int second)
  {
  // YYYY-MM-DD HH:MM:SS
  char buf[GAVL_METADATA_DATE_TIME_STRING_LEN];
  gavl_metadata_date_time_to_string(year,
                                    month,
                                    day,
                                    hour,
                                    minute,
                                    second,
                                    buf);
  gavl_dictionary_set_string(m, key, buf);
  }

GAVL_PUBLIC int
gavl_dictionary_get_date(gavl_dictionary_t * m,
                       const char * key,
                       int * year,
                       int * month,
                       int * day)
  {
  const char * val = gavl_dictionary_get_string(m, key);
  if(!val)
    return 0;

  if(sscanf(val, "%04d-%02d-%02d", year, month, day) < 3)
    return 0;
  return 1;
  }

GAVL_PUBLIC int
gavl_dictionary_get_date_time(gavl_dictionary_t * m,
                            const char * key,
                            int * year,
                            int * month,
                            int * day,
                            int * hour,
                            int * minute,
                            int * second)
  {
  const char * val = gavl_dictionary_get_string(m, key);
  if(!val)
    return 0;

  if(sscanf(val, "%04d-%02d-%02d %02d:%02d:%02d",
            year, month, day, hour, minute, second) < 6)
    return 0;
  return 1;
  }


int
gavl_metadata_equal(const gavl_dictionary_t * m1,
                    const gavl_dictionary_t * m2)
  {
  return !gavl_dictionary_compare(m1, m2);
  }

static const char * compression_fields[] =
  {
    GAVL_META_SOFTWARE,
    GAVL_META_MIMETYPE,
    GAVL_META_FORMAT,
    GAVL_META_BITRATE,
    GAVL_META_AUDIO_BITS,
    GAVL_META_VIDEO_BPP,
    NULL,
  };

const char * implicit_fields[] =
  {
    GAVL_META_URI,
    GAVL_META_BIG_ENDIAN,
    GAVL_META_AVG_BITRATE,
    GAVL_META_AVG_FRAMERATE,
    NULL,
  };


void
gavl_metadata_delete_compression_fields(gavl_dictionary_t * m)
  {
  gavl_dictionary_delete_fields(m, compression_fields);
  }

void
gavl_metadata_delete_implicit_fields(gavl_dictionary_t * m)
  {
  gavl_dictionary_delete_fields(m, implicit_fields);
  }

void
gavl_dictionary_set_string_endian(gavl_dictionary_t * m)
  {
  int val;
#ifdef WORDS_BIGENDIAN
  val = 1;
#else
  val = 0;
#endif
  gavl_dictionary_set_int(m, "BigEndian", val);
  }

int
gavl_metadata_do_swap_endian(const gavl_dictionary_t * m)
  {
  int val;

  if(!m)
    return 0;
  
  if(!gavl_dictionary_get_int(m, "BigEndian", &val))
    val = 0;
#ifdef WORDS_BIGENDIAN
  if(!val)
    return 1;
#else
  if(val)
    return 1;
#endif
  else
    return 0;
  }

/* Array functions */

void
gavl_metadata_append_nocpy(gavl_dictionary_t * m,
                           const char * key,
                           char * val)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_string_nocopy(&v, val);
  gavl_dictionary_append_nocopy(m, key, &v);
  }

void
gavl_metadata_append(gavl_dictionary_t * m,
                     const char * key,
                     const char * val)
  {
  gavl_metadata_append_nocpy(m, key, gavl_strdup(val));
  }

static char * metadata_get_arr_internal(const gavl_dictionary_t * m,
                                        const char * key,
                                        int i, int ign)
  {
  const gavl_value_t * val;

  if(ign)
    val = gavl_dictionary_get_i(m, key);
  else
    val = gavl_dictionary_get(m, key);
  
  if(val->type == GAVL_TYPE_ARRAY)
    {
    if(i < 0)
      return NULL;
    if(i >= val->v.array.num_entries)
      return NULL;
    if(val->v.array.entries[i].type != GAVL_TYPE_STRING)
      return NULL;
    return val->v.array.entries[i].v.str;
    }
  else if((val->type == GAVL_TYPE_STRING) && !i)
    return val->v.str;
  else
    return NULL;
  
  }

const char * 
gavl_dictionary_get_arr(const gavl_dictionary_t * m,
                      const char * key,
                      int i)
  {
  return metadata_get_arr_internal(m, key, i, 0);
  }

const char * 
gavl_dictionary_get_arr_i(const gavl_dictionary_t * m,
                        const char * key,
                        int i)
  {
  return metadata_get_arr_internal(m, key, i, 1);
  }



int gavl_dictionary_get_arr_len(const gavl_dictionary_t * m,
                              const char * key)
  {
  const gavl_value_t * val = gavl_dictionary_get(m, key);
  
  if(!val)
    return 0;
  else if(val->type == GAVL_TYPE_STRING)
    return 1;
  else if(val->type == GAVL_TYPE_ARRAY)
    return val->v.array.num_entries;
  else
    return 0;
  }
  
char * 
gavl_metadata_join_arr(const gavl_dictionary_t * m,
                       const char * key, const char * glue)
  {
#if 0
  char * ret;
  int ret_len;
  int glue_len;
  int i;
  int idx = find_tag(m, key);
  if(idx < 0)
    return NULL;
  
  glue_len = strlen(glue);
  
  ret_len = strlen(m->tags[idx].val) + 1;
  
  for(i = 0; i < m->tags[idx].arr_len; i++)
    ret_len += strlen(m->tags[idx].val_arr[i]) + glue_len;

  ret = malloc(ret_len);
  strncpy(ret, m->tags[idx].val, ret_len);
  for(i = 0; i < m->tags[idx].arr_len; i++)
    {
    strncat(ret, glue, ret_len - strlen(ret));
    strncat(ret, m->tags[idx].val_arr[i], ret_len - strlen(ret));
    }
  return ret;
#else
  char * ret;
  int ret_len;
  int glue_len;
  int i;
  int idx;
  const gavl_array_t * arr;

  const gavl_value_t * val = gavl_dictionary_get(m, key);
  if(!val)
    return NULL;
  if(val->type == GAVL_TYPE_STRING)
    return gavl_strdup(val->v.str);
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    glue_len = strlen(glue);
    
    ret_len = 1; // "\0"

    idx = 0;
    
    arr = &val->v.array;
    for(i = 0; i < arr->num_entries; i++)
      {
      if(arr->entries[i].type != GAVL_TYPE_STRING)
        continue;
      
      if(idx)
        ret_len += glue_len;
      
      ret_len += strlen(arr->entries[i].v.str);
      idx++;
      }

    ret = malloc(ret_len);
    *ret = '\0';

    if(!idx)
      return ret;
    
    idx = 0;
    for(i = 0; i < arr->num_entries; i++)
      {
      if(arr->entries[i].type != GAVL_TYPE_STRING)
        continue;
      
      if(idx)
        strncat(ret, glue, ret_len - strlen(ret));

      strncat(ret, arr->entries[i].v.str, ret_len - strlen(ret));
      idx++;
      }
    return ret;
    }
  else
    return NULL;
  
#endif
  }

// Format is w|h|mimetype|url

void gavl_metadata_add_image_uri(gavl_dictionary_t * m,
                                 const char * key,
                                 int w, int h,
                                 const char * mimetype,
                                 const char * uri)
  {
  gavl_value_t child;
  gavl_value_t val;

  gavl_dictionary_t * dict;
  
  gavl_value_init(&child);

  dict = gavl_value_set_dictionary(&child);
  
  gavl_value_init(&val);
  gavl_value_set_string(&val, uri);
  gavl_dictionary_set_nocopy(dict, GAVL_META_URI, &val);

  if(mimetype)
    {
    gavl_value_set_string(&val, mimetype);
    gavl_dictionary_set_nocopy(dict, GAVL_META_MIMETYPE, &val);
    }
  if(w > 0)
    {
    gavl_value_set_int(&val, w);
    gavl_dictionary_set_nocopy(dict, "w", &val);
    }
  if(h > 0)
    {
    gavl_value_set_int(&val, h);
    gavl_dictionary_set_nocopy(dict, "h", &val);
    }
  gavl_dictionary_append_nocopy(m, key, &child);
  }

const char * gavl_dictionary_get_string_image_uri(const gavl_dictionary_t * m,
                                         const char * key,
                                         int i,
                                         int * wp, int * hp,
                                         const char ** mimetype)
  {
  const gavl_dictionary_t * dict = NULL;
  const gavl_value_t * val;
  const char * ret;
  
  if(mimetype)
    *mimetype = NULL;
  
  if(!(val = gavl_dictionary_get(m, key)))
    return NULL;
  
  if((val->type == GAVL_TYPE_DICTIONARY) && !i)
    dict = &val->v.dictionary;
  else if(val->type == GAVL_TYPE_ARRAY)
    {
    val = gavl_array_get(&val->v.array, i);
    if(val->type == GAVL_TYPE_DICTIONARY)
      dict = &val->v.dictionary;
    }
  if(!dict)
    return NULL;

  if(!(val = gavl_dictionary_get(dict, GAVL_META_URI)))
    return NULL;

  ret = gavl_value_get_string_c(val);

  /* mimetype, width, height */
  if(mimetype && (val = gavl_dictionary_get(dict, GAVL_META_MIMETYPE)))
    *mimetype = gavl_strdup(gavl_value_get_string_c(val));
  if(wp  && (val = gavl_dictionary_get(dict, "w")))
    gavl_value_get_int(val, wp);
  if(hp  && (val = gavl_dictionary_get(dict, "h")))
    gavl_value_get_int(val, hp);

  return ret;
  }

const char *
gavl_dictionary_get_string_image_max(const gavl_dictionary_t * m,
                            const char * key,
                            int w, int h,
                            const char * mimetype)
  {
  int i;
  const gavl_value_t * val;
  
  int val_w = -1, val_h = -1;
  const char * val_mimetype;

  
  int num;
  int w_max = 0, i_max = -1;

  if(!(val = gavl_dictionary_get(m, key)))
    return NULL;
  
  num = gavl_value_get_num_items(val);
  
  for(i = 0; i < num; i++)
    {
    /* Invalid */
    if(!gavl_dictionary_get_string_image_uri(m,
                                    key,
                                    i,
                                    &val_w, &val_h,
                                    &val_mimetype))
      continue;

    /* Too large */
    if(((w > 0) && (val_w > w)) || ((h > 0) && (val_h > h)))
      continue;
      
    /* Wrong type */
    if(mimetype && val_mimetype && strcmp(mimetype, val_mimetype))
      continue;

    if(!w_max || !val_w || (w_max < val_w))
      {
      w_max = val_w;
      i_max = i;
      }
    }
  
  if(i_max < 0)
    i_max = 0;
  
  return gavl_dictionary_get_string_image_uri(m, key, i_max, NULL, NULL, NULL);
  }

gavl_dictionary_t *
gavl_metadata_add_src(gavl_dictionary_t * m, const char * key,
                      const char * mimetype, const char * location)
  {
  int idx;
  gavl_value_t val;
  gavl_value_t child_val;
  gavl_dictionary_t * ret;
  gavl_value_t * valp;
  /* Check if the location is already there */

  const char * loc;

  idx = 0;

  while((valp = gavl_dictionary_get_item_nc(m, GAVL_META_SRC, idx)))
    {
    if(valp->type != GAVL_TYPE_DICTIONARY)
      return NULL;
    
    ret = &valp->v.dictionary;
    
    loc = gavl_dictionary_get_string(ret, GAVL_META_URI);
    
    if(!strcmp(loc, location))
      {
      gavl_dictionary_set_int(m, GAVL_META_SRCIDX, idx);
      return ret;
      }
    else
      idx++;
    }

  ret = NULL;
  idx = 0;
  
  gavl_value_init(&val);
  gavl_value_init(&child_val);

  ret = gavl_value_set_dictionary(&val);
  
  gavl_dictionary_set_string(ret, GAVL_META_URI, location);
  
  if(mimetype)
    gavl_dictionary_set_string(ret, GAVL_META_MIMETYPE, mimetype);
  
  gavl_dictionary_append_nocopy(m, key, &val);
  idx = gavl_dictionary_find(m, key, 0);
  valp = &m->entries[idx].v;
  
  if(valp->type == GAVL_TYPE_ARRAY)
    return &valp->v.array.entries[valp->v.array.num_entries-1].v.dictionary;
  else if(valp->type == GAVL_TYPE_DICTIONARY)
    return &valp->v.dictionary;
  else
    return NULL;
  }

const gavl_dictionary_t *
gavl_dictionary_get_string_src(const gavl_dictionary_t * m, const char * key, int idx,
                               const char ** mimetype, const char ** location)
  {
  const gavl_value_t * val;
  const gavl_dictionary_t * dict;
  
  if(!(val = gavl_dictionary_get_item(m, key, idx)) ||
     (val->type != GAVL_TYPE_DICTIONARY))
    return NULL;

  dict = &val->v.dictionary;
  
  if(location)
    *location = gavl_dictionary_get_string(dict, GAVL_META_URI);
  
  if(mimetype)
    *mimetype = gavl_dictionary_get_string(dict, GAVL_META_MIMETYPE);
  return dict;
  }

int gavl_metadata_has_src(const gavl_dictionary_t * m, const char * key,
                          const char * location)
  {
  int i = 0;
  const char * loc;

  while(gavl_dictionary_get_string_src(m, key, i, NULL, &loc))
    {
    if(!strcmp(loc, location))
      return 1;
    i++;
    }
  return 0;
  }

/* */

static gavl_dictionary_t *
dictionary_get_stream(gavl_dictionary_t * d, int i, const char * tag)
  {
  gavl_array_t * arr;
  gavl_value_t * val;

  if((arr = gavl_dictionary_get_array_nc(d, tag)) &&
     (val = gavl_array_get_nc(arr, i)))
    return gavl_value_get_dictionary_nc(val);

  return NULL; 
  }

static const gavl_dictionary_t *
dictionary_get_stream_c(const gavl_dictionary_t * d, int i, const char * tag)
  {
  const gavl_array_t * arr;
  const gavl_value_t * val;

  if((arr = gavl_dictionary_get_array(d, tag)) &&
     (val = gavl_array_get(arr, i)))
    return gavl_value_get_dictionary(val);
  
  return NULL; 
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
  
  if(!(arr = gavl_dictionary_get_array_nc(d, tag)))
    {
    gavl_value_init(&val);
    gavl_value_set_array(&val);
    gavl_dictionary_set_nocopy(d, tag, &val);
    arr = gavl_dictionary_get_array_nc(d, tag);
    }

  gavl_value_init(&val);
  gavl_value_set_dictionary(&val);
  gavl_array_splice_val_nocopy(arr, arr->num_entries, 0, &val);

  return &arr->entries[arr->num_entries-1].v.dictionary;
  }

gavl_dictionary_t * gavl_dictionary_get_audio_stream(gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream(d, i, GAVL_META_AUDIO_STREAMS);
  }

const gavl_dictionary_t * gavl_dictionary_get_audio_stream_c(const gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream_c(d, i, GAVL_META_AUDIO_STREAMS);
  }
  

int gavl_dictionary_get_num_audio_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_AUDIO_STREAMS);
  }
  
gavl_dictionary_t * gavl_dictionary_append_audio_stream(gavl_dictionary_t * d)
  {
  return append_stream(d, GAVL_META_AUDIO_STREAMS);
  }

/* */
gavl_dictionary_t * gavl_dictionary_get_video_stream(gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream(d, i, GAVL_META_VIDEO_STREAMS);
  }

const gavl_dictionary_t * gavl_dictionary_get_video_stream_c(const gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream_c(d, i, GAVL_META_VIDEO_STREAMS);
  }
  
int gavl_dictionary_get_num_video_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_VIDEO_STREAMS);
  }
  
gavl_dictionary_t * gavl_dictionary_append_video_stream(gavl_dictionary_t * d)
  {
  return append_stream(d, GAVL_META_VIDEO_STREAMS);
  }

/* */
gavl_dictionary_t * gavl_dictionary_get_text_stream(gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream(d, i, GAVL_META_TEXT_STREAMS);
  }

const gavl_dictionary_t * gavl_dictionary_get_text_stream_c(const gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream_c(d, i, GAVL_META_TEXT_STREAMS);
  }

int gavl_dictionary_get_num_text_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_TEXT_STREAMS);

  }
  
gavl_dictionary_t * gavl_dictionary_append_text_stream(gavl_dictionary_t * d)
  {
  return append_stream(d, GAVL_META_TEXT_STREAMS);
  }
  
/* */
gavl_dictionary_t * gavl_dictionary_get_overlay_stream(gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream(d, i, GAVL_META_OVERLAY_STREAMS);
  }

const gavl_dictionary_t * gavl_dictionary_get_overlay_stream_c(const gavl_dictionary_t * d, int i)
  {
  return dictionary_get_stream_c(d, i, GAVL_META_OVERLAY_STREAMS);
  }

int gavl_dictionary_get_num_overlay_streams(const gavl_dictionary_t * d)
  {
  return dictionary_get_num_streams(d, GAVL_META_OVERLAY_STREAMS);
  }
  
gavl_dictionary_t * gavl_dictionary_append_overlay_stream(gavl_dictionary_t * d)
  {
  return append_stream(d, GAVL_META_OVERLAY_STREAMS);
  }
     

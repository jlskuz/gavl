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

#if 0
static char * my_strdup(const char * str)
  {
  char * ret;
  int len = strlen(str) + 1;

  ret = malloc(len);
  strncpy(ret, str, len);
  return ret;
  }

static void free_val(gavl_metadata_tag_t * tag)
  {
  int j;
  free(tag->val);
  
  if(tag->val_arr)
    {
    for(j = 0; j < tag->arr_len; j++)
      free(tag->val_arr[j]);
    free(tag->val_arr);
    }
  }

static void init_val(gavl_metadata_tag_t * tag)
  {
  char * key = tag->key;
  memset(tag, 0, sizeof(*tag));
  tag->key = key;
  }

static void free_tag(gavl_metadata_tag_t * tag)
  {
  free(tag->key);
  free_val(tag);
  }

void
gavl_metadata_free(gavl_metadata_t * m)
  {
  int i;
  for(i = 0; i < m->num_tags; i++)
    free_tag(&m->tags[i]);
  
  if(m->tags)
    free(m->tags);
  gavl_metadata_init(m);
  }
static void gavl_metadata_copy_tag(gavl_metadata_t * dst,
                                   gavl_metadata_tag_t * src)
  {
  int i;
  gavl_metadata_set(dst, src->key, src->val);

  for(i = 0; i < src->arr_len; i++)
    {
    gavl_metadata_append(dst, src->key, src->val_arr[i]);
    }
  }

void
gavl_metadata_copy(gavl_metadata_t * dst,
                   const gavl_metadata_t * src)
  {
  int i;
  if(!src->tags_alloc)
    return;
  for(i = 0; i < src->num_tags; i++)
    gavl_metadata_copy_tag(dst, &src->tags[i]);
  }

static int find_tag(const gavl_metadata_t * m, const char * key)
  {
  int i;
  for(i = 0; i < m->num_tags; i++)
    {
    if(!strcmp(m->tags[i].key, key))
      return i;
    }
  return -1;
  }

static int find_tag_i(const gavl_metadata_t * m, const char * key)
  {
  int i;
  for(i = 0; i < m->num_tags; i++)
    {
    if(!strcasecmp(m->tags[i].key, key))
      return i;
    }
  return -1;
  }

void
gavl_metadata_init(gavl_metadata_t * m)
  {
  memset(m, 0, sizeof(*m));
  }

void
gavl_metadata_set(gavl_metadata_t * m,
                  const char * key,
                  const char * val_c)
  {
  char * val;
  if(val_c && (*val_c != '\0'))
    val = my_strdup(val_c);
  else
    val = NULL;
  gavl_metadata_set_nocpy(m, key, val);
  }

static gavl_metadata_tag_t * create_tag(gavl_metadata_t * m,
                                        const char * key)
  {
  gavl_metadata_tag_t * ret;
  if(m->num_tags + 1 > m->tags_alloc)
    {
    m->tags_alloc = m->num_tags + 16;
    m->tags = realloc(m->tags,
                      m->tags_alloc * sizeof(*m->tags));
    memset(m->tags + m->num_tags,
           0, (m->tags_alloc - m->num_tags) * sizeof(*m->tags));
    }
  
  ret = &m->tags[m->num_tags];
  ret->key = my_strdup(key);
  m->num_tags++;
  return ret;
  }

static gavl_metadata_tag_t * ensure_tag(gavl_metadata_t * m,
                                        const char * key)
  {
  int idx;
  if((idx = find_tag(m, key)) > 0)
    return &m->tags[idx];
  else
    return create_tag(m, key);
  }
void
gavl_metadata_set_nocpy(gavl_metadata_t * m,
                        const char * key,
                        char * val)
  {
  int idx = find_tag(m, key);

  if(idx >= 0) // Tag exists
    {
    free_val(&m->tags[idx]);
    init_val(&m->tags[idx]);
    
    if(val && (*val != '\0')) // Replace tag
      m->tags[idx].val = val;
    else // Delete tag
      {
      free_tag(&m->tags[idx]);
      if(idx < (m->num_tags - 1))
        {
        memmove(m->tags + idx, m->tags + idx + 1,
                (m->num_tags - 1 - idx) * sizeof(*m->tags));
        }
      m->num_tags--;
      }
    }
  else if(val)
    {
    gavl_metadata_tag_t * tag = create_tag(m, key);
    tag->val = val;
    }
  }
const char * gavl_metadata_get(const gavl_metadata_t * m,
                               const char * key)
  {
  int idx = find_tag(m, key);
  if(idx < 0)
    return NULL;
  return m->tags[idx].val;
  }

const char * gavl_metadata_get_i(const gavl_metadata_t * m,
                                 const char * key)
  {
  int idx = find_tag_i(m, key);
  if(idx < 0)
    return NULL;
  return m->tags[idx].val;
  }

void gavl_metadata_merge(gavl_metadata_t * dst,
                         const gavl_metadata_t * src1,
                         const gavl_metadata_t * src2)
  {
  int i;
  /* src1 has priority */
  for(i = 0; i < src1->num_tags; i++)
    gavl_metadata_copy_tag(dst, &src1->tags[i]);

  /* From src2 we take only the tags, which aren't available */
  for(i = 0; i < src2->num_tags; i++)
    {
    if(!gavl_metadata_get(dst, src2->tags[i].key))
      gavl_metadata_copy_tag(dst, &src2->tags[i]);
    }
  }

void gavl_metadata_merge2(gavl_metadata_t * dst,
                          const gavl_metadata_t * src)
  {
  int i;
  for(i = 0; i < src->num_tags; i++)
    {
    if(!gavl_metadata_get(dst, src->tags[i].key))
      gavl_metadata_copy_tag(dst, &src->tags[i]);
    }
  }

GAVL_PUBLIC void
gavl_metadata_dump(const gavl_metadata_t * m, int indent)
  {
  int len, i, j, k;
  int max_key_len = 0;
  
  for(i = 0; i < m->num_tags; i++)
    {
    len = strlen(m->tags[i].key);
    if(len > max_key_len)
      max_key_len = len;
    }
  
  for(i = 0; i < m->num_tags; i++)
    {
    len = strlen(m->tags[i].key);

    for(j = 0; j < indent; j++)
      fprintf(stderr, " ");

    fprintf(stderr, "%s: ", m->tags[i].key);

    for(j = 0; j < max_key_len - len; j++)
      fprintf(stderr, " ");

    fprintf(stderr, "%s\n", m->tags[i].val);

    for(k = 0; k < m->tags[i].arr_len; k++)
      {
      for(j = 0; j < indent; j++)
        fprintf(stderr, " ");

      fprintf(stderr, "%s: ", m->tags[i].key);

      for(j = 0; j < max_key_len - len; j++)
        fprintf(stderr, " ");
      fprintf(stderr, "%s\n", m->tags[i].val_arr[k]);
      }
    }
  }

#endif


#define STR_SIZE 128

void
gavl_metadata_set_int(gavl_metadata_t * m,
                      const char * key,
                      int val)
  {
  char str[STR_SIZE];
  snprintf(str, STR_SIZE, "%d", val);
  gavl_metadata_set(m, key, str);
  }

void
gavl_metadata_set_long(gavl_metadata_t * m,
                       const char * key,
                       int64_t val)
  {
  char str[STR_SIZE];
  snprintf(str, STR_SIZE, "%"PRId64, val);
  gavl_metadata_set(m, key, str);
  }

void
gavl_metadata_set_float(gavl_metadata_t * m,
                        const char * key,
                        float val)
  {
  char str[STR_SIZE];
  snprintf(str, STR_SIZE, "%.6f", val);
  gavl_metadata_set(m, key, str);
  }

#undef STR_SIZE

int gavl_metadata_get_int(const gavl_metadata_t * m,
                          const char * key, int * ret)
  {
  char * rest;
  const char * val_str = gavl_metadata_get(m, key);
  if(!val_str)
    return 0;
  *ret = strtol(val_str, &rest, 10);
  if(*rest != '\0')
    return 0;
  return 1;
  }

int gavl_metadata_get_int_i(const gavl_metadata_t * m,
                            const char * key, int * ret)
  {
  char * rest;
  const char * val_str = gavl_metadata_get_i(m, key);
  if(!val_str)
    return 0;
  *ret = strtol(val_str, &rest, 10);
  if(*rest != '\0')
    return 0;
  return 1;
  }

int gavl_metadata_get_long(const gavl_metadata_t * m,
                           const char * key, int64_t * ret)
  {
  char * rest;
  const char * val_str = gavl_metadata_get(m, key);
  if(!val_str)
    return 0;
  *ret = strtoll(val_str, &rest, 10);
  if(*rest != '\0')
    return 0;
  return 1;
  }

int gavl_metadata_get_long_i(const gavl_metadata_t * m,
                             const char * key, int64_t * ret)
  {
  char * rest;
  const char * val_str = gavl_metadata_get_i(m, key);
  if(!val_str)
    return 0;
  *ret = strtoll(val_str, &rest, 10);
  if(*rest != '\0')
    return 0;
  return 1;
  }

int gavl_metadata_get_float(const gavl_metadata_t * m,
                           const char * key, float * ret)
  {
  const char * val_str = gavl_metadata_get(m, key);
  if(!val_str)
    return 0;
  if(sscanf(val_str, "%f", ret) != 1)
    return 0;
  return 1;
  }

int gavl_metadata_get_float_i(const gavl_metadata_t * m,
                             const char * key, float * ret)
  {
  const char * val_str = gavl_metadata_get_i(m, key);
  if(!val_str)
    return 0;
  if(sscanf(val_str, "%f", ret) != 1)
    return 0;
  return 1;
  }

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
gavl_metadata_set_date(gavl_metadata_t * m,
                       const char * key,
                       int year,
                       int month,
                       int day)
  {
  // YYYY-MM-DD
  char buf[GAVL_METADATA_DATE_STRING_LEN];
  gavl_metadata_date_to_string(year, month, day, buf);
  gavl_metadata_set(m, key, buf);
  }

void
gavl_metadata_set_date_time(gavl_metadata_t * m,
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
  gavl_metadata_set(m, key, buf);
  }

GAVL_PUBLIC int
gavl_metadata_get_date(gavl_metadata_t * m,
                       const char * key,
                       int * year,
                       int * month,
                       int * day)
  {
  const char * val = gavl_metadata_get(m, key);
  if(!val)
    return 0;

  if(sscanf(val, "%04d-%02d-%02d", year, month, day) < 3)
    return 0;
  return 1;
  }

GAVL_PUBLIC int
gavl_metadata_get_date_time(gavl_metadata_t * m,
                            const char * key,
                            int * year,
                            int * month,
                            int * day,
                            int * hour,
                            int * minute,
                            int * second)
  {
  const char * val = gavl_metadata_get(m, key);
  if(!val)
    return 0;

  if(sscanf(val, "%04d-%02d-%02d %02d:%02d:%02d",
            year, month, day, hour, minute, second) < 6)
    return 0;
  return 1;
  }


int
gavl_metadata_equal(const gavl_metadata_t * m1,
                    const gavl_metadata_t * m2)
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
    GAVL_META_LOCATION,
    GAVL_META_BIG_ENDIAN,
    GAVL_META_AVG_BITRATE,
    GAVL_META_AVG_FRAMERATE,
    NULL,
  };


void
gavl_metadata_delete_compression_fields(gavl_metadata_t * m)
  {
  gavl_dictionary_delete_fields(m, compression_fields);
  }

void
gavl_metadata_delete_implicit_fields(gavl_metadata_t * m)
  {
  gavl_dictionary_delete_fields(m, implicit_fields);
  }

void
gavl_metadata_set_endian(gavl_metadata_t * m)
  {
  int val;
#ifdef WORDS_BIGENDIAN
  val = 1;
#else
  val = 0;
#endif
  gavl_metadata_set_int(m, "BigEndian", val);
  }

int
gavl_metadata_do_swap_endian(const gavl_metadata_t * m)
  {
  int val;

  if(!m)
    return 0;
  
  if(!gavl_metadata_get_int(m, "BigEndian", &val))
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
gavl_metadata_append_nocpy(gavl_metadata_t * m,
                           const char * key,
                           char * val)
  {
  gavl_value_t v;
  gavl_value_init(&v);
  gavl_value_set_string_nocopy(&v, val);
  gavl_dictionary_append_nocopy(m, key, &v);
  }

void
gavl_metadata_append(gavl_metadata_t * m,
                     const char * key,
                     const char * val)
  {
  gavl_metadata_append_nocpy(m, key, gavl_strdup(val));
  }

static char * metadata_get_arr_internal(const gavl_metadata_t * m,
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
gavl_metadata_get_arr(const gavl_metadata_t * m,
                      const char * key,
                      int i)
  {
  return metadata_get_arr_internal(m, key, i, 0);
  }

const char * 
gavl_metadata_get_arr_i(const gavl_metadata_t * m,
                        const char * key,
                        int i)
  {
  return metadata_get_arr_internal(m, key, i, 1);
  }



int gavl_metadata_get_arr_len(const gavl_metadata_t * m,
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
gavl_metadata_join_arr(const gavl_metadata_t * m,
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

void gavl_metadata_add_image_uri(gavl_metadata_t * m,
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
  gavl_dictionary_set_nocopy(dict, GAVL_META_LOCATION, &val);

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

const char * gavl_metadata_get_image_uri(const gavl_metadata_t * m,
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

  if(!(val = gavl_dictionary_get(dict, GAVL_META_LOCATION)))
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
gavl_metadata_get_image_max(const gavl_metadata_t * m,
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
    if(!gavl_metadata_get_image_uri(m,
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
  
  return gavl_metadata_get_image_uri(m, key, i_max, NULL, NULL, NULL);
  }

gavl_dictionary_t *
gavl_metadata_add_src(gavl_metadata_t * m, const char * key,
                      const char * mimetype, const char * location)
  {
  int idx;
  gavl_value_t val;
  gavl_value_t child_val;
  gavl_dictionary_t * ret;
  gavl_value_t * valp;

  gavl_value_init(&val);
  gavl_value_init(&child_val);

  ret = gavl_value_set_dictionary(&val);
  
  gavl_value_set_string(&child_val, location);
  gavl_dictionary_set_nocopy(ret, GAVL_META_LOCATION, &child_val);
  
  if(mimetype)
    {
    gavl_value_set_string(&child_val, mimetype);
    gavl_dictionary_set_nocopy(ret, GAVL_META_MIMETYPE, &child_val);
    }
  
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
gavl_metadata_get_src(const gavl_metadata_t * m, const char * key, int idx,
                      char ** mimetype, char ** location)
  {
  const gavl_value_t * val;
  const gavl_dictionary_t * dict;
  
  if(!(val = gavl_dictionary_get_item(m, key, idx)) ||
     (val->type != GAVL_TYPE_DICTIONARY))
    return 0;

  dict = &val->v.dictionary;
  
  if(location)
    *location = gavl_strdup(gavl_dictionary_get_string(dict, GAVL_META_LOCATION));
  if(mimetype)
    *mimetype = gavl_strdup(gavl_dictionary_get_string(dict, GAVL_META_MIMETYPE));
  return dict;
  }

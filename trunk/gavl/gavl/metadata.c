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
#include <gavl/metadata.h>
#include <gavl/metatags.h>
#include <gavl/utils.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

void
gavl_metadata_init(gavl_metadata_t * m)
  {
  memset(m, 0, sizeof(*m));
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


#undef STR_SIZE

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

int
gavl_metadata_equal(const gavl_metadata_t * m1,
                    const gavl_metadata_t * m2)
  {
  int i;
  const char * val;

  /* Check if tags from m1 are present in m2 */
  for(i = 0; i < m1->num_tags; i++)
    {
    val = gavl_metadata_get(m2, m1->tags[i].key);
    if(!val || strcmp(val, m1->tags[i].val))
      return 0;
    }
  
  /* Check if tags from m2 are present in m1 */
  for(i = 0; i < m2->num_tags; i++)
    {
    if(!gavl_metadata_get(m1, m2->tags[i].key))
      return 0;
    }
  return 1;
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
    NULL,
  };

static void
delete_fields(gavl_metadata_t * m, const char * fields[])
  {
  int found;
  int i, j;

  i = 0;
  while(i < m->num_tags)
    {
    j = 0;

    found = 0;
    
    while(fields[j])
      {
      if(!strcmp(fields[j], m->tags[i].key))
        {
        gavl_metadata_set(m, fields[j], NULL);
        found = 1;
        break;
        }
      j++;
      }
    if(!found)
      i++;
    }
  }

void
gavl_metadata_delete_compression_fields(gavl_metadata_t * m)
  {
  delete_fields(m, compression_fields);
  }

void
gavl_metadata_delete_implicit_fields(gavl_metadata_t * m)
  {
  delete_fields(m, implicit_fields);
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
  gavl_metadata_tag_t * tag = ensure_tag(m, key);
  if(!val)
    return;
  
  if(!tag->val)
    {
    tag->val = val;
    return;
    }

  if(tag->arr_len + 1 > tag->arr_alloc)
    {
    tag->arr_alloc += 64;
    tag->val_arr = realloc(tag->val_arr,
                           tag->arr_alloc * sizeof(*tag->val_arr));
    
    memset(tag->val_arr + tag->arr_len, 0,
           (tag->arr_alloc - tag->arr_len) * sizeof(*tag->val_arr));
    }
  
  tag->val_arr[tag->arr_len] = val;
  tag->arr_len++;
  }


void
gavl_metadata_append(gavl_metadata_t * m,
                     const char * key,
                     const char * val)
  {
  gavl_metadata_append_nocpy(m, key, gavl_strdup(val));
  }

static const char * get_arr(gavl_metadata_tag_t * tag, int idx)
  {
  if(idx < 0)
    return NULL;

  if(!idx)
    return tag->val;

  idx--;
  
  if(idx >= tag->arr_len)
    return NULL;

  return tag->val_arr[idx];
  }

const char * 
gavl_metadata_get_arr(const gavl_metadata_t * m,
                      const char * key,
                      int i)
  {
  int idx = find_tag(m, key);
  if(idx < 0)
    return NULL;
  return get_arr(&m->tags[idx], i);
  }

int gavl_metadata_get_arr_len(const gavl_metadata_t * m,
                              const char * key)
  {
  int idx = find_tag(m, key);
  if(idx < 0)
    return 0;
  
  return m->tags[idx].arr_len + 1;
  }
  
char * 
gavl_metadata_join_arr(const gavl_metadata_t * m,
                       const char * key, const char * glue)
  {
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
  }


const char * 
gavl_metadata_get_arr_i(gavl_metadata_t * m,
                        const char * key,
                        int i)
  {
  int idx = find_tag_i(m, key);
  if(idx < 0)
    return NULL;
  return get_arr(&m->tags[idx], i);
  }

// Format is w|h|mimetype|url

void gavl_metadata_add_image_uri(gavl_metadata_t * m,
                                 const char * key,
                                 int w, int h,
                                 const char * mimetype,
                                 const char * uri)
  {
  char * value;
  char * ptr;
  int len = strlen(uri) + 32;
  if(mimetype)
    len += strlen(mimetype);

  value = malloc(len);

  ptr = value;

  if(w > 0)
    {
    snprintf(ptr, len - (ptr - value), "%d", w);
    ptr += strlen(ptr);
    }
  strncpy(ptr, "|", len - (ptr - value));
  ptr++;
  
  if(h > 0)
    {
    snprintf(ptr, len - (ptr - value), "%d", h);
    ptr += strlen(ptr);
    }
  strncpy(ptr, "|", len - (ptr - value));
  ptr++;

  if(mimetype)
    {
    strncpy(ptr, mimetype, len - (ptr - value));
    ptr += strlen(ptr);
    }
  strncpy(ptr, "|", len - (ptr - value));
  ptr++;
  
  strncpy(ptr, uri, len - (ptr - value));

  gavl_metadata_append_nocpy(m, key, value);
  
  }
                                 
const char * gavl_metadata_get_image_uri(const gavl_metadata_t * m,
                                         const char * key,
                                         int i,
                                         int * wp, int * hp,
                                         char ** mimetype)
  {
  int w = -1, h = -1;
  const char * val;
  const char * pos;
  char * end;

  if(mimetype)
    *mimetype = NULL;
  
  if(!(val = gavl_metadata_get_arr(m, key, i)))
    return NULL;

  pos = val;
  
  if(*pos != '|')
    {
    w = strtol(pos, &end, 10);
    if(end == pos)
      return NULL;
    pos = end;
    }
  pos++;

  if(*pos != '|')
    {
    h = strtol(pos, &end, 10);
    if(end == pos)
      return NULL;
    pos = end;
    }
  pos++;

  if(*pos != '|')
    {
    pos++;
    end = strchr(pos, '|');
    
    if(!end)
      return NULL;
    
    if(end)
      {
      if(mimetype)
        *mimetype = gavl_strndup(pos, end);
      }
    pos = end;
    }
  pos++;

  if(wp)
    *wp = w;
  if(hp)
    *hp = h;
  
  return pos;
  }

const char *
gavl_metadata_get_image_max(const gavl_metadata_t * m,
                            const char * key,
                            int w, int h,
                            const char * mimetype)
  {
  const char * var;
  int w_test, h_test;
  int i;
  const char * pos;
  
  int w_max = 0, i_max = -1;
  
  int num = gavl_metadata_get_arr_len(m, key);

  if(!num)
    return NULL;

  for(i = 0; i < num; i++)
    {
    var = gavl_metadata_get_arr(m, key, i);

    pos = var;
    
    if(*pos == '|')
      continue;

    if(sscanf(pos, "%d|%d|", &w_test, &h_test) < 2)
      continue;

    pos = strchr(pos, '|');
    pos++;
    pos = strchr(pos, '|');
    pos++;

    if(mimetype)
      {
      if((*pos != '|') && strncmp(mimetype, pos, strlen(mimetype)))
        continue;
      }
    
    if(((w > 0) && (w_test > w)) ||
       ((h > 0) && (h_test > h)))
      continue;
    
    if(w_max < w_test)
      {
      i_max = i;
      w_max = w_test;
      }
    }
  
  if(i_max < 0)
    i_max = 0;
  
  var = gavl_metadata_get_arr(m, key, i_max);
  pos = var;
  pos = strchr(pos, '|');
  pos++;
  pos = strchr(pos, '|');
  pos++;
  pos = strchr(pos, '|');
  pos++;
  
  return pos;
  }

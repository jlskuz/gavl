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

#include <gavl/gavl.h>
#include <gavl/chapterlist.h>
#include <gavl/metatags.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


static gavl_array_t * get_chapters(gavl_chapter_list_t * list)
  {
  gavl_value_t * valp;
  if(!(valp = gavl_dictionary_get_nc(list, GAVL_CHAPTERLIST_CHAPTERS)))
    {
    gavl_value_t val;
    gavl_value_init(&val);
    gavl_value_set_array(&val);
    gavl_dictionary_set_nocopy(list, GAVL_CHAPTERLIST_CHAPTERS, &val);
    valp = gavl_dictionary_get_nc(list, GAVL_CHAPTERLIST_CHAPTERS);
    }
  return &valp->v.array;
  }

static const gavl_array_t * get_chapters_c(const gavl_chapter_list_t * list)
  {
  const gavl_array_t * arr;
  const gavl_value_t * valp;
  
  if(!(valp = gavl_dictionary_get(list, GAVL_CHAPTERLIST_CHAPTERS)) ||
     !(arr = gavl_value_get_array(valp)))
    return NULL; 
  return arr;
  }

int gavl_chapter_list_is_valid(const gavl_chapter_list_t * list)
  {
  const gavl_array_t * arr = get_chapters_c(list);
  if(!arr || !arr->num_entries)
    return 0;
  return 1;
  }

gavl_dictionary_t * gavl_chapter_list_insert(gavl_chapter_list_t * list, int index,
                                             int64_t time, const char * name)
  {
  gavl_value_t val;
  gavl_dictionary_t * dict;
  
  gavl_array_t * arr = get_chapters(list);

  gavl_value_init(&val);
  dict = gavl_value_set_dictionary(&val);
  gavl_dictionary_set_long(dict, GAVL_CHAPTERLIST_TIME, time);
  if(name)
    gavl_dictionary_set_string(dict, GAVL_META_LABEL, name);

  gavl_array_splice_val_nocopy(arr, index, 0, &val);

  return &arr->entries[index].v.dictionary;
  }

void gavl_chapter_list_delete(gavl_chapter_list_t * list, int index)
  {
  gavl_array_t * arr = get_chapters(list);
  gavl_array_splice_val(arr, index, 1, NULL);
  }

static int64_t get_time(const gavl_chapter_list_t * list, int idx)
  {
  int64_t ret = GAVL_TIME_UNDEFINED;
  const gavl_array_t * arr;
  
  if((idx < 0) ||
     !(arr = get_chapters_c(list)) ||
     (idx >= arr->num_entries) ||
     !gavl_value_get_long(&arr->entries[idx], &ret))
    {
    return GAVL_TIME_UNDEFINED;
    }
  return ret;
  }

int gavl_chapter_list_get_current(gavl_chapter_list_t * list,
                                  gavl_time_t time)
  {
  int i;
  int timescale;
  int64_t time_scaled;
  gavl_array_t * arr = get_chapters(list);
  
  if(!(timescale = gavl_chapter_list_get_timescale(list)))
    return 0;
  
  time_scaled = gavl_time_scale(timescale, time);
  
  for(i = 0; i < arr->num_entries-1; i++)
    {
    if(time_scaled < get_time(list, i))
      return i;
    }
  return arr->num_entries-1;
  }

void gavl_chapter_list_set_timescale(gavl_chapter_list_t * list, int timescale)
  {
  gavl_dictionary_set_int(list, GAVL_CHAPTERLIST_TIMESCALE, timescale);
  }

int gavl_chapter_list_get_timescale(const gavl_chapter_list_t * list)
  {
  int timescale = 0;
  if(!gavl_dictionary_get_int(list, GAVL_CHAPTERLIST_TIMESCALE, &timescale))
    return 0;
  return timescale;
  }

int gavl_chapter_list_get_num(const gavl_chapter_list_t * list)
  {
  const gavl_array_t * arr;

  if(!(arr = get_chapters_c(list)))
    return 0;
  return arr->num_entries;
  }

gavl_dictionary_t * gavl_chapter_list_get(gavl_chapter_list_t * list, int idx)
  {
  gavl_array_t * arr = get_chapters(list);

  if((idx < 0) || (idx >= arr->num_entries) || (arr->entries[idx].type != GAVL_TYPE_DICTIONARY))
    return NULL;
  return &arr->entries[idx].v.dictionary;
  }

const gavl_dictionary_t * gavl_chapter_list_get_c(const gavl_chapter_list_t * list, int idx)
  {
  const gavl_array_t * arr;

  if(!(arr = get_chapters_c(list)))
    return NULL;

  if((idx < 0) || (idx >= arr->num_entries) || (arr->entries[idx].type != GAVL_TYPE_DICTIONARY))
    return NULL;
  return &arr->entries[idx].v.dictionary;
  }

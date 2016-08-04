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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <gavl/gavl.h>
#include <gavl/utils.h>
#include <gavl/value.h>

/* Array */

static gavl_value_t * array_append(gavl_array_t * m)
  {
  gavl_value_t * ret;
  if(m->num_entries == m->entries_alloc)
    {
    m->entries_alloc += 128;
    m->entries = realloc(m->entries, m->entries_alloc * sizeof(*m->entries));
    memset(m->entries + m->num_entries, 0, (m->entries_alloc - m->num_entries) * sizeof(*m->entries));
    }
  ret = m->entries + m->num_entries;
  m->num_entries++;
  return ret;
  }

void gavl_array_init(gavl_array_t * d)
  {
  memset(d, 0, sizeof(*d));
  }

void gavl_array_dump(const gavl_array_t * a, int indent)
  {
  int i;
  gavl_diprintf(0, "\n");
  gavl_diprintf(indent + 2, "[\n");
  
  for(i = 0; i < a->num_entries; i++)
    {
    gavl_value_dump(&a->entries[i], indent + 4);
    
    if(i < a->num_entries - 1)
      gavl_diprintf(0, ",\n");
    else
      gavl_diprintf(0, "\n");
    }
  gavl_diprintf(indent + 2, "]");
  }

void gavl_array_push(gavl_array_t * d, const gavl_value_t * val)
  {
  gavl_value_t * dst = array_append(d);
  gavl_value_copy(dst, val);
  }

void gavl_array_push_nocopy(gavl_array_t * d, gavl_value_t * val)
  {
  gavl_value_t * dst = array_append(d);
  gavl_value_move(dst, val);
  }

const gavl_value_t * gavl_array_get(gavl_array_t * d, int idx)
  {
  if((idx < 0) || (idx >= d->num_entries))
    return NULL;
  return d->entries + idx;
  }

void gavl_array_free(gavl_array_t * d)
  {
  int i;
  if(d->entries)
    {
    for(i = 0; i < d->num_entries; i++)
      gavl_value_free(d->entries + i);
    free(d->entries);
    }
  }

void gavl_array_copy(gavl_array_t * dst, const gavl_array_t * src)
  {
  int i;
  gavl_array_free(dst);
  gavl_array_init(dst);

  dst->entries_alloc = src->entries_alloc;
  dst->num_entries   = src->num_entries;
  dst->entries = calloc(dst->entries_alloc, sizeof(*dst->entries));
  
  for(i = 0; i < dst->num_entries; i++)
    gavl_value_copy(dst->entries + i, src->entries + i);
  
  }

int
gavl_array_compare(const gavl_array_t * m1,
                   const gavl_array_t * m2)
  {
  int i;
  int result = 1;
  
  if(m1->num_entries != m2->num_entries)
    return 1;
  
  /* Check if tags from m1 are present in m2 */
  for(i = 0; i < m1->num_entries; i++)
    {
    if((result = gavl_value_compare(&m1->entries[i], &m2->entries[i])))
      return result;
    }
  return 0;
  }

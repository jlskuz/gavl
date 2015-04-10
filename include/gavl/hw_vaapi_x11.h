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

#include <hw_vaapi.h>


/**
 * @file hw_vaapi_x11.h
 * external api header.
 */

/* Structure returned by gavl_hw_get_native_handle */
typedef struct
  {
  VADisplay dpy; // Must be first
  Display * x11_dpy;
  } gavl_hw_vaapi_x11_t;

gavl_hw_context_t * gavl_hw_create_vaapi_x11(Display * dpy);


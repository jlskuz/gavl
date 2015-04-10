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

#include <stdlib.h>

#include <X11/Xlib.h>
#include <hw_vaapi_x11.h>
#include <va/va_x11.h>

typedef struct
  {
  gavl_hw_vaapi_x11_t x11;
  
  Display * private_display;
  } vaapi_x11_t;

static void destroy_native(void * native)
  {
  vaapi_x11_t * priv = native;
  vaTerminate(priv->x11.dpy);
  if(priv->private_display)
    XCloseDisplay(priv->private_display);
  free(priv);
  }


static const gavl_hw_funcs_t funcs =
  {
    .destroy_native = destroy_native,
    .get_pixelformats = gavl_vaapi_get_pixelformats,
    .video_frame_create_hw = gavl_vaapi_video_frame_create_hw,
    .video_frame_create_ram = gavl_vaapi_video_frame_create_ram,
    .video_frame_destroy = gavl_vaapi_video_frame_destroy,
    .video_frame_to_ram = gavl_vaapi_video_frame_to_ram,
    .video_frame_to_hw  = gavl_vaapi_video_frame_to_hw
    
  };

gavl_hw_context_t * gavl_hw_create_vaapi_x11(Display * dpy)
  {
  int major, minor;
  vaapi_x11_t * priv;
  VAStatus result;
  
  priv = calloc(1, sizeof(*priv));

  if(!dpy)
    {
    priv->x11.x11_dpy = dpy;
    }
  else
    {
    priv->private_display = XOpenDisplay(NULL);

    if(!priv->private_display)
      return NULL;
    
    priv->x11.x11_dpy = priv->private_display;
    }
  
  priv->x11.dpy = vaGetDisplay(priv->x11.x11_dpy);

  if(!priv->x11.dpy)
    return NULL;
  
  if((result = vaInitialize(priv->x11.dpy,
                            &major,
                            &minor)) != VA_STATUS_SUCCESS)
    {
    return NULL;
    }

  return gavl_hw_context_create_internal(priv, &funcs);
  }

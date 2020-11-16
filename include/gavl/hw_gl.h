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

// #include <gavl/gavldefs.h>

int gavl_get_gl_format(gavl_pixelformat_t fmt, GLenum * format, GLenum * type);

gavl_pixelformat_t * gavl_gl_get_image_formats(gavl_hw_context_t * ctx);
gavl_pixelformat_t * gavl_gl_get_overlay_formats(gavl_hw_context_t * ctx);


void gavl_gl_adjust_video_format(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt);


/* The following functions require a current GL context */
GLuint gavl_gl_create_texture(const gavl_video_format_t * fmt);

void gavl_gl_frame_to_ram(const gavl_video_format_t * fmt,
                          gavl_video_frame_t * dst,
                          GLuint src);

void gavl_gl_frame_to_hw(const gavl_video_format_t * fmt,
                         GLuint dst,
                         gavl_video_frame_t * src);


/* Generic utilities */

const char * gavl_gl_get_error_string(GLenum err);
void gavl_gl_log_error(const char * funcname);
void gavl_gl_flush_errors();

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

/**
 * @file hw.h
 * external api header.
 */

typedef enum
  {
    GAVL_HW_NONE = 0, // Frames in RAM
    GAVL_HW_GLX,
    GAVL_HW_VAAPI_X11,
  } gavl_hw_type_t;

/* Global handle for accessing a piece of hardware */
typedef struct gavl_hw_context_s gavl_hw_context_t;

void gavl_hw_supported(gavl_hw_type_t type);

void gavl_hw_ctx_destroy(gavl_hw_context_t * ctx);

/* Return type is one of the structs in the backend specific headers */
void * gavl_hw_ctx_get_native_handle(gavl_hw_context_t * ctx);

const gavl_pixelformat_t * gavl_hw_ctx_get_pixelformats(gavl_hw_context_t * ctx);

/* Format will be adjusted if it's not supported by the hardware */
void gavl_hw_video_format_adjust(gavl_hw_context_t * ctx,
                                 gavl_video_format_t * fmt);

/* Create a video frame. The frame will be a reference for a hardware surface */
gavl_video_frame_t * gavl_hw_video_frame_create_hw(gavl_hw_context_t * ctx,
                                                   gavl_video_format_t * fmt);

/* Create a video frame. The frame will have data available for CPU access but is
 suitable for transfer to a hardware surface */
gavl_video_frame_t * gavl_hw_video_frame_create_ram(gavl_hw_context_t * ctx,
                                                    gavl_video_format_t * fmt);

/* Load a video frame from RAM into the hardware */
void gavl_video_frame_ram_to_hw(const gavl_video_format_t * fmt,
                                gavl_video_frame_t * dst,
                                const gavl_video_frame_t * src);

/* Load a video frame from the hardware into RAM */
void gavl_video_frame_hw_to_ram(const gavl_video_format_t * fmt,
                                gavl_video_frame_t * dst,
                                const gavl_video_frame_t * src);


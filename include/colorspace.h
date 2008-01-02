/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2008 Members of the Gmerlin project
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

/**************************************
* Function table for pixelformats
 **************************************/

typedef struct
  {
  /* Conversions among RGB formats */
  
  gavl_video_func_t swap_rgb_24;
  gavl_video_func_t swap_rgb_32;
  gavl_video_func_t swap_rgb_16;
  gavl_video_func_t swap_rgb_15;

  gavl_video_func_t rgb_15_to_16;
  gavl_video_func_t rgb_15_to_24;
  gavl_video_func_t rgb_15_to_32;
  gavl_video_func_t rgb_15_to_48;
  gavl_video_func_t rgb_15_to_float;

  gavl_video_func_t rgb_16_to_15;
  gavl_video_func_t rgb_16_to_24;
  gavl_video_func_t rgb_16_to_32;
  gavl_video_func_t rgb_16_to_48;
  gavl_video_func_t rgb_16_to_float;
  
  gavl_video_func_t rgb_24_to_15;
  gavl_video_func_t rgb_24_to_16;
  gavl_video_func_t rgb_24_to_32;
  gavl_video_func_t rgb_24_to_48;
  gavl_video_func_t rgb_24_to_float;

  gavl_video_func_t rgb_32_to_15;
  gavl_video_func_t rgb_32_to_16;
  gavl_video_func_t rgb_32_to_24;
  gavl_video_func_t rgb_32_to_48;
  gavl_video_func_t rgb_32_to_float;

  gavl_video_func_t rgb_48_to_15;
  gavl_video_func_t rgb_48_to_16;
  gavl_video_func_t rgb_48_to_24;
  gavl_video_func_t rgb_48_to_32;
  gavl_video_func_t rgb_48_to_48;
  gavl_video_func_t rgb_48_to_float;

  gavl_video_func_t rgb_float_to_15;
  gavl_video_func_t rgb_float_to_16;
  gavl_video_func_t rgb_float_to_24;
  gavl_video_func_t rgb_float_to_32;
  gavl_video_func_t rgb_float_to_48;
  
  gavl_video_func_t rgb_15_to_16_swap;
  gavl_video_func_t rgb_15_to_24_swap;
  gavl_video_func_t rgb_15_to_32_swap;
  gavl_video_func_t rgb_15_to_48_swap;
  gavl_video_func_t rgb_15_to_float_swap;

  gavl_video_func_t rgb_16_to_15_swap;
  gavl_video_func_t rgb_16_to_24_swap;
  gavl_video_func_t rgb_16_to_32_swap;
  gavl_video_func_t rgb_16_to_48_swap;
  gavl_video_func_t rgb_16_to_float_swap;
  
  gavl_video_func_t rgb_24_to_15_swap;
  gavl_video_func_t rgb_24_to_16_swap;
  gavl_video_func_t rgb_24_to_32_swap;
  gavl_video_func_t rgb_24_to_48_swap;
  gavl_video_func_t rgb_24_to_float_swap;
  
  gavl_video_func_t rgb_32_to_15_swap;
  gavl_video_func_t rgb_32_to_16_swap;
  gavl_video_func_t rgb_32_to_24_swap;
  gavl_video_func_t rgb_32_to_48_swap;
  gavl_video_func_t rgb_32_to_float_swap;

  gavl_video_func_t rgb_48_to_15_swap;
  gavl_video_func_t rgb_48_to_16_swap;
  gavl_video_func_t rgb_48_to_24_swap;
  gavl_video_func_t rgb_48_to_32_swap;

  gavl_video_func_t rgb_float_to_15_swap;
  gavl_video_func_t rgb_float_to_16_swap;
  gavl_video_func_t rgb_float_to_24_swap;
  gavl_video_func_t rgb_float_to_32_swap;
 
  /* Conversion from RGBA to RGB formats */

  gavl_video_func_t rgba_32_to_rgb_15;
  gavl_video_func_t rgba_32_to_bgr_15;
  gavl_video_func_t rgba_32_to_rgb_16;
  gavl_video_func_t rgba_32_to_bgr_16;
  gavl_video_func_t rgba_32_to_rgb_24;
  gavl_video_func_t rgba_32_to_bgr_24;
  gavl_video_func_t rgba_32_to_rgb_32;
  gavl_video_func_t rgba_32_to_bgr_32;
  gavl_video_func_t rgba_32_to_rgb_48;
  gavl_video_func_t rgba_32_to_rgba_64;
  gavl_video_func_t rgba_32_to_rgb_float;
  gavl_video_func_t rgba_32_to_rgba_float;

  gavl_video_func_t rgba_64_to_rgb_15;
  gavl_video_func_t rgba_64_to_bgr_15;
  gavl_video_func_t rgba_64_to_rgb_16;
  gavl_video_func_t rgba_64_to_bgr_16;
  gavl_video_func_t rgba_64_to_rgb_24;
  gavl_video_func_t rgba_64_to_bgr_24;
  gavl_video_func_t rgba_64_to_rgb_32;
  gavl_video_func_t rgba_64_to_bgr_32;
  gavl_video_func_t rgba_64_to_rgb_48;
  gavl_video_func_t rgba_64_to_rgb_float;

  gavl_video_func_t rgba_float_to_rgb_15;
  gavl_video_func_t rgba_float_to_bgr_15;
  gavl_video_func_t rgba_float_to_rgb_16;
  gavl_video_func_t rgba_float_to_bgr_16;
  gavl_video_func_t rgba_float_to_rgb_24;
  gavl_video_func_t rgba_float_to_bgr_24;
  gavl_video_func_t rgba_float_to_rgb_32;
  gavl_video_func_t rgba_float_to_bgr_32;
  gavl_video_func_t rgba_float_to_rgb_48;
  gavl_video_func_t rgba_float_to_rgb_float;
 
  /* Conversion from RGB formats to RGBA */

  gavl_video_func_t rgb_15_to_rgba_32;
  gavl_video_func_t bgr_15_to_rgba_32;
  gavl_video_func_t rgb_16_to_rgba_32;
  gavl_video_func_t bgr_16_to_rgba_32;
  gavl_video_func_t rgb_24_to_rgba_32;
  gavl_video_func_t bgr_24_to_rgba_32;
  gavl_video_func_t rgb_32_to_rgba_32;
  gavl_video_func_t bgr_32_to_rgba_32;
  gavl_video_func_t rgb_48_to_rgba_32;
  gavl_video_func_t rgba_64_to_rgba_32;
  gavl_video_func_t rgb_float_to_rgba_32;
  gavl_video_func_t rgba_float_to_rgba_32;

  gavl_video_func_t rgb_15_to_rgba_64;
  gavl_video_func_t bgr_15_to_rgba_64;
  gavl_video_func_t rgb_16_to_rgba_64;
  gavl_video_func_t bgr_16_to_rgba_64;
  gavl_video_func_t rgb_24_to_rgba_64;
  gavl_video_func_t bgr_24_to_rgba_64;
  gavl_video_func_t rgb_32_to_rgba_64;
  gavl_video_func_t bgr_32_to_rgba_64;
  gavl_video_func_t rgb_48_to_rgba_64;
  gavl_video_func_t rgb_float_to_rgba_64;
  gavl_video_func_t rgba_float_to_rgba_64;

  
  gavl_video_func_t rgb_15_to_rgba_float;
  gavl_video_func_t bgr_15_to_rgba_float;
  gavl_video_func_t rgb_16_to_rgba_float;
  gavl_video_func_t bgr_16_to_rgba_float;
  gavl_video_func_t rgb_24_to_rgba_float;
  gavl_video_func_t bgr_24_to_rgba_float;
  gavl_video_func_t rgb_32_to_rgba_float;
  gavl_video_func_t bgr_32_to_rgba_float;
  gavl_video_func_t rgb_48_to_rgba_float;
  gavl_video_func_t rgba_64_to_rgba_float;
  gavl_video_func_t rgb_float_to_rgba_float;
  
  /* RGB -> YUV */

  gavl_video_func_t rgb_15_to_yuy2;
  gavl_video_func_t rgb_15_to_yuva_32;
  gavl_video_func_t rgb_15_to_uyvy;
  gavl_video_func_t rgb_15_to_yuv_420_p;
  gavl_video_func_t rgb_15_to_yuv_410_p;
  gavl_video_func_t rgb_15_to_yuv_422_p;
  gavl_video_func_t rgb_15_to_yuv_422_p_16;
  gavl_video_func_t rgb_15_to_yuv_411_p;
  gavl_video_func_t rgb_15_to_yuv_444_p;
  gavl_video_func_t rgb_15_to_yuv_444_p_16;
  gavl_video_func_t rgb_15_to_yuvj_420_p;
  gavl_video_func_t rgb_15_to_yuvj_422_p;
  gavl_video_func_t rgb_15_to_yuvj_444_p;

  gavl_video_func_t bgr_15_to_yuy2;
  gavl_video_func_t bgr_15_to_yuva_32;
  gavl_video_func_t bgr_15_to_uyvy;
  gavl_video_func_t bgr_15_to_yuv_420_p;
  gavl_video_func_t bgr_15_to_yuv_410_p;
  gavl_video_func_t bgr_15_to_yuv_422_p;
  gavl_video_func_t bgr_15_to_yuv_422_p_16;
  gavl_video_func_t bgr_15_to_yuv_411_p;
  gavl_video_func_t bgr_15_to_yuv_444_p;
  gavl_video_func_t bgr_15_to_yuv_444_p_16;
  gavl_video_func_t bgr_15_to_yuvj_420_p;
  gavl_video_func_t bgr_15_to_yuvj_422_p;
  gavl_video_func_t bgr_15_to_yuvj_411_p;
  gavl_video_func_t bgr_15_to_yuvj_444_p;

  gavl_video_func_t rgb_16_to_yuy2;
  gavl_video_func_t rgb_16_to_yuva_32;
  gavl_video_func_t rgb_16_to_uyvy;
  gavl_video_func_t rgb_16_to_yuv_420_p;
  gavl_video_func_t rgb_16_to_yuv_410_p;
  gavl_video_func_t rgb_16_to_yuv_422_p;
  gavl_video_func_t rgb_16_to_yuv_422_p_16;
  gavl_video_func_t rgb_16_to_yuv_411_p;
  gavl_video_func_t rgb_16_to_yuv_444_p;
  gavl_video_func_t rgb_16_to_yuv_444_p_16;
  gavl_video_func_t rgb_16_to_yuvj_420_p;
  gavl_video_func_t rgb_16_to_yuvj_422_p;
  gavl_video_func_t rgb_16_to_yuvj_444_p;

  gavl_video_func_t bgr_16_to_yuy2;
  gavl_video_func_t bgr_16_to_yuva_32;
  gavl_video_func_t bgr_16_to_uyvy;
  gavl_video_func_t bgr_16_to_yuv_420_p;
  gavl_video_func_t bgr_16_to_yuv_410_p;
  gavl_video_func_t bgr_16_to_yuv_422_p;
  gavl_video_func_t bgr_16_to_yuv_422_p_16;
  gavl_video_func_t bgr_16_to_yuv_411_p;
  gavl_video_func_t bgr_16_to_yuv_444_p;
  gavl_video_func_t bgr_16_to_yuv_444_p_16;
  gavl_video_func_t bgr_16_to_yuvj_420_p;
  gavl_video_func_t bgr_16_to_yuvj_422_p;
  gavl_video_func_t bgr_16_to_yuvj_444_p;

  gavl_video_func_t rgb_24_to_yuy2;
  gavl_video_func_t rgb_24_to_yuva_32;
  gavl_video_func_t rgb_24_to_uyvy;
  gavl_video_func_t rgb_24_to_yuv_420_p;
  gavl_video_func_t rgb_24_to_yuv_410_p;
  gavl_video_func_t rgb_24_to_yuv_422_p;
  gavl_video_func_t rgb_24_to_yuv_422_p_16;
  gavl_video_func_t rgb_24_to_yuv_411_p;
  gavl_video_func_t rgb_24_to_yuv_444_p;
  gavl_video_func_t rgb_24_to_yuv_444_p_16;
  gavl_video_func_t rgb_24_to_yuvj_420_p;
  gavl_video_func_t rgb_24_to_yuvj_422_p;
  gavl_video_func_t rgb_24_to_yuvj_444_p;

  gavl_video_func_t bgr_24_to_yuy2;
  gavl_video_func_t bgr_24_to_yuva_32;
  gavl_video_func_t bgr_24_to_uyvy;
  gavl_video_func_t bgr_24_to_yuv_420_p;
  gavl_video_func_t bgr_24_to_yuv_410_p;
  gavl_video_func_t bgr_24_to_yuv_422_p;
  gavl_video_func_t bgr_24_to_yuv_422_p_16;
  gavl_video_func_t bgr_24_to_yuv_411_p;
  gavl_video_func_t bgr_24_to_yuv_444_p;
  gavl_video_func_t bgr_24_to_yuv_444_p_16;
  gavl_video_func_t bgr_24_to_yuvj_420_p;
  gavl_video_func_t bgr_24_to_yuvj_422_p;
  gavl_video_func_t bgr_24_to_yuvj_444_p;

  gavl_video_func_t rgb_32_to_yuy2;
  gavl_video_func_t rgb_32_to_yuva_32;
  gavl_video_func_t rgb_32_to_uyvy;
  gavl_video_func_t rgb_32_to_yuv_420_p;
  gavl_video_func_t rgb_32_to_yuv_410_p;
  gavl_video_func_t rgb_32_to_yuv_422_p;
  gavl_video_func_t rgb_32_to_yuv_422_p_16;
  gavl_video_func_t rgb_32_to_yuv_411_p;
  gavl_video_func_t rgb_32_to_yuv_444_p;
  gavl_video_func_t rgb_32_to_yuv_444_p_16;
  gavl_video_func_t rgb_32_to_yuvj_420_p;
  gavl_video_func_t rgb_32_to_yuvj_422_p;
  gavl_video_func_t rgb_32_to_yuvj_444_p;

  gavl_video_func_t bgr_32_to_yuy2;
  gavl_video_func_t bgr_32_to_yuva_32;
  gavl_video_func_t bgr_32_to_uyvy;
  gavl_video_func_t bgr_32_to_yuv_420_p;
  gavl_video_func_t bgr_32_to_yuv_410_p;
  gavl_video_func_t bgr_32_to_yuv_422_p;
  gavl_video_func_t bgr_32_to_yuv_422_p_16;
  gavl_video_func_t bgr_32_to_yuv_411_p;
  gavl_video_func_t bgr_32_to_yuv_444_p;
  gavl_video_func_t bgr_32_to_yuv_444_p_16;
  gavl_video_func_t bgr_32_to_yuvj_420_p;
  gavl_video_func_t bgr_32_to_yuvj_422_p;
  gavl_video_func_t bgr_32_to_yuvj_444_p;

  gavl_video_func_t rgba_32_to_yuy2;
  gavl_video_func_t rgba_32_to_yuva_32;
  gavl_video_func_t rgba_32_to_uyvy;
  gavl_video_func_t rgba_32_to_yuv_420_p;
  gavl_video_func_t rgba_32_to_yuv_410_p;
  gavl_video_func_t rgba_32_to_yuv_422_p;
  gavl_video_func_t rgba_32_to_yuv_422_p_16;
  gavl_video_func_t rgba_32_to_yuv_411_p;
  gavl_video_func_t rgba_32_to_yuv_444_p;
  gavl_video_func_t rgba_32_to_yuv_444_p_16;
  gavl_video_func_t rgba_32_to_yuvj_420_p;
  gavl_video_func_t rgba_32_to_yuvj_422_p;
  gavl_video_func_t rgba_32_to_yuvj_444_p;

  gavl_video_func_t rgb_48_to_yuy2;
  gavl_video_func_t rgb_48_to_yuva_32;
  gavl_video_func_t rgb_48_to_uyvy;
  gavl_video_func_t rgb_48_to_yuv_420_p;
  gavl_video_func_t rgb_48_to_yuv_410_p;
  gavl_video_func_t rgb_48_to_yuv_422_p;
  gavl_video_func_t rgb_48_to_yuv_422_p_16;
  gavl_video_func_t rgb_48_to_yuv_411_p;
  gavl_video_func_t rgb_48_to_yuv_444_p;
  gavl_video_func_t rgb_48_to_yuv_444_p_16;
  gavl_video_func_t rgb_48_to_yuvj_420_p;
  gavl_video_func_t rgb_48_to_yuvj_422_p;
  gavl_video_func_t rgb_48_to_yuvj_444_p;

  gavl_video_func_t rgba_64_to_yuy2;
  gavl_video_func_t rgba_64_to_yuva_32;
  gavl_video_func_t rgba_64_to_uyvy;
  gavl_video_func_t rgba_64_to_yuv_420_p;
  gavl_video_func_t rgba_64_to_yuv_410_p;
  gavl_video_func_t rgba_64_to_yuv_422_p;
  gavl_video_func_t rgba_64_to_yuv_422_p_16;
  gavl_video_func_t rgba_64_to_yuv_411_p;
  gavl_video_func_t rgba_64_to_yuv_444_p;
  gavl_video_func_t rgba_64_to_yuv_444_p_16;
  gavl_video_func_t rgba_64_to_yuvj_420_p;
  gavl_video_func_t rgba_64_to_yuvj_422_p;
  gavl_video_func_t rgba_64_to_yuvj_444_p;

  gavl_video_func_t rgb_float_to_yuy2;
  gavl_video_func_t rgb_float_to_yuva_32;
  gavl_video_func_t rgb_float_to_uyvy;
  gavl_video_func_t rgb_float_to_yuv_420_p;
  gavl_video_func_t rgb_float_to_yuv_410_p;
  gavl_video_func_t rgb_float_to_yuv_422_p;
  gavl_video_func_t rgb_float_to_yuv_422_p_16;
  gavl_video_func_t rgb_float_to_yuv_411_p;
  gavl_video_func_t rgb_float_to_yuv_444_p;
  gavl_video_func_t rgb_float_to_yuv_444_p_16;
  gavl_video_func_t rgb_float_to_yuvj_420_p;
  gavl_video_func_t rgb_float_to_yuvj_422_p;
  gavl_video_func_t rgb_float_to_yuvj_444_p;

  gavl_video_func_t rgba_float_to_yuy2;
  gavl_video_func_t rgba_float_to_yuva_32;
  gavl_video_func_t rgba_float_to_uyvy;
  gavl_video_func_t rgba_float_to_yuv_420_p;
  gavl_video_func_t rgba_float_to_yuv_410_p;
  gavl_video_func_t rgba_float_to_yuv_422_p;
  gavl_video_func_t rgba_float_to_yuv_422_p_16;
  gavl_video_func_t rgba_float_to_yuv_411_p;
  gavl_video_func_t rgba_float_to_yuv_444_p;
  gavl_video_func_t rgba_float_to_yuv_444_p_16;
  gavl_video_func_t rgba_float_to_yuvj_420_p;
  gavl_video_func_t rgba_float_to_yuvj_422_p;
  gavl_video_func_t rgba_float_to_yuvj_444_p;
 
  /* YUV -> RGB */

  gavl_video_func_t yuy2_to_rgb_15;
  gavl_video_func_t yuy2_to_bgr_15;
  gavl_video_func_t yuy2_to_rgb_16;
  gavl_video_func_t yuy2_to_bgr_16;
  gavl_video_func_t yuy2_to_rgb_24;
  gavl_video_func_t yuy2_to_bgr_24;
  gavl_video_func_t yuy2_to_rgb_32;
  gavl_video_func_t yuy2_to_bgr_32;
  gavl_video_func_t yuy2_to_rgba_32;
  gavl_video_func_t yuy2_to_rgb_48;
  gavl_video_func_t yuy2_to_rgba_64;
  gavl_video_func_t yuy2_to_rgb_float;
  gavl_video_func_t yuy2_to_rgba_float;

  gavl_video_func_t yuva_32_to_rgb_15;
  gavl_video_func_t yuva_32_to_bgr_15;
  gavl_video_func_t yuva_32_to_rgb_16;
  gavl_video_func_t yuva_32_to_bgr_16;
  gavl_video_func_t yuva_32_to_rgb_24;
  gavl_video_func_t yuva_32_to_bgr_24;
  gavl_video_func_t yuva_32_to_rgb_32;
  gavl_video_func_t yuva_32_to_bgr_32;
  gavl_video_func_t yuva_32_to_rgba_32;
  gavl_video_func_t yuva_32_to_rgb_48;
  gavl_video_func_t yuva_32_to_rgba_64;
  gavl_video_func_t yuva_32_to_rgb_float;
  gavl_video_func_t yuva_32_to_rgba_float;

  
  
  gavl_video_func_t uyvy_to_rgb_15;
  gavl_video_func_t uyvy_to_bgr_15;
  gavl_video_func_t uyvy_to_rgb_16;
  gavl_video_func_t uyvy_to_bgr_16;
  gavl_video_func_t uyvy_to_rgb_24;
  gavl_video_func_t uyvy_to_bgr_24;
  gavl_video_func_t uyvy_to_rgb_32;
  gavl_video_func_t uyvy_to_bgr_32;
  gavl_video_func_t uyvy_to_rgba_32;
  gavl_video_func_t uyvy_to_rgb_48;
  gavl_video_func_t uyvy_to_rgba_64;
  gavl_video_func_t uyvy_to_rgb_float;
  gavl_video_func_t uyvy_to_rgba_float;

  gavl_video_func_t yuv_420_p_to_rgb_15;
  gavl_video_func_t yuv_420_p_to_bgr_15;
  gavl_video_func_t yuv_420_p_to_rgb_16;
  gavl_video_func_t yuv_420_p_to_bgr_16;
  gavl_video_func_t yuv_420_p_to_rgb_24;
  gavl_video_func_t yuv_420_p_to_bgr_24;
  gavl_video_func_t yuv_420_p_to_rgb_32;
  gavl_video_func_t yuv_420_p_to_bgr_32;
  gavl_video_func_t yuv_420_p_to_rgba_32;
  gavl_video_func_t yuv_420_p_to_rgb_48;
  gavl_video_func_t yuv_420_p_to_rgba_64;
  gavl_video_func_t yuv_420_p_to_rgb_float;
  gavl_video_func_t yuv_420_p_to_rgba_float;
  
  gavl_video_func_t yuv_410_p_to_rgb_15;
  gavl_video_func_t yuv_410_p_to_bgr_15;
  gavl_video_func_t yuv_410_p_to_rgb_16;
  gavl_video_func_t yuv_410_p_to_bgr_16;
  gavl_video_func_t yuv_410_p_to_rgb_24;
  gavl_video_func_t yuv_410_p_to_bgr_24;
  gavl_video_func_t yuv_410_p_to_rgb_32;
  gavl_video_func_t yuv_410_p_to_bgr_32;
  gavl_video_func_t yuv_410_p_to_rgba_32;
  gavl_video_func_t yuv_410_p_to_rgb_48;
  gavl_video_func_t yuv_410_p_to_rgba_64;
  gavl_video_func_t yuv_410_p_to_rgb_float;
  gavl_video_func_t yuv_410_p_to_rgba_float;
  
  gavl_video_func_t yuvj_420_p_to_rgb_15;
  gavl_video_func_t yuvj_420_p_to_bgr_15;
  gavl_video_func_t yuvj_420_p_to_rgb_16;
  gavl_video_func_t yuvj_420_p_to_bgr_16;
  gavl_video_func_t yuvj_420_p_to_rgb_24;
  gavl_video_func_t yuvj_420_p_to_bgr_24;
  gavl_video_func_t yuvj_420_p_to_rgb_32;
  gavl_video_func_t yuvj_420_p_to_bgr_32;
  gavl_video_func_t yuvj_420_p_to_rgba_32;
  gavl_video_func_t yuvj_420_p_to_rgb_48;
  gavl_video_func_t yuvj_420_p_to_rgba_64;
  gavl_video_func_t yuvj_420_p_to_rgb_float;
  gavl_video_func_t yuvj_420_p_to_rgba_float;

  gavl_video_func_t yuv_422_p_to_rgb_15;
  gavl_video_func_t yuv_422_p_to_bgr_15;
  gavl_video_func_t yuv_422_p_to_rgb_16;
  gavl_video_func_t yuv_422_p_to_bgr_16;
  gavl_video_func_t yuv_422_p_to_rgb_24;
  gavl_video_func_t yuv_422_p_to_bgr_24;
  gavl_video_func_t yuv_422_p_to_rgb_32;
  gavl_video_func_t yuv_422_p_to_bgr_32;
  gavl_video_func_t yuv_422_p_to_rgba_32;
  gavl_video_func_t yuv_422_p_to_rgb_48;
  gavl_video_func_t yuv_422_p_to_rgba_64;
  gavl_video_func_t yuv_422_p_to_rgb_float;
  gavl_video_func_t yuv_422_p_to_rgba_float;

  gavl_video_func_t yuv_422_p_16_to_rgb_15;
  gavl_video_func_t yuv_422_p_16_to_bgr_15;
  gavl_video_func_t yuv_422_p_16_to_rgb_16;
  gavl_video_func_t yuv_422_p_16_to_bgr_16;
  gavl_video_func_t yuv_422_p_16_to_rgb_24;
  gavl_video_func_t yuv_422_p_16_to_bgr_24;
  gavl_video_func_t yuv_422_p_16_to_rgb_32;
  gavl_video_func_t yuv_422_p_16_to_bgr_32;
  gavl_video_func_t yuv_422_p_16_to_rgba_32;
  gavl_video_func_t yuv_422_p_16_to_rgb_48;
  gavl_video_func_t yuv_422_p_16_to_rgba_64;
  gavl_video_func_t yuv_422_p_16_to_rgb_float;
  gavl_video_func_t yuv_422_p_16_to_rgba_float;

  gavl_video_func_t yuv_411_p_to_rgb_15;
  gavl_video_func_t yuv_411_p_to_bgr_15;
  gavl_video_func_t yuv_411_p_to_rgb_16;
  gavl_video_func_t yuv_411_p_to_bgr_16;
  gavl_video_func_t yuv_411_p_to_rgb_24;
  gavl_video_func_t yuv_411_p_to_bgr_24;
  gavl_video_func_t yuv_411_p_to_rgb_32;
  gavl_video_func_t yuv_411_p_to_bgr_32;
  gavl_video_func_t yuv_411_p_to_rgba_32;
  gavl_video_func_t yuv_411_p_to_rgb_48;
  gavl_video_func_t yuv_411_p_to_rgba_64;
  gavl_video_func_t yuv_411_p_to_rgb_float;
  gavl_video_func_t yuv_411_p_to_rgba_float;

  gavl_video_func_t yuvj_422_p_to_rgb_15;
  gavl_video_func_t yuvj_422_p_to_bgr_15;
  gavl_video_func_t yuvj_422_p_to_rgb_16;
  gavl_video_func_t yuvj_422_p_to_bgr_16;
  gavl_video_func_t yuvj_422_p_to_rgb_24;
  gavl_video_func_t yuvj_422_p_to_bgr_24;
  gavl_video_func_t yuvj_422_p_to_rgb_32;
  gavl_video_func_t yuvj_422_p_to_bgr_32;
  gavl_video_func_t yuvj_422_p_to_rgba_32;
  gavl_video_func_t yuvj_422_p_to_rgb_48;
  gavl_video_func_t yuvj_422_p_to_rgba_64;
  gavl_video_func_t yuvj_422_p_to_rgb_float;
  gavl_video_func_t yuvj_422_p_to_rgba_float;

  gavl_video_func_t yuv_444_p_to_rgb_15;
  gavl_video_func_t yuv_444_p_to_bgr_15;
  gavl_video_func_t yuv_444_p_to_rgb_16;
  gavl_video_func_t yuv_444_p_to_bgr_16;
  gavl_video_func_t yuv_444_p_to_rgb_24;
  gavl_video_func_t yuv_444_p_to_bgr_24;
  gavl_video_func_t yuv_444_p_to_rgb_32;
  gavl_video_func_t yuv_444_p_to_bgr_32;
  gavl_video_func_t yuv_444_p_to_rgba_32;
  gavl_video_func_t yuv_444_p_to_rgb_48;
  gavl_video_func_t yuv_444_p_to_rgba_64;
  gavl_video_func_t yuv_444_p_to_rgb_float;
  gavl_video_func_t yuv_444_p_to_rgba_float;

  gavl_video_func_t yuv_444_p_16_to_rgb_15;
  gavl_video_func_t yuv_444_p_16_to_bgr_15;
  gavl_video_func_t yuv_444_p_16_to_rgb_16;
  gavl_video_func_t yuv_444_p_16_to_bgr_16;
  gavl_video_func_t yuv_444_p_16_to_rgb_24;
  gavl_video_func_t yuv_444_p_16_to_bgr_24;
  gavl_video_func_t yuv_444_p_16_to_rgb_32;
  gavl_video_func_t yuv_444_p_16_to_bgr_32;
  gavl_video_func_t yuv_444_p_16_to_rgba_32;
  gavl_video_func_t yuv_444_p_16_to_rgb_48;
  gavl_video_func_t yuv_444_p_16_to_rgba_64;
  gavl_video_func_t yuv_444_p_16_to_rgb_float;
  gavl_video_func_t yuv_444_p_16_to_rgba_float;

  
  gavl_video_func_t yuvj_444_p_to_rgb_15;
  gavl_video_func_t yuvj_444_p_to_bgr_15;
  gavl_video_func_t yuvj_444_p_to_rgb_16;
  gavl_video_func_t yuvj_444_p_to_bgr_16;
  gavl_video_func_t yuvj_444_p_to_rgb_24;
  gavl_video_func_t yuvj_444_p_to_bgr_24;
  gavl_video_func_t yuvj_444_p_to_rgb_32;
  gavl_video_func_t yuvj_444_p_to_bgr_32;
  gavl_video_func_t yuvj_444_p_to_rgba_32;
  gavl_video_func_t yuvj_444_p_to_rgb_48;
  gavl_video_func_t yuvj_444_p_to_rgba_64;
  gavl_video_func_t yuvj_444_p_to_rgb_float;
  gavl_video_func_t yuvj_444_p_to_rgba_float;
  
  /* Conversions among YUV formats */

  gavl_video_func_t yuy2_to_yuv_420_p;
  gavl_video_func_t yuy2_to_yuv_410_p;
  gavl_video_func_t yuy2_to_yuv_422_p;
  gavl_video_func_t yuy2_to_yuv_422_p_16;
  gavl_video_func_t yuy2_to_yuv_411_p;
  gavl_video_func_t yuy2_to_yuv_444_p;
  gavl_video_func_t yuy2_to_yuv_444_p_16;
  gavl_video_func_t yuy2_to_yuvj_420_p;
  gavl_video_func_t yuy2_to_yuvj_422_p;
  gavl_video_func_t yuy2_to_yuvj_444_p;
  gavl_video_func_t yuy2_to_yuva_32;

  gavl_video_func_t uyvy_to_yuy2;
  gavl_video_func_t uyvy_to_yuva_32;
  gavl_video_func_t uyvy_to_yuv_420_p;
  gavl_video_func_t uyvy_to_yuv_410_p;
  gavl_video_func_t uyvy_to_yuv_422_p;
  gavl_video_func_t uyvy_to_yuv_422_p_16;
  gavl_video_func_t uyvy_to_yuv_411_p;
  gavl_video_func_t uyvy_to_yuv_444_p;
  gavl_video_func_t uyvy_to_yuv_444_p_16;
  gavl_video_func_t uyvy_to_yuvj_420_p;
  gavl_video_func_t uyvy_to_yuvj_422_p;
  gavl_video_func_t uyvy_to_yuvj_444_p;
  
  gavl_video_func_t yuv_420_p_to_yuv_422_p;
  gavl_video_func_t yuv_420_p_to_yuv_422_p_16;
  gavl_video_func_t yuv_420_p_to_yuv_411_p;
  gavl_video_func_t yuv_420_p_to_yuv_444_p;
  gavl_video_func_t yuv_420_p_to_yuv_444_p_16;
  gavl_video_func_t yuv_420_p_to_yuv_410_p;
  gavl_video_func_t yuv_420_p_to_yuvj_420_p;
  gavl_video_func_t yuv_420_p_to_yuvj_422_p;
  gavl_video_func_t yuv_420_p_to_yuvj_444_p;
  gavl_video_func_t yuv_420_p_to_yuy2;
  gavl_video_func_t yuv_420_p_to_yuva_32;
  gavl_video_func_t yuv_420_p_to_uyvy;

  gavl_video_func_t yuv_410_p_to_yuv_420_p;
  gavl_video_func_t yuv_410_p_to_yuv_422_p;
  gavl_video_func_t yuv_410_p_to_yuv_422_p_16;
  gavl_video_func_t yuv_410_p_to_yuv_411_p;
  gavl_video_func_t yuv_410_p_to_yuv_444_p;
  gavl_video_func_t yuv_410_p_to_yuv_444_p_16;
  gavl_video_func_t yuv_410_p_to_yuvj_420_p;
  gavl_video_func_t yuv_410_p_to_yuvj_422_p;
  gavl_video_func_t yuv_410_p_to_yuvj_444_p;
  gavl_video_func_t yuv_410_p_to_yuy2;
  gavl_video_func_t yuv_410_p_to_yuva_32;
  gavl_video_func_t yuv_410_p_to_uyvy;

  gavl_video_func_t yuv_422_p_to_yuv_420_p;
  gavl_video_func_t yuv_422_p_to_yuv_410_p;
  gavl_video_func_t yuv_422_p_to_yuv_411_p;
  gavl_video_func_t yuv_422_p_to_yuv_444_p;
  gavl_video_func_t yuv_422_p_to_yuv_444_p_16;
  gavl_video_func_t yuv_422_p_to_yuv_422_p_16;
  gavl_video_func_t yuv_422_p_to_yuvj_420_p;
  gavl_video_func_t yuv_422_p_to_yuvj_422_p;
  gavl_video_func_t yuv_422_p_to_yuvj_444_p;
  gavl_video_func_t yuv_422_p_to_yuy2;
  gavl_video_func_t yuv_422_p_to_yuva_32;
  gavl_video_func_t yuv_422_p_to_uyvy;

  gavl_video_func_t yuv_422_p_16_to_yuv_420_p;
  gavl_video_func_t yuv_422_p_16_to_yuv_410_p;
  gavl_video_func_t yuv_422_p_16_to_yuv_411_p;
  gavl_video_func_t yuv_422_p_16_to_yuv_444_p;
  gavl_video_func_t yuv_422_p_16_to_yuv_422_p;
  gavl_video_func_t yuv_422_p_16_to_yuv_444_p_16;
  gavl_video_func_t yuv_422_p_16_to_yuvj_420_p;
  gavl_video_func_t yuv_422_p_16_to_yuvj_422_p;
  gavl_video_func_t yuv_422_p_16_to_yuvj_444_p;
  gavl_video_func_t yuv_422_p_16_to_yuy2;
  gavl_video_func_t yuv_422_p_16_to_yuva_32;
  gavl_video_func_t yuv_422_p_16_to_uyvy;

  
  gavl_video_func_t yuv_411_p_to_yuv_420_p;
  gavl_video_func_t yuv_411_p_to_yuv_410_p;
  gavl_video_func_t yuv_411_p_to_yuv_444_p;
  gavl_video_func_t yuv_411_p_to_yuv_444_p_16;
  gavl_video_func_t yuv_411_p_to_yuv_422_p;
  gavl_video_func_t yuv_411_p_to_yuv_422_p_16;
  gavl_video_func_t yuv_411_p_to_yuvj_420_p;
  gavl_video_func_t yuv_411_p_to_yuvj_422_p;
  gavl_video_func_t yuv_411_p_to_yuvj_444_p;
  gavl_video_func_t yuv_411_p_to_yuy2;
  gavl_video_func_t yuv_411_p_to_yuva_32;
  gavl_video_func_t yuv_411_p_to_uyvy;
  
  gavl_video_func_t yuv_444_p_to_yuv_420_p;
  gavl_video_func_t yuv_444_p_to_yuv_410_p;
  gavl_video_func_t yuv_444_p_to_yuv_422_p;
  gavl_video_func_t yuv_444_p_to_yuv_422_p_16;
  gavl_video_func_t yuv_444_p_to_yuv_444_p_16;
  gavl_video_func_t yuv_444_p_to_yuv_411_p;
  gavl_video_func_t yuv_444_p_to_yuvj_420_p;
  gavl_video_func_t yuv_444_p_to_yuvj_422_p;
  gavl_video_func_t yuv_444_p_to_yuvj_444_p;
  gavl_video_func_t yuv_444_p_to_yuy2;
  gavl_video_func_t yuv_444_p_to_yuva_32;
  gavl_video_func_t yuv_444_p_to_uyvy;

  gavl_video_func_t yuv_444_p_16_to_yuv_420_p;
  gavl_video_func_t yuv_444_p_16_to_yuv_410_p;
  gavl_video_func_t yuv_444_p_16_to_yuv_422_p;
  gavl_video_func_t yuv_444_p_16_to_yuv_422_p_16;
  gavl_video_func_t yuv_444_p_16_to_yuv_444_p;
  gavl_video_func_t yuv_444_p_16_to_yuv_411_p;
  gavl_video_func_t yuv_444_p_16_to_yuvj_420_p;
  gavl_video_func_t yuv_444_p_16_to_yuvj_422_p;
  gavl_video_func_t yuv_444_p_16_to_yuvj_444_p;
  gavl_video_func_t yuv_444_p_16_to_yuy2;
  gavl_video_func_t yuv_444_p_16_to_yuva_32;
  gavl_video_func_t yuv_444_p_16_to_uyvy;

  
  gavl_video_func_t yuvj_420_p_to_yuv_420_p;
  gavl_video_func_t yuvj_420_p_to_yuv_410_p;
  gavl_video_func_t yuvj_420_p_to_yuv_422_p;
  gavl_video_func_t yuvj_420_p_to_yuv_422_p_16;
  gavl_video_func_t yuvj_420_p_to_yuv_411_p;
  gavl_video_func_t yuvj_420_p_to_yuv_444_p;
  gavl_video_func_t yuvj_420_p_to_yuv_444_p_16;
  gavl_video_func_t yuvj_420_p_to_yuy2;
  gavl_video_func_t yuvj_420_p_to_yuva_32;
  gavl_video_func_t yuvj_420_p_to_uyvy;

  gavl_video_func_t yuvj_422_p_to_yuv_420_p;
  gavl_video_func_t yuvj_422_p_to_yuv_410_p;
  gavl_video_func_t yuvj_422_p_to_yuv_422_p;
  gavl_video_func_t yuvj_422_p_to_yuv_422_p_16;
  gavl_video_func_t yuvj_422_p_to_yuv_411_p;
  gavl_video_func_t yuvj_422_p_to_yuv_444_p;
  gavl_video_func_t yuvj_422_p_to_yuv_444_p_16;
  gavl_video_func_t yuvj_422_p_to_yuy2;
  gavl_video_func_t yuvj_422_p_to_yuva_32;
  gavl_video_func_t yuvj_422_p_to_uyvy;

  gavl_video_func_t yuvj_444_p_to_yuv_420_p;
  gavl_video_func_t yuvj_444_p_to_yuv_410_p;
  gavl_video_func_t yuvj_444_p_to_yuv_422_p;
  gavl_video_func_t yuvj_444_p_to_yuv_422_p_16;
  gavl_video_func_t yuvj_444_p_to_yuv_411_p;
  gavl_video_func_t yuvj_444_p_to_yuv_444_p;
  gavl_video_func_t yuvj_444_p_to_yuv_444_p_16;
  gavl_video_func_t yuvj_444_p_to_yuy2;
  gavl_video_func_t yuvj_444_p_to_yuva_32;
  gavl_video_func_t yuvj_444_p_to_uyvy;

  gavl_video_func_t yuva_32_to_yuy2;
  gavl_video_func_t yuva_32_to_uyvy;
  gavl_video_func_t yuva_32_to_yuva_32;
  gavl_video_func_t yuva_32_to_yuv_420_p;
  gavl_video_func_t yuva_32_to_yuv_410_p;
  gavl_video_func_t yuva_32_to_yuv_422_p;
  gavl_video_func_t yuva_32_to_yuv_422_p_16;
  gavl_video_func_t yuva_32_to_yuv_411_p;
  gavl_video_func_t yuva_32_to_yuv_444_p;
  gavl_video_func_t yuva_32_to_yuv_444_p_16;
  gavl_video_func_t yuva_32_to_yuvj_420_p;
  gavl_video_func_t yuva_32_to_yuvj_422_p;
  gavl_video_func_t yuva_32_to_yuvj_444_p;

  
  } gavl_pixelformat_function_table_t;

void gavl_init_rgb_rgb_funcs_c(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);
void gavl_init_rgb_yuv_funcs_c(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);
void gavl_init_yuv_yuv_funcs_c(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);
void gavl_init_yuv_rgb_funcs_c(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);

void gavl_init_rgb_rgb_funcs_hq(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);
void gavl_init_rgb_yuv_funcs_hq(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);
void gavl_init_yuv_yuv_funcs_hq(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);
void gavl_init_yuv_rgb_funcs_hq(gavl_pixelformat_function_table_t *, const gavl_video_options_t * opt);


#ifdef ARCH_X86
void gavl_init_rgb_rgb_funcs_mmx(gavl_pixelformat_function_table_t *,
                                 int width, const gavl_video_options_t * opt);

void
gavl_init_rgb_yuv_funcs_mmx(gavl_pixelformat_function_table_t *,
                            int width, const gavl_video_options_t * opt);


void
gavl_init_yuv_yuv_funcs_mmx(gavl_pixelformat_function_table_t *,
                            int width, const gavl_video_options_t * opt);


void
gavl_init_yuv_rgb_funcs_mmx(gavl_pixelformat_function_table_t *,
                            int width, const gavl_video_options_t * opt);

void
gavl_init_rgb_rgb_funcs_mmxext(gavl_pixelformat_function_table_t *,
                               int width, const gavl_video_options_t * opt);


void gavl_init_rgb_yuv_funcs_mmxext(gavl_pixelformat_function_table_t *,
                                    int width, const gavl_video_options_t * opt);


void gavl_init_yuv_yuv_funcs_mmxext(gavl_pixelformat_function_table_t *,
                                    int width, const gavl_video_options_t * opt);

void
gavl_init_yuv_rgb_funcs_mmxext(gavl_pixelformat_function_table_t *,
                               int width, const gavl_video_options_t * opt);

#endif

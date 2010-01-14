/*****************************************************************
 * gavl - a general purpose audio/video processing library
 *
 * Copyright (c) 2001 - 2010 Members of the Gmerlin project
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
#include <gavl/gavl.h>
#include <video.h>
#include <scale.h>

#include "scale_macros.h"

#define TMP_TYPE_8 int64_t
#define TMP_TYPE_16 int64_t

/* x-Direction */

#define FUNC_NAME scale_rgb_15_x_bilinear_c
#define TYPE color_15
#define INIT TMP_TYPE_8 tmp;
#define SCALE                                                \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1->r +   \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2->r);   \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                                        \
  dst->r = tmp;                                                    \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1->g +   \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2->g);   \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                                        \
  dst->g = tmp; \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1->b +   \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2->b);   \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,2);                                                        \
  dst->b = tmp;                                                    \

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_rgb_16_x_bilinear_c
#define TYPE color_16
#define INIT TMP_TYPE_8 tmp;
#define SCALE \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1->r +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2->r);       \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                                     \
  dst->r = tmp;                                               \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1->g +      \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2->g);       \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                                     \
  dst->g = tmp;                                               \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1->b + \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2->b);\
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,2);                                                     \
  dst->b = tmp;                                               \

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_uint8_x_1_x_bilinear_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;
#define SCALE \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[0] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[0]);            \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,ctx->plane);                                                \
  dst[0] = tmp;

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_uint8_x_2_x_bilinear_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;
#define SCALE \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[0] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[0]);       \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                                         \
  dst[0] = tmp;                                                    \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[1] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[1]);       \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                                          \
  dst[1] = tmp;

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_uint8_x_3_x_bilinear_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;
#define SCALE \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[0] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[0]);       \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                                         \
  dst[0] = tmp;                                                    \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[1] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[1]);       \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                                          \
  dst[1] = tmp;                                                    \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[2] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[2]);       \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,2);                                                          \
  dst[2] = tmp;

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_uint8_x_4_x_bilinear_c
#define TYPE uint8_t
#define INIT TMP_TYPE_8 tmp;
#define SCALE \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[0] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[0]);            \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                                         \
  dst[0] = tmp;                                                    \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[1] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[1]);            \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                                          \
  dst[1] = tmp;                                                    \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[2] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[2]);            \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,2);                                                          \
  dst[2] = tmp;                                                    \
  tmp = ((TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[0] * src_1[3] +            \
         (TMP_TYPE_8)ctx->table_h.pixels[i].factor_i[1] * src_2[3]);            \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,3);                                                          \
  dst[3] = tmp;
 
#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_uint16_x_1_x_bilinear_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;
#define SCALE                                                \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[0] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[0]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,ctx->plane);                                             \
  dst[0] = tmp;

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_uint16_x_2_x_bilinear_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;
#define SCALE                                                           \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[0] +   \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[0]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                                       \
  dst[0] = tmp; \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[1] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[1]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                               \
  dst[1] = tmp;

#define NUM_TAPS 3
#include "scale_x.h"


#define FUNC_NAME scale_uint16_x_3_x_bilinear_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;
#define SCALE                                                           \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[0] +   \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[0]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                                       \
  dst[0] = tmp; \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[1] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[1]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                               \
  dst[1] = tmp; \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[2] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[2]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,2);                                               \
  dst[2] = tmp;

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_uint16_x_4_x_bilinear_c
#define TYPE uint16_t
#define INIT TMP_TYPE_16 tmp;
#define SCALE                                                           \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[0] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[0]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,0);                                               \
  dst[0] = tmp; \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[1] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[1]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,1);                                               \
  dst[1] = tmp; \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[2] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[2]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,2);                                               \
  dst[2] = tmp; \
  tmp = ((TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[0] * src_1[3] + \
         (TMP_TYPE_16)ctx->table_h.pixels[i].factor_i[1] * src_2[3]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_H(tmp,3);                                                       \
  dst[3] = tmp;

#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_float_x_1_x_bilinear_c
#define TYPE float
#define SCALE                                                   \
  dst[0] = (ctx->table_h.pixels[i].factor_f[0] * src_1[0] + \
            ctx->table_h.pixels[i].factor_f[1] * src_2[0]);         \
  RECLIP_FLOAT(dst[0], 0);

#define NUM_TAPS 3
#include "scale_x.h"


#define FUNC_NAME scale_float_x_2_x_bilinear_c
#define TYPE float
#define SCALE                                                   \
  dst[0] = (ctx->table_h.pixels[i].factor_f[0] * src_1[0] + \
            ctx->table_h.pixels[i].factor_f[1] * src_2[0]);         \
  RECLIP_FLOAT(dst[0], 0);                                                 \
  dst[1] = (ctx->table_h.pixels[i].factor_f[0] * src_1[1] + \
            ctx->table_h.pixels[i].factor_f[1] * src_2[1]);         \
  RECLIP_FLOAT(dst[1], 1);

#define NUM_TAPS 3
#include "scale_x.h"



#define FUNC_NAME scale_float_x_3_x_bilinear_c
#define TYPE float
#define SCALE                                                   \
  dst[0] = (ctx->table_h.pixels[i].factor_f[0] * src_1[0] + \
            ctx->table_h.pixels[i].factor_f[1] * src_2[0]);         \
  RECLIP_FLOAT(dst[0], 0);                                                 \
  dst[1] = (ctx->table_h.pixels[i].factor_f[0] * src_1[1] + \
            ctx->table_h.pixels[i].factor_f[1] * src_2[1]);         \
  RECLIP_FLOAT(dst[1], 1);                                                 \
  dst[2] = (ctx->table_h.pixels[i].factor_f[0] * src_1[2] + \
            ctx->table_h.pixels[i].factor_f[1] * src_2[2]);\
  RECLIP_FLOAT(dst[2], 2);                                        \


#define NUM_TAPS 3
#include "scale_x.h"

#define FUNC_NAME scale_float_x_4_x_bilinear_c
#define TYPE float
#define SCALE                                                           \
  dst[0] = (ctx->table_h.pixels[i].factor_f[0] * src_1[0] +         \
            ctx->table_h.pixels[i].factor_f[1] * src_2[0]);         \
  RECLIP_FLOAT(dst[0], 0);                                                 \
  dst[1] = (ctx->table_h.pixels[i].factor_f[0] * src_1[1] +         \
            ctx->table_h.pixels[i].factor_f[1] * src_2[1]);         \
  RECLIP_FLOAT(dst[1], 1);                                                 \
  dst[2] = (ctx->table_h.pixels[i].factor_f[0] * src_1[2] +         \
            ctx->table_h.pixels[i].factor_f[1] * src_2[2]);         \
  RECLIP_FLOAT(dst[2], 2);                                                 \
  dst[3] = (ctx->table_h.pixels[i].factor_f[0] * src_1[3] +         \
            ctx->table_h.pixels[i].factor_f[1] * src_2[3]);\
  RECLIP_FLOAT(dst[3], 3);                                        \

#define NUM_TAPS 3
#include "scale_x.h"

/* y-Direction */

#define FUNC_NAME scale_rgb_15_y_bilinear_c
#define TYPE color_15
#define INIT int64_t fac_1, fac_2;        \
  int64_t tmp; \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define NO_UINT8

#define SCALE                                                           \
  tmp = (fac_1 * src_1->r + \
         fac_2 * src_2->r);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 0);                                       \
  dst->r = tmp;                                    \
  tmp = (fac_1 * src_1->g +                             \
         fac_2 * src_2->g);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 1);                                       \
  dst->g = tmp;                                    \
  tmp = (fac_1 * src_1->b +                             \
         fac_2 * src_2->b);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 2);                                       \
  dst->b = tmp;

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_rgb_16_y_bilinear_c
#define TYPE color_16
#define INIT int64_t fac_1, fac_2;\
  int64_t tmp;                                                  \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];


#define NO_UINT8

#define SCALE                                           \
  tmp = (fac_1 * src_1->r +                             \
         fac_2 * src_2->r);                             \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 0);                                       \
  dst->r = tmp;                                     \
  tmp = (fac_1 * src_1->g +                             \
         fac_2 * src_2->g);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 1);                                       \
  dst->g = tmp;                                     \
  tmp = (fac_1 * src_1->b +                             \
         fac_2 * src_2->b);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 2);                                       \
  dst->b = tmp;                                     \

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_uint8_x_1_y_bilinear_c
#define TYPE uint8_t
#define INIT int64_t fac_1, fac_2, tmp;               \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define SCALE                  \
  tmp = (fac_1 * src_1[0] + \
         fac_2 * src_2[0]);                     \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                      \
  dst[0] = tmp;

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_uint8_x_2_y_bilinear_c
#define TYPE uint8_t
#define INIT int64_t fac_1, fac_2, tmp;               \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define SCALE                  \
  tmp = (fac_1 * src_1[0] + \
         fac_2 * src_2[0]);                     \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                      \
  dst[0] = tmp;\
  tmp = (fac_1 * src_1[1] + \
         fac_2 * src_2[1]);                     \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                      \
  dst[1] = tmp;

#define NUM_TAPS 3
#include "scale_y.h"


#define FUNC_NAME scale_uint8_x_3_y_bilinear_c
#define TYPE uint8_t
#define INIT int64_t fac_1, fac_2, tmp;               \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define SCALE               \
  tmp = (fac_1 * src_1[0] +    \
         fac_2 * src_2[0]);                 \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 0);                                \
  dst[0] = tmp;                             \
  tmp = (fac_1 * src_1[1] +                             \
         fac_2 * src_2[1]);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                              \
  dst[1] = tmp;                                    \
  tmp = (fac_1 * src_1[2] +                             \
         fac_2 * src_2[2]);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                              \
  dst[2] = tmp;

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_uint8_x_4_y_bilinear_c
#define TYPE uint8_t
#define INIT int64_t fac_1, fac_2, tmp;               \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define SCALE               \
  tmp = (fac_1 * src_1[0] +    \
         fac_2 * src_2[0]);                 \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 0);                                \
  dst[0] = tmp;                             \
  tmp = (fac_1 * src_1[1] +                             \
         fac_2 * src_2[1]);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 1);                              \
  dst[1] = tmp;                                    \
  tmp = (fac_1 * src_1[2] +                             \
         fac_2 * src_2[2]);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 2);                                       \
  dst[2] = tmp;                                    \
  tmp = (fac_1 * src_1[3] +                             \
         fac_2 * src_2[3]);                        \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 3);                              \
  dst[3] = tmp;



#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_uint16_x_1_y_bilinear_c
#define TYPE uint16_t
#define INIT int64_t tmp; \
  int64_t fac_1, fac_2;                           \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];   \
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define NO_UINT8

#define SCALE                                  \
  tmp = (fac_1 * src_1[0] + \
         fac_2 * src_2[0]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                    \
  dst[0] = tmp;

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_uint16_x_2_y_bilinear_c
#define TYPE uint16_t
#define INIT int64_t tmp; \
  int64_t fac_1, fac_2;                           \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];   \
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define NO_UINT8

#define SCALE                                  \
  tmp = (fac_1 * src_1[0] + \
         fac_2 * src_2[0]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                    \
  dst[0] = tmp;\
  tmp = (fac_1 * src_1[1] + \
         fac_2 * src_2[1]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, ctx->plane);                    \
  dst[1] = tmp;

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_uint16_x_3_y_bilinear_c
#define TYPE uint16_t
#define INIT int64_t tmp;                      \
  int64_t fac_1, fac_2;                           \
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define NO_UINT8

#define SCALE                                  \
  tmp = (fac_1 * src_1[0] +                    \
         fac_2 * src_2[0]);                    \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 0);                              \
  dst[0] = tmp;                          \
  tmp = (fac_1 * src_1[1] +                    \
         fac_2 * src_2[1]);                    \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 1);                              \
  dst[1] = tmp;                          \
  tmp = (fac_1 * src_1[2] +                    \
         fac_2 * src_2[2]);                    \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 2);                              \
  dst[2] = tmp;

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_uint16_x_4_y_bilinear_c
#define TYPE uint16_t
#define INIT int64_t tmp;\
  int64_t fac_1, fac_2;\
  fac_1 = ctx->table_v.pixels[scanline].factor_i[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_i[1];

#define NO_UINT8

#define SCALE                                  \
  tmp = (fac_1 * src_1[0] + \
         fac_2 * src_2[0]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 0);                              \
  dst[0] = tmp; \
  tmp = (fac_1 * src_1[1] + \
         fac_2 * src_2[1]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 1);                              \
  dst[1] = tmp; \
  tmp = (fac_1 * src_1[2] + \
         fac_2 * src_2[2]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 2);                              \
  dst[2] = tmp; \
  tmp = (fac_1 * src_1[3] + \
         fac_2 * src_2[3]); \
  tmp=DOWNSHIFT(tmp,16);\
  RECLIP_V(tmp, 3);                              \
  dst[3] = tmp;

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_float_x_1_y_bilinear_c
#define TYPE float
#define INIT float fac_1, fac_2;\
  fac_1 = ctx->table_v.pixels[scanline].factor_f[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_f[1];

#define NO_UINT8
  
#define SCALE                  \
  dst[0] = (fac_1 * src_1[0] + \
            fac_2 * src_2[0]); \
  RECLIP_FLOAT(dst[0], 0);

#define NUM_TAPS 3
#include "scale_y.h"


#define FUNC_NAME scale_float_x_2_y_bilinear_c
#define TYPE float
#define INIT float fac_1, fac_2;\
  fac_1 = ctx->table_v.pixels[scanline].factor_f[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_f[1];

#define NO_UINT8
  
#define SCALE                  \
  dst[0] = (fac_1 * src_1[0] + \
            fac_2 * src_2[0]); \
  RECLIP_FLOAT(dst[0], 0);        \
  dst[1] = (fac_1 * src_1[1] + \
            fac_2 * src_2[1]); \
  RECLIP_FLOAT(dst[1], 1);

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_float_x_3_y_bilinear_c
#define TYPE float
#define INIT float fac_1, fac_2;\
  fac_1 = ctx->table_v.pixels[scanline].factor_f[0];\
  fac_2 = ctx->table_v.pixels[scanline].factor_f[1];

#define NO_UINT8
  
#define SCALE                  \
  dst[0] = (fac_1 * src_1[0] + \
            fac_2 * src_2[0]); \
  RECLIP_FLOAT(dst[0], 0);        \
  dst[1] = (fac_1 * src_1[1] + \
            fac_2 * src_2[1]); \
  RECLIP_FLOAT(dst[1], 1);        \
  dst[2] = (fac_1 * src_1[2] + \
            fac_2 * src_2[2]);\
  RECLIP_FLOAT(dst[2], 2);

#define NUM_TAPS 3
#include "scale_y.h"

#define FUNC_NAME scale_float_x_4_y_bilinear_c
#define TYPE float
#define INIT float fac_1, fac_2;                  \
  fac_1 = ctx->table_v.pixels[scanline].factor_f[0];   \
  fac_2 = ctx->table_v.pixels[scanline].factor_f[1];

#define NO_UINT8

#define SCALE                  \
  dst[0] = (fac_1 * src_1[0] + \
            fac_2 * src_2[0]); \
  RECLIP_FLOAT(dst[0], 0);        \
  dst[1] = (fac_1 * src_1[1] + \
            fac_2 * src_2[1]); \
  RECLIP_FLOAT(dst[1], 1);                           \
  dst[2] = (fac_1 * src_1[2] +                    \
            fac_2 * src_2[2]);                    \
  RECLIP_FLOAT(dst[2], 2);                           \
  dst[3] = (fac_1 * src_1[3] +                    \
            fac_2 * src_2[3]);                    \
  RECLIP_FLOAT(dst[3], 3);

#define NUM_TAPS 3
#include "scale_y.h"

#ifdef NOCLIP
void gavl_init_scale_funcs_bilinear_noclip_c(gavl_scale_funcs_t * tab)
#else
void gavl_init_scale_funcs_bilinear_c(gavl_scale_funcs_t * tab)
#endif
  {
  //  fprintf(stderr, "gavl_init_scale_funcs_bilinear_c\n");
  tab->funcs_x.scale_rgb_15 =     scale_rgb_15_x_bilinear_c;
  tab->funcs_x.scale_rgb_16 =     scale_rgb_16_x_bilinear_c;
  tab->funcs_x.scale_uint8_x_1_advance =  scale_uint8_x_1_x_bilinear_c;
  tab->funcs_x.scale_uint8_x_1_noadvance =  scale_uint8_x_1_x_bilinear_c;
  tab->funcs_x.scale_uint8_x_2 =  scale_uint8_x_2_x_bilinear_c;
  tab->funcs_x.scale_uint8_x_3 =  scale_uint8_x_3_x_bilinear_c;
  tab->funcs_x.scale_uint8_x_4 =  scale_uint8_x_4_x_bilinear_c;
  tab->funcs_x.scale_uint16_x_1 = scale_uint16_x_1_x_bilinear_c;
  tab->funcs_x.scale_uint16_x_2 = scale_uint16_x_2_x_bilinear_c;
  tab->funcs_x.scale_uint16_x_3 = scale_uint16_x_3_x_bilinear_c;
  tab->funcs_x.scale_uint16_x_4 = scale_uint16_x_4_x_bilinear_c;
  tab->funcs_x.scale_float_x_1 =  scale_float_x_1_x_bilinear_c;
  tab->funcs_x.scale_float_x_2 =  scale_float_x_2_x_bilinear_c;
  tab->funcs_x.scale_float_x_3 =  scale_float_x_3_x_bilinear_c;
  tab->funcs_x.scale_float_x_4 =  scale_float_x_4_x_bilinear_c;
  tab->funcs_x.bits_rgb_15 = 16;
  tab->funcs_x.bits_rgb_16 = 16;
  tab->funcs_x.bits_uint8_advance  = 16;
  tab->funcs_x.bits_uint8_noadvance  = 16;
  tab->funcs_x.bits_uint16 = 16;

  tab->funcs_y.scale_rgb_15 =     scale_rgb_15_y_bilinear_c;
  tab->funcs_y.scale_rgb_16 =     scale_rgb_16_y_bilinear_c;
  tab->funcs_y.scale_uint8_x_1_advance =  scale_uint8_x_1_y_bilinear_c;
  tab->funcs_y.scale_uint8_x_1_noadvance =  scale_uint8_x_1_y_bilinear_c;
  tab->funcs_y.scale_uint8_x_2 =  scale_uint8_x_2_y_bilinear_c;
  tab->funcs_y.scale_uint8_x_3 =  scale_uint8_x_3_y_bilinear_c;
  tab->funcs_y.scale_uint8_x_4 =  scale_uint8_x_4_y_bilinear_c;
  tab->funcs_y.scale_uint16_x_1 = scale_uint16_x_1_y_bilinear_c;
  tab->funcs_y.scale_uint16_x_2 = scale_uint16_x_2_y_bilinear_c;
  tab->funcs_y.scale_uint16_x_3 = scale_uint16_x_3_y_bilinear_c;
  tab->funcs_y.scale_uint16_x_4 = scale_uint16_x_4_y_bilinear_c;
  tab->funcs_y.scale_float_x_1 =  scale_float_x_1_y_bilinear_c;
  tab->funcs_y.scale_float_x_2 =  scale_float_x_2_y_bilinear_c;
  tab->funcs_y.scale_float_x_3 =  scale_float_x_3_y_bilinear_c;
  tab->funcs_y.scale_float_x_4 =  scale_float_x_4_y_bilinear_c;
  tab->funcs_y.bits_rgb_15 = 16;
  tab->funcs_y.bits_rgb_16 = 16;
  tab->funcs_y.bits_uint8_advance  = 16;
  tab->funcs_y.bits_uint8_noadvance  = 16;
  tab->funcs_y.bits_uint16 = 16;
  
  }

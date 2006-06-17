/*****************************************************************

  scale_context.c

  Copyright (c) 2005 by Burkhard Plaum - plaum@ipf.uni-stuttgart.de

  http://gmerlin.sourceforge.net

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gavl/gavl.h>
#include <scale.h>

#define SCALE_X        0
#define SCALE_Y        1
#define SCALE_XY       2
#define SCALE_X_THEN_Y 3
#define SCALE_Y_THEN_X 4

#define ALIGNMENT_BYTES 8
#define ALIGN(a) a=((a+ALIGNMENT_BYTES-1)/ALIGNMENT_BYTES)*ALIGNMENT_BYTES

#define EPS 1e-4 /* Needed to determine if a double and an int are equal. */

static void copy_scanline_noadvance(gavl_video_scale_context_t * ctx)
  {
  //  fprintf(stderr, "copy_scanline_noadvance\n");
  memcpy(ctx->dst, ctx->src, ctx->bytes_per_line);
  ctx->src += ctx->src_stride;
  }

static void copy_scanline_advance(gavl_video_scale_context_t * ctx)
  {
  int i;
  uint8_t * src = ctx->src;
  //  fprintf(stderr, "copy_scanline_advance\n");
  for(i = 0; i < ctx->dst_rect.w; i++)
    {
    *ctx->dst = *src;

    ctx->dst += ctx->offset->dst_advance;
    src += ctx->offset->src_advance;
    }
  ctx->src += ctx->src_stride;
  }

/* Debugging utility */
#if 0
static void dump_offset(gavl_video_scale_offsets_t*off)
  {
  fprintf(stderr, "Scale offset:\n");
  fprintf(stderr, "  src_advance: %d\n", off->src_advance);
  fprintf(stderr, "  src_offset:  %d\n", off->src_offset);
  fprintf(stderr, "  dst_advance: %d\n", off->dst_advance);
  fprintf(stderr, "  dst_offset:  %d\n", off->dst_offset);
  }
#endif

static gavl_video_scale_scanline_func get_func(gavl_scale_func_tab_t * tab,
                                        gavl_pixelformat_t pixelformat, int * bits)
  {
  switch(pixelformat)
    {
    case GAVL_PIXELFORMAT_NONE:
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      *bits = tab->bits_rgb_15;
      return tab->scale_rgb_15;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      *bits = tab->bits_rgb_16;
      return tab->scale_rgb_16;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      *bits = tab->bits_uint8;
      return tab->scale_uint8_x_3;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      *bits = tab->bits_uint8;
      return tab->scale_uint8_x_3;
      break;
    case GAVL_YUVA_32:
    case GAVL_RGBA_32:
      *bits = tab->bits_uint8;
      return tab->scale_uint8_x_4;
      break;
    case GAVL_YUY2:
      *bits = tab->bits_uint8;
      return tab->scale_uint8_x_1;
      break;
    case GAVL_UYVY:
      *bits = tab->bits_uint8;
      return tab->scale_uint8_x_1;
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
      *bits = tab->bits_uint8;
      return tab->scale_uint8_x_1;
      break;
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      *bits = tab->bits_uint8;
      return tab->scale_uint8_x_1;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_1;
      break;
    case GAVL_RGB_48:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_3;
      break;
    case GAVL_RGBA_64:
      *bits = tab->bits_uint16;
      return tab->scale_uint16_x_4;
      break;
    case GAVL_RGB_FLOAT:
      *bits = 0;
      return tab->scale_float_x_3;
      break;
    case GAVL_RGBA_FLOAT:
      *bits = 0;
      return tab->scale_float_x_4;
      break;
    }
  return (gavl_video_scale_scanline_func)0;
  }

static void get_offsets(gavl_pixelformat_t pixelformat,
                        int plane,
                        int * advance, int * offset)
  {
  switch(pixelformat)
    {
    case GAVL_PIXELFORMAT_NONE:
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      *advance = 2;
      *offset = 0;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      *advance = 2;
      *offset = 0;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      *advance = 3;
      *offset = 0;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      *advance = 4;
      *offset = 0;
      break;
    case GAVL_YUVA_32:
    case GAVL_RGBA_32:
      *advance = 4;
      *offset = 0;
      break;
    case GAVL_YUY2:
      switch(plane)
        {
        /* YUYV */
        case 0:
          *advance = 2;
          *offset = 0;
          break;
        case 1:
          *advance = 4;
          *offset = 1;
          break;
        case 2:
          *advance = 4;
          *offset = 3;
          break;
        }
      break;
    case GAVL_UYVY:
      switch(plane)
        {
        /* UYVY */
        case 0:
          *advance = 2;
          *offset = 1;
          break;
        case 1:
          *advance = 4;
          *offset = 0;
          break;
        case 2:
          *advance = 4;
          *offset = 2;
          break;
        }
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
      *advance = 1;
      *offset = 0;
      break;
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
      *advance = 1;
      *offset = 0;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      *advance = 2;
      *offset = 0;
      break;
    case GAVL_RGB_48:
      *advance = 6;
      *offset = 0;
      break;
    case GAVL_RGBA_64:
      *advance = 8;
      *offset = 0;
      break;
    case GAVL_RGB_FLOAT:
      *advance = 3 * sizeof(float);
      *offset = 0;
      break;
    case GAVL_RGBA_FLOAT:
      *advance = 4 * sizeof(float);
      *offset = 0;
      break;
    }
  }

static void get_minmax(gavl_pixelformat_t pixelformat,
                       uint32_t * min, uint32_t * max)
  {
  switch(pixelformat)
    {
    case GAVL_PIXELFORMAT_NONE:
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      max[0] = (1<<5)-1;
      max[1] = (1<<5)-1;
      max[2] = (1<<5)-1;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      max[0] = (1<<5)-1;
      max[1] = (1<<6)-1;
      max[2] = (1<<5)-1;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGB_32:
    case GAVL_BGR_32:
    case GAVL_YUVJ_420_P:
    case GAVL_YUVJ_422_P:
    case GAVL_YUVJ_444_P:
    case GAVL_RGBA_32:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      min[3] = 0;
      max[0] = (1<<8)-1;
      max[1] = (1<<8)-1;
      max[2] = (1<<8)-1;
      max[3] = (1<<8)-1;
      break;
    case GAVL_YUVA_32:
      min[0] = 16;
      min[1] = 16;
      min[2] = 16;
      min[3] = 0;
      max[0] = 235;
      max[1] = 240;
      max[2] = 240;
      max[3] = 255;
      break;
    case GAVL_YUY2:
    case GAVL_UYVY:
    case GAVL_YUV_420_P:
    case GAVL_YUV_422_P:
    case GAVL_YUV_444_P:
    case GAVL_YUV_411_P:
    case GAVL_YUV_410_P:
      min[0] = 16;
      min[1] = 16;
      min[2] = 16;
      max[0] = 235;
      max[1] = 240;
      max[2] = 240;
      break;
    case GAVL_YUV_444_P_16:
    case GAVL_YUV_422_P_16:
      min[0] = 16<<8;
      min[1] = 16<<8;
      min[2] = 16<<8;
      max[0] = 235<<8;
      max[1] = 240<<8;
      max[2] = 240<<8;
      break;
    case GAVL_RGB_48:
    case GAVL_RGBA_64:
      min[0] = 0;
      min[1] = 0;
      min[2] = 0;
      min[3] = 0;
      max[0] = (1<<16)-1;
      max[1] = (1<<16)-1;
      max[2] = (1<<16)-1;
      max[3] = (1<<16)-1;
      break;
    case GAVL_RGB_FLOAT:
    case GAVL_RGBA_FLOAT:
      break;
    }
  }

static void alloc_temp(gavl_video_scale_context_t * ctx, gavl_pixelformat_t pixelformat)
  {
  int size;
  if((pixelformat == GAVL_YUY2) || (pixelformat == GAVL_UYVY))
    ctx->buffer_stride = ctx->buffer_width;
  else if(gavl_pixelformat_is_planar(pixelformat))
    ctx->buffer_stride = ctx->buffer_width * gavl_pixelformat_bytes_per_component(pixelformat);
  else
    ctx->buffer_stride = ctx->buffer_width * gavl_pixelformat_bytes_per_pixel(pixelformat);
  
  ALIGN(ctx->buffer_stride);

  size = ctx->buffer_stride * ctx->buffer_height;

  if(ctx->buffer_alloc < size)
    {
    ctx->buffer_alloc = size + 8192;
    ctx->buffer = realloc(ctx->buffer, ctx->buffer_alloc);
    }
  
  }

/*
 * Scale offset is the position of the first dst pixel in source
 * coordinates. First, we calculate the distances from the image border
 * of the first source and dst pixels respectively. dst_dist is scaled to
 * source coordinates and the offset becomes dst_dist - src_dist.
 */

static
float get_scale_offset(int src_field, int dst_field,
                       int src_fields, int dst_fields,
                       float scale_fac, int sub_in, int sub_out,
                       float chroma_offset_src, float chroma_offset_dst)
  {
  float result;
  
  float src_dist; /* Distance of the first source sample from the picture border in destination
                     coordinates */
  
  float dst_dist; /* Distance of the first destination sample from the picture border in
                     destination coordinates */
  
  float chroma_scale_fac = (float)sub_in / (float)sub_out;
#if 0
  fprintf(stderr, "get_scale_offset:\n\
  src_field: %d\n  dst_field: %d\n\
  src_fields: %d\n  dst_fields: %d\n\
  scale_fac: %f\n  sub_in: %d\n\
  sub_out: %d\n  chroma_offset_src: %f\n\
  chroma_offset_dst: %f\n\
  chroma_scale_fac: %f\n",
          src_field, dst_field,
          src_fields, dst_fields,
          scale_fac, sub_in, sub_out,
          chroma_offset_src, chroma_offset_dst,
          chroma_scale_fac);
#endif

  scale_fac *= chroma_scale_fac;
  
  /* Different calculation for diffrent interlacing options */
  
  if(src_fields == 1) /* Progressive input -> Progressive */
    {
    //    fprintf(stderr, "Progressive -> Progressive\n");

    src_dist = (0.5 + chroma_offset_src) / sub_in;
    dst_dist = (0.5 + chroma_offset_dst) / (sub_out * scale_fac);
    }
  else if(dst_fields == 1) /* Interlaced -> Progressive */
    {
    if(src_field == 0) /* Top field -> Progressive */
      {
      //      fprintf(stderr, "Top field -> Progressive\n");

      src_dist = (0.25 + chroma_offset_src) / (sub_in);
      dst_dist = (0.5 + chroma_offset_dst) / (2.0*scale_fac * sub_out);
      }
    else /* Bottom field -> Progressive */
      {
      //      fprintf(stderr, "Bottom field -> Progressive\n");
      
      src_dist = (0.75 + chroma_offset_src) / (sub_in);
      dst_dist = (0.5 + chroma_offset_dst) / (2.0*scale_fac * sub_out);
      }
    }
  else /* Interlaced -> Interlaced */
    {
    if(dst_field == src_field)
      {
      if(!dst_field) /* Top field -> Top field */
        {
        //        fprintf(stderr, "Top field -> Top field\n");

        src_dist = (0.25 + chroma_offset_src) / (sub_in);
        dst_dist = (0.25 + chroma_offset_dst) / (scale_fac * sub_out);
        
        }
      else /* Bottom field -> Bottom field */
        {
        //        fprintf(stderr, "Bottom field -> Bottom field\n");

        src_dist = (0.75 + chroma_offset_src) / (sub_in);
        dst_dist = (0.75 + chroma_offset_dst) / (scale_fac * sub_out);
        }
      }
    else
      {
      fprintf(stderr, "BUG: scaler cannot swap fields\n");
      return 0;
      }
    }

  result = dst_dist - src_dist;
#if 0
  fprintf(stderr, "Src dist: %f, dst_dist: %f\n", src_dist, dst_dist);
  fprintf(stderr, "result: %f\n", result);
#endif
  return result;
  }

int gavl_video_scale_context_init(gavl_video_scale_context_t*ctx,
                                  gavl_video_options_t * opt, int plane,
                                  const gavl_video_format_t * src_format,
                                  const gavl_video_format_t * dst_format,
                                  int src_field, int dst_field,
                                  int src_fields, int dst_fields)
  {
  int bits, i;
  int sub_h_in = 1, sub_v_in = 1, sub_h_out = 1, sub_v_out = 1;
  int scale_x, scale_y;
  gavl_video_options_t tmp_opt, tmp_opt_y;
  double scale_factor_x, scale_factor_y;
  float src_chroma_offset_x, src_chroma_offset_y, dst_chroma_offset_x, dst_chroma_offset_y;
  gavl_rectangle_i_t src_rect_i;

  
  int src_width, src_height; /* Needed for generating the scale table */
  float offset_x, offset_y;
  
  gavl_scale_funcs_t funcs;
  
#if 0  
  fprintf(stderr, "scale_context_init: src_field: %d, dst_field: %d plane: %d\n",
          src_field, dst_field, plane);
#endif  
  gavl_rectangle_f_copy(&(ctx->src_rect), &(opt->src_rect));
  gavl_rectangle_i_copy(&(ctx->dst_rect), &(opt->dst_rect));
  
  scale_factor_x = (float)ctx->dst_rect.w / ctx->src_rect.w;
  scale_factor_y = (float)ctx->dst_rect.h / ctx->src_rect.h;
    
  ctx->plane = plane;

  
  
  if(plane)
    {
    /* Get chroma subsampling factors for source and destination */
    gavl_pixelformat_chroma_sub(src_format->pixelformat, &sub_h_in, &sub_v_in);
    gavl_pixelformat_chroma_sub(dst_format->pixelformat, &sub_h_out, &sub_v_out);
    
    ctx->src_rect.w /= sub_h_in;
    ctx->src_rect.x /= sub_h_in;

    ctx->src_rect.h /= sub_v_in;
    ctx->src_rect.y /= sub_v_in;
    
    ctx->dst_rect.w /= sub_h_out;
    ctx->dst_rect.x /= sub_h_out;

    ctx->dst_rect.h /= sub_v_out;
    ctx->dst_rect.y /= sub_v_out;

    src_width = src_format->image_width / sub_h_in;
    src_height = src_format->image_height / sub_v_in;

    }
  else
    {
    src_width = src_format->image_width;
    src_height = src_format->image_height;
    }
  
  if(src_fields == 2)
    {
    ctx->src_rect.h /= 2.0;
    ctx->src_rect.y /= 2.0;
    src_height /= 2;
    }
  if(dst_fields == 2)
    {
    ctx->dst_rect.h /= 2;
    ctx->dst_rect.y /= 2;
    }
  
#if 0
  fprintf(stderr, "gavl_video_scale_context_init\n");
  gavl_rectangle_f_dump(&(ctx->src_rect));
  fprintf(stderr, "\n");
  gavl_rectangle_i_dump(&(ctx->dst_rect));
  fprintf(stderr, "\n");
#endif
  

  /* Calculate chroma offsets  */
    
  gavl_video_format_get_chroma_offset(src_format, src_field, plane,
                                      &src_chroma_offset_x,
                                      &src_chroma_offset_y);
  
  gavl_video_format_get_chroma_offset(dst_format, dst_field, plane,
                                      &dst_chroma_offset_x,
                                      &dst_chroma_offset_y);
  
  offset_x = get_scale_offset(0, 0, 1, 1,
                              scale_factor_x, sub_h_in, sub_h_out,
                              src_chroma_offset_x, dst_chroma_offset_x);
  offset_y = get_scale_offset(src_field, dst_field, src_fields, dst_fields,
                              scale_factor_y, sub_v_in, sub_v_out,
                              src_chroma_offset_y, dst_chroma_offset_y);
  
  if((fabs(ctx->src_rect.w - ctx->dst_rect.w) > EPS) ||
     (fabs(ctx->src_rect.x) > EPS) || (fabs(offset_x) > EPS))
    scale_x = 1;
  else
    scale_x = 0;
  
  if((fabs(ctx->src_rect.h - ctx->dst_rect.h) > EPS) ||
     (fabs(ctx->src_rect.y) > EPS) || (fabs(offset_y) > EPS))
    scale_y = 1;
  else
    scale_y = 0;
#if 0
  fprintf(stderr, "Offsets: %f %f, scale_factors: %f %f\n",
          offset_x, offset_y, ctx->dst_rect.w / ctx->src_rect.w,
          ctx->dst_rect.h / ctx->src_rect.h);
#endif  
  //  ctx->src_rect.x += offset_x;
  //  ctx->src_rect.y += offset_y;

  ctx->func1 = NULL;
  ctx->func2 = NULL;
  
  ctx->num_directions = 0;
  if(scale_x)
    ctx->num_directions++;
  if(scale_y)
    ctx->num_directions++;

  /* Set source and destination frame planes */

  if(gavl_pixelformat_is_planar(src_format->pixelformat))
    ctx->src_frame_plane = plane;
  else
    ctx->src_frame_plane = 0;

  if(gavl_pixelformat_is_planar(dst_format->pixelformat))
    ctx->dst_frame_plane = plane;
  else
    ctx->dst_frame_plane = 0;
  
  if(!ctx->num_directions)
    {
    ctx->bytes_per_line = gavl_pixelformat_is_planar(src_format->pixelformat) ?
      ctx->dst_rect.w * gavl_pixelformat_bytes_per_component(src_format->pixelformat) :
      ctx->dst_rect.w * gavl_pixelformat_bytes_per_pixel(src_format->pixelformat);

    if((src_format->pixelformat == GAVL_YUY2) ||
       (src_format->pixelformat == GAVL_UYVY) ||
       (dst_format->pixelformat == GAVL_YUY2) ||
       (dst_format->pixelformat == GAVL_UYVY))
      ctx->func1 = copy_scanline_advance;
    else
      ctx->func1 = copy_scanline_noadvance;

    /* Set source and destination offsets */
    get_offsets(src_format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);
    get_offsets(dst_format->pixelformat,
                plane, &ctx->offset1.dst_advance, &ctx->offset1.dst_offset);

    /* We set this once here */

    ctx->offset = &(ctx->offset1);
    ctx->dst_size = ctx->dst_rect.w;

    ctx->num_directions = 1;
    return 0;
    }

  else if(scale_x && scale_y)
    {
    //    fprintf(stderr, "Initializing x table\n");
    gavl_video_options_copy(&tmp_opt, opt);
    gavl_video_scale_table_init(&(ctx->table_h), &tmp_opt, ctx->src_rect.x + offset_x,
                                ctx->src_rect.w, ctx->dst_rect.w, src_width);
    //    fprintf(stderr, "Initializing x table done\n");

    //    fprintf(stderr, "Initializing y table\n");
    gavl_video_options_copy(&tmp_opt_y, opt);
    gavl_video_scale_table_init(&(ctx->table_v), &tmp_opt_y, ctx->src_rect.y + offset_y,
                                ctx->src_rect.h, ctx->dst_rect.h, src_height);
    
    //    fprintf(stderr, "Initializing y table done\n");
    
    /* Check if we can scale in x and y-directions at once */

    if((tmp_opt.scale_mode == tmp_opt_y.scale_mode) &&
       (tmp_opt.scale_order == tmp_opt_y.scale_order))
      {
      memset(&funcs, 0, sizeof(funcs));
      gavl_init_scale_funcs(&funcs, &tmp_opt);
      ctx->func1 = get_func(&(funcs.funcs_xy), src_format->pixelformat, &bits);
      //      fprintf(stderr, "X AND Y\n");
      }
    
    if(ctx->func1) /* Scaling routines for x-y are there, good */
      {
      ctx->num_directions = 1;
            
      gavl_video_scale_table_init_int(&(ctx->table_h), bits);
      gavl_video_scale_table_init_int(&(ctx->table_v), bits);
      }
    else
      {
      gavl_video_scale_table_get_src_indices(&(ctx->table_h),
                                             &src_rect_i.x, &src_rect_i.w);
      gavl_video_scale_table_get_src_indices(&(ctx->table_v),
                                             &src_rect_i.y, &src_rect_i.h);

      /* Decide the scale order depending on whats computationally less expensive */
      /* We calculate the sizes (in pixels) of the temporary frame for both options and
         take the smaller one */
#if 0
      fprintf(stderr, "Src rect: ");
      gavl_rectangle_i_dump(&src_rect_i);
      fprintf(stderr, "\nDst rect: ");
      gavl_rectangle_i_dump(&ctx->dst_rect);
      fprintf(stderr, "\n");
#endif
      if(src_rect_i.h * ctx->dst_rect.w < ctx->dst_rect.h * src_rect_i.w)
        {
        //        fprintf(stderr, "X then Y\n");
        /* X then Y */

        ctx->buffer_width  = ctx->dst_rect.w;
        ctx->buffer_height = src_rect_i.h;
        
        gavl_video_scale_table_shift_indices(&(ctx->table_v), -src_rect_i.y);
        
        memset(&funcs, 0, sizeof(funcs));
        gavl_init_scale_funcs(&funcs, &tmp_opt);
        ctx->func1 = get_func(&funcs.funcs_x, src_format->pixelformat, &bits);
        gavl_video_scale_table_init_int(&(ctx->table_h), bits);

        memset(&funcs, 0, sizeof(funcs));
        gavl_init_scale_funcs(&funcs, &tmp_opt_y);
        ctx->func2 = get_func(&funcs.funcs_y, src_format->pixelformat, &bits);
        gavl_video_scale_table_init_int(&(ctx->table_v), bits);
        
        }
      else
        {
        //        fprintf(stderr, "Y then X\n");
        /* Y then X */

        ctx->buffer_width = src_rect_i.w;
        ctx->buffer_height = ctx->dst_rect.h;
        
        gavl_video_scale_table_shift_indices(&(ctx->table_h), -src_rect_i.x);

        memset(&funcs, 0, sizeof(funcs));
        gavl_init_scale_funcs(&funcs, &tmp_opt_y);
        ctx->func1 = get_func(&funcs.funcs_y, src_format->pixelformat, &bits);
        gavl_video_scale_table_init_int(&(ctx->table_v), bits);

        memset(&funcs, 0, sizeof(funcs));
        gavl_init_scale_funcs(&funcs, &tmp_opt);
        ctx->func2 = get_func(&(funcs.funcs_x), src_format->pixelformat, &bits);
        gavl_video_scale_table_init_int(&(ctx->table_h), bits);
        }
      
      /* Allocate temporary buffer */
      alloc_temp(ctx, src_format->pixelformat);
      }
    }
  else if(scale_x)
    {
    //    fprintf(stderr, "Initializing x table\n");
    gavl_video_options_copy(&tmp_opt, opt);
    gavl_video_scale_table_init(&(ctx->table_h), &tmp_opt, ctx->src_rect.x + offset_x,
                                ctx->src_rect.w, ctx->dst_rect.w, src_width);
    //    fprintf(stderr, "Initializing x table done\n");

    memset(&funcs, 0, sizeof(funcs));
    gavl_init_scale_funcs(&funcs, &tmp_opt);
    ctx->func1 = get_func(&(funcs.funcs_x), src_format->pixelformat, &bits);
    
    gavl_video_scale_table_init_int(&(ctx->table_h), bits);
    }
  else if(scale_y)
    {
    //    fprintf(stderr, "Initializing y table\n");
    gavl_video_options_copy(&tmp_opt, opt);
    gavl_video_scale_table_init(&(ctx->table_v), &tmp_opt, ctx->src_rect.y + offset_y,
                                ctx->src_rect.h, ctx->dst_rect.h, src_height);
    //    fprintf(stderr, "Initializing y table done\n");
    memset(&funcs, 0, sizeof(funcs));
    gavl_init_scale_funcs(&funcs, &tmp_opt);
    ctx->func1 = get_func(&(funcs.funcs_y), src_format->pixelformat, &bits);
    
    gavl_video_scale_table_init_int(&(ctx->table_v), bits);
    }
  
  /* Set source and destination offsets */

  if(ctx->num_directions == 1)
    {
    get_offsets(src_format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);
    get_offsets(dst_format->pixelformat,
                plane, &ctx->offset1.dst_advance, &ctx->offset1.dst_offset);

    /* We set this once here */

    ctx->offset = &(ctx->offset1);
    ctx->dst_size = ctx->dst_rect.w;
    }
  else if(ctx->num_directions == 2)
    {
    get_offsets(src_format->pixelformat,
                plane, &ctx->offset1.src_advance, &ctx->offset1.src_offset);

    get_offsets(src_format->pixelformat,
                plane, &ctx->offset1.dst_advance, &ctx->offset1.dst_offset);
    ctx->offset1.dst_offset = 0;

    if((src_format->pixelformat == GAVL_YUY2) || 
       (src_format->pixelformat == GAVL_UYVY))
      {
      ctx->offset1.dst_advance = 1;
      }
    ctx->offset2.src_advance = ctx->offset1.dst_advance;
    ctx->offset2.src_offset  = ctx->offset1.dst_offset;
    
    get_offsets(dst_format->pixelformat,
                plane, &ctx->offset2.dst_advance, &ctx->offset2.dst_offset);
    }
  
#if 0  
  /* Dump final scale tables */
  fprintf(stderr, "Horizontal table:\n");
  gavl_video_scale_table_dump(&(ctx->table_h));
  fprintf(stderr, "Vertical table:\n");
  gavl_video_scale_table_dump(&(ctx->table_v));
#endif

  if(scale_x)
    ctx->num_taps = ctx->table_h.factors_per_pixel;
  else
    ctx->num_taps = ctx->table_v.factors_per_pixel;
  
  get_minmax(src_format->pixelformat, ctx->min_values, ctx->max_values);
  for(i = 0; i < 4; i++)
    {
    ctx->min_values[i] <<= bits;
    ctx->max_values[i] <<= bits;
    }

#if 0
  for(i = 0; i < 4; i++)
    {
    fprintf(stderr, "Channel %d: min: %08x, max: %08x\n", i,
            ctx->min_values[i], ctx->max_values[i]);
    }
#endif
  return 1;
  }

void gavl_video_scale_context_cleanup(gavl_video_scale_context_t * ctx)
  {
  gavl_video_scale_table_cleanup(&(ctx->table_h));
  gavl_video_scale_table_cleanup(&(ctx->table_v));
  }


void gavl_video_scale_context_scale(gavl_video_scale_context_t * ctx,
                                    gavl_video_frame_t * src, gavl_video_frame_t * dst)
  {
  uint8_t * dst_save;
  //  fprintf(stderr, "gavl_video_scale_context_scale %p %p %d %d\n", src, dst, ctx->num_directions, ctx->dst_rect.h);
  switch(ctx->num_directions)
    {
    case 0:
      /* Copy field plane */
      break;
    case 1:
      /* Only step */
      ctx->src = src->planes[ctx->src_frame_plane] + ctx->offset->src_offset;
      ctx->src_stride = src->strides[ctx->src_frame_plane];

      dst_save = dst->planes[ctx->dst_frame_plane] + ctx->offset->dst_offset;
            
      for(ctx->scanline = 0; ctx->scanline < ctx->dst_rect.h; ctx->scanline++)
        {
        //        fprintf(stderr, "scanline: %d (%d)\n", ctx->scanline, ctx->dst_rect.h);
        ctx->dst = dst_save;
        ctx->func1(ctx);
        dst_save += dst->strides[ctx->dst_frame_plane];
        }
      break;
    case 2:
      /* First step */
      //      fprintf(stderr, "First direction...\n");
      ctx->offset = &(ctx->offset1);
      //      dump_offset(ctx->offset);
      ctx->src = src->planes[ctx->src_frame_plane] + ctx->offset->src_offset;
      ctx->src_stride = src->strides[ctx->src_frame_plane];
      ctx->dst_size = ctx->buffer_width;

      dst_save = ctx->buffer;
            
      for(ctx->scanline = 0; ctx->scanline < ctx->buffer_height; ctx->scanline++)
        {
        ctx->dst = dst_save;
        ctx->func1(ctx);
        dst_save += ctx->buffer_stride;
        }
      //      fprintf(stderr, "done\n");

      /* Second step */
      //      fprintf(stderr, "Second direction...\n");
      ctx->offset = &(ctx->offset2);

      //      dump_offset(ctx->offset);

      ctx->src = ctx->buffer;
      ctx->src_stride = ctx->buffer_stride;
      ctx->dst_size = ctx->dst_rect.w;
      dst_save = dst->planes[ctx->dst_frame_plane] + ctx->offset->dst_offset;
      for(ctx->scanline = 0; ctx->scanline < ctx->dst_rect.h; ctx->scanline++)
        {
        //        fprintf(stderr, "i: %d\n", ctx->scanline);
        ctx->dst = dst_save;
        ctx->func2(ctx);
        dst_save += dst->strides[ctx->dst_frame_plane];
        }
      //      fprintf(stderr, "done\n");
      break;
    }
  }


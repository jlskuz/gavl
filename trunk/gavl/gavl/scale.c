
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <gavl/gavl.h>
#include <scale.h>
#include <accel.h>

// #define DUMP_TABLES

#define SCALE_ACTION_NOOP  0
#define SCALE_ACTION_COPY  1
#define SCALE_ACTION_SCALE 2

static void alloc_coeffs_1D(gavl_video_scale_coeff_1D_t ** coeffs,
                            int * coeffs_alloc, int num)
  {
  if(*coeffs_alloc < num)
    {
    *coeffs_alloc = num + 128;
    *coeffs = realloc(*coeffs, *coeffs_alloc * sizeof(**coeffs));
    }
  }

static void init_coeffs_nearest(gavl_video_scale_coeff_1D_t * coeffs,
                                int src_size, int dst_size)
  {
  int i;
  double scale_factor;
  double src_index_f;
  double dst_index_f;

  scale_factor = (float)(dst_size) / (float)(src_size);
    
  for(i = 0; i < dst_size; i++)
    {
    dst_index_f = (double)i + 0.5;
    src_index_f = dst_index_f / scale_factor;
    //    fprintf(stderr, "i: %d\n", i);
    coeffs[i].index = (int)(src_index_f);
    }

#ifdef DUMP_TABLES
  for(i = 0; i < dst_size; i++)
    {
    fprintf(stderr, "dst[%d] = src[%d]\n", i, coeffs[i].index[0]);
    }
#endif

  }

static void init_coeffs_bilinear_1D(gavl_video_scale_coeff_1D_t * coeffs,
                                    int src_size, int dst_size)
  {
  double scale_factor;
  double src_index_1_f;
  double dst_index_f;
  int i;
  double factor_2;
  
  scale_factor = (float)(dst_size) / (float)(src_size);

  if(src_size == 1)
    {
    for(i = 0; i < dst_size; i++)
      {
      coeffs[i].index = 0;
      coeffs[i].factor[0].fac_f = 1.0;
      coeffs[i].factor[1].fac_f = 0.0;
      }
    return;
    }
  
  for(i = 0; i < dst_size; i++)
    {
    dst_index_f = (double)i + 0.5;
    src_index_1_f = dst_index_f / scale_factor;

    //    printf("dst_index_f: %f src_index_1_f: %f\n", dst_index_f, 
    //           src_index_1_f);

    coeffs[i].index = (int)(floor(src_index_1_f-0.5));
        
    if(coeffs[i].index < 0)
      {
      coeffs[i].index++;
      factor_2 = 0.0;
      }
    else if(coeffs[i].index >= src_size-1)
      {
      coeffs[i].index--;
      factor_2 = 1.0;
      }
    else
      factor_2 = (src_index_1_f - coeffs[i].index) - 0.5;

    //    coeffs[i].index[1] = coeffs[i].index[0] + 1;
    
    coeffs[i].factor[1].fac_f = factor_2;
    if(coeffs[i].factor[1].fac_f > 1.0)
      coeffs[i].factor[1].fac_f = 1.0;

    coeffs[i].factor[0].fac_f = 1.0 - coeffs[i].factor[1].fac_f;
    
    // factor = 1.0 - (src_index_1_f - (double)src_index_1_i);
    // printf("dst[%d] = %f * src[%d] + %f * src[%d]\n",
    // i, 1.0 - factor_2, src_index_1_i, factor_2, src_index_1_i + 1);
    }
#ifdef DUMP_TABLES
  for(i = 0; i < dst_size; i++)
    fprintf(stderr, "dst[%d] = src[%d] * %f + dst[%d] * %f\n",
            i, coeffs[i].index[0], coeffs[i].factor[0].fac_f,
            coeffs[i].index[1], coeffs[i].factor[1].fac_f);
#endif
  }

#if 0
static void init_coeffs_bilinear_2D(gavl_video_scale_coeff_2D_t ** coeffs,
                                    gavl_video_scale_coeff_1D_t * coeffs_h,
                                    gavl_video_scale_coeff_1D_t * coeffs_v,
                                    int w, int h)
  {
  int i, j;

  for(i = 0; i < h; i++)
    {

    for(j = 0; j < w; j++)
      {
      //      coeffs[i][j].index[0] = 
      }
    
    }
  }
#endif

static void init_plane(gavl_video_scaler_t * s,
                       gavl_scale_mode_t scale_mode,
                       int plane)
  {
  /* Allocate coefficient arrays */
  s->table[plane].num_coeffs_h = s->dst_rect[plane].w;
  s->table[plane].num_coeffs_v = s->dst_rect[plane].h;

  switch(scale_mode)
    {
    case GAVL_SCALE_AUTO:
    case GAVL_SCALE_NEAREST:
      alloc_coeffs_1D(&(s->table[plane].coeffs_h),
                   &(s->table[plane].coeffs_h_alloc), s->dst_rect[plane].w);
      init_coeffs_nearest(s->table[plane].coeffs_h,
                          s->src_rect[plane].w, s->dst_rect[plane].w);
      break;
    case GAVL_SCALE_BILINEAR:

      if(s->src_rect[plane].w != s->dst_rect[plane].w)
        {
        alloc_coeffs_1D(&(s->table[plane].coeffs_h),
                     &(s->table[plane].coeffs_h_alloc), s->dst_rect[plane].w);
        init_coeffs_bilinear_1D(s->table[plane].coeffs_h,
                                s->src_rect[plane].w, s->dst_rect[plane].w);
        }
      break;
    case GAVL_SCALE_NONE:
      break;
    }
  switch(scale_mode)
    {
    case GAVL_SCALE_AUTO:
    case GAVL_SCALE_NEAREST:
        alloc_coeffs_1D(&(s->table[plane].coeffs_v),
                     &(s->table[plane].coeffs_v_alloc), s->dst_rect[plane].h);
        init_coeffs_nearest(s->table[plane].coeffs_v,
                            s->src_rect[plane].h, s->dst_rect[plane].h);
        break;
      case GAVL_SCALE_BILINEAR:
        if(s->src_rect[plane].h != s->dst_rect[plane].h)
          {
          alloc_coeffs_1D(&(s->table[plane].coeffs_v),
                       &(s->table[plane].coeffs_v_alloc), s->dst_rect[plane].h);
          
          init_coeffs_bilinear_1D(s->table[plane].coeffs_v,
                               s->src_rect[plane].h, s->dst_rect[plane].h);
          }
        break;
    case GAVL_SCALE_NONE:
      break;
    }
  }

/* Initialize scale table */

static int init_scale_table(gavl_video_scaler_t * s,
                            gavl_scale_mode_t scale_mode,
                            gavl_colorspace_t colorspace,
                            gavl_scale_funcs_t * funcs)
  {
  switch(colorspace)
    {
    case GAVL_COLORSPACE_NONE:
      return -1;
      break;
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      
      init_plane(s, scale_mode, 0);

      s->table[0].scanline_func = funcs->scale_15_16;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      
      init_plane(s, scale_mode, 0);
      
      s->table[0].scanline_func = funcs->scale_16_16;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
      
      init_plane(s, scale_mode, 0);
      s->table[0].scanline_func = funcs->scale_24_24;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      
      init_plane(s, scale_mode, 0);
      s->table[0].scanline_func = funcs->scale_24_32;
      break;
    case GAVL_RGBA_32:
    case GAVL_YUVA_32:
      
      init_plane(s, scale_mode, 0);
      s->table[0].scanline_func = funcs->scale_32_32;
      break;
    case GAVL_YUY2:
      
      init_plane(s, scale_mode, 0);
      init_plane(s, scale_mode, 1);
      
      s->table[0].scanline_func = funcs->scale_yuy2;
      break;
    case GAVL_UYVY:
      
      init_plane(s, scale_mode, 0);
      init_plane(s, scale_mode, 1);
      
      s->table[0].scanline_func = funcs->scale_uyvy;

      s->table[0].byte_advance = 2;
      s->table[1].byte_advance = 4;
      
      break;
    case GAVL_YUV_420_P:
    case GAVL_YUVJ_420_P:
      init_plane(s, scale_mode, 0);
      init_plane(s, scale_mode, 1);
      init_plane(s, scale_mode, 2);
            
      s->table[0].scanline_func = funcs->scale_8;
      s->table[1].scanline_func = funcs->scale_8;
      s->table[2].scanline_func = funcs->scale_8;

      s->table[0].byte_advance = 1;
      s->table[1].byte_advance = 1;
      s->table[2].byte_advance = 1;
      break;
    case GAVL_YUV_422_P:
    case GAVL_YUVJ_422_P:

      init_plane(s, scale_mode, 0);
      init_plane(s, scale_mode, 1);
      init_plane(s, scale_mode, 2);
            
      s->table[0].scanline_func = funcs->scale_8;
      s->table[1].scanline_func = funcs->scale_8;
      s->table[2].scanline_func = funcs->scale_8;

      s->table[0].byte_advance = 1;
      s->table[1].byte_advance = 1;
      s->table[2].byte_advance = 1;

      break;
    case GAVL_YUV_444_P:
    case GAVL_YUVJ_444_P:
      

      init_plane(s, scale_mode, 0);
      init_plane(s, scale_mode, 1);
      init_plane(s, scale_mode, 2);

      s->table[0].scanline_func = funcs->scale_8;
      s->table[1].scanline_func = funcs->scale_8;
      s->table[2].scanline_func = funcs->scale_8;
      s->table[0].byte_advance = 1;
      s->table[1].byte_advance = 1;
      s->table[2].byte_advance = 1;
      
      break;
    case GAVL_YUV_411_P:
      

      init_plane(s, scale_mode, 0);
      init_plane(s, scale_mode, 1);
      init_plane(s, scale_mode, 2);

      s->table[0].scanline_func = funcs->scale_8;
      s->table[1].scanline_func = funcs->scale_8;
      s->table[2].scanline_func = funcs->scale_8;
      s->table[0].byte_advance = 1;
      s->table[1].byte_advance = 1;
      s->table[2].byte_advance = 1;


      break;
    case GAVL_YUV_410_P:
      init_plane(s, scale_mode, 0);
      init_plane(s, scale_mode, 1);
      init_plane(s, scale_mode, 2);

      s->table[0].scanline_func = funcs->scale_8;
      s->table[1].scanline_func = funcs->scale_8;
      s->table[2].scanline_func = funcs->scale_8;
      s->table[0].byte_advance = 1;
      s->table[1].byte_advance = 1;
      s->table[2].byte_advance = 1;
      break;
    }
  if(!s->table[0].scanline_func)
    return 0;
  return 1;
  }

#define FREE(ptr) if(ptr) free(ptr);

static void free_scale_table(gavl_video_scale_table_t * tab)
  {
  FREE(tab->coeffs_h);
  FREE(tab->coeffs_v);
  }

#undef FREE

/* Public functions */

void gavl_video_scaler_destroy(gavl_video_scaler_t * s)
  {
  int i;

  gavl_video_frame_null(s->src);
  gavl_video_frame_null(s->dst);

  gavl_video_frame_destroy(s->src);
  gavl_video_frame_destroy(s->dst);

  for(i = 0; i < 4; i++)
    free_scale_table(&(s->table[i]));
  free(s);  
  }

gavl_video_scaler_t * gavl_video_scaler_create()
  {
  gavl_video_scaler_t * ret;
  ret = calloc(1, sizeof(*ret));

  ret->src = gavl_video_frame_create((gavl_video_format_t*)0);
  ret->dst = gavl_video_frame_create((gavl_video_format_t*)0);
  
  return ret;
  }


int gavl_video_scaler_init(gavl_video_scaler_t * scaler,
                           gavl_colorspace_t colorspace,
                           gavl_rectangle_t * src_rect,
                           gavl_rectangle_t * dst_rect,
                           const gavl_video_format_t * src_format,
                           const gavl_video_format_t * dst_format)
  {
  int i;
  int chroma_sub_h,  chroma_sub_v; 
  gavl_scale_funcs_t funcs;

  int scale_x, scale_y;

#ifdef ARCH_X86
  int min_scanline_width;
#endif
  
  scaler->colorspace = colorspace;
  scaler->num_planes = gavl_colorspace_num_planes(colorspace);
  gavl_colorspace_chroma_sub(colorspace, &chroma_sub_h, &chroma_sub_v);

#if 0
  fprintf(stderr, "src_format: ");
  gavl_video_format_dump(src_format);
  fprintf(stderr, "\n");

  fprintf(stderr, "dst_format: ");
  gavl_video_format_dump(dst_format);
  fprintf(stderr, "\n");

  fprintf(stderr, "src_rect 1: ");
  gavl_rectangle_dump(src_rect);
  fprintf(stderr, "\n");

  fprintf(stderr, "dst_rect 1: ");
  gavl_rectangle_dump(dst_rect);
  fprintf(stderr, "\n");
#endif
  
  /* Crop rectangles to formats */
  
  gavl_rectangle_crop_to_format_scale(src_rect,
                                      dst_rect,
                                      src_format,
                                      dst_format);

#if 0
  fprintf(stderr, "src_rect 2: ");
  gavl_rectangle_dump(src_rect);
  fprintf(stderr, "\n");

  fprintf(stderr, "dst_rect_2: ");
  gavl_rectangle_dump(dst_rect);
  fprintf(stderr, "\n");
#endif

  /* Align src and dst windows */
  
  gavl_rectangle_align(src_rect, chroma_sub_h, chroma_sub_v);
  gavl_rectangle_align(dst_rect, chroma_sub_h, chroma_sub_v);

  
  /* Copy arguments */
  gavl_rectangle_copy(&(scaler->src_rect[0]), src_rect);
  gavl_rectangle_copy(&(scaler->dst_rect[0]), dst_rect);
  
  /* Check if there is something to scale */

  if(gavl_rectangle_is_empty(&(scaler->src_rect[0])) ||
     gavl_rectangle_is_empty(&(scaler->dst_rect[0])))
    {
    scaler->action = SCALE_ACTION_NOOP;
    return 1;
    }

  scale_x = (scaler->src_rect[0].w == scaler->dst_rect[0].w) ? 0 : 1;
  scale_y = (scaler->src_rect[0].h == scaler->dst_rect[0].h) ? 0 : 1;

  if(!scale_x && !scale_y)
    {
    scaler->action = SCALE_ACTION_COPY;

    scaler->copy_format.colorspace = colorspace;
    scaler->copy_format.image_width = scaler->src_rect[0].w;
    scaler->copy_format.frame_width = scaler->src_rect[0].w;

    scaler->copy_format.image_height = scaler->src_rect[0].h;
    scaler->copy_format.frame_height = scaler->src_rect[0].h;

    scaler->copy_format.pixel_width  = 1;
    scaler->copy_format.pixel_height = 1;
    return 1;
    }

  scaler->action = SCALE_ACTION_SCALE;
  
  /* Set up rectangles for other planes */

  if((scaler->num_planes == 1) && ((chroma_sub_h > 1) || ((chroma_sub_v > 1))))
    {
    for(i = 1; i < GAVL_MAX_PLANES; i++)
      {
      gavl_rectangle_subsample(&(scaler->src_rect[i]),
                               &(scaler->src_rect[0]),
                               chroma_sub_h, chroma_sub_v);
      gavl_rectangle_subsample(&(scaler->dst_rect[i]),
                               &(scaler->dst_rect[0]),
                               chroma_sub_h, chroma_sub_v);
      }
    }
  
  for(i = 1; i < scaler->num_planes; i++)
    {
    gavl_rectangle_subsample(&(scaler->src_rect[i]),
                             &(scaler->src_rect[0]),
                             chroma_sub_h, chroma_sub_v);
    
    gavl_rectangle_subsample(&(scaler->dst_rect[i]),
                             &(scaler->dst_rect[0]),
                             chroma_sub_h, chroma_sub_v);
    }

  memset(&funcs, 0, sizeof(funcs));
    
  if(!scaler->opt.accel_flags || (scaler->opt.accel_flags & GAVL_ACCEL_C))
    {
    gavl_init_scale_funcs_c(&funcs,
                            scaler->opt.scale_mode,
                            scale_x, scale_y);
    }
  
#ifdef ARCH_X86

  min_scanline_width = scaler->dst_rect[1].w ? scaler->dst_rect[1].w : scaler->dst_rect[0].w;
  
  if((!scaler->opt.accel_flags && (scaler->opt.quality < 3)) ||
     scaler->opt.accel_flags & GAVL_ACCEL_MMX)
    {
    gavl_init_scale_funcs_mmx(&funcs,
                              scaler->opt.scale_mode,
                              scale_x, scale_y, min_scanline_width);
    }
  if((!scaler->opt.accel_flags && (scaler->opt.quality < 3)) ||
     scaler->opt.accel_flags & GAVL_ACCEL_MMXEXT)
    {
    gavl_init_scale_funcs_mmxext(&funcs,
                                 scaler->opt.scale_mode,
                                 scale_x, scale_y, min_scanline_width);
    }
#endif
  
  if(!init_scale_table(scaler,
                       scaler->opt.scale_mode,
                       colorspace,
                       &funcs))
    return -1;
  return 1;
  }

gavl_video_options_t * gavl_video_scaler_get_options(gavl_video_scaler_t * s)
  {
  return &(s->opt);
  }

void gavl_video_scaler_scale(gavl_video_scaler_t * s,
                             gavl_video_frame_t * src,
                             gavl_video_frame_t * dst)
  {
  uint8_t * dst_ptr;
  int i, j;

  if(s->action == SCALE_ACTION_NOOP)
    return;
    
  gavl_video_frame_get_subframe(s->colorspace, src, 
                                s->src, &(s->src_rect[0]));

  gavl_video_frame_get_subframe(s->colorspace, dst, 
                                s->dst, &(s->dst_rect[0]));

  if(s->action == SCALE_ACTION_COPY)
    {
    //    fprintf(stderr, "Copying frame\n");
    gavl_video_frame_copy(&(s->copy_format),
                          s->dst, s->src);
    return;
    }
  
  for(i = 0; i < s->num_planes; i++)
    {
    dst_ptr = s->dst->planes[i];
    for(j = 0; j < s->table[i].num_coeffs_v; j++)
      {
      s->table[i].scanline_func(s, s->src->planes[i],
                                s->src->strides[i],
                                dst_ptr,
                                i, j);
      dst_ptr += s->dst->strides[i];
      }
    }
  }

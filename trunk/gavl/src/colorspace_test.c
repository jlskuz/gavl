#include "../include/gavl.h"
//#include "colorspace.h" // Common routines
#include <stdio.h>
#include <png.h>
#include <unistd.h>
#include <stdlib.h>


// Masks for BGR16 and RGB16 formats

#define RGB16_LOWER_MASK  0x001f
#define RGB16_MIDDLE_MASK 0x07e0
#define RGB16_UPPER_MASK  0xf800

// Extract unsigned char RGB values from 15 bit pixels

#define RGB16_TO_R(pixel) ((pixel & RGB16_UPPER_MASK)>>8)
#define RGB16_TO_G(pixel) ((pixel & RGB16_MIDDLE_MASK)>>3) 
#define RGB16_TO_B(pixel) ((pixel & RGB16_LOWER_MASK)<<3)

#define RGB15_LOWER_MASK  0x001f
#define RGB15_MIDDLE_MASK 0x3e0
#define RGB15_UPPER_MASK  0x7C00

#define RGB15_TO_R(pixel) ((pixel & RGB15_UPPER_MASK)>>7)
#define RGB15_TO_G(pixel) ((pixel & RGB15_MIDDLE_MASK)>>2) 
#define RGB15_TO_B(pixel) ((pixel & RGB15_LOWER_MASK)<<3)

// ((((((src[2]<<5)&0xff00)|src[1])<<6)&0xfff00)|src[0])>>3;

#define PACK_RGB16(_r,_g,_b,_pixel) \
_pixel=((((((_r<<5)&0xff00)|_g)<<6)&0xfff00)|_b)>>3

// ((((((src[2]<<5)&0xff00)|src[1])<<5)&0xfff00)|src[0])>>3;

#define PACK_RGB15(_r,_g,_b,_pixel) \
_pixel=((((((_r<<5)&0xff00)|_g)<<5)&0xfff00)|_b)>>3

#define TEST_PICTURE_WIDTH  320
#define TEST_PICTURE_HEIGHT 256

/*
 *   Some braindead YUV conversion (but works at least :-)
 */

static int * r_to_y = (int*)0;
static int * g_to_y = (int*)0;
static int * b_to_y = (int*)0;

static int * r_to_u = (int*)0;
static int * g_to_u = (int*)0;
static int * b_to_u = (int*)0;

static int * r_to_v = (int*)0;
static int * g_to_v = (int*)0;
static int * b_to_v = (int*)0;

static int * y_to_rgb = (int*)0;
static int * v_to_r = (int*)0;
static int * u_to_g = (int*)0;
static int * v_to_g = (int*)0;
static int * u_to_b = (int*)0;

void init_yuv()
  {
  int i;
  
  r_to_y = malloc(0x100 * sizeof(int));
  g_to_y = malloc(0x100 * sizeof(int));
  b_to_y = malloc(0x100 * sizeof(int));

  r_to_u = malloc(0x100 * sizeof(int));
  g_to_u = malloc(0x100 * sizeof(int));
  b_to_u = malloc(0x100 * sizeof(int));

  r_to_v = malloc(0x100 * sizeof(int));
  g_to_v = malloc(0x100 * sizeof(int));
  b_to_v = malloc(0x100 * sizeof(int));

  y_to_rgb = malloc(0x100 * sizeof(int));
  v_to_r   = malloc(0x100 * sizeof(int));
  u_to_g   = malloc(0x100 * sizeof(int));
  v_to_g   = malloc(0x100 * sizeof(int));
  u_to_b   = malloc(0x100 * sizeof(int));
  
  for(i = 0; i < 0x100; i++)
    {
    // RGB to YUV conversion

    r_to_y[i] = (int)(0.257*0x10000 * i + 16 * 0x10000);
    g_to_y[i] = (int)(0.504*0x10000 * i);
    b_to_y[i] = (int)(0.098*0x10000 * i);
    
    r_to_u[i] = (int)(-0.148*0x10000 * i);
    g_to_u[i] = (int)(-0.291*0x10000 * i);
    b_to_u[i] = (int)( 0.439*0x10000 * i + 0x800000);
    
    r_to_v[i] = (int)( 0.439*0x10000 * i);
    g_to_v[i] = (int)(-0.368*0x10000 * i);
    b_to_v[i] = (int)(-0.071*0x10000 * i + 0x800000);


    // YUV to RGB conversion

    y_to_rgb[i] = (int)(1.164*(i-16)) * 0x10000;
    
    v_to_r[i]   = (int)( 1.596  * (i - 0x80) * 0x10000);
    u_to_g[i]   = (int)(-0.392  * (i - 0x80) * 0x10000);
    v_to_g[i]   = (int)(-0.813  * (i - 0x80) * 0x10000);
    u_to_b[i]   = (int)( 2.017 * (i - 0x80) * 0x10000);
    }
  }

#define RECLIP(color) (uint8_t)((color>0xFF)?0xff:((color<0)?0:color))

#define YUV_2_RGB(y,u,v,r,g,b) i_tmp=(y_to_rgb[y]+v_to_r[v])>>16;\
                                r=RECLIP(i_tmp);\
                                i_tmp=(y_to_rgb[y]+u_to_g[u]+v_to_g[v])>>16;\
                                g=RECLIP(i_tmp);\
                                i_tmp=(y_to_rgb[y]+u_to_b[u])>>16;\
                                b=RECLIP(i_tmp);

#define RGB_TO_YUV(r,g,b,y,u,v) y=(r_to_y[r]+g_to_y[g]+b_to_y[b])>>16;\
                               u=(r_to_u[r]+g_to_u[g]+b_to_u[b])>>16;\
                               v=(r_to_v[r]+g_to_v[g]+b_to_v[b])>>16

#define RGB_TO_Y(r,g,b,y) y=(r_to_y[r]+g_to_y[g]+b_to_y[b])>>16

void convert_15_to_24(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;

  uint16_t * in_pixel;
  uint8_t  * out_pixel;

  uint8_t  * out_pixel_save = out_frame->pixels;
  uint8_t * in_pixel_save = in_frame->pixels;
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = RGB15_TO_R(*in_pixel);
      out_pixel[1] = RGB15_TO_G(*in_pixel);
      out_pixel[2] = RGB15_TO_B(*in_pixel);
      
      in_pixel++;
      out_pixel += 3;
      }
    in_pixel_save += in_frame->pixels_stride;
    out_pixel_save +=  out_frame->pixels_stride;
    }
  }

void convert_16_to_24(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;
  uint16_t * in_pixel;
  uint8_t  * out_pixel;

  uint8_t  * out_pixel_save = out_frame->pixels;
  uint8_t * in_pixel_save = in_frame->pixels;
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint16_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = RGB16_TO_R(*in_pixel);
      out_pixel[1] = RGB16_TO_G(*in_pixel);
      out_pixel[2] = RGB16_TO_B(*in_pixel);

      in_pixel++;
      out_pixel += 3;
      }
    in_pixel_save += in_frame->pixels_stride;
    out_pixel_save +=  out_frame->pixels_stride;
    }
  }

void convert_32_to_24(gavl_video_frame_t * in_frame,
                      gavl_video_frame_t * out_frame,
                      int width, int height)
  {
  int i, j;
  uint8_t * in_pixel;
  uint8_t  * out_pixel;
  
  uint8_t  * out_pixel_save = out_frame->pixels;
  uint8_t * in_pixel_save = in_frame->pixels;
  
  for(i = 0; i < height; i++)
    {
    in_pixel = (uint8_t*)in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width; j++)
      {
      out_pixel[0] = in_pixel[0];
      out_pixel[1] = in_pixel[1];
      out_pixel[2] = in_pixel[2];
      in_pixel+=4;
      out_pixel+=3;
      }
    in_pixel_save += in_frame->pixels_stride;
    out_pixel_save +=  out_frame->pixels_stride;
    }
  }

void convert_YUV_420_P_to_RGB24(gavl_video_frame_t * in_frame,
                                gavl_video_frame_t * out_frame,
                                int width, int height)
  {
  int i, j, i_tmp;
  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->pixels;
  uint8_t * in_y_save = in_frame->y;
  uint8_t * in_u_save = in_frame->u;
  uint8_t * in_v_save = in_frame->v;

  for(i = 0; i < height/2; i++)
    {

    /* Even Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->pixels_stride;
    in_y_save += in_frame->y_stride;

    /* Odd Rows */

    out_pixel = out_pixel_save;
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->pixels_stride;
    in_y_save += in_frame->y_stride;

    in_u_save += in_frame->u_stride;
    in_v_save += in_frame->v_stride;
    }
  }

void convert_YUV_422_P_to_RGB24(gavl_video_frame_t * in_frame,
                               gavl_video_frame_t * out_frame,
                               int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_y;
  uint8_t * in_u;
  uint8_t * in_v;

  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->pixels;
  uint8_t * in_y_save = in_frame->y;
  uint8_t * in_u_save = in_frame->u;
  uint8_t * in_v_save = in_frame->v;

  for(i = 0; i < height; i++)
    {
    in_y = in_y_save;
    in_u = in_u_save;
    in_v = in_v_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      out_pixel += 3;
      
      YUV_2_RGB(*in_y, *in_u, *in_v,
                out_pixel[0], out_pixel[1], out_pixel[2]);
      in_y++;
      in_u++;
      in_v++;
      out_pixel += 3;
      }
    out_pixel_save += out_frame->pixels_stride;
    in_y_save += in_frame->y_stride;
    in_u_save += in_frame->u_stride;
    in_v_save += in_frame->v_stride;
    }
  }

void convert_YUY2_to_RGB24(gavl_video_frame_t * in_frame,
                           gavl_video_frame_t * out_frame,
                           int width, int height)
  {
  int i, j, i_tmp;

  uint8_t * in_pixel;
  uint8_t * out_pixel;

  uint8_t * out_pixel_save = out_frame->pixels;
  uint8_t * in_pixel_save = in_frame->pixels;

  for(i = 0; i < height; i++)
    {
    in_pixel = in_pixel_save;
    out_pixel = out_pixel_save;
    for(j = 0; j < width/2; j++)
      {
      YUV_2_RGB(in_pixel[0], in_pixel[1], in_pixel[3],
                out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel+=3;
      YUV_2_RGB(in_pixel[2], in_pixel[1], in_pixel[3],
                out_pixel[0], out_pixel[1], out_pixel[2]);
      out_pixel+=3;
      in_pixel += 4;
      }
    in_pixel_save += in_frame->pixels_stride;
    out_pixel_save +=  out_frame->pixels_stride;
    }
  }

/*
 *  This function writes a png file of the video frame in the given format
 *  The format can have all supported colorspaces, so we'll convert them
 *  without the use of gavl (i.e. in less optimized but bugfree code)
 */

/* On error, FALSE is returned */

int write_file(const char * name,
               gavl_video_frame_t * frame, gavl_video_format_t * format)
  {
  int color_type;  
  int png_transforms;
  gavl_video_frame_t * tmp_frame;
  gavl_video_frame_t * out_frame = (gavl_video_frame_t *)0;
  gavl_video_format_t tmp_format;

  int i;
  char ** row_pointers;  
  png_structp png_ptr;  png_infop info_ptr;
  
  FILE *fp = fopen(name, "wb");
  if (!fp)
    {
    fprintf(stderr, "Cannot open file %s, exiting \n", name);
    _exit(-1);
    return 0;
    }

  png_ptr = png_create_write_struct
    (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr)
    return 0;
  
  info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
    {
    png_destroy_write_struct(&png_ptr,
                             (png_infopp)NULL);
    return 0;
    }
  
  setjmp(png_jmpbuf(png_ptr));

  png_init_io(png_ptr, fp);

  switch(format->colorspace)
    {
    case GAVL_RGBA_32:
      color_type = PNG_COLOR_TYPE_RGB_ALPHA;
      break;
    default:
      color_type = PNG_COLOR_TYPE_RGB;
    }
  
  png_set_IHDR(png_ptr, info_ptr, format->width, format->height,
               8, color_type, PNG_INTERLACE_NONE,
                PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  /* Set up the temporary video frame */

  tmp_format.width = format->width;
  tmp_format.height = format->height;

  /* We convert everything to either RGB24, BGR24 or RGBA */
  
  switch(format->colorspace)
    {
    case GAVL_BGR_15:
    case GAVL_BGR_16:
    case GAVL_BGR_24:
    case GAVL_BGR_32:
      png_transforms = PNG_TRANSFORM_BGR;
      tmp_format.colorspace = GAVL_BGR_24;
      break;
    default:
      png_transforms = PNG_TRANSFORM_IDENTITY;
      tmp_format.colorspace = GAVL_RGB_24;
    }

  /* Allocate the video frame structure */

  tmp_frame = NULL;

  switch(format->colorspace)
    {
    case GAVL_RGB_15:
    case GAVL_BGR_15:
      tmp_frame = gavl_create_video_frame(&tmp_format);
      convert_15_to_24(frame, tmp_frame, format->width, format->height);
      out_frame = tmp_frame;
      break;
    case GAVL_RGB_16:
    case GAVL_BGR_16:
      tmp_frame = gavl_create_video_frame(&tmp_format);
      convert_16_to_24(frame, tmp_frame, format->width, format->height);
      out_frame = tmp_frame;
      break;
    case GAVL_RGB_24:
    case GAVL_BGR_24:
    case GAVL_RGBA_32:
      out_frame = frame;
      break;
    case GAVL_RGB_32:
    case GAVL_BGR_32:
      tmp_frame = gavl_create_video_frame(&tmp_format);
      convert_32_to_24(frame, tmp_frame, format->width, format->height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUY2:
      tmp_frame = gavl_create_video_frame(&tmp_format);
      convert_YUY2_to_RGB24(frame, tmp_frame, format->width, format->height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_420_P:
      tmp_frame = gavl_create_video_frame(&tmp_format);
      convert_YUV_420_P_to_RGB24(frame, tmp_frame, format->width,
                                 format->height);
      out_frame = tmp_frame;
      break;
    case GAVL_YUV_422_P:
      tmp_frame = gavl_create_video_frame(&tmp_format);
      convert_YUV_422_P_to_RGB24(frame, tmp_frame, format->width,
                                 format->height);
      out_frame = tmp_frame;
      break;
    }

  /* Set up the row pointers */

  row_pointers = malloc(format->height * sizeof(char*));
 
  for(i = 0; i < format->height; i++)
    row_pointers[i] =
      out_frame->pixels + i * out_frame->pixels_stride;
  
  png_set_rows(png_ptr, info_ptr, (png_bytep*)row_pointers);

  png_write_png(png_ptr, info_ptr, png_transforms, NULL);

  free(row_pointers);

  if(tmp_frame)
    gavl_destroy_video_frame(tmp_frame);
  png_destroy_write_struct(&png_ptr, &info_ptr);
  fclose(fp);
    
  return 1;
  }

/**********************************************************
 * Test picture generator
 **********************************************************/

static uint8_t colorbar_colors[16][2][3] =
  {
    {
      { 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, } /* White */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, }
    },
    {
      { 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0x00, }  /* Yellow */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0x00, }
    },
    {
      { 0x00, 0x00, 0x00 }, { 0x00, 0xFF, 0xFF, } /* Cyan */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0x00, 0xFF, 0xFF, } 
    },
    {
      { 0x00, 0x00, 0x00 }, { 0x00, 0xFF, 0x00, } /* Green */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0x00, 0xFF, 0x00, } 
    },
    {
      { 0x00, 0x00, 0x00 }, { 0xFF, 0x00, 0xFF, } /* Magenta */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0xFF, 0x00, 0xFF, }
    },
    {
      { 0x00, 0x00, 0x00 }, { 0xFF, 0x00, 0x00, } /* Red */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0xFF, 0x00, 0x00, }
    },
    {
      { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0xFF, }  /* Blue */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0x00, 0x00, 0xFF, } 
    },
    {
      { 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, }, /* Black */
    },
    {
      { 0xFF, 0xFF, 0xFF }, { 0x00, 0x00, 0x00, }
    }
  };

/* Define this to have a horizontal colorbar */

#define COLORBAR_HORIZONAL

void get_pixel_colorbar(int x, int y, uint8_t * r_ret, uint8_t * g_ret,
                        uint8_t * b_ret, uint8_t * a_ret)
  {
  int color_index;
  int r_tmp, g_tmp, b_tmp, a_tmp;
  
  uint8_t alpha = ((TEST_PICTURE_HEIGHT - y) * 0xff) / (TEST_PICTURE_HEIGHT);
  color_index = (x * 16) / (TEST_PICTURE_WIDTH - 1);


  /*  fprintf(stderr, "y: %d alpha: %d\n", y ,(int)alpha); */
/* #ifdef COLORBAR_HORIZONAL */
/* #else */
/* #endif */

  // colorbar_colors[16][2][3]
  
  r_tmp = (colorbar_colors[color_index][0][0] * (0xFF - alpha) + 
           colorbar_colors[color_index][1][0] * alpha) >> 8; 
  g_tmp = (colorbar_colors[color_index][0][1] * (0xFF - alpha) +
           colorbar_colors[color_index][1][1] * alpha) >> 8; 
  b_tmp = (colorbar_colors[color_index][0][2] * (0xFF - alpha) +
           colorbar_colors[color_index][1][2] * alpha) >> 8; 

/*   r_tmp =  colorbar_colors[color_index][1][0]; */
/*   g_tmp =  colorbar_colors[color_index][1][1]; */
/*   b_tmp =  colorbar_colors[color_index][1][2]; */
  
  a_tmp = 0xFF; /* Not used for now */
  
  *r_ret = RECLIP(r_tmp);
  *g_ret = RECLIP(g_tmp);
  *b_ret = RECLIP(b_tmp);
  *a_ret = RECLIP(a_tmp);
  
  }

                        

gavl_video_frame_t * create_picture(gavl_colorspace_t colorspace,
                                    void (*get_pixel)(int x, int y,
                                                     uint8_t * r_ret,
                                                     uint8_t * g_ret,
                                                     uint8_t * b_ret,
                                                     uint8_t * a_ret))
     
  {
  uint8_t r_tmp;
  uint8_t g_tmp;
  uint8_t b_tmp;
  uint8_t a_tmp;

  uint8_t  * pixel;
  uint16_t * pixel_16;
  uint8_t * y;
  uint8_t * u;
  uint8_t * v;
    
  int row, col;
  gavl_video_frame_t * ret;
  gavl_video_format_t format;
  format.colorspace = colorspace;
  format.width = TEST_PICTURE_WIDTH;
  format.height = TEST_PICTURE_HEIGHT;
  
  ret = gavl_create_video_frame(&format);

  switch(colorspace)
    {
    case GAVL_RGB_15:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->pixels + row * ret->pixels_stride);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          PACK_RGB15(r_tmp, g_tmp, b_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_BGR_15:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->pixels + row * ret->pixels_stride);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          PACK_RGB15(b_tmp, g_tmp, r_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_RGB_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->pixels + row * ret->pixels_stride);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          PACK_RGB16(r_tmp, g_tmp, b_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_BGR_16:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel_16 = (uint16_t*)(ret->pixels + row * ret->pixels_stride);
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          PACK_RGB16(b_tmp, g_tmp, r_tmp, *pixel_16);
          pixel_16++;
          }
        }
      break;
    case GAVL_RGB_24:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->pixels + row * ret->pixels_stride;
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &pixel[0], &pixel[1], &pixel[2], &a_tmp);
          pixel += 3;
          }
        }
      break;
    case GAVL_BGR_24:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->pixels + row * ret->pixels_stride;
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &pixel[2], &pixel[1], &pixel[0], &a_tmp);
          pixel += 3;
          }
        }
      break;
    case GAVL_RGB_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->pixels + row * ret->pixels_stride;
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &pixel[0], &pixel[1], &pixel[2], &a_tmp);
          pixel += 4;
          }
        }
      break;
    case GAVL_BGR_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->pixels + row * ret->pixels_stride;
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &pixel[2], &pixel[1], &pixel[0], &a_tmp);
          pixel += 4;
          }
        }
      break;
    case GAVL_RGBA_32:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->pixels + row * ret->pixels_stride;
        for(col = 0; col < TEST_PICTURE_WIDTH; col++)
          {
          get_pixel(col, row, &pixel[0], &pixel[1], &pixel[2], &pixel[3]);
          pixel += 4;
          }
        }
      break;
    case GAVL_YUY2:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        pixel = ret->pixels + row * ret->pixels_stride;
        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          RGB_TO_YUV(r_tmp, g_tmp, b_tmp, pixel[0], pixel[1], pixel[3]);
                    
          get_pixel(2*col+1, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          RGB_TO_Y(r_tmp, g_tmp, b_tmp, pixel[2]);
          pixel+= 4;
          }
        }
      break;
    case GAVL_YUV_420_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT/2; row++)
        {
        y = ret->y + 2 * row * ret->y_stride;
        u = ret->u + row * ret->u_stride;
        v = ret->v + row * ret->v_stride;

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, 2*row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);

          RGB_TO_YUV(r_tmp, g_tmp, b_tmp, *y, *u, *v);

          y++;
          
          get_pixel(2*col+1, 2*row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          RGB_TO_YUV(r_tmp, g_tmp, b_tmp, *y, *u, *v);
          
          y++;
          u++;
          v++;
          }

        y = ret->y + (2 * row + 1) * ret->y_stride;

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2*col, 2*row+1, &r_tmp, &g_tmp, &b_tmp, &a_tmp);

          RGB_TO_Y(r_tmp, g_tmp, b_tmp, *y);

          y++;
          
          get_pixel(2*col+1, 2*row+1, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          RGB_TO_Y(r_tmp, g_tmp, b_tmp, *y);
          
          y++;
          }
        }
      break;
    case GAVL_YUV_422_P:
      for(row = 0; row < TEST_PICTURE_HEIGHT; row++)
        {
        y = ret->y + row * ret->y_stride;
        u = ret->u + row * ret->u_stride;
        v = ret->v + row * ret->v_stride;

        for(col = 0; col < TEST_PICTURE_WIDTH/2; col++)
          {
          get_pixel(2* col, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);

          RGB_TO_YUV(r_tmp, g_tmp, b_tmp, *y, *u, *v);

          y++;
          
          get_pixel(2* col + 1, row, &r_tmp, &g_tmp, &b_tmp, &a_tmp);
          RGB_TO_YUV(r_tmp, g_tmp, b_tmp, *y, *u, *v);
          
          y++;
          u++;
          v++;
          }
        }
      break;
    }
  
  return  ret;
  }

int main(int argc, char ** argv)
  {
  int i, j;
  const char * tmp1, * tmp2;
  char filename_buffer[128];

  gavl_video_format_t input_format;
  gavl_video_format_t output_format;

  gavl_video_frame_t * input_frame;
  gavl_video_frame_t * output_frame;
  
  gavl_video_options_t opt;

  gavl_video_converter_t * cnv = gavl_create_video_converter();
  
  input_format.width = TEST_PICTURE_WIDTH;
  input_format.height = TEST_PICTURE_HEIGHT;

  output_format.width = TEST_PICTURE_WIDTH;
  output_format.height = TEST_PICTURE_HEIGHT;
 
  init_yuv();
 
  for(i = 0; i < gavl_num_colorspaces(); i++)
    {
    input_format.colorspace = gavl_get_colorspace(i);
    input_frame = create_picture(input_format.colorspace,
                                 get_pixel_colorbar);

    tmp1 = gavl_colorspace_to_string(input_format.colorspace);
    sprintf(filename_buffer, "0test_%s.png", tmp1);
    
    write_file(filename_buffer,
                 input_frame, &input_format);

    for(j = 0; j < gavl_num_colorspaces(); j++)
      {
      gavl_video_default_options(&opt);

      output_format.colorspace = gavl_get_colorspace(j);

      if(input_format.colorspace == output_format.colorspace)
        continue;
      
      output_frame = gavl_create_video_frame(&output_format);
      fprintf(stderr, "************* Colorspace conversion ");
      fprintf(stderr, "%s -> ", tmp1);
      tmp2 = gavl_colorspace_to_string(output_format.colorspace);
      
      fprintf(stderr, "%s *************\n", tmp2);

      opt.accel_flags = GAVL_ACCEL_C;
        
      if(!gavl_video_init(cnv, &opt, &input_format, &output_format))
        {
        fprintf(stderr, "No Conversion defined yet\n");
        continue;
        }

      /* Now, do some conversions */

      sprintf(filename_buffer, "%s_to_%s_c.png", tmp1, tmp2);

      gavl_clear_video_frame(output_frame, &output_format);
      
      fprintf(stderr, "ANSI C Version: ");
      
      gavl_video_convert(cnv, input_frame, output_frame);

      write_file(filename_buffer,
                 output_frame, &output_format);
      fprintf(stderr, "Wrote %s\n", filename_buffer);
      
      
      /* Now, initialize with MMX */

      opt.accel_flags = GAVL_ACCEL_MMX;
      
      if(!gavl_video_init(cnv, &opt, &input_format, &output_format))
        fprintf(stderr, "No MMX Conversion defined yet\n");
      else
        {
        sprintf(filename_buffer, "%s_to_%s_mmx.png", tmp1, tmp2);
        gavl_clear_video_frame(output_frame, &output_format);
        fprintf(stderr, "MMX Version:    ");
        
        gavl_video_convert(cnv, input_frame, output_frame);
        
        write_file(filename_buffer,
                   output_frame, &output_format);
        fprintf(stderr, "Wrote %s\n", filename_buffer);
        }

      opt.accel_flags = GAVL_ACCEL_MMXEXT;
      
      if(!gavl_video_init(cnv, &opt, &input_format, &output_format))
        {
        fprintf(stderr, "No MMXEXT Conversion defined yet\n");
        }
      else
        {
        sprintf(filename_buffer, "%s_to_%s_mmxext.png", tmp1, tmp2);
        gavl_clear_video_frame(output_frame, &output_format);
        fprintf(stderr, "MMXEXT Version:    ");
        
        gavl_video_convert(cnv, input_frame, output_frame);
        
        write_file(filename_buffer,
                   output_frame, &output_format);
        fprintf(stderr, "Wrote %s\n", filename_buffer);
        }

      
      gavl_destroy_video_frame(output_frame);

      
      }
    
    }
  
  
  return 0;
  }

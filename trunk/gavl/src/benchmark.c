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


#include <config.h>
#include <gavl.h>
#include <accel.h> /* Private header */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <sys/time.h>
#include <time.h>

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

#undef ARCH_X86

#define OUT_PFMT GAVL_RGB_24
#define IN_PFMT GAVL_PIXELFORMAT_NONE

#define IN_LOOP
// #define OUT_LOOP


// #define INIT_RUNS 5
// #define NUM_RUNS 10

#define INIT_RUNS 100
#define NUM_RUNS  200

int do_html = 0;

#if defined(ARCH_X86) || defined(HAVE_SYS_TIMES_H)
static unsigned long long int get_time(int config_flags)
{
struct timeval tv;
#ifdef ARCH_X86
  unsigned long long int x;
  /* that should prevent us from trying cpuid with old cpus */
  if( config_flags & GAVL_ACCEL_MMX ) {
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
  } else {
#endif
  gettimeofday(&tv, NULL);
  return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
#ifdef ARCH_X86
  }
#endif
}
#else



static uint64_t get_time(int config_flags)
{
struct timeval tv;
gettimeofday(&tv, NULL);
return (uint64_t)(tv.tv_sec) * 1000000 + tv.tv_usec;
/* FIXME: implement an equivalent for using optimized memcpy on other
            architectures */
#ifdef HAVE_SYS_TIMES_H
#else
	return ((uint64_t)0);
#endif /* HAVE_SYS_TIMES_H */
}
#endif

typedef struct
  {
  uint64_t times[NUM_RUNS];
  uint64_t min;
  uint64_t max;
  uint64_t avg;

  void (*func)(void*);
  void (*init)(void*);
  void *data;

  int accel_supported;
  int num_discard;
  
  } gavl_benchmark_t;



static void gavl_benchmark_run(gavl_benchmark_t * b)
  {
  int i;
  uint64_t time_before;
  uint64_t time_after;
  b->accel_supported = gavl_accel_supported();
  b->avg = 0;
  
  for(i = 0; i < INIT_RUNS + NUM_RUNS; i++)
    {
    if(b->init)
      b->init(b->data);
    time_before = get_time(b->accel_supported);
    b->func(b->data);
    time_after = get_time(b->accel_supported);

    if(i >= INIT_RUNS)
      {
      b->times[i-INIT_RUNS] = time_after - time_before;
      b->avg += b->times[i-INIT_RUNS];
      }
    }
  
  b->avg /= NUM_RUNS;
  b->num_discard = 0;
  
  for(i = 0; i < NUM_RUNS; i++)
    {
    if(b->times[i] > (3 * b->avg)/2)
      {
      b->times[i] = 0;
      b->num_discard++;
      }
    }
  i = 0;
  while(!b->times[i])
    i++;

  b->min = b->times[i];
  b->max = b->times[i];
  b->avg = b->times[i];
  i++;
  
  while(i < NUM_RUNS)
    {
    if(!b->times[i])
      {
      i++;
      continue;
      }
    if(b->times[i] > b->max)
      b->max = b->times[i];
    if(b->times[i] < b->min)
      b->min = b->times[i];

    b->avg += b->times[i];
    i++;
    }
  b->avg /= (NUM_RUNS-b->num_discard);
  }

static void gavl_benchmark_print_header(gavl_benchmark_t * b)
  {
  if(do_html)
    {
    printf("<td align=\"right\">Average</td><td align=\"right\">Minimum</td><td align=\"right\">Maximum</td><td align=\"right\">Discarded</td>");
    }
  }

static void gavl_benchmark_print_results(gavl_benchmark_t * b)
  {
  if(do_html)
    {
    printf("<td align=\"right\">%"PRId64"</td><td align=\"right\">%"PRId64"</td><td align=\"right\">%"PRId64"</td><td align=\"right\">%d</td>",
         b->avg, b->min, b->max, b->num_discard);
    }
  else
    printf("  Avg: %8"PRId64", Min: %8"PRId64", Max: %8"PRId64", %2d\n",
           b->avg, b->min, b->max, b->num_discard);
  }

/* Audio conversions */

typedef struct
  {
  gavl_audio_format_t in_format;
  gavl_audio_format_t out_format;
  
  gavl_audio_converter_t * cnv;
  gavl_audio_options_t * opt;
  
  gavl_audio_frame_t * in_frame;
  gavl_audio_frame_t * out_frame;
  
  } audio_convert_context_t;

static void audio_convert_context_create(audio_convert_context_t * ctx)
  {
  ctx->cnv = gavl_audio_converter_create();
  ctx->opt = gavl_audio_converter_get_options(ctx->cnv);
  }


static void audio_convert_context_init(audio_convert_context_t * ctx)
  {
  ctx->in_frame = gavl_audio_frame_create(&ctx->in_format);
  ctx->out_frame = gavl_audio_frame_create(&ctx->out_format);
  gavl_audio_converter_init(ctx->cnv, &ctx->in_format, &ctx->out_format);
  }

static void audio_convert_context_cleanup(audio_convert_context_t * ctx)
  {
  gavl_audio_frame_destroy(ctx->in_frame);
  gavl_audio_frame_destroy(ctx->out_frame);
  }

static void audio_convert_context_destroy(audio_convert_context_t * ctx)
  {
  gavl_audio_converter_destroy(ctx->cnv);
  }

static void audio_convert_init(void * data)
  {
  audio_convert_context_t * ctx = (audio_convert_context_t*)data;
  ctx->in_frame->valid_samples = ctx->in_format.samples_per_frame;
  }

static void audio_convert_func(void * data)
  {
  audio_convert_context_t * ctx = (audio_convert_context_t*)data;
  gavl_audio_convert(ctx->cnv, ctx->in_frame, ctx->out_frame);
  }

static const gavl_sample_format_t sampleformats[] =
  {
    GAVL_SAMPLE_U8, /*!< Unsigned 8 bit */
    GAVL_SAMPLE_S8, /*!< Signed 8 bit */
    GAVL_SAMPLE_U16, /*!< Unsigned 16 bit */
    GAVL_SAMPLE_S16, /*!< Signed 16 bit */
    GAVL_SAMPLE_S32, /*!< Signed 32 bit */
    GAVL_SAMPLE_FLOAT,  /*!< Floating point (-1.0 .. 1.0) */
    GAVL_SAMPLE_DOUBLE  /*!< Double (-1.0 .. 1.0) */
  };

static const struct
  {
  gavl_audio_dither_mode_t mode;
  const char * name;
  }
dither_modes[] =
  {
    { GAVL_AUDIO_DITHER_NONE, "None" },
    { GAVL_AUDIO_DITHER_RECT, "Rect"}, 
    { GAVL_AUDIO_DITHER_TRI,  "Triangular" },
    { GAVL_AUDIO_DITHER_SHAPED, "Shaped" }
  };

static void benchmark_sampleformat()
  {
  int num_sampleformats;
  int num_dither_modes;
  gavl_sample_format_t in_format;
  gavl_sample_format_t out_format;

  int i, j, k;

  audio_convert_context_t ctx;
  gavl_benchmark_t b;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = audio_convert_init;
  b.func = audio_convert_func;
  b.data = &ctx;
  
  ctx.in_format.num_channels = 2;
  ctx.in_format.samplerate = 48000;
  ctx.in_format.interleave_mode = GAVL_INTERLEAVE_NONE;
  ctx.in_format.channel_locations[0] = GAVL_CHID_NONE;
  ctx.in_format.samples_per_frame = 10240;

  gavl_set_channel_setup(&ctx.in_format);
  
  gavl_audio_format_copy(&ctx.out_format, &ctx.in_format);

  num_sampleformats = sizeof(sampleformats)/sizeof(sampleformats[0]);
  num_dither_modes = sizeof(dither_modes)/sizeof(dither_modes[0]);

  audio_convert_context_create(&ctx);

  if(do_html)
    {
    printf("Conversion of %d samples, %d channels<p>\n", ctx.in_format.samples_per_frame,
           ctx.in_format.num_channels);
    printf("<table border=\"1\" width=\"100%%\"><tr><td>Conversion</td><td>Dithering</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  
  for(i = 0; i < num_sampleformats; i++)
    {
    in_format = sampleformats[i];
    ctx.in_format.sample_format = in_format;
    for(j = 0; j < num_sampleformats; j++)
      {
      out_format = sampleformats[j];
      if(in_format == out_format)
        continue;
      
      ctx.out_format.sample_format = out_format;

      if((gavl_bytes_per_sample(out_format) > 2) ||
         (in_format < GAVL_SAMPLE_FLOAT))
        {
        if(do_html)
          {
          printf("<td>%s -> %s</td><td>Not available</td>",
                 gavl_sample_format_to_string(in_format),
                 gavl_sample_format_to_string(out_format));
          }
        else
          printf("%s -> %s\n",
                 gavl_sample_format_to_string(in_format),
                 gavl_sample_format_to_string(out_format));
        
        gavl_audio_options_set_dither_mode(ctx.opt, GAVL_AUDIO_DITHER_NONE);
        audio_convert_context_init(&ctx);
        gavl_benchmark_run(&b);
        audio_convert_context_cleanup(&ctx);
        gavl_benchmark_print_results(&b);

        if(do_html)
          {
          printf("</tr>");
          }
        printf("\n");
        }
      else
        {
        for(k = 0; k < num_dither_modes; k++)
          {
          if(do_html)
            {
            printf("<td>%s -> %s</td><td>%s</td>",
                   gavl_sample_format_to_string(in_format),
                   gavl_sample_format_to_string(out_format),
                   dither_modes[k].name);
            }
          else
            printf("%s -> %s, Dithering: %s\n",
                   gavl_sample_format_to_string(in_format),
                   gavl_sample_format_to_string(out_format),
                   dither_modes[k].name);
           
          gavl_audio_options_set_dither_mode(ctx.opt, dither_modes[k].mode);
          audio_convert_context_init(&ctx);
          gavl_benchmark_run(&b);
          audio_convert_context_cleanup(&ctx);
          gavl_benchmark_print_results(&b);
          
          if(do_html)
            {
            printf("</tr>");
            }
          printf("\n");
          }
        }
      }
    }
  audio_convert_context_destroy(&ctx);

  if(do_html)
    printf("</table>\n");
  
  }

static void benchmark_mix()
  {
  int num_sampleformats;
  gavl_sample_format_t in_format;

  int i;

  audio_convert_context_t ctx;
  gavl_benchmark_t b;
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = audio_convert_init;
  b.func = audio_convert_func;
  b.data = &ctx;
  
  ctx.in_format.num_channels = 6;
  ctx.in_format.samplerate = 48000;
  ctx.in_format.interleave_mode = GAVL_INTERLEAVE_NONE;
  ctx.in_format.channel_locations[0] = GAVL_CHID_NONE;
  ctx.in_format.samples_per_frame = 102400;

  gavl_set_channel_setup(&ctx.in_format);
  
  gavl_audio_format_copy(&ctx.out_format, &ctx.in_format);

  ctx.out_format.num_channels = 2;
  ctx.out_format.channel_locations[0] = GAVL_CHID_NONE;
  gavl_set_channel_setup(&ctx.out_format);

  num_sampleformats = sizeof(sampleformats)/sizeof(sampleformats[0]);
  audio_convert_context_create(&ctx);

  if(do_html)
    {
    printf("Mixing of %d samples, from %d to %d channels<p>\n",
           ctx.in_format.samples_per_frame,
           ctx.in_format.num_channels,
           ctx.out_format.num_channels);
    printf("<table border=\"1\" width=\"100%%\"><tr><td>Conversion</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }

  
  for(i = 0; i < num_sampleformats; i++)
    {
    in_format = sampleformats[i];
    ctx.in_format.sample_format = in_format;
    ctx.out_format.sample_format = in_format;
    printf("<td>%s</td>", gavl_sample_format_to_string(in_format));
    audio_convert_context_init(&ctx);
    gavl_benchmark_run(&b);
    audio_convert_context_cleanup(&ctx);
    gavl_benchmark_print_results(&b);

    if(do_html)
      {
      printf("</tr>");
      }
    printf("\n");
    }
  
  audio_convert_context_destroy(&ctx);
  
  if(do_html)
    printf("</table>\n");
  
  }

/* Video converter */

typedef struct
  {
  gavl_video_format_t in_format;
  gavl_video_format_t out_format;
  
  gavl_video_converter_t * cnv;
  gavl_video_options_t * opt;
  
  gavl_video_frame_t * in_frame;
  gavl_video_frame_t * out_frame;
  
  } video_convert_context_t;

static void video_convert_context_create(video_convert_context_t * ctx)
  {
  ctx->cnv = gavl_video_converter_create();
  ctx->opt = gavl_video_converter_get_options(ctx->cnv);
  }


static int video_convert_context_init(video_convert_context_t * ctx)
  {
  ctx->in_frame = gavl_video_frame_create(&ctx->in_format);
  ctx->out_frame = gavl_video_frame_create(&ctx->out_format);
  return gavl_video_converter_init(ctx->cnv, &ctx->in_format, &ctx->out_format) <= 0 ? 0 : 1;
  }

static void video_convert_context_cleanup(video_convert_context_t * ctx)
  {
  gavl_video_frame_destroy(ctx->in_frame);
  gavl_video_frame_destroy(ctx->out_frame);
  }

static void video_convert_context_destroy(video_convert_context_t * ctx)
  {
  gavl_video_converter_destroy(ctx->cnv);
  }

static void video_convert_init(void * data)
  {
  //  video_convert_context_t * ctx = (video_convert_context_t*)data;
  
  }

static void video_convert_func(void * data)
  {
  video_convert_context_t * ctx = (video_convert_context_t*)data;
  gavl_video_convert(ctx->cnv, ctx->in_frame, ctx->out_frame);
  }

static void do_pixelformat(video_convert_context_t * ctx,
                           gavl_benchmark_t * b, gavl_pixelformat_t in_format,
                           gavl_pixelformat_t out_format, char * name)
  {
  int flags;
  int in_sub_h;
  int in_sub_v;
  int out_sub_h;
  int out_sub_v;
  

  gavl_pixelformat_chroma_sub(in_format, &in_sub_h, &in_sub_v);
  gavl_pixelformat_chroma_sub(out_format, &out_sub_h, &out_sub_v);
  ctx->in_format.pixelformat = in_format;
  ctx->out_format.pixelformat = out_format;
  
  if((in_sub_h == out_sub_h) && (in_sub_v == out_sub_v))
    {
    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    flags &= ~GAVL_RESAMPLE_CHROMA;
    gavl_video_options_set_conversion_flags(ctx->opt, flags);
    
    if(video_convert_context_init(ctx))
      {
      if(do_html)
        {
        printf("<tr><td>%s</td><td>Not needed</td>", name);
        }
      else
        printf("  %s           ", name);
      
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      if(do_html)
        printf("</tr>\n");
      }
    video_convert_context_cleanup(ctx);
    }
  else
    {
    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    flags &= ~GAVL_RESAMPLE_CHROMA;
    gavl_video_options_set_conversion_flags(ctx->opt, flags);
    
    if(video_convert_context_init(ctx))
      {
      if(do_html)
        {
        printf("<tr><td>%s<td>Off</td>", name);
        }
      else
        printf("  %s [off]     ", name);
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      
      if(do_html)
        printf("</tr>\n");
      }
    video_convert_context_cleanup(ctx);

    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    flags |= GAVL_RESAMPLE_CHROMA;
    gavl_video_options_set_conversion_flags(ctx->opt, flags);
    gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_BILINEAR);
    
    if(video_convert_context_init(ctx))
      {
      if(do_html)
        {
        printf("<tr><td>%s</td><td>Linear</td>", name);
        }
      else
        printf("  %s [linear]  ", name);
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      if(do_html)
        printf("</tr>\n");
      }
    video_convert_context_cleanup(ctx);

    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    gavl_video_options_set_conversion_flags(ctx->opt, flags);
    gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_CUBIC_BSPLINE);
    
    if(video_convert_context_init(ctx))
      {
      if(do_html)
        {
        printf("<tr><td>%s</td><td>B-Spline</td>", name);
        }
      else
        printf("  %s [bspline] ", name);
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      if(do_html)
        printf("</tr>\n");
      }
    video_convert_context_cleanup(ctx);

    flags = gavl_video_options_get_conversion_flags(ctx->opt);
    gavl_video_options_set_conversion_flags(ctx->opt, flags);
    gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_SINC_LANCZOS);
    
    if(video_convert_context_init(ctx))
      {
      if(do_html)
        {
        printf("<tr><td>%s</td><td>Sinc (4th order)</td>", name);
        }
      else
        printf("  %s [sinc 4]  ", name);
      gavl_benchmark_run(b);
      gavl_benchmark_print_results(b);
      if(do_html)
        printf("</tr>\n");
      }
    video_convert_context_cleanup(ctx);

    }
  }

static void benchmark_pixelformat()
  {
  int num_pixelformats;
  gavl_pixelformat_t in_format;
  gavl_pixelformat_t out_format;
  int i, j;

  video_convert_context_t ctx;
  gavl_benchmark_t b;
  
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = video_convert_init;
  b.func = video_convert_func;
  b.data = &ctx;
  
  ctx.in_format.image_width = 720;
  ctx.in_format.image_height = 576;

  ctx.in_format.frame_width = 720;
  ctx.in_format.frame_height = 576;
  ctx.in_format.pixel_width = 1;
  ctx.in_format.pixel_height = 1;
  
  gavl_video_format_copy(&ctx.out_format, &ctx.in_format);

  if(do_html)
    {
    printf("Image size: %d x %d<p>\n",
           ctx.in_format.image_width,
           ctx.in_format.image_height);
    printf("<table border=\"1\" width=\"100%%\"><tr><td>Flavour</td><td>Chroma resampling</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }

  
  num_pixelformats = gavl_num_pixelformats();
  
  video_convert_context_create(&ctx);
  /* Disable autoselection */
  gavl_video_options_set_quality(ctx.opt, 0);

  /* Must set this to prevent a crash (not in gavl but due to
   *  the benchmarking logic)
   */
  gavl_video_options_set_alpha_mode(ctx.opt, GAVL_ALPHA_BLEND_COLOR);

#ifndef IN_LOOP
  in_format = IN_PFMT;
#else
  for(i = 0; i < num_pixelformats; i++)
    {
    in_format = gavl_get_pixelformat(i);
#endif
    
#ifndef OUT_LOOP
    out_format = OUT_PFMT;
#else
    for(j = 0; j < num_pixelformats; j++)
      {
      out_format = gavl_get_pixelformat(j);
#endif
      if(in_format == out_format)
        continue;

      if(do_html)
        printf("<tr><td colspan=\"6\"><b>%s -> %s<b></td></tr>\n",
               gavl_pixelformat_to_string(in_format),
               gavl_pixelformat_to_string(out_format));
      
      
      /* C-Version */
      
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C);
      do_pixelformat(&ctx, &b, in_format, out_format, "C    ");
      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMX);
      do_pixelformat(&ctx, &b, in_format, out_format, "MMX  ");

      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMXEXT);
      do_pixelformat(&ctx, &b, in_format, out_format, "MMXEXT  ");

      fflush(stdout);
      gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C_HQ);
      do_pixelformat(&ctx, &b, in_format, out_format, "HQ   ");
      fflush(stdout);
      
#ifdef OUT_LOOP
      }
#endif


#ifdef IN_LOOP
    }
#endif
  video_convert_context_destroy(&ctx);
  
  }


static void do_scale(video_convert_context_t * ctx,
                     gavl_benchmark_t * b, gavl_pixelformat_t in_format,
                     char * name)
  {
  
  ctx->in_format.pixelformat = in_format;
  ctx->out_format.pixelformat = in_format;

  gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_NEAREST);
  if(video_convert_context_init(ctx))
    {
    if(do_html)
      printf("<tr><td>%s</td><td>Nearest</td>", name);
    
    gavl_benchmark_run(b);
    gavl_benchmark_print_results(b);
    if(do_html)
      printf("</tr>\n");
    }
  video_convert_context_cleanup(ctx);

  gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_BILINEAR);
  if(video_convert_context_init(ctx))
    {
    if(do_html)
      printf("<tr><td>%s</td><td>Linear</td>", name);
    
    gavl_benchmark_run(b);
    gavl_benchmark_print_results(b);
    if(do_html)
      printf("</tr>\n");
    }
  video_convert_context_cleanup(ctx);

  
  gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_QUADRATIC);
  if(video_convert_context_init(ctx))
    {
    if(do_html)
      printf("<tr><td>%s</td><td>Quadratic</td>", name);
    
    gavl_benchmark_run(b);
    gavl_benchmark_print_results(b);
    if(do_html)
      printf("</tr>\n");
    }
  video_convert_context_cleanup(ctx);

  gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_CUBIC_BSPLINE);
  if(video_convert_context_init(ctx))
    {
    if(do_html)
      printf("<tr><td>%s</td><td>Cubic B-Spline</td>", name);
    
    gavl_benchmark_run(b);
    gavl_benchmark_print_results(b);
    if(do_html)
      printf("</tr>\n");
    }
  video_convert_context_cleanup(ctx);


  
  gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_CUBIC_CATMULL);
  if(video_convert_context_init(ctx))
    {
    if(do_html)
      printf("<tr><td>%s</td><td>Cubic Cubic Catmull-Rom</td>", name);
    
    gavl_benchmark_run(b);
    gavl_benchmark_print_results(b);
    if(do_html)
      printf("</tr>\n");
    }
  video_convert_context_cleanup(ctx);
  
  gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_CUBIC_MITCHELL);
  if(video_convert_context_init(ctx))
    {
    if(do_html)
      printf("<tr><td>%s</td><td>Cubic Mitchell-Netravali</td>", name);
    
    gavl_benchmark_run(b);
    gavl_benchmark_print_results(b);
    if(do_html)
      printf("</tr>\n");
    }
  video_convert_context_cleanup(ctx);

  gavl_video_options_set_scale_mode(ctx->opt, GAVL_SCALE_SINC_LANCZOS);
  gavl_video_options_set_scale_order(ctx->opt, 4);
  if(video_convert_context_init(ctx))
    {
    if(do_html)
      printf("<tr><td>%s</td><td>Sinc, 4th order</td>", name);
    
    gavl_benchmark_run(b);
    gavl_benchmark_print_results(b);
    if(do_html)
      printf("</tr>\n");
    }
  video_convert_context_cleanup(ctx);

  
  
  }


static void benchmark_scale()
  {
  int num_pixelformats;
  gavl_pixelformat_t in_format;
  int i;

  video_convert_context_t ctx;
  gavl_benchmark_t b;
  
  memset(&ctx, 0, sizeof(ctx));
  memset(&b, 0, sizeof(b));

  b.init = video_convert_init;
  b.func = video_convert_func;
  b.data = &ctx;
  
  ctx.in_format.image_width = 720;
  ctx.in_format.image_height = 576;
  ctx.in_format.frame_width = 720;
  ctx.in_format.frame_height = 576;
  
  ctx.in_format.pixel_width = 1;
  ctx.in_format.pixel_height = 1;
  
  gavl_video_format_copy(&ctx.out_format, &ctx.in_format);

  ctx.out_format.image_width = 1280;
  ctx.out_format.image_height = 1024;
  ctx.out_format.frame_width = 1280;
  ctx.out_format.frame_height = 1024;
  
  if(do_html)
    {
    printf("Source size: %d x %d, Destination size: %d x %d<p>\n",
           ctx.in_format.image_width,
           ctx.in_format.image_height,
           ctx.out_format.image_width,
           ctx.out_format.image_height);
    printf("<table border=\"1\" width=\"100%%\"><tr><td>Flavour</td><td>Scale method</td>");
    gavl_benchmark_print_header(&b);
    printf("</tr>\n");
    }
  
  num_pixelformats = gavl_num_pixelformats();
  
  video_convert_context_create(&ctx);
  /* Disable autoselection */
  gavl_video_options_set_quality(ctx.opt, 0);
  
  for(i = 0; i < num_pixelformats; i++)
  //  for(i = 0; i < 3; i++)
    {
    in_format = gavl_get_pixelformat(i);
    
    if(do_html)
      printf("<tr><td colspan=\"6\"><b>%s<b></td></tr>\n",
             gavl_pixelformat_to_string(in_format));

    /* C-Version */
      
    gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C);
    do_scale(&ctx, &b, in_format, "C    ");
    
    gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_MMX);
    do_scale(&ctx, &b, in_format, "MMX  ");
    
    gavl_video_options_set_accel_flags(ctx.opt, GAVL_ACCEL_C_HQ);
    do_scale(&ctx, &b, in_format, "HQ   ");
        
    }
  video_convert_context_destroy(&ctx);
  }



#define BENCHMARK_SAMPLEFORMAT (1<<0)
#define BENCHMARK_MIX          (1<<1)
#define BENCHMARK_VOLUME       (1<<2)
#define BENCHMARK_PEAK_DETECT  (1<<3)
#define BENCHMARK_INTERLEAVE   (1<<4)

#define BENCHMARK_PIXELFORMAT  (1<<5)
#define BENCHMARK_SCALE        (1<<6)
#define BENCHMARK_DEINTERLACE  (1<<7)
#define BENCHMARK_INTERPOLATE  (1<<8)
#define BENCHMARK_SAD          (1<<9)

static const struct
  {
  const char * option;
  const char * help;
  int flag;
  }
benchmark_flags[] =
  {
    { "-sfmt", "Sampleformat conversions", BENCHMARK_SAMPLEFORMAT },
    { "-mix", "Audio mixing", BENCHMARK_MIX },
    { "-vol", "Volume control", BENCHMARK_VOLUME},
    { "-pd", "Peak detection", BENCHMARK_PEAK_DETECT},
    { "-il", "Interleaving conversion", BENCHMARK_INTERLEAVE},
    { "-pfmt", "Pixelformat conversions", BENCHMARK_PIXELFORMAT },
    { "-scale", "Scale", BENCHMARK_SCALE},
    { "-deint", "Deinterlacing", BENCHMARK_DEINTERLACE},
    { "-ip", "Video frame interpolation", BENCHMARK_INTERPOLATE},
    { "-sad", "SAD routines", BENCHMARK_SAD},
  };

static int get_flag(char * opt, int * flag_ret)
  {
  int i;
  for(i = 0; i < sizeof(benchmark_flags)/sizeof(benchmark_flags[0]); i++)
    {
    if(!strcmp(benchmark_flags[i].option, opt))
      {
      *flag_ret = benchmark_flags[i].flag;
      return 1;
      }
    }
  return 0;
  }

int main(int argc, char ** argv)
  {
  int i;
  int flags = 0;
  int flag;

  i = 1;

  while(i < argc)
    {
    if(get_flag(argv[i], &flag))
      flags |= flag;
    else if(!strcmp(argv[i], "-a"))
      flags = ~0x0;
    else if(!strcmp(argv[i], "-html"))
      do_html = 1;
    i++;
    }

  if(do_html)
    {
    printf("<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\">\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>Gmerlin</title>\n");
    printf("<link rel=\"stylesheet\" href=\"css/style.css\">\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<h1>gavl Benchmarks</h1>");
    printf("These benchmarks are generated with the benchmark tool in the src/ directory of gavl.<br>");
    printf("Number of init runs: %d, number of counted runs: %d<br>\n", INIT_RUNS, NUM_RUNS);
    printf("Times are microseconds returned by gettimeofday<br>\n");
    printf("<h2>/proc/cpuinfo</h2>\n");
    printf("<pre>\n");
    fflush(stdout);
    system("cat /proc/cpuinfo");
    printf("</pre>\n");
    
    if(flags & BENCHMARK_SAMPLEFORMAT)
      printf("<a href=\"#sfmt\">Sampleformat conversions</a><br>\n");

    if(flags & BENCHMARK_MIX)
      printf("<a href=\"#mix\">Mixing routines</a><br>\n");
    if(flags & BENCHMARK_PIXELFORMAT)
      printf("<a href=\"#pfmt\">Pixelformat conversions</a><br>\n");
    if(flags & BENCHMARK_SCALE)
      printf("<a href=\"#scale\">Pixelformat conversions</a><br>\n");
    
    }
  
  if(flags & BENCHMARK_SAMPLEFORMAT)
    {
    if(do_html)
      {
      printf("<a name=\"sfmt\"></a>");
      printf("<h2>Sampleformat conversions</h2>\n");
      }
    benchmark_sampleformat();
    }
  
  if(flags & BENCHMARK_MIX)
    {
    if(do_html)
      {
      printf("<a name=\"mix\"></a>");
      printf("<h2>Mixing routines</h2>\n");
      }
    benchmark_mix();
    }
  if(flags & BENCHMARK_PIXELFORMAT)
    {
    if(do_html)
      {
      printf("<a name=\"pfmt\"></a>");
      printf("<h2>Pixelformat conversions</h2>\n");
      }
    benchmark_pixelformat();
    }
  if(flags & BENCHMARK_SCALE)
    {
    if(do_html)
      {
      printf("<a name=\"scale\"></a>");
      printf("<h2>Video scaling</h2>\n");
      }
    benchmark_scale();
    }
  if(do_html)
    {
    printf("</body>\n</html>\n");
    }
  
  return 0;
  }


#include <string.h>

#include <gavfprivate.h>
#include <gavl/metatags.h>

/* Footer structure
 *
 * GAVFFOOT
 * array of dictionaries
 * stream footer * num_streams
 * GAVFFOOT
 * Relative start position (64 bit)
 */


#if 0
  int32_t size_min;
  int32_t size_max;
  int64_t duration_min;
  int64_t duration_max;
  int64_t pts_start;
  int64_t pts_end;

  int64_t total_bytes;   // For average bitrate 
  int64_t total_packets; // For average framerate
#endif

#define TAG_SIZE_MIN      "smin"
#define TAG_SIZE_MAX      "smax"
#define TAG_DURATION_MIN  "dmin"
#define TAG_DURATION_MAX  "dmax"
#define TAG_PTS_START     "tmin"
#define TAG_PTS_END       "tmax"
#define TAG_TOTAL_PACKETS "packets"
#define TAG_TOTAL_BYTES   "bytes"
#define TAG_STATS         "stats"

static void stats_to_dict(const gavf_stream_stats_t * s, gavl_dictionary_t * dict)
  {
  gavl_dictionary_set_int(dict, TAG_SIZE_MIN, s->size_min);
  gavl_dictionary_set_int(dict, TAG_SIZE_MAX, s->size_max);

  gavl_dictionary_set_long(dict, TAG_DURATION_MIN, s->duration_min);
  gavl_dictionary_set_long(dict, TAG_DURATION_MAX, s->duration_max);

  gavl_dictionary_set_long(dict, TAG_PTS_START, s->pts_start);
  gavl_dictionary_set_long(dict, TAG_PTS_END,   s->pts_end);

  gavl_dictionary_set_long(dict, TAG_TOTAL_PACKETS, s->total_packets);
  gavl_dictionary_set_long(dict, TAG_TOTAL_BYTES,   s->total_bytes);
  }

static int stats_from_dict(gavf_stream_stats_t * s, const gavl_dictionary_t * dict)
  {
  return gavl_dictionary_get_int(dict, TAG_SIZE_MIN, &s->size_min) &&
    gavl_dictionary_get_int(dict, TAG_SIZE_MAX, &s->size_max)  &&
    gavl_dictionary_get_long(dict, TAG_DURATION_MIN, &s->duration_min) &&
    gavl_dictionary_get_long(dict, TAG_DURATION_MAX, &s->duration_max) &&
    gavl_dictionary_get_long(dict, TAG_PTS_START, &s->pts_start) &&
    gavl_dictionary_get_long(dict, TAG_PTS_END,   &s->pts_end) &&
    gavl_dictionary_get_long(dict, TAG_TOTAL_PACKETS, &s->total_packets) &&
    gavl_dictionary_get_long(dict, TAG_TOTAL_BYTES,   &s->total_bytes);
  }

int gavf_footer_check(gavf_t * g)
  {
  uint8_t buf[8];
  int64_t last_pos;
  int64_t footer_start_pos;
  int i;
  gavf_stream_header_t * s;
  int ret = 0;
  gavf_chunk_t chunk;

  gavl_dictionary_t foot;
  const gavl_array_t * stats_arr;
  
  if(!g->io->seek_func)
    return 0;
  
  last_pos = g->io->position;

  /* Read last 16 bytes */
  gavf_io_seek(g->io, -16, SEEK_END);
  if((gavf_io_read_data(g->io, buf, 8) < 8))
    goto end;

  if(memcmp(buf, GAVF_TAG_TAIL, 8))
    goto end;
    
  if(!gavf_io_read_int64f(g->io, &footer_start_pos))
    goto end;
  
  /* Seek to footer start */
  gavf_io_seek(g->io, footer_start_pos, SEEK_SET);
  gavl_dictionary_init(&foot);
  
  if(!gavf_chunk_read_header(g->io, &chunk) ||
     !gavf_chunk_is(&chunk, GAVF_TAG_FOOTER) ||
     !gavl_dictionary_read(g->io, &foot) ||
     !(stats_arr = gavl_dictionary_get_array(&foot, "stats")) ||
     (g->ph.num_streams != stats_arr->num_entries))
    goto end;
  
  for(i = 0; i < g->ph.num_streams; i++)
    {
    const gavl_dictionary_t * stats;

    s = g->ph.streams + i;

    if(!(stats = gavl_value_get_dictionary(&stats_arr->entries[i])) ||
       !stats_from_dict(&s->stats, stats))
      goto end;
    
    /* Set some useful values from the footer */
    gavf_stream_header_apply_footer(s);
    }

  /* Read remaining stuff */
  ret = 1;

  while(gavf_chunk_read_header(g->io, &chunk))
    {
    if(gavf_chunk_is(&chunk, GAVF_TAG_SYNC_INDEX))
      {
      if(gavf_sync_index_read(g->io, &g->si))
        g->opt.flags |= GAVF_OPT_FLAG_SYNC_INDEX;
      }
    else if(gavf_chunk_is(&chunk, GAVF_TAG_PACKET_INDEX))
      {
      if(gavf_packet_index_read(g->io, &g->pi))
        g->opt.flags |= GAVF_OPT_FLAG_PACKET_INDEX;
      }
    else
      {
      /* TODO: Skip */
      }
    
    }
  
  end:
    
  gavf_io_seek(g->io, last_pos, SEEK_SET);
  return ret;
    
  }

int gavf_footer_write(gavf_t * g)
  {
  int i;
  gavl_dictionary_t foot;
  gavl_value_t arr_val;
  gavl_array_t * arr;

  
  gavf_stream_header_t * s;
  uint64_t footer_start_pos = g->io->position;
  if(gavf_io_write_data(g->io, (uint8_t*)GAVF_TAG_FOOTER, 8) < 8)
    return 0;

  gavl_dictionary_init(&foot);

  gavl_value_init(&arr_val);

  arr = gavl_value_set_array(&arr_val);
  
  for(i = 0; i < g->ph.num_streams; i++)
    {
    gavl_value_t stats_val;
    gavl_dictionary_t * stats;

    s = g->ph.streams + i;
    
    gavl_value_init(&stats_val);
    stats = gavl_value_set_dictionary(&stats_val);

    stats_to_dict(&s->stats, stats);

    gavl_array_splice_val_nocopy(arr, -1, 0, &stats_val);
    }
  gavl_dictionary_set_nocopy(&foot, "stats", &arr_val);
  
  /* Write indices */  
  if(g->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    {
    if(g->opt.flags & GAVF_OPT_FLAG_DUMP_INDICES)
      gavf_sync_index_dump(&g->si);
    if(!gavf_sync_index_write(g->io, &g->si))
      return 0;
    }
    
  if(g->opt.flags & GAVF_OPT_FLAG_PACKET_INDEX)
    {
    if(g->opt.flags & GAVF_OPT_FLAG_DUMP_INDICES)
      gavf_packet_index_dump(&g->pi);
    if(!gavf_packet_index_write(g->io, &g->pi))
      return 0;
    }
  
  /* Write final tag */
  if((gavf_io_write_data(g->io, (uint8_t*)GAVF_TAG_FOOTER, 8) < 8) ||
     !gavf_io_write_uint64f(g->io, footer_start_pos))
    return 0;
  
  return 1;
  }

void gavf_footer_init(gavf_program_header_t * ph)
  {
  int i;
  gavf_stream_header_t * s;

  for(i = 0; i < ph->num_streams; i++)
    {
    s = ph->streams + i;
    gavf_stream_stats_init(&s->stats);
    }
  }

void gavf_stream_stats_init(gavf_stream_stats_t * f)
  {
  memset(f, 0, sizeof(*f));
  f->duration_min = GAVL_TIME_UNDEFINED;
  f->duration_max = GAVL_TIME_UNDEFINED;
  f->pts_start    = GAVL_TIME_UNDEFINED;
  f->pts_end      = GAVL_TIME_UNDEFINED;
  f->size_min     = -1;
  f->size_max     = -1;
  }
  
void gavf_stream_stats_update(gavf_stream_stats_t * f, const gavl_packet_t * p)
  {
  gavf_stream_stats_update_params(f, p->pts, p->duration, p->data_len,
                                  p->flags);
  }

void gavf_stream_stats_update_params(gavf_stream_stats_t * f,
                                     int64_t pts, int64_t duration, int data_len,
                                     int flags)
  {
  if(f->pts_start == GAVL_TIME_UNDEFINED)
    f->pts_start    = pts;

  if((duration > 0) && !(flags & GAVL_PACKET_NOOUTPUT))
    {
    if((f->pts_end == GAVL_TIME_UNDEFINED) || (f->pts_end < pts + duration))
      f->pts_end = pts + duration;

    if((f->duration_min == GAVL_TIME_UNDEFINED) ||
       (f->duration_min > duration))
      f->duration_min = duration;

    if((f->duration_max == GAVL_TIME_UNDEFINED) ||
       (f->duration_max < duration))
      f->duration_max = duration;
    }
  

  if(data_len > 0)
    {
    if((f->size_min < 0) || (f->size_min > data_len))
      f->size_min = data_len;
    
    if((f->size_max < 0) || (f->size_max < data_len))
      f->size_max = data_len;
    }
  
  if(!(flags & GAVL_PACKET_NOOUTPUT))
    f->total_packets++;
  
  if(data_len > 0)
    f->total_bytes += data_len;
  }

void gavf_stream_stats_apply_generic(gavf_stream_stats_t * f,
                                     gavl_compression_info_t * ci,
                                     gavl_dictionary_t * m)
  {
  if(ci && (ci->max_packet_size <= 0))
    ci->max_packet_size = f->size_max;

  if((f->pts_start != GAVL_TIME_UNDEFINED) && (f->pts_start != 0))
    gavl_dictionary_set_long(m, GAVL_META_STREAM_PTS_START, f->pts_start);
  if(f->pts_end > 0)
    gavl_dictionary_set_long(m, GAVL_META_STREAM_PTS_END, f->pts_end);
  if(f->pts_end > 0)
    {
    int64_t duration = f->pts_end;
    if((f->pts_start != GAVL_TIME_UNDEFINED) && (f->pts_start != 0))
      duration -= f->pts_start;
    gavl_dictionary_set_long(m, GAVL_META_STREAM_DURATION, duration);
    }
  if(f->total_bytes > 0)
    gavl_dictionary_set_long(m, GAVL_META_STREAM_RAWSIZE, f->total_bytes);

  if(f->size_min > 0)
    gavl_dictionary_set_long(m, GAVL_META_STREAM_PACKET_SIZE_MIN, f->size_min);
  if(f->size_max > 0)
    gavl_dictionary_set_long(m, GAVL_META_STREAM_PACKET_SIZE_MAX, f->size_max);
  if(f->duration_min > 0)
    gavl_dictionary_set_long(m, GAVL_META_STREAM_PACKET_DURATION_MIN, f->duration_min);
  if(f->duration_max > 0)
    gavl_dictionary_set_long(m, GAVL_META_STREAM_PACKET_DURATION_MAX, f->duration_max);
  
  }

static void calc_bitrate(gavf_stream_stats_t * f, int timescale,
                         gavl_compression_info_t * ci,
                         gavl_dictionary_t * m)
  {
  if((!ci || (ci->bitrate <= 0)) && (f->total_bytes > 0) && (f->pts_end > f->pts_start))
    {
    double avg_rate =
      (double)(f->total_bytes) / 
      (gavl_time_to_seconds(gavl_time_unscale(timescale,
                                              f->pts_end-f->pts_start)) * 125.0);
    gavl_dictionary_set_float(m, GAVL_META_AVG_BITRATE, avg_rate);
    }
  }
                         

void gavf_stream_stats_apply_audio(gavf_stream_stats_t * f, 
                                   const gavl_audio_format_t * fmt,
                                   gavl_compression_info_t * ci,
                                   gavl_dictionary_t * m)
  {
  if(fmt)
    calc_bitrate(f, fmt->samplerate, ci, m);
  gavf_stream_stats_apply_generic(f, ci, m);
  }

void gavf_stream_stats_apply_video(gavf_stream_stats_t * f, 
                                   gavl_video_format_t * fmt,
                                   gavl_compression_info_t * ci,
                                   gavl_dictionary_t * m)
  {
  if(fmt)
    calc_bitrate(f, fmt->timescale, ci, m);
  
  gavf_stream_stats_apply_generic(f, ci, m);
  
  if(fmt && (fmt->framerate_mode == GAVL_FRAMERATE_VARIABLE))
    {
    if((f->duration_min > 0) && (f->duration_min == f->duration_max))
      {
      /* Detect constant framerate */
      fmt->framerate_mode = GAVL_FRAMERATE_CONSTANT;
      fmt->frame_duration = f->duration_min;
      }
    else if((f->total_packets > 0) && (f->pts_end > f->pts_start))
      {
      double avg_rate =
        (double)f->total_packets / 
        gavl_time_to_seconds(gavl_time_unscale(fmt->timescale,
                                               f->pts_end-f->pts_start));
      
      gavl_dictionary_set_float(m, GAVL_META_AVG_FRAMERATE, avg_rate);
      }
    }
  }

void gavf_stream_stats_apply_subtitle(gavf_stream_stats_t * f, 
                                      gavl_dictionary_t * m)
  {
  gavf_stream_stats_apply_generic(f, NULL, m);
  }

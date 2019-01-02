#include <stdlib.h>
#include <string.h>

#include <config.h>

#include <gavfprivate.h>
#include <metatags.h>

// #define DUMP_EOF

static struct
  {
  gavl_stream_type_t type;
  const char * name;
  }
stream_types[] =
  {
    { GAVL_STREAM_AUDIO, "audio" },
    { GAVL_STREAM_VIDEO, "video" },
    { GAVL_STREAM_TEXT,  "text"  },
    { GAVL_STREAM_OVERLAY,  "overlay" },
    { GAVL_STREAM_MSG,      "msg"  },
  };

GAVL_PUBLIC
const char * gavf_stream_type_name(gavl_stream_type_t t)
  {
  int i;
  for(i = 0; i < sizeof(stream_types)/sizeof(stream_types[0]); i++)
    {
    if(stream_types[i].type == t)
      return stream_types[i].name;
    }
  return NULL;
  }

gavf_options_t * gavf_get_options(gavf_t * g)
  {
  return &g->opt;
  }

/* Extensions */

int gavf_extension_header_read(gavf_io_t * io, gavl_extension_header_t * eh)
  {
  if(!gavf_io_read_uint32v(io, &eh->key) ||
     !gavf_io_read_uint32v(io, &eh->len))
    return 0;
  return 1;
  }

int gavf_extension_write(gavf_io_t * io, uint32_t key, uint32_t len,
                         uint8_t * data)
  {
  if(!gavf_io_write_uint32v(io, key) ||
     !gavf_io_write_uint32v(io, len) ||
     (gavf_io_write_data(io, data, len) < len))
    return 0;
  return 1;
  }

/*
 */

static int write_sync_header(gavf_t * g, int stream, const gavl_packet_t * p)
  {
  int i;
  gavf_stream_t * s;

  
  for(i = 0; i < g->ph.num_streams; i++)
    {
    s = &g->streams[i];

    if(i == stream)
      {
      g->sync_pts[i] = p->pts;
      s->packets_since_sync = 0;
      }
    else if((stream >= 0) &&
            (s->h->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES))
      {
      gavl_packet_t * p;
      p = gavf_packet_buffer_peek_read(s->pb);
      if(!p && !(p->flags & GAVL_PACKET_KEYFRAME))
        g->sync_pts[i] = GAVL_TIME_UNDEFINED;
      else
        g->sync_pts[i] = p->pts;
      }
    else if((g->streams[i].flags & STREAM_FLAG_DISCONTINUOUS) &&
            (g->streams[i].next_sync_pts == GAVL_TIME_UNDEFINED))
      {
      g->sync_pts[i] = 0;
      s->packets_since_sync = 0;
      }
    else
      {
      g->sync_pts[i] = g->streams[i].next_sync_pts;
      s->packets_since_sync = 0;
      }
    }

  /* Update sync index */
  if(g->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    {
    gavf_sync_index_add(&g->si, g->io->position,
                        g->sync_pts);
    }

  /* If that's the first sync header, update file index */
  if(!g->first_sync_pos)
    {
    /* Write GAVF_TAG_PACKETS tag */
    
    g->first_sync_pos = g->io->position;
  
    }
  if(!gavf_io_cb(g->io, GAVF_IO_CB_SYNC_HEADER_START, g->sync_pts))
    return 0;
  /* Write the sync header */
  if(gavf_io_write_data(g->io, (uint8_t*)GAVF_TAG_SYNC_HEADER, 8) < 8)
    return 0;
#if 0
  fprintf(stderr, "Write sync header\n");
#endif
  
  for(i = 0; i < g->ph.num_streams; i++)
    {
#if 0
    fprintf(stderr, "PTS[%d]: %"PRId64"\n", i, g->sync_pts[i]);
#endif
    if(!gavf_io_write_int64v(g->io, g->sync_pts[i]))
      return 0;
    }

  /* Update last sync pts */
  for(i = 0; i < g->ph.num_streams; i++)
    {
    if(g->sync_pts[i] != GAVL_TIME_UNDEFINED)
      g->streams[i].last_sync_pts = g->sync_pts[i];
    }
  if(!gavf_io_flush(g->io))
    return 0;
  
  if(!gavf_io_cb(g->io, GAVF_IO_CB_SYNC_HEADER_END, g->sync_pts))
    return 0;
  return 1;
  }

static gavl_sink_status_t
write_packet(gavf_t * g, int stream, const gavl_packet_t * p)
  {
  gavl_time_t pts;
  int write_sync = 0;
  gavf_stream_t * s = &g->streams[stream];
#if 0
  fprintf(stderr, "write_packet %d\n", s->h->id);

  if(s->h->id == 5)
    {
    fprintf(stderr, "write_packet %p\n", p);
    gavl_packet_dump(p);
    }
#endif
  pts = gavl_time_unscale(s->timescale, p->pts);
  
  /* Decide whether to write a sync header */
  if(!g->first_sync_pos)
    write_sync = 1;
  else if(g->sync_distance)
    {
    if(pts - g->last_sync_time > g->sync_distance)
      write_sync = 1;
    }
  else
    {
    if((s->h->type == GAVL_STREAM_VIDEO) &&
       (s->h->ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES) &&
       (p->flags & GAVL_PACKET_KEYFRAME) &&
       s->packets_since_sync)
      write_sync = 1;
    }

  if(write_sync)
    {
    if(!write_sync_header(g, stream, p))
      return GAVL_SINK_ERROR;
    if(g->sync_distance)
      g->last_sync_time = pts;
    }
  
  // If a stream has B-frames, this won't be correct
  // for the next sync timestamp (it will be taken from the
  // packet pts in write_sync_header)
  
  if(s->next_sync_pts < p->pts + p->duration)
    s->next_sync_pts = p->pts + p->duration;

  /* Update packet index */
  
  if(g->opt.flags & GAVF_OPT_FLAG_PACKET_INDEX)
    {
    gavf_packet_index_add(&g->pi,
                          s->h->id, p->flags, g->io->position,
                          p->pts);
    }

  if(!gavf_io_cb(g->io, GAVF_IO_CB_PACKET_START, p))
    return GAVL_SINK_ERROR;
  
  if((gavf_io_write_data(g->io,
                         (const uint8_t*)GAVF_TAG_PACKET_HEADER, 1) < 1) ||
     (!gavf_io_write_uint32v(g->io, s->h->id)))
    return GAVL_SINK_ERROR;

  if(!gavf_write_gavl_packet(g->io, g->pkt_io, s->packet_duration, s->packet_flags, s->last_sync_pts, p))
    return GAVL_SINK_ERROR;

  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", s->h->id);
    gavl_packet_dump(p);
    }
  
  s->packets_since_sync++;

  if(!gavf_io_flush(g->io))
    return GAVL_SINK_ERROR;

  if(!gavf_io_cb(g->io, GAVF_IO_CB_PACKET_END, p))
    return GAVL_SINK_ERROR;
  
  return GAVL_SINK_OK;
  }

int gavf_write_gavl_packet(gavf_io_t * io, gavf_io_t * hdr_io, int packet_duration,
                           int packet_flags, int64_t last_sync_pts,
                           const gavl_packet_t * p)
  {
  gavl_buffer_t * buf;
  buf = gavf_io_buf_get(hdr_io);
  gavf_io_buf_reset(hdr_io);

  if(!gavf_write_gavl_packet_header(hdr_io, packet_duration, packet_flags, last_sync_pts, p) ||
     !gavf_io_write_uint32v(io, buf->len + p->data_len) ||
     (gavf_io_write_data(io, buf->buf, buf->len) < buf->len) ||
     (gavf_io_write_data(io, p->data, p->data_len) < p->data_len))
    return 0;
  return 1;
  }


/*
 *  Flush packets. s is the stream of the last packet written.
 *  If s is NULL, flush all streams
 */

gavl_sink_status_t gavf_flush_packets(gavf_t * g, gavf_stream_t * s)
  {
  int i;
  gavl_packet_t * p;
  int min_index;
  gavl_time_t min_time;
  gavl_time_t test_time;
  gavf_stream_t * ws;
  gavl_sink_status_t st;

  //  fprintf(stderr, "flush_packets\n");
  
  if(!g->first_sync_pos)
    {
    for(i = 0; i < g->ph.num_streams; i++)
      {
      if(!(g->streams[i].flags & STREAM_FLAG_DISCONTINUOUS) &&
         (g->streams[i].h->stats.pts_start == GAVL_TIME_UNDEFINED))
        return GAVL_SINK_OK;
      }
    }

  /* Flush discontinuous streams */
  for(i = 0; i < g->ph.num_streams; i++)
    {
    ws = &g->streams[i];
    if(ws->flags & STREAM_FLAG_DISCONTINUOUS)
      {
      while((p = gavf_packet_buffer_get_read(ws->pb)))
        {
        if((st = write_packet(g, i, p)) != GAVL_SINK_OK)
          return st;
        }
      }
    }
  
  /* Flush as many packets as possible */
  while(1)
    {
    /*
     *  1. Find stream we need to output now
     */

    min_index = -1;
    min_time = GAVL_TIME_UNDEFINED;
    for(i = 0; i < g->ph.num_streams; i++)
      {
      ws = &g->streams[i];

      if(ws->flags & STREAM_FLAG_DISCONTINUOUS)
        continue;
      
      test_time =
        gavf_packet_buffer_get_min_pts(ws->pb);
      
      if(test_time != GAVL_TIME_UNDEFINED)
        {
        if((min_time == GAVL_TIME_UNDEFINED) ||
           (test_time < min_time))
          {
          min_time = test_time;
          min_index = i;
          }
        }
      else
        {
        if(!(ws->flags & STREAM_FLAG_DISCONTINUOUS) &&
           s && (g->encoding_mode == ENC_INTERLEAVE))
          {
          /* Some streams without packets: stop here */
          return GAVL_SINK_OK;
          }
        }
      }

    /*
     *  2. Exit if we are done
     */
    if(min_index < 0)
      return GAVL_SINK_OK;

    /*
     *  3. Output packet
     */

    ws = &g->streams[min_index];
    p = gavf_packet_buffer_get_read(ws->pb);

    if((st = write_packet(g, min_index, p)) != GAVL_SINK_OK)
      return st;
    
    /*
     *  4. If we have B-frames, output the complete Mini-GOP
     */
    
    if(ws->h->type == GAVL_STREAM_VIDEO)
      {
      while(1)
        {
        p = gavf_packet_buffer_peek_read(ws->pb);
        if(p && ((p->flags & GAVL_PACKET_TYPE_MASK) == GAVL_PACKET_TYPE_B))
          {
          if((st = write_packet(g, min_index, p)) != GAVL_SINK_OK)
            return st;
          }
        else
          break;
        }
      }

    /* Continue */
    
    }
  
  }

static int get_audio_sample_size(const gavl_audio_format_t * fmt,
                                 const gavl_compression_info_t * ci)
  {
  if(!ci || (ci->id == GAVL_CODEC_ID_NONE))
    return gavl_bytes_per_sample(fmt->sample_format);
  else
    return gavl_compression_get_sample_size(ci->id);
  }

int gavf_get_max_audio_packet_size(const gavl_audio_format_t * fmt,
                                   const gavl_compression_info_t * ci)
  {
  int sample_size = 0;

  if(ci && ci->max_packet_size)
    return ci->max_packet_size;

  sample_size =
    get_audio_sample_size(fmt, ci);
  
  return fmt->samples_per_frame * fmt->num_channels * sample_size;
  }


static void set_implicit_stream_fields(gavf_stream_t * s)
  {
  const char * var;

  var = gavl_compression_get_mimetype(&s->h->ci);
  if(var)
    gavl_dictionary_set_string(&s->h->m, GAVL_META_MIMETYPE, var);
  
  var = gavl_compression_get_long_name(s->h->ci.id);
  if(var)
    gavl_dictionary_set_string(&s->h->m, GAVL_META_FORMAT, var);

  if(s->h->ci.bitrate)
    gavl_dictionary_set_int(&s->h->m, GAVL_META_BITRATE, s->h->ci.bitrate);
  }

/* Streams */

static void gavf_stream_init_audio(gavf_t * g, gavf_stream_t * s)
  {
  
  int sample_size;
  s->timescale = s->h->format.audio.samplerate;
  
  s->h->ci.max_packet_size =
    gavf_get_max_audio_packet_size(&s->h->format.audio, &s->h->ci);

  sample_size =
    get_audio_sample_size(&s->h->format.audio, &s->h->ci);
  
  /* Figure out the packet duration */
  if(gavl_compression_constant_frame_samples(s->h->ci.id) ||
     sample_size)
    s->packet_duration = s->h->format.audio.samples_per_frame;
  else
    s->packet_flags |= GAVF_PACKET_WRITE_DURATION;
  if(g->wr)
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    if(s->h->ci.id == GAVL_CODEC_ID_NONE)
      gavl_dictionary_set_string_endian(&s->h->m);
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);

    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

GAVL_PUBLIC 
int gavf_get_max_video_packet_size(const gavl_video_format_t * fmt,
                                   const gavl_compression_info_t * ci)
  {
  if(ci->max_packet_size)
    return ci->max_packet_size;
  if(ci->id == GAVL_CODEC_ID_NONE)
    return gavl_video_format_get_image_size(fmt);
  return 0;
  }


static void gavf_stream_init_video(gavf_t * g, gavf_stream_t * s,
                                   int num_streams)
  {
  s->timescale = s->h->format.video.timescale;

  if((s->h->ci.flags & GAVL_COMPRESSION_HAS_B_FRAMES) ||
     (s->h->type == GAVL_STREAM_OVERLAY))
    s->packet_flags |= GAVF_PACKET_WRITE_PTS;
  
  if(s->h->ci.flags & GAVL_COMPRESSION_HAS_P_FRAMES)
    g->sync_distance = 0;
  
  if(s->h->format.video.framerate_mode == GAVL_FRAMERATE_CONSTANT)
    s->packet_duration = s->h->format.video.frame_duration;
  else
    s->packet_flags |= GAVF_PACKET_WRITE_DURATION;
  
  if(((s->h->format.video.interlace_mode == GAVL_INTERLACE_MIXED) ||
      (s->h->format.video.interlace_mode == GAVL_INTERLACE_MIXED_TOP) ||
      (s->h->format.video.interlace_mode == GAVL_INTERLACE_MIXED_BOTTOM)) &&
     (s->h->ci.id == GAVL_CODEC_ID_NONE))
    s->packet_flags |= GAVF_PACKET_WRITE_INTERLACE;

  if(s->h->ci.flags & GAVL_COMPRESSION_HAS_FIELD_PICTURES)
    s->packet_flags |= GAVF_PACKET_WRITE_FIELD2;
    
  
  if(num_streams > 1)
    {
    if((s->h->format.video.framerate_mode == GAVL_FRAMERATE_STILL) ||
       (s->h->type == GAVL_STREAM_OVERLAY))
      s->flags |= STREAM_FLAG_DISCONTINUOUS;
    }
  
  s->h->ci.max_packet_size =
    gavf_get_max_video_packet_size(&s->h->format.video,
                                   &s->h->ci);
  
  if(g->wr)
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    if(s->h->ci.id == GAVL_CODEC_ID_NONE)
      gavl_dictionary_set_string_endian(&s->h->m);
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);
    
    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

static void gavf_stream_init_text(gavf_t * g, gavf_stream_t * s,
                                  int num_streams)
  {
  gavl_dictionary_get_int(&s->h->m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &s->timescale);
  
  s->packet_flags |=
    (GAVF_PACKET_WRITE_PTS|GAVF_PACKET_WRITE_DURATION);

  if(num_streams > 1)
    s->flags |= STREAM_FLAG_DISCONTINUOUS;
  
  if(g->wr)
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);

    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

static void gavf_stream_init_msg(gavf_t * g, gavf_stream_t * s,
                                 int num_streams)
  {
  s->packet_flags |= (GAVF_PACKET_WRITE_PTS|GAVF_PACKET_WRITE_DURATION);

  //  if(num_streams > 1)
  //    s->flags |= STREAM_FLAG_DISCONTINUOUS;
  
  if(g->wr)
    {
    /* Create packet sink */
    gavf_stream_create_packet_sink(g, s);
    }
  else
    {
    /* Create packet source */
    gavf_stream_create_packet_src(g, s);

    /* Set redundant metadata fields */
    set_implicit_stream_fields(s);
    }
  }

int gavf_stream_get_timescale(const gavf_stream_header_t * sh)
  {
  switch(sh->type)
    {
    case GAVL_STREAM_AUDIO:
      return sh->format.audio.samplerate;
      break;
    case GAVL_STREAM_VIDEO:
    case GAVL_STREAM_OVERLAY:
      return sh->format.video.timescale;
      break;
    case GAVL_STREAM_TEXT:
      {
      int ret = 0;
      gavl_dictionary_get_int(&sh->m, GAVL_META_STREAM_SAMPLE_TIMESCALE, &ret);
      return ret;
      }
      break;
    case GAVL_STREAM_NONE:
    case GAVL_STREAM_MSG:
      break;
    }
  return 0;
  }

static void init_streams(gavf_t * g)
  {
  int i;
  
  gavf_sync_index_init(&g->si, g->ph.num_streams);
  
  g->streams = calloc(g->ph.num_streams, sizeof(*g->streams));
  g->sync_pts = calloc(g->ph.num_streams, sizeof(*g->sync_pts));

  for(i = 0; i < g->ph.num_streams; i++)
    {
    g->sync_pts[i] = GAVL_TIME_UNDEFINED;

    g->streams[i].next_sync_pts = GAVL_TIME_UNDEFINED;
    g->streams[i].last_sync_pts = GAVL_TIME_UNDEFINED;
    
    g->streams[i].h = &g->ph.streams[i];
    g->streams[i].g = g;
    
    switch(g->streams[i].h->type)
      {
      case GAVL_STREAM_AUDIO:
        gavf_stream_init_audio(g, &g->streams[i]);
        break;
      case GAVL_STREAM_VIDEO:
      case GAVL_STREAM_OVERLAY:
        gavf_stream_init_video(g, &g->streams[i], g->ph.num_streams);
        break;
      case GAVL_STREAM_TEXT:
        gavf_stream_init_text(g, &g->streams[i], g->ph.num_streams);
        break;
      case GAVL_STREAM_MSG:
        gavf_stream_init_msg(g, &g->streams[i], g->ph.num_streams);
        break;
      case GAVL_STREAM_NONE:
        break;
      }
    g->streams[i].pb =
      gavf_packet_buffer_create(g->streams[i].timescale);
    }

  }

static void gavf_stream_free(gavf_stream_t * s)
  {
  if(s->asrc)
    gavl_audio_source_destroy(s->asrc);
  if(s->vsrc)
    gavl_video_source_destroy(s->vsrc);
  if(s->psrc)
    gavl_packet_source_destroy(s->psrc);

  if(s->asink)
    gavl_audio_sink_destroy(s->asink);
  if(s->vsink)
    gavl_video_sink_destroy(s->vsink);
  if(s->psink)
    gavl_packet_sink_destroy(s->psink);
  if(s->pb)
    gavf_packet_buffer_destroy(s->pb);
  
  if(s->aframe)
    {
    gavl_audio_frame_null(s->aframe);
    gavl_audio_frame_destroy(s->aframe);
    }
  if(s->vframe)
    {
    gavl_video_frame_null(s->vframe);
    gavl_video_frame_destroy(s->vframe);
    }
  if(s->dsp)
    gavl_dsp_context_destroy(s->dsp);
  }


gavf_t * gavf_create()
  {
  gavf_t * ret;
  ret = calloc(1, sizeof(*ret));
  ret->msg_id = -1;
  return ret;
  }

/* Read support */

static int handle_chunk(gavf_t * g, char * sig)
  {
  if(!strncmp(sig, GAVF_TAG_PROGRAM_HEADER, 8))
    {
    if(!gavf_program_header_read(g->io, &g->ph))
      return 0;
    
    init_streams(g);
    }
  else if(!strncmp(sig, GAVF_TAG_SYNC_HEADER, 8))
    {
    g->first_sync_pos = g->io->position;
    }  
  return 1;
  }

static int read_sync_header(gavf_t * g)
  {
  int i;
  /* Read sync header */
#if 0
  fprintf(stderr, "Read sync header\n");
#endif
  for(i = 0; i < g->ph.num_streams; i++)
    {
    if(!gavf_io_read_int64v(g->io, &g->sync_pts[i]))
      return 0;
#if 0
    fprintf(stderr, "PTS[%d]: %ld\n", i, g->sync_pts[i]);
#endif
    if(g->sync_pts[i] != GAVL_TIME_UNDEFINED)
      {
      g->streams[i].last_sync_pts = g->sync_pts[i];
      g->streams[i].next_pts = g->sync_pts[i];
      }
    }

  if(!gavf_io_cb(g->io, GAVF_IO_CB_SYNC_HEADER_END, g->sync_pts))
    return 0;
  
  return 1;
  }

static void calc_pts_offset(gavf_t * g)
  {
  gavl_time_t min_time= GAVL_TIME_UNDEFINED;
  gavl_time_t test_time;
  int i, min_index = -1;
  int64_t off;
  int scale;
  
  for(i = 0; i < g->ph.num_streams; i++)
    {
    if(g->sync_pts[i] != GAVL_TIME_UNDEFINED)
      {
      test_time =
        gavl_time_unscale(g->streams[i].timescale,
                          g->sync_pts[i]+g->streams[i].h->ci.pre_skip);
      if((min_time == GAVL_TIME_UNDEFINED) ||
         (test_time < min_time))
        {
        min_index = i;
        min_time = test_time;
        }
      }
    }

  if(min_index < 0)
    return;

  off = -(g->sync_pts[min_index]+g->streams[min_index].h->ci.pre_skip);
  scale = g->streams[min_index].timescale;
  
  g->streams[min_index].pts_offset = off;
  
  for(i = 0; i < g->ph.num_streams; i++)
    {
    if(i != min_index)
      {
      g->streams[i].pts_offset =
        gavl_time_rescale(scale,
                          g->streams[i].timescale,
                          off);
      }
#if 0
    fprintf(stderr, "PTS offset %d: %"PRId64"\n", i,
            g->streams[i].pts_offset);
#endif
    }
  }

int gavf_open_read(gavf_t * g, gavf_io_t * io)
  {
  gavl_time_t duration;
  char sig[8];
  
  g->io = io;

  g->opt.flags &= ~(GAVF_OPT_FLAG_SYNC_INDEX|
                    GAVF_OPT_FLAG_PACKET_INDEX);
  
  /* Read up to the first sync header */

  while(1)
    {
    if(gavf_io_read_data(g->io, (uint8_t*)sig, 8) < 8)
      return 0;
    
    if(!handle_chunk(g, sig))
      return 0;
    
    if(g->first_sync_pos > 0)
      break;
    }

  gavf_footer_check(g);

  /* Dump stuff */
  
  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_HEADERS)
    {
    gavf_program_header_dump(&g->ph);
    }
  
  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_INDICES)
    {
    if((g->opt.flags & GAVF_OPT_FLAG_DUMP_INDICES) ||
       (g->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX))
      gavf_sync_index_dump(&g->si);
    if((g->opt.flags & GAVF_OPT_FLAG_DUMP_INDICES) ||
       (g->opt.flags & GAVF_OPT_FLAG_PACKET_INDEX))
      gavf_packet_index_dump(&g->pi);
    }
  
  gavf_reset(g);

  if(!(g->opt.flags & GAVF_OPT_FLAG_ORIG_PTS))
    calc_pts_offset(g);

  if(gavf_program_header_get_duration(&g->ph, NULL, &duration))
    gavl_dictionary_set_long(&g->ph.m, GAVL_META_APPROX_DURATION, duration);
  
  return 1;
  }

int gavf_reset(gavf_t * g)
  {
  if(g->first_sync_pos != g->io->position)
    {
    if(g->io->seek_func)
      gavf_io_seek(g->io, g->first_sync_pos, SEEK_SET);
    else
      return 0;
    }
  
  g->have_pkt_header = 0;
  
  if(!read_sync_header(g))
    return 0;
  return 1;
  }

gavf_program_header_t * gavf_get_program_header(gavf_t * g)
  {
  return &g->ph;
  }

const gavf_packet_header_t * gavf_packet_read_header(gavf_t * g)
  {
  char c[8];

#ifdef DUMP_EOF
  fprintf(stderr, "gavf_packet_read_header\n");
#endif
  
  if(g->eof)
    {
#ifdef DUMP_EOF
    fprintf(stderr, "EOF 0\n");
#endif
    return NULL;
    }
  
  while(1)
    {
    if(!gavf_io_read_data(g->io, (uint8_t*)c, 1))
      {
#ifdef DUMP_EOF
      fprintf(stderr, "EOF 1\n");
#endif
      goto got_eof;
      }
    if(c[0] == GAVF_TAG_PACKET_HEADER_C)
      {
      gavf_stream_t * s;
      /* Got new packet */
      if(!gavf_io_read_uint32v(g->io, &g->pkthdr.stream_id))
        {
#ifdef DUMP_EOF
        fprintf(stderr, "EOF 2\n");
#endif
        goto got_eof;
        }
      
      /* Check whether to skip this stream */
      if((s = gavf_find_stream_by_id(g, g->pkthdr.stream_id)) &&
         (s->flags & STREAM_FLAG_SKIP))
        {
        if(s->skip_func)
          s->skip_func(g, s->h, s->skip_priv);
        else
          {
          gavf_packet_skip(g);
          if(!gavf_io_cb(g->io, GAVF_IO_CB_PACKET_END, NULL))
            {
#ifdef DUMP_EOF
            fprintf(stderr, "EOF 3\n");
#endif
            goto got_eof;
            }
          }
        }
      else
        {
        g->have_pkt_header = 1;
        return &g->pkthdr;
        }
      }
    else
      {
      if(gavf_io_read_data(g->io, (uint8_t*)&c[1], 7) < 7)
        {
#ifdef DUMP_EOF
        fprintf(stderr, "EOF 6\n");
#endif
        goto got_eof;
        }

      if(!strncmp(c, GAVF_TAG_SYNC_HEADER, 8))
        {
        if(!read_sync_header(g))
          {
#ifdef DUMP_EOF
          fprintf(stderr, "EOF 7\n");
#endif
          goto got_eof;
          }
        }
      else
        {
#ifdef DUMP_EOF
       fprintf(stderr, "EOF 8 %8s\n", c);
#endif
        goto got_eof;
        }
      }
    }
  
  got_eof:
  g->eof = 1;
  return NULL;
  }

int gavf_put_message(void * data, gavl_msg_t * m)
  {
  gavl_packet_sink_t * sink;
  gavl_packet_t * p;
  gavl_msg_t msg;
  gavf_t * g = data;

  gavl_dprintf("Got inline message\n");
  gavl_msg_dump(m, 2);gavl_dprintf("\n");
  
  if(g->msg_id < 0)
    {
    fprintf(stderr, "BUG: Metadata update without metadata stream\n");
    return 0;
    }

  sink = gavf_get_packet_sink(g, g->msg_id);
  p = gavl_packet_sink_get_packet(sink);

  gavl_msg_init(&msg);
  gavf_msg_to_packet(m, p);
  gavl_packet_sink_put_packet(sink, p);
  
  return 1;
  }

void gavf_packet_skip(gavf_t * g)
  {
  uint32_t len;
  if(!gavf_io_read_uint32v(g->io, &len))
    return;
  gavf_io_skip(g->io, len);
  }

int gavf_packet_read_packet(gavf_t * g, gavl_packet_t * p)
  {
  gavf_stream_t * s = NULL;

  g->have_pkt_header = 0;

  if(!(s = gavf_find_stream_by_id(g, g->pkthdr.stream_id)))
    return 0;
  
  
  if(!gavf_read_gavl_packet(g->io, s->packet_duration, s->packet_flags, s->last_sync_pts, &s->next_pts, s->pts_offset, p))
    return 0;

  p->id = s->h->id;

  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", g->pkthdr.stream_id);
    gavl_packet_dump(p);
    }

  
  return 1;
  }

const int64_t * gavf_first_pts(gavf_t * gavf)
  {
  if(gavf->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    return gavf->si.entries[0].pts;
  else
    return NULL;
  }

/* Get the last PTS of the streams */

const int64_t * gavf_end_pts(gavf_t * gavf)
  {
  if(gavf->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX)
    return gavf->si.entries[gavf->si.num_entries - 1].pts;
  else
    return NULL;
  }

/* Seek to a specific time. Return the sync timestamps of
   all streams at the current position */

const int64_t * gavf_seek(gavf_t * g, int64_t time, int scale)
  {
  int stream = 0;
  int64_t index_position;
  int64_t time_scaled;
  int done = 0;
  
  if(!(g->opt.flags & GAVF_OPT_FLAG_SYNC_INDEX))
    return NULL;

  g->eof = 0;
  
  index_position = g->si.num_entries - 1;
  while(!done)
    {
    /* Find next continuous stream */
    while(g->streams[stream].flags & STREAM_FLAG_DISCONTINUOUS)
      {
      stream++;
      if(stream >= g->ph.num_streams)
        {
        done = 1;
        break;
        }
      }

    if(done)
      break;
    
    time_scaled = gavl_time_rescale(scale, g->streams[stream].timescale, time);
    
    /* Descrease index pointer until we are before this time */
    
    while(g->si.entries[index_position].pts[stream] > time_scaled)
      {
      if(!index_position)
        {
        done = 1;
        break;
        }
      index_position--;
      }
    stream++;

    if(stream >= g->ph.num_streams)
      {
      done = 1;
      break;
      }
    }
  
  /* Seek to the positon */
  gavf_io_seek(g->io, g->si.entries[index_position].pos, SEEK_SET);

  //  fprintf(stderr, "Index position: %ld, file position: %ld\n", index_position,
  //          g->si.entries[index_position].pos);

  return g->si.entries[index_position].pts;
  }


/* Write support */

int gavf_open_write(gavf_t * g, gavf_io_t * io,
                    const gavl_dictionary_t * m)
  {
  g->io = io;
  g->wr = 1;
  /* Initialize packet buffer */

  g->pkt_io = gavf_io_create_buf_write();

  if(m)
    gavl_dictionary_copy(&g->ph.m, m);
  
  return 1;
  }

int gavf_add_audio_stream(gavf_t * g,
                          const gavl_compression_info_t * ci,
                          const gavl_audio_format_t * format,
                          const gavl_dictionary_t * m)
  {
  return gavf_program_header_add_audio_stream(&g->ph, ci, format, m);
  }


int gavf_add_video_stream(gavf_t * g,
                          const gavl_compression_info_t * ci,
                          const gavl_video_format_t * format,
                          const gavl_dictionary_t * m)
  {
  return gavf_program_header_add_video_stream(&g->ph, ci, format, m);
  }

int gavf_add_text_stream(gavf_t * g,
                         uint32_t timescale,
                         const gavl_dictionary_t * m)
  {
  return gavf_program_header_add_text_stream(&g->ph, timescale, m);
  }


int gavf_add_overlay_stream(gavf_t * g,
                            const gavl_compression_info_t * ci,
                            const gavl_video_format_t * format,
                            const gavl_dictionary_t * m)
  {
  return gavf_program_header_add_overlay_stream(&g->ph, ci, format, m);
  }

int gavf_add_msg_stream(gavf_t * g, const gavl_dictionary_t * m)
  {
  if(g->msg_id >= 0)
    return g->msg_id;
  g->msg_id = gavf_program_header_add_msg_stream(&g->ph, m);
  return g->msg_id;
  }

void gavf_add_streams(gavf_t * g, const gavf_program_header_t * ph)
  {
  gavf_program_header_copy(&g->ph, ph);
  }

int gavf_start(gavf_t * g)
  {
  if(!g->wr || g->streams)
    return 1;
  
  g->sync_distance = g->opt.sync_distance;
  
  init_streams(g);
  
  gavf_footer_init(&g->ph);
  
  if(g->ph.num_streams == 1)
    {
    g->encoding_mode = ENC_SYNCHRONOUS;
    g->final_encoding_mode = ENC_SYNCHRONOUS;
    }
  else
    {
    g->encoding_mode = ENC_STARTING;
    
    if(g->opt.flags & GAVF_OPT_FLAG_INTERLEAVE)
      g->final_encoding_mode = ENC_INTERLEAVE;
    else
      g->final_encoding_mode = ENC_SYNCHRONOUS;
    }
  
  gavl_metadata_delete_implicit_fields(&g->ph.m);
  gavl_dictionary_set_string(&g->ph.m, GAVL_META_SOFTWARE, PACKAGE"-"VERSION);
  
  if(g->opt.flags & GAVF_OPT_FLAG_DUMP_HEADERS)
    gavf_program_header_dump(&g->ph);
  
  if(!gavf_program_header_write(g->io, &g->ph))
    return 0;
  
  return 1;
  }


void gavf_video_frame_to_packet_metadata(const gavl_video_frame_t * frame,
                                         gavl_packet_t * pkt)
  {
  pkt->pts = frame->timestamp;
  pkt->duration = frame->duration;
  pkt->timecode = frame->timecode;
  pkt->interlace_mode = frame->interlace_mode;

  gavl_rectangle_i_copy(&pkt->src_rect, &frame->src_rect);
  pkt->dst_x = frame->dst_x;
  pkt->dst_y = frame->dst_y;
  }

/* LEGACY */
int gavf_write_video_frame(gavf_t * g,
                           int stream, gavl_video_frame_t * frame)
  {
  gavf_stream_t * s = &g->streams[stream];
  if(!s->vsink)
    return 0;
  return (gavl_video_sink_put_frame(s->vsink, frame) == GAVL_SINK_OK);
  }

void gavf_packet_to_video_frame(gavl_packet_t * p, gavl_video_frame_t * frame,
                                const gavl_video_format_t * format,
                                const gavl_dictionary_t * m,
                                gavl_dsp_context_t ** ctx)
  {
  frame->timestamp = p->pts;
  frame->duration = p->duration;

  frame->interlace_mode = p->interlace_mode;
  frame->timecode  = p->timecode;

  gavl_rectangle_i_copy(&frame->src_rect, &p->src_rect);
  frame->dst_x = p->dst_x;
  frame->dst_y = p->dst_y;
  
  frame->strides[0] = 0;
  gavl_video_frame_set_planes(frame, format, p->data);

  if(gavl_metadata_do_swap_endian(m))
    {
    if(!(*ctx))
      *ctx = gavl_dsp_context_create();
    gavl_dsp_video_frame_swap_endian(*ctx, frame, format);
    }
  }

static void get_overlay_format(const gavl_video_format_t * src,
                               gavl_video_format_t * dst,
                               const gavl_rectangle_i_t * src_rect)
  {
  gavl_video_format_copy(dst, src);
  dst->image_width  = src_rect->w + src_rect->x;
  dst->image_height = src_rect->h + src_rect->y;
  gavl_video_format_set_frame_size(dst, 0, 0);
  }

void gavf_packet_to_overlay(gavl_packet_t * p, gavl_video_frame_t * frame,
                            const gavl_video_format_t * format)
  {
  gavl_video_format_t copy_format;
  gavl_video_frame_t tmp_frame_src;
  
  frame->timestamp = p->pts;
  frame->duration  = p->duration;
  
  memset(&tmp_frame_src, 0, sizeof(tmp_frame_src));

  get_overlay_format(format, &copy_format, &p->src_rect);
  gavl_video_frame_set_planes(&tmp_frame_src, &copy_format, p->data);
  
  gavl_video_frame_copy(&copy_format, frame, &tmp_frame_src);

  gavl_rectangle_i_copy(&frame->src_rect, &p->src_rect);
  frame->dst_x = p->dst_x;
  frame->dst_y = p->dst_y;
  }

void gavf_overlay_to_packet(gavl_video_frame_t * frame, 
                            gavl_packet_t * p,
                            const gavl_video_format_t * format)
  {
  gavl_video_format_t copy_format;
  gavl_video_frame_t tmp_frame_src;
  gavl_video_frame_t tmp_frame_dst;
  int sub_h, sub_v; // Not necessary yet but well....
  gavl_rectangle_i_t rect;
  p->pts      = frame->timestamp;
  p->duration = frame->duration;
  
  memset(&tmp_frame_src, 0, sizeof(tmp_frame_src));
  memset(&tmp_frame_dst, 0, sizeof(tmp_frame_dst));
  gavl_pixelformat_chroma_sub(format->pixelformat, &sub_h, &sub_v);
  
  gavl_rectangle_i_copy(&p->src_rect, &frame->src_rect);

  /* Shift rectangles */

  p->src_rect.x = frame->src_rect.x % sub_h;
  p->src_rect.y = frame->src_rect.y % sub_v;
  
  get_overlay_format(format, &copy_format, &p->src_rect);

  rect.x = frame->src_rect.x - p->src_rect.x;
  rect.y = frame->src_rect.y - p->src_rect.y;
  rect.w = copy_format.image_width;
  rect.h = copy_format.image_height;
  
  gavl_video_frame_get_subframe(format->pixelformat,
                                frame,
                                &tmp_frame_src,
                                &rect);

  /* p->data is assumed to have the proper allocation already!! */
  gavl_video_frame_set_planes(&tmp_frame_dst, &copy_format, p->data);

  gavl_video_frame_copy(&copy_format, &tmp_frame_dst, &tmp_frame_src);
  
  p->dst_x = frame->dst_x;
  p->dst_y = frame->dst_y;
  p->data_len = gavl_video_format_get_image_size(&copy_format);
  }

void gavf_audio_frame_to_packet_metadata(const gavl_audio_frame_t * frame,
                                         gavl_packet_t * pkt)
  {
  pkt->pts = frame->timestamp;
  pkt->duration = frame->valid_samples;
  }

int gavf_write_audio_frame(gavf_t * g, int stream, gavl_audio_frame_t * frame)
  {
  gavf_stream_t * s;
  gavl_audio_frame_t * frame_internal;  
  
  s = &g->streams[stream];
  if(!s->asink)
    return 0;

  frame_internal = gavl_audio_sink_get_frame(s->asink);
  //  gavl_video_frame_copy(&s->h->format.video, frame_internal, frame);

  frame_internal->valid_samples =
    gavl_audio_frame_copy(&s->h->format.audio,
                          frame_internal,
                          frame,
                          0,
                          0,
                          frame->valid_samples,
                          s->h->format.audio.samples_per_frame);
  
  frame_internal->timestamp = frame->timestamp;
  return (gavl_audio_sink_put_frame(s->asink, frame_internal) == GAVL_SINK_OK);
  }

void gavf_packet_to_audio_frame(gavl_packet_t * p,
                                gavl_audio_frame_t * frame,
                                const gavl_audio_format_t * format,
                                const gavl_dictionary_t * m,
                                gavl_dsp_context_t ** ctx)
  {
  frame->valid_samples = p->duration;
  frame->timestamp = p->pts;
  gavl_audio_frame_set_channels(frame, format, p->data);

  if(gavl_metadata_do_swap_endian(m))
    {
    if(!(*ctx))
      *ctx = gavl_dsp_context_create();
    gavl_dsp_audio_frame_swap_endian(*ctx, frame, format);
    }

  }


/* Close */

void gavf_close(gavf_t * g)
  {
  int i;
  if(g->wr)
    {
    if(g->streams)
      {
      /* Flush packets if any */
      gavf_flush_packets(g, NULL);
    
      /* Append final sync header */
      write_sync_header(g, -1, NULL);
      }
    
    /* Write footer (might fail silently) */
    gavf_footer_write(g);
    }
  
  /* Free stuff */

  if(g->streams)
    {
    for(i = 0; i < g->ph.num_streams; i++)
      gavf_stream_free(&g->streams[i]);
    free(g->streams);
    }
  gavf_sync_index_free(&g->si);
  gavf_packet_index_free(&g->pi);
  gavf_program_header_free(&g->ph);

#if 0  
  if(g->write_vframe)
    {
    gavl_video_frame_null(g->write_vframe);
    gavl_video_frame_destroy(g->write_vframe);
    }

  if(g->write_aframe)
    {
    gavl_audio_frame_null(g->write_aframe);
    gavl_audio_frame_destroy(g->write_aframe);
    }
#endif
  
  if(g->sync_pts)
    free(g->sync_pts);
  
  gavl_packet_free(&g->write_pkt);

  if(g->pkt_io)
    gavf_io_destroy(g->pkt_io);
  
  free(g);
  }

gavl_packet_sink_t *
gavf_get_packet_sink(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;
  return s->psink;
  }

gavl_packet_source_t *
gavf_get_packet_source(gavf_t * g, uint32_t id)
  {
  gavf_stream_t * s;
  if(!(s = gavf_find_stream_by_id(g, id)))
    return NULL;
  return s->psrc;
  }

void gavf_stream_set_skip(gavf_t * g, uint32_t id,
                          gavf_stream_skip_func func, void * priv)
  {
  gavf_stream_t * s;
  if((s = gavf_find_stream_by_id(g, id)))
    {
    s->flags |= STREAM_FLAG_SKIP;
    s->skip_func = func;
    s->skip_priv = priv;
    }
  }

gavf_stream_t * gavf_find_stream_by_id(gavf_t * g, uint32_t id)
  {
  int i;
  for(i = 0; i < g->ph.num_streams; i++)
    {
    if(g->streams[i].h->id == id)
      return &g->streams[i];
    }
  return NULL;
  }


int gavf_get_num_streams(gavf_t * g, int type)
  {
  return gavf_program_header_get_num_streams(&g->ph, type);
  }

const gavf_stream_header_t * gavf_get_stream(gavf_t * g, int index,
                                             int type)
  {
  return gavf_program_header_get_stream(&g->ph, index, type);
  }

int gavf_chunk_is(const gavf_chunk_t * head, const char * eightcc)
  {
  if(!strncmp(head->eightcc, eightcc, 8))
    return 1;
  else
    return 0;
  }


static int align_write(gavf_io_t * io)
  {
  int rest;
  int64_t position;
  uint8_t buf[8] = { 0x00, 0x00, 0x00, 0x00, 
                     0x00, 0x00, 0x00, 0x00 };
  
  position = gavf_io_position(io);

  rest = position % 8;
  
  if(rest)
    {
    rest = 8 - rest;
    
    if(gavf_io_write_data(io, (const uint8_t*)buf, rest) < rest)
      return 0;

    gavf_io_flush(io);
    }
  return 1;
  }

static int align_read(gavf_io_t * io)
  {
  int rest;
  int64_t position;
  uint8_t buf[8];
  
  position = gavf_io_position(io);

  rest = position % 8;
  
  if(rest)
    {
    rest = 8 - rest;
    
    if(gavf_io_read_data(io, buf, rest) < rest)
      return 0;
    }
  return 1;
  }

int gavf_chunk_read_header(gavf_io_t * io, gavf_chunk_t * head)
  {
  /* Byte align (if not already aligned) */
  if(!align_read(io))
    return 0;
  
  if((gavf_io_read_data(io, (uint8_t*)head->eightcc, 8) < 8) ||
     !gavf_io_read_int64f(io, &head->len))
    return 0;
  /* Be strcmp friendly */
  head->eightcc[8] = 0x00;
  return 1;
  }


int gavf_chunk_start(gavf_io_t * io, gavf_chunk_t * head, const char * eightcc)
  {
  align_write(io);
  
  head->start = gavf_io_position(io);
  
  if((gavf_io_write_data(io, (const uint8_t*)eightcc, 8) < 8) ||
     !gavf_io_write_int64f(io, 0))
    return 0;
  return 1;
  }

int gavf_chunk_finish(gavf_io_t * io, gavf_chunk_t * head, int write_size)
  {
  int64_t position;
  int64_t size;
  
  /* Pad to multiple of 8 */

  position = gavf_io_position(io);
  
  if(write_size && gavf_io_can_seek(io))
    {
    size = (position - head->start) - 16;

    position = gavf_io_position(io);
    
    gavf_io_seek(io, head->start+8, SEEK_SET);
    gavf_io_write_int64f(io, size);
    gavf_io_seek(io, position, SEEK_SET);
    }
  return 1;
  }

gavf_io_t * gavf_chunk_start_io(gavf_io_t * io, gavf_chunk_t * head, const char * eightcc)
  {
  gavf_io_t * sub_io = gavf_io_create_mem_write();
  
  gavf_chunk_start(sub_io, head, eightcc);
  return sub_io;
  }
  

int gavf_chunk_finish_io(gavf_io_t * io, gavf_chunk_t * head, gavf_io_t * sub_io)
  {
  int len = 0;
  uint8_t * ret;
  
  gavf_chunk_finish(sub_io, head, 1);
  ret = gavf_io_mem_get_buf(sub_io, &len);
  gavf_io_destroy(sub_io);
  
  gavf_io_write_data(io, ret, len);
  gavf_io_flush(io);
  free(ret);

  return 1;
  }


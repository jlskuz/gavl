#include <gavfprivate.h>

/* Packet source */

static gavl_source_status_t
read_packet_func_nobuffer(void * priv, gavl_packet_t ** p)
  {
  gavf_stream_t * s = priv;

  if(!s->g->have_pkt_header && !gavf_packet_read_header(s->g))
    return GAVL_SOURCE_EOF;

  if(s->g->pkthdr.stream_id != s->h->id)
    return GAVL_SOURCE_AGAIN;
  
  s->g->have_pkt_header = 0;
  
  if(!gavf_read_gavl_packet(s->g->io, s->packet_duration, s->packet_flags, s->last_sync_pts, &s->next_pts, s->pts_offset, *p))
    return GAVL_SOURCE_EOF;

  (*p)->id = s->h->id;

  if(s->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
    {
    fprintf(stderr, "ID: %d ", s->g->pkthdr.stream_id);
    gavl_packet_dump(*p);
    }

  
  
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_packet_func_buffer_cont(void * priv, gavl_packet_t ** p)
  {
  gavl_packet_t * read_packet;
  gavf_stream_t * read_stream;
  gavf_stream_t * s = priv;
  gavf_t * g = s->g;
  
  while(!(*p = gavf_packet_buffer_get_read(s->pb)))
    {
    /* Read header */
    if(!g->have_pkt_header && !gavf_packet_read_header(g))
      {
      //      fprintf(stderr, "Have no header\n");
      return GAVL_SOURCE_EOF;
      }
    read_stream = gavf_find_stream_by_id(g, g->pkthdr.stream_id);
    if(!read_stream)
      {
      /* Should never happen */
      return GAVL_SOURCE_EOF;
      }
    
    read_packet = gavf_packet_buffer_get_write(read_stream->pb);

    if(!gavf_read_gavl_packet(s->g->io, read_stream->packet_duration, read_stream->packet_flags,
                              read_stream->last_sync_pts, &read_stream->next_pts, read_stream->pts_offset, read_packet))
      return GAVL_SOURCE_EOF;

    read_packet->id = read_stream->h->id;

    if(read_stream->g->opt.flags & GAVF_OPT_FLAG_DUMP_PACKETS)
      {
      fprintf(stderr, "ID: %d ", read_stream->g->pkthdr.stream_id);
      gavl_packet_dump(*p);
      }
    
    gavf_packet_buffer_done_write(read_stream->pb);
    //    fprintf(stderr, "Got packet id: %d\n", read_stream->h->id);
    //    gavl_packet_dump(read_packet);
    
    s->g->have_pkt_header = 0;
    }
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
read_packet_func_buffer_discont(void * priv, gavl_packet_t ** p)
  {
  gavf_stream_t * s = priv;

  if((*p = gavf_packet_buffer_get_read(s->pb)))
    return GAVL_SOURCE_OK;
  else if(s->g->eof)
    return GAVL_SOURCE_EOF;
  else
    return GAVL_SOURCE_AGAIN;
  }

void gavf_stream_create_packet_src(gavf_t * g, gavf_stream_t * s)
  {
  gavl_packet_source_func_t func;
  int flags;
  
  if(!(g->opt.flags & GAVF_OPT_FLAG_BUFFER_READ))
    {
    func = read_packet_func_nobuffer;
    flags = 0;
    }
  else
    {
    if(s->flags & STREAM_FLAG_DISCONTINUOUS)
      {
      func = read_packet_func_buffer_discont;
      flags = GAVL_SOURCE_SRC_ALLOC;
      }
    else
      {
      func = read_packet_func_buffer_cont;
      flags = GAVL_SOURCE_SRC_ALLOC;
      }
    }

  switch(s->h->type)
    {
    case GAVF_STREAM_AUDIO:
      s->psrc = gavl_packet_source_create_audio(func, s, flags,
                                                &s->h->ci, &s->h->format.audio);
      break;
    case GAVF_STREAM_OVERLAY:
    case GAVF_STREAM_VIDEO:
      s->psrc = gavl_packet_source_create_video(func, s, flags,
                                                &s->h->ci, &s->h->format.video);
      break;
    case GAVF_STREAM_TEXT:
      s->psrc = gavl_packet_source_create_text(func, s, flags,
                                               s->h->format.text.timescale);
      break;
    case GAVF_STREAM_NONE:
    case GAVF_STREAM_MSG:
      break;
    }
  }

/* Packet sink */

static gavl_sink_status_t
put_packet_func(void * priv, gavl_packet_t * p)
  {
  gavf_stream_t * s = priv;
  gavf_packet_buffer_done_write(s->pb);
  p->id = s->h->id;
  
  /* Update footer */
#if 0
  fprintf(stderr, "put packet %d\n", s->h->id);
  gavl_packet_dump(p);
#endif
  /* Fist packet */
  if(s->h->foot.pts_start == GAVL_TIME_UNDEFINED)
    s->next_sync_pts = p->pts;
  
  gavf_stream_footer_update(&s->h->foot, p);
  
  return gavf_flush_packets(s->g, s);
  
  // return gavf_write_packet(s->g, (int)(s - s->g->streams), p) ?
  //    GAVL_SINK_OK : GAVL_SINK_ERROR;
  }

static gavl_packet_t *
get_packet_func(void * priv)
  {
  gavf_stream_t * s = priv;
  return gavf_packet_buffer_get_write(s->pb);
  }

void gavf_stream_create_packet_sink(gavf_t * g, gavf_stream_t * s)
  {
  /* Create packet sink */
  s->psink = gavl_packet_sink_create(get_packet_func,
                                     put_packet_func, s);
  
  }

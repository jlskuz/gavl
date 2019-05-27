
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


#if 0

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

#endif

int gavf_footer_write(gavf_t * g)
  {
  int i;
  gavl_dictionary_t foot;
  gavl_value_t arr_val;
  gavl_array_t * arr;
  uint64_t footer_start_pos = g->io->position;

  gavf_chunk_t footer;
  //  gavf_chunk_t tail;
  
  gavf_chunk_start(g->io, &footer, GAVF_TAG_FOOTER);
  
  gavl_dictionary_init(&foot);

  gavl_value_init(&arr_val);
  arr = gavl_value_set_array(&arr_val);
  
  for(i = 0; i < g->num_streams; i++)
    {
    gavl_value_t stream_val;
    gavl_dictionary_t * stream;
    
    gavl_value_init(&stream_val);
    stream = gavl_value_set_dictionary(&stream_val);
    gavl_stream_set_stats(stream, &g->streams[i].stats);
    gavl_array_splice_val_nocopy(arr, -1, 0, &stream_val);
    }

  gavl_dictionary_set_nocopy(&foot, GAVL_META_STREAMS, &arr_val);
  
  fprintf(stderr, "Writing footer\n");
  gavl_dictionary_dump(&foot, 2);
  
  gavl_dictionary_write(g->io, &foot);
  gavl_dictionary_free(&foot);

#if 0
  
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

#endif
  
  gavf_chunk_finish(g->io, &footer, 1);
  
  /* Write final tag */
  if((gavf_io_write_data(g->io, (uint8_t*)GAVF_TAG_TAIL, 8) < 8) ||
     !gavf_io_write_uint64f(g->io, footer.start))
    return 0;

  if(!gavf_io_write_uint64f(g->io, gavf_io_position(g->io) + 8))
    return 0;
  
  return 1;
  }

void gavf_footer_init(gavf_t * g)
  {
  int i;
  gavf_stream_t * s;

  for(i = 0; i < g->num_streams; i++)
    {
    s = g->streams + i;
    gavl_stream_stats_init(&s->stats);
    }
  }


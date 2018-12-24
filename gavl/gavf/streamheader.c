#include <gavfprivate.h>
#include <gavl/metatags.h>
#include <gavl/trackinfo.h>

#define KEY_TYPE "typ"
  
void gavf_stream_header_to_dict(const gavf_stream_header_t * src, gavl_dictionary_t * dst)
  {
  int ts = 0;
  gavl_dictionary_t * m_dst;
  
  gavl_dictionary_set_int(dst, KEY_TYPE, src->type);
  gavl_dictionary_set_int(dst, GAVL_META_ID, src->id);

  gavl_dictionary_set_dictionary(dst, GAVL_META_METADATA, &src->m);
  m_dst = gavl_dictionary_get_dictionary_nc(dst, GAVL_META_METADATA);
  
  switch(src->type)
    {
    case GAVF_STREAM_AUDIO:
      gavl_stream_set_compression_info(dst, &src->ci);
      gavl_dictionary_set_audio_format(dst, GAVL_META_STREAM_FORMAT, &src->format.audio);
      ts = src->format.audio.samplerate;
      break;
    case GAVF_STREAM_VIDEO:
    case GAVF_STREAM_OVERLAY:
      gavl_stream_set_compression_info(dst, &src->ci);
      gavl_dictionary_set_video_format(dst, GAVL_META_STREAM_FORMAT, &src->format.video);
      ts = src->format.video.timescale;
      break;
    case GAVF_STREAM_TEXT:
      break;
    case GAVF_STREAM_MSG:
      ts = GAVL_TIME_SCALE;
      break;
    case GAVF_STREAM_NONE:
      break;
    }
  
  if(ts > 0)
    {
    gavl_dictionary_set_int(m_dst, GAVL_META_STREAM_PACKET_TIMESCALE, ts);
    gavl_dictionary_set_int(m_dst, GAVL_META_STREAM_SAMPLE_TIMESCALE, ts);
    }
  
  }

void gavf_stream_header_from_dict(gavf_stream_header_t * dst, const gavl_dictionary_t * src)
  {
  int type;
  int id;
  const gavl_dictionary_t * m;
  
  if((m = gavl_dictionary_get_dictionary(src, GAVL_META_METADATA)))
    gavl_dictionary_copy(&dst->m, m);
  
  gavl_dictionary_get_int(src, KEY_TYPE, &type);
  gavl_dictionary_get_int(src, GAVL_META_ID, &id);
  
  dst->type = type;
  dst->id = id;

  switch(dst->type)
    {
    case GAVF_STREAM_AUDIO:
      gavl_stream_get_compression_info(src, &dst->ci);
      gavl_audio_format_copy(&dst->format.audio,
                             gavl_dictionary_get_audio_format(src, GAVL_META_STREAM_FORMAT));
      break;
    case GAVF_STREAM_VIDEO:
    case GAVF_STREAM_OVERLAY:
      gavl_stream_get_compression_info(src, &dst->ci);
      gavl_video_format_copy(&dst->format.video,
                             gavl_dictionary_get_video_format(src, GAVL_META_STREAM_FORMAT));
      break;
    case GAVF_STREAM_TEXT:
    case GAVF_STREAM_MSG:
    case GAVF_STREAM_NONE:
      break;
    }
  }

#if 0
int gavf_stream_header_read(gavf_io_t * io, gavf_stream_header_t * h)
  {
  if(!gavf_io_read_uint32v(io, &h->type) ||
     !gavf_io_read_uint32v(io, &h->id))
    return 0;

  if(!gavl_dictionary_read(io, &h->m))
    return 0;
  
  switch(h->type)
    {
    case GAVF_STREAM_AUDIO:
      if(!gavf_read_compression_info(io, &h->ci) ||
         !gavf_read_audio_format(io, &h->format.audio))
        return 0;
      break;
    case GAVF_STREAM_VIDEO:
    case GAVF_STREAM_OVERLAY:
      if(!gavf_read_compression_info(io, &h->ci) ||
         !gavf_read_video_format(io, &h->format.video))
        return 0;
      break;
    case GAVF_STREAM_TEXT:
      //      if(!gavf_io_read_uint32v(io, &h->format.text.timescale))
      //        return 0;
      break;
    case GAVF_STREAM_NONE:
    case GAVF_STREAM_MSG:
      break;
    }
  return 1;
  }
#endif

void gavf_stream_header_apply_footer(gavf_stream_header_t * h)
  {
  /* Set maximum packet size */
  if(!h->ci.max_packet_size)
    h->ci.max_packet_size = h->stats.size_max;

  switch(h->type)
    {
    case GAVF_STREAM_VIDEO:
      gavf_stream_stats_apply_video(&h->stats, 
                                    &h->format.video,
                                    &h->ci,
                                    &h->m);
      break;
    case GAVF_STREAM_OVERLAY:
    case GAVF_STREAM_TEXT:
      break;
    case GAVF_STREAM_AUDIO:
      gavf_stream_stats_apply_audio(&h->stats, 
                                     &h->format.audio,
                                     &h->ci,
                                     &h->m);
      break;
    case GAVF_STREAM_NONE:
    case GAVF_STREAM_MSG:
      break;
    }
  }

#if 0
int gavf_stream_header_write(gavf_io_t * io, const gavf_stream_header_t * h)
  {
  if(!gavf_io_write_uint32v(io, h->type) ||
     !gavf_io_write_uint32v(io, h->id))
    return 0;

  if(!gavl_dictionary_write(io, &h->m))
    return 0;
  
  switch(h->type)
    {
    case GAVF_STREAM_AUDIO:
      if(!gavf_write_compression_info(io, &h->ci) ||
         !gavf_write_audio_format(io, &h->format.audio))
        return 0;
      break;
    case GAVF_STREAM_VIDEO:
    case GAVF_STREAM_OVERLAY:
      if(!gavf_write_compression_info(io, &h->ci) ||
         !gavf_write_video_format(io, &h->format.video))
        return 0;
      break;
    case GAVF_STREAM_TEXT:
      //      if(!gavf_io_write_uint32v(io, h->format.text.timescale))
      //        return 0;
      break;
    case GAVF_STREAM_NONE:
    case GAVF_STREAM_MSG:
      break;
    }
  return 1;
  }
#endif

void gavf_stream_header_free(gavf_stream_header_t * h)
  {
  gavl_compression_info_free(&h->ci);
  gavl_dictionary_free(&h->m);
  }

void gavf_stream_header_dump(const gavf_stream_header_t * h)
  {
  fprintf(stderr, "    Type: %d (%s)\n", h->type,
          gavf_stream_type_name(h->type));
  fprintf(stderr, "    ID:   %d\n", h->id);

  switch(h->type)
    {
    case GAVF_STREAM_AUDIO:
      gavl_compression_info_dumpi(&h->ci, 4);
      fprintf(stderr, "    Format:\n");
      gavl_audio_format_dumpi(&h->format.audio, 4);
      break;
    case GAVF_STREAM_VIDEO:
    case GAVF_STREAM_OVERLAY:
      gavl_compression_info_dumpi(&h->ci, 4);
      fprintf(stderr, "    Format:\n");
      gavl_video_format_dumpi(&h->format.video, 4);
      break;
    case GAVF_STREAM_TEXT:
    case GAVF_STREAM_NONE:
    case GAVF_STREAM_MSG:
      break;
    }
  fprintf(stderr, "    Metadata:\n");
  gavl_dictionary_dump(&h->m, 6);
  
  fprintf(stderr, "\n    Footer: ");
  if(h->stats.pts_start == GAVL_TIME_UNDEFINED)
    fprintf(stderr, "Not present\n");
  else
    {
    fprintf(stderr, "Present\n");
    fprintf(stderr, "      size_min:     %d\n", h->stats.size_min);
    fprintf(stderr, "      size_max:     %d\n", h->stats.size_max);
    fprintf(stderr, "      duration_min: %"PRId64"\n", h->stats.duration_min);
    fprintf(stderr, "      duration_max: %"PRId64"\n", h->stats.duration_max);
    fprintf(stderr, "      pts_start:    %"PRId64"\n", h->stats.pts_start);
    fprintf(stderr, "      pts_end:      %"PRId64"\n", h->stats.pts_end);
    }
  }

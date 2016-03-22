#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gavfprivate.h>
#include <gavl/utils.h>

void gavf_io_init(gavf_io_t * ret,
                  gavf_read_func  r,
                  gavf_write_func w,
                  gavf_seek_func  s,
                  gavf_close_func c,
                  gavf_flush_func f,
                  void * priv)
  {
  memset(ret, 0, sizeof(*ret));
  ret->read_func = r;
  ret->write_func = w;
  ret->seek_func = s;
  ret->close_func = c;
  ret->flush_func = f;
  ret->priv = priv;
  }

int gavf_io_can_seek(gavf_io_t * io)
  {
  return io->seek_func ? 1 : 0;
  }

gavf_io_t * gavf_io_create(gavf_read_func  r,
                           gavf_write_func w,
                           gavf_seek_func  s,
                           gavf_close_func c,
                           gavf_flush_func f,
                           void * priv)
  {
  gavf_io_t * ret;
  ret = malloc(sizeof(*ret));
  if(!ret)
    return NULL;
  gavf_io_init(ret, r, w, s, c, f, priv);
  return ret;
  }


void gavf_io_destroy(gavf_io_t * io)
  {
  if(io->flush_func)
    io->flush_func(io->priv);
  if(io->close_func)
    io->close_func(io->priv);
  if(io->filename)
    free(io->filename);
  if(io->mimetype)
    free(io->mimetype);
  free(io);
  }

void gavf_io_set_info(gavf_io_t * io, int64_t total_bytes, const char * filename, const char * mimetype)
  {
  if(total_bytes > 0)
    io->total_bytes = total_bytes;
  io->filename = gavl_strrep(io->filename, filename);
  io->mimetype = gavl_strrep(io->filename, mimetype);
  }

int64_t gavf_io_total_bytes(gavf_io_t * io)
  {
  return io->total_bytes;
  }

const char * gavf_io_filename(gavf_io_t * io)
  {
  return io->filename;
  }

const char * gavf_io_mimetype(gavf_io_t * io)
  {
  return io->mimetype;
  }



int gavf_io_flush(gavf_io_t * io)
  {
  int ret = 1;
  if(io->got_error)
    return 0;
  
  if(io->flush_func)
    ret = io->flush_func(io->priv);
  if(!ret)
    io->got_error = 1;
  return ret;
  }

int gavf_io_got_error(gavf_io_t * io)
  {
  return io->got_error;
  }

int64_t gavf_io_position(gavf_io_t * io)
  {
  return io->position;
  }

int gavf_io_read_data(gavf_io_t * io, uint8_t * buf, int len)
  {
  int ret;
  if(!io->read_func)
    return 0;
  ret = io->read_func(io->priv, buf, len);
  if(ret > 0)
    io->position += ret;
  return ret;
  }

int gavf_io_write_data(gavf_io_t * io, const uint8_t * buf, int len)
  {
  int ret;
  if(io->got_error)
    return -1;

  if(!io->write_func)
    return 0;
  ret = io->write_func(io->priv, buf, len);
  if(ret > 0)
    io->position += ret;

  if(ret < len)
    io->got_error = 1;
  return ret;
  }

void gavf_io_skip(gavf_io_t * io, int bytes)
  {
  if(io->seek_func)
    io->position = io->seek_func(io->priv, bytes, SEEK_CUR);
  else
    {
    int i;
    uint8_t c;
    for(i = 0; i < bytes; i++)
      {
      if(gavf_io_read_data(io, &c, 1) < 1)
        break;
      io->position++;
      }
    }
  }

int64_t gavf_io_seek(gavf_io_t * io, int64_t pos, int whence)
  {
  if(!io->seek_func)
    return -1;
  io->position = io->seek_func(io->priv, pos, whence);
  return io->position;
  }


/* Fixed size integers (BE) */

int gavf_io_write_uint64f(gavf_io_t * io, uint64_t num)
 {
  uint8_t buf[8];
  buf[7] = num & 0xff;
  buf[6] = (num>>8) & 0xff;
  buf[5] = (num>>16) & 0xff;
  buf[4] = (num>>24) & 0xff;
  buf[3] = (num>>32) & 0xff;
  buf[2] = (num>>40) & 0xff;
  buf[1] = (num>>48) & 0xff;
  buf[0] = (num>>56) & 0xff;
  return (gavf_io_write_data(io, buf, 8) < 8) ? 0 : 1;
  }

int gavf_io_read_uint64f(gavf_io_t * io, uint64_t * num)
  {
  uint8_t buf[8];
  if(gavf_io_read_data(io, buf, 8) < 8)
    return 0;

  *num =
    ((uint64_t)buf[0] << 56 ) |
    ((uint64_t)buf[1] << 48 ) |
    ((uint64_t)buf[2] << 40 ) |
    ((uint64_t)buf[3] << 32 ) |
    ((uint64_t)buf[4] << 24 ) |
    ((uint64_t)buf[5] << 16 ) |
    ((uint64_t)buf[6] << 8 ) |
    ((uint64_t)buf[7]);
  return 1;
  }

static int write_uint32f(gavf_io_t * io, uint32_t num)
  {
  uint8_t buf[4];
  buf[3] = num & 0xff;
  buf[2] = (num>>8) & 0xff;
  buf[1] = (num>>16) & 0xff;
  buf[0] = (num>>24) & 0xff;
  return (gavf_io_write_data(io, buf, 4) < 4) ? 0 : 1;
  }

static int read_uint32f(gavf_io_t * io, uint32_t * num)
  {
  uint8_t buf[4];
  if(gavf_io_read_data(io, buf, 4) < 4)
    return 0;
  *num =
    ((uint32_t)buf[0] << 24 ) ||
    ((uint32_t)buf[1] << 16 ) ||
    ((uint32_t)buf[2] << 8 ) ||
    ((uint32_t)buf[3]);
  return 1;
  }


/*
 *  Integer formats:
 *
 *  7 bit (0..127, -64..63)
 *  1xxx xxxx
 *
 *  14 bit (0..16383, -8192..8193)
 *  01xx xxxx xxxx xxxx
 *
 *  21 bit (0..2097151, -1048576..1048575)
 *  001x xxxx xxxx xxxx xxxx xxxx
 *
 *  28 bit (0..268435455, -134217728..134217727)
 *  0001 xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  35 bit
 *  0000 1xxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  42 bit
 *  0000 01xx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  49 bit
 *  0000 001x xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  56 bit
 *  0000 0001 xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 *
 *  64 bit
 *  0000 0000 xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
 */

static const int64_t int_offsets[] =
  {
    ((uint64_t)1)<<6,
    ((uint64_t)1)<<13,
    ((uint64_t)1)<<20,
    ((uint64_t)1)<<27,
    ((uint64_t)1)<<34,
    ((uint64_t)1)<<41,
    ((uint64_t)1)<<48,
    ((uint64_t)1)<<55,
  };

static const uint64_t uint_limits[] =
  {
    ((uint64_t)1)<<7,
    ((uint64_t)1)<<14,
    ((uint64_t)1)<<21,
    ((uint64_t)1)<<28,
    ((uint64_t)1)<<35,
    ((uint64_t)1)<<42,
    ((uint64_t)1)<<49,
    ((uint64_t)1)<<56,
  };

static int get_len_uint(uint64_t num)
  {
  int i;
  for(i = 0; i < 8; i++)
    {
    if(num < uint_limits[i])
      return i+1;
    }
  return 9;
  }

static int get_len_int(int64_t num)
  {
  int i;
  for(i = 0; i < 8; i++)
    {
    if((num < int_offsets[i]) && (num >= -int_offsets[i]))
      return i+1;
    }
  return 9;
  }

static int get_len_read(uint8_t num)
  {
  int i;
  for(i = 0; i < 8; i++)
    {
    if(num & (0x80 >> i))
      return i+1;
    }
  return 9;
  }

static int write_uint64v(gavf_io_t * io, uint64_t num, int len)
  {
  int idx;
  uint8_t buf[9];

  /* Length byte */
  buf[0] = 0x80 >> (len-1);

  idx = len - 1;

  while(idx)
    {
    buf[idx] = num & 0xff;
    num >>= 8;
    idx--;
    }

  if(len < 8)
    buf[0] |= (num & (0xff >> len));
  
  return (gavf_io_write_data(io, buf, len) < len) ? 0 : 1;
  }

int gavf_io_write_uint64v(gavf_io_t * io, uint64_t num)
  {
  return write_uint64v(io, num, get_len_uint(num));
  }

int gavf_io_write_int64v(gavf_io_t * io, int64_t num)
  {
  uint64_t num_u;
  int len = get_len_int(num);
  
  if(len == 9)
    num_u = num ^ 0x8000000000000000LL;
  else
    num_u = num + int_offsets[len-1];
  return write_uint64v(io, num_u, len);
  }

static int read_uint64v(gavf_io_t * io, uint64_t * num, int * len)
  {
  int i;
  int len1;
  uint8_t buf[9];

  if(!gavf_io_read_data(io, buf, 1))
    return 0;
  len1 = get_len_read(buf[0]);
  
  if(len1 > 1)
    {
    if(gavf_io_read_data(io, &buf[1], len1 - 1) < len1 - 1)
      return 0;
    }
  
  *num = buf[0] & (0xff >> len1);

  for(i = 1; i < len1; i++)
    {
    *num <<= 8;
    *num |= buf[i];
    }
  if(len)
    *len = len1;
  return 1;
  }
  
int gavf_io_read_int64v(gavf_io_t * io, int64_t * num)
  {
  int len;
  if(!read_uint64v(io, (uint64_t*)num, &len))
    return 0;

  if(len == 9)
    *num ^= 0x8000000000000000LL;
  else
    *num -= int_offsets[len-1];
  
  return 1;
  }


int gavf_io_read_uint64v(gavf_io_t * io, uint64_t * num)
  {
  return read_uint64v(io, num, NULL);
  }

int gavf_io_write_uint32v(gavf_io_t * io, uint32_t num)
  {
  return gavf_io_write_uint64v(io, num);
  }

int gavf_io_read_uint32v(gavf_io_t * io, uint32_t * num)
  {
  uint64_t ret;

  if(!gavf_io_read_uint64v(io, &ret))
    return 0;
  *num = ret;
  return 1;
  }

int gavf_io_write_int32v(gavf_io_t * io, int32_t num)
  {
  return gavf_io_write_int64v(io, num);
  }

int gavf_io_read_int32v(gavf_io_t * io, int32_t * num)
  {
  int64_t ret = 0;
  if(!gavf_io_read_int64v(io, &ret))
    return 0;
  *num = ret;
  return 1;
  }

/* int <-> float conversion routines taken from
   ffmpeg */

static float int2flt(int32_t v)
  {
  if((uint32_t)v+v > 0xFF000000U)
    return NAN;
  return ldexp(((v&0x7FFFFF) + (1<<23)) * (v>>31|1), (v>>23&0xFF)-150);
  }

static int32_t flt2int(float d)
  {
  int e;
  if     ( !d) return 0;
  else if(d-d) return 0x7F800000 + ((d<0)<<31) + (d!=d);
  d= frexp(d, &e);
  return (d<0)<<31 | (e+126)<<23 | (int64_t)((fabs(d)-0.5)*(1<<24));
  }

int gavf_io_read_float(gavf_io_t * io, float * num)
  {
  uint32_t val;
  if(!read_uint32f(io, &val))
    return 0;
  *num = int2flt(val);
  return 1;
  }

int gavf_io_write_float(gavf_io_t * io, float num)
  {
  uint32_t val = flt2int(num);
  return write_uint32f(io, val);
  }


int gavf_io_read_string(gavf_io_t * io, char ** ret)
  {
  uint32_t len;
  
  if(!gavf_io_read_uint32v(io, &len))
    return 0;

  if(!len)
    {
    *ret = NULL;
    return 1;
    }
  *ret = malloc(len + 1);
  if(!ret)
    return 0;

  if(gavf_io_read_data(io, (uint8_t*)(*ret), len) < len)
    return 0;

  /* Zero terminate */
  (*ret)[len] = '\0';
  return 1;
  }

int gavf_io_write_string(gavf_io_t * io, const char * str)
  {
  uint32_t len;

  if(!str)
    return gavf_io_write_uint32v(io, 0);
  
  len = strlen(str);
  if(!gavf_io_write_uint32v(io, len) ||
     gavf_io_write_data(io, (const uint8_t*)str, len) < len)
    return 0;
  return 1;
  }

int gavf_io_read_buffer(gavf_io_t * io, gavf_buffer_t * ret)
  {
  uint32_t len;
  
  if(!gavf_io_read_uint32v(io, &len) ||
     !gavf_buffer_alloc(ret, len) ||
     (gavf_io_read_data(io, ret->buf, len) < len))
    return 0;
  ret->len = len;
  return 1;
  }

int gavf_io_write_buffer(gavf_io_t * io, const gavf_buffer_t * buf)
  {
  if(!gavf_io_write_uint32v(io, buf->len) ||
     (gavf_io_write_data(io, buf->buf, buf->len) < buf->len))
    return 0;
  return 1;
  }

void gavf_io_set_cb(gavf_io_t * io, gavf_io_cb_func cb, void * cb_priv)
  {
  io->cb = cb;
  io->cb_priv = cb_priv;
  }

int gavf_io_cb(gavf_io_t * io, int type, const void * data)
  {
  int ret;
  if(!io->cb)
    return 1;
  if(io->got_error)
    return 0;
  ret = io->cb(io->cb_priv, type, data);

  if(!ret)
    io->got_error = 1;
  return ret;
  }

/* A/V frames */

typedef struct
  {
  gavl_packet_source_t * psrc;
  gavl_metadata_t m;
  } audio_source_priv_t;

typedef struct
  {
  gavl_packet_sink_t * psink;
  } audio_sink_priv_t;


typedef struct
  {
  gavl_packet_source_t * psrc;
  gavl_metadata_t m;
  } video_source_priv_t;

typedef struct
  {
  gavl_packet_sink_t * psink;
  } video_sink_priv_t;

static gavl_source_status_t
audio_source_func(void * data, gavl_audio_frame_t ** frame)
  {
  audio_source_priv_t * p = data;

  
  
  }

static gavl_source_status_t
video_source_func(void * data, gavl_video_frame_t ** frame)
  {
  video_source_priv_t * priv = data;
  gavl_packet_t * p = NULL;
  gavl_source_status_t st;

  if((st = gavl_packet_source_read_packet(priv->psrc, &p)) != GAVL_SOURCE_OK)
    return st;

  //  if(!priv->frame)
  //    priv->frame = gavl_video_frame_create(NULL);
  
  //  gavf_packet_to_video_frame(p, priv->frame, , &s->h->m, &s->dsp);
  //  *frame = s->vframe;
  return GAVL_SOURCE_OK;

  
  }

static void audio_source_free(void * data)
  {
  audio_source_priv_t * p = data;
  if(p->psrc)
    gavl_packet_source_destroy(p->psrc);
  free(p);
  }

static void audio_sink_free(void * data)
  {
  audio_sink_priv_t * p = data;
  if(p->psink)
    gavl_packet_sink_destroy(p->psink);
  free(p);
  }

static void video_source_free(void * data)
  {
  video_source_priv_t * p = data;
  if(p->psrc)
    gavl_packet_source_destroy(p->psrc);
  free(p);
  }

static void video_sink_free(void * data)
  {
  video_sink_priv_t * p = data;
  if(p->psink)
    gavl_packet_sink_destroy(p->psink);
  free(p);
  }


gavl_audio_source_t * gavl_audio_source_create_io(gavf_io_t * io,
                                                  gavl_audio_format_t * fmt,
                                                  gavl_metadata_t * m)
  {
  gavl_audio_source_t * ret;
  int src_flags = 0;
  audio_source_priv_t * priv = calloc(1, sizeof(*priv));
  priv->psrc = gavl_packet_source_create_io(io, fmt->samples_per_frame,
                                            GAVF_PACKET_WRITE_PTS);
  
  ret = gavl_audio_source_create(audio_source_func, priv, src_flags, fmt);

  gavl_audio_source_set_free_func(ret, audio_source_free);
  
  return ret;
  }

gavl_audio_frame_t * audio_sink_get(void * priv)
  {

  }

gavl_sink_status_t
audio_sink_put(void * priv, gavl_audio_frame_t * f)
  {
  
  }

gavl_audio_sink_t * gavl_audio_sink_create_io(gavf_io_t * io,
                                              gavl_audio_format_t * fmt,
                                              gavl_metadata_t * m)
  {
  gavl_audio_sink_t * ret;
  audio_sink_priv_t * priv = calloc(1, sizeof(*priv));

  priv->psink = gavl_packet_sink_create_io(io,
                                           fmt->samples_per_frame,
                                           GAVF_PACKET_WRITE_PTS);

  ret = gavl_audio_sink_create(audio_sink_get, audio_sink_put, priv, fmt);
  }
  
gavl_video_source_t * gavl_video_source_create_io(gavf_io_t * io,
                                                  gavl_video_format_t * fmt,
                                                  gavl_metadata_t * m)
  {
  gavl_video_source_t * ret;
  int src_flags = 0;
  int default_duration;
  int packet_flags = GAVF_PACKET_WRITE_PTS;
  video_source_priv_t * priv = calloc(1, sizeof(*priv));

  if(fmt->framerate_mode == GAVL_FRAMERATE_CONSTANT)
    default_duration = fmt->frame_duration;
  else
    {
    default_duration = 0;
    packet_flags |= GAVF_PACKET_WRITE_DURATION;
    }
  
  priv->psrc = gavl_packet_source_create_io(io, default_duration,
                                            packet_flags);
  
  ret = gavl_video_source_create(video_source_func, priv, src_flags, fmt);
  return ret;
  }

gavl_video_sink_t * gavl_video_sink_create_io(gavf_io_t * io,
                                              gavl_video_format_t * fmt,
                                              gavl_metadata_t * m)
  {
  gavl_video_sink_t * ret;
  int src_flags = 0;
  int default_duration;
  int packet_flags = GAVF_PACKET_WRITE_PTS;

  video_sink_priv_t * priv = calloc(1, sizeof(*priv));

  if(fmt->framerate_mode == GAVL_FRAMERATE_CONSTANT)
    default_duration = fmt->frame_duration;
  else
    {
    default_duration = 0;
    packet_flags |= GAVF_PACKET_WRITE_DURATION;
    }

  priv->psink = gavl_packet_sink_create_io(io, default_duration,
                                           packet_flags);
  
  }

/* Packet source */

typedef struct
  {
  gavf_io_t * io;
  int default_duration;
  int packet_flags;
  int64_t pts;
  } packet_source_priv_t;

static gavl_source_status_t
packet_source_func(void * data, gavl_packet_t ** p)
  {
  packet_source_priv_t * priv = data;

  if(!gavf_read_gavl_packet(priv->io,
                            priv->default_duration,
                            priv->packet_flags,
                            0, &priv->pts, 0, *p))
    return GAVL_SOURCE_EOF;
  else
    return GAVL_SOURCE_OK;
  }

gavl_packet_source_t * gavl_packet_source_create_io(gavf_io_t * io,
                                                    int default_duration,
                                                    int packet_flags)
  {
  gavl_packet_source_t * ret;
  packet_source_priv_t * priv = calloc(1, sizeof(*priv));
  priv->io               = io;
  priv->default_duration = default_duration;
  priv->packet_flags     = packet_flags;

  ret = gavl_packet_source_create(packet_source_func, priv, 0);
  gavl_packet_source_set_free_func(ret, free);
  }

/* Packet sink */

typedef struct
  {
  gavf_io_t * io;
  int default_duration;
  int packet_flags;
  gavl_packet_t p;

  gavf_io_t * hdr_io;
  
  } packet_sink_priv_t;

static gavl_packet_t *
packet_sink_get_func(void * data)
  {
  packet_sink_priv_t * p = data;
  return &p->p;
  }

static gavl_sink_status_t
packet_sink_put_func(void * data, gavl_packet_t * p)
  {
  packet_sink_priv_t * priv = data;
  
  if(!gavf_write_gavl_packet(priv->io, priv->hdr_io, priv->default_duration, priv->packet_flags, 0,
                             &priv->p))
    return GAVL_SINK_ERROR;
  else
    return GAVL_SINK_OK;
  }

static void packet_sink_free_func(void * data)
  {
  packet_sink_priv_t * p = data;
  if(p->hdr_io)
    gavf_io_destroy(p->hdr_io);
  gavl_packet_free(&p->p);
  free(p);
  }

gavl_packet_sink_t * gavl_packet_sink_create_io(gavf_io_t * io,
                                                int default_duration,
                                                int packet_flags)
  {
  gavl_packet_sink_t * ret;
  packet_sink_priv_t * priv = calloc(1, sizeof(*priv));
  priv->io               = io;
  priv->default_duration = default_duration;
  priv->packet_flags     = packet_flags;
  priv->hdr_io = gavf_io_create_buf_write();

  ret = gavl_packet_sink_create(packet_sink_get_func,
                                packet_sink_put_func,
                                priv);

  gavl_packet_sink_set_free_func(ret, packet_sink_free_func);
  
  return ret;
  }

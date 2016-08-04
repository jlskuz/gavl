#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <gavfprivate.h>
#include <gavl/utils.h>
#include <gavl/numptr.h>
#include <gavl/value.h>

// #define DUMP_MSG_WRITE
// #define DUMP_MSG_READ

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
  GAVL_64BE_2_PTR(num, buf);
  return (gavf_io_write_data(io, buf, 8) < 8) ? 0 : 1;
  }

int gavf_io_read_uint64f(gavf_io_t * io, uint64_t * num)
  {
  uint8_t buf[8];
  if(gavf_io_read_data(io, buf, 8) < 8)
    return 0;
  *num = GAVL_PTR_2_64BE(buf);
  return 1;
  }

static int write_uint32f(gavf_io_t * io, uint32_t num)
  {
  uint8_t buf[4];

  GAVL_32BE_2_PTR(num, buf);

  //  fprintf(stderr, "write_uint32f\n");
  //  gavl_hexdump(buf, 4, 4);

  return (gavf_io_write_data(io, buf, 4) < 4) ? 0 : 1;
  }

static int read_uint32f(gavf_io_t * io, uint32_t * num)
  {
  uint8_t buf[4];
  if(gavf_io_read_data(io, buf, 4) < 4)
    return 0;

  //  fprintf(stderr, "read_uint32f\n");
  //  gavl_hexdump(buf, 4, 4);

  *num = GAVL_PTR_2_32BE(buf);
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

typedef union {
    uint32_t i;
    float    f;
} intfloat32_t;

typedef union  {
    uint64_t i;
    double   f;
} intfloat64_t;


static float int2flt(uint32_t i)
  {
  intfloat32_t v;
  v.i = i;
  return v.f;
  }

static uint32_t flt2int(float f)
  {
  intfloat32_t v;
  v.f = f;
  return v.i;
  }

static double int2dbl(uint64_t i)
  {
  intfloat64_t v;
  v.i = i;
  return v.f;
  }

static uint64_t dbl2int(double f)
  {
  intfloat64_t v;
  v.f = f;
  return v.i;
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

int gavf_io_read_double(gavf_io_t * io, double * num)
  {
  uint64_t val;
  if(!gavf_io_read_uint64f(io, &val))
    return 0;
  *num = int2dbl(val);
  return 1;
  }

int gavf_io_write_double(gavf_io_t * io, double num)
  {
  uint64_t val = dbl2int(num);
  return gavf_io_write_uint64f(io, val);
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

int gavf_io_read_buffer(gavf_io_t * io, gavl_buffer_t * ret)
  {
  uint32_t len;
  
  if(!gavf_io_read_uint32v(io, &len))
    return 0;

  if(!len)
    {
    gavl_buffer_free(ret);
    gavl_buffer_init(ret);
    }
  else if(!gavl_buffer_alloc(ret, len) ||
          (gavf_io_read_data(io, ret->buf, len) < len))
    return 0;
  ret->len = len;
  return 1;
  }

int gavf_io_write_buffer(gavf_io_t * io, const gavl_buffer_t * buf)
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
  gavl_audio_frame_t * frame;
  gavl_dsp_context_t * dsp;
  gavl_audio_format_t fmt;
  } audio_source_priv_t;

typedef struct
  {
  gavl_packet_sink_t * psink;
  gavl_packet_t * p;
  gavl_audio_frame_t * frame;
  int max_packet_size;
  gavl_audio_format_t fmt;
  } audio_sink_priv_t;

typedef struct
  {
  gavl_packet_source_t * psrc;
  gavl_metadata_t m;
  gavl_video_frame_t * frame;
  gavl_dsp_context_t * dsp;
  gavl_video_format_t fmt;
  } video_source_priv_t;

typedef struct
  {
  gavl_packet_sink_t * psink;
  int max_packet_size;
  gavl_packet_t * p;
  gavl_video_frame_t * frame;
  gavl_video_format_t fmt;
  } video_sink_priv_t;

static gavl_source_status_t
audio_source_func(void * data, gavl_audio_frame_t ** frame)
  {
  audio_source_priv_t * priv = data;

  gavl_packet_t * p = NULL;
  gavl_source_status_t st;

  if((st = gavl_packet_source_read_packet(priv->psrc, &p)) != GAVL_SOURCE_OK)
    return st;

  if(!priv->frame)
    priv->frame = gavl_audio_frame_create(NULL);
  
  gavf_packet_to_audio_frame(p, priv->frame, &priv->fmt, &priv->m, &priv->dsp);
  *frame = priv->frame;
  return GAVL_SOURCE_OK;
  }

static gavl_source_status_t
video_source_func(void * data, gavl_video_frame_t ** frame)
  {
  video_source_priv_t * priv = data;
  gavl_packet_t * p = NULL;
  gavl_source_status_t st;

  if((st = gavl_packet_source_read_packet(priv->psrc, &p)) != GAVL_SOURCE_OK)
    return st;

  if(!priv->frame)
    priv->frame = gavl_video_frame_create(NULL);
  
  gavf_packet_to_video_frame(p, priv->frame, &priv->fmt, &priv->m, &priv->dsp);
  *frame = priv->frame;
  return GAVL_SOURCE_OK;
  }

static void audio_source_free(void * data)
  {
  audio_source_priv_t * p = data;
  if(p->psrc)
    gavl_packet_source_destroy(p->psrc);

  if(p->frame)
    {
    gavl_audio_frame_null(p->frame);
    gavl_audio_frame_destroy(p->frame);
    }
  if(p->dsp)
    gavl_dsp_context_destroy(p->dsp);

  free(p);
  }

static void audio_sink_free(void * data)
  {
  audio_sink_priv_t * p = data;
  if(p->psink)
    gavl_packet_sink_destroy(p->psink);
  if(p->frame)
    {
    gavl_audio_frame_null(p->frame);
    gavl_audio_frame_destroy(p->frame);
    }
  free(p);
  }

static void video_source_free(void * data)
  {
  video_source_priv_t * p = data;
  if(p->psrc)
    gavl_packet_source_destroy(p->psrc);

  if(p->frame)
    {
    gavl_video_frame_null(p->frame);
    gavl_video_frame_destroy(p->frame);
    }
  if(p->dsp)
    gavl_dsp_context_destroy(p->dsp);
  
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
                                                  const gavl_audio_format_t * fmt,
                                                  const gavl_metadata_t * m)
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

static
gavl_audio_frame_t * audio_sink_get(void * data)
  {
  audio_sink_priv_t * priv = data;

  priv->p = gavl_packet_sink_get_packet(priv->psink);

  gavl_packet_reset(priv->p);
  gavl_packet_alloc(priv->p, priv->max_packet_size);

  if(!priv->frame)
    priv->frame = gavl_audio_frame_create(NULL);

  priv->frame->valid_samples = priv->fmt.samples_per_frame;
  
  gavl_audio_frame_set_channels(priv->frame, &priv->fmt, priv->p->data);

  priv->frame->valid_samples = 0;
  return priv->frame;
  }

static gavl_sink_status_t
audio_sink_put(void * data, gavl_audio_frame_t * f)
  {
  gavl_sink_status_t st;
  audio_sink_priv_t * priv = data;
  
  gavf_audio_frame_to_packet_metadata(priv->frame, priv->p);
  priv->p->data_len = priv->max_packet_size;
  gavf_shrink_audio_frame(priv->frame, priv->p, &priv->fmt);

  st = gavl_packet_sink_put_packet(priv->psink, priv->p);
  priv->p = NULL;
  return st;
  }

gavl_audio_sink_t * gavl_audio_sink_create_io(gavf_io_t * io,
                                              gavl_audio_format_t * fmt,
                                              gavl_metadata_t * m)
  {
  gavl_audio_sink_t * ret;
  audio_sink_priv_t * priv = calloc(1, sizeof(*priv));
  gavl_audio_format_copy(&priv->fmt, fmt);

  gavl_metadata_set_endian(m);
  
  priv->psink = gavl_packet_sink_create_io(io,
                                           fmt->samples_per_frame,
                                           GAVF_PACKET_WRITE_PTS);
  
  ret = gavl_audio_sink_create(audio_sink_get, audio_sink_put, priv, fmt);
  gavl_audio_sink_set_free_func(ret, audio_sink_free);
  return ret;
  }
  
gavl_video_source_t * gavl_video_source_create_io(gavf_io_t * io,
                                                  const gavl_video_format_t * fmt,
                                                  const gavl_metadata_t * m)
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

  gavl_video_format_copy(&priv->fmt, fmt);
  gavl_metadata_copy(&priv->m, m);
  
  priv->psrc = gavl_packet_source_create_io(io, default_duration,
                                            packet_flags);
  
  ret = gavl_video_source_create(video_source_func, priv, src_flags, fmt);

  gavl_video_source_set_free_func(ret, video_source_free);

  return ret;
  }

static
gavl_video_frame_t * video_sink_get(void * data)
  {
  video_sink_priv_t * priv = data;

  priv->p = gavl_packet_sink_get_packet(priv->psink);

  gavl_packet_reset(priv->p);
  gavl_packet_alloc(priv->p, priv->max_packet_size);

  if(!priv->frame)
    priv->frame = gavl_video_frame_create(NULL);

  priv->frame->strides[0] = 0;
  gavl_video_frame_set_planes(priv->frame,
                              &priv->fmt, priv->p->data);
  return priv->frame;
  }

static gavl_sink_status_t
video_sink_put(void * data, gavl_video_frame_t * f)
  {
  gavl_sink_status_t st;
  video_sink_priv_t * priv = data;
  
  gavf_video_frame_to_packet_metadata(priv->frame, priv->p);
  priv->p->data_len = priv->max_packet_size;
  st = gavl_packet_sink_put_packet(priv->psink, priv->p);
  priv->p = NULL;
  return st;
  }

gavl_video_sink_t * gavl_video_sink_create_io(gavf_io_t * io,
                                              gavl_video_format_t * fmt,
                                              gavl_metadata_t * m)
  {
  gavl_video_sink_t * ret;
  int default_duration;
  int packet_flags = GAVF_PACKET_WRITE_PTS;

  video_sink_priv_t * priv = calloc(1, sizeof(*priv));

  gavl_metadata_set_endian(m);
  
  if(fmt->framerate_mode == GAVL_FRAMERATE_CONSTANT)
    default_duration = fmt->frame_duration;
  else
    {
    default_duration = 0;
    packet_flags |= GAVF_PACKET_WRITE_DURATION;
    }

  gavl_video_format_copy(&priv->fmt, fmt);
  
  priv->max_packet_size =
    gavf_get_max_video_packet_size(&priv->fmt,
                                   NULL);
  
  priv->psink = gavl_packet_sink_create_io(io, default_duration,
                                           packet_flags);
  
  ret = gavl_video_sink_create(video_sink_get, video_sink_put, priv, fmt);
  gavl_video_sink_set_free_func(ret, video_sink_free);
  return ret;
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
  return ret;
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

/* */


int gavl_msg_read(gavl_msg_t * ret, gavf_io_t * io)
  {
  int i;
  
  int32_t val_i;

  memset(ret, 0, sizeof(*ret));

  /* Message Namespace */
  
  if(!gavf_io_read_int32v(io, &val_i))
    return 0;
  
  ret->ns = val_i;
  
  /* Message ID */
  
  if(!gavf_io_read_int32v(io, &val_i))
    return 0;
  
  ret->id = val_i;

  //  fprintf(stderr, "Got ID: %d\n", ret->id);
  
  /* Number of arguments */

  if(!gavf_io_read_int32v(io, &val_i))
    return 0;
  
  ret->num_args = val_i;

  for(i = 0; i < ret->num_args; i++)
    gavl_value_read(io, &ret->args[i]);
  
#ifdef DUMP_MSG_READ
  fprintf(stderr, "read message:\n");
  gavl_msg_dump(ret, 1);
#endif
  return 1;
  }


int gavl_msg_write(const gavl_msg_t * msg, gavf_io_t * io)
  {
  int i;

#ifdef DUMP_MSG_WRITE
  fprintf(stderr, "writing message:\n");
  gavl_msg_dump(msg, 1);
#endif
  
  /* Namespace */

  if(!gavf_io_write_int32v(io, msg->ns))
    return 0;
  
  /* Message id */

  if(!gavf_io_write_int32v(io, msg->id))
    return 0;

  /* Number of arguments */

  if(!gavf_io_write_int32v(io, msg->num_args))
    return 0;
  
  /* Arguments */

  for(i = 0; i < msg->num_args; i++)
    gavl_value_write(io, &msg->args[i]);
  
  return 1;
  }

/* */

int gavl_value_read(gavf_io_t * io, gavl_value_t * v)
  {
  if(!gavf_io_read_int32v(io, (int32_t*)&v->type))
    return 0;
  
  switch(v->type)
    {
    case GAVL_TYPE_UNDEFINED:
      break;
    case GAVL_TYPE_INT:
      if(!gavf_io_read_int32v(io, &v->v.i))
        return 0;
      break;
    case GAVL_TYPE_LONG:
      if(!gavf_io_read_int64v(io, &v->v.l))
        return 0;
      break;
    case GAVL_TYPE_FLOAT:
      if(!gavf_io_read_double(io, &v->v.d))
        return 0;
      break;
    case GAVL_TYPE_STRING:
      if(!gavf_io_read_string(io, &v->v.str))
        return 0;
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      {
      gavl_buffer_t buf;
      gavl_buffer_init(&buf);
      if(!gavf_io_read_buffer(io, &buf) ||
         !gavl_audio_format_from_buffer(buf.buf, buf.len, &v->v.audioformat))
        {
        gavl_buffer_free(&buf);
        return 0;
        }
      gavl_buffer_free(&buf);
      }
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      {
      gavl_buffer_t buf;
      gavl_buffer_init(&buf);
      if(!gavf_io_read_buffer(io, &buf) ||
         !gavl_video_format_from_buffer(buf.buf, buf.len, &v->v.videoformat))
        {
        gavl_buffer_free(&buf);
        return 0;
        }
      gavl_buffer_free(&buf);
      }
      break;
    case GAVL_TYPE_COLOR_RGB:
      if(!gavf_io_read_double(io, &v->v.color[0]) ||
         !gavf_io_read_double(io, &v->v.color[1]) ||
         !gavf_io_read_double(io, &v->v.color[2]))
        return 0;
      break;
    case GAVL_TYPE_COLOR_RGBA:
      if(!gavf_io_read_double(io, &v->v.color[0]) ||
         !gavf_io_read_double(io, &v->v.color[1]) ||
         !gavf_io_read_double(io, &v->v.color[2]) ||
         !gavf_io_read_double(io, &v->v.color[3]))
        return 0;
      break;
    case GAVL_TYPE_POSITION:
      if(!gavf_io_read_double(io, &v->v.position[0]) ||
         !gavf_io_read_double(io, &v->v.position[1]))
        return 0;
      break;
    case GAVL_TYPE_DICTIONARY:
      {
      gavl_dictionary_read(io, &v->v.dictionary);
      }
      break;
    case GAVL_TYPE_ARRAY:
      {
      int i;
      if(!gavf_io_read_int32v(io, &v->v.array.num_entries))
        return 0;

      v->v.array.entries_alloc = v->v.array.num_entries;
      v->v.array.entries = calloc(v->v.array.entries_alloc, sizeof(v->v.array.entries));

      for(i = 0; i < v->v.array.num_entries; i++)
        {
        if(!gavl_value_read(io, &v->v.array.entries[i]))
          return 0;
        }
      break;
      }
      
      break;
    }
  return 1;
  }

int gavl_dictionary_write(gavf_io_t * io, const gavl_dictionary_t * dict)
  {
  int i;
  if(!gavf_io_write_int32v(io, dict->num_entries))
    return 0;
  for(i = 0; i < dict->num_entries; i++)
    {
    if(!gavf_io_write_string(io, dict->entries[i].name) ||
       !gavl_value_write(io, &dict->entries[i].v))
      return 0;
    }
  return 1;
  }

int gavl_dictionary_read(gavf_io_t * io, gavl_dictionary_t * dict)
  {
  int i;
  if(!gavf_io_read_int32v(io, &dict->num_entries))
    return 0;
  
  dict->entries_alloc = dict->num_entries;
  dict->entries = calloc(dict->entries_alloc, sizeof(dict->entries));
  
  for(i = 0; i < dict->num_entries; i++)
    {
    if(!gavf_io_read_string(io, &dict->entries[i].name) ||
       !gavl_value_read(io, &dict->entries[i].v))
      return 0;
    }
  return 1;
  }

int gavl_value_write(gavf_io_t * io, const gavl_value_t * v)
  {
  if(!gavf_io_write_int32v(io, v->type))
    return 0;
  
  switch(v->type)
    {
    case GAVL_TYPE_UNDEFINED:
      break;
    case GAVL_TYPE_INT:
      if(!gavf_io_write_int32v(io, v->v.i))
        return 0;
      break;
    case GAVL_TYPE_LONG:
      if(!gavf_io_write_int64v(io, v->v.l))
        return 0;
      break;
    case GAVL_TYPE_FLOAT:
      if(!gavf_io_write_double(io, v->v.d))
        return 0;
      break;
    case GAVL_TYPE_STRING:
      if(!gavf_io_write_string(io, v->v.str))
        return 0;
      break;
    case GAVL_TYPE_AUDIOFORMAT:
      {
      gavl_buffer_t buf;
      gavl_buffer_init(&buf);
      
      if(!(buf.buf = gavl_audio_format_to_buffer(&buf.len, &v->v.audioformat)) ||
         !gavf_io_write_buffer(io, &buf))
        {
        gavl_buffer_free(&buf);
        return 0;
        }
      gavl_buffer_free(&buf);
      }
      break;
    case GAVL_TYPE_VIDEOFORMAT:
      {
      gavl_buffer_t buf;
      gavl_buffer_init(&buf);
      
      if(!(buf.buf = gavl_video_format_to_buffer(&buf.len, &v->v.videoformat)) ||
         !gavf_io_write_buffer(io, &buf))
        {
        gavl_buffer_free(&buf);
        return 0;
        }
      gavl_buffer_free(&buf);
      }
      break;
    case GAVL_TYPE_COLOR_RGB:
      if(!gavf_io_write_double(io, v->v.color[0]) ||
         !gavf_io_write_double(io, v->v.color[1]) ||
         !gavf_io_write_double(io, v->v.color[2]))
        return 0;
      break;
    case GAVL_TYPE_COLOR_RGBA:
      if(!gavf_io_write_double(io, v->v.color[0]) ||
         !gavf_io_write_double(io, v->v.color[1]) ||
         !gavf_io_write_double(io, v->v.color[2]) ||
         !gavf_io_write_double(io, v->v.color[3]))
        return 0;
      break;
    case GAVL_TYPE_POSITION:
      if(!gavf_io_write_double(io, v->v.position[0]) ||
         !gavf_io_write_double(io, v->v.position[1]))
        return 0;
      break;
    case GAVL_TYPE_DICTIONARY:
      if(!gavl_dictionary_write(io, &v->v.dictionary))
        return 0;
      break;
    case GAVL_TYPE_ARRAY:
      {
      int i;
      if(!gavf_io_write_int32v(io, v->v.array.num_entries))
        return 0;
      for(i = 0; i < v->v.array.num_entries; i++)
        {
        if(!gavl_value_write(io, &v->v.array.entries[i]))
          return 0;
        }
      }
      break;
    }
  return 1;
  }

uint8_t * gavl_msg_to_buffer(int * len, const gavl_msg_t * msg)
  {
  uint8_t * ret;
  gavf_io_t * io = gavf_io_create_mem_write();
  gavl_msg_write(msg, io);
  ret = gavf_io_mem_get_buf(io, len);
  gavf_io_destroy(io);
  return ret;
  }

int gavl_msg_from_buffer(const uint8_t * buf, int len, gavl_msg_t * msg)
  {
  int result;
  gavf_io_t * io = gavf_io_create_mem_read(buf, len);
  result = gavl_msg_read(msg, io);

  //  fprintf(stderr, "bg_msg_from_buffer: %"PRId64"/%d\n", gavf_io_position(io), len);
  
  gavf_io_destroy(io);
  return result;
  }

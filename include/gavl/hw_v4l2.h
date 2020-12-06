#include <gavl/connectors.h>

typedef enum
  {
   GAVL_V4L_DEVICE_UNKNOWN   = 0,
   GAVL_V4L_DEVICE_SOURCE    = (1<<0),
   GAVL_V4L_DEVICE_SINK      = (1<<1),
   GAVL_V4L_DEVICE_ENCODER   = (1<<2),
   GAVL_V4L_DEVICE_DECODER   = (1<<3),
   GAVL_V4L_DEVICE_CONVERTER = (1<<4),
  } gavl_v4l_device_type_t;

/*
 * Metadata tags for devices.
 * Also supported are:
 *
 * GAVL_META_LABEL
 * GAVL_META_URI
 *
 */

#define GAVL_V4L_TYPE        "type"
#define GAVL_V4L_TYPE_STRING "typestr"
#define GAVL_V4L_CAPABILITIES "caps"

#define GAVL_V4L_SRC_FORMATS  "src_fmts"
#define GAVL_V4L_SINK_FORMATS "sink_fmts"

#define GAVL_V4L_FORMAT_V4L_PIX_FMT "pixfmt"
#define GAVL_V4L_FORMAT_V4L_FLAGS   "flags"

#define GAVL_V4L_FORMAT_GAVL_PIXELFORMAT "pixelformat"
#define GAVL_V4L_FORMAT_GAVL_CODEC_ID    "codecid"

typedef struct gavl_v4l_device_s gavl_v4l_device_t;

GAVL_PUBLIC void gavl_v4l_devices_scan(gavl_array_t * ret);
GAVL_PUBLIC void gavl_v4l_devices_scan_by_type(int type_mask, gavl_array_t * ret);

GAVL_PUBLIC int gavl_v4l_has_decoder(gavl_array_t * arr, gavl_codec_id_t id);

GAVL_PUBLIC gavl_codec_id_t gavl_v4l_pix_fmt_to_codec_id(uint32_t fmt);
GAVL_PUBLIC gavl_pixelformat_t gavl_v4l_pix_fmt_to_pixelformat(uint32_t fmt);
GAVL_PUBLIC uint32_t gavl_v4l_codec_id_to_pix_fmt(gavl_codec_id_t id);

// GAVL_PUBLIC gavl_packet_t * gavl_v4l_device_get_packet_write(gavl_v4l_device_t * dev);
// GAVL_PUBLIC gavl_sink_status_t gavl_v4l_device_put_packet_write(gavl_v4l_device_t * dev);
// GAVL_PUBLIC gavl_source_status_t gavl_v4l_device_read_frame(gavl_v4l_device_t * dev, gavl_video_frame_t ** frame);



GAVL_PUBLIC gavl_v4l_device_t * gavl_v4l_device_open(const gavl_dictionary_t * dev);

GAVL_PUBLIC void gavl_v4l_device_close(gavl_v4l_device_t * dev);

GAVL_PUBLIC int gavl_v4l_device_get_fd(gavl_v4l_device_t * dev);


GAVL_PUBLIC const gavl_dictionary_t * gavl_v4l_get_decoder(const gavl_array_t * arr, gavl_codec_id_t id);


GAVL_PUBLIC gavl_packet_sink_t * gavl_v4l_device_get_packet_sink(gavl_v4l_device_t * dev);
GAVL_PUBLIC gavl_packet_source_t * gavl_v4l_device_get_packet_source(gavl_v4l_device_t * dev);
GAVL_PUBLIC gavl_video_sink_t * gavl_v4l_device_get_video_sink(gavl_v4l_device_t * dev);
GAVL_PUBLIC gavl_video_source_t * gavl_v4l_device_get_video_source(gavl_v4l_device_t * dev);

#if 0
GAVL_PUBLIC int gavl_v4l_device_init_capture(gavl_v4l_device_t * dev,
                                             gavl_video_format_t * fmt);

GAVL_PUBLIC int gavl_v4l_device_init_output(gavl_v4l_device_t * dev,
                                            gavl_video_format_t * fmt);

GAVL_PUBLIC int gavl_v4l_device_init_encoder(gavl_v4l_device_t * dev,
                                             gavl_video_format_t * fmt,
                                             gavl_compression_info_t * cmp);

GAVL_PUBLIC int gavl_v4l_device_init_decoder(gavl_v4l_device_t * dev,
                                             gavl_video_format_t * fmt,
                                             const gavl_compression_info_t * cmp);
#endif

GAVL_PUBLIC int gavl_v4l_device_init_decoder(gavl_v4l_device_t * dev, gavl_dictionary_t * stream,
                                             gavl_packet_source_t * psrc);

GAVL_PUBLIC void gavl_v4l_device_info(const char * dev);
GAVL_PUBLIC void gavl_v4l_device_infos();


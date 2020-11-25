

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

#define GAVL_V4L_TYPE "type"

typedef struct gavl_v4l_device_s gavl_v4l_device_t;

GAVL_PUBLIC gavl_array_t * gavl_v4l_devices_scan();
GAVL_PUBLIC gavl_array_t * gavl_v4l_devices_scan_by_type(int type_mask);

GAVL_PUBLIC gavl_codec_id_t gavl_v4l_pix_fmt_to_codec_id(uint32_t fmt);
GAVL_PUBLIC gavl_pixelformat_t gavl_v4l_pix_fmt_to_pixelformat(uint32_t fmt);


GAVL_PUBLIC gavl_v4l_device_t * gavl_v4l_device_open(const char * dev);


GAVL_PUBLIC gavl_packet_sink_t * gavl_v4l_device_get_packet_sink(gavl_v4l_device_t * dev);
GAVL_PUBLIC gavl_packet_source_t * gavl_v4l_device_get_packet_source(gavl_v4l_device_t * dev);
GAVL_PUBLIC gavl_video_sink_t * gavl_v4l_device_get_video_sink(gavl_v4l_device_t * dev);
GAVL_PUBLIC gavl_video_source_t * gavl_v4l_device_get_video_source(gavl_v4l_device_t * dev);



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


GAVL_PUBLIC void gavl_v4l_device_info(const char * dev);
GAVL_PUBLIC void gavl_v4l_device_infos();


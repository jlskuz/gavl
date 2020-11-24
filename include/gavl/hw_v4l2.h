


typedef struct gavl_v4l_device_s gavl_v4l_device_t;


GAVL_PUBLIC gavl_v4l_device_t * gavl_v4l_device_open(const char * dev);

GAVL_PUBLIC int gavl_v4l_device_reads_video(gavl_v4l_device_t *);
GAVL_PUBLIC int gavl_v4l_device_writes_video(gavl_v4l_device_t *);

GAVL_PUBLIC int gavl_v4l_device_reads_packets(gavl_v4l_device_t *);
GAVL_PUBLIC int gavl_v4l_device_writes_packets(gavl_v4l_device_t *);

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



#include <config.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>


#include <gavl/gavl.h>
#include <gavl/compression.h>

#include <gavl/log.h>
#define LOG_DOMAIN "v4l"

#include <gavl/hw_v4l2.h>


#include <hw_private.h>

static int my_ioctl(int fd, int request, void * arg)
  {
  int r;
  
  do{
    r = ioctl (fd, request, arg);
  } while (-1 == r && EINTR == errno);
  
  return r;
  }

static struct
  {
  int cap_flag;
  const char * name;
  }
capabilities[] =
  {

   { V4L2_CAP_VIDEO_CAPTURE, "Capture" },                   // Is a video capture device
   { V4L2_CAP_VIDEO_OUTPUT,  "Output" },                    // Is a video output device
   { V4L2_CAP_VIDEO_OVERLAY, "Overlay" },                   // Can do video overlay
   { V4L2_CAP_VBI_CAPTURE,   "VBI Capture" },               // Is a raw VBI capture device
   { V4L2_CAP_VBI_OUTPUT,    "VBO Output" },                // Is a raw VBI output device
   { V4L2_CAP_SLICED_VBI_CAPTURE, "Sliced VBI Capture" },   // Is a sliced VBI capture devic
   { V4L2_CAP_SLICED_VBI_OUTPUT,  "Sliced VBI Output" },    // Is a sliced VBI output device
   { V4L2_CAP_RDS_CAPTURE, "RDS Capture"  },                // RDS data capture
   { V4L2_CAP_VIDEO_OUTPUT_OVERLAY, "Output Overlay" },     // Can do video output overlay
   { V4L2_CAP_HW_FREQ_SEEK, "Freq Seek" },                  // Can do hardware frequency seek
   { V4L2_CAP_RDS_OUTPUT, "RDS Output" },                   // Is an RDS encoder

   /* Is a video capture device that supports multiplanar formats */
   { V4L2_CAP_VIDEO_CAPTURE_MPLANE, "Capture multiplane" },  
   /* Is a video output device that supports multiplanar formats */
   { V4L2_CAP_VIDEO_OUTPUT_MPLANE, "Output multiplane" },
   /* Is a video mem-to-mem device that supports multiplanar formats */
   { V4L2_CAP_VIDEO_M2M_MPLANE, "M2M multiplane" },
   /* Is a video mem-to-mem device */
   { V4L2_CAP_VIDEO_M2M, "M2M" },

   { V4L2_CAP_TUNER, "Tuner" },                             // has a tuner
   { V4L2_CAP_AUDIO, "Audio" },                             // has audio support
   { V4L2_CAP_RADIO, "Radio" },                             // is a radio device
   { V4L2_CAP_MODULATOR, "Modulator" },                     // has a modulator

   { V4L2_CAP_SDR_CAPTURE, "SDR Capture" },                 // Is a SDR capture device
   { V4L2_CAP_EXT_PIX_FORMAT, "Extended Pixelformat" },     // Supports the extended pixel format
   { V4L2_CAP_SDR_OUTPUT, "SDR Output" },                   // Is a SDR output device
   { V4L2_CAP_META_CAPTURE, "Metadata Capture" },           // Is a metadata capture device

   { V4L2_CAP_READWRITE, "read/write" },                    // read/write systemcalls
   { V4L2_CAP_ASYNCIO, "Async I/O" },                       // async I/O
   { V4L2_CAP_STREAMING, "Streaming" },                     // streaming I/O ioctls

#ifdef V4L2_CAP_META_OUTPUT
   { V4L2_CAP_META_OUTPUT, "metadata output" },             // Is a metadata output device
#endif
   
   { V4L2_CAP_TOUCH, "Touch" },                             // Is a touch device

   { V4L2_CAP_DEVICE_CAPS, "Device caps" },                 // sets device capabilities field
   { /* End */ },
   
  };


void gavl_v4l_device_info(const char * dev)
  {
  char * flag_str = NULL;
  int fd;
  int idx;
  
  struct v4l2_capability cap;
  
  if((fd = open(dev, O_RDWR /* required */ | O_NONBLOCK, 0)) < 0)
    {
    return;
    }

  if(my_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
    {
    
    }

  gavl_dprintf("Device:       %s\n", dev);
  gavl_dprintf("Driver:       %s\n", cap.driver);
  gavl_dprintf("Card:         %s\n", cap.card);
  gavl_dprintf("Bus info:     %s\n", cap.bus_info);
  gavl_dprintf("Version:      %d\n", cap.version);
  
  idx = 0;
  while(capabilities[idx].cap_flag)
    {
    if(capabilities[idx].cap_flag & cap.capabilities)
      {
      if(flag_str)
        flag_str = gavl_strcat(flag_str, ", ");
      flag_str = gavl_strcat(flag_str, capabilities[idx].name);
      }
    idx++;
    }

  gavl_dprintf("Capabilities: %s\n", flag_str);

  if(flag_str)
    free(flag_str);
  
  close(fd);

  gavl_dprintf("\n");
  
  }

void gavl_v4l_device_infos()
  {
  int i;
  char * dev;

  for(i = 0; i < 128; i++)
    {
    dev = gavl_sprintf("/dev/video%d", i);
    gavl_v4l_device_info(dev);
    free(dev);
    }
  
  }

  
/* v4l formats vs gavl_pixelformat_t and 
   gavl_compression_id_t */

static const struct
  {
  gavl_pixelformat_t pixelformat;

  uint32_t           v4l2;
  
  gavl_codec_id_t compression_id;
  }
pixelformats[] =
  {
    /*      Pixel format         FOURCC                        depth  Description  */
    // #define V4L2_PIX_FMT_RGB332  v4l2_fourcc('R','G','B','1') /*  8  RGB-3-3-2     */
    // #define V4L2_PIX_FMT_RGB444  v4l2_fourcc('R','4','4','4') /* 16  xxxxrrrr ggggbbbb */
    // #define V4L2_PIX_FMT_RGB555  v4l2_fourcc('R','G','B','O') /* 16  RGB-5-5-5     */
    // #define V4L2_PIX_FMT_RGB565  v4l2_fourcc('R','G','B','P') /* 16  RGB-5-6-5     */
    // #define V4L2_PIX_FMT_RGB555X v4l2_fourcc('R','G','B','Q') /* 16  RGB-5-5-5 BE  */
    // #define V4L2_PIX_FMT_RGB565X v4l2_fourcc('R','G','B','R') /* 16  RGB-5-6-5 BE  */
    // #define V4L2_PIX_FMT_BGR24   v4l2_fourcc('B','G','R','3') /* 24  BGR-8-8-8     */
   { V4L2_PIX_FMT_BGR24, GAVL_BGR_24, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_RGB24   v4l2_fourcc('R','G','B','3') /* 24  RGB-8-8-8     */
   { V4L2_PIX_FMT_RGB24, GAVL_RGB_24, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_BGR32   v4l2_fourcc('B','G','R','4') /* 32  BGR-8-8-8-8   */
   { V4L2_PIX_FMT_BGR32, GAVL_BGR_32, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_RGB32   v4l2_fourcc('R','G','B','4') /* 32  RGB-8-8-8-8   */
   { V4L2_PIX_FMT_RGB32, GAVL_RGB_32, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_GREY    v4l2_fourcc('G','R','E','Y') /*  8  Greyscale     */
   { V4L2_PIX_FMT_GREY, GAVL_GRAY_8,  GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_PAL8    v4l2_fourcc('P','A','L','8') /*  8  8-bit palette */
    // #define V4L2_PIX_FMT_YVU410  v4l2_fourcc('Y','V','U','9') /*  9  YVU 4:1:0     */
   { V4L2_PIX_FMT_YVU410, GAVL_YUV_410_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YVU420  v4l2_fourcc('Y','V','1','2') /* 12  YVU 4:2:0     */
   { V4L2_PIX_FMT_YVU420, GAVL_YUV_420_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YUYV    v4l2_fourcc('Y','U','Y','V') /* 16  YUV 4:2:2     */
   { V4L2_PIX_FMT_YUYV, GAVL_YUY2, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_UYVY    v4l2_fourcc('U','Y','V','Y') /* 16  YUV 4:2:2     */
   { V4L2_PIX_FMT_UYVY, GAVL_UYVY, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YUV422P v4l2_fourcc('4','2','2','P') /* 16  YVU422 planar */
   { V4L2_PIX_FMT_YUV422P, GAVL_YUV_422_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_YUV411P v4l2_fourcc('4','1','1','P') /* 16  YVU411 planar */
   { V4L2_PIX_FMT_YUV411P, GAVL_YUV_411_P, GAVL_CODEC_ID_NONE },
    // #define V4L2_PIX_FMT_Y41P    v4l2_fourcc('Y','4','1','P') /* 12  YUV 4:1:1     */
   { V4L2_PIX_FMT_Y41P, GAVL_YUV_411_P, GAVL_CODEC_ID_NONE},
    // #define V4L2_PIX_FMT_YUV444  v4l2_fourcc('Y','4','4','4') /* 16  xxxxyyyy uuuuvvvv */
    // #define V4L2_PIX_FMT_YUV555  v4l2_fourcc('Y','U','V','O') /* 16  YUV-5-5-5     */
    // #define V4L2_PIX_FMT_YUV565  v4l2_fourcc('Y','U','V','P') /* 16  YUV-5-6-5     */
    // #define V4L2_PIX_FMT_YUV32   v4l2_fourcc('Y','U','V','4') /* 32  YUV-8-8-8-8   */

/* two planes -- one Y, one Cr + Cb interleaved  */
    // #define V4L2_PIX_FMT_NV12    v4l2_fourcc('N','V','1','2') /* 12  Y/CbCr 4:2:0  */
    // #define V4L2_PIX_FMT_NV21    v4l2_fourcc('N','V','2','1') /* 12  Y/CrCb 4:2:0  */

/*  The following formats are not defined in the V4L2 specification */
    // #define V4L2_PIX_FMT_YUV410  v4l2_fourcc('Y','U','V','9') /*  9  YUV 4:1:0     */
    // #define V4L2_PIX_FMT_YUV420  v4l2_fourcc('Y','U','1','2') /* 12  YUV 4:2:0     */
   { V4L2_PIX_FMT_YUV420, GAVL_YUV_420_P, GAVL_CODEC_ID_NONE },
    
    // #define V4L2_PIX_FMT_YYUV    v4l2_fourcc('Y','Y','U','V') /* 16  YUV 4:2:2     */
    // #define V4L2_PIX_FMT_HI240   v4l2_fourcc('H','I','2','4') /*  8  8-bit color   */
    // #define V4L2_PIX_FMT_HM12    v4l2_fourcc('H','M','1','2') /*  8  YUV 4:2:0 16x16 macroblocks */

/* see http://www.siliconimaging.com/RGB%20Bayer.htm */
    // #define V4L2_PIX_FMT_SBGGR8  v4l2_fourcc('B','A','8','1') /*  8  BGBG.. GRGR.. */

/* compressed formats */

/* compressed formats */
// #define V4L2_PIX_FMT_MJPEG    v4l2_fourcc('M', 'J', 'P', 'G') /* Motion-JPEG   */

    
// #define V4L2_PIX_FMT_JPEG     v4l2_fourcc('J', 'P', 'E', 'G') /* JFIF JPEG     */
   { V4L2_PIX_FMT_JPEG, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_JPEG },

// #define V4L2_PIX_FMT_DV       v4l2_fourcc('d', 'v', 's', 'd') /* 1394          */
   { V4L2_PIX_FMT_JPEG, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_DV },

   // #define V4L2_PIX_FMT_MPEG     v4l2_fourcc('M', 'P', 'E', 'G') /* MPEG-1/2/4 Multiplexed */
    // #define V4L2_PIX_FMT_H264     v4l2_fourcc('H', '2', '6', '4') /* H264 with start codes */
   { V4L2_PIX_FMT_H264, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_H264 },
    // #define V4L2_PIX_FMT_H264_NO_SC v4l2_fourcc('A', 'V', 'C', '1') /* H264 without start codes */
    // #define V4L2_PIX_FMT_H264_MVC v4l2_fourcc('M', '2', '6', '4') /* H264 MVC */
    // #define V4L2_PIX_FMT_H263     v4l2_fourcc('H', '2', '6', '3') /* H263          */
    // #define V4L2_PIX_FMT_MPEG1    v4l2_fourcc('M', 'P', 'G', '1') /* MPEG-1 ES     */
   { V4L2_PIX_FMT_MPEG1, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_MPEG1 },
    // #define V4L2_PIX_FMT_MPEG2    v4l2_fourcc('M', 'P', 'G', '2') /* MPEG-2 ES     */
   { V4L2_PIX_FMT_MPEG2, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_MPEG2 },
    // #define V4L2_PIX_FMT_MPEG2_SLICE v4l2_fourcc('M', 'G', '2', 'S') /* MPEG-2 parsed slice data */
    // #define V4L2_PIX_FMT_MPEG4    v4l2_fourcc('M', 'P', 'G', '4') /* MPEG-4 part 2 ES */
   { V4L2_PIX_FMT_MPEG4, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_MPEG4_ASP },
    // #define V4L2_PIX_FMT_XVID     v4l2_fourcc('X', 'V', 'I', 'D') /* Xvid           */
    // #define V4L2_PIX_FMT_VC1_ANNEX_G v4l2_fourcc('V', 'C', '1', 'G') /* SMPTE 421M Annex G compliant stream */
    // #define V4L2_PIX_FMT_VC1_ANNEX_L v4l2_fourcc('V', 'C', '1', 'L') /* SMPTE 421M Annex L compliant stream */
    // #define V4L2_PIX_FMT_VP8      v4l2_fourcc('V', 'P', '8', '0') /* VP8 */
   { V4L2_PIX_FMT_VP8, GAVL_PIXELFORMAT_NONE, GAVL_CODEC_ID_VP8 },
    // #define V4L2_PIX_FMT_VP9      v4l2_fourcc('V', 'P', '9', '0') /* VP9 */
    // #define V4L2_PIX_FMT_HEVC     v4l2_fourcc('H', 'E', 'V', 'C') /* HEVC aka H.265 */
    // #define V4L2_PIX_FMT_FWHT     v4l2_fourcc('F', 'W', 'H', 'T') /* Fast Walsh Hadamard Transform (vicodec) */
    // #define V4L2_PIX_FMT_FWHT_STATELESS     v4l2_fourcc('S', 'F', 'W', 'H') /* Stateless FWHT (vicodec) */
    

    
/*  Vendor-specific formats   */
    // #define V4L2_PIX_FMT_WNVA     v4l2_fourcc('W','N','V','A') /* Winnov hw compress */
    // #define V4L2_PIX_FMT_SN9C10X  v4l2_fourcc('S','9','1','0') /* SN9C10x compression */
    // #define V4L2_PIX_FMT_PWC1     v4l2_fourcc('P','W','C','1') /* pwc older webcam */
    // #define V4L2_PIX_FMT_PWC2     v4l2_fourcc('P','W','C','2') /* pwc newer webcam */
    // #define V4L2_PIX_FMT_ET61X251 v4l2_fourcc('E','6','2','5') /* ET61X251 compression */
    
  };


   
struct gavl_v4l_device_s
  {
  int fd;
  
  };

gavl_v4l_device_t * gavl_v4l_device_open(const char * dev)
  {
  
  }

int gavl_v4l_device_reads_video(gavl_v4l_device_t * dev)
  {
  
  }

int gavl_v4l_device_writes_video(gavl_v4l_device_t *  dev)
  {
  
  }

int gavl_v4l_device_reads_packets(gavl_v4l_device_t * dev)
  {

  }

int gavl_v4l_device_writes_packets(gavl_v4l_device_t * dev)
  {
  
  }

gavl_packet_sink_t * gavl_v4l_device_get_packet_sink(gavl_v4l_device_t * dev)
  {

  }

gavl_packet_source_t * gavl_v4l_device_get_packet_source(gavl_v4l_device_t * dev)
  {

  }

gavl_video_sink_t * gavl_v4l_device_get_video_sink(gavl_v4l_device_t * dev)
  {
  }

gavl_video_source_t * gavl_v4l_device_get_video_source(gavl_v4l_device_t * dev)
  {

  }


int gavl_v4l_device_init_capture(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt)
  {
  
  } 

int gavl_v4l_device_init_output(gavl_v4l_device_t * dev,
                           gavl_video_format_t * fmt)
  {
  
  }

/* Unused for now */
#if 0
int v4l_device_init_encoder(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt,
                            gavl_compression_info_t * cmp);

#endif

int v4l_device_init_decoder(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt,
                            const gavl_compression_info_t * cmp)
  {
  
  }



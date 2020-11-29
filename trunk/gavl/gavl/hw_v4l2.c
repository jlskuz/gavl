
#include <config.h>

#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <glob.h>
#include <string.h>
#include <stdio.h>


#include <gavl/gavl.h>
#include <gavl/compression.h>
#include <gavl/metatags.h>
#include <gavl/trackinfo.h>

#include <gavl/log.h>
#define LOG_DOMAIN "v4l"

#include <gavl/hw_v4l2.h>


#include <hw_private.h>

typedef struct
  {
  int index;
  void * buf;
  int size;
  
  } buffer_t;
  
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

static void enum_formats(int fd, int type)
  {
  int idx = 0;
  struct v4l2_fmtdesc fmt;

  while(1)
    {
    fmt.index = idx++;
    fmt.type = type;

    if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
      {
      return;
      }
    
    gavl_dprintf("  Format %d: %c%c%c%c %s\n", idx,
                 fmt.pixelformat & 0xff,
                 (fmt.pixelformat>>8) & 0xff,
                 (fmt.pixelformat>>16) & 0xff,
                 (fmt.pixelformat>>24) & 0xff,
                 fmt.description);
    
    }
  }

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


  if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_M2M))
    {
    gavl_dprintf("Output formats\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT);
    }
  
  if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    gavl_dprintf("Output formats (planar)\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
    }

  if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_M2M))
    {
    gavl_dprintf("Capture formats\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE);
    }
  
  if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    gavl_dprintf("Capture formats (planar)\n");
    enum_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    }
  
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

static void query_formats(int fd, int buf_type, gavl_array_t * ret)
  {
  int idx = 0;
  struct v4l2_fmtdesc fmt;

  gavl_value_t val;
  gavl_dictionary_t * dict;
  
  while(1)
    {
    memset(&fmt, 0, sizeof(fmt));
    
    fmt.index = idx++;
    fmt.type = buf_type;
  
    if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
      break;

    gavl_value_init(&val);
    dict = gavl_value_set_dictionary(&val);

    gavl_dictionary_set_int(dict, GAVL_V4L_FORMAT_V4L_PIX_FMT, fmt.pixelformat);
    gavl_dictionary_set_int(dict, GAVL_V4L_FORMAT_V4L_FLAGS, fmt.flags);

    gavl_dictionary_set_string(dict, GAVL_META_LABEL, (char*)fmt.description);

    if(fmt.flags & V4L2_FMT_FLAG_COMPRESSED)
      gavl_dictionary_set_int(dict, GAVL_V4L_FORMAT_GAVL_CODEC_ID,
                              gavl_v4l_pix_fmt_to_codec_id(fmt.pixelformat));
    else
      gavl_dictionary_set_int(dict, GAVL_V4L_FORMAT_GAVL_PIXELFORMAT,
                              gavl_v4l_pix_fmt_to_pixelformat(fmt.pixelformat));

    gavl_array_splice_val_nocopy(ret, -1, 0, &val);
    }
  
  }

#if 0
static gavl_v4l_device_type_t detect_device_type(int fd, struct v4l2_capability * cap)
  {
  /* Encoder, Decoder, Converter Check in- and output formats */
  
  if(cap->capabilities & (V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_VIDEO_M2M))
    {
    int idx = 0;
    int reads_compressed = 0;
    int writes_compressed = 0;
    struct v4l2_fmtdesc fmt;

    if(cap->capabilities & V4L2_CAP_VIDEO_M2M_MPLANE)
      {
      idx = 0;
      
      while(1)
        {
        fmt.index = idx++;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        
        if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
          break;

        if(fmt.flags & V4L2_FMT_FLAG_COMPRESSED)
          {
          reads_compressed = 1;
          break;
          }
        }

      idx = 0;
      
      while(1)
        {
        fmt.index = idx++;
        fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
        
        if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
          break;

        if(fmt.flags & V4L2_FMT_FLAG_COMPRESSED)
          {
          writes_compressed = 1;
          break;
          }
        }
      
      }
    else if(cap->capabilities & V4L2_CAP_VIDEO_M2M)
      {
      
      idx = 0;
      
      while(1)
        {
        fmt.index = idx++;
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
        if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
          break;

        if(fmt.flags & V4L2_FMT_FLAG_COMPRESSED)
          {
          reads_compressed = 1;
          break;
          }
        }
      
      idx = 0;
      
      while(1)
        {
        fmt.index = idx++;
        fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
        
        if(my_ioctl(fd, VIDIOC_ENUM_FMT, &fmt) == -1)
          break;

        if(fmt.flags & V4L2_FMT_FLAG_COMPRESSED)
          {
          writes_compressed = 1;
          break;
          }
        }
      
      }

    if(reads_compressed && !writes_compressed)
      return GAVL_V4L_DEVICE_ENCODER;
    else if(!reads_compressed && writes_compressed)
      return GAVL_V4L_DEVICE_DECODER;
    else if(!reads_compressed && !writes_compressed)
      return GAVL_V4L_DEVICE_CONVERTER;
    else
      return GAVL_V4L_DEVICE_UNKNOWN;
    }

  else if(cap->capabilities & (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_CAPTURE))
    {
    return GAVL_V4L_DEVICE_SOURCE;
    }

  else if(cap->capabilities & (V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_OUTPUT))
    {
    return GAVL_V4L_DEVICE_SINK;
    }
  
  return GAVL_V4L_DEVICE_UNKNOWN;
  }

#endif

static const struct
  {
  gavl_v4l_device_type_t type;
  const char * name;
  }
device_types[] =
  {
    { GAVL_V4L_DEVICE_SOURCE,  "Source"  },
    { GAVL_V4L_DEVICE_SINK,    "Sink"    },
    { GAVL_V4L_DEVICE_ENCODER, "Encoder" },
    { GAVL_V4L_DEVICE_DECODER, "Decoder" },
    { GAVL_V4L_DEVICE_CONVERTER, "Converter" },
    { GAVL_V4L_DEVICE_UNKNOWN, "Unknown" },
    { },
  };

static const char * get_type_label(gavl_v4l_device_type_t type)
  {
  int idx = 0;
  
  while(device_types[idx].name)
    {
    if(type == device_types[idx].type)
      return device_types[idx].name;
    idx++;
    }
  return NULL;
  }

static int formats_compressed(const gavl_array_t * arr)
  {
  const gavl_dictionary_t * fmt;
  int flags;
  
  /* Just checking the first format should be sufficient */

  if(arr->num_entries < 1)
    return 0;
  
  if((fmt = gavl_value_get_dictionary(&arr->entries[0])) &&
     gavl_dictionary_get_int(fmt, GAVL_V4L_FORMAT_V4L_FLAGS, &flags) &&
     (flags & V4L2_FMT_FLAG_COMPRESSED))
    return 1;
  else
    return 0;
  }
  
void gavl_v4l_devices_scan_by_type(int type_mask, gavl_array_t * ret)
  {
  int i;
  glob_t g;

  gavl_value_t dev_val;
  gavl_dictionary_t * dev;

  gavl_array_t * src_formats;
  gavl_array_t * sink_formats;
  
  glob("/dev/video*", 0, NULL, &g);

  for(i = 0; i < g.gl_pathc; i++)
    {
    int fd;
    gavl_v4l_device_type_t type;
    
    struct v4l2_capability cap;
    memset(&cap, 0, sizeof(cap));

    src_formats = NULL;
    sink_formats = NULL;

    
    if((fd = open(g.gl_pathv[i], O_RDWR /* required */ | O_NONBLOCK, 0)) < 0)
      continue;
    
    if(my_ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1)
      {
      close(fd);
      continue;
      } 

    gavl_value_init(&dev_val);
    dev = gavl_value_set_dictionary(&dev_val);

    gavl_dictionary_set_string(dev, GAVL_META_LABEL, (const char*)cap.card);
    gavl_dictionary_set_string(dev, GAVL_META_URI, g.gl_pathv[i]);

    gavl_dictionary_set_int(dev, GAVL_V4L_CAPABILITIES, cap.capabilities);
    
    /* Get source formats */
    if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
      {
      src_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L_SRC_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE, src_formats);
      }
    else if(cap.capabilities & (V4L2_CAP_VIDEO_CAPTURE | V4L2_CAP_VIDEO_M2M))
      {
      src_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L_SRC_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_CAPTURE, src_formats);
      }
    
    /* Get output formats */
    if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT_MPLANE | V4L2_CAP_VIDEO_M2M_MPLANE))
      {
      sink_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L_SINK_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE, sink_formats);
      }
    else if(cap.capabilities & (V4L2_CAP_VIDEO_OUTPUT | V4L2_CAP_VIDEO_M2M))
      {
      sink_formats = gavl_dictionary_get_array_create(dev, GAVL_V4L_SINK_FORMATS);
      query_formats(fd, V4L2_BUF_TYPE_VIDEO_OUTPUT, sink_formats);
      }

    if(src_formats && !sink_formats)
      {
      type = GAVL_V4L_DEVICE_SOURCE;
      
      }
    else if(!src_formats && sink_formats)
      {
      type = GAVL_V4L_DEVICE_SINK;
      
      }
    else if(src_formats && sink_formats)
      {
      int src_compressed;
      int sink_compressed;
      
      src_compressed = formats_compressed(src_formats);
      sink_compressed = formats_compressed(sink_formats);

      if(!src_compressed && sink_compressed)
        type = GAVL_V4L_DEVICE_DECODER;
      else if(src_compressed && !sink_compressed)
        type = GAVL_V4L_DEVICE_ENCODER;
      else if(!src_compressed && !sink_compressed)
        type = GAVL_V4L_DEVICE_CONVERTER;
      else
        type = GAVL_V4L_DEVICE_UNKNOWN;
      }
    else
      type = GAVL_V4L_DEVICE_UNKNOWN;
    
    gavl_dictionary_set_int(dev, GAVL_V4L_TYPE, type);
    gavl_dictionary_set_string(dev, GAVL_V4L_TYPE_STRING, get_type_label(type));

    close(fd);
    
    if(type_mask && !(type & type_mask))
      {
      gavl_value_free(&dev_val);
      }
    else
      {
      gavl_array_splice_val_nocopy(ret, -1, 0, &dev_val);
      }
    
    }
  
  globfree(&g);
  }

const gavl_dictionary_t * gavl_v4l_get_decoder(const gavl_array_t * arr, gavl_codec_id_t id)
  {
  int i, j;

  const gavl_array_t * formats;
  
  const gavl_dictionary_t * dev;
  const gavl_dictionary_t * fmt;

  int type = GAVL_V4L_DEVICE_UNKNOWN;
  int codec_id;
  
  for(i = 0; i < arr->num_entries; i++)
    {
    if((dev = gavl_value_get_dictionary(&arr->entries[i])) &&
       gavl_dictionary_get_int(dev, GAVL_V4L_TYPE, &type) &&
       (type == GAVL_V4L_DEVICE_DECODER) &&
       (formats = gavl_dictionary_get_array(dev, GAVL_V4L_SINK_FORMATS)))
      {
      for(j = 0; j < formats->num_entries; j++)
        {
        if((fmt = gavl_value_get_dictionary(&formats->entries[j])) &&
           gavl_dictionary_get_int(fmt, GAVL_V4L_FORMAT_GAVL_CODEC_ID, &codec_id) &&
           (codec_id == id))
          return dev;
        }
      }
    }
  
  return NULL;
  }


void gavl_v4l_devices_scan(gavl_array_t * ret)
  {
  return gavl_v4l_devices_scan_by_type(0, ret);
  }

  
/* v4l formats vs gavl_pixelformat_t and 
   gavl_compression_id_t */

static const struct
  {
  uint32_t           v4l2;

  gavl_pixelformat_t pixelformat;

  
  gavl_codec_id_t codec_id;
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
   
   { },
  };

gavl_codec_id_t gavl_v4l_pix_fmt_to_codec_id(uint32_t fmt)
  {
  int idx = 0;

  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].v4l2 == fmt)
      return pixelformats[idx].codec_id;
    idx++;
    }
  return GAVL_CODEC_ID_NONE;
  }

uint32_t gavl_v4l_codec_id_to_pix_fmt(gavl_codec_id_t id)
  {
  int idx = 0;

  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].codec_id == id)
      return pixelformats[idx].v4l2;
    idx++;
    }
  return 0;
  
  }


gavl_pixelformat_t gavl_v4l_pix_fmt_to_pixelformat(uint32_t fmt)
  {
  int idx = 0;

  while(pixelformats[idx].v4l2)
    {
    if(pixelformats[idx].v4l2 == fmt)
      return pixelformats[idx].pixelformat;
    idx++;
    }
  return GAVL_PIXELFORMAT_NONE;
  }
   
struct gavl_v4l_device_s
  {
  gavl_dictionary_t dev;
  int fd;

  int is_planar;
  
  };

static int request_buffers_mmap(gavl_v4l_device_t * dev, int type, int count)
  {
  int i;
  
  struct v4l2_buffer buf;
  struct v4l2_requestbuffers req;

  memset(&req, 0, sizeof(req));
  
  req.count = count;
  req.type = type;
  req.memory = V4L2_MEMORY_MMAP;

  if(my_ioctl(dev->fd, VIDIOC_REQBUFS, &req) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Requesting buffers failed: %s", strerror(errno));
    return 0;
    }

  gavl_log(GAVL_LOG_INFO, LOG_DOMAIN, "Requested %d buffers, got %d", count, req.count);

  for(i = 0; i < req.count; i++)
    {
    memset(&buf, 0, sizeof(buf));
    buf.index = i;
    buf.type = type;

    if(my_ioctl(dev->fd, VIDIOC_QUERYBUF, &buf) == -1)
      {
      gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_QUERYBUF failed: %s", strerror(errno));
      return 0;
      }

    }
  
  
  }

gavl_v4l_device_t * gavl_v4l_device_open(const gavl_dictionary_t * dev)
  {
  gavl_v4l_device_t * ret = calloc(1, sizeof(*ret));
  
  const char * path;

  gavl_dictionary_copy(&ret->dev, dev);
  
  if(!(path = gavl_dictionary_get_string(dev, GAVL_META_URI)))
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "BUG: Path member missing");
    goto fail;
    }
    
  if((ret->fd = open(path, O_RDWR /* required */ | O_NONBLOCK, 0)) < 0)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "Couldn't open %s: %s", path, strerror(errno));
    goto fail;
    }
  
  
  return ret;

  fail:

  if(ret)
    gavl_v4l_device_close(ret);
  
  return NULL;
  }

int gavl_v4l_device_init_decoder(gavl_v4l_device_t * dev, gavl_dictionary_t * stream)
  {
  int caps = 0;
  int ret = 0;
  gavl_video_format_t * gavl_format;
  gavl_compression_info_t ci;
  struct v4l2_format fmt;
  gavl_stream_stats_t stats;
  int max_packet_size;

  int buf_type;
  
  memset(&fmt, 0, sizeof(fmt));
  
  fprintf(stderr, "gavl_v4l_device_init_decoder\n");
  gavl_dictionary_dump(stream, 2);

  gavl_format = gavl_stream_get_video_format_nc(stream);

  memset(&ci, 0, sizeof(ci));
  
  if(!gavl_stream_get_compression_info(stream, &ci))
    goto fail;

  gavl_stream_stats_init(&stats);
  gavl_stream_get_stats(stream, &stats);

  if(stats.size_max > 0)
    max_packet_size = stats.size_max;
  else
    max_packet_size = (gavl_format->image_width * gavl_format->image_width * 3) / 4 + 128;
  
  if(gavl_dictionary_get_int(&dev->dev, GAVL_V4L_CAPABILITIES, &caps) &&
     (caps & V4L2_CAP_VIDEO_M2M_MPLANE))
    {
    dev->is_planar = 1;
    fprintf(stderr, "Using planar API\n");

    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
          
    fmt.fmt.pix_mp.width = gavl_format->image_width;
    fmt.fmt.pix_mp.height = gavl_format->image_height;

    fmt.fmt.pix_mp.pixelformat = gavl_v4l_codec_id_to_pix_fmt(ci.id);
    fmt.fmt.pix_mp.colorspace = V4L2_COLORSPACE_DEFAULT;

    fmt.fmt.pix_mp.num_planes = 1;

    /* Estimate taken from ffmpeg. TODO: Used stream stats if available e.g. from mp4 files */
    
    fmt.fmt.pix_mp.plane_fmt[0].sizeimage = max_packet_size;
    //fmt.fmt.pix_mp.plane_fmt[0].bytesperline = 0;
        
    }
  else
    {
    /* Untested */
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    
    fmt.fmt.pix.width = gavl_format->image_width;
    fmt.fmt.pix.height = gavl_format->image_height;

    fmt.fmt.pix.pixelformat = gavl_v4l_codec_id_to_pix_fmt(ci.id);
    fmt.fmt.pix.colorspace = V4L2_COLORSPACE_DEFAULT;
    fmt.fmt.pix.sizeimage = max_packet_size;
    }
  
  if(my_ioctl(dev->fd, VIDIOC_S_FMT, &fmt) == -1)
    {
    gavl_log(GAVL_LOG_ERROR, LOG_DOMAIN, "VIDIOC_S_FMT failed: %s", strerror(errno));
    goto fail;
    }

  if(dev->is_planar)
    buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
  else
    buf_type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  
  if(!request_buffers_mmap(dev, buf_type, 4))
    goto fail;
     
  /* */
    
  //  ret = 1;
  fail:
  
  
  return ret;
  
  }

int gavl_v4l_device_get_fd(gavl_v4l_device_t * dev)
  {
  return dev->fd;
  }

void gavl_v4l_device_close(gavl_v4l_device_t * dev)
  {
  if(dev->fd >= 0)
    close(dev->fd);

  gavl_dictionary_free(&dev->dev);
  
  free(dev);
  }

#if 0

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
int v4l_device_init_encoder(gavl_v4l_device_t * dev,
                            gavl_video_format_t * fmt,
                            gavl_compression_info_t * cmp);

#endif



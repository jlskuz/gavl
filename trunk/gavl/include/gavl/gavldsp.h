/** \defgroup dsp DSP Context
 *  \brief DSP Context
 *
 *  In addition to the higher level gavl converters,
 *  gavl provides some low-level dsp routines.
 *  They can be used for writing filters or codecs.
 *  The DSP context is initialized with a quality parameter
 *  (see \ref quality), which selects among various versions
 *  of a function.
 *
 *  @{
 */

/** \brief Opaque DSP context
 *
 *  You don't want to know, what's inside here.
 */

typedef struct gavl_dsp_context_s gavl_dsp_context_t;

/** \brief Function table
 *
 *  This structure contains pointers to the available DSP
 *  functions.
 */

typedef struct
  {
  /** \brief Get the sum of absolute differences (RGB/BGR15)
   *  \param src_1 Plane 1
   *  \param src_2 Plane 2
   *  \param stride_1 Byte distance between scanlines for src_1
   *  \param stride_2 Byte distance between scanlines for src_2
   *  \param w Width
   *  \param h Height
   *  \returns The sum of absolute differences
   *
   *  The RGB values will be scaled to 8 bit before the
   *  differences are calculated.
   */
  
  int (*sad_rgb15)(uint8_t * src_1, uint8_t * src_2, 
                   int stride_1, int stride_2, 
                   int w, int h);

  /** \brief Get the sum of absolute differences (RGB/BGR16)
   *  \param src_1 Plane 1
   *  \param src_2 Plane 2
   *  \param stride_1 Byte distance between scanlines for src_1
   *  \param stride_2 Byte distance between scanlines for src_2
   *  \param w Width
   *  \param h Height
   *  \returns The sum of absolute differences
   *
   *  The RGB values will be scaled to 8 bit before the
   *  differences are calculated.
   */

  int (*sad_rgb16)(uint8_t * src_1, uint8_t * src_2, 
                   int stride_1, int stride_2, 
                   int w, int h);

  /** \brief Get the sum of absolute differences (8 bit)
   *  \param src_1 Plane 1
   *  \param src_2 Plane 2
   *  \param stride_1 Byte distance between scanlines for src_1
   *  \param stride_2 Byte distance between scanlines for src_2
   *  \param w Width
   *  \param h Height
   *  \returns The sum of absolute differences
   */

  int (*sad_8)(uint8_t * src_1, uint8_t * src_2, 
               int stride_1, int stride_2, 
               int w, int h);

  /** \brief Get the sum of absolute differences (16 bit)
   *  \param src_1 Plane 1
   *  \param src_2 Plane 2
   *  \param stride_1 Byte distance between scanlines for src_1
   *  \param stride_2 Byte distance between scanlines for src_2
   *  \param w Width
   *  \param h Height
   *  \returns The sum of absolute differences
   */

  int (*sad_16)(uint8_t * src_1, uint8_t * src_2, 
               int stride_1, int stride_2, 
               int w, int h);

  /** \brief Get the sum of absolute differences (float)
   *  \param src_1 Plane 1
   *  \param src_2 Plane 2
   *  \param stride_1 Byte distance between scanlines for src_1
   *  \param stride_2 Byte distance between scanlines for src_2
   *  \param w Width
   *  \param h Height
   *  \returns The sum of absolute differences
   */
  float (*sad_f)(uint8_t * src_1, uint8_t * src_2, 
                 int stride_1, int stride_2, 
                 int w, int h);

  /** \brief Average 2 scanlines (RGB/BGR15)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of pixels
   */
  
  void (*average_rgb15)(uint8_t * src_1, uint8_t * src_2, 
                        uint8_t * dst, int num);

  /** \brief Average 2 scanlines (RGB/BGR16)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of pixels
   */
  void (*average_rgb16)(uint8_t * src_1, uint8_t * src_2, 
                        uint8_t * dst, int num);

  /** \brief Average 2 scanlines (8 bit)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of bytes
   */
  void (*average_8)(uint8_t * src_1, uint8_t * src_2, 
                    uint8_t * dst, int num);

  /** \brief Average 2 scanlines (16 bit)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of shorts
   */
  void (*average_16)(uint8_t * src_1, uint8_t * src_2, 
                     uint8_t * dst, int num);

  /** \brief Average 2 scanlines (float)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of floats
   */
  
  void (*average_f)(uint8_t * src_1, uint8_t * src_2, 
                    uint8_t * dst, int num);


  /** \brief Interpolate 2 scanlines (RGB/BGR15)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of pixels
   *  \param fac Factor for src_1 (0..255)
   */
  
  void (*interpolate_rgb15)(uint8_t * src_1, uint8_t * src_2, 
                            uint8_t * dst, int num, int fac);

  /** \brief Interpolate 2 scanlines (RGB/BGR16)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of pixels
   *  \param fac Factor for src_1 (0..255)
   */
  void (*interpolate_rgb16)(uint8_t * src_1, uint8_t * src_2, 
                            uint8_t * dst, int num, int fac);

  /** \brief Interpolate 2 scanlines (8 bit)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of bytes
   *  \param fac Factor for src_1 (0..255)
   */
  void (*interpolate_8)(uint8_t * src_1, uint8_t * src_2, 
                        uint8_t * dst, int num, int fac);

  /** \brief Interpolate 2 scanlines (16 bit)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of shorts
   *  \param fac Factor for src_1 (0..65535)
   */
  void (*interpolate_16)(uint8_t * src_1, uint8_t * src_2, 
                         uint8_t * dst, int num, int fac);

  /** \brief Interpolate 2 scanlines (float)
   *  \param src_1 Scanline 1
   *  \param src_2 Scanline 2
   *  \param dst Destination
   *  \param num Number of floats
   *  \param fac Factor for src_1 (0.0..1.0)
   */
  
  void (*interpolate_f)(uint8_t * src_1, uint8_t * src_2, 
                        uint8_t * dst, int num, float fac);
  
  } gavl_dsp_funcs_t;

/** \brief Create a DSP context
 *  \returns A newly allocated DSP context
 *
 *  This initialized a DSP context with all functions
 *  chosen according to the default quality value.
 */

gavl_dsp_context_t * gavl_dsp_context_create();

/** \brief Set the quality for a DSP context
 *  \param ctx A DSP context
 *  \param q A quality level
 *
 *  See \ref quality.
 */

void gavl_dsp_context_set_quality(gavl_dsp_context_t * ctx,
                                  int q);

/** \brief Get the functions
 *  \param ctx A DSP context
 *  \returns The function table
 *
 *  The address of the table itself won't change by a call
 *  to \ref gavl_dsp_context_set_quality, but the contained
 *  functions will.
 */

gavl_dsp_funcs_t * 
gavl_dsp_context_get_funcs(gavl_dsp_context_t * ctx);

/** \brief Destroy a DSP context
 *  \param ctx A DSP context
 */

void gavl_dsp_context_destroy(gavl_dsp_context_t * ctx);

/** @}
 */

/** \defgroup dsputils DSP Utilities
 *  \ingroup dsp
 *  \brief DSP Utilities
 *
 *  These are some utility functions, which use a DSP context but
 *  operate on higher level structures like video frames.
 *  As usual, these are supported for any format.
 *
 *  @{
 */

/** \brief Do a linear interpolation of a video frame
 *  \param ctx A DSP context
 *  \param format Video format
 *  \param src1 Frame 1
 *  \param src2 Frame 2
 *  \param dst  Destination frame
 *  \param factor Interpolation factor
 *
 *  If factor is 1.0, dst will be equal to src1,
 *  if factor is 0.0, dst will be equal to src2.
 */

void gavl_dsp_interpolate_video_frame(gavl_dsp_context_t * ctx,
                                      gavl_video_format_t * format,
                                      gavl_video_frame_t * src_1,
                                      gavl_video_frame_t * src_2,
                                      gavl_video_frame_t * dst,
                                      float factor);
/**
 * @}
 */

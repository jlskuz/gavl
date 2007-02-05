/*****************************************************************

  videooptions.c

  Copyright (c) 2001 by Burkhard Plaum - plaum@ipf.uni-stuttgart.de

  http://gmerlin.sourceforge.net

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

#include <stdlib.h> /* calloc, free */
#include <string.h> /* calloc, free */

#ifdef DEBUG
#include <stdio.h>  
#endif

#include <gavl.h>
#include <config.h>
#include <video.h>
#include <accel.h>

/***************************************************
 * Default Options
 ***************************************************/

void gavl_video_options_set_defaults(gavl_video_options_t * opt)
  {
  memset(opt, 0, sizeof(*opt));
  opt->accel_flags = gavl_accel_supported();
  opt->scale_order = 4;
  opt->quality = GAVL_QUALITY_DEFAULT;
  gavl_init_memcpy();
  }
  
void gavl_video_options_set_rectangles(gavl_video_options_t * opt,
                                       const gavl_rectangle_f_t * src_rect,
                                       const gavl_rectangle_i_t * dst_rect)
  {
  if(!src_rect || !dst_rect)
    opt->have_rectangles = 0;
  
  gavl_rectangle_f_copy(&(opt->src_rect), src_rect);
  gavl_rectangle_i_copy(&(opt->dst_rect), dst_rect);
  opt->have_rectangles = 1;
  }

#define SET_INT(p) opt->p = p


void gavl_video_options_set_quality(gavl_video_options_t * opt, int quality)
  {
  SET_INT(quality);
  }

void gavl_video_options_set_accel_flags(gavl_video_options_t * opt,
                                        int accel_flags)
  {
  SET_INT(accel_flags);
  }

void gavl_video_options_set_conversion_flags(gavl_video_options_t * opt,
                                             int conversion_flags)
  {
  SET_INT(conversion_flags);
  }

void gavl_video_options_set_alpha_mode(gavl_video_options_t * opt,
                                       gavl_alpha_mode_t alpha_mode)
  {
  SET_INT(alpha_mode);
  }

void gavl_video_options_set_scale_mode(gavl_video_options_t * opt,
                                       gavl_scale_mode_t scale_mode)
  {
  SET_INT(scale_mode);
  }

void gavl_video_options_set_scale_order(gavl_video_options_t * opt,
                                        int scale_order)
  {
  SET_INT(scale_order);
  }

void
gavl_video_options_set_deinterlace_mode(gavl_video_options_t * opt,
                                        gavl_deinterlace_mode_t deinterlace_mode)
  {
  SET_INT(deinterlace_mode);
  }

void
gavl_video_options_set_deinterlace_drop_mode(gavl_video_options_t * opt,
                                             gavl_deinterlace_drop_mode_t deinterlace_drop_mode)
  {
  SET_INT(deinterlace_drop_mode);
  }

#undef SET_INT

#define CLIP_FLOAT(a) if(a < 0.0) a = 0.0; if(a>1.0) a = 1.0;

void gavl_video_options_set_background_color(gavl_video_options_t * opt,
                                             float * color)
  {
  memcpy(opt->background_float, color, 3*sizeof(*color));

  CLIP_FLOAT(opt->background_float[0]);
  CLIP_FLOAT(opt->background_float[1]);
  CLIP_FLOAT(opt->background_float[2]);
  opt->background_16[0] = (uint16_t)(opt->background_float[0] * 65535.0 + 0.5);
  opt->background_16[1] = (uint16_t)(opt->background_float[1] * 65535.0 + 0.5);
  opt->background_16[2] = (uint16_t)(opt->background_float[2] * 65535.0 + 0.5);
  
  }

void gavl_video_options_get_background_color(gavl_video_options_t * opt,
                                             float * color)
  {
  memcpy(color, opt->background_float, 3*sizeof(*color));
  }

int gavl_video_options_get_accel_flags(gavl_video_options_t * opt)
  {
  return opt->accel_flags;
  }


int gavl_video_options_get_conversion_flags(gavl_video_options_t * opt)
  {
  return opt->conversion_flags;
  }

gavl_video_options_t * gavl_video_options_create()
  {
  gavl_video_options_t * ret = calloc(1, sizeof(*ret));
  gavl_video_options_set_defaults(ret);
  return ret;
  }

void gavl_video_options_copy(gavl_video_options_t * dst,
                             const gavl_video_options_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

void gavl_video_options_destroy(gavl_video_options_t * opt)
  {
  free(opt);
  }

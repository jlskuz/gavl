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

#include "gavl.h"
#include "config.h"
#include "video.h"

/***************************************************
 * Default Options
 ***************************************************/

void gavl_video_options_set_defaults(gavl_video_options_t * opt)
  {
  memset(opt, 0, sizeof(*opt));
  opt->accel_flags = GAVL_ACCEL_C;
  }

void gavl_video_options_copy(gavl_video_options_t * dst,
                                    const gavl_video_options_t * src)
  {
  memcpy(dst, src, sizeof(*dst));
  }

  
void gavl_video_options_set_rectangles(gavl_video_options_t * opt,
                                       const gavl_rectangle_t * src_rect,
                                       const gavl_rectangle_t * dst_rect)
  {
  gavl_rectangle_copy(&(opt->src_rect), src_rect);
  gavl_rectangle_copy(&(opt->dst_rect), dst_rect);
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

#undef SET_INT


void gavl_video_options_set_background_color(gavl_video_options_t * opt,
                                             float * color)
  {
  int i_tmp;

  i_tmp = (int)(color[0] * 65535.0 + 0.5);
  if(i_tmp < 0)      i_tmp = 0;
  if(i_tmp > 0xffff) i_tmp = 0xffff;
  opt->background_red = i_tmp;

  i_tmp = (int)(color[1] * 65535.0 + 0.5);
  if(i_tmp < 0)      i_tmp = 0;
  if(i_tmp > 0xffff) i_tmp = 0xffff;
  opt->background_green = i_tmp;
  
  i_tmp = (int)(color[2] * 65535.0 + 0.5);
  if(i_tmp < 0)      i_tmp = 0;
  if(i_tmp > 0xffff) i_tmp = 0xffff;
  opt->background_blue = i_tmp;
  }

int gavl_video_options_get_accel_flags(gavl_video_options_t * opt)
  {
  return opt->accel_flags;
  }


int gavl_video_options_get_conversion_flags(gavl_video_options_t * opt)
  {
  return opt->conversion_flags;
  }

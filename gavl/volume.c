/*****************************************************************

  volume.c

  Copyright (c) 2004 by Burkhard Plaum - plaum@ipf.uni-stuttgart.de

  http://gmerlin.sourceforge.net

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.

*****************************************************************/

#include <stdlib.h>
#include <math.h>

#include <config.h>
#include <gavl.h>
#include <volume.h>

struct gavl_volume_control_s
  {
  gavl_audio_format_t format;
  
  float factor;
    
  void (*set_volume)(gavl_volume_control_t * v,
                     gavl_audio_frame_t * frame);
  
  void (*set_volume_channel)(void * samples, float factor,
                             int num_samples);
  
  };


static void set_volume_interleave_none(gavl_volume_control_t * v,
                                       gavl_audio_frame_t * frame)
  {
  int i;
  /* All channels separate */

  for(i = 0; i < v->format.num_channels; i++)
    {
    v->set_volume_channel(frame->channels.s_8[i], v->factor,
                          frame->valid_samples);
    }
  }

static void set_volume_interleave_2(gavl_volume_control_t * v,
                                    gavl_audio_frame_t * frame)
  {
  int i;
  int imax;

  imax = v->format.num_channels/2;
  
  for(i = 0; i < imax; i++)
    {
    v->set_volume_channel(frame->channels.s_8[2*i], v->factor,
                          frame->valid_samples * 2);
    }

  if(v->format.num_channels % 2)
    {
    v->set_volume_channel(frame->channels.s_8[2*imax], v->factor,
                          frame->valid_samples);
    }
  }

static void set_volume_interleave_all(gavl_volume_control_t * v,
                                      gavl_audio_frame_t * frame)
  {
  v->set_volume_channel(frame->samples.s_8, v->factor,
                        frame->valid_samples * v->format.num_channels);
  }


/* Create / destroy */
  
gavl_volume_control_t * gavl_volume_control_create()
  {
  gavl_volume_control_t * v;
  v = calloc(1, sizeof(*v));
  return v;
  }

void gavl_volume_control_destroy(gavl_volume_control_t * v)
  {
  free(v);
  }

/* Set format: can be called multiple times with one instance */

void gavl_volume_control_set_format(gavl_volume_control_t * v,
                                    gavl_audio_format_t * format)
  {
  gavl_volume_funcs_t * funcs;

  gavl_audio_format_copy(&(v->format), format);
  
  funcs = gavl_volume_funcs_create();
  
  switch(format->sample_format)
    {
    case GAVL_SAMPLE_S8:
      v->set_volume_channel = funcs->set_volume_s8;
      break;

    case GAVL_SAMPLE_U8:
      v->set_volume_channel = funcs->set_volume_u8;
      break;

    case GAVL_SAMPLE_S16:
      v->set_volume_channel = funcs->set_volume_s16;
      break;

    case GAVL_SAMPLE_U16:
      v->set_volume_channel = funcs->set_volume_u16;
      break;

    case GAVL_SAMPLE_S32:
      v->set_volume_channel = funcs->set_volume_s32;
      break;

    case GAVL_SAMPLE_FLOAT:
      v->set_volume_channel = funcs->set_volume_float;
      break;

    case GAVL_SAMPLE_NONE:
      break;
    }

  gavl_volume_funcs_destroy(funcs);

  switch(format->interleave_mode)
    {
    case GAVL_INTERLEAVE_NONE:
      v->set_volume = set_volume_interleave_none;
      break;
    case GAVL_INTERLEAVE_2:
      v->set_volume = set_volume_interleave_2;
      break;
    case GAVL_INTERLEAVE_ALL:
      v->set_volume = set_volume_interleave_all;
      break;
    }
  }

/* Apply the volume control to one audio frame */
  
void gavl_volume_control_apply(gavl_volume_control_t * v,
                               gavl_audio_frame_t * frame)
  {
  v->set_volume(v, frame);
  }

void gavl_volume_control_set_volume(gavl_volume_control_t * v,
                                    float volume)
  {
  v->factor = pow(10, volume/20.0);
  }


gavl_volume_funcs_t * gavl_volume_funcs_create()
  {
  gavl_volume_funcs_t * ret;
  ret = calloc(1, sizeof(*ret));
  
  gavl_init_volume_funcs_c(ret);

#ifdef ARCH_X86
  //  gavl_init_volume_funcs_mmx(ret);
#endif
  
  return ret;
  }

void gavl_volume_funcs_destroy(gavl_volume_funcs_t * funcs)
  {
  free(funcs);
  }

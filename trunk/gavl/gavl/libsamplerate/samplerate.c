/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"config.h"

#include	"samplerate.h"
#include	"common.h"
#include	"float_cast.h"

static int psrc_set_converter (SRC_PRIVATE	*psrc, int converter_type, int d) ;


SRC_STATE *
gavl_src_new (int converter_type, int channels, int *error, int d)
{	SRC_PRIVATE	*psrc ;

	if (error)
		*error = SRC_ERR_NO_ERROR ;

	if (channels < 1)
	{	if (error)
			*error = SRC_ERR_BAD_CHANNEL_COUNT ;
		return NULL ;
		} ;

	if ((psrc = calloc (1, sizeof (*psrc))) == NULL)
	{	if (error)
			*error = SRC_ERR_MALLOC_FAILED ;
		return NULL ;
		} ;

	psrc->channels = channels ;
	psrc->mode = SRC_MODE_PROCESS ;

	if (psrc_set_converter (psrc, converter_type, d) != SRC_ERR_NO_ERROR)
	{	if (error)
			*error = SRC_ERR_BAD_CONVERTER ;
		free (psrc) ;
		psrc = NULL ;
		} ;

	gavl_src_reset ((SRC_STATE*) psrc) ;

	return (SRC_STATE*) psrc ;
} /* gavl_src_new */

SRC_STATE*
gavl_src_callback_new (src_callback_t func, int converter_type, int channels, int *error, void* cb_data)
{	SRC_STATE	*src_state ;

	if (func == NULL)
	{	if (error)
			*error = SRC_ERR_BAD_CALLBACK ;
		return NULL ;
		} ;

	if (error != NULL)
		*error = 0 ;

	src_state = gavl_src_new (converter_type, channels, error, 0) ;

	gavl_src_reset (src_state) ;

	((SRC_PRIVATE*) src_state)->mode = SRC_MODE_CALLBACK ;
	((SRC_PRIVATE*) src_state)->callback_func = func ;
	((SRC_PRIVATE*) src_state)->user_callback_data = cb_data ;

	return src_state ;
} /* gavl_src_callback_new */

SRC_STATE *
gavl_src_delete (SRC_STATE *state)
{	SRC_PRIVATE *psrc ;

	psrc = (SRC_PRIVATE*) state ;
	if (psrc)
	{	if (psrc->private_data)
			free (psrc->private_data) ;
		memset (psrc, 0, sizeof (SRC_PRIVATE)) ;
		free (psrc) ;
		} ;

	return NULL ;
} /* src_state */

int
gavl_src_process (SRC_STATE *state, SRC_DATA *data)
{	SRC_PRIVATE *psrc ;
	int error ;

	psrc = (SRC_PRIVATE*) state ;

	if (psrc == NULL)
		return SRC_ERR_BAD_STATE ;
	if (psrc->process == NULL)
		return SRC_ERR_BAD_PROC_PTR ;

	if (psrc->mode != SRC_MODE_PROCESS)
		return SRC_ERR_BAD_MODE ;

	/* Check for valid SRC_DATA first. */
	if (data == NULL)
		return SRC_ERR_BAD_DATA ;

	/* Check src_ratio is in range. */
	if (data->src_ratio < (1.0 / SRC_MAX_RATIO) || data->src_ratio > (1.0 * SRC_MAX_RATIO))
		return SRC_ERR_BAD_SRC_RATIO ;

	if (data->input_frames < 0)
		data->input_frames = 0 ;
	if (data->output_frames < 0)
		data->output_frames = 0 ;

	/* Set the input and output counts to zero. */
	data->input_frames_used = 0 ;
	data->output_frames_gen = 0 ;

	/* Special case for when last_ratio has not been set. */
	if (psrc->last_ratio < (1.0 / SRC_MAX_RATIO))
		psrc->last_ratio = data->src_ratio ;

	/* Now process. */
	error = psrc->process (psrc, data) ;

	return error ;
} /* gavl_src_process */

/*==========================================================================
*/

int
gavl_src_set_ratio (SRC_STATE *state, double new_ratio)
{	SRC_PRIVATE *psrc ;

	psrc = (SRC_PRIVATE*) state ;

	if (psrc == NULL)
		return SRC_ERR_BAD_STATE ;
	if (psrc->process == NULL)
		return SRC_ERR_BAD_PROC_PTR ;

	psrc->last_ratio = new_ratio ;

	return SRC_ERR_NO_ERROR ;
} /* gavl_src_set_ratio */

int
gavl_src_reset (SRC_STATE *state)
{	SRC_PRIVATE *psrc ;

	if ((psrc = (SRC_PRIVATE*) state) == NULL)
		return SRC_ERR_BAD_STATE ;

	if (psrc->reset != NULL)
		psrc->reset (psrc) ;

	psrc->last_position = 0.0 ;
	psrc->last_ratio = 0.0 ;

	psrc->saved_data = NULL ;
	psrc->saved_frames = 0 ;

	psrc->error = SRC_ERR_NO_ERROR ;

	return SRC_ERR_NO_ERROR ;
} /* gavl_src_reset */

/*==============================================================================
**	Control functions.
*/

const char *
gavl_src_get_name (int converter_type)
{	const char *desc ;

	if ((desc = gavl_sinc_get_name (converter_type)) != NULL)
		return desc ;

	if ((desc = gavl_zoh_get_name (converter_type)) != NULL)
		return desc ;

	if ((desc = gavl_linear_get_name (converter_type)) != NULL)
		return desc ;

	return NULL ;
} /* gavl_src_get_name */

const char *
gavl_src_get_description (int converter_type)
{	const char *desc ;

	if ((desc = gavl_sinc_get_description (converter_type)) != NULL)
		return desc ;

	if ((desc = gavl_zoh_get_description (converter_type)) != NULL)
		return desc ;

	if ((desc = gavl_linear_get_description (converter_type)) != NULL)
		return desc ;

	return NULL ;
} /* gavl_src_get_description */

const char *
gavl_src_get_version (void)
{	return PACKAGE "-" VERSION ;
} /* gavl_src_get_version */

int
gavl_src_is_valid_ratio (double ratio)
{
	if (ratio < (1.0 / SRC_MAX_RATIO) || ratio > (1.0 * SRC_MAX_RATIO))
		return SRC_FALSE ;

	return SRC_TRUE ;
} /* gavl_src_is_valid_ratio */

/*==============================================================================
**	Error reporting functions.
*/

int
gavl_src_error (SRC_STATE *state)
{	if (state)
		return ((SRC_PRIVATE*) state)->error ;
	return SRC_ERR_NO_ERROR ;
} /* gavl_src_error */

const char*
gavl_src_strerror (int error)
{
	switch (error)
	{	case SRC_ERR_NO_ERROR :
				return "No error." ;
		case SRC_ERR_MALLOC_FAILED :
				return "Malloc failed." ;
		case SRC_ERR_BAD_STATE :
				return "SRC_STATE pointer is NULL." ;
		case SRC_ERR_BAD_DATA :
				return "SRC_DATA pointer is NULL." ;
		case SRC_ERR_BAD_DATA_PTR :
				return "SRC_DATA->data_out is NULL." ;
		case SRC_ERR_NO_PRIVATE :
				return "Internal error. No private data." ;
		case SRC_ERR_BAD_SRC_RATIO :
				return "SRC ratio outside [1/12, 12] range." ;
		case SRC_ERR_BAD_SINC_STATE :
				return "gavl_src_process() called without reset after end_of_input." ;
		case SRC_ERR_BAD_PROC_PTR :
				return "Internal error. No process pointer." ;
		case SRC_ERR_SHIFT_BITS :
				return "Internal error. SHIFT_BITS too large." ;
		case SRC_ERR_FILTER_LEN :
				return "Internal error. Filter length too large." ;
		case SRC_ERR_BAD_CONVERTER :
				return "Bad converter number." ;
		case SRC_ERR_BAD_CHANNEL_COUNT :
				return "Channel count must be >= 1." ;
		case SRC_ERR_SINC_BAD_BUFFER_LEN :
				return "Internal error. Bad buffer length. Please report this." ;
		case SRC_ERR_SIZE_INCOMPATIBILITY :
				return "Internal error. Input data / internal buffer size difference. Please report this." ;
		case SRC_ERR_BAD_PRIV_PTR :
				return "Internal error. Private pointer is NULL. Please report this." ;
		case SRC_ERR_DATA_OVERLAP :
				return "Input and output data arrays overlap." ;
		case SRC_ERR_BAD_CALLBACK :
				return "Supplied callback function pointer is NULL." ;
		case SRC_ERR_BAD_MODE :
				return "Calling mode differs from initialisation mode (ie process v callback)." ;
		case SRC_ERR_NULL_CALLBACK :
				return "Callback function pointer is NULL in gavl_src_callback_read ()." ;

		case SRC_ERR_MAX_ERROR :
				return "Placeholder. No error defined for this error number." ;

		default : 						break ;
		}

	return NULL ;
} /* gavl_src_strerror */

/*==============================================================================
**	Simple interface for performing a single conversion from input buffer to
**	output buffer at a fixed conversion ratio.
*/

void
gavl_src_short_to_float_array (const short *in, float *out, int len)
{
	while (len)
	{	len -- ;
		out [len] = in [len] / (1.0 * 0x8000) ;
		} ;

	return ;
} /* gavl_src_short_to_float_array */

void
gavl_src_float_to_short_array (const float *in, short *out, int len)
{	float scaled_value ;

	while (len)
	{	len -- ;

		scaled_value = in [len] * (8.0 * 0x10000000) ;
		if (CPU_CLIPS_POSITIVE == 0 && scaled_value >= (1.0 * 0x7FFFFFFF))
		{	out [len] = 32767 ;
			continue ;
			} ;
		if (CPU_CLIPS_NEGATIVE == 0 && scaled_value <= (-8.0 * 0x10000000))
		{	out [len] = -32768 ;
			continue ;
			} ;

		out [len] = (lrintf (scaled_value) >> 16) ;
		} ;

} /* gavl_src_float_to_short_array */

/*==============================================================================
**	Private functions.
*/

static int
psrc_set_converter (SRC_PRIVATE	*psrc, int converter_type, int d)
{
	if (gavl_sinc_set_converter (psrc, converter_type, d) == SRC_ERR_NO_ERROR)
		return SRC_ERR_NO_ERROR ;

	if (gavl_zoh_set_converter (psrc, converter_type, d) == SRC_ERR_NO_ERROR)
		return SRC_ERR_NO_ERROR ;

	if (gavl_linear_set_converter (psrc, converter_type, d) == SRC_ERR_NO_ERROR)
		return SRC_ERR_NO_ERROR ;

	return SRC_ERR_BAD_CONVERTER ;
} /* psrc_set_converter */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: a5c5f514-a370-4210-a066-7f2035de67fb
*/


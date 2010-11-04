/**----------------------------------------------------------------------------

 Fennec DSP Plug-in 1.0 (Multichannel).
 Copyright (C) 2009 Chase <c-h@users.sf.net>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

----------------------------------------------------------------------------**/

#include "effect.h"




/* local prototypes ---------------------------------------------------------*/

static fennec_sample  delay(fennec_sample ins, int channel);




/* data ---------------------------------------------------------------------*/

struct speaker_details    speakers;




/* code ---------------------------------------------------------------------*/



int sp_init(void)
{
	return 0;
}

int sp_uninit(void)
{
	return 0;
}

/*
 * process one combined sample (block - 1 to 8 sample block).
 */
void sp_process(fennec_sample *block_out, fennec_sample  *block_in, int in_channels)
{
	fennec_sample   s_dummy;
	fennec_sample  *os_front_left   = &block_out[0];
	fennec_sample  *os_front_right  = &block_out[1];
	fennec_sample  *os_front_center;
	fennec_sample  *os_subwoofer;
	fennec_sample  *os_rear_left;
	fennec_sample  *os_rear_right;
	fennec_sample  *os_side_left;
	fennec_sample  *os_side_right;
	fennec_sample  in_front_left    = block_in[speakers.mapping.front_left];
	fennec_sample  in_front_right   = block_in[speakers.mapping.front_right];
	fennec_sample  in_front_center;
	fennec_sample  in_subwoofer;
	fennec_sample  in_rear_left;
	fennec_sample  in_rear_right;
	fennec_sample  in_side_left;
	fennec_sample  in_side_right;

	if(speakers.count >= 5)
	{
		os_front_center = &block_out[2];
		os_subwoofer    = &block_out[3];
		os_rear_left    = &block_out[4];
		os_rear_right   = &block_out[5];

	}else{
		os_front_center = &s_dummy;
		os_subwoofer    = &s_dummy;
		os_rear_left    = &s_dummy;
		os_rear_right   = &s_dummy;
		
	}

	if(in_channels >= 5)
	{
		in_front_center = block_in[speakers.mapping.front_center];
		in_subwoofer 	= block_in[speakers.mapping.subwoofer];
		in_rear_left 	= block_in[speakers.mapping.rear_left];
		in_rear_right 	= block_in[speakers.mapping.rear_right];
	}else{
		in_front_center = block_in[0];
		in_subwoofer	= block_in[0];
		in_rear_left    = block_in[0];
		in_rear_right	= block_in[1];
	}

	if(speakers.count >= 8)
	{
		os_side_left    = &block_out[speakers.mapping.side_left];
		os_side_right   = &block_out[speakers.mapping.side_right];
	}else{
		os_side_left    = &s_dummy;
		os_side_right   = &s_dummy;
	}

	if(in_channels >= 8)
	{
		in_side_left  = block_in[6];
		in_side_right = block_in[7];
	}else{
		in_side_left  = block_in[0];
		in_side_right = block_in[1];
	}

	if(in_channels == 2) /* stereo to multichannel */
	{
		*os_front_left   = speakers.volumes.front_left   * in_front_left;
		*os_front_right	 = speakers.volumes.front_right	 * in_front_right;
		*os_front_center = speakers.volumes.front_center * (in_front_left + in_front_right) / 2.0;
		*os_subwoofer	 = speakers.volumes.subwoofer	 * (in_front_left + in_front_right) / 2.0;
		*os_rear_left	 = speakers.volumes.rear_left	 * delay(in_front_left,  channel_rear_left);
		*os_rear_right	 = speakers.volumes.rear_right	 * delay(in_front_right, channel_rear_right);
		*os_side_left	 = speakers.volumes.side_left	 * in_front_left;
		*os_side_right	 = speakers.volumes.side_right	 * in_front_right;
	}
	else if(in_channels == 5) /* 5.1 to multichannel */
	{
		*os_front_left   = speakers.volumes.front_left   * in_front_left;
		*os_front_right	 = speakers.volumes.front_right	 * in_front_right;
		*os_front_center = speakers.volumes.front_center * in_front_center;
		*os_subwoofer	 = speakers.volumes.subwoofer	 * in_subwoofer;
		*os_rear_left	 = speakers.volumes.rear_left	 * in_rear_left;
		*os_rear_right	 = speakers.volumes.rear_right	 * in_rear_right;
		*os_side_left	 = speakers.volumes.side_left	 * in_front_left;
		*os_side_right	 = speakers.volumes.side_right	 * in_front_right;
	}
	else if(in_channels == 8) /* 7.1? just copy! */
	{
		*os_front_left   = speakers.volumes.front_left   * in_front_left;
		*os_front_right	 = speakers.volumes.front_right	 * in_front_right;
		*os_front_center = speakers.volumes.front_center * in_front_center;
		*os_subwoofer	 = speakers.volumes.subwoofer	 * in_subwoofer;
		*os_rear_left	 = speakers.volumes.rear_left	 * in_rear_left;
		*os_rear_right	 = speakers.volumes.rear_right	 * in_rear_right;
		*os_side_left	 = speakers.volumes.side_left	 * in_side_left;
		*os_side_right	 = speakers.volumes.side_right	 * in_side_right;
	}

	return;
}

/* locals -------------------------------------------------------------------*/

static fennec_sample  delay(fennec_sample ins, int channel)
{	
	int            *i;
	int             delay_ms;
	fennec_sample  *buffer, r;

	switch(channel)
	{
	case channel_rear_left:
		i = &speakers.delay.point_rear_left;
		delay_ms = speakers.delay.rear_delay_ms;
		buffer = speakers.delay.side_delay_left;
		break;

	case channel_rear_right:
		i = &speakers.delay.point_rear_right;
		delay_ms = speakers.delay.rear_delay_ms;
		buffer = speakers.delay.side_delay_right;
		break;

	default:
		return ins;
	}

	r         = buffer[(*i)];
	buffer[(*i)] = ins;
	
	if(++(*i) >= (speakers.sampling_rate / 1000) * delay_ms)(*i) = 0;
	return r;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

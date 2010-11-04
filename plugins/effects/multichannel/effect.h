/**----------------------------------------------------------------------------

 Fennec DSP Plug-in 1.0 (Extra Bass).
 Copyright (C) 2007 Chase <c-h@users.sf.net>

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

-------------------------------------------------------------------------------

----------------------------------------------------------------------------**/

#include "../../../include/fennec.h"
#include "../../../include/system.h"
#include "../../../include/ids.h"
#include <math.h>

#define effect_set_mode_low  0
#define effect_set_mode_high 1

#define channel_front_left    0
#define channel_front_right   1
#define channel_front_center  2
#define channel_subwoofer     3
#define channel_rear_left     4
#define channel_rear_right    5
#define channel_side_left     6
#define channel_side_right    7

struct speaker_details
{
	struct
	{
		double  front_left;
		double  front_right;
		double  front_center;
		double  subwoofer;
		double  rear_left;
		double  rear_right;
		double  side_left;
		double  side_right;
	}volumes;

	struct
	{
		int     front_left;
		int     front_right;
		int     front_center;
		int     subwoofer;
		int     rear_left;
		int     rear_right;
		int     side_left;
		int     side_right;
	}mapping;

	struct
	{
		int     do_lowpass;
		int     do_lowshelf;
		int     lowpass_cutoff;
		int     lowshelf_freq;
		int     lowshelf_boost;
	}subwoofer;

	struct
	{
		int				front_delay_ms; /* center would be the source */
		int				rear_delay_ms;
		int				side_delay_ms;
		fennec_sample   front_delay_left[10000];
		fennec_sample   front_delay_right[10000];
		fennec_sample   rear_delay_left[10000];
		fennec_sample   rear_delay_right[10000];
		fennec_sample   side_delay_left[10000];
		fennec_sample   side_delay_right[10000];
		int				point_front_left;
		int				point_front_right;
		int				point_rear_left;
		int				point_rear_right;
		int				point_side_left;
		int				point_side_right;


	}delay;

	struct
	{
		double  rear_presence;
		double  side_presence;
	}presence;

	int count;
	int sampling_rate;
};


void            effect_initialize(void);
void            effect_set(unsigned long frequency, unsigned long channels, int mode);
int callc       messageproc(int id, int a, void* b, double d);
fennec_sample   effect_process(fennec_sample v, unsigned long frequency, unsigned long channels);


/* speaker effects */

int  sp_init(void);
int  sp_uninit(void);
void sp_process(fennec_sample *block_out, fennec_sample  *block_in, int in_channels);

extern struct speaker_details    speakers;

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


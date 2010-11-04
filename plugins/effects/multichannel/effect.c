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

-------------------------------------------------------------------------------

----------------------------------------------------------------------------**/

#include "effect.h"




/* types --------------------------------------------------------------------*/

struct channel_data
{
	int i;
};




/* data ---------------------------------------------------------------------*/

struct channel_data  chd[max_channels];




/* code ---------------------------------------------------------------------*/


/*
 * initialize: called only once.
 */
void effect_initialize(void)
{

}


/*
 * settings changing callback.
 */
void effect_set(unsigned long frequency, unsigned long channels, int mode)
{
	unsigned int i;

}


/*
 * sample processing.
 */
fennec_sample effect_process(fennec_sample v, unsigned long frequency, unsigned long channel)
{

	return v;
}

/*
 * callback function for message handling.
 */
int callc messageproc(int id, int a, void* b, double d)
{
	switch(id)
	{
	case plugin_message_getchannels:
		return 6;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0
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

#include "plugin.h"




/* code ---------------------------------------------------------------------*/


int encoder_setdefaults(unsigned long id)
{
	return 1;
}

int encoder_appendextension(unsigned long id, string fpath)
{
	str_cat(pestreams[id].filepath, uni(".extension"));
	return 1;
}

int encoder_deleteextension(unsigned long id, string fpath)
{
	int i = (int)str_len(fpath);

	if(i - 3 <= 0)return 0;
	i -= 3;

	fpath[i] = 0;
	return 1;
}

int encoder_initialize(unsigned long id, int fcreatemode)
{
	pestreams[id].firstwrite = 1;

	if(pestreams[id].smode == fennec_encoder_file)
	{
		if(fcreatemode == fennec_encode_openmode_createnew)
		{
			

		}else{

			/* unknown mode! */
			return 0;
		}
	}

	return 1;
}


int encoder_uninitialize(unsigned long id)
{
	if(!pestreams[id].firstwrite)
	{

		pestreams[id].firstwrite = 1;
	}
	return 1;
}

int encoder_set(unsigned long id)
{	

	return 1;
}

int encoder_write(unsigned long id, void* rbuffer, unsigned long bsize)
{	
	if(pestreams[id].firstwrite)
	{

		pestreams[id].firstwrite = 0;
	}

	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


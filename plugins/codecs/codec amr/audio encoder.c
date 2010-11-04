/**----------------------------------------------------------------------------

 Fennec Codec Plug-in 1.0 (Sound Files).
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
#include "amr/interf_enc.h"
#include "amr/sp_enc.h"

#ifndef ETSI
#	ifndef IF2
#		define AMR_MAGIC_NUMBER "#!AMR\n"
#	endif
#endif

int encoder_setdefaults(unsigned long id)
{
	return 1;
}

int encoder_appendextension(unsigned long id, string fpath)
{
	str_cat(pestreams[id].filepath, uni(".amr"));
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
		sys_file_close(pestreams[id].fhandle);
		Encoder_Interface_exit(pestreams[id].enstate);
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
	short         lspeech[160];
	unsigned int  i = 0, j, z, byte_counter, k;

	if(pestreams[id].firstwrite)
	{
		if(pestreams[id].firstwrite == -1) return 0;

		if(pestreams[id].cfrequency != 8000 || pestreams[id].cchannels != 1)
		{
			MessageBox(0, uni("Sorry, input file needs to be in following format:\n8000Hz, Mono.\n\nPlease use \"Sound File Encoder\" to convert between formats (example: convert your file(s) into Wave format (with above settings), then convert those (.wav) file(s) into AMR)."), uni("AMR Conversion"), MB_ICONEXCLAMATION);
			pestreams[id].firstwrite = -1;
			return 0;
		}

		pestreams[id].fhandle = sys_file_createforcestream(pestreams[id].filepath, v_sys_file_forwrite);
		
		if(pestreams[id].fhandle == v_error_sys_file_create)return 0;


		memset(pestreams[id].serial_data, 0, sizeof(pestreams[id].serial_data));

		pestreams[id].req_mode = MR122;
		pestreams[id].dtx      = 0;
		pestreams[id].c        = 0;

		pestreams[id].enstate = Encoder_Interface_init(pestreams[id].dtx);

		sys_file_write(pestreams[id].fhandle, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER));
		
		pestreams[id].firstwrite = 0;
	}

	j = pestreams[id].c;

	for(;;)
	{
		z = min(bsize - i, sizeof(pestreams[id].speech));
		memcpy(((char*)pestreams[id].speech) + j, ((char*)rbuffer) + i, z - j);

		pestreams[id].c = z - j;

		if(pestreams[id].c == sizeof(pestreams[id].speech))
		{
			for(k=0; k<160; k++)
				lspeech[k] = (short)(pestreams[id].speech[k] * 32767.0);

			byte_counter = Encoder_Interface_Encode(pestreams[id].enstate, pestreams[id].req_mode, lspeech, pestreams[id].serial_data, 0);

			sys_file_write(pestreams[id].fhandle, pestreams[id].serial_data, byte_counter);
		}

		i += z - j;
		j = 0;

		if(i >= bsize)break;
	}
	

	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


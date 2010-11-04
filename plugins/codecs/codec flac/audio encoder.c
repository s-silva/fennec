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
	str_cat(pestreams[id].filepath, uni(".flac"));
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
		FLAC__file_encoder_finish(pestreams[id].flacenc);
		FLAC__file_encoder_delete(pestreams[id].flacenc);

		if(pestreams[id].sbuffer)
			sys_mem_free(pestreams[id].sbuffer);

		pestreams[id].sbuffer = 0;

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
	unsigned long   rsize, i, c;
	fennec_sample  *fbuffer;

	if(pestreams[id].firstwrite)
	{
		char  fpath[v_sys_maxpath];
		BOOL  useddef = 1;

		WideCharToMultiByte(CP_ACP, 0, pestreams[id].filepath, -1, fpath, sizeof(fpath), "?", &useddef);


		pestreams[id].flacenc = FLAC__file_encoder_new();
		if(!pestreams[id].flacenc) return 0;

		if(!FLAC__file_encoder_set_sample_rate(pestreams[id].flacenc, pestreams[id].cfrequency))
			return 0;

		if(!FLAC__file_encoder_set_channels(pestreams[id].flacenc, pestreams[id].cchannels))
			return 0;
		
		
 		if(!FLAC__file_encoder_set_filename(pestreams[id].flacenc, fpath))
			return 0;
		
		pestreams[id].flacencstate = FLAC__file_encoder_init(pestreams[id].flacenc);

		if(pestreams[id].flacencstate != FLAC__FILE_ENCODER_OK) return 0;

		pestreams[id].bufsize = 0;
		pestreams[id].sbuffer = 0;

		pestreams[id].firstwrite = 0;
	}

	rsize = (bsize * sizeof(FLAC__int32)) / sizeof(fennec_sample);
	c     = bsize / sizeof(fennec_sample);

	if(pestreams[id].bufsize < rsize)
	{
		pestreams[id].bufsize = rsize;

		if(!pestreams[id].sbuffer)
			pestreams[id].sbuffer = sys_mem_alloc(rsize);
		else
			pestreams[id].sbuffer = sys_mem_realloc(pestreams[id].sbuffer, rsize);
	}

	fbuffer = (fennec_sample*) rbuffer;

	for(i=0; i<c; i++)
		pestreams[id].sbuffer[i] = (FLAC__int32)(fbuffer[i] * 32767.0);

	FLAC__file_encoder_process_interleaved(pestreams[id].flacenc, pestreams[id].sbuffer, c / pestreams[id].cchannels);

	


	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


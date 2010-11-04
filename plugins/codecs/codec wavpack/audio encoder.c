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
	str_cat(pestreams[id].filepath, uni(".wv"));
	return 1;
}

int encoder_deleteextension(unsigned long id, string fpath)
{
	int i = (int)str_len(fpath);

	for(;i>0; i--)
	{
		if(fpath[i] == uni('.'))
		{
			fpath[i] = 0;
			break;
		}
	}

	return 1;
}

int encoder_initialize(unsigned long id, int fcreatemode)
{
	pestreams[id].firstwrite = 1;

	if(pestreams[id].smode == fennec_encoder_file)
	{
		if(fcreatemode == fennec_encode_openmode_createnew)
		{
			pestreams[id].fhandle = sys_file_createforcestream(pestreams[id].filepath, v_sys_file_forwrite);
			if(pestreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seteof(pestreams[id].fhandle);

		}else if(fcreatemode == fennec_encode_openmode_create){

			pestreams[id].fhandle = sys_file_createforcestream(pestreams[id].filepath, v_sys_file_forwrite);
			if(pestreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seteof(pestreams[id].fhandle);

		}else if(fcreatemode == fennec_encode_openmode_append){

			pestreams[id].fhandle = sys_file_createforcestream(pestreams[id].filepath, v_sys_file_forwrite);
			if(pestreams[id].fhandle == v_error_sys_file_create)return 0;
			sys_file_seek(pestreams[id].fhandle, sys_file_getsize(pestreams[id].fhandle));

		}else{

			/* unknown mode! */
			return 0;
		}
	}

	return 1;
}


int encoder_uninitialize(unsigned long id)
{
	char *block_buff;

	if(!pestreams[id].firstwrite)
	{
		if(pestreams[id].wpc)
		{
			WavpackFlushSamples(pestreams[id].wpc);


			block_buff = sys_mem_alloc(pestreams[id].block_size);
			
			sys_file_seek(pestreams[id].fhandle, 0);
			sys_file_read(pestreams[id].fhandle, block_buff, pestreams[id].block_size);
			WavpackUpdateNumSamples(pestreams[id].wpc, block_buff);

			WavpackCloseFile(pestreams[id].wpc);
		}


		sys_file_close(pestreams[id].fhandle);
		pestreams[id].firstwrite = 1;
	}
	return 1;
}

int encoder_set(unsigned long id)
{	

	return 1;
}


static int write_block (void *iid, void *data, int32_t bcount)
{
	int id = (int)iid;

	if(id != -1)
	{
		sys_file_write(pestreams[id].fhandle, data, bcount);

		if(!pestreams[id].block_size)
			pestreams[id].block_size = bcount;
	}
	
	return 1;
}



int encoder_write(unsigned long id, void* rbuffer, unsigned long bsize)
{	
	fennec_sample *fbuffer = (fennec_sample*) rbuffer;
	int32_t       *sbuffer;
	unsigned long  scount = bsize / sizeof(fennec_sample), i;

	if(pestreams[id].firstwrite)
	{
		pestreams[id].block_size = 0;

		pestreams[id].swritten = 0;
		pestreams[id].wvid     = (void*) id;
		pestreams[id].wvcid    = (void*) -1;
		pestreams[id].wpc      = WavpackOpenFileOutput(write_block, pestreams[id].wvid, pestreams[id].wvcid);
		
		if(pestreams[id].wpc)
		{
			memset(&pestreams[id].wpconfig, 0, sizeof(WavpackConfig));

			pestreams[id].wpconfig.bytes_per_sample = 2;
			pestreams[id].wpconfig.bits_per_sample  = 16;
			pestreams[id].wpconfig.channel_mask     = (1 << pestreams[id].cchannels) - 1;
			pestreams[id].wpconfig.num_channels     = pestreams[id].cchannels;
			pestreams[id].wpconfig.sample_rate      = pestreams[id].cfrequency;

			pestreams[id].wpconfig.flags = CONFIG_VERY_HIGH_FLAG;

			WavpackSetConfiguration(pestreams[id].wpc, &pestreams[id].wpconfig, -1);

			WavpackPackInit(pestreams[id].wpc);
		}

		pestreams[id].firstwrite = 0;
	}

	if(pestreams[id].wpc)
	{
		sbuffer = sys_mem_alloc(scount * sizeof(int32_t));

		for(i=0; i<scount; i++)
			sbuffer[i] = (int32_t)(fbuffer[i] * 32767.0);
		
		WavpackPackSamples(pestreams[id].wpc, sbuffer, scount / pestreams[id].cchannels);

		pestreams[id].swritten += scount / pestreams[id].cchannels;

		sys_mem_free(sbuffer);
		
		return 1;
	}else{
		return 0;
	}

}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


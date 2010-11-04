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




/* variables ----------------------------------------------------------------*/

struct plugin_stream *pstreams;
unsigned long         pstreams_count = 0;
int                   plugin_busy    = 0;

string info_base = uni("FFMpeg Codec");

extern struct video_dec ffvid_dec;
extern CRITICAL_SECTION cs;

/* functions ----------------------------------------------------------------*/


/*
 * callback function for decoder initialization.
 */
unsigned long callc fennec_initialize_input(struct general_input_data* gin)
{
	if(gin->ptype != fennec_plugintype_audiodecoder)
		return fennec_input_invalidtype;
	
	if(gin->fiversion >= plugin_version)
	{
		gin->initialize    = fennec_plugin_initialize;
		gin->open          = fennec_plugin_open;
		gin->getformat     = fennec_plugin_getformat;
		gin->getinfo       = 0;
		gin->getduration   = fennec_plugin_getduration;
		gin->getposition   = fennec_plugin_getposition;
		gin->setposition   = fennec_plugin_setposition;
		gin->getstatus     = fennec_plugin_getstatus;
		gin->read          = fennec_plugin_read;
		gin->close         = fennec_plugin_close;
		gin->uninitialize  = fennec_plugin_uninitialize;
		gin->about         = fennec_plugin_about;
		gin->settings      = fennec_plugin_settings;
		gin->tagread       = fennec_plugin_tagread;
		gin->tagwrite      = fennec_plugin_tagwrite;
		gin->score         = fennec_plugin_score;

		gin->get_extension           = fennec_plugin_get_extension;
		gin->get_initialization_info = fennec_plugin_get_initialization_info;

		ffvid_dec.video_decoder_getsize  = video_decoder_getsize;
		ffvid_dec.video_decoder_getframe = video_decoder_getframe;
		ffvid_dec.video_decoder_seek     = video_decoder_seek;
		ffvid_dec.video_decoder_getframe_sync    = video_decoder_getframe_sync;
		ffvid_dec.video_decoder_getsubtitle      = video_decoder_getsubtitle;
		ffvid_dec.video_decoder_trans_info       = video_decoder_trans_info;

		gin->vid_dec = &ffvid_dec;

		if(gin->fiversion == plugin_version)
		{
			return fennec_input_initialized;
		}else{
			gin->pversion = plugin_version;
			return fennec_input_unknownversion;
		}

	}else{

		return fennec_input_invalidversion;
	}
}


/*
 * real initialization.
 */
int callc fennec_plugin_initialize(void)
{
	unsigned long i;

	if(pstreams_count)return 0;

	plugin_busy = 1;

	pstreams_count = plugin_player_base;
	pstreams       = (struct plugin_stream*) sys_mem_alloc(pstreams_count * sizeof(struct plugin_stream));
	
	if(pstreams == v_error_sys_mem_alloc)
	{
		pstreams_count = 0;
		plugin_busy = 0;
		return 0;
	}

	for(i=0; i<pstreams_count; i++) /* free everything */
	{
		pstreams[i].initialized = 0;
		pstreams[i].loaded      = 0;
	}

	plugin_busy = 0;
	return 1;
}


/*
 * load stream/file.
 */
unsigned long callc fennec_plugin_open(unsigned long otype, unsigned long osize, const string odata)
{
	int           found = 0;
	unsigned long i;
	unsigned long last_count;
	unsigned long j;

	while(plugin_busy)
		sys_pass();

	plugin_busy = 1;


	if(otype == fennec_input_open_file || otype == fennec_input_open_internet_stream)
	{
		/* search for an empty player */

		for(i=0; i<pstreams_count; i++)
		{
			if(!pstreams[i].initialized)
			{
				found = 1;
				break;
			}
		}

		if(found)
		{
			/* there's an empty player, id - 'i' */

			pstreams[i].initialized = 1;
			j = i;

		}else{
			/* should reallocate some memory */

			last_count = pstreams_count;

			if(pstreams_count >= plugin_player_max)
			{
				/* we've reached to the maximum, sorry can't load more */
				plugin_busy = 0;
				return 0;
			}

			pstreams_count += plugin_player_add;
			if(pstreams_count > plugin_player_max)pstreams_count = plugin_player_max;

			pstreams = (struct plugin_stream*) sys_mem_realloc(pstreams, pstreams_count * sizeof(struct plugin_stream));
			
			for(i=last_count; i<pstreams_count; i++)
			{
				pstreams[i].initialized = 0;
				pstreams[i].loaded      = 0;
			}

			j = last_count + 1;
			pstreams[j].initialized = 1;
		}
		
		decoder_load(j, odata);

		plugin_busy = 0;
		return j;
	}

	plugin_busy = 0;
	return 0;
}

/*
 * get format information (raw).
 */
int callc fennec_plugin_getformat(unsigned long id, unsigned long* ftype, void* fdata)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)sys_pass();
	plugin_busy = 1;

	switch(*ftype)
	{
	case fennec_input_format_base:
		((struct base_format*)fdata)->bbits      = pstreams[id].bitspersample;
		((struct base_format*)fdata)->bfrequency = pstreams[id].frequency;
		((struct base_format*)fdata)->bchannels  = pstreams[id].channels;
		break;

	default:
		plugin_busy = 0;
		return 0;
	}

	plugin_busy = 0;
	return 1;
}


/*
 * get current position.
 */
int callc fennec_plugin_getposition(unsigned long id, unsigned long* ptype, void* pos)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	switch(*ptype)
	{
	case fennec_input_position_32ms:

		*((unsigned long*)pos) = 0;
		break;

	default:
		return 0;

	}

	return 1;
}


/*
 * set playing position (some media doesn't support this, they must
 * return zero.
 */
int callc fennec_plugin_setposition(unsigned long id, unsigned long ptype, void* pos)
{
	int rv = 1;
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)
		sys_pass();

	plugin_busy = 1;

	switch(ptype)
	{
	case fennec_input_position_32ms:
		{
			double d = ((double)*((unsigned long*)pos)) / ((double)(pstreams[id].duration * 1000));
			decoder_seek(id, &d);
		}
		break;

	case fennec_input_position_fraction:
		rv = decoder_seek(id, (double*)pos);
		break;

	default:
		plugin_busy = 0;
		return 0;

	}

	plugin_busy = 0;
	return rv;
}


/*
 * get duration.
 */
int callc fennec_plugin_getduration(unsigned long id, unsigned long* ptype, void* pos)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	switch(*ptype)
	{
	case fennec_input_position_32ms:

		*((unsigned long*)pos) = pstreams[id].duration;
		break;

	default:
		return 0;

	}

	return 1;
}


/*
 * get plugin status.
 */
int callc fennec_plugin_getstatus(unsigned long id, unsigned long* stype, void* stt)
{
	if(*stype != fennec_input_status_base)return 0;

	((struct base_status*)stt)->bstate = fennec_input_status_sleeping;
	return 1;
}


/*
 * read (decode) next raw block. (use 'setposition' to seek between blocks.
 */
int callc fennec_plugin_read(unsigned long id, unsigned long dsize, unsigned long* dread, void* rdata)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)sys_pass();
	plugin_busy = 1;

	*dread = audio_decoder_read(id, rdata, dsize);

	plugin_busy = 0;
		
	if(!(*dread))
		return fennec_input_nodata;
	else
		return 1;
}


/*
 * show 'about plugin' dialog etc.
 */
int callc fennec_plugin_about(void* pdata)
{

#	if defined(system_microsoft_windows)

	MessageBox((HWND)pdata, uni("FFMPEG Codec plugin for fennec."), uni("About FFMPEG Codec Plugin"), MB_ICONINFORMATION);

#	endif

	return 1;
}


/*
 * show 'settings' dialog etc.
 */
int callc fennec_plugin_settings(void* pdata)
{
#	if defined(system_microsoft_windows)

	MessageBox((HWND)pdata, uni("There's nothing to configure."), uni("FFMPEG Codec Plugin Settings"), MB_ICONINFORMATION);

#	endif

	return 1;
}


/*
 * get information about plugin.
 */
int callc fennec_plugin_get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize)
{
	if(inforreq == fennec_information_basic)
	{
		str_cpy(outinfo, info_base);
		*outsize = (unsigned long)str_size(info_base);
		return 0;
	}
	return 0;
}

int callc fennec_plugin_get_extension_in(int id, string out_ext, string out_desc, int* newdsc)
{
	int                    i;
	static int             lastp = 0;
	static AVInputFormat  *ofmt = 0;
	static char            oneext[128];

	if(id == 0)
	{
		*newdsc = 1;
		ofmt = 0;
	}
	
point_readd:

	if(!lastp)
		ofmt = av_iformat_next(ofmt);
	
	if(!ofmt)return 0;

	if(!ofmt->extensions)goto point_readd;
	if(!ofmt->extensions[0])goto point_readd;

	for(i=lastp; ;i++)
	{
		if(ofmt->extensions[i] == ',' || ofmt->extensions[i] == 0)
		{
			memset(oneext, 0, sizeof(oneext));
			strncpy(oneext, ofmt->extensions + lastp, i - lastp);
			lastp = i + 1;
			if(!ofmt->extensions[i])lastp = 0;
			break;
		}
	}

	MultiByteToWideChar(CP_ACP, 0, oneext, -1, out_ext, 512);
	MultiByteToWideChar(CP_ACP, 0, ofmt->long_name, -1, out_desc, 512);
	return 0;
}

int callc fennec_plugin_get_extension_out(int id, string out_ext, string out_desc, int* newdsc)
{
	int                    i;
	static int             lastp = 0;
	static AVOutputFormat *ofmt = 0;
	static char            oneext[128];

	if(id == 0)
	{
		*newdsc = 1;
		ofmt = 0;
	}
	
point_readd:

	if(!lastp)
		ofmt = av_oformat_next(ofmt);
	
	if(!ofmt)return 0;

	if(!ofmt->extensions)goto point_readd;
	if(!ofmt->extensions[0])goto point_readd;

	for(i=lastp; ;i++)
	{
		if(ofmt->extensions[i] == ',' || ofmt->extensions[i] == 0)
		{
			memset(oneext, 0, sizeof(oneext));
			strncpy(oneext, ofmt->extensions + lastp, i - lastp);
			lastp = i + 1;
			if(!ofmt->extensions[i])lastp = 0;
			break;
		}
	}

	MultiByteToWideChar(CP_ACP, 0, oneext, -1, out_ext, 512);
	MultiByteToWideChar(CP_ACP, 0, ofmt->long_name, -1, out_desc, 512);
	return 1;
}

/*
 * extension list callback; (return 0 - end).
 */
int callc fennec_plugin_get_extensionx(int id, string out_ext, string out_desc, int* newdsc)
{
	static int infinished = 0, jsid = 0;
	int        r;

	if(id ==0)
	{
		av_register_all();
		infinished = 0;
		jsid = 0;
	}

	if(!infinished)
	{
		r = fennec_plugin_get_extension_out(id, out_ext, out_desc, newdsc);
		if(!r)
		{
			jsid = id;
			infinished = 1;
		}
	}else{
		r = fennec_plugin_get_extension_in(id - jsid - 1, out_ext, out_desc, newdsc);
		if(!r) return 0;
	}

	return 1;
}

/*
 * extension list callback; (return 0 - end).
 */
int callc fennec_plugin_get_extension(int id, string out_ext, string out_desc, int* newdsc)
{
	if(id == 0)*newdsc = 1;

	switch(id)
	{
	case 0:
		str_cpy(out_ext,  uni("mpg"));
		str_cpy(out_desc, uni("MPEG Video"));
		break;

	case 1:
		str_cpy(out_ext,  uni("mpeg"));
		str_cpy(out_desc, uni("MPEG Video"));
		break;

	case 2:
		str_cpy(out_ext,  uni("avi"));
		str_cpy(out_desc, uni("Audio Video Interleave"));
		break;

	case 3:
		str_cpy(out_ext,  uni("vob"));
		str_cpy(out_desc, uni("MPEG2 PS Format"));
		break;

	case 4:
		str_cpy(out_ext,  uni("flv"));
		str_cpy(out_desc, uni("Flash Video"));
		break;

	case 5:
		str_cpy(out_ext,  uni("3gp"));
		str_cpy(out_desc, uni("3gp Video"));
		break;


	case 6:
		str_cpy(out_ext,  uni("mov"));
		str_cpy(out_desc, uni("QuickTime Video"));
		break;

	case 7:
		str_cpy(out_ext,  uni("qt"));
		str_cpy(out_desc, uni("QuickTime Video"));
		break;

	case 8:
		str_cpy(out_ext,  uni("asf"));
		str_cpy(out_desc, uni("Windows Media Video"));
		break;

	case 9:
		str_cpy(out_ext,  uni("wmv"));
		str_cpy(out_desc, uni("Windows Media Video"));
		break;

	case 10:
		str_cpy(out_ext,  uni("divx"));
		str_cpy(out_desc, uni("DivX Video"));
		break;

	
	case 11:
		str_cpy(out_ext,  uni("mkv"));
		str_cpy(out_desc, uni("Matroska Multimedia File"));
		break;

	
	case 12:
		str_cpy(out_ext,  uni("mp4"));
		str_cpy(out_desc, uni("MPEG-4 Format"));
		break;

	
	case 13:
		str_cpy(out_ext,  uni("ogm"));
		str_cpy(out_desc, uni("Ogg Media Format"));
		break;

	case 14:
		str_cpy(out_ext,  uni("rm"));
		str_cpy(out_desc, uni("Real Media Format"));
		break;

	case 15:
		str_cpy(out_ext,  uni("rmvb"));
		str_cpy(out_desc, uni("Real Media Format"));
		break;

	default:
		return 0;
	}
	return -1; /* check out plug-in, if everything fails */
}


/*
 * score stream/file.
 * see 'score_min', 'score_max'...
 */
int callc fennec_plugin_score(const string fname, int options)
{
	return decoder_score(fname, options);
}


/*
 * read tags.
 */
int callc fennec_plugin_tagread(const string fname, struct fennec_audiotag* rtag)
{
	return decoder_tagread(fname, rtag);
}


/*
 * write tags.
 */
int callc fennec_plugin_tagwrite(const string fname, struct fennec_audiotag* wtag)
{
	return decoder_tagwrite(fname, wtag);
}


/*
 * unload stream.
 */
int callc fennec_plugin_close(unsigned long id)
{
	if(id >= pstreams_count)return 0;
	if(!pstreams[id].initialized)return 0;

	while(plugin_busy)sys_pass();
	plugin_busy = 1;

	decoder_close(id);

	pstreams[id].initialized = 0;
	plugin_busy = 0;
	return 1;
}


/*
 * uninitialize plugin and unload all streams.
 */
int callc fennec_plugin_uninitialize(void)
{
	unsigned long i;

	if(!pstreams_count)return 0;

	for(i=0; i<pstreams_count; i++)
	{
		if(pstreams[i].initialized)
		{
			fennec_plugin_close(i);
		}
	}

	pstreams_count = 0;
	sys_mem_free(pstreams);

	return 1;
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

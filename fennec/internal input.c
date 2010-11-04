/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.2 (added scoring)
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

#include "fennec main.h"
#include "fennec audio.h"
#include "plugins.h"




/* structures ---------------------------------------------------------------*/

typedef struct _input_plugin
{
	unsigned short            extensions_count;         /* extensions count in 'extensions' array */
	letter                    extensions[200][16];      /* extensions, null terminated data without leading period */
	letter                    typedescription[200][128]; /* file type description for extensions */
	letter                    pluginname[128];          /* plug-in name */
	letter                    fname[1024];              /* plug-in file name */
	struct general_input_data inputdata;                /* functions and details */
	t_sys_library_handle      lhandle;                  /* handle to plug-in (if loaded) */
	unsigned char             inuse;                    /* in use, don't try to uninitialize */
	int                       pushes;                   /* dependencies */
	int                       plugin_type;              /* 0 - default, 1 - check-out */
}input_plugin;




/* declarations -------------------------------------------------------------*/

int local_cache_refresh(const string spath);




/* defines ------------------------------------------------------------------*/

#define input_plugins_version     1
#define input_invalid_plugin      -1

#define plugin_push(p) (input_plugins[(p)].pushes++)
#define plugin_pop(p)  (input_plugins[(p)].pushes = (input_plugins[(p)].pushes > 0 ? input_plugins[(p)].pushes - 1 : input_plugins[(p)].pushes))




/* data ---------------------------------------------------------------------*/

input_plugin  *input_plugins;
int            input_initialized     = 0;
unsigned long  input_lastused_plugin = (unsigned long)-1;
unsigned long  input_extensionscount = 0;


/* code ---------------------------------------------------------------------*/


/*
 * initialize 'internal input' (not plug-ins).
 */
int internal_input_initialize(void)
{
	unsigned long          i;
	unsigned long          iplg_size = 0;
	struct internal_input_plugin  iplg;

	/* scan for plug-ins if they've not been cached */

	if(!settings.plugins.input_plugins_count)local_cache_refresh(settings.plugins.input_path);

	/* allocate memory for 'inputpluginscount' */
	
	input_plugins = (input_plugin*) sys_mem_alloc(settings.plugins.input_plugins_count * sizeof(input_plugin)); 

	input_extensionscount = 0;

	report("%u input plugins found", rt_info, settings.plugins.input_plugins_count);

	/* initialization from plug-ins cache (in settings file) */

	for(i=0; i<settings.plugins.input_plugins_count; i++)
	{
		memset(&iplg, 0, sizeof(iplg));
		
		input_plugins[i].extensions_count = 0;
		input_plugins[i].lhandle = 0;
		input_plugins[i].pushes = 0;

		/* load cached plugin information */

		settings_data_get(setting_id_input_plugin, i, &iplg, &iplg_size);

		if(iplg_size != sizeof(iplg))goto ignore_plugin;
		if(!iplg.fname)goto ignore_plugin;

		/* same type of data, simply copy'em out */

		input_plugins[i].extensions_count = iplg.extensions_count;
		input_plugins[i].plugin_type      = iplg.plugin_type;
		
		memcpy(input_plugins[i].extensions,		  iplg.extensions,       sizeof(iplg.extensions));
		memcpy(input_plugins[i].typedescription,  iplg.typedescription,  sizeof(iplg.typedescription));
		memcpy(input_plugins[i].pluginname,       iplg.pluginname,       sizeof(iplg.pluginname));
		memcpy(input_plugins[i].fname,	          iplg.fname,            sizeof(iplg.fname));
		
		input_extensionscount += iplg.extensions_count;

		reportx("input plugin caching (not loading): %s", rt_stepping, input_plugins[i].fname);

ignore_plugin:;
	}

	input_initialized = 1;
	return 1;
}


/*
 * uninitialize 'internal input' (and plug-ins).
 */
int internal_input_uninitialize(void)
{
	unsigned long i;

	if(!input_initialized)return 0;

	report("uninitializing input system", rt_stepping);

	/* uninitialize each plug-in 'uninitialization' 
	   function will take care of validations. */

	for(i = 0;  i < settings.plugins.input_plugins_count;  i++)
		internal_input_plugin_uninitialize(i, 1 /* force! */);

	sys_mem_free(input_plugins);
	input_lastused_plugin = (unsigned long)-1;
	return 1;
}


/*
 * input selection by analyzing the file.
 * [todo:]
 *
 * this function must be expanded into a file
 * structure analyzation; by letting all the
 * plug-ins to score the file (0 to ~10,000).
 * so we can stop the loop (scanning files) if
 * any plug-in's gonna give us the maximum
 * score (10,000). if not, we'll have to try
 * the highest score holder.
 *
 * i'm trying to introduce  this scoring
 * system just because some formats don't
 * have an exactly recognizable file
 * structure (i.e. mp3), in this system they
 * can always score medium level points.
 * 
 * the only drawback of this system is the
 * need of (un)initializing all the cached
 * plug-ins, if there's no exact match
 * (maximum score).
 *
 * if a plug-in gives the highest score for
 * a file, let it play the file. There's no
 * need to check for others.
 *
 */
int internal_input_selectinput(const string fname)
{
	letter        ext[v_sys_maxpath];
	unsigned long i, j; /* maxscore = 0, cscore, maxscorep; */
	unsigned long foundi = (unsigned long)-1, supcount = 0;
	uint8_t       foundplgs[512];

	memset(foundplgs, 0, sizeof(foundplgs));

	if(!fname)return input_invalid_plugin;

	/* extension search */

	i = (unsigned long) str_len(fname);

	while(fname[i] != uni('.') && i){i--;}
	i++; /* to terminate the leading period */

	if(i)str_cpy(ext, fname + i);

	/* initialize from zero to include internal wave input */
	for(i=0; i<settings.plugins.input_plugins_count; i++)
	{
		for(j=0; j<input_plugins[i].extensions_count; j++)
		{
			if(!str_icmp(ext, input_plugins[i].extensions[j]))
			{
				/* matching extensions */
				foundi = i;
				supcount++;
				foundplgs[i] = 1;
			}
		}
	}

	if(supcount == 1) return foundi;

	if(supcount > 1) /* more than one input plugins */
	{
		unsigned long maxscorep, cscore, maxscore = 0;

		/* plugin by plugin search + scoring [todo] */

		maxscorep = 0;

		for(i=0; i<settings.plugins.input_plugins_count; i++)
		{
			if(!foundplgs[i]) continue;

			internal_input_plugin_initialize(i);

			cscore = min(max(internal_input_plugin_score(i, fname), 0), 10000);
			
			if(cscore == 10000) /* sure to handle */
				return i;

			internal_input_plugin_uninitialize(i, 0);
			
			if(cscore > maxscore)
			{
				maxscore  = cscore;
				maxscorep = i;
			}
		}

		return maxscorep;
	}

	for(i=0; i<settings.plugins.input_plugins_count; i++)
	{
		if(input_plugins[i].plugin_type == 1)
		{
			return i;
		}
	}

	return input_invalid_plugin;
}


/*
 * initialize plug-in,
 * or just push it if already initialized.
 */
int internal_input_plugin_initialize(unsigned long pid)
{
	fennec_decoder_initialize  plg_initialize;
	letter                     fpath[v_sys_maxpath];

	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;

	if(input_plugins[pid].lhandle)
	{
		plugin_push(pid);
		return 1; /* already initialized */
	}

	report("initializing input plugin (id): %u", rt_stepping, pid);

	/* load plug-in */

	input_plugins[pid].lhandle = sys_library_load(input_plugins[pid].fname);

	if(input_plugins[pid].lhandle == v_error_sys_library_load)
	{
		input_plugins[pid].lhandle = 0;

		/* try using the path */

		if(settings.plugins.input_path[0] == '\\' || settings.plugins.input_path[0] == '/')
		{
			str_cpy(fpath, fennec_get_path(0, 0));
			str_cat(fpath, settings.plugins.input_path);
			str_cat(fpath, input_plugins[pid].fname);
		}else{
			memset(fpath, 0, sizeof(fpath));
			str_cpy(fpath, settings.plugins.input_path);
			str_cat(fpath, input_plugins[pid].fname);
		}

		reportx("initializing input plugin (rel->abs path): %s", rt_stepping, fpath);

		input_plugins[pid].lhandle = sys_library_load(fpath);
		if(input_plugins[pid].lhandle == v_error_sys_library_load)
		{
			input_plugins[pid].lhandle = 0;
			return 0;
		}
	}

	/* get address */

	plg_initialize = (fennec_decoder_initialize) sys_library_getaddress(input_plugins[pid].lhandle, "fennec_initialize_input");
	
	if(plg_initialize == v_error_sys_library_getaddress)
	{
		report("input plugin - error on loading : %s", rt_error, input_plugins[pid].pluginname);
		sys_library_free(input_plugins[pid].lhandle);
		input_plugins[pid].lhandle = 0;
		return 0;
	}

	input_plugins[pid].inputdata.ptype     = fennec_plugintype_audiodecoder;
	input_plugins[pid].inputdata.fiversion = input_plugins_version;
	input_plugins[pid].inputdata.language_text = strings_list;
	input_plugins[pid].inputdata.vid_dec   = 0;

	plugin_settings_fillstruct(&input_plugins[pid].inputdata.fsettings);

	/* call initialization function */

	plg_initialize(&input_plugins[pid].inputdata);

	if(!input_plugins[pid].inputdata.initialize)
	{
		report("input plugin - called but not working: %s", rt_error, input_plugins[pid].pluginname);
		sys_library_free(input_plugins[pid].lhandle);
		input_plugins[pid].lhandle = 0;
		return 0;
	}

	sys_pass();

	input_plugins[pid].inputdata.initialize();

	input_plugins[pid].inuse = 1;

	plugin_push(pid);

	report("input plugin - initialized fine", rt_stepping);

	return 0;
}


/*
 * uninitialize plug-in.
 */
int internal_input_plugin_uninitialize(unsigned long pid, int force)
{	
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(input_plugins[pid].pushes > 0)input_plugins[pid].pushes--;

	if((input_plugins[pid].pushes == 0) || force)
	{	
		reportx("input plugin - uninitializing: %s", rt_stepping, input_plugins[pid].pluginname);

		input_plugins[pid].inputdata.uninitialize();
		if(sys_library_free(input_plugins[pid].lhandle) == v_error_sys_library_free)return 0;

		input_plugins[pid].inuse   = 0;
		input_plugins[pid].lhandle = 0;

		input_plugins[pid].pushes = 0;
	}

	return 1;
}


/*
 * set plug-in is in use or not?
 */
int internal_input_plugin_mark_usage(unsigned long pid, int isused)
{
	if(pid == input_invalid_plugin)return 0;
	if(isused == 0)
	{
		input_plugins[pid].inuse = 0;
	}else if(isused == 1){
		input_plugins[pid].inuse = 1;
	}
	return input_plugins[pid].inuse;
}


/*
 * get file scoring.
 */
int internal_input_plugin_score(unsigned long pid, const string sfile)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(input_plugins[pid].inputdata.score)
	{
		return input_plugins[pid].inputdata.score(sfile, 0);
	}
	return 0;
}



/*
 * load file/stream.
 */
int internal_input_plugin_loadfile(unsigned long pid, const string sfile, unsigned long* fid)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(input_plugins[pid].inputdata.open)
	{
		if(input_plugins[pid].inputdata.vid_dec && !(settings.player.video_window_manual_show != video_show_manual))
		{
			skins_function_getdata(set_video_window, (void*)1, 0);
		}

		*fid = input_plugins[pid].inputdata.open(fennec_input_open_file, 0, sfile);
		return (*fid != -1);
	}
	return 0;
}


/*
 * get format, must return zeros for invalid files.
 */
int internal_input_plugin_getformat(unsigned long pid, unsigned long fid, unsigned long* freq, unsigned long* bps, unsigned long* nchannels)
{
	int           ret;
	unsigned long typetmp;
	struct base_format bfmt;

	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;


	if(input_plugins[pid].inputdata.getformat)
	{
		typetmp = fennec_input_format_base;

		ret = input_plugins[pid].inputdata.getformat(fid, &typetmp, &bfmt);

		if(typetmp == fennec_input_format_base && ret)
		{
			if(freq)
			{
				*bps       = fennec_sample_bits; /* bfmt.bbits; */
				*freq      = bfmt.bfrequency;
				*nchannels = bfmt.bchannels;

				if((bfmt.bchannels == 0 || bfmt.bchannels > 16) && (bfmt.bfrequency == (unsigned long)-1))
				{
					int cfreq = 0, cchan = 0, cbps = 0;

					*freq      = bfmt.bfrequency;
					*nchannels = bfmt.bchannels;

					if(basewindows_selectformat(&cfreq, &cchan, &cbps))
					{
						struct base_format nfmt;
						unsigned long tmpnf;

						nfmt.bfrequency = cfreq;
						nfmt.bchannels  = cchan;
						nfmt.bbits      = cbps;

						/* send new format */
						tmpnf = fennec_input_format_base + 1;

						input_plugins[pid].inputdata.getformat(fid, &tmpnf, &nfmt);
					
						*freq      = nfmt.bfrequency;
						*nchannels = nfmt.bchannels;
					}

					
				}else{

					*freq      = bfmt.bfrequency;
					*nchannels = bfmt.bchannels;
				}

			}else{
				*bps       = bfmt.bbits;
			}
			return ret;
		}
	}
	return 0;
}


/*
 * read data.
 */
int internal_input_plugin_readdata(unsigned long pid, unsigned long fid, unsigned long* dsize, void* odata)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(input_plugins[pid].inputdata.read)
	{
		fennec_sample        *samples = (fennec_sample*) odata;
		unsigned long         i = 0, data_size, data_read, ret, typetmp, frames_read;
		struct base_format    tmpformat;


		/* get real format information */

		typetmp = fennec_input_format_base;

		input_plugins[pid].inputdata.getformat(fid, &typetmp, &tmpformat);

		if(typetmp != fennec_input_format_base)return 0;


		/* read samples to floating point buffer and convert'em inside the same buffer */


		if(tmpformat.bbits == fennec_sample_bits)
		{
			ret = input_plugins[pid].inputdata.read(fid, *dsize, dsize, odata);
			
			if(ret == fennec_input_nodata)
				return 0;
			else
				return ret;

		}else{

			data_size = (*dsize * tmpformat.bbits) / fennec_sample_bits;
			data_read = data_size;

			ret = input_plugins[pid].inputdata.read(fid, data_size, &data_read, samples);

			if(ret == fennec_input_nodata)
			{
				memset(odata, 0, data_size);
				return 0;
			}
				

			frames_read = data_read / (tmpformat.bbits / 8);

			if(data_read)
			{
				if(tmpformat.bbits == 16)
				{
					short *sbuffer = (short*) samples;

					if(data_read != data_size)
						memset(odata, 0, data_size);

					for(i=0; i<frames_read; i++)
					{
						samples[frames_read - i - 1] = ((fennec_sample)sbuffer[frames_read - i - 1]) / 32768.0;
					}

				}else if(tmpformat.bbits == 32 /* float */){

					float          *sbuffer = (float*) samples;
					fennec_sample   x;

					if(data_read != data_size)
						memset(odata, 0, data_size);

					for(i=0; i<frames_read; i++)
					{
						x = (fennec_sample) sbuffer[frames_read - i - 1];
						if     (x >  1) x =  1;
						else if(x < -1) x = -1;
						samples[frames_read - i - 1] = x;
					}
				}
			}

			return frames_read * fennec_sample_size;
		}

		/* </conversion> */
	}
	return 0;
}


/*
 * get duration in milliseconds.
 */
unsigned long internal_input_plugin_getduration_ms(unsigned long pid, unsigned long fid)
{
	unsigned long pos, ret = fennec_input_position_32ms;
		
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	input_plugins[pid].inputdata.getduration(fid, &ret, &pos);
	
	if(ret == fennec_input_position_32ms)
	{
		return pos;
	}else if(ret == fennec_input_position_seconds){
		return pos * 1000;
	}else{
		return 0;
	}
}


/*
 * set position fraction.
 */
int internal_input_plugin_setposition(unsigned long pid, unsigned long fid, double spos)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(spos < 0.0f || spos > 1.0f)return 0;

	if(input_plugins[pid].inputdata.setposition)
	{
		return input_plugins[pid].inputdata.setposition(fid, fennec_input_position_fraction, &spos);
	}
	return 1;
}

int internal_input_plugin_setposition_ex(unsigned long pid, unsigned long fid, double *spos)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(*spos < 0.0f || *spos > 1.0f)return 0;

	if(input_plugins[pid].inputdata.setposition)
	{
		return input_plugins[pid].inputdata.setposition(fid, fennec_input_position_fraction, spos);
	}
	return 1;
}


/*
 * read tags and return plug-in id.
 */
unsigned long internal_input_tagread(const string fname, struct fennec_audiotag* rtag)
{
	unsigned long pid;

	if(rtag)
	{
		rtag->tag_title.tsize           = 0;
		rtag->tag_album.tsize           = 0;
		rtag->tag_artist.tsize          = 0;
		rtag->tag_origartist.tsize      = 0;
		rtag->tag_composer.tsize        = 0;
		rtag->tag_lyricist.tsize        = 0;
		rtag->tag_band.tsize            = 0;
		rtag->tag_copyright.tsize       = 0;
		rtag->tag_publish.tsize         = 0;
		rtag->tag_encodedby.tsize       = 0;
		rtag->tag_genre.tsize           = 0;
		rtag->tag_year.tsize            = 0;
		rtag->tag_url.tsize             = 0;
		rtag->tag_offiartisturl.tsize   = 0;
		rtag->tag_filepath.tsize        = 0;
		rtag->tag_filename.tsize        = 0;
		rtag->tag_comments.tsize        = 0;
		rtag->tag_lyric.tsize           = 0;
		rtag->tag_bpm.tsize             = 0;
		rtag->tag_tracknum.tsize        = 0;
		rtag->tag_image.tsize           = 0;
	}

	pid = internal_input_selectinput(fname);

	internal_input_plugin_initialize(pid);

	if(pid == input_invalid_plugin)return (unsigned long)input_invalid_plugin;

	if(input_plugins[pid].inputdata.tagread)
	{
		input_plugins[pid].inputdata.tagread(fname, rtag);
		/* internal_input_plugin_uninitialize(pid, 0); static */
		return pid;
	}
	return (unsigned long)input_invalid_plugin;
}


/*
 * read tags from a known plug-in.
 * use this function to free the memory allocated
 * by plug-ins for the tag. to do this, just pass
 * the address of the tag with file name set to null.
 */
int internal_input_tagread_known(unsigned long pid, const string fname, struct fennec_audiotag* rtag)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	internal_input_plugin_initialize(pid);
	/*if(!fname)plugin_pop(pid);*/

	if(input_plugins[pid].inputdata.tagread)
	{
		int res;
		res = input_plugins[pid].inputdata.tagread(fname, rtag);
		if(!fname)internal_input_plugin_uninitialize(pid, 0);
		return res;
	}
	return 0;
}


/*
 * write (edit) tags if possible.
 */
int internal_input_tagwrite(const string fname, struct fennec_audiotag* rtag)
{
	unsigned long pid;

	pid = internal_input_selectinput(fname);

	internal_input_plugin_initialize(pid);

	if(pid == input_invalid_plugin)return input_invalid_plugin;

	if(input_plugins[pid].inputdata.tagwrite)
	{
		input_plugins[pid].inputdata.tagwrite(fname, rtag);
		internal_input_plugin_uninitialize(pid, 0);
		return pid;
	}
	return input_invalid_plugin;
}


/*
 * show about (pdata = hwnd for windows).
 */
int internal_input_show_about(unsigned long pid, void *pdata)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(input_plugins[pid].inputdata.about)
	{
		input_plugins[pid].inputdata.about(pdata);
		return 1;
	}
	return 0;
}


/*
 * show settings (same as 'show about').
 */
int internal_input_show_settings(unsigned long pid, void *pdata)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	if(input_plugins[pid].inputdata.settings)
	{
		input_plugins[pid].inputdata.settings(pdata);
		return 1;
	}
	return 0;
}

/*
 * unload file/stream.
 */
int internal_input_plugin_getvideo(unsigned long pid, unsigned long fid, struct video_dec **vdec)
{
	*vdec = 0;

	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	*vdec = input_plugins[pid].inputdata.vid_dec;

	if(input_plugins[pid].inputdata.vid_dec)
		return 1;
	
	return 0;
}


/*
 * unload file/stream.
 */
int internal_input_plugin_unloadfile(unsigned long pid, unsigned long fid)
{
	if(pid == input_invalid_plugin)return 0;
	if(pid > settings.plugins.input_plugins_count)return 0;
	if(!input_plugins[pid].lhandle)return 0;

	report("unloading file", rt_stepping);

	if(input_plugins[pid].inputdata.close)
	{
		return input_plugins[pid].inputdata.close(fid);
	}
	return 0;
}


/*
 * get operating system based handle/id of a plug-in.
 */
t_sys_library_handle internal_input_gethandle(const string fname)
{
	unsigned long pid;

	pid = internal_input_selectinput(fname);

	if(pid == input_invalid_plugin)return (t_sys_library_handle)input_invalid_plugin;

	internal_input_plugin_initialize(pid);

	return input_plugins[pid].lhandle;
}


/*
 * get extensions and information.
 * id     - extension id (increasing from zero).
 * ext    - extension.
 * info   - type description (zero length null terminated if same type of the last).
 * return - 1 if available.
 */
int internal_input_getextensionsinfo(unsigned long id, string ext, string info)
{
	unsigned long i, exsum = 0, lexsum = 0;

	if(!input_initialized)return 0;
	if(id >= input_extensionscount)return 0;

	for(i=0; i<settings.plugins.input_plugins_count; i++)
	{
		exsum += input_plugins[i].extensions_count;

		if(exsum > id)
		{
			memcpy(ext,  input_plugins[i].extensions[id - lexsum], sizeof(letter) * 16);
			memcpy(info, input_plugins[i].typedescription[id - lexsum], sizeof(letter) * 128);
			return 1;
		}
		lexsum = exsum;
	}

	return 0;
}


/*
 * eject drive.
 */
int internal_input_drive_eject(char driveid)
{
	unsigned long  pid;
	int            (*drive_eject)(char driveid);

	/* search for a cd audio decoder plugin */

	pid = internal_input_selectinput(uni("X:\\Track1.cda"));

	if(pid == input_invalid_plugin)return 0;

	internal_input_plugin_initialize(pid);

	drive_eject = (int(*)(char))sys_library_getaddress(input_plugins[pid].lhandle, "audiocd_tray_eject");

	if(drive_eject == v_error_sys_library_getaddress)
	{
		internal_input_plugin_uninitialize(pid, 0);
		return 0;
	}

	drive_eject(driveid);
	return 1;
}


/*
 * load drive.
 */
int internal_input_drive_load(char driveid)
{
	unsigned long  pid;
	int            (*drive_load)(char driveid);

	/* search for a cd audio decoder plugin */

	pid = internal_input_selectinput(uni("X:\\Track1.cda"));

	if(pid == input_invalid_plugin)return 0;

	internal_input_plugin_initialize(pid);

	drive_load = (int(*)(char)) sys_library_getaddress(input_plugins[pid].lhandle, "audiocd_tray_load");

	if(drive_load == v_error_sys_library_getaddress)
	{
		internal_input_plugin_uninitialize(pid, 0);
		return 0;
	}

	drive_load(driveid);
	return 1;
}


/*
 * get tracks count.
 */
int internal_input_drive_gettracks(char driveid)
{
	unsigned long  pid;
	int            (*drive_gettracks)(char driveid);

	/* search for a cd audio decoder plugin */

	pid = internal_input_selectinput(uni("X:\\Track1.cda"));

	if(pid == input_invalid_plugin)return 0;

	internal_input_plugin_initialize(pid);

	drive_gettracks = (int(*)(char)) sys_library_getaddress(input_plugins[pid].lhandle, "audiocd_gettrackscount");

	if(drive_gettracks == v_error_sys_library_getaddress)
	{
		internal_input_plugin_uninitialize(pid, 0);
		return 0;
	}
	
	return drive_gettracks(driveid);
}


/*
 * scan and cache plug-ins (to settings file).
 * spath: search path.
 */
int local_cache_refresh(const string rpath)
{
	int                        foundfile;
	t_sys_fs_find_handle       find_handle;
	string                     spath_wild;
	string                     plg_path;
	letter                     plg_name[320];
	struct general_input_data  gid;
	unsigned long              rsize = 0;
	unsigned long              i;
	letter                     plg_ext[32];
	letter                     plg_desc[256];
	letter                     spath[1024];
	int                        isnew, exret = 0;

	letter                     plg_init_info[1024];

	t_sys_library_handle       plg_handle;

	fennec_decoder_initialize  plg_func_initialize;
	
	struct internal_input_plugin  plg_info;


	/* check path */

	if(settings.plugins.input_path[0] == '\\' || settings.plugins.input_path[0] == '/')
	{
		/* relative path */

		str_cpy(spath, fennec_get_path(0, 0));
		str_cat(spath, rpath);
	}else{
		str_cpy(spath, rpath);
	}

	/* it's better to clean up existing caches first */

	settings_data_clean(setting_id_input_plugin);

	/* allocate memory for search path + extension + wildcard (dir\*.lib) */

	spath_wild = (string) sys_mem_alloc(str_size(spath) + str_size(v_sys_lbrary_extension) + 16);

	/* make search path */

	str_cpy(spath_wild, spath);
	str_cat(spath_wild, uni("*"));
	str_cat(spath_wild, v_sys_lbrary_extension);

	/* allocate memory for the file name buffer */

	plg_path = (string)sys_mem_alloc(1024);

	/* start the search process */

	foundfile = sys_fs_find_start(spath_wild, plg_name, 320, &find_handle);

	while(foundfile)
	{
		//memset(plg_path, 0, 1023);
		str_cpy(plg_path, spath);
		str_cat(plg_path, plg_name);

		plg_handle = sys_library_load(plg_path);

		if(plg_handle == v_error_sys_library_load)goto ignore_cache_plugin;

		plg_func_initialize = (fennec_decoder_initialize) sys_library_getaddress(plg_handle, "fennec_initialize_input");

		if(plg_func_initialize == v_error_sys_library_getaddress)
		{
			sys_library_free(plg_handle);
			goto ignore_cache_plugin;
		}

		gid.ptype     = fennec_plugintype_audiodecoder;
		gid.fiversion = input_plugins_version;

		plg_func_initialize(&gid);

		str_cpy(plg_info.fname, plg_name); /* we're searching in the 'input plugins' folder, so full path shouldn't be used */

		/* initialize plugin */
		gid.initialize();

		/* get information */
		plg_init_info[0] = 0;

		gid.get_initialization_info(fennec_information_basic, plg_init_info, &rsize);

		if(rsize)
		{
			str_ncpy(plg_info.pluginname, plg_init_info, sizeof(plg_info.pluginname) / sizeof(plg_info.pluginname[0]));
		}
		/* get extensions */

		i = 0;
		isnew = 0;
		plg_info.extensions_count = 0;
		plg_info.plugin_type      = 0;

		while((exret = gid.get_extension(i, plg_ext, plg_desc, &isnew)) != 0)
		{
			if(exret == -1) /* check out plug-in */
			{
				plg_info.plugin_type = 1;
			}
			if(i >= 200)break; /* we can't handle more than 128 extensions */

			memset(plg_info.typedescription[i], 0, sizeof(plg_info.typedescription[0]));
			str_cpy(plg_info.typedescription[i], plg_desc);

			memset(plg_info.extensions[i], 0, sizeof(plg_info.extensions[i]));
			str_cpy(plg_info.extensions[i], plg_ext);
			i++;
			plg_info.extensions_count++;
		}

		/* add plugin's information into the cache file (settings file) */

		settings_data_add(setting_id_input_plugin, &plg_info, sizeof(plg_info));
		settings.plugins.input_plugins_count++;

		report("refresh cache: input plugin found: %s", rt_stepping, plg_info.fname);

		/* uninitialize plugin */
		
		//gid.uninitialize();
		sys_library_free(plg_handle);

ignore_cache_plugin:
		foundfile = sys_fs_find_next(plg_name, 300, find_handle);
	}

	sys_fs_find_close(find_handle);

	sys_mem_free(spath_wild);
	sys_mem_free(plg_path);
	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

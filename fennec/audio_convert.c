/**----------------------------------------------------------------------------

 Fennec 7.1 Player 1.0
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

#include "fennec_main.h"
#include "fennec_audio.h"




/* structs ------------------------------------------------------------------*/

struct
{
	unsigned long converter_playerhandle;
	unsigned long converter_streamhandle;
	unsigned long encoder_playerhandle;
	unsigned long encoder_streamhandle;
	unsigned long format_freq;
	unsigned long format_bps;
	unsigned long format_chan;
	unsigned int  buffersize;
	char*         buffer;
	int*          cancelop;
	int*          pauseop;
	audioconvert_file_pfunc cfunc;
}joining_data;





/* data ---------------------------------------------------------------------*/

int joining_started = 0;





/* functions ----------------------------------------------------------------*/


/*
 * set volume/gain of a sample buffer.
 * vol - a value between 0.0 and 2.0 (0 to 1 - volume, 1 to 2 - gain).
 */
void convert_setblock_volume(fennec_sample *buf, unsigned int freq, unsigned int chan, unsigned int bps, unsigned int bsize, float vol)
{
	register unsigned int   i;
	register unsigned int   scount = (bsize / (bps / 8));

	for(i=0; i<scount; i++)
	{
		buf[i] *= vol;

		if     (buf[i] >  1.0) buf[i] =  1.0;
		else if(buf[i] < -1.0) buf[i] = -1.0;
	}
}

/*
 * This function simply converts a file ('infile') to another file ('outfile').
 *
 * read the input stream from 'audio' interface and send it to 'encoder-file'
 * interface.
 *
 * eid      - encoder id.
 * bsize    - buffer size (in bytes, will be padded if necessary).
 * cancelop - set to 1 to stop the operation and return 1 if it was sucessful.
 * pauseop  - set to 1 to pause the operation (till you set it back to 0).
 * (uses the same system in 'internal output')
 * trans    - transcoder settings (new!).
 *
 * use a thread to call this function, or just put this into a thread.
 */
int audioconvert_convertfile_ex(const string infile, string outfile, unsigned long eid, unsigned int bsize, int* cancelop, int* pauseop, audioconvert_file_pfunc cfunc, transcoder_settings *trans)
{
	unsigned long converter_playerhandle;
	unsigned long converter_streamhandle;
	unsigned long encoder_playerhandle    = eid;
	unsigned long encoder_streamhandle;
	unsigned long converter_format_freq;         /* frequency */
	unsigned long converter_format_bps;          /* bits per sample, not bits per second */
	unsigned long converter_format_chan;         /* channels */
	unsigned long converter_format_blocksize;    /* (bits per sample / 8) * channels */
	unsigned int  rbuffsize;                     /* real buffer size (after padding) */
	char*         cbuffer;                       /* buffer (current buffer?) */
	unsigned int  rsize;                         /* return size */
	int           readreturn;
	unsigned long ssize = 0;                     /* data size (sum size?) */
	unsigned long csize = 0;                     /* bytes converted (current size) */
	void*         ehandle;                       /* equalizer handle */


	/* what kinda file we got?? */

	converter_playerhandle = audio_input_selectinput(infile);
	if(converter_playerhandle == -1)return -1; /* not supported,  -1? (maybe.. 'invalid player id') :) */
	
	/*
	   we've selected a player (decoder plugin), so initialize it first 
	   remember, we don't need to uninitialize it, cuz the 'internal input'
	   would do it for us :)
	*/

	audio_input_plugin_initialize(converter_playerhandle);

	/* 
	   load the input file,
	   anyhow, fennec ain't just designed to listen something and stop that,
	   it can gather input from more than one streams at once (?), so we gotta use a
	   handle to identify the file <- it can be any value between 0 and infinity.
	   (ok, 0xffffffff... :) ).
	*/

	if(!audio_input_plugin_loadfile(converter_playerhandle, infile, &converter_streamhandle))return -2;

	/* get the ....... format first (we'll need some padding) */

	audio_input_plugin_getformat(converter_playerhandle, converter_streamhandle, &converter_format_freq, &converter_format_bps, &converter_format_chan);

	/* calculate block size for padding */

	converter_format_blocksize = (converter_format_bps / 8) * converter_format_chan;

	/* allocate the buffer */

	if(bsize % converter_format_blocksize) /* add padding if necessary */
		rbuffsize = bsize + (converter_format_blocksize - (bsize % converter_format_blocksize));
	else
		rbuffsize = bsize;

	cbuffer = sys_mem_alloc(rbuffsize);

	if(!cbuffer)
	{
		/* not enough memory!, turn off the computer and add another core
		   (>1MB) and restart the routine. :) 4 nw v unld d fy1 */

		audio_input_plugin_unloadfile(converter_playerhandle, converter_streamhandle);
		return -3;
	}

	/*
	   calc the raw size of the input stream (stream?... return -1 or 0?)
	   we don't need a real 'ssize' value, it's just a progress notification.
	*/

	ssize = audio_input_plugin_getduration_ms(converter_playerhandle, converter_streamhandle) * /* bytes per ms */ ((converter_format_freq / 1000) * (converter_format_bps / 8) * converter_format_chan);

	/* we need an encoder too */

	if(!encoder_initialize())
	{
		sys_mem_free(cbuffer);
		audio_input_plugin_unloadfile(converter_playerhandle, converter_streamhandle);
		return -4;
	}

	encoder_plugin_initialize(encoder_playerhandle);

	/* create a file */

	encoder_streamhandle = encoder_plugin_file_create(encoder_playerhandle, outfile);

	if(converter_streamhandle == -1)
	{
		sys_mem_free(cbuffer);
		audio_input_plugin_unloadfile(converter_playerhandle, converter_streamhandle);
		encoder_uninitialize();
		return -5;
	}

	/* initialize the equalizer, all we need is an array of bands */

	ehandle = equalize_buffer_variable_init(&trans->eq.eq, converter_format_chan, converter_format_freq);

	/* well... hummm..... start conversion */

	for(;;)
	{
		
		/* first of all, we gotta check those pause/cancel flags */

		if(*cancelop)goto point_end;
		if(*pauseop) goto point_continue;

		/* 'rsize' actually != just 'return size' */

		rsize = rbuffsize;

		/*
		   read data, on eof, error; 'read data' returns zero .
		   and we can also determine it using 'rsize'.
		*/

		readreturn = audio_input_plugin_readdata(converter_playerhandle, converter_streamhandle, (unsigned long*)&rsize, cbuffer);

		if(!readreturn)goto point_end;

		/* set volume/gain */

		if(trans->volume.enable_vol)
		{
			if(trans->volume.vol < 0.98f)
			{
				convert_setblock_volume((fennec_sample*) cbuffer, converter_format_freq, converter_format_chan, converter_format_bps, rsize, (float)trans->volume.vol);
			}else if(trans->volume.gain > 0.02f){
				convert_setblock_volume((fennec_sample*)cbuffer, converter_format_freq, converter_format_chan, converter_format_bps, rsize, (float)(trans->volume.gain * 4.0) + 1.0f);
			}
		}

		/* set equalizer */

		if(trans->eq.enable_eq)
		{
			equalize_buffer_variable(cbuffer, 0, converter_format_chan, converter_format_freq, converter_format_bps, rsize, ehandle);
		}

		sys_sleep(2);

		
		/* copy data to the output file */

		encoder_plugin_file_write(encoder_playerhandle, encoder_streamhandle, cbuffer,
			                      converter_format_freq,
								  converter_format_bps,
								  converter_format_chan, rsize);

		/* call set position to dispaly some info */

		csize += rsize;
		if(cfunc && ssize)cfunc((double)csize / (double)ssize);
		sys_pass();
		
		/* now check for eof */

		if(rsize < rbuffsize)goto point_end;

		/* continue if paused */

		goto point_continue_fast;

point_continue:
		sys_sleep(100);

point_continue_fast:;

	}

point_end:

	/* finish! */

	if(cfunc)cfunc(1.0f);

	/* you should clean your ... by yourself :o) */

	sys_mem_free(cbuffer);
	audio_input_plugin_unloadfile(converter_playerhandle, converter_streamhandle);
	encoder_plugin_file_close(encoder_playerhandle, encoder_streamhandle);
	equalize_buffer_variable_uninit(ehandle);

	/* encoder_plugin_uninitialize(encoder_playerhandle);
	   encoder_uninitialize(); */
	return 0;
}


/*
 * Old simple function that had many wrong codings (still used for ripping).
 */
int audioconvert_convertfile(const string infile, string outfile, unsigned long eid, unsigned int bsize, int* cancelop, int* pauseop, audioconvert_file_pfunc cfunc)
{
	transcoder_settings  tsettings;

	memcpy(&tsettings.eq.eq, &settings.conversion.equalizer_bands, sizeof(equalizer_preset));
	tsettings.eq.enable_eq = settings.conversion.use_equalizer;
	tsettings.volume.vol = settings.conversion.volume;
	tsettings.volume.gain = settings.conversion.volume_gain;
	if(tsettings.volume.vol < 0.98 || tsettings.volume.gain > 0.02)
		tsettings.volume.enable_vol = 1;
	else
		tsettings.volume.enable_vol = 0;

	return audioconvert_convertfile_ex(infile, outfile, eid, bsize, cancelop, pauseop, cfunc, &tsettings);
}

/*
 * Audio joining function, same as the routine above but a little bit
 * splited.
 *
 * actually into three parts:
 *     o. start. (start the routine and join one file).
 *     o. push. (add another file, somehow there's no 'pop' function :-) ).
 *     o. end. (finish).
 */
int audiojoining_start(const string infile, string outfile, unsigned long eid, unsigned int bsize, int* cancelop, int* pauseop, double spos, double epos, audioconvert_file_pfunc cfunc)
{
	int           format_blocksize;
	unsigned int  current_size;           /* current file size */
	unsigned int  processed_size = 0;     /* bytes encoded */
	unsigned int  rsize;
	int           readreturn;
	void*         ehandle;                /* equalizer handle */


	if(joining_started)return 0; /* already started */

	/* set flag pointers */

	joining_data.cancelop = cancelop;
	joining_data.pauseop  = pauseop;
	joining_data.cfunc    = cfunc;

	/* input selection */

	joining_data.converter_playerhandle = audio_input_selectinput(infile);

	/* can we read the input file? */
	if(joining_data.converter_playerhandle == (unsigned long)-1)return 0;

	audio_input_plugin_initialize(joining_data.converter_playerhandle);

	/* first, we gotta get some information */

	if(!audio_input_plugin_loadfile(joining_data.converter_playerhandle, infile, &joining_data.converter_streamhandle))return 0;

	/* calculate padding (we need to allocate a buffer to store all samples, not halves) */

	audio_input_plugin_getformat(joining_data.converter_playerhandle, joining_data.converter_streamhandle, &joining_data.format_freq, &joining_data.format_bps, &joining_data.format_chan);

	/* calculate block size for padding */

	format_blocksize = (joining_data.format_bps / 8) * joining_data.format_chan;

	/* allocate the buffer */

	if(bsize % format_blocksize) /* don't need to add padding */
		joining_data.buffersize = bsize;
	else
		joining_data.buffersize = bsize + (format_blocksize - (bsize % format_blocksize));
	
	joining_data.buffer = (char*) sys_mem_alloc(joining_data.buffersize);

	/* allright? */

	if(!joining_data.buffer)
	{
		audio_input_plugin_unloadfile(joining_data.converter_playerhandle, joining_data.converter_streamhandle);
		return 0;
	}

	current_size = audio_input_plugin_getduration_ms(joining_data.converter_playerhandle, joining_data.converter_streamhandle) * /* bytes per ms */ ((joining_data.format_freq / 1000) * (joining_data.format_bps / 8) * joining_data.format_chan);

	audio_input_plugin_setposition(joining_data.converter_playerhandle, joining_data.converter_streamhandle, spos);
	current_size = (unsigned int)((((double)current_size) * epos) - (((double)current_size) * spos));

	/* we need an encoder too */

	if(!encoder_initialize())
	{
		sys_mem_free(joining_data.buffer);
		audio_input_plugin_unloadfile(joining_data.converter_playerhandle, joining_data.converter_streamhandle);
		return 0;
	}

	/* which encoder? */

	joining_data.encoder_playerhandle = eid;

	encoder_plugin_initialize(joining_data.encoder_playerhandle);

	/* create a file */

	joining_data.encoder_streamhandle = encoder_plugin_file_create(joining_data.encoder_playerhandle, outfile);

	if(joining_data.encoder_streamhandle == -1)
	{
		sys_mem_free(joining_data.buffer);
		audio_input_plugin_unloadfile(joining_data.converter_playerhandle, joining_data.converter_streamhandle);
		encoder_uninitialize();
		return 1;
	}

	/* we gotta clear out the equalizer history buffer */

	ehandle = equalize_buffer_variable_init(&settings.conversion.equalizer_bands, joining_data.format_chan, joining_data.format_freq);


	/* start conversion */

	for(;;)
	{
		
		/* first of all, we gotta check those pause/cancel flags */

		if(*cancelop)goto point_end;
		if(*pauseop) goto point_continue;

		/* set buffer size to be read */

		if(((int)current_size) - (int)(processed_size) > (int)joining_data.buffersize)
			rsize = joining_data.buffersize;
		else if(((int)current_size) - (int)(processed_size) > 0)
			rsize = (((int)current_size) - (int)(processed_size));
		else
			rsize = 0;

		/*
		   read data, on eof, error; 'read data' returns zero .
		   and we can also determine it using 'rsize'.
		*/

		readreturn = audio_input_plugin_readdata(joining_data.converter_playerhandle, joining_data.converter_streamhandle, (unsigned long*)&rsize, joining_data.buffer);

		/* end of the file? */
		if(!readreturn)goto point_end;

		/* set block volume, gain */
		if(settings.conversion.volume < 0.98f)
		{
			convert_setblock_volume((fennec_sample*) joining_data.buffer, joining_data.format_freq, joining_data.format_chan, joining_data.format_bps, rsize, (float)settings.conversion.volume);
		}else if(settings.conversion.volume_gain > 0.02f){
			convert_setblock_volume((fennec_sample*) joining_data.buffer, joining_data.format_freq, joining_data.format_chan, joining_data.format_bps, rsize, (float)settings.conversion.volume + 1.0f);
		}

		/* equalize block */

		if(settings.joining.use_equalizer)
		{
			equalize_buffer_variable(joining_data.buffer, 0, joining_data.format_chan, joining_data.format_freq, joining_data.format_bps, rsize, ehandle);
		}

		/* copy data to the output file */

		encoder_plugin_file_write(joining_data.encoder_playerhandle, joining_data.encoder_streamhandle, joining_data.buffer,
			                      joining_data.format_freq,
								  joining_data.format_bps,
								  joining_data.format_chan, rsize);

		/* call set position to dispaly some info */

		processed_size += rsize;
		if(cfunc && current_size)cfunc((double)processed_size / (double)current_size);
		
		/* sleep awhile |-) */

		sys_pass();
		
		/* now check for eof */

		if(rsize < joining_data.buffersize)goto point_end;

		/* continue if paused */

		goto point_continue_fast;

point_continue:
		sys_sleep(100);

point_continue_fast:;

	}

point_end:


	/* finish! */

	if(cfunc)cfunc(1.0f);

	audio_input_plugin_unloadfile(joining_data.converter_playerhandle, joining_data.converter_streamhandle);

	joining_started = 1;
	return 1;
}

int audiojoining_push(int hjoin /* not used */, const string fpath, double spos, double epos)
{
	unsigned int  current_size;
	unsigned int  processed_size = 0;
	void*         ehandle;             /* equalizer handle */

	unsigned int  rsize;
	int           readreturn;
	unsigned long tmp_freq, tmp_chan, tmp_bps;

	if(!joining_started)return 0;

	/* select input */

	joining_data.converter_playerhandle = audio_input_selectinput(fpath);

	/* can we read the file? */

	if(joining_data.converter_playerhandle == -1)return 0;

	/* start reading */

	audio_input_plugin_loadfile(joining_data.converter_playerhandle, fpath, &joining_data.converter_streamhandle);

	audio_input_plugin_getformat(joining_data.converter_playerhandle, joining_data.converter_streamhandle, &tmp_freq, &tmp_bps, &tmp_chan);

	/* check formats */

	if(joining_data.format_bps  != tmp_bps  ||
	   joining_data.format_chan != tmp_chan ||
	   joining_data.format_freq != tmp_freq)
	{
		audio_input_plugin_unloadfile(joining_data.converter_playerhandle, joining_data.converter_streamhandle);
		return 0;
	}

	current_size = audio_input_plugin_getduration_ms(joining_data.converter_playerhandle, joining_data.converter_streamhandle) * /* bytes per ms */ ((joining_data.format_freq / 1000) * (joining_data.format_bps / 8) * joining_data.format_chan);

	/* set epos and spos */

	audio_input_plugin_setposition(joining_data.converter_playerhandle, joining_data.converter_streamhandle, spos);
	current_size = (unsigned int)((((double)current_size) * epos) - (((double)current_size) * spos));

	ehandle = equalize_buffer_variable_init(&settings.conversion.equalizer_bands, joining_data.format_chan, joining_data.format_freq);

	/* start conversion */

	for(;;)
	{
		
		/* first of all, we gotta check those pause/cancel flags */

		if(*joining_data.cancelop)goto point_end;
		if(*joining_data.pauseop) goto point_continue;

		/* set buffer size to be read */

		if(((int)current_size) - (int)(processed_size) > (int)joining_data.buffersize)
			rsize = joining_data.buffersize;
		else if(((int)current_size) - (int)(processed_size) > 0)
			rsize = (((int)current_size) - (int)(processed_size));
		else
			rsize = 0;


		/*
		   read data, on eof, an error; 'read data' returns zero .
		   and we can also determine it using 'rsize'.
		*/

		readreturn = audio_input_plugin_readdata(joining_data.converter_playerhandle, joining_data.converter_streamhandle, (unsigned long*)&rsize, joining_data.buffer);

		/* end of the file? */
		if(!readreturn)goto point_end;

		/* set block volume, gain */
		if(settings.conversion.volume < 0.98f)
		{
			convert_setblock_volume((fennec_sample*) joining_data.buffer, joining_data.format_freq, joining_data.format_chan, joining_data.format_bps, rsize, (float)settings.conversion.volume);
		}else if(settings.conversion.volume_gain > 0.02f){
			convert_setblock_volume((fennec_sample*) joining_data.buffer, joining_data.format_freq, joining_data.format_chan, joining_data.format_bps, rsize, (float)settings.conversion.volume + 1.0f);
		}

		/* equalize block */

		if(settings.joining.use_equalizer)
		{
			equalize_buffer_variable(joining_data.buffer, 0, joining_data.format_chan, joining_data.format_freq, joining_data.format_bps, rsize, &ehandle);
		}

		/* copy data to the output file */

		encoder_plugin_file_write(joining_data.encoder_playerhandle, joining_data.encoder_streamhandle, joining_data.buffer,
			                      joining_data.format_freq,
								  joining_data.format_bps,
								  joining_data.format_chan, rsize);

		/* call set position to dispaly some info */

		processed_size += rsize;
		if(joining_data.cfunc && current_size)joining_data.cfunc((double)processed_size / (double)current_size);
		
		/* sleep awhile |-) */

		sys_pass();
		
		/* now check for eof */

		if(rsize < joining_data.buffersize)goto point_end;

		/* continue if paused */

		goto point_continue_fast;

point_continue:
		sys_sleep(100);

point_continue_fast:;

	}

point_end:


	/* finish! */

	if(joining_data.cfunc)joining_data.cfunc(1.0f);

	audio_input_plugin_unloadfile(joining_data.converter_playerhandle, joining_data.converter_streamhandle);

	return 1;
}

int audiojoining_end(int hjoin /* not used */)
{
	if(!joining_started)return 0;

	sys_mem_free(joining_data.buffer);
	audio_input_plugin_unloadfile(joining_data.converter_playerhandle, joining_data.converter_streamhandle);
	encoder_plugin_file_close(joining_data.encoder_playerhandle, joining_data.encoder_streamhandle);
	/*encoder_plugin_uninitialize(joining_data.encoder_playerhandle);
	  encoder_uninitialize(); */
	joining_started = 0;

	return 1;
}

/*-----------------------------------------------------------------------------
 fennec, may. september. 2007.
-----------------------------------------------------------------------------*/

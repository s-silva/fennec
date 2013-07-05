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


/* RAM and M3U reading should be in a separate section */

#define pls_format_asx   0
#define pls_format_wax   1
#define pls_format_wvx   2
#define pls_format_pls   3
#define pls_format_b4s   4
#define pls_format_magma 5
#define pls_format_smil  6
#define pls_format_wpl   7
#define pls_format_rdf   8
#define pls_format_xspf  9

#define max_playlist_adds_on_load 500

string format_search_strings[] = {uni("<ref"), uni("<ref"), uni("<ref"),
								  uni("file"), uni("<entry playstring"), uni("&as"),
								  uni("<audio"), uni("<audio"), uni("<dc:identifier>")};

int read_playlist_general(int format, string fmem);
int playlist_read_m3u(const string cpath, const string line);
int playlist_read_pls(const string cpath, const string line);

sys_thread_function_header(thread_playlist_extra);

letter last_playlist_fname[v_sys_maxpath];

string current_playlist_path = 0;

/*
 * save current playlist (audio.playlist) to a text file.
 * zero: sucessful, negative: error, positive: warning.
 */
int playlist_t_save_current(const string fname)
{
	letter                apath[v_sys_maxpath];
	register unsigned int i;
	unsigned int          c = audio_playlist_getcount();
	t_sys_file_handle     fhandle;
	uint8_t               rhead[2] = {0xff, 0xfe};

	fhandle = sys_file_createforceshare(fname, v_sys_file_forwrite);
	if(fhandle == v_error_sys_file_create)return -1;

	sys_file_write(fhandle, rhead, 2);

	for(i=0; i<c; i++)
	{
		str_cpy(apath, audio_playlist_getsource(i));
		str_cat(apath, uni("\r\n"));
		sys_file_write(fhandle, apath, (unsigned long) str_size(apath));
	}

	sys_file_close(fhandle);
	return 0;
}

string playlist_t_get_extensions(int i)
{
	switch(i)
	{
	case 0:
		return uni("txt");

	case 1:
		return uni("m3u");

	case 2:
		return uni("ram");

	case 3:
		return uni("m3u8");

	case pls_format_asx + 4:
		return uni("asx");

	case pls_format_wax + 4:
		return uni("wax");

	case pls_format_wvx + 4:
		return uni("wvx");

	case pls_format_pls + 4:
		return uni("pls");

	case pls_format_b4s + 4:
		return uni("b4s");

	case pls_format_magma + 4:
		return uni("magma");

	case pls_format_smil + 4:
		return uni("smil");

	case pls_format_wpl + 4:
		return uni("wpl");

	case pls_format_rdf + 4:
		return uni("rdf");

	case pls_format_xspf + 4:
		return uni("xspf");

	default:
		return 0;
	}

}

int playlist_t_isplaylistfile(const string fname)
{
	int                    fmt_i = 0, fmt_len;
	letter                 fextbuf[64];

	if(!fname)return 0;

	fmt_len = str_len(fname) - 1;

	while(fname[fmt_len] != uni('.'))
	{
		fmt_len--;
	}
	fmt_len++;

	str_cpy(fextbuf, fname + fmt_len);


	fmt_i = 0;
	while(playlist_t_get_extensions(fmt_i))
	{
		if(str_icmp(fextbuf, playlist_t_get_extensions(fmt_i)) == 0)return 1;
		fmt_i++;
	}

	return 0;
}

int playlist_t_load_current_ex(const string fname, int startpos, int uselimit)
{
	register unsigned int  i = 0;
	unsigned int           tsize, lpos = 0;
	int                    foundsec;
	string                 tmem;
	t_sys_file_handle      fhandle;
	letter                 cpath[v_sys_maxpath];
	uint8_t                rhead[2];
	int                    unimode = 0, uniheadsize = 0;
	int                    fmt_found = 0, fmt_i = 0, fmt_len;
	letter                 fextbuf[64];
	int                    fcount = 0;


	tsize = str_len(fname);
	str_cpy(cpath, fname);

	while(tsize && (cpath[tsize] != uni('\\')) && (cpath[tsize] != uni('/')))
	{
		tsize--;
	}

	cpath[tsize] = uni('\0');



	fhandle = sys_file_openshare(fname, v_sys_file_forread);
	if(fhandle == v_error_sys_file_open)return -1; /* error: invalid file */

	tsize = sys_file_getsize(fhandle);
	if(!tsize)
	{
		sys_file_close(fhandle);
		return -2; /* error: empty file */
	}


	/* get file format */

	fmt_len = str_len(fname) - 1;

	while(fname[fmt_len] != uni('.'))
	{
		fmt_len--;
	}
	fmt_len++;

	str_cpy(fextbuf, fname + fmt_len);


	fmt_i = 0;
	while(playlist_t_get_extensions(fmt_i))
	{
		if(str_icmp(fextbuf, playlist_t_get_extensions(fmt_i)) == 0)
		{
			fmt_found = 1;
			break;
		}
		fmt_i++;
	}

	/* read file */

	sys_file_read(fhandle, rhead, 2);

	if(rhead[0] == 0xff && rhead[1] == 0xbb)
	{
		sys_file_read(fhandle, rhead, 1); /* utf-8 */
		if(rhead[0] == 0xbf)
		{
			uniheadsize = 3;
			unimode = 1;
		}
	}

	if(rhead[0] == 0xff && rhead[1] == 0xfe) /* utf-16 little endian */
	{
		uniheadsize = 2;
		unimode = 2;
	}

	if(rhead[0] == 0xfe && rhead[1] == 0xff) /* utf-16 big endian */
	{
		uniheadsize = 2;
		unimode = 3;
	}

	if(unimode == 0)
	{
		uint8_t *fmem;
		
		fmem = (uint8_t*) sys_mem_alloc(tsize);
		tmem = (string) sys_mem_alloc(tsize * sizeof(letter));
		
		sys_file_read(fhandle, fmem + 2, tsize - 2);
		fmem[0] = rhead[0];
		fmem[1] = rhead[1];

		MultiByteToWideChar(CP_ACP, 0, (LPCSTR)fmem, -1, tmem, tsize);

		tsize *= sizeof(letter);
	}

	if(unimode == 2)
	{
		tsize -= uniheadsize;

		tmem = (string) sys_mem_alloc(tsize + 1);
		
		sys_file_read(fhandle, tmem, tsize);
	}

	if(fmt_found && fmt_i >= 3)
	{
		current_playlist_path = cpath;

		read_playlist_general(fmt_i - 3, tmem);

	}else{


		for(;;)
		{
			if(i > tsize / sizeof(letter))break;

			foundsec = 0;

			if((tmem[i] == uni('\r') || tmem[i] == uni('\n')) || i == tsize)
			{
				if((tmem[i + 1] == uni('\r')) || (tmem[i + 1] == uni('\n'))) foundsec = 1;

				tmem[i] = 0;

				if(i - lpos > 0)
				{
					int rv;
					//if(isalpha(tmem[lpos]) && (tmem[lpos + 1] == uni(':')))
					//{
					//	audio_playlist_add(tmem + lpos, 1, 1);
					//}

					if(!uselimit && fcount < startpos)sys_sleep(0);

					if(fcount >= startpos)
					{
						if(!uselimit)
						{
							sys_sleep(0);

							if(fcount % 200 == 0)
							{
								sys_sleep(2);
							}
						}

						if(uselimit)
						{
							if(fcount > max_playlist_adds_on_load)
							{
								str_cpy(last_playlist_fname, fname);
								sys_thread_call((t_sys_thread_function)thread_playlist_extra);
								sys_mem_free(tmem);
								sys_file_close(fhandle);
								return 0;
							}
						}

						if(fmt_found)
						{
							switch(fmt_i)
							{
							case 0:
							case 1:
							case 2:
							case 3:
								playlist_read_m3u(cpath, tmem + lpos); /* ram */
								break;
							}
						}else{
							rv = playlist_read_m3u(cpath, tmem + lpos);
						}
					}
					fcount++;
				}

				tmem[i] = uni('\r');

				lpos = i + 1;
				if(foundsec)
				{
					lpos++;	/* for 'rn' */
					i++;
				}
				if(i == tsize) break;
			}
			
			i += 1;

		}
	}

	sys_mem_free(tmem);
	sys_file_close(fhandle);
	return 0;
}

int playlist_t_load_current(const string fname)
{
	return playlist_t_load_current_ex(fname, 0, 1);
}

sys_thread_function_header(thread_playlist_extra)
{
	playlist_t_load_current_ex(last_playlist_fname, max_playlist_adds_on_load + 1, 0);
	fennec_refresh(fennec_v_refresh_force_high);
	return 0;
}

int playlist_read_m3u(const string cpath, const string line)
{
	letter		npath[v_sys_maxpath];

	if(line[0] == uni('#')) return 0; // comment/control line

	if(line[1] == uni(':')) goto point_abs;
	if(str_icmp(line, uni("http:")) == 0) goto point_abs;
	if(str_icmp(line, uni("ftp:")) == 0) goto point_abs;

	str_cpy(npath, cpath);
	str_cat(npath, uni("\\"));
	str_cat(npath, line);
	
	audio_playlist_add(npath, 1, 1);
	return 1;

point_abs:

	audio_playlist_add(line, 1, 1);
	return 1;
}

int playlist_read_pls(const string cpath, const string line)
{
	letter		npath[v_sys_maxpath];
	letter		fname[v_sys_maxpath];
	int         idummy;
	int         i = 0;

	if(line[0] == uni('\0')) return 0; // empty line

	if(swscanf(line, uni("File%d=%s"), &idummy, fname) != 2)return 0;

	while(line[i] != uni('='))i++;
	i++;

	str_cpy(fname, line + i);


	if(fname[1] == uni(':')) goto point_abs;
	if(str_icmp(fname, uni("http:")) == 0) goto point_abs;
	if(str_icmp(fname, uni("ftp:")) == 0) goto point_abs;

	str_cpy(npath, cpath);
	str_cat(npath, uni("\\"));
	str_cat(npath, fname);
	
	audio_playlist_add(npath, 1, 1);
	return 1;

point_abs:

	audio_playlist_add(fname, 1, 1);
	return 1;
}









/*
   credits should go to Lucas Gonze <http://gonze.com/playlists/playlist-format-survey.html>
   this is a hack based on the document. needs unicode conversion.
   - chase <c-h@users.sf.net>
*/

int playlist_scanf(int format, const string strin, string pfile)
{
	letter dummy[1024];

#ifdef system_microsoft_windows 

	switch(format)
	{
	case pls_format_asx:
	case pls_format_wax:
	case pls_format_wvx:
		return swscanf(strin, uni("<ref href = \"%[^\"]/>"), pfile);
	case pls_format_pls:
		return swscanf(strin, uni("file%[^=]=%[^\n]"), dummy, pfile);
	case pls_format_b4s:
		return swscanf(strin, uni("<entry playstring = \"%[^\"]"), pfile);
	case pls_format_magma:
		return swscanf(strin, uni("&as = %[^\n]"), pfile);
	case pls_format_smil:
	case pls_format_wpl:
		return swscanf(strin, uni("<audio src = \"%[^\"]/>"), pfile);
	case pls_format_rdf:
		return swscanf(strin, uni("<dc:identifier>%[^<]"), pfile);
	case pls_format_xspf:
		return swscanf(strin, uni("<location>%[^<]</location>"), pfile);
	}

#endif
	return 0;
}


void add_playlist_item(string itm)
{
	letter ndata[1024];
	letter npath[1024];
	string nitm = ndata;
	int    stillleading = 1;

	while(*itm){
		if(stillleading)
		{
			/* [will need to leave one slash in unix systems] */
			if(*itm == uni('\\') || *itm == uni('/') || *itm == uni(' '))
			{
				itm++; continue;
			}else{
				stillleading = 0;
			}
		}

		if(*itm != uni('\n') && *itm != uni('\r') && *itm != uni('\t'))
		{
			*nitm = *itm;
			nitm++;
		}
		itm++;
	}
	*nitm = 0;
	nitm = ndata;

	if(str_incmp(ndata, uni("file:"), 5) == 0)
	{
		/* eliminate file: and slashes */
		nitm += 5;
		while(*nitm == uni('\\') || *nitm == uni('/'))nitm++;
		/* unix will need one slash addition here
		*nitm--;
		*/
	}


	if(nitm[1] == uni(':')) goto point_abs;
	if(str_incmp(nitm, uni("http:"), 5) == 0) goto point_abs;
	if(str_incmp(nitm, uni("ftp:"), 4) == 0) goto point_abs;
	if(str_incmp(nitm, uni("rtsp:"), 5) == 0) goto point_abs;

	str_cpy(npath, current_playlist_path);
	str_cat(npath, uni("\\"));
	str_cat(npath, nitm);
	
	audio_playlist_add(npath, 1, 1);
	return;

point_abs:

	audio_playlist_add(nitm, 1, 1);
}

int read_playlist_general(int format, string fmem)
{
	letter pfile[1024];
	string psubstr;
	string fmemlower;
	string npos;

	int    fstart, flen;

	
	if(!fmem) return 0;

	fmemlower = sys_mem_alloc(str_size(fmem) + sizeof(letter));
	str_cpy(fmemlower, fmem);
	fmemlower = str_lower(fmemlower);

	psubstr = fmemlower;

	while((psubstr = str_str(psubstr + 1, format_search_strings[format])) != 0)
	{
		if(!playlist_scanf(format, psubstr, pfile))continue;

		npos = str_str(psubstr, pfile);
		fstart = (int)(npos - fmemlower);
		flen = str_size(pfile);

		memcpy(pfile, fmem + fstart, flen);
		add_playlist_item(pfile);
	}

	sys_mem_free(fmemlower);
	return 1;
}

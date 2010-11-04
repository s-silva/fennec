#include "subtitles.h"

#define subtitles_currentline_size 4096

int subtitle_count = 0;
int subtitle_index = 0;
int subtitle_mode = 0;


typedef HANDLE t_sys_fs_find_handle;

int sys_fs_find_start(const string inpath, string outpath, size_t osize, t_sys_fs_find_handle* shandle)
{
	WIN32_FIND_DATA wfd;

	*shandle = FindFirstFile(inpath, &wfd);

	if(*shandle == INVALID_HANDLE_VALUE)return 0;

	//memset(outpath, 0, osize);
	str_cpy(outpath, wfd.cFileName);
	return 1;
}

int sys_fs_find_next(string outpath, size_t osize, t_sys_fs_find_handle shandle)
{
	WIN32_FIND_DATA wfd;

	if(FindNextFile(shandle, &wfd))
	{
		//memset(outpath, 0, osize);
		str_cpy(outpath, wfd.cFileName);
		return 1;
	}else{
		return 0;
	}
}

int sys_fs_find_close(t_sys_fs_find_handle shandle)
{
	if(FindClose(shandle))
	{
		return 1;
	}else{
		return 0;
	}
}

void terminate_to_path(string fpath)
{
	int     i;
	i = (int)str_len(fpath);

	for(; i>0; i--)
	{
		if(fpath[i] == uni('/') || fpath[i] == uni('\\'))
		{
			fpath[i] = 0;
			break;
		}
	}
}

int subtitles_load(const string fpath, struct sublist *list, int sforce)
{
	int     i, found;
	letter  subpath[1024];
	letter  subsearch[1024], subsearchret[1024];
	static  letter last_subpath[1024];
	t_sys_fs_find_handle fh;

	list->count = 0;
	list->current_index = 0;
	list->lines = 0;
	list->current_line = sys_mem_alloc(subtitles_currentline_size);
	list->loaded = 1;

	if(sforce)goto pos_forced;

	if(fpath)
	{
		str_cpy(last_subpath, fpath);
		str_cpy(subpath, fpath);
	}else{
		str_cpy(subpath, last_subpath);
	}

	i = (int)str_len(subpath);
	for(; i>0; i--)
	{
		if(subpath[i] == uni('.'))
		{
			subpath[i] = 0;
			break;
		}
	}

	subtitle_count = 0;

	//str_cat(subpath, uni(".srt"));
	str_cpy(subsearch, subpath);
	str_cat(subsearch, uni("*.srt"));

	found = sys_fs_find_start(subsearch, subsearchret, 1024, &fh);
	if(!found)subtitle_index = 0;

	while(found)
	{
		if(subtitle_count == subtitle_index)
		{
			subtitle_mode = sub_mode_srt;
			terminate_to_path(subpath);
			str_cat(subpath, "/");
			str_cat(subpath, subsearchret);
		}

		subtitle_count++;
		found = sys_fs_find_next(subsearchret, 1024, fh);
	}

	sys_fs_find_close(fh);

pos_forced:

	if(sforce)
	{
		subtitle_index = 0;
		subtitle_mode = sub_mode_custom;
	}

	switch(subtitle_mode)
	{
	case sub_mode_srt:
		return open_subrip(subpath, list);
	case sub_mode_custom:
		if(sforce)
		{
			int rv;
			rv = open_subrip(fpath, list);
			return rv;
		}
		
	}
	return 0;
}

int subtitles_unload(struct sublist *list)
{
	unsigned long i, j;
	if(!list) return 0;

	for(i=0; i<list->count; i++)
	{
		for(j=0; j<(unsigned long)list->lines[i].line_count; j++)
		{
			free(list->lines[i].lines[j]);
		}
	}
	free(list->lines);
	sys_mem_free(list->current_line);
	list->current_line = 0;
	list->loaded = 0;
	//free(list);
	return 1;
}

string subtitles_get(struct sublist *list, float ctime)
{
	unsigned long i, j;

	for(i=0; i<list->count; i++)
	{
		if((list->lines[i].starttime <= ctime) && (list->lines[i].endtime >= ctime))
		{
			memset(list->current_line, 0, 10);

			for(j=0; j<(unsigned long)list->lines[i].line_count; j++)
			{
				str_cat(list->current_line, list->lines[i].lines[j]);
				str_cat(list->current_line, uni("\n"));
			}

			return list->current_line;//lines[i].lines[0];
		}
	}
	return 0;
}


int sub_open(const string fname, struct sub_file_data *sfile)
{
	void              *fdata;
	uint8_t           *rdata;
	unsigned long      fsize;
	t_sys_file_handle  fhandle;

	sfile->data = 0;

	fhandle = sys_file_open(fname, v_sys_file_forread);
	if(fhandle == v_error_sys_file_open) return 0;
	
	fsize = sys_file_getsize(fhandle);
	if(!fsize)
	{
		sys_file_close(fhandle);
		return 0;
	}
	
	fdata = sys_mem_alloc(fsize + 4);
	sys_file_read(fhandle, fdata, fsize);


	rdata = (uint8_t*)fdata;

	if(rdata[0] = 0xff && rdata[1] == 0xfe) /* utf-16 little endian - no changes */
	{
		sfile->cptr = 2;
	}else if(rdata[0] = 0xef && rdata[1] == 0xbb && rdata[2] == 0xbf){ /* utf-8 */

		BOOL usedef = 1;


		/* calculate needed memory */
		fsize = MultiByteToWideChar(CP_UTF8, 0, rdata + 3, -1, 0, 0);
		
		fdata = sys_mem_alloc(fsize * sizeof(letter) + 4);

		MultiByteToWideChar(CP_UTF8, 0, rdata + 3, -1, fdata, fsize);

		sys_mem_free(rdata);
		sfile->cptr = 0;

	}else{ /* should be ansi */

		int lsize = fsize;

		/* calculate needed memory */
		fsize = MultiByteToWideChar(CP_UTF8, 0, rdata, lsize, 0, 0) * sizeof(letter);
		
		fdata = sys_mem_alloc((fsize * sizeof(letter)) + 16);

		MultiByteToWideChar(CP_UTF8, 0, rdata, lsize, fdata, fsize);

		sys_mem_free(rdata);
		sfile->cptr = 0;
	}

	sfile->data = fdata;
	sfile->size = fsize;
	
	sys_file_close(fhandle);
	return 0;
}

int sub_close(struct sub_file_data *sfile)
{
	if(!sfile) return 0;
	if(sfile->data) return 0;
	sys_mem_free(sfile->data);
	sfile->data = 0;
	return 1;
}

int sub_read(struct sub_file_data *sfile, string sline)
{
	unsigned long i = 0, k = sfile->cptr;
	string rdata = (string)sfile->data;

	for(;;)
	{
		if(i + k >= sfile->size)
			return 0;
		if(rdata[i + k] == uni('\n') || i >= 120)
		{
			sline[i - 1] = 0;
			sline[i] = 0;
			sfile->cptr = k + i + 1;
			return 1;
		}else{
			sline[i] = rdata[i + k];
		}
		i++;
	}
}







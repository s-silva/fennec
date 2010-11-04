#include "subtitles.h"
#include "subtitle interface.h"


struct subtitle_file
{
	wchar_t  path[260];
	wchar_t  name[128];
	int     language_id;
};


struct subtitle_file *sub_files = 0;
int                   sub_file_count;

struct subtitle_set *subs_pri = 0;
struct subtitle_set *subs_sec = 0;

int                  sel_sub_sec = -1, sel_sub_pri = -1;
int                  sel_sub_default = -1;


void add_subfile(const string fpath, const string fname, int lid)
{
	int id;

	id = sub_file_count;
	sub_file_count++;

	sub_files = (struct subtitle_file*) realloc(sub_files, sub_file_count * sizeof(struct subtitle_file));

	sub_files[id].language_id = lid;
	str_cpy(sub_files[id].path, fpath);
	str_cpy(sub_files[id].name, fname);
}

int is_exist_subfile(const string fpath)
{
	int i;
	for(i=0; i<sub_file_count; i++)
		if(str_cmp(sub_files[i].path, fpath) == 0) return 1;
	return 0;
}

void free_subtitle_files()
{
	if(sub_files)
	{
		free(sub_files);
		sub_files = 0;
		sub_file_count = 0;
	}

	if(subs_pri) sub_list_uninitialize(subs_pri);
	if(subs_sec) sub_list_uninitialize(subs_sec);
	subs_pri = 0;
	subs_sec = 0;
	sel_sub_default = -1;
}


void detect_subtitle_files(const string fpath)
{
	WIN32_FIND_DATA wfd;
	HANDLE          fhandle;
	letter   tpath[v_sys_maxpath];
	letter   tfpath[v_sys_maxpath];
	letter   afpath[v_sys_maxpath];
	letter   fnpath[v_sys_maxpath];
	letter   mfilename[v_sys_maxpath];
	int      i, k, j;
	string   ext;
	int      mfilename_len;


	str_cpy(tpath, fpath);

	for(i=str_len(fpath) - 1;i>=0;i--)
		if(fpath[i] == uni('/') || fpath[i] == uni('\\')) break;

	tpath[i] = 0;

	str_cpy(fnpath, fpath);

	str_cpy(mfilename, tpath + i + 1);
	for(j=str_len(mfilename) - 1; j>=0; j--)
		if(mfilename[j] == uni('.')) break;

	if(j >= 0) mfilename[j] = 0;

	mfilename_len = j;


	for(i=str_len(fpath) - 1;i>=0;i--)
		if(fpath[i] == uni('.')) break;

	fnpath[i] = 0;

	free_subtitle_files();

	
	k = 0;

	for(k=0; k<4; k++)
	{
		i = 0;
		ext = get_subtitle_file_extension(i);
		while(ext)
		{
			str_cpy(tfpath, tpath);
			switch(k)
			{
			case 0: 
				str_cat(fnpath, uni("*"));
				str_cpy(tfpath, fnpath);
				break;
			case 1: str_cat(tfpath, L"/*."); break;
			case 2: str_cat(tfpath, uni("/Subtitles/*.")); break;
			case 3: str_cat(tfpath, uni("/Subs/*.")); break;
			}
			str_cat(tfpath, ext);
			

			fhandle = FindFirstFile(tfpath, &wfd);
			
			if(fhandle != INVALID_HANDLE_VALUE)
			{
				str_cpy(afpath, tpath);
				str_cat(afpath, L"/");
				str_cat(afpath, wfd.cFileName);

				if(sel_sub_default == -1)
				{
					if(str_incmp(wfd.cFileName, mfilename, mfilename_len) == 0)
					{
						sel_sub_default = sub_file_count;
					}
				}

				if(k == 1)
				{
					if(!is_exist_subfile(afpath))
						add_subfile(afpath, wfd.cFileName, 0);
				}else{
					add_subfile(afpath, wfd.cFileName, 0);
				}

				while(fhandle != INVALID_HANDLE_VALUE)
				{
					if(!FindNextFile(fhandle, &wfd))break;

					str_cpy(afpath, tpath);
					str_cat(afpath, L"/");
					str_cat(afpath, wfd.cFileName);

					if(sel_sub_default == -1)
					{
						if(str_incmp(wfd.cFileName, mfilename, mfilename_len) == 0)
						{
							sel_sub_default = sub_file_count;
						}
					}

					if(k == 1)
					{
						if(!is_exist_subfile(afpath))
							add_subfile(afpath, wfd.cFileName, 0);
					}else{
						add_subfile(afpath, wfd.cFileName, 0);
					}
				}

				FindClose(fhandle);
			}

			i++;
			ext = get_subtitle_file_extension(i);
		}
	}

}

int get_subtitle_file_count()
{
	if(!sub_files) return 0;
	return sub_file_count;
}

const string get_subtitle_file_name(int id)
{
	if(!sub_files) return 0;
	if(id < 0 || id >= sub_file_count) return 0;

	return sub_files[id].name;
}

const string get_subtitle_file_language(int id)
{
	if(!sub_files) return 0;
	if(id < 0 || id >= sub_file_count) return 0;

	return uni("Default");
}

void set_primary_subtitle_file(int id)
{
	int  fid;

	if(id == -1) id = sel_sub_default;

	if(id == -1) return;

	if(!sub_files) return;
	if(id >= sub_file_count) return;

	if(subs_pri) sub_list_uninitialize(subs_pri);

	if(!id < 0)
	{
		subs_pri = 0;
		sel_sub_pri = 0;
		return;
	}

	subs_pri = sub_list_initialize();

	fid = get_file_type(sub_files[id].path);
	read_text_subtitle_file(sub_files[id].path, fid, subs_pri);
	sel_sub_pri = id;
}

void set_secondary_subtitle_file(int id)
{
	int  fid;

	if(!sub_files) return;
	if(id >= sub_file_count) return;

	if(subs_sec) sub_list_uninitialize(subs_sec);

	if(!id < 0)
	{
		subs_sec = 0;
		sel_sub_sec = -1;
		return;
	}

	subs_sec = sub_list_initialize();

	fid = get_file_type(sub_files[id].path);
	read_text_subtitle_file(sub_files[id].path, fid, subs_sec);
	sel_sub_sec = id;
}

void set_custom_primary_subtitle_file(const string fpath)
{
	int  fid;

	if(subs_pri) sub_list_uninitialize(subs_pri);
	if(!fpath)
	{
		subs_pri = 0;
		sel_sub_pri = -1;
		return;
	}

	subs_pri = sub_list_initialize();

	fid = get_file_type(fpath);
	read_text_subtitle_file(fpath, fid, subs_pri);
	sel_sub_pri = -2; /* custom */
}


const string get_line(int sec, float ctime)
{
	int                  i;
	struct subtitle_set *subs;

	if(sec)
		subs = subs_sec;
	else
		subs = subs_pri;

	if(!subs) return 0;

	for(i=0; i<subs->line_count; i++)
	{
		if((subs->data[i].start_time <= ctime) && (subs->data[i].end_time >= ctime))
		{
			if(sec)
				return subs->data[i].text;
			else 
				return subs->data[i].text;
		}
	}
	return 0;
}

void free_loaded_subtitles()
{
	sel_sub_sec = -1;
	sel_sub_pri = -1;
	if(subs_sec) sub_list_uninitialize(subs_sec);
	if(subs_pri) sub_list_uninitialize(subs_pri);
	subs_sec = 0;
	subs_pri = 0;
	sel_sub_default = -1;
}

int get_current_subtitle_file_id(int sec)
{
	if(sec)
		return sel_sub_sec;
	else
		return sel_sub_pri;
}
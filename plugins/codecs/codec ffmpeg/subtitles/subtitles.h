#include "..\..\..\..\include\system.h"
#include "..\..\..\..\include\fennec.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>



struct sub_line
{
	double start_time; /* star time (milliseconds) */
	double end_time;
	string text;	
};


struct subtitle_set
{
	int              line_count;
	int              alloc_count;
	struct sub_line *data;
	letter           temp_line[2048];
	int              temp_i1, temp_i2, temp_i3;
	double           temp_d1, temp_d2, temp_d3;
};

struct subtitle_set* sub_list_initialize();
void   sub_list_uninitialize(struct subtitle_set* subset);

const  string get_file_type_info(int id);
int    get_file_type(const string fname);
void   read_text_subtitle_file(const string fpath, int subformat, struct subtitle_set* subset);
int    is_subtitle_file_extension(const string ext);
const string get_subtitle_file_extension(int id);

void detect_subtitle_files(const string fpath);
int  get_subtitle_file_count();
const string get_subtitle_file_name(int id);
const string get_subtitle_file_language(int id);
void set_primary_subtitle_file(int id); /* -1 = default */
void set_secondary_subtitle_file(int id);
const string get_line(int sec, float ctime);
void free_loaded_subtitles();
void set_custom_primary_subtitle_file(const string fpath);
int get_current_subtitle_file_id(int sec);
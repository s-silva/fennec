#include "..\..\..\include\system.h"
#include <stdio.h>


#define single_line_length 1024
#define max_lines          16


extern int subtitle_count;
extern int subtitle_index;
extern int subtitle_mode;


enum textmodes
{
	 sub_text_mode_normal
	,sub_text_mode_bold
	,sub_text_mode_italic
	,sub_text_mode_bold_italic
};

enum
{
	sub_mode_srt,
	sub_mode_custom
};


struct subline
{
	float          starttime;  /* display time in seconds */
	float          endtime;    /* hide time in seconds */
	unsigned char  c_red;
	unsigned char  c_green;
	unsigned char  c_blue;
	string         lines[max_lines];
	int            line_count;
	enum textmodes text_mode;
};

struct sublist
{
	struct subline  *lines;
	unsigned long    count;
	unsigned long    current_index;  /* for the user */
	string           current_line;
	int              loaded;
};


struct sub_file_data
{
	string        data;
	unsigned long size;
	int           cptr;
};


int sub_open(const string fname, struct sub_file_data *sfile);
int sub_close(struct sub_file_data *sfile);
int sub_read(struct sub_file_data *sfile, string sline);
typedef struct sub_file_data sub_file;




int subtitles_load(const string fpath, struct sublist *list, int sforce);
int subtitles_unload(struct sublist *list);
string subtitles_get(struct sublist *list, float ctime);


int open_subrip(string fname, struct sublist *list);


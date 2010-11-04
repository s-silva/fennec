/* This program is licensed under the GNU General Public License, 
 * version 2, a copy of which is included with this program.
 *
 * (c) 2000-2002 Michael Smith <msmith@xiph.org>
 * (c) 2001 Ralph Giles <giles@xiph.org>
 *
 * Front end to show how to use vcedit;
 * Of limited usability on its own, but could be useful.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#include <share.h>

#include <wchar.h>

#include "main.h"
#include "utf8.h"
#include "i18n.h"

#include "vcedit.h"


/* local parameter storage from parsed options */
typedef struct {
	int    commentcount;
	char **comments;
} param_t;

#define MODE_NONE  0
#define MODE_LIST  1
#define MODE_WRITE 2
#define MODE_APPEND 3

/* prototypes */
void usage(void);
void print_comments(FILE *out, vorbis_comment *vc, int raw);
int  add_comment(char *line, vorbis_comment *vc, int raw);

param_t	*new_param(void);
void free_param(param_t *param);
void parse_options(int argc, char *argv[], param_t *param);
void open_files(param_t *p);
void close_files(param_t *p);

/**********

   Take a line of the form "TAG=value string", parse it, convert the
   value to UTF-8, and add it to the
   vorbis_comment structure. Error checking is performed.

   Note that this assumes a null-terminated string, which may cause
   problems with > 8-bit character sets!

***********/

int  add_comment(char *comment, vorbis_comment *vc, int tlen)
{
	int i, found = -1;

	for(i = 0;  i < vc->comments;  i++)
	{
		if(!memicmp(comment, vc->user_comments[i], tlen))
			found = i;
	}

	if(found != -1)
	{
		vc->comment_lengths[found] = (int)strlen(comment);

		vc->user_comments[found] = realloc(vc->user_comments[found],
					                      (vc->comment_lengths[found] + 1));

		strcpy(vc->user_comments[found], comment);


	}else{
		vc->user_comments = realloc(vc->user_comments,
					(vc->comments+2)*sizeof(*vc->user_comments));

		vc->comment_lengths = realloc(vc->comment_lengths,
      						(vc->comments+2)*sizeof(*vc->comment_lengths));

		vc->comment_lengths[vc->comments] = (int)strlen(comment);

		vc->user_comments[vc->comments] = malloc(vc->comment_lengths[vc->comments]+1);
		strcpy(vc->user_comments[vc->comments], comment);
		
		vc->comments++;
		vc->user_comments[vc->comments] = NULL;
	}
	return 0;
}


void free_param(param_t *param)
{
    free(param);
}

/**********

   allocate and initialize a the parameter struct

***********/

param_t *new_param(void)
{
	param_t *param = (param_t *)malloc(sizeof(param_t));

	param->commentcount = 0;
	param->comments     = 0;
	return param;
}

void local_addcomment(vorbis_comment *cms, const char *name, const string data)
{
	char    *cm;
	size_t   sz;
	int      vlen;

	sz = (str_size(data) * 4);

	cm = (char*) malloc(strlen(name) + sz + 2);

	strcpy(cm, name);
	strcat(cm, "=");
	
	WideCharToMultiByte(CP_UTF8, 0, data, -1, cm + (int)strlen(name) + 1, (int)sz, 0, 0);

	vlen = (int)strlen(cm) /* would this work for UTF-8 ? */;

	add_comment(cm, cms, vlen);
}

int write_ogg_tag(const string fname, struct fennec_audiotag *wtag)
{
	vcedit_state   *state;
	vorbis_comment *vc;
	param_t	       *param;
	FILE           *tfile;
	FILE           *of;
	string          outname;

	struct fennec_audiotag_item *ct;

	if(!fname || !wtag)return -3;

	setlocale(LC_ALL, "");

	/* initialize the cmdline interface */
	param = new_param();
	
	tfile = _wfsopen(fname, uni("r+b"), _SH_DENYRW);

	if(!tfile)
	{
		MessageBox(0, uni("Access denied, please stop playback and try again (you don't need to close this window)."), uni("Tag Editing"), MB_ICONINFORMATION);
		return -1;
	}

	state = vcedit_new_state();
	
	if(vcedit_open(state, tfile) < 0)
	{
		fclose(tfile);
		free_param(param);
		vcedit_clear(state);
		return -2;
	}
	
	vc = vcedit_comments(state);

	ct = &wtag->tag_title;         if(ct->tsize)local_addcomment(vc, "TITLE",             ct->tdata); else local_addcomment(vc, "TITLE",              uni("") );
	ct = &wtag->tag_album;         if(ct->tsize)local_addcomment(vc, "ALBUM",             ct->tdata); else local_addcomment(vc, "ALBUM",              uni("") );
	ct = &wtag->tag_artist;        if(ct->tsize)local_addcomment(vc, "ARTIST",            ct->tdata); else local_addcomment(vc, "ARTIST",             uni("") );
	ct = &wtag->tag_origartist;    if(ct->tsize)local_addcomment(vc, "ORIGINALARTIST",    ct->tdata); else local_addcomment(vc, "ORIGINALARTIST",     uni("") );
	ct = &wtag->tag_composer;      if(ct->tsize)local_addcomment(vc, "COMPOSER",          ct->tdata); else local_addcomment(vc, "COMPOSER",           uni("") );
	ct = &wtag->tag_lyricist;      if(ct->tsize)local_addcomment(vc, "LYRICIST",          ct->tdata); else local_addcomment(vc, "LYRICIST",           uni("") );
	ct = &wtag->tag_band;          if(ct->tsize)local_addcomment(vc, "BANDNAME",          ct->tdata); else local_addcomment(vc, "BANDNAME",           uni("") );
	ct = &wtag->tag_copyright;     if(ct->tsize)local_addcomment(vc, "COPYRIGHT",         ct->tdata); else local_addcomment(vc, "COPYRIGHT",          uni("") );
	ct = &wtag->tag_publish;       if(ct->tsize)local_addcomment(vc, "PUBLISHER",         ct->tdata); else local_addcomment(vc, "PUBLISHER",          uni("") );
	ct = &wtag->tag_encodedby;     if(ct->tsize)local_addcomment(vc, "ENCODEDBY",         ct->tdata); else local_addcomment(vc, "ENCODEDBY",          uni("") );
	ct = &wtag->tag_genre;         if(ct->tsize)local_addcomment(vc, "GENRE",             ct->tdata); else local_addcomment(vc, "GENRE",              uni("") );
	ct = &wtag->tag_year;          if(ct->tsize)local_addcomment(vc, "YEAR",              ct->tdata); else local_addcomment(vc, "YEAR",               uni("") );
	ct = &wtag->tag_url;           if(ct->tsize)local_addcomment(vc, "URL",               ct->tdata); else local_addcomment(vc, "URL",                uni("") );
	ct = &wtag->tag_offiartisturl; if(ct->tsize)local_addcomment(vc, "OFFICIALARTISTURL", ct->tdata); else local_addcomment(vc, "OFFICIALARTISTURL",  uni("") );
	ct = &wtag->tag_comments;      if(ct->tsize)local_addcomment(vc, "COMMENT",           ct->tdata); else local_addcomment(vc, "COMMENT",            uni("") );
	ct = &wtag->tag_lyric;         if(ct->tsize)local_addcomment(vc, "LYRIC",             ct->tdata); else local_addcomment(vc, "LYRIC",              uni("") );
	ct = &wtag->tag_bpm;           if(ct->tsize)local_addcomment(vc, "BPM",               ct->tdata); else local_addcomment(vc, "BPM",                uni("") );
	ct = &wtag->tag_tracknum;      if(ct->tsize)local_addcomment(vc, "TRACKNUMBER",       ct->tdata); else local_addcomment(vc, "TRACKNUMBER",        uni("") );

	outname = (string) malloc(str_size(fname) + (5 * sizeof(letter)));

	str_cpy(outname, fname);
	str_cat(outname, uni(".tmp"));

	of = _wfopen(outname, uni("wb"));

	if(vcedit_write(state, of) < 0)
	{
		fclose(of);
		fclose(tfile);
		free_param(param);
		vcedit_clear(state);
		free(outname);
		return 1;
	}

	fclose(of);
	
	/* done */

	vcedit_clear(state);
	fclose(tfile);
	free_param(param);


	_wremove(fname);
	sys_sleep(0);
	_wrename(outname, fname);
		
	free(outname);
	return 0;
}
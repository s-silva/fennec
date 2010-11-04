/*
 *	Text Based Subtitle Reader Code.
 *	Supports reading over 50 subtitle formats.
 *  (for Fennec Media Project - http://fennec.sf.net)
 *
 *  Copyright (c) Sandaruwan Silva 2010.
 *
 *  Any use  of  this work  or a modified version of  this
 *  work  without  a  prior  written  permission  from the
 *  author is  strictly  prohibited.  No  warranty  either
 *  implicit or implied is included. Use at your own risk.
 */


#include "subtitles.h"


#define subtitle_format_srt   0x1   /* SubRip */
#define subtitle_format_ass   0x2   /* Advanced SubStation Alpha*/
#define subtitle_format_aqt   0x3   /* AQTitle */
#define subtitle_format_890   0x4   /* Scantitle */
#define subtitle_format_stl   0x5   /* Spruce */
#define subtitle_format_dks   0x6   /* DKS */
#define subtitle_format_asc   0x7   /* Cheetah */
#define subtitle_format_lrc   0x8   /* Karaoke Lyrics LRC */
#define subtitle_format_vkt   0x9   /* Karaoke Lyrics VKT */
#define subtitle_format_mpl1  0xa   /* MPlayer1 */
#define subtitle_format_mpl2  0xb   /* MPlayer2 */
#define subtitle_format_js    0xc   /* JACOSub */
#define subtitle_format_zeg   0xd   /* ZeroG */
#define subtitle_format_pimp  0xe   /* ;) Pinnacle Impression (.txt) */
#define subtitle_format_tts   0xf   /* Turbo Titler */
#define subtitle_format_vsf   0x10  /* ViPlay */
#define subtitle_format_mac   0x11  /* MAC DVD Studio Pro (.txt) */
#define subtitle_format_dvds  0x12  /* DVD Subtitle System (.txt) */
#define subtitle_format_dvdj  0x13  /* DVD Junior (.txt) */
#define subtitle_format_ssts  0x14  /* Stream SubText Script */
#define subtitle_format_ssa   0x15  /* SubStation Alpha */
#define subtitle_format_son   0x16  /* Spruce DVDMaestro */
#define subtitle_format_mdvd  0x17  /* MicroDVD (.sub) */
#define subtitle_format_sstso 0x18  /* Sonic Scenarist (.sst) */
#define subtitle_format_psb   0x19  /* Power DivX */
#define subtitle_format_pjs   0x1a  /* Phoenix Japanimation Society */
#define subtitle_format_ovr   0x1b  /* OVR Script */
#define subtitle_format_subcr 0x1c  /* SubCreator (.txt) */
#define subtitle_format_wtt   0x1d  /* Wincaps Text Timecoded (.txt) */
#define subtitle_format_udw   0x1e  /* ULead DVD Workshop (.txt) */
#define subtitle_format_ppx   0x1f  /* PowerPixel (.txt) */
#define subtitle_format_fab   0x20  /* FAB (.txt) */
#define subtitle_format_cav   0x22  /* Cavena (.txt) */
#define subtitle_format_capi  0x23  /* Captions inc. (.txt, .cin) */
#define subtitle_format_subv2 0x24  /* SubViewer 2 (.sub) */
#define subtitle_format_sofni 0x25  /* Sofni (.sub) */
#define subtitle_format_xas   0x26  /* Advanced Subtitles */
#define subtitle_format_cpc   0x27  /* CPC-600 (.txt) */
#define subtitle_format_rt    0x28  /* RealTime */
#define subtitle_format_s2k   0x29  /* Sasami Script */
#define subtitle_format_srtf  0x2a  /* Softitler RTF (.rtf) */
#define subtitle_format_sbt   0x2b  /* SBT */
#define subtitle_format_qtt   0x2c  /* QuickTime Text (.txt) */
#define subtitle_format_pan   0x2d  /* Panimator */
#define subtitle_format_scr   0x2e  /* MacSUB */
#define subtitle_format_subv1 0x2f  /* SubViewer (.sub) */
#define subtitle_format_dass  0x30  /* DVD Architect Subtitle Script (.sub) */
#define subtitle_format_sdvd  0x31  /* Sonic DVD Creator (.sub) */
#define subtitle_format_dsub  0x32  /* DVD Subtitle (.sub) */
#define subtitle_format_sson  0x33  /* SubSonic (.sub) */
#define subtitle_format_c32   0x34  /* Captions 32 (.txt) */
#define subtitle_format_ias   0x35  /* I-Author Script (.txt) */
#define subtitle_format_psvd  0x36  /* Philips SVCD Designer (.sub) */
#define subtitle_format_icg   0x37  /* Inscriber CG (.txt) */

#define str_scan swscanf


double subtitle_framerate = 30.0;


struct subtitle_set* sub_list_initialize()
{
	struct subtitle_set* subset = (struct subtitle_set*)malloc(sizeof(struct subtitle_set));
	subset->line_count          = 0;
	subset->alloc_count         = 10;
	subset->data                = (struct sub_line*)malloc(sizeof(struct sub_line) * subset->alloc_count);

	memset(subset->temp_line, 0, sizeof(subset->temp_line));
	subset->temp_i1 = 0;
	subset->temp_i2 = 0;
	subset->temp_i3 = 0;
	subset->temp_d1 = 0.0;
	subset->temp_d2 = 0.0;
	subset->temp_d3 = 0.0;
	return subset;
}

void sub_list_add(struct subtitle_set* subset, double s_time, double e_time, const string text)
{
	if(subset->line_count + 1 >= subset->alloc_count)
	{
		subset->alloc_count += 10;
		subset->data         = (struct sub_line*)realloc(subset->data, sizeof(struct sub_line) * subset->alloc_count);
	}
	

	subset->data[subset->line_count].start_time = s_time;
	subset->data[subset->line_count].end_time   = e_time;

	subset->data[subset->line_count].text = (string) malloc(str_size(text) + sizeof(letter));
	str_cpy(subset->data[subset->line_count].text, text);

	subset->line_count++;
}

void sub_list_uninitialize(struct subtitle_set* subset)
{
	int i;

	if(!subset) return;

	for(i=0; i<subset->line_count; i++)
	{
		if(subset->data[i].text)
			free(subset->data[i].text);
	}

	free(subset->data);
	free(subset);
}

void sub_list_fix_endtimes(struct subtitle_set* subset)
{
	int i;
	for(i=0; i<subset->line_count; i++)
	{
		if(subset->data[i].end_time < 0.0)
		{
			if(i + 1 < subset->line_count)
				subset->data[i].end_time = subset->data[i + 1].start_time;
			else
				subset->data[i].end_time = subset->data[i].start_time + 100;
		}
	}
}

void replace_2char_with_3(string str, letter s1, letter s2, letter d1, letter d2)
{
	string mstr = str;
	int len = (int)str_len(str), pos = 0, i;
	while(*mstr)
	{
		if(*mstr == s1 && *(mstr + 1) == s2)
		{
			mstr++;
			*mstr = d1;

			for(i=len; i>pos; i--)
			{
				str[i + 1] = str[i];
			}

			*(mstr + 1)= d2;
			mstr++;
			len++;
			pos+=2;
		}
		mstr++;
		pos++;
	}
}

void replace_str(string str, const string s, const string d)
{
	string mstr = str;
	int len, dlen, slen, ldiff, pos, i;

	if(!str) return;
	if(!s) return;
	if(!d) return;
	
	len   = (int)str_len(str);
	pos   = 0;
	dlen  = (int)str_len(d);
	slen  = (int)str_len(s);
	ldiff = dlen - slen;

	while(*mstr)
	{
		if(str_ncmp(mstr, s, slen) == 0)
		{

			if(dlen < slen)
			{
				str_cpy(mstr + dlen, mstr + slen);

			}else if(dlen > slen){

				for(i=len; i>pos; i--)
				{
					str[i + ldiff] = str[i];
				}
			}

			memcpy(mstr, d, dlen * sizeof(letter));

			mstr += dlen;
			len += ldiff;
			pos += dlen;
		}
		mstr++;
		pos++;
	}
}

void replace_1char_with_2(string str, letter s1, letter d1, letter d2)
{
	string mstr = str;
	int len = (int)str_len(str), pos = 0, i;
	while(*mstr)
	{
		if(*mstr == s1)
		{
			*mstr = d1;
			mstr++;

			for(i=len; i>pos; i--)
			{
				str[i + 1] = str[i];
			}

			*mstr = d2;
			mstr++;
			len++;
			pos+=2;
		}
		mstr++;
		pos++;
	}
}

void replace_2char_with_2(string str, letter s1, letter s2, letter d1, letter d2)
{
	string mstr = str;
	while(*mstr)
	{
		if(*mstr == s1 && *(mstr + 1) == s2)
		{
			*mstr = d1;
			mstr++;
			*mstr = d2;
		}
		mstr++;
	}
}

void terminate_last_newlines(string str)
{
	int len = (int)str_len(str), i;
	for(i=len; i>=2; i--)
	{
		if(str[i] == uni('\n') || str[i] == uni('\r'))
		{
			if(str[i - 2] != uni('\n') && str[i - 2] != uni('\r'))
			{
				str[i] = 0;
			}
		}
	}
}

void terminate_last_spaces(string str)
{
	int len = (int)str_len(str) - 1, i;
	if(len < 0)return;
	for(i=len; i>=0; i--)
	{
		if(str[i] == uni(' '))str[i] = uni('\0');
		else return;
	}
}

int get_reverse_position_by_counting(string str, int spaces, letter ch)
{
	int len = (int)str_len(str) - 1, i;
	if(len < 0)return 0;
	for(i=len; i>=0; i--)
	{
		if(str[i] == ch)
		{
			spaces--;
			if(!spaces)return i;
		}
	}
	return 0;
}


void replace_char(string str, letter s, letter d)
{
	string mstr = str;
	while(*mstr)
	{
		if(*mstr == s)
			*mstr = d;
		mstr++;
	}
}




int sub_list_readline(const string l, int format, struct subtitle_set* subset)
{
	int t_a1, t_a2, t_a3, t_a4;
	int t_b1, t_b2, t_b3, t_b4;
	long t_l1;
	static letter ts1[2048];
	int ts1len;
	int tbool = 0;

	switch(format)
	{
	case subtitle_format_srt:

		if(str_scan(l, uni("%d:%d:%d,%d --> %d:%d:%d,%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				subset->temp_line[str_len(subset->temp_line) - subset->temp_i2 - 4] = 0;
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + t_a4;
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + t_b4;
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */
			subset->temp_i2 = 0;

		}else if(subset->temp_i1){

			str_cat(subset->temp_line, l);
			str_cat(subset->temp_line, uni("\n\r"));
			subset->temp_i2 = (int)str_len(l) + 2;
		}
		return 1;

	case subtitle_format_890:

		if(str_scan(l, uni(" %*d %d:%d:%d:%d %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;


	case subtitle_format_psvd:

		if(str_cmp(l, uni("[LIST]")) == 0)
		{
			subset->temp_i2 = 1;
			subset->temp_line[0] = 0;
			return 1;
		}

		if(subset->temp_i2 == 1)
		{
			str_cpy(ts1, l + get_reverse_position_by_counting(l, 6, uni(' ')) + 1);
			
			if(str_scan(ts1, uni("%d:%d:%d:%d %d:%d:%d:%d %*[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
			{
				/* convert to milliseconds */

				str_cpy(ts1, l);
				ts1[get_reverse_position_by_counting(l, 6, uni(' '))] = 0;

				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, ts1);

				subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
				subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);

				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_line[0] = 0;

			}else{

				if(str_len(l) > 0)
				{
					if(subset->temp_line[0])
						str_cat(subset->temp_line, uni("\n\r"));
					str_cat(subset->temp_line, l);
				}
			}
		}
		return 1;

	case subtitle_format_icg:

		if(str_ncmp(l, uni("@@9 LANG"), 8) == 0)
		{
			subset->temp_i2 = 1;
			subset->temp_line[0] = 0;
			return 1;
		}

		if(subset->temp_i2 == 1)
		{
			str_cpy(ts1, l + get_reverse_position_by_counting(l, 2, uni('<')) + 1);
			
			if(str_scan(ts1, uni("%d:%d:%d:%d><%d:%d:%d:%d>"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
			{
				/* convert to milliseconds */

				str_cpy(ts1, l);
				ts1[get_reverse_position_by_counting(l, 2, uni('<'))] = 0;

				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, ts1 + 3);

				subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
				subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);

				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_line[0] = 0;

			}else{

				if(str_len(l) > 3)
				{
					if(subset->temp_line[0])
						str_cat(subset->temp_line, uni("\n\r"));
					str_cat(subset->temp_line, l + 3);
				}
			}
		}
		return 1;

	case subtitle_format_ias:

		if(str_scan(l, uni("BMPFILE: %[^\n\0]"), ts1) == 1)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			str_cpy(subset->temp_line, ts1);
			subset->temp_d1 = -100 /* temporary */;
			subset->temp_d2 = -100 /* temporary */;
			subset->temp_i1 = 1; /* check for markers */

		}else if(subset->temp_i1){

			if(str_scan(l, uni("STARTTIME: %d.%d"), &t_a1, &t_a2) == 2)
			{
				subset->temp_d1 = (t_a1 * 1000) + (t_a2 * 10);
			}

			if(str_scan(l, uni("TIME: %d.%d DISABLE_OGT"), &t_a1, &t_a2) == 2)
			{
				subset->temp_d2 = (t_a1 * 1000) + (t_a2 * 10);
			}
		}
		return 1;


	case subtitle_format_dass:

		if(str_scan(l, uni("%*d %d:%d:%d:%d %d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, ts1) == 9)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			str_cpy(subset->temp_line, ts1);
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
			subset->temp_i1 = 1; /* start of text */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_sdvd:

		if(str_scan(l, uni("%*d %d:%d:%d:%d %d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, ts1) == 9)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			str_cpy(subset->temp_line, ts1);
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 33);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 33);
			subset->temp_i1 = 1; /* start of text */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_pan:

		if(!subset->temp_i2)
		{
			if(str_scan(l, uni("/d %d %d"), &t_a1, &t_a2) == 2)
			{
				if(subset->temp_i1)
				{
					/* add line */
					sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
					subset->temp_i1 = 0;
				}

				/* convert to milliseconds */

				subset->temp_d1 = (t_a1 * 1000) + (t_a2 * 10);
				subset->temp_d2 = -100; /* temporary */
				subset->temp_i1 = 1; /* start of text */
				subset->temp_i2 = 1; /* block this until we find the ending time */
				subset->temp_line[0] = 0; /* empty the line */
				return 1;
			}
		}

		if(subset->temp_i1)
		{
			if(str_scan(l, uni("/d %d %d"), &t_a1, &t_a2) == 2)
			{
				subset->temp_d2 = (t_a1 * 1000) + (t_a2 * 10);
				subset->temp_i2 = 0;

			}else{
				if(str_len(l) > 0)
				{
					if(l[0] != uni('/'))
					{
						if(subset->temp_line[0])
							str_cat(subset->temp_line, uni("\n\r"));
						str_cat(subset->temp_line, l);
					}
				}
			}
		}
		return 1;


	case subtitle_format_scr:

		if(!subset->temp_i2)
		{
			if(str_scan(l, uni("/%d"), &t_a1) ==  1)
			{
				if(subset->temp_i1)
				{
					/* add line */
					sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
					subset->temp_i1 = 0;
				}

				/* convert to milliseconds */

				subset->temp_d1 = ((double)t_a1 / subtitle_framerate) * 1000;
				subset->temp_d2 = -100; /* temporary */
				subset->temp_i1 = 1; /* start of text */
				subset->temp_i2 = 1; /* block this until we find the ending time */
				subset->temp_line[0] = 0; /* empty the line */
				return 1;
			}
		}

		if(subset->temp_i1)
		{
			if(str_scan(l, uni("/%d"), &t_a1) == 1)
			{
				subset->temp_d2 = ((double)t_a1 / subtitle_framerate) * 1000;
				subset->temp_i2 = 0;

			}else{
				if(str_len(l) > 0)
				{
					if(l[0] != uni('/'))
					{
						if(subset->temp_line[0])
							str_cat(subset->temp_line, uni("\n\r"));
						str_cat(subset->temp_line, l);
					}
				}
			}
		}
		return 1;

	case subtitle_format_sson:

		if(!subset->temp_i2)
		{
			if(str_scan(l, uni("%*[^ ] %d.%d %*[^~]~:\\%[^\n\0]"), &t_a1, &t_a2, ts1) == 3)
			{
				if(subset->temp_i1)
				{
					/* add line */
					sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
					subset->temp_i1 = 0;
				}

				/* convert to milliseconds */

				str_cpy(subset->temp_line, ts1);
				subset->temp_d1 = (t_a1 * 1000) + (t_a2 * 10);
				subset->temp_d2 = -100; /* temporary */
				subset->temp_i1 = 1; /* start of text */
				subset->temp_i2 = 1; /* block this until we find the ending time */
				return 1;
			}
		}

		if(subset->temp_i1)
		{
			if(str_scan(l, uni("%*[^ ] %d.%d"), &t_a1, &t_a2) == 2)
			{
				subset->temp_d2 = (t_a1 * 1000) + (t_a2 * 10);
				subset->temp_i2 = 0;

			}else{
				if(str_len(l) > 0)
				{
					if(l[0] != uni('/'))
					{
						if(subset->temp_line[0])
							str_cat(subset->temp_line, uni("\n\r"));
						str_cat(subset->temp_line, l);
					}
				}
			}
		}
		return 1;

	case subtitle_format_dsub:

		if(str_scan(l, uni("{T %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4) ==  4)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = -100; /* temporary */
			subset->temp_i1 = 1; /* start of text */
			subset->temp_i2 = 1; /* block this until we find the ending time */
			subset->temp_line[0] = 0; /* empty the line */
			return 1;

		}else if(subset->temp_i1){
		
			if(str_len(l) > 0)
			{
				if(l[0] != uni('}'))
				{
					if(subset->temp_line[0])
						str_cat(subset->temp_line, uni("\n\r"));
					str_cat(subset->temp_line, l);
				}
			}
		}
		return 1;

	case subtitle_format_subv1:

		if(!subset->temp_i2)
		{
			if(str_scan(l, uni("[%d:%d:%d]"), &t_a1, &t_a2, &t_a3) ==  3)
			{
				if(subset->temp_i1)
				{
					/* add line */

					replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));
					sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
					subset->temp_i1 = 0;
				}

				/* convert to milliseconds */

				subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000);
				subset->temp_d2 = -100; /* temporary */
				subset->temp_i1 = 1; /* start of text */
				subset->temp_i2 = 1; /* block this until we find the ending time */
				subset->temp_i3 = 1; /* text could be parsed */
				subset->temp_line[0] = 0; /* empty the line */
				return 1;
			}
		}

		if(subset->temp_i1)
		{
			if(str_scan(l, uni("[%d:%d:%d]"), &t_a1, &t_a2, &t_a3) == 3)
			{
				subset->temp_d2 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000);
				subset->temp_i2 = 0;
				subset->temp_i3 = 0; /* end of text */

			}else{
				if(subset->temp_i3)
				{
					if(str_len(l) > 0)
					{
						if(subset->temp_line[0])
							str_cat(subset->temp_line, uni("\n\r"));

						str_cat(subset->temp_line, l);
					}
				}
			}
		}
		return 1;

	case subtitle_format_s2k:

		str_cpy(ts1, l);

		if(ts1[0] == uni(';'))
		{
			replace_str(ts1, uni("="), uni(" = "));
		}

		if(str_scan(ts1, uni(";Set.Time.Start = %d"), &t_l1) == 1)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = t_l1;
			subset->temp_d2 = -100; /* will be fixed at the end */;
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1 && ts1[0] != uni(';')){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_qtt:

		if(str_scan(l, uni("[%d:%d:%d.%d]"), &t_a1, &t_a2, &t_a3, &t_a4) == 4)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = -100; /* will be fixed at the end */;
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;


	case subtitle_format_srtf:

		if(!subset->temp_i2)
		{
			if(str_scan(l, uni("\\par %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4) == 4)
			{
				if(subset->temp_i1)
				{
					/* add line */
					sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
					subset->temp_i1 = 0;
				}

				/* convert to milliseconds */

				subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
				subset->temp_d2 = -100; /* temporary fix */;
				subset->temp_i1 = 1; /* start of text */
				subset->temp_i2 = 1; /* block start time reading till we find the end time */
				subset->temp_line[0] = 0; /* empty the line */
				return 1;
			}
		}

		if(subset->temp_i1){

			if(str_scan(l, uni("\\par %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4) == 4)
			{
				subset->temp_d2 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
				subset->temp_i2 = 0;
			}else{

				if(str_len(l) > 0)
				{
					if(str_scan(l, uni("\\par %[^\n\0]"), ts1) == 1)
					{
						if(ts1[0] != uni('['))
						{
							if(subset->temp_line[0])
								str_cat(subset->temp_line, uni("\n\r"));
							str_cat(subset->temp_line, ts1);
						}
					}
				}
			}
		}
		return 1;


	case subtitle_format_sbt:

		if(!subset->temp_i2)
		{
			if(str_scan(l, uni("%d:%d:%d"), &t_a1, &t_a2, &t_a3) == 3)
			{
				if(subset->temp_i1)
				{
					/* add line */
					sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
					subset->temp_i1 = 0;
				}

				/* convert to milliseconds */

				subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000);
				subset->temp_d2 = -100; /* temporary fix */;
				subset->temp_i1 = 1; /* start of text */
				subset->temp_i2 = 1; /* block start time reading till we find the end time */
				subset->temp_line[0] = 0; /* empty the line */
				return 1;
			}
		}

		if(subset->temp_i1){

			if(str_scan(l, uni("%d:%d:%d"), &t_a1, &t_a2, &t_a3) == 3)
			{
				subset->temp_d2 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000);
				subset->temp_i2 = 0;
			}else{

				if(str_len(l) > 0)
				{
					if(subset->temp_line[0])
						str_cat(subset->temp_line, uni("\n\r"));
					str_cat(subset->temp_line, l);
				}
			}
		}
		return 1;


	case subtitle_format_capi:

		if(str_scan(l, uni("%d:%d:%d:%d %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));

				if(str_scan(l, uni("%*[^ ] %*[^ ] %[^\n\0]"), ts1) == 1)
					str_cat(subset->temp_line, ts1);
			}
		}
		return 1;

	case subtitle_format_subv2:

		if(str_scan(l, uni("%d:%d:%d.%d,%d:%d:%d.%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));

				str_cpy(ts1, l);
				replace_str(ts1, uni("[br]"), uni("\n\r"));
				str_cat(subset->temp_line, ts1);
			}
		}
		return 1;

	case subtitle_format_sofni:

		if(str_scan(l, uni("%d:%d:%d.%d\\%d:%d:%d.%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));

				str_cpy(ts1, l);
				replace_str(ts1, uni("[br]"), uni("\n\r"));
				str_cat(subset->temp_line, ts1);
			}
		}
		return 1;

	case subtitle_format_wtt:

		if(str_scan(l, uni(" %*[^ ] %d:%d:%d:%d %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 33);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 33);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_fab:

		if(str_scan(l, uni("%d:%d:%d:%d %d:%d:%d:%d %*[^ ]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 33);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 33);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_ppx:

		if(str_scan(l, uni("%d:%d:%d:%d %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_udw:

		if(str_scan(l, uni(" %*[^ ] %d:%d:%d.%d %d:%d:%d.%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 33);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 33);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_cav:

		str_cpy(ts1, l);
		ts1[0] = uni(' ');

		if(str_scan(ts1, uni(" %*[^ ] %d:%d:%d:%d %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4) == 8)
		{
			if(subset->temp_i1)
			{
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 43);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 43);
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	case subtitle_format_ass:

		str_cpy(ts1, l);
		replace_2char_with_3(ts1, uni(','), uni(','), uni('0'), uni(','));
		

		if(str_scan(ts1, uni("Dialogue: %*d,%d:%d:%d.%d,%d:%d:%d.%d,%*[^,],%*[^,],%*d,%*d,%*d,%*[^,],%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('N'), uni('\n'), uni('\r'));
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('n'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_ssa:

		str_cpy(ts1, l);
		replace_2char_with_3(ts1, uni(','), uni(','), uni('0'), uni(','));
		
		if(str_scan(ts1, uni("%*[^=]=%*d,%d:%d:%d.%d,%d:%d:%d.%d,%*[^,],%*[^,],%*d,%*d,%*d,%*[^,],%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('N'), uni('\n'), uni('\r'));
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('n'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_xas:

		str_cpy(ts1, l);
		replace_str(ts1, uni(" = "), uni("="));
		replace_str(ts1, uni("= "), uni("="));
		replace_str(ts1, uni("= "), uni("="));
		replace_str(ts1, uni(">"), uni(" >"));
		replace_str(ts1, uni("<"), uni(" < "));
		

		if(str_scan(ts1, uni(" < p %*[^ ] begin=\"%d:%d:%d:%d\" end=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}
		if(str_scan(ts1, uni(" < p %*[^ ] end=\"%d:%d:%d:%d\" begin=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}
		if(str_scan(ts1, uni(" < p %*[^ ] begin=\"%d:%d:%d:%d\" %*[^ ] end=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}
		if(str_scan(ts1, uni(" < p end=\"%d:%d:%d:%d\" %*[^ ] begin=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}
		if(str_scan(ts1, uni(" < p begin=\"%d:%d:%d:%d\" end=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}
		if(str_scan(ts1, uni(" < p end=\"%d:%d:%d:%d\" begin=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}
		if(str_scan(ts1, uni(" < p begin=\"%d:%d:%d:%d\" %*[^ ] end=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}
		if(str_scan(ts1, uni(" < p end=\"%d:%d:%d:%d\" %*[^ ] begin=\"%d:%d:%d:%d\" >%[^<]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_xas_dec;}

pos_xas_dec:


		if(tbool)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_str(subset->temp_line, uni("&apos;"), uni("\'"));
			replace_str(subset->temp_line, uni("&quot;"), uni("\""));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_rt:

		str_cpy(ts1, l);
		replace_str(ts1, uni(" = "), uni("="));
		replace_str(ts1, uni("= "), uni("="));
		replace_str(ts1, uni("= "), uni("="));
		replace_str(ts1, uni("/>"), uni(" />"));
		replace_str(ts1, uni(">"), uni("> "));
		replace_str(ts1, uni("<"), uni(" < "));
		

		if(str_scan(ts1, uni(" < Time %*[^ ] begin=\"%d:%d:%d.%d\" end=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}
		if(str_scan(ts1, uni(" < Time %*[^ ] end=\"%d:%d:%d.%d\" begin=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}
		if(str_scan(ts1, uni(" < Time %*[^ ] begin=\"%d:%d:%d.%d\" %*[^ ] end=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}
		if(str_scan(ts1, uni(" < Time end=\"%d:%d:%d.%d\" %*[^ ] begin=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}
		if(str_scan(ts1, uni(" < Time begin=\"%d:%d:%d.%d\" end=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}
		if(str_scan(ts1, uni(" < Time end=\"%d:%d:%d.%d\" begin=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}
		if(str_scan(ts1, uni(" < Time begin=\"%d:%d:%d.%d\" %*[^ ] end=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}
		if(str_scan(ts1, uni(" < Time end=\"%d:%d:%d.%d\" %*[^ ] begin=\"%d:%d:%d.%d\" /> %*[^>]> %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9) {tbool = 1; goto pos_rt_dec;}

pos_rt_dec:


		if(tbool)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_str(subset->temp_line, uni("<br>"), uni("\n\r"));
			replace_str(subset->temp_line, uni("< br >"), uni("\n\r"));
			replace_str(subset->temp_line, uni("< br>"), uni("\n\r"));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_zeg:

		str_cpy(ts1, l);
		replace_2char_with_3(ts1, uni(','), uni(','), uni('0'), uni(','));
		

		if(str_scan(ts1, uni("%*[^ ] %*[^ ] %d:%d:%d.%d %d:%d:%d.%d %*[^ ] %*[^ ] %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('N'), uni('\n'), uni('\r'));
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('n'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_pimp:

		if(str_scan(l, uni("%d:%d:%d:%d %d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('N'), uni('\n'), uni('\r'));
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('n'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_vsf:

		if(str_scan(l, uni("%d:%d:%d,%d-%d:%d:%d,%d=%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + t_a4;
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + t_b4;
		
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('N'), uni('\n'), uni('\r'));
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('n'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_tts:

		if(str_scan(l, uni("%d:%d:%d.%d,%d:%d:%d.%d%*[^ ] %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('N'), uni('\n'), uni('\r'));
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('n'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_mac:

		if(str_scan(l, uni("%d:%d:%d:%d %d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_str(subset->temp_line, uni("<P>"), uni("\n\r")); 
			replace_str(subset->temp_line, uni("<p>"), uni("\n\r"));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_dvds:

		if(str_scan(l, uni("%d:%d:%d:%d %d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_2char_with_2(subset->temp_line, uni('/'), uni('/'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_dvdj:

		if(str_scan(l, uni("%d:%d:%d:%d&%d:%d:%d:%d#%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_1char_with_2(subset->temp_line, uni('<'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_son:

		str_cpy(ts1, l);
		replace_char(ts1, uni('\t'), uni(' '));
		
		if(str_scan(ts1, uni("%*[^ ] %d:%d:%d:%d %d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_sstso:

		str_cpy(ts1, l);
		replace_char(ts1, uni('\t'), uni(' '));
		
		if(str_scan(ts1, uni("%*[^ ] %d:%d:%d:%d %d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 33);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 33);

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_ssts:

		str_cpy(ts1, l);
		replace_char(ts1, uni('\t'), uni(' '));
		
		if(str_scan(ts1, uni("%*[^ ] %*[^ ] %d:%d:%d.%d %d:%d:%d.%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);

			replace_str(subset->temp_line, uni(" SubRip"), uni(""));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_js:

		str_cpy(ts1, l);
		replace_2char_with_3(ts1, uni(','), uni(','), uni('0'), uni(','));
		
		if(str_scan(ts1, uni("%d:%d:%d.%d %d:%d:%d.%d {%*[^}]} %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('N'), uni('\n'), uni('\r'));
			replace_2char_with_2(subset->temp_line, uni('\\'), uni('n'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_c32:

		if(str_scan(l, uni("%d:%d:%d:%d , %d:%d:%d:%d , %[^|]|%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line, ts1) == 10)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 10);
		
			terminate_last_spaces(subset->temp_line);
			terminate_last_spaces(ts1);

			if(ts1[0])
			{
				str_cat(subset->temp_line, uni("\n\r"));
				str_cat(subset->temp_line, ts1);
			}

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_psb:

		if(str_scan(l, uni("{%d:%d:%d}{%d:%d:%d}%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_b1, &t_b2, &t_b3, subset->temp_line) == 7)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000);
		
			replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_pjs:

		if(str_scan(l, uni(" %d, %d, \"%[^\"]\""), &t_a1, &t_a2, subset->temp_line) == 3)
		{
			subset->temp_d1 = ((double)t_a1 / subtitle_framerate) * 1000;
			subset->temp_d2 = ((double)t_a2 / subtitle_framerate) * 1000;
		
			replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;


	case subtitle_format_mpl1:

		str_cpy(ts1, l);
		replace_2char_with_3(ts1, uni(','), uni(','), uni('0'), uni(','));

		if(str_scan(ts1, uni("%d,%d,%*d,%[^\n\0]"), &t_a1, &t_a2, subset->temp_line) == 3)
		{
			subset->temp_d1 = ((double)t_a1 / subtitle_framerate) * 1000;
			subset->temp_d2 = ((double)t_a2 / subtitle_framerate) * 1000;
		
			replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_mpl2:

		if(str_scan(l, uni("[%d][%d]%[^\n\0]"), &t_a1, &t_a2, subset->temp_line) == 3)
		{
			subset->temp_d1 = t_a1 * 100;
			subset->temp_d2 = t_a2 * 100;
		
			replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_mdvd:

		if(str_scan(l, uni("{%d}{%d}%[^\n\0]"), &t_a1, &t_a2, subset->temp_line) == 3)
		{
			if(t_a1 == 1 && t_a2 == 1) /* set framerate */
			{
				float frate = 0.0f;
				if(str_scan(l, uni("{%*d}{%*d}%f"), &frate) == 1)
					subtitle_framerate = frate;
			}else{
				subset->temp_d1 = ((double)t_a1 / subtitle_framerate) * 1000;
				subset->temp_d2 = ((double)t_a2 / subtitle_framerate) * 1000;
			
				replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));

				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
			}
		}
		return 1;

	case subtitle_format_stl: /* spruce */


		if(str_scan(l, uni("%d:%d:%d:%d,%d:%d:%d:%d,%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, &t_b1, &t_b2, &t_b3, &t_b4, subset->temp_line) == 9)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 33);
			subset->temp_d2 = (((t_b1 * 3600) + (t_b2 * 60) + t_b3) * 1000) + (t_b4 * 33);
		
			replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}
		return 1;

	case subtitle_format_aqt:

		if(str_scan(l, uni("-->> %d"), &t_l1) == 1)
		{
			if(subset->temp_i1)
			{
				/* add line */
				subset->temp_d2 = ((double)t_l1 / subtitle_framerate) * 1000;

				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
				subset->temp_i2 = 0;
			}else{

				/* convert to milliseconds (from frame index) */

				subset->temp_d1 = ((double)t_l1 / subtitle_framerate) * 1000;
				subset->temp_i1 = 1; /* start of text */
				subset->temp_line[0] = 0; /* empty the line */
				subset->temp_i2 = 1;
			}

		}else if(subset->temp_i1){

			if(str_len(l) > 0)
			{
				if(subset->temp_i2 != 1)
					str_cat(subset->temp_line, uni("\n\r"));

				str_cat(subset->temp_line, l);
				subset->temp_i2++;
			}
			
		}
		return 1;


	case subtitle_format_dks:

		str_cpy(ts1, l);

		ts1len = (int)str_len(ts1);
		if(ts1[ts1len-1] == uni(']'))
		{
			ts1[ts1len]     = uni(' ');
			ts1[ts1len + 1] = uni('~');
			ts1[ts1len + 2] = 0;
		}
		
		if(str_scan(ts1, uni("[%d:%d:%d] %[^\n\0]"), &t_a1, &t_a2, &t_a3, subset->temp_line) == 4)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000);
			subset->temp_d2 = -100.0; /* will be fixed at the end */
		
			replace_str(subset->temp_line, uni("[br]"), uni("\n\r"));
			
			if(subset->temp_line[0] == uni('~'))
				subset->temp_line[0] = uni(' ');

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}

		return 1;

	case subtitle_format_ovr:
		
		if(str_scan(l, uni("%d:%d:%d:%d %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, subset->temp_line) == 5)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = -100.0; /* will be fixed at the end */
		
			replace_2char_with_2(subset->temp_line, uni('/'), uni('/'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		
		}else if(str_scan(l, uni("%d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4) == 4){
			
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = -100.0; /* will be fixed at the end */

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, uni(" "));
	
		}

		return 1;


	case subtitle_format_cpc:
		
		if(str_scan(l, uni("%d:%d:%d:%d_%*[^_]_%[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, subset->temp_line) == 5)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = -100.0; /* will be fixed at the end */
		
			replace_1char_with_2(subset->temp_line, uni('\\'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		
		}else if(str_scan(l, uni("%d:%d:%d:%d_%*[^_]_"), &t_a1, &t_a2, &t_a3, &t_a4) == 4){
			
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = -100.0; /* will be fixed at the end */

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, uni(" "));
	
		}

		return 1;


	case subtitle_format_subcr:
		
		if(str_scan(l, uni("%d:%d:%d.%d: %[^\n\0]"), &t_a1, &t_a2, &t_a3, &t_a4, subset->temp_line) == 5)
		{
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 100);
			subset->temp_d2 = -100.0; /* will be fixed at the end */
		
			replace_1char_with_2(subset->temp_line, uni('|'), uni('\n'), uni('\r'));

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		
		}else if(str_scan(l, uni("%d:%d:%d.%d:"), &t_a1, &t_a2, &t_a3, &t_a4) == 4){
			
			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 100);
			subset->temp_d2 = -100.0; /* will be fixed at the end */

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, uni(" "));
	
		}

		return 1;

	case subtitle_format_lrc:

		str_cpy(ts1, l);

		ts1len = (int)str_len(ts1);
		if(ts1[ts1len-1] == uni(']'))
		{
			ts1[ts1len]     = uni('~');
			ts1[ts1len + 1] = 0;
		}
		
		if(str_scan(ts1, uni("[%d:%d.%d]%[^\n\0]"), &t_a1, &t_a2, &t_a3, subset->temp_line) == 4)
		{
			subset->temp_d1 = (((t_a1 * 60) + t_a2) * 1000) + (t_a3 * 10);
			subset->temp_d2 = -100.0; /* will be fixed at the end */
		
			replace_str(subset->temp_line, uni("[br]"), uni("\n\r"));
			
			if(subset->temp_line[0] == uni('~'))
				subset->temp_line[0] = uni(' ');

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}

		return 1;

	case subtitle_format_vkt:

		str_cpy(ts1, l);
		replace_2char_with_3(ts1, uni(' '), uni('}'), uni('~'), uni('}'));

		if(str_scan(ts1, uni("{%d %[^}]}"), &t_a1, subset->temp_line) == 2)
		{
			subset->temp_d1 = t_a1 * 10;
			subset->temp_d2 = -100.0; /* will be fixed at the end */
		
			replace_str(subset->temp_line, uni("[br]"), uni("\n\r"));
			
			if(subset->temp_line[0] == uni('~'))
				subset->temp_line[0] = uni(' ');

			sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		}

		return 1;

	case subtitle_format_asc:

		if(str_scan(l, uni("*T %d:%d:%d:%d"), &t_a1, &t_a2, &t_a3, &t_a4) == 4)
		{
			if(subset->temp_i1)
			{
				terminate_last_newlines(subset->temp_line);
				/* add line */
				sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
				subset->temp_i1 = 0;
			}

			/* convert to milliseconds */

			subset->temp_d1 = (((t_a1 * 3600) + (t_a2 * 60) + t_a3) * 1000) + (t_a4 * 10);
			subset->temp_d2 = -100.0; /* will be fixed at the end */
			subset->temp_i1 = 1; /* start of text */
			subset->temp_line[0] = 0; /* empty the line */


		}else if(subset->temp_i1){

			if(l[0] != uni('*'))
			{
				if(subset->temp_line[0])
					str_cat(subset->temp_line, uni("\n\r"));

				str_cat(subset->temp_line, l);
			}
		}
		return 1;

	}

	return 0;
}

void read_text_subtitle_file(const string fpath, int subformat, struct subtitle_set* subset)
{
	FILE *fp;
	static letter tstr[1024];

	subset->temp_i1 = 0;
	subset->temp_i2 = 0;
	subset->temp_i3 = 0;

	fp = _wfopen(fpath, uni("r"));

	if(!fp)
	{
		wprintf(uni("file not found!\n"));
		return;
	}

	while(fgetws(tstr, sizeof(tstr)/sizeof(letter), fp))
	{
		int len = (int)str_len(tstr);
		if(len)
		{
			if(tstr[len - 1] == uni('\n') || tstr[len - 1] == uni('\r'))tstr[len - 1] = 0;
			len--;
			if(len)
			{
				if(tstr[len - 1] == uni('\n') || tstr[len - 1] == uni('\r'))tstr[len - 1] = 0;
				len--;
			}
			sub_list_readline(tstr, subformat, subset);
		}
	}

	sub_list_fix_endtimes(subset);

	if(subset->temp_i1 == 1)
	{
		/* add line */
		sub_list_add(subset, subset->temp_d1, subset->temp_d2, subset->temp_line);
		subset->temp_i1 = 0;
	}
	
	fclose(fp);
}

string* scan_file(const string fpath)
{
	FILE          *fp;
	static letter  tstr[1024];
	int            i = 0;
	string        *colstr;

	fp = _wfopen(fpath, uni("r"));

	if(!fp) return 0;

	colstr = (string*) malloc(100 * sizeof(string));

	for(i=0; i<100; i++)
		colstr[i] = 0;

	i = 0;

	while(fgetws(tstr, sizeof(tstr)/sizeof(letter), fp))
	{
		int len = (int)str_len(tstr);
		if(len)
		{
			if(i >= 99) break;
			if(tstr[len - 1] == uni('\n') || tstr[len - 1] == uni('\r'))tstr[len - 1] = 0;
			len--;
			if(len)
			{
				if(tstr[len - 1] == uni('\n') || tstr[len - 1] == uni('\r'))tstr[len - 1] = 0;
				len--;
			}

			colstr[i] = malloc((str_size(tstr) + 1) * sizeof(letter));
			str_cpy(colstr[i], tstr);
			i++;
		}
	}

	fclose(fp);
	return colstr;
}

void free_scan_file(string *c)
{
	int i;
	for(i=0; i<100; i++)
	{
		if(c[i]) free(c[i]);
	}
	free(c);
}

int scan_for_signature(const string s, string* ss)
{
	int i;
	static letter buf[1024];

	for(i=0; i<100; i++)
	{
		if(ss[i])
		{
			if(str_scan(ss[i], s, buf) == 1) return 1;
		}
	}
	return 0;
}

int scan_for_raw_signature(const string s, string* ss)
{
	int i, sl = (int)str_len(s);

	for(i=0; i<100; i++)
	{
		if(ss[i])
		{
			if(str_ncmp(ss[i], s, sl) == 0) return 1;
		}
	}
	return 0;
}

int scan_set_replace(string* ss, const string so, const string dst)
{
	int i;

	for(i=0; i<100; i++)
	{
		if(ss[i])
		{
			replace_str(ss[i], so, dst);
		}
	}
	return 0;
}



/*
 * returns -1 if the file type is invalid.
 */
int get_file_type(const string fname)
{
	letter ext[128];
	int    fname_len = (int)str_len(fname), i;
	string *ss;

	for(i=fname_len-1; i>=0; i--){
		if(fname[i] == uni('.'))break;
	}
	
	if(fname_len - i > 128)return -1;

	str_cpy(ext, fname + i + 1);

	if(!str_icmp(ext, uni("srt")))   return subtitle_format_srt;    
	if(!str_icmp(ext, uni("ass")))   return subtitle_format_ass;	  
	if(!str_icmp(ext, uni("aqt")))   return subtitle_format_aqt;	  
	if(!str_icmp(ext, uni("890")))   return subtitle_format_890;	  
	if(!str_icmp(ext, uni("stl")))   return subtitle_format_stl;	  
	if(!str_icmp(ext, uni("dks")))   return subtitle_format_dks;	  
	if(!str_icmp(ext, uni("asc")))   return subtitle_format_asc;	  
	if(!str_icmp(ext, uni("lrc")))   return subtitle_format_lrc;	  
	if(!str_icmp(ext, uni("vkt")))   return subtitle_format_vkt;	  
	if(!str_icmp(ext, uni("js")))    return subtitle_format_js;	  /* + jss */
	if(!str_icmp(ext, uni("zeg")))   return subtitle_format_zeg;	  
	if(!str_icmp(ext, uni("tts")))   return subtitle_format_tts;	  
	if(!str_icmp(ext, uni("vsf")))   return subtitle_format_vsf;	  
	if(!str_icmp(ext, uni("ssts")))  return subtitle_format_ssts;	  
	if(!str_icmp(ext, uni("ssa")))   return subtitle_format_ssa;	  
	if(!str_icmp(ext, uni("son")))   return subtitle_format_son;	  
	if(!str_icmp(ext, uni("sst")))   return subtitle_format_sstso;  /* sst */
	if(!str_icmp(ext, uni("psb")))   return subtitle_format_psb;	  
	if(!str_icmp(ext, uni("pjs")))   return subtitle_format_pjs;	  
	if(!str_icmp(ext, uni("ovr")))   return subtitle_format_ovr;	  
	if(!str_icmp(ext, uni("cin")))   return subtitle_format_capi;
	if(!str_icmp(ext, uni("xas")))   return subtitle_format_xas;	  
	if(!str_icmp(ext, uni("rt")))    return subtitle_format_rt;	  
	if(!str_icmp(ext, uni("s2k")))   return subtitle_format_s2k;	  
	if(!str_icmp(ext, uni("rtf")))   return subtitle_format_srtf;	  /* rtf */
	if(!str_icmp(ext, uni("sbt")))   return subtitle_format_sbt;	  
	if(!str_icmp(ext, uni("pan")))   return subtitle_format_pan;	  
	if(!str_icmp(ext, uni("scr")))   return subtitle_format_scr;	

	ss = scan_file(fname);

	if(!ss) return -1;

	if(!str_icmp(ext, uni("mpl")))
	{
		if(scan_for_signature(uni("[%*d][%*d]%[^\n\0]"), ss))
			return subtitle_format_mpl2;

		return subtitle_format_mpl1;
	}

	if(!str_icmp(ext, uni("txt")))
	{
		if(scan_for_raw_signature(uni("#INPOINT"), ss))
			return subtitle_format_pimp;

		if(scan_for_raw_signature(uni("BMPFILE:"), ss))
			return subtitle_format_ias;	

		if(scan_for_raw_signature(uni("{QTtext}"), ss))
			return subtitle_format_qtt;	

		if(scan_for_raw_signature(uni("#Ulead"), ss))
			return subtitle_format_udw;	

		if(scan_for_raw_signature(uni("~CPC"), ss))
			return subtitle_format_cpc;	

		if(scan_for_raw_signature(uni("*Timecode"), ss))
			return subtitle_format_capi;

		if(scan_for_raw_signature(uni("@@9"), ss))
			return subtitle_format_icg;	
		
		if(scan_for_signature(uni("%*d %*d:%*d:%*d:%*d %*d:%*d:%*d:%d"), ss) && scan_for_raw_signature(uni("0001 0"), ss))
			return subtitle_format_wtt;	 

		if(scan_for_signature(uni(" %*d %*d:%*d:%*d:%*d %*d:%*d:%*d:%d"), ss))
			return subtitle_format_cav;	 

		if(scan_for_signature(uni("%*d:%*d:%*d:%*d %*d:%*d:%*d:%*d %*d:%d"), ss))
			return subtitle_format_fab;	 

		if(scan_for_signature(uni("%*d:%*d:%*d.%*d:%[^\n\0]"), ss))
			return subtitle_format_subcr; 

		

		

		if(scan_for_signature(uni("%*d:%*d:%*d:%*d&%*d:%*d:%*d:%*d#%[^\n\0]"), ss))
			return subtitle_format_dvdj;


		scan_set_replace(ss, uni("\t"), uni("~~~"));

		if(scan_for_signature(uni("%*d:%*d:%*d:%*d~~~%*d:%*d:%*d:%*d~~~%[^\n\0]"), ss))
			return subtitle_format_mac;

		if(scan_for_signature(uni("%*d:%*d:%*d:%*d %*d:%*d:%*d:%*d %[^\n\0]"), ss))
			return subtitle_format_dvds;
		
		
		if(scan_for_signature(uni("%*d:%*d:%*d:%*d~~~%*d:%*d:%*d:%d"), ss))
			return subtitle_format_ppx;	

		return subtitle_format_c32;	
	}

	if(!str_icmp(ext, uni("sub")))
	{
		if(scan_for_signature(uni("%*d:%*d:%*d.%*d,%*d:%*d:%*d.%d"), ss))
			return subtitle_format_subv2;

		if(scan_for_signature(uni("[%*d:%*d:%d]"), ss))
			return subtitle_format_subv1;

		if(scan_for_signature(uni("%*d:%*d:%*d.%*d\\%*d:%*d:%*d.%d"), ss))
			return subtitle_format_sofni;

		if(scan_for_raw_signature(uni("[LIST]"), ss))
			return subtitle_format_psvd;

		if(scan_for_signature(uni("%*d %*d:%*d:%*d:%*d %*d:%*d:%*d:%d %*[^\n\0]"), ss) && scan_for_raw_signature(uni("0001\t0"), ss))
			return subtitle_format_dass;

		if(scan_for_signature(uni("%*d %*d:%*d:%*d:%*d %*d:%*d:%*d:%d %*[^\n\0]"), ss))
			return subtitle_format_sdvd;

		if(scan_for_signature(uni("{T %*d:%*d:%*d:%d"), ss))
			return subtitle_format_dsub;

		if(scan_for_signature(uni("%*[^ ] %*d.%*d %*[^~]~:\\%[^\n\0]"), ss))
			return subtitle_format_sson;

		return subtitle_format_mdvd;
	}

	free_scan_file(ss);
	return -1;
}

const string get_file_type_info(int id)
{
	switch(id)
	{
	case subtitle_format_srt  :  return uni("SubRip");
	case subtitle_format_ass  :  return uni("Advanced SubStation Alpha");
	case subtitle_format_aqt  :  return uni("AQTitle");
	case subtitle_format_890  :  return uni("Scantitle");
	case subtitle_format_stl  :  return uni("Spruce");
	case subtitle_format_dks  :  return uni("DKS");
	case subtitle_format_asc  :  return uni("Cheetah");
	case subtitle_format_lrc  :  return uni("Karaoke Lyrics LRC");
	case subtitle_format_vkt  :  return uni("Karaoke Lyrics VKT");
	case subtitle_format_mpl1 :  return uni("MPlayer1");
	case subtitle_format_mpl2 :  return uni("MPlayer2");
	case subtitle_format_js   :  return uni("JACOSub");
	case subtitle_format_zeg  :  return uni("ZeroG");
	case subtitle_format_pimp :  return uni("Pinnacle Impression");
	case subtitle_format_tts  :  return uni("Turbo Titler");
	case subtitle_format_vsf  :  return uni("ViPlay");
	case subtitle_format_mac  :  return uni("MAC DVD Studio Pro");
	case subtitle_format_dvds :  return uni("DVD Subtitle System");
	case subtitle_format_dvdj :  return uni("DVD Junior");
	case subtitle_format_ssts :  return uni("Stream SubText Script");
	case subtitle_format_ssa  :  return uni("SubStation Alpha");
	case subtitle_format_son  :  return uni("Spruce DVDMaestro");
	case subtitle_format_mdvd :  return uni("MicroDVD");
	case subtitle_format_sstso:  return uni("Sonic Scenarist");
	case subtitle_format_psb  :  return uni("Power DivX");
	case subtitle_format_pjs  :  return uni("Phoenix Japanimation Society");
	case subtitle_format_ovr  :  return uni("OVR Script");
	case subtitle_format_subcr:  return uni("SubCreator");
	case subtitle_format_wtt  :  return uni("Wincaps Text Timecoded");
	case subtitle_format_udw  :  return uni("ULead DVD Workshop");
	case subtitle_format_ppx  :  return uni("PowerPixel");
	case subtitle_format_fab  :  return uni("FAB");
	case subtitle_format_cav  :  return uni("Cavena");
	case subtitle_format_capi :  return uni("Captions inc.");
	case subtitle_format_subv2:  return uni("SubViewer 2");
	case subtitle_format_sofni:  return uni("Sofni");
	case subtitle_format_xas  :  return uni("Advanced Subtitles");
	case subtitle_format_cpc  :  return uni("CPC-600");
	case subtitle_format_rt   :  return uni("RealTime");
	case subtitle_format_s2k  :  return uni("Sasami Script");
	case subtitle_format_srtf :  return uni("Softitler RTF");
	case subtitle_format_sbt  :  return uni("SBT");
	case subtitle_format_qtt  :  return uni("QuickTime Text");
	case subtitle_format_pan  :  return uni("Panimator");
	case subtitle_format_scr  :  return uni("MacSUB");
	case subtitle_format_subv1:  return uni("SubViewer");
	case subtitle_format_dass :  return uni("DVD Architect Subtitle Script");
	case subtitle_format_sdvd :  return uni("Sonic DVD Creator");
	case subtitle_format_dsub :  return uni("DVD Subtitle");
	case subtitle_format_sson :  return uni("SubSonic");
	case subtitle_format_c32  :  return uni("Captions 32");
	case subtitle_format_ias  :  return uni("I-Author Script");
	case subtitle_format_psvd :  return uni("Philips SVCD Designer");
	case subtitle_format_icg  :  return uni("Inscriber CG");
	}
	return uni("[Invalid]");
}

int is_subtitle_file_extension(const string ext)
{
	if(!str_icmp(ext, uni("srt")))  return 1; 
	if(!str_icmp(ext, uni("ass")))  return 1; 
	if(!str_icmp(ext, uni("aqt")))  return 1; 
	if(!str_icmp(ext, uni("890")))  return 1; 
	if(!str_icmp(ext, uni("stl")))  return 1; 
	if(!str_icmp(ext, uni("dks")))  return 1; 
	if(!str_icmp(ext, uni("asc")))  return 1; 
	if(!str_icmp(ext, uni("lrc")))  return 1; 
	if(!str_icmp(ext, uni("vkt")))  return 1; 
	if(!str_icmp(ext, uni("js")))   return 1; 
	if(!str_icmp(ext, uni("zeg")))  return 1; 
	if(!str_icmp(ext, uni("tts")))  return 1; 
	if(!str_icmp(ext, uni("vsf")))  return 1; 
	if(!str_icmp(ext, uni("ssts"))) return 1; 
	if(!str_icmp(ext, uni("ssa")))  return 1; 
	if(!str_icmp(ext, uni("son")))  return 1; 
	if(!str_icmp(ext, uni("sst")))  return 1; 
	if(!str_icmp(ext, uni("psb")))  return 1; 
	if(!str_icmp(ext, uni("pjs")))  return 1; 
	if(!str_icmp(ext, uni("ovr")))  return 1; 
	if(!str_icmp(ext, uni("cin")))  return 1; 
	if(!str_icmp(ext, uni("xas")))  return 1; 
	if(!str_icmp(ext, uni("rt")))   return 1; 
	if(!str_icmp(ext, uni("s2k")))  return 1; 
	if(!str_icmp(ext, uni("rtf")))  return 1; 
	if(!str_icmp(ext, uni("sbt")))  return 1; 
	if(!str_icmp(ext, uni("pan")))  return 1; 
	if(!str_icmp(ext, uni("scr")))  return 1; 
	if(!str_icmp(ext, uni("txt")))  return 1; 
	if(!str_icmp(ext, uni("mpl")))  return 1; 
	if(!str_icmp(ext, uni("sub")))  return 1; 
	return 0;
}

const string get_subtitle_file_extension(int id)
{
	switch(id)
	{
	case 0:  return uni("srt");
	case 1:  return uni("ass");
	case 2:  return uni("aqt");
	case 3:  return uni("890");
	case 4:  return uni("stl");
	case 5:  return uni("dks");
	case 6:  return uni("asc");
	case 7:  return uni("lrc");
	case 8:  return uni("vkt");
	case 9:  return uni("js");
	case 10: return uni("zeg");
	case 11: return uni("tts");
	case 12: return uni("vsf");
	case 13: return uni("ssts");
	case 14: return uni("ssa");
	case 15: return uni("son");
	case 16: return uni("sst");
	case 17: return uni("psb");
	case 18: return uni("pjs");
	case 19: return uni("ovr");
	case 20: return uni("cin");
	case 21: return uni("xas");
	case 22: return uni("rt");
	case 23: return uni("s2k");
	case 24: return uni("rtf");
	case 25: return uni("sbt");
	case 26: return uni("pan");
	case 27: return uni("scr");
	case 28: return uni("txt");
	case 29: return uni("mpl");
	case 30: return uni("sub");
	}
	return 0;
}


/*


int wmain(int argc, wchar_t *argv[])
{
	int i, id;
	struct subtitle_set* subset;

	if(argc <= 1) return;

	id = get_file_type(argv[1]);

	wprintf(uni("File Type: %s\n\n"), get_file_type_info(id));

	subset = sub_list_initialize();

	read_text_subtitle_file(argv[1], id, subset);

	for(i=0; i<subset->line_count; i++)
	{
		if(subset->data[i].text)
			wprintf(uni("%d -> %d : %s\n"), (long)subset->data[i].start_time, (long)subset->data[i].end_time,subset->data[i].text);
	}

	sub_list_uninitialize(subset);
	
	
	getch(); 
}





*/
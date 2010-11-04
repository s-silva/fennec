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

#include "tagging.h"
#include "systems/system.h"
#include <ctype.h>
#include <string.h>

/* data ---------------------------------------------------------------------*/

const string tag_genres[] = 
{ uni("Blues"),             uni("Classic Rock"), uni("Country"),          uni("Dance"),            uni("Disco"),             uni("Funk"),            uni("Grunge")
, uni("Hip-Hop"),           uni("Jazz"),         uni("Metal"),            uni("New Age"),          uni("Oldies"),            uni("Other"),           uni("Pop"),          uni("R&B"),   uni("Rap")
, uni("Reggae"),            uni("Rock"),         uni("Techno"),           uni("Industrial"),       uni("Alternative"),       uni("Ska"),             uni("Death Metal")
, uni("Pranks"),            uni("Soundtrack"),   uni("Euro-Techno"),      uni("Ambient"),          uni("Trip-Hop"),          uni("Vocal")			 
, uni("Jazz+Funk"),         uni("Fusion"),       uni("Trance"),           uni("Classical"),        uni("Instrumental")
, uni("Acid"),              uni("House"),        uni("Game"),             uni("Sound Clip"),       uni("Gospel"),  uni("Noise"),  uni("AlternRock")
, uni("Bass"),              uni("Soul"),         uni("Punk"),             uni("Space"),            uni("Meditative"),        uni("Instrumental Pop")
, uni("Instrumental Rock"), uni("Ethnic"),       uni("Gothic"),           uni("Darkwave"),         uni("Techno-Industrial")
, uni("Electronic"),        uni("Pop-Folk"),     uni("Eurodance"),        uni("Dream"),            uni("Southern Rock"),     uni("Comedy")			 
, uni("Cult"),              uni("Gangsta"),      uni("Top 40"),           uni("Christian Rap"),    uni("Pop/Funk"),          uni("Jungle")			 
, uni("Native American"),   uni("Cabaret"),      uni("New Wave"),         uni("Psychadelic"),      uni("Rave"),              uni("Showtunes")	 
, uni("Trailer"),           uni("Lo-Fi"),        uni("Tribal"),           uni("Acid Punk"),        uni("Acid Jazz"),         uni("Polka"),           uni("Retro")
, uni("Musical"),           uni("Rock & Roll"),  uni("Hard Rock"),        uni("Folk"),             uni("Folk-Rock"),         uni("National Folk")
, uni("Swing"),             uni("Fast Fusion"),  uni("Bebob"),            uni("Latin"),            uni("Revival"),           uni("Celtic"),          uni("Bluegrass")
, uni("Avantgarde"),        uni("Gothic Rock"),  uni("Progressive Rock"), uni("Psychedelic Rock")
, uni("Symphonic Rock"),    uni("Slow Rock"),    uni("Big Band"),         uni("Chorus"),           uni("Easy Listening")
, uni("Acoustic"),          uni("Humour"),       uni("Speech"),           uni("Chanson"),          uni("Opera"),             uni("Chamber Music"),   uni("Sonata")
, uni("Symphony"),          uni("Booty Bass"),   uni("Primus"),           uni("Porn Groove"),      uni("Satire"),            uni("Slow Jam")
, uni("Club"),              uni("Tango"),        uni("Samba"),            uni("Folklore"),         uni("Ballad"),            uni("Power Ballad")
, uni("Rhythmic Soul"),     uni("Freestyle"),    uni("Duet"),             uni("Punk Rock"),        uni("Drum Solo"),         uni("A capella")
, uni("Euro-House"),        uni("Dance Hall") };


/* functions ----------------------------------------------------------------*/

/*
 * format string into proper english (for a title).
 */
int tags_str_propercase(string str, size_t mc)
{
	unsigned int i = 0;

	/* first, convert to lowercase */
	str_lower(str);

	while(str[i])
	{
		if(i >= mc)return (int)i;

		if(i == 0)
		{
			str[i] = (char)str_cupper(str[i]);
		}else{
			if(str[i - 1] == ' ' || (str_ispunct((unsigned char)str[i - 1]) && str[i - 1] != '\'' && str[i - 1] != '`'))str[i] = (char)str_cupper(str[i]);
		}
		i++;
	}
	return (int)i;
}

/*
 * find and count strings in a string for a given length.
 */
unsigned int tags_str_findcount(const char* str, const char* fstr, unsigned int len)
{
	unsigned int fstrz = (unsigned int)strlen(fstr);
	unsigned int i;
	unsigned int j = 0;
	unsigned int c = 0; /* count */

	for(i=0; i<len; i++)
	{
		if(str[i] == fstr[j])
		{
			j++;
			if(j >= fstrz)
			{
				c++;
				j = 0;
			}
		}else{
			j = 0;
		}
	}
	return c;
}

/*
 * find string in a string for a given length.
 */
int tags_str_find(const string str, const string fstr, unsigned int len)
{
	unsigned int fstrz = (unsigned int)str_len(fstr);
	unsigned int i;
	unsigned int j = 0;

	for(i=0; i<len; i++)
	{
		if(str[i] == fstr[j])
		{
			j++;
			if(j >= fstrz)return 1;
		}else{
			j = 0;
		}
	}
	return 0;
}

/*
 * replace data.
 */
void tags_str_replaceex(string str, const string dstr, const string sstr, unsigned int dlen /* dstr length */, unsigned int dslen /* sstr length */)
{
	unsigned int i = 0;                /* str (string to be replaced) pointer */
	unsigned int j = 0;                /* source pointer */
	int          m = 0;                /* pointer to '[' */
 	unsigned int slen  = dslen;        /* source length */
	unsigned int flen;                 /* full (str) length */
	int          copt;                 /* optional? */


	if(!dstr || !str_len(dstr))
	{
		/* search for a '[' */

		unsigned int strz = (unsigned int)str_len(str);

		while(str[i])
		{
			if(str[i] == uni('['))j = i;
			if(str[i] == uni(']'))
			{
				if(tags_str_find(str + j, sstr, i - j))
				{
					memcpy(str + j, str + i + 1, (strz - i - 1) * sizeof(letter));
					strz -= (i - j) + 1;
					i = j;
				}
			}
			i++;
		}

		if(strz)
			str[strz] = uni('\0');
		return;
	}

	while(str[i])
	{
		if(tolower(str[i]) == tolower(sstr[j]))
		{
			/* remove 'tolower' to enable case sensitivity */

			if(j >= slen - 1) /* string equal (-1 to exclude the last null) */
			{
				i -= slen - 1; /* -1 to remove last null */
				flen = (unsigned int)str_len(str);

				/* go backward searching for a '[' (but not a ']') */
				
				m = i;

				for(;;)
				{
					if(str[m] == uni(']')){copt = 0; goto point_rep;}

					if(str[m] == uni('['))
					{
						str_mcpy(str + m, str + m + 1, flen - 1);
						i--;
						flen--;
						goto point_closeoptional;
					}
					
					if(!m){copt = 0; goto point_rep;}

					m--;
				}

point_closeoptional:

				/* go forward searching for a ']' ('[' will terminate the loop */

				m = i + dslen; /* don't need to scan the source string */

				while(str[m])
				{
					if(str[m] == uni('[')){copt = 0; goto point_rep;}

					if(str[m] == uni(']'))
					{
						str_mcpy(str + m, str + m + 1, flen - 1);
						flen--;
						goto point_rep;
					}

					m++;
				}

point_rep: /* real replace */

				str_mmov(str + i + dlen, str + i + slen, flen - i - slen);
				str_mmov(str + i, dstr, dlen);

				j = 0;
				i += dlen;
				str[flen + (dlen - slen)] = 0; /* terminate by null */

				slen = dslen;
			}

			j++;

		}else{

			j = 0; /* wrong way, back to beginning */
		}

		i++;
	}
}

/*
 * replace strings.
 */
void tags_str_replace(string str, const string dstr, const string sstr)
{
	tags_str_replaceex(str, dstr, sstr, (unsigned int)str_len(dstr), (unsigned int)str_len(sstr));
}

/*
 * translate tags into a string.
 *
 * mem: memory with style definitions string copied at pointer 0.
 *
 * styles:
 *   <...-r> - required tag ("Unknown" if not available).
 *   <...-d> - default.
 *   <...-p> - proper case.
 *   <...-l> - lower case.
 *   <...-u> - upper case.
 */

int tags_translate(string mem, struct fennec_audiotag* tags, string fpath)
{
	letter        *mtxt, fstr[64], fname[v_sys_maxpath];
	unsigned int  i, fstrz, mz = 1024;
	struct fennec_audiotag_item* ct, ft_path, ft_name; /* current tag, fake tag */

	if(fpath)
	{
		ft_path.tdata = fpath;
		ft_path.tsize = (unsigned long)str_size(fpath);
	}else{
		ft_path.tdata = 0;
		ft_path.tsize = 0;
	}

	ft_path.tmode = tag_memmode_static;

	if(fpath)
	{
#	ifdef fennec_mode_multibyte
		_splitpath(fpath, 0, 0, fname, 0);
#	else
		_wsplitpath(fpath, 0, 0, fname, 0);
#	endif

		ft_name.tdata = fname;
		ft_name.tsize = (unsigned long)str_size(fname);

	}else{

		ft_name.tdata = 0;
		ft_name.tsize = 0;
	}

	ft_name.tmode = tag_memmode_static;

	mtxt = (string) sys_mem_alloc(mz * sizeof(letter));
	i    = 0;

point_selecttag:

	i++;

	switch(i)
	{
	case 1:
		ct = &tags->tag_title;
		str_cpy(fstr, uni("<title-x>"));
		goto point_applytag;
	case 2:
		ct = &tags->tag_album;
		str_cpy(fstr, uni("<album-x>"));
		goto point_applytag;
	case 3:
		ct = &tags->tag_artist;
		str_cpy(fstr, uni("<artist-x>"));
		goto point_applytag;
	case 4:
		ct = &tags->tag_origartist;
		str_cpy(fstr, uni("<artist.o-x>"));
		goto point_applytag;
	case 5:
		ct = &tags->tag_composer;
		str_cpy(fstr, uni("<composer-x>"));
		goto point_applytag;
	case 6:
		ct = &tags->tag_lyricist;
		str_cpy(fstr, uni("<lyricist-x>"));
		goto point_applytag;
	case 7:
		ct = &tags->tag_band;
		str_cpy(fstr, uni("<band-x>"));
		goto point_applytag;
	case 8:
		ct = &tags->tag_copyright;
		str_cpy(fstr, uni("<copyright-x>"));
		goto point_applytag;
	case 9:
		ct = &tags->tag_publish;
		str_cpy(fstr, uni("<publish-x>"));
		goto point_applytag;
	case 10:
		ct = &tags->tag_encodedby;
		str_cpy(fstr, uni("<encodedby-x>"));
		goto point_applytag;
	case 11:
		ct = &tags->tag_genre;
		str_cpy(fstr, uni("<genre-x>"));
		goto point_applytag;
	case 12:
		ct = &tags->tag_year;
		str_cpy(fstr, uni("<year-x>"));
		goto point_applytag;
	case 13:
		ct = &tags->tag_url;
		str_cpy(fstr, uni("<url-x>"));
		goto point_applytag;
	case 14:
		ct = &tags->tag_offiartisturl;
		str_cpy(fstr, uni("<url.artist.o-x>"));
		goto point_applytag;
	case 15:
		ct = &ft_path;
		str_cpy(fstr, uni("<file.path-x>"));
		goto point_applytag;
	case 16:
		ct = &ft_name;
		str_cpy(fstr, uni("<file.name-x>"));
		goto point_applytag;
	case 17:
		ct = &tags->tag_comments;
		str_cpy(fstr, uni("<comments-x>"));
		goto point_applytag;
	case 18:
		ct = &tags->tag_lyric;
		str_cpy(fstr, uni("<lyrics-x>"));
		goto point_applytag;
	case 19:
		ct = &tags->tag_bpm;
		str_cpy(fstr, uni("<bpm-x>"));
		goto point_applytag;
	case 20:
		ct = &tags->tag_tracknum;
		str_cpy(fstr, uni("<tracknum-x>"));
		goto point_applytag;
	}

	sys_mem_free(mtxt);
	return 1;

point_applytag:

	if(ct->tdata && ct->tsize)
	{
		if(mz < ct->tsize + 1)
		{
			if(ct->tsize > 0x100000 /* 1MB */)goto point_invalidtag;

			mz = ct->tsize + 10;
			mtxt = (string) sys_mem_realloc(mtxt, mz * sizeof(letter));
		}

		str_cpy(mtxt, ct->tdata);
		mtxt[ct->tsize] = 0; /* null terminated string */

		fstrz = (unsigned int) str_len(fstr);

		fstr[fstrz - 2] = uni('d');
		tags_str_replace(mem, mtxt, fstr);
	
		fstr[fstrz - 2] = uni('r');
		tags_str_replace(mem, mtxt, fstr);

		fstr[fstrz - 2] = uni('p');
		tags_str_propercase(mtxt, mz);
		tags_str_replace(mem, mtxt, fstr);
		
		fstr[fstrz - 2] = uni('u');
		str_upper(mtxt);
		tags_str_replace(mem, mtxt, fstr);
		
		fstr[fstrz - 2] = uni('l');
		str_lower(mtxt);
		tags_str_replace(mem, mtxt, fstr);

	}else{

point_invalidtag:

		fstrz = (unsigned int) str_len(fstr);

		fstr[fstrz - 2] = uni('r');
		tags_str_replace(mem, uni("Unknown"), fstr);

		fstr[fstrz - 2] = uni('d');
		tags_str_replace(mem, uni(""), fstr);
		
		fstr[fstrz - 2] = uni('p');
		tags_str_propercase(mtxt, mz);
		tags_str_replace(mem, uni(""), fstr);
		
		fstr[fstrz - 2] = uni('u');
		str_upper(mtxt);
		tags_str_replace(mem, uni(""), fstr);
		
		fstr[fstrz - 2] = uni('l');
		str_lower(mtxt);
		tags_str_replace(mem, uni(""), fstr);
	}

	goto point_selecttag;
}

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/
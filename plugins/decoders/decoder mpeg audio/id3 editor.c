/**----------------------------------------------------------------------------

 ID3 Tag Editing 1.0.
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

 ID 3 version '1', '2.3.0', '2.4.0' simple tag editing.

----------------------------------------------------------------------------**/


#include "id3 editor.h"

/* functions ----------------------------------------------------------------*/

/*
 * converts a 'double word' to an unsync 4 bytes
 * (big endian, msb cleared)
 */
void inline itoui(const unsigned int v, unsigned char *oint, const unsigned long iint)
{
	if(v >= 4)
	{
		oint[3] = (unsigned char) ( iint & 0x7f     );        /* mask 0111 1111b << 00d */
		oint[2] = (unsigned char) ((iint & 0x3f80   ) >> 7);  /* mask 0111 1111b << 07d */
		oint[1] = (unsigned char) ((iint & 0x1fc000 ) >> 14); /* mask 0111 1111b << 14d */ 
		oint[0] = (unsigned char) ((iint & 0xfe00000) >> 21); /* mask 0111 1111b << 21d */
	}else{
		oint[3] = (unsigned char) ( iint & 0xff     );       
		oint[2] = (unsigned char) ((iint & 0xff00   ) >> 8); 
		oint[1] = (unsigned char) ((iint & 0xff0000 ) >> 16); 
		oint[0] = (unsigned char) ((iint & 0xff00000) >> 24);
	}
}

/*
 * converts a unsync 4 bytes to a 'double word'
 */
unsigned long inline uitoi(const unsigned int v, const unsigned char *oint)
{
	register unsigned long k = 0;

	if(v >= 4)
	{
		k  = ((unsigned long)oint[3]);
		k |= ((unsigned long)oint[2]) << 7;
		k |= ((unsigned long)oint[1]) << 14;
		k |= ((unsigned long)oint[0]) << 21;

	}else{

		k  = ((unsigned long)oint[3]);
		k |= ((unsigned long)oint[2]) << 8;
		k |= ((unsigned long)oint[1]) << 16;
		k |= ((unsigned long)oint[0]) << 24;
	}

	return k;
}

/*
 * read the tag into memory.
 */
void* tag_fromfile(const string fpath)
{
	char *tmem;               /* tag memory */
	char  thead[10];          /* tag header */
	t_sys_file_handle ofile;  /* file handle */
	unsigned long tsize;      /* tag size */

	/* open the file */

	ofile = sys_file_openbuffering(fpath, v_sys_file_forread);
	
	if(ofile == v_error_sys_file_open)return 0;

	/* try to locate the tag at the beginning of the file */

	sys_file_read(ofile, thead, 10);

	if( (thead[0] != 'I' && thead[0] != 'i') ||
		(thead[1] != 'D' && thead[1] != 'd') ||
		(thead[2] != '3'))
	{
		/* tag not found, search for an appended tag */

		sys_file_seek(ofile, sys_file_getsize(ofile) - 10);

		sys_file_read(ofile, thead, 10);

		if( (thead[2] != 'I' && thead[2] != 'i') ||
			(thead[1] != 'D' && thead[1] != 'd') ||
			(thead[0] != '3'))
		{
			/* no tags found */
		
			sys_file_close(ofile);
			return 0;
		}else{

			/* an appending tag */

			tsize = uitoi(4, thead + 6) + 20; /* tag, header and footer */
			
			sys_file_seek(ofile, sys_file_getsize(ofile) - ((long)tsize));
		}

	}else{
		
		/* tag found */

		tsize = uitoi(4, thead + 6) + 10; /* tag and its header */
		sys_file_seek(ofile, 0);     /* rewind :) */
	}

	/* whatever kinda tag, now we just have to load it */

	tmem = (char*) sys_mem_alloc(tsize + 0x100); /* 256b padding */
	memset(tmem, 0, tsize + 0x100);

	sys_file_read(ofile, tmem, tsize);

	sys_file_close(ofile);
	return tmem;
}

/*
 * free memory allocated for the tag.
 */
int tag_free(void* tmem)
{
	if(tmem)sys_mem_free(tmem);
	return tmem ? 1 : 0;
}

int tag_clean_start(char *tdata, int spos, int msize)
{
	int     lastbom = 0, i, k;

	i = k = spos;

	for(i = spos;;i++)
	{
		if(tdata[i] == 0)
		{
			k++;
		}else if((tdata[i] == (char)0xff) || (tdata[i] == (char)0xfe)){
			if(lastbom)
			{
				k += 2;
				lastbom = 0;
			}else{
				lastbom = 1;
			}
		}else{
			break;
		}
	}
	return k;
}

/*
 * get a string frame from the tag.
 */
string tag_get_string(char* tmem, const char *tid)
{
	char          *fhead;       /* frame header */
	unsigned char *ufhead;      /* unsigned frame header */
	unsigned long tstart = 10;  /* tag start */
	unsigned long cp;           /* current pointer */
	unsigned long cfsize;       /* current frame size */
	unsigned long ftsize;       /* full tag size (with header) */
	char*         ntstr;        /* null terminated string */
	string        untstr;       /* unicode null terminated string */
	unsigned int  ucbom = 0, emode = 0 /* 0 - iso, 1 - utf16 ... as in id3 flags */ ;
	size_t        tmp;

	if(!tmem)return 0;

	ftsize = uitoi(4, tmem + 6) + 10;

	/* skip over the extended header */

	if(tmem[5] & 0x40 /* 0100 0000 */)
	{
		/* extended header found */
		tstart = uitoi(tmem[3], tmem + 10) + 10;
	}
	
	cp = tstart;
		
	while(1)
	{
		/* read a frame */

		fhead = tmem + cp;
		ufhead = tmem + cp;

		cfsize = uitoi(tmem[3], fhead + 4);

		if((tid[0] == fhead[0]) &&
		   (tid[1] == fhead[1]) &&
		   (tid[2] == fhead[2]) &&
		   (tid[3] == fhead[3]))
		{
			/* frame found */

			unsigned long ftstart = 10; /* frame text start */

			
	
			if(fhead[10] <= 0x20)
			{
				ftstart++;

				if(((tid[0] == 'U') && (tid[1] == 'S') && (tid[2] == 'L') && (tid[3] == 'T')) ||
					((tid[0] == 'C') && (tid[1] == 'O') && (tid[2] == 'M') && (tid[3] == 'M')) ||
					((tid[0] == 'U') && (tid[1] == 'S') && (tid[2] == 'E') && (tid[3] == 'R')))
				{
					ftstart += 3; /* language */
				}


				if(fhead[10]       == 0x0)
				{
					/* ANSI */
read_iso:;			
					for(;;)
					{
						if(fhead[ftstart] == 0)
						{
							ftstart++;
						}else{
							break;
						}	
					}

					if(cfsize <= (ftstart - 10))return 0;

					ntstr = (char*) sys_mem_alloc(cfsize - (ftstart - 10) + 1);
					
					memcpy(ntstr, fhead + ftstart, cfsize - (ftstart - 10));
					ntstr[cfsize - (ftstart - 10)] = 0;

					tmp = strlen(ntstr);

					untstr = sys_mem_alloc((tmp + 1) * sizeof(letter));

					MultiByteToWideChar(CP_ACP, 0, ntstr, -1, untstr, (int)tmp);

					untstr[tmp] = uni('\0');

					sys_mem_free(ntstr);

					return untstr;

				}else if(fhead[10] == 0x1){

					/* wrong endian conversion is not implemented */
					if(cfsize > (ftstart - 10))
					{
						size_t  msize = cfsize - (ftstart - 10) - sizeof(letter);

						ftstart = tag_clean_start(fhead, ftstart, (int)msize);
					

						untstr = sys_mem_alloc(msize + sizeof(letter));
					
						memcpy(untstr, fhead + ftstart, msize);

						untstr[msize / sizeof(letter)] = uni('\0');

						return untstr;
					}else{
						return 0;
					}

					/* UTF 16 */

				}else if(fhead[10] == 0x2){

					if(cfsize > (ftstart - 10))
					{
						size_t  msize = cfsize - (ftstart - 10);
						
						ftstart = tag_clean_start(fhead, ftstart, (int)msize);
					
						untstr = sys_mem_alloc(msize + sizeof(letter));
						
						memcpy(untstr, fhead + ftstart, msize);

						untstr[msize / sizeof(letter)] = uni('\0');

						return untstr;
					}else{
						return 0;
					}

					/* UTF 16 BE (without BOM)*/

				}else if(fhead[10] == 0x3){

					if(cfsize > (ftstart - 10))
					{
						size_t  msize = (cfsize - (ftstart - 10)) * sizeof(letter);

						ftstart = tag_clean_start(fhead, ftstart, (int)msize);
						
						untstr = sys_mem_alloc(msize + sizeof(letter));
						
						MultiByteToWideChar(CP_UTF8, 0, fhead + ftstart, -1, untstr, (int)(msize / sizeof(letter)));
						
						untstr[msize / sizeof(letter)] = uni('\0');

						return untstr;
					}else{
						return 0;
					}

					/* UTF 8 */

				}
			}

			/* if no encoding found, assume the encoding as iso */

			goto read_iso;
					
		}
		
		/* pass if not the last frame */

		cp += cfsize + 10;

		if(cp >= ftsize)return 0;
	}

	return 0;
}

/*
 * free memory allocated for a string.
 */
int tag_string_free(string tstr)
{
	if(tstr)sys_mem_free(tstr);
	tstr = 0;
	return 1;
}

/*
 * reset unsync numbers and set version.
 */
int tag_write_reset_4(char* tmem)
{
	char          *fhead;
	unsigned long tstart = 10;
	unsigned long vid;
	unsigned long tsize;
	unsigned long k, cp;

	vid = tmem[3];

	if(vid >= 4)return 0; /* version check */

	tsize = uitoi(4, tmem + 6) + 10;

	/* header */

	/*k = uitoi(vid, tmem + 6);
	itoui(tmem + 6, k);*/

	/* footer */

	if(tmem[5] & 0x10 /* 0100 0000 */)
	{
		/* footer found */

		k = uitoi(vid, tmem + tsize + 6);
		itoui(4, tmem + tsize + 6, k);
	}

	/* skip over extended header */

	if(tmem[5] & 0x40 /* 0100 0000 */)
	{
		/* extended header found */
		tstart = uitoi(vid, tmem + 10) + 10;
	}
	
	/* reset frame sizes */

	cp = tstart;

	tmem[3] = 4; /* set version - 4.00 */
	tmem[4] = 0;

	while(1)
	{
		fhead = tmem + cp;

		if(!fhead[0])return 1;

		k = uitoi(vid, fhead + 4);
		itoui(4, fhead + 4, k);

		cp += k + 10;

		if(cp >= tsize)return 1;
	}

	return 1;
}

/*
 * reset unsync numbers and set version.
 */
int tag_write_reset_3(char* tmem)
{
	char          *fhead;
	unsigned long tstart = 10;
	unsigned long vid;
	unsigned long tsize;
	unsigned long k, cp;

	vid = tmem[3];

	if(vid <= 3)return 0; /* version check */

	tsize = uitoi(4, tmem + 6) + 10;

	/* header */

	/*k = uitoi(vid, tmem + 6);
	itoui(tmem + 6, k);*/

	/* footer */

	if(tmem[5] & 0x10 /* 0100 0000 */)
	{
		/* footer found */

		k = uitoi(vid, tmem + tsize + 6);
		itoui(3, tmem + tsize + 6, k);
	}

	/* skip over extended header */

	if(tmem[5] & 0x40 /* 0100 0000 */)
	{
		/* extended header found */
		tstart = uitoi(vid, tmem + 10) + 10;
	}
	
	/* reset frame sizes */

	cp = tstart;

	tmem[3] = 4; /* set version - 4.00 */
	tmem[4] = 0;

	while(1)
	{
		fhead = tmem + cp;

		if(!fhead[0])return 1;

		k = uitoi(vid, fhead + 4);
		itoui(3, fhead + 4, k);

		cp += k + 10;

		if(cp >= tsize)return 1;
	}

	return 1;
}

/*
 * set frame (if not existing, create new).
 */
void* tag_write_setframe(char *tmem, const char *tid, const string dstr)
{
	char          *fhead;
	unsigned long  tstart = 10;
	unsigned long  tsize;
	unsigned long  cp, apoint = 0;
	unsigned long  fsize;
	char          *fstr;
	unsigned long  fstrlen;
	int            fe = 0;     /* footer exist */
	int            lset = 0;   /* set language */

	/* UTF8 Encoding <- (some players don't support this) */

#	ifdef write_mode_utf_8

	fstr = sys_mem_alloc(str_size(dstr) * 2 /* assumed: maximum size - 32bits */ );

	fstrlen = WideCharToMultiByte(CP_UTF8, 0, dstr, -1, fstr, str_size(dstr) * 2, 0, 0);

#	else

	fstrlen = (unsigned long)str_size(dstr) + sizeof(letter);
	
	fstr = sys_mem_alloc(fstrlen);

	memcpy(fstr + sizeof(letter), dstr, fstrlen - sizeof(letter));

	fstr[0] = 0xff;
	fstr[1] = 0xfe;

#	endif

	/* reset first */
	//if(tmem[3] < 4)tag_write_reset(tmem);

	/* set language? */

	if((tid[0] == 'U') && (tid[1] == 'S') &&(tid[2] == 'L') &&(tid[3] == 'T'))lset = 1; /* 'USLT' */
	if((tid[0] == 'C') && (tid[1] == 'O') &&(tid[2] == 'M') &&(tid[3] == 'M'))lset = 1; /* 'COMM' */
	if((tid[0] == 'U') && (tid[1] == 'S') &&(tid[2] == 'E') &&(tid[3] == 'R'))lset = 1; /* 'USER' */

	/* footer exist? */

	fe = (int)(tmem[5] & 0x10);

	/* get size */
	
	tsize = uitoi(4, tmem + 6) + 10;
	tsize += fe ? 10 : 0; /* add footer size */

	/* skip over extended header */

	if(tmem[5] & 0x40 /* 0100 0000 */)
	{
		/* extended header found */
		tstart = uitoi(tmem[3], tmem + 10) + 10;
	}
	
	/* reset frame sizes */

	cp = tstart;

	while(1)
	{
		fhead = tmem + cp;

		if(!fhead[0] /* padding area */)
		{
			apoint = cp;
			goto tag_write_new;
		}

		fsize = uitoi(tmem[3], fhead + 4);

		if((tid[0] == fhead[0]) && (tid[1] == fhead[1]) &&
		   (tid[2] == fhead[2]) && (tid[3] == fhead[3]))
		{
			long mdelta;

			/* set default flags */

			fhead[8] = 0;
			fhead[9] = 0;

			/* calculate memory delta */

			mdelta = (fstrlen + 1 /* encoding byte */ + (lset ? 4 : 0) /* language bytes */) - fsize;

			if(mdelta < 0)
			{
				/* memory shift */

				memcpy(fhead + fstrlen + 10 + 1 + (lset ? 4 : 0), fhead + fsize + 10, tsize - cp - 10 - fsize);
				
				/* set encoding */

#				ifdef write_mode_utf_8
					fhead[10] = 0x3; /* utf-8 */
#				else
					fhead[10] = 0x1; /* utf-16 */
#				endif

				/* copy string */

				memcpy(fhead + 11 + (lset ? 4 : 0) /* language bytes */, fstr, fstrlen);

				if(lset)
				{
					memcpy(fhead + 11, "\0\0\0\0", 4);
				}
			}

			if(mdelta == 0)
			{
				/* set encoding */

#				ifdef write_mode_utf_8
					fhead[10] = 0x3; /* utf-8 */
#				else
					fhead[10] = 0x1; /* utf-16-BE */
#				endif

				/* copy string */

				memcpy(fhead + 11 + (lset ? 4 : 0) /* language bytes */, fstr, fstrlen);

				if(lset)
				{
					memcpy(fhead + 11, "\0\0\0\0", 4);
				}
			}

			/* sys_mem_reallocate memory */

			if(mdelta && (tsize + mdelta) > 0)
			{
				tmem = (char*) sys_mem_realloc(tmem, tsize + mdelta + 0x100); /* plus 256b padding */
				fhead = tmem + cp;
				tsize += mdelta;
			}

			/* move memory */

			if(mdelta > 0)
			{
				/* memory shift */

				memcpy(fhead + fstrlen + 10 + 1 + (lset ? 4 : 0), fhead + fsize + 10, tsize - cp - 10 - fsize - mdelta);
					
				/* set encoding */

#				ifdef write_mode_utf_8
					fhead[10] = 0x3; /* utf-8 */
#				else
					fhead[10] = 0x1; /* utf-16-BE */
#				endif

				/* copy string */

				memcpy(fhead + 11 + (lset ? 4 : 0) /* language bytes */, fstr, fstrlen);
				if(lset)
				{
					memcpy(fhead + 11, "\0\0\0\0", 4);
				}
			}

			/* rewrite header, footer, frame sizes */

			itoui(tmem[3], fhead + 4, fstrlen + 1 + (lset ? 4 : 0) /* language bytes */);
			itoui(4, tmem + 6, tsize - 10 - (fe ? 10 : 0));
			if(fe)itoui(4, tmem + tsize - 20 + 6, tsize - 20);

			if(fstr != (const char*)dstr)sys_mem_free(fstr);

			return tmem;
		}

		cp += fsize + 10;
		
		if(cp >= tsize)
		{
			apoint = tsize;
			goto tag_write_new;
		}
	}

	if(fstr != (const char*)dstr)sys_mem_free(fstr);
	return tmem;

tag_write_new:;

	/* append tag at 'apoint' */

	/* first, allocate some more memory */

	tsize += fstrlen + 1 + 10 + (lset ? 4 : 0) /* language bytes */; /* one for encoding byte, ten for frame header */
	
	tmem   = (char*) sys_mem_realloc(tmem, tsize + 0x100); /* plus 256b padding */
	
	/* write frame header */

	fhead = tmem + apoint;

	fhead[0] = tid[0];
	fhead[1] = tid[1];
	fhead[2] = tid[2];
	fhead[3] = tid[3];

	fhead[8] = 0; /* flags */
	fhead[9] = 0; /* flags */
	
#ifdef write_mode_utf_8
	fhead[10] = 0x3; /* utf-8 */
#else
	fhead[10] = 0x1; /* utf-16-BE */
#endif

	if(lset)
	{
		fhead[11] = '\0';
		fhead[12] = '\0';
		fhead[13] = '\0';
		fhead[13] = '\0';
	}

	/* set size */

	itoui(tmem[3], fhead + 4, fstrlen + 1 + (lset ? 4 : 0) /* language bytes */);

	/* copy string */
	
	memcpy(fhead + 11 + (lset ? 4 : 0) /* language bytes */, fstr, fstrlen);

	/* rewrite header, footer, frame sizes */

	itoui(4, tmem + 6, tsize - 10 - (fe ? 10 : 0));
	if(fe)itoui(4, tmem + tsize - 20 + 6, tsize - 20);

	if(fstr != (const char*)dstr)sys_mem_free(fstr);
	return tmem;
}

/*
 * remove a frame.
 */
int tag_write_removeframe(char *tmem, const char *tid)
{
	char          *fhead;
	unsigned long  tstart = 10;
	unsigned long  tsize;
	unsigned long  cp, apoint = 0;
	unsigned long  fsize;
	int            fe = 0;     /* footer exist */

	/* footer exist? */

	fe = (int)(tmem[5] & 0x10);

	/* get size */
	
	tsize = uitoi(4, tmem + 6) + 10;
	tsize += fe ? 10 : 0; /* add footer size */

	/* skip over extended header */

	if(tmem[5] & 0x40 /* 0100 0000 */)
	{
		/* extended header found */
		tstart = uitoi(tmem[3], tmem + 10) + 10;
	}
	
	/* reset frame sizes */

	cp = tstart;

	while(1)
	{
		fhead = tmem + cp;

		if(!fhead[0] /* padding area */)return 1; /* no tag to delete */

		fsize = uitoi(tmem[3], fhead + 4) + 10;

		if((tid[0] == fhead[0]) && (tid[1] == fhead[1]) &&
		   (tid[2] == fhead[2]) && (tid[3] == fhead[3]))
		{

			memcpy(fhead, fhead + fsize, tsize - fsize);
			memset(tmem + (tsize - fsize), 0, fsize);
			tsize -= fsize;

			itoui(4, tmem + 6, tsize - 10 - (fe ? 10 : 0));
			if(fe)itoui(4, tmem + tsize - 20 + 6, tsize - 20);
			return 1;
		}

		cp += fsize;
		
		if(cp >= tsize)
		{
			apoint = tsize;
			return 1;
		}
	}

	return 1;
}

/*
 * calculate tag size (without padding)
 */
unsigned long tag_write_calcrealsize(char* tmem)
{
	char          *fhead;
	unsigned long  tstart = 10;
	unsigned long  tsize;
	unsigned long  cp;
	unsigned long  fsize;
	unsigned long  vid;

	vid = tmem[3];

	/* get size */
	
	tsize = uitoi(4, tmem + 6) + 10;

	/* skip over extended header */

	if(tmem[5] & 0x40 /* 0100 0000 */)
	{
		/* extended header found */
		tstart = uitoi(vid, tmem + 10) + 10;
	}
	
	/* reset frame sizes */

	cp = tstart;

	while(1)
	{
		fhead = tmem + cp;

		if(!fhead[0] /* padding area */)return cp;

		fsize = uitoi(vid, fhead + 4);

		cp += fsize + 10;
		
		if(cp >= tsize)return tsize;
	}
	return cp;
}

int tag_write_tofile(char *tmem, const string fpath)
{		
	char              *fmem;
	unsigned long      tmemsize;
	unsigned long      otsize;
	int                fsize;
	int                newsize, i;
	char               tmp[257];
	char               thead[10];     /* tag header */
	t_sys_file_handle  ofile;         /* file handle */

	/* open the file */

	ofile = CreateFile(fpath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	
	if(ofile == v_error_sys_file_open)return GetLastError();

	/* try to locate the tag at the beginning of the file */

	sys_file_read(ofile, thead, 10);

	if( (thead[0] != 'I' && thead[0] != 'i') ||
		(thead[1] != 'D' && thead[1] != 'd') ||
		(thead[2] != '3'))
	{
		/* tag not found */

		otsize = 0; /* no tag */
		goto write_file_tag;

	}else{

		otsize = uitoi(4, thead + 6) + 10 + ((thead[5] & 0x10) ? 10 : 0) /* footer */;
		
write_file_tag:

		fsize = sys_file_getsize(ofile);

		tmemsize = tag_write_calcrealsize(tmem) + 256; /* 256b padding */

		itoui(4, tmem + 6, tmemsize);

		fsize -= otsize;

		newsize = fsize + tmemsize;
		
		if(fsize > 0)
		{
			int fr, fb = 0;

			sys_file_seek(ofile, otsize);

			/* shift file data to place the tag */

			fmem = (char*)sys_mem_alloc(fsize);
			
			fb = 0;

			while(1)
			{
				fr = sys_file_read(ofile, fmem + fb, fsize - fb);
				if(!fr)break;
				fb += fr;

				if(fb >= fsize)break;
			}
			
			sys_file_seek(ofile, tmemsize + 10);

			fb = 0;

			while(1)
			{
				fr = sys_file_write(ofile, fmem + fb, fsize - fb);
				if(!fr)break;
				fb += fr;

				if(fb >= fsize)break;
			}

			sys_mem_free(fmem);

		}

		sys_file_seek(ofile, 0);

		sys_file_write(ofile, tmem, tmemsize - 256);

		for(i=0; i<=256; i++)
		{
			tmp[i] = 0;
		}

		sys_file_write(ofile, tmp, 256);
		sys_file_write(ofile, tmp, 10);

		sys_file_seek(ofile, newsize);
		sys_file_seteof(ofile);
	}

	sys_file_close(ofile);
	return 0;
}

/*
 * clear existing id3v2 tag.
 */
int tag_write_clear(const string fpath)
{		
	t_sys_file_handle  ofile;
	char              *fmem;
	unsigned long      otsize;
	int                fsize;
	int                newsize;
	char               thead[10];

	/* open the file */

	ofile = CreateFile(fpath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	
	if(ofile == v_error_sys_file_open)return 0;

	/* try to locate the tag at the beginning of the file */

	sys_file_read(ofile, thead, 10);

	if( (thead[0] == 'I' || thead[0] == 'i') &&
		(thead[1] == 'D' || thead[1] == 'd') &&
		(thead[2] == '3'))
	{
		int fr, fb = 0;

		otsize = uitoi(4, thead + 6) + 10 + ((thead[5] & 0x10) ? 10 : 0) /* footer */;
		
		fsize = sys_file_getsize(ofile);

		fsize -= otsize;
		newsize = fsize;

		if(newsize < 0)
		{
			sys_file_close(ofile);
			return 0;
		}
		
		sys_file_seek(ofile, otsize);

		/* shift file data to place the tag */

		fmem = (char*)sys_mem_alloc(fsize);
			
		fb = 0;

		while(1)
		{
			fr = sys_file_read(ofile, fmem + fb, fsize - fb);
			if(!fr)break;
			fb += fr;

			if(fb >= fsize)break;
		}
			
		sys_file_seek(ofile, 0);

		fb = 0;

		while(1)
		{
			fr = sys_file_write(ofile, fmem + fb, fsize - fb);
			if(!fr)break;
			fb += fr;

			if(fb >= fsize)break;
		}

		sys_mem_free(fmem);

		sys_file_seek(ofile, newsize);
		sys_file_seteof(ofile);
	}

	sys_file_close(ofile);
	return 0;
}

/*
 * create a dummy tag.
 */
void* tag_create(int tversion)
{
	char *tmem;

	tmem = (char*) sys_mem_alloc(128 + 0x100);
	memset(tmem, 0, 128 + 0x100);

	tmem[0] = 'I';
	tmem[1] = 'D';
	tmem[2] = '3';
	tmem[3] = (tversion == 4) ? 0x4 : 0x3;
	tmem[4] = 0; /* minor */
	tmem[5] = 0; /* flags */
	
	itoui(4, tmem + 6, 12);
	
	/* add a simple frame */

	tmem[10] = 'T';
	tmem[11] = 'X';
	tmem[12] = 'X';
	tmem[13] = 'X';
	tmem[14]  = 0;
	tmem[15]  = 0;
	tmem[16]  = 0;
	tmem[17]  = 2;
	tmem[18]  = 0; /* flags */
	tmem[19]  = 0; /* flags */

	tmem[20]  = 0; /* encoding */
	tmem[21]  = ' ';

	return tmem;
}

/*
 * local function.
 */
void tag_removelastspaces(char* str, int len)
{
	int i = len - 1;

	while(str[i] == ' ')
	{
		str[i] = 0;
		i--;
	}
	
}

/*
 * read id3v1 tag.
 */
int tag_old_read(const string fname, string tartist, string ttitle, string talbum, string tyear, string tcomments, int* tgenre, int* tnumber)
{
	struct id3v1_tag tagv1;
	t_sys_file_handle tag_file;

	
	char v1title  [32];
	char v1artist [32];
	char v1album  [32];
	char v1year   [32];
	char v1comment[32];

	/* id3 version 1 stuff */
	
	memset(tagv1.title  , 0, sizeof(tagv1.title  ));
	memset(tagv1.artist , 0, sizeof(tagv1.artist ));
	memset(tagv1.album  , 0, sizeof(tagv1.album  ));
	memset(tagv1.year   , 0, sizeof(tagv1.year   ));
	memset(tagv1.comment, 0, sizeof(tagv1.comment));

	tag_file = sys_file_openbuffering(fname, v_sys_file_forread);
	
	if(tag_file != v_error_sys_file_open)
	{
		/* read id3 version 1 tag */

		sys_file_seek(tag_file, sys_file_getsize(tag_file) - sizeof(tagv1));
		sys_file_read(tag_file, &tagv1, sizeof(tagv1));

		if( toupper(tagv1.tag[0]) != 'T' ||
			toupper(tagv1.tag[1]) != 'A' ||
			toupper(tagv1.tag[2]) != 'G')
		{

			if(tartist)  *tartist   = 0;
			if(ttitle)   *ttitle    = 0;
			if(talbum)   *talbum    = 0;
			if(tyear)    *tyear     = 0;
			if(tcomments)*tcomments = 0;
			if(tgenre)   *tgenre    = 0;
			if(tnumber)  *tnumber   = 0;

			sys_file_close(tag_file);

			return 0;

		}else{
			memcpy(v1title  ,tagv1.title  , 30);
			memcpy(v1artist ,tagv1.artist , 30);
			memcpy(v1album  ,tagv1.album  , 30);
			memcpy(v1year   ,tagv1.year   , 30);
			memcpy(v1comment,tagv1.comment, 30);

			v1title  [30] = 0;
			v1artist [30] = 0;
			v1album  [30] = 0;
			v1year   [ 4] = 0;
			v1comment[30] = 0;

			tag_removelastspaces(v1title  , 30);
			tag_removelastspaces(v1artist , 30);
			tag_removelastspaces(v1album  , 30);
			tag_removelastspaces(v1comment, 30);

	  		if(tartist)  
				MultiByteToWideChar(CP_ACP, 0, v1artist, -1, tartist, 30);

			if(ttitle)   
				MultiByteToWideChar(CP_ACP, 0, v1title, -1, ttitle, 30);

			if(talbum)  
				MultiByteToWideChar(CP_ACP, 0, v1album, -1, talbum, 30);

			if(tyear) 
				MultiByteToWideChar(CP_ACP, 0, v1year, -1, tyear, 30);

			if(tcomments)
				MultiByteToWideChar(CP_ACP, 0, v1comment, -1, tcomments, 30);

			if(tnumber)  {*tnumber = ((tagv1.comment[28] == 0) ? tagv1.tnumber : 0);}
			if(tgenre)   {*tgenre = tagv1.genre;}

		}
		sys_file_close(tag_file);
	}

	return 1;
}

/*
 * write id3v1 tag.
 */
int tag_old_write(const string fname, string twartist, string twtitle, string twalbum, string twyear, string twcomments, int tgenre, int tnumber)
{
	struct id3v1_tag    tagv1;
	t_sys_file_handle   tag_file;
	char                tartist  [32];
	char                ttitle   [32];
	char                talbum   [32];
	char                tyear    [32];
	char                tcomments[32];
	BOOL                uc;

	WideCharToMultiByte(CP_ACP, 0, twartist  , -1, tartist  , sizeof(tartist  ), " ", &uc);
	WideCharToMultiByte(CP_ACP, 0, twtitle   , -1, ttitle   , sizeof(ttitle   ), " ", &uc);
	WideCharToMultiByte(CP_ACP, 0, twalbum   , -1, talbum   , sizeof(talbum   ), " ", &uc);
	WideCharToMultiByte(CP_ACP, 0, twyear    , -1, tyear    , sizeof(tyear    ), " ", &uc);
	WideCharToMultiByte(CP_ACP, 0, twcomments, -1, tcomments, sizeof(tcomments), " ", &uc);

	/* id3 version 1 stuff */
	
	memset(tagv1.title  , 0, sizeof(tagv1.title  ));
	memset(tagv1.artist , 0, sizeof(tagv1.artist ));
	memset(tagv1.album  , 0, sizeof(tagv1.album  ));
	memset(tagv1.year   , 0, sizeof(tagv1.year   ));
	memset(tagv1.comment, 0, sizeof(tagv1.comment));

	tag_file = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	
	if(tag_file != v_error_sys_file_open)
	{
		/* read id3 version 1 tag */

		sys_file_seek(tag_file, sys_file_getsize(tag_file) - sizeof(tagv1));
		sys_file_read(tag_file, &tagv1, sizeof(tagv1));

		if( toupper(tagv1.tag[0]) != 'T' ||
			toupper(tagv1.tag[1]) != 'A' ||
			toupper(tagv1.tag[2]) != 'G')
		{

			/* no tags */

	  		if(tartist)  {strncpy(tagv1.artist , tartist  , 30); tartist[30]   = 0;}
			if(ttitle)   {strncpy(tagv1.title  , ttitle   , 30); ttitle[30]    = 0;}
			if(talbum)   {strncpy(tagv1.album  , talbum   , 30); talbum[30]    = 0;}
			if(tyear)    {memcpy (tagv1.year   , tyear    , 4); tyear[4]      = 0;}
			if(tcomments){strncpy(tagv1.comment, tcomments, 29); tcomments[30] = 0;}
			
			if(tnumber != -1) {tagv1.comment[28] = 0; tagv1.tnumber = tnumber;}
			if(tgenre  != -1) {tagv1.genre = tgenre;}

			sys_file_seek(tag_file, sys_file_getsize(tag_file));
			sys_file_write(tag_file, &tagv1, sizeof(tagv1));

		}else{

	  		if(tartist)  {strncpy(tagv1.artist , tartist  , 30); tartist[30]   = 0;}
			if(ttitle)   {strncpy(tagv1.title  , ttitle   , 30); ttitle[30]    = 0;}
			if(talbum)   {strncpy(tagv1.album  , talbum   , 30); talbum[30]    = 0;}
			if(tyear)    {memcpy (tagv1.year   , tyear    , 4); tyear[4]      = 0;}
			if(tcomments){strncpy(tagv1.comment, tcomments, 29); tcomments[30] = 0;}
			
			if(tnumber != -1) {tagv1.comment[28] = 0; tagv1.tnumber = tnumber;}
			if(tgenre  != -1) {tagv1.genre = tgenre;}

			sys_file_seek(tag_file, sys_file_getsize(tag_file) - sizeof(tagv1));
			sys_file_write(tag_file, &tagv1, sizeof(tagv1));

		}
		sys_file_close(tag_file);
	}

	return 1;
}

/*
 * clear id3v1 tag from a file.
 */
int tag_old_clear(const string fname)
{
	struct id3v1_tag tagv1;
	t_sys_file_handle tag_file;

	tag_file = CreateFile(fname, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	
	if(tag_file == v_error_sys_file_open)return 0;

	tagv1.tag[0] = 0;
	tagv1.tag[1] = 0;
	tagv1.tag[2] = 0;

	sys_file_seek(tag_file, sys_file_getsize(tag_file) - sizeof(tagv1));
	sys_file_read(tag_file, &tagv1, sizeof(tagv1));

	if( toupper(tagv1.tag[0]) == 'T' &&
		toupper(tagv1.tag[1]) == 'A' &&
		toupper(tagv1.tag[2]) == 'G')
	{
		sys_file_seek(tag_file, sys_file_getsize(tag_file) - sizeof(tagv1));
		sys_file_seteof(tag_file);
	}

	sys_file_close(tag_file);
	return 1;
}

/*-----------------------------------------------------------------------------
 may 2007.
-----------------------------------------------------------------------------*/

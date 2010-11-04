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

----------------------------------------------------------------------------**/

#include "../../../include/system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define inline __inline  /* assume that we have __inline :-) */

/* structs ------------------------------------------------------------------*/

#pragma pack(push, 1)

struct id3v1_tag
{
	char tag    [3];
	char title  [30];
	char artist [30];
	char album  [30];
	char year   [4];
	char comment[29];
	char tnumber;
	unsigned char genre;
};

#pragma pack(pop)

/* declarations -------------------------------------------------------------*/

void*  tag_fromfile(const string fpath);
int    tag_free(void* tmem);
string tag_get_string(char* tmem, const char *tid);
int    tag_string_free(string tstr);
int    tag_write_reset_4(char* tmem);
int    tag_write_reset_3(char* tmem);
void*  tag_write_setframe(char *tmem, const char *tid, const string fstr);
int    tag_write_removeframe(char *tmem, const char *tid);
int    tag_write_tofile(char *tmem, const string fpath);
int    tag_write_clear(const string fpath);
void*  tag_create(int tversion);

int tag_old_read(const string fname, string tartist, string ttitle, string talbum, string tyear, string tcomments, int* tgenre, int* tnumber);
int tag_old_write(const string fname, string tartist, string ttitle, string talbum, string tyear, string tcomments, int tgenre, int tnumber);
int tag_old_clear(const string fname);

/*-----------------------------------------------------------------------------
 may 2007.
-----------------------------------------------------------------------------*/

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

#include "fennec main.h"




string memory_strings = 0;
string strings_list[strings_count];




int lang_load(string fname)
{
	t_sys_file_handle fhandle;
	letter            fpath[v_sys_maxpath];
	size_t            fsize;
	int               i, j;


	for(i=0; i<strings_count; i++)
		strings_list[i] = uni("N/D");


	fennec_get_abs_path(fpath, fname);
	fhandle = sys_file_open(fpath, v_sys_file_forread);

	if(fhandle == v_error_sys_file_open)return -1;
	

	
	
	fsize = sys_file_getsize(fhandle);

	if(!memory_strings)
		memory_strings = (string)sys_mem_alloc(fsize + 16);
	else
		memory_strings = (string)sys_mem_realloc(memory_strings, fsize + 16);

	sys_file_read(fhandle, memory_strings, (unsigned long)fsize);

	sys_file_close(fhandle);


	strings_list[0] = memory_strings + 1;

	for(i=0, j=1; i<(int)(fsize/sizeof(letter)); i++)
	{
		if(j > strings_count)break;

		if(memory_strings[i] == uni('\n'))
		{
			if((memory_strings[i + 1] != uni('-')) &&
			   (memory_strings[i + 1] != uni(' ')) &&
			   (memory_strings[i + 1] != uni('\r')) )
			{
				strings_list[j] = memory_strings + i + 1;
				j++;
			}
			memory_strings[i]     = uni('\0');
			if(i > 0)
				memory_strings[i - 1] = uni('\0');
		}
	}

	return 0;
}


int lang_check(string fname, string tlang, string ttitle, string tauthor, string tcomments)
{
	t_sys_file_handle fhandle;
	size_t            fsize;
	int               i = 1, li;
	string            buffer;

	fhandle = sys_file_open(fname, v_sys_file_forread);

	if(fhandle == v_error_sys_file_open)return -1;

	fsize = sys_file_getsize(fhandle);

	buffer = (string)sys_mem_alloc(fsize);
	sys_file_read(fhandle, buffer, (unsigned long)fsize);
	
	sys_file_close(fhandle);

	/* language */

	li = 1;
	while(buffer[i] != uni('\r'))i++;
	str_ncpy(tlang, buffer + li, min(i - li, v_sys_maxpath));
	tlang[min(i - li, v_sys_maxpath)] = uni('\0');
	li = i + 2;
	i++;

	/* title */

	while(buffer[i] != uni('\r'))i++;
	str_ncpy(ttitle, buffer + li, min(i - li, v_sys_maxpath));
	ttitle[min(i - li, v_sys_maxpath)] = uni('\0');
	li = i + 2;
	i++;

	/* author */

	while(buffer[i] != uni('\r'))i++;
	str_ncpy(tauthor, buffer + li, min(i - li, v_sys_maxpath));
	tauthor[min(i - li, v_sys_maxpath)] = uni('\0');
	li = i + 2;
	i++;

	/* comments */

	while(buffer[i] != uni('\r'))i++;
	str_ncpy(tcomments, buffer + li, min(i - li, 4096));
	tcomments[min(i - li, 4096)] = uni('\0');


	sys_mem_free(buffer);
	return 0;
}


int lang_uninitialize(void)
{
	if(!memory_strings)return -1;

	sys_mem_free(memory_strings);
	memory_strings = 0;
	return 0;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


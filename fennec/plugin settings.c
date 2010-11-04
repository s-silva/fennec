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
#include "plugins.h"

unsigned char  plugin_settings_init = 0;
char          *plugin_settings_mem  = 0;
unsigned int   plugin_settings_size;

int plugin_settings_fillstruct(struct plugin_settings *o)
{
	o->plugin_settings_ret      = plugin_settings_ret;
	o->plugin_settings_get      = plugin_settings_get;
	o->plugin_settings_getnum   = plugin_settings_getnum;
	o->plugin_settings_set      = plugin_settings_set;
	o->plugin_settings_setfloat = plugin_settings_setfloat;
	o->plugin_settings_setnum   = plugin_settings_setnum;
	o->plugin_settings_remove   = plugin_settings_remove;
	
	return 0;
}

int plugin_settings_load(const string fname)
{
	t_sys_file_handle      fhandle;

	if(plugin_settings_init)return 1; /* warning: already initialized */

	fhandle = sys_file_openshare(fname, v_sys_file_forread);

	if(fhandle == v_error_sys_file_open)
	{
point_load_anyway:

		plugin_settings_mem = (char*) sys_mem_alloc(256);
		
		strcpy(plugin_settings_mem, "fennec player 1.0 (fennec 7.1) plug-in settings file\r\n");
		plugin_settings_size = (unsigned int)strlen(plugin_settings_mem);

		plugin_settings_save(fname);

		plugin_settings_init = 1;
		return 1;
	}

	plugin_settings_size = sys_file_getsize(fhandle);

	if(!plugin_settings_size)
	{
		sys_file_close(fhandle);
		goto point_load_anyway;
	}

	plugin_settings_mem = (char*) sys_mem_alloc(plugin_settings_size + 1);
	

	sys_file_read(fhandle, plugin_settings_mem, plugin_settings_size);
	
	plugin_settings_mem[plugin_settings_size] = 0;

	sys_file_close(fhandle);

	plugin_settings_init = 1;
	return 0;
}

int plugin_settings_save(const string fname)
{
	t_sys_file_handle     fhandle;

	if(!plugin_settings_mem)return -1;

	fhandle = sys_file_createforceshare(fname, v_sys_file_forwrite);
	if(fhandle == v_error_sys_file_create)return -1;

	sys_file_write(fhandle, plugin_settings_mem, plugin_settings_size);

	sys_file_close(fhandle);
	return 0;
}

int plugin_settings_unload(void)
{
	if(!plugin_settings_mem)return -1;
	if(!plugin_settings_init)return 1; /* warning: already uninitialized */

	sys_mem_free(plugin_settings_mem);
	plugin_settings_init = 0;
	return 0;
}

char* plugin_settings_ret(const char *splg, const char *sname, unsigned int *vzret /* value size */, unsigned int *azret /* absolute size */)
{
	char                  tmpbuf[512];
	register unsigned int i = 0, lpos = 0, blen;

	if(!plugin_settings_mem)return (char*)-1;
	if(!plugin_settings_init)return (char*)-1;

	strcpy(tmpbuf, splg);
	strcat(tmpbuf, ".");
	strcat(tmpbuf, sname);
	strcat(tmpbuf, " = ");

	blen = (unsigned int)strlen(tmpbuf);

	for(;;)
	{
		if(i + 1 >= plugin_settings_size)break;
		
		if(plugin_settings_mem[i] == '\r' && plugin_settings_mem[i + 1] == '\n')
		{
			if(lpos < i)
			{
				if(memicmp(plugin_settings_mem + lpos, tmpbuf, blen) == 0)
				{
					*azret = i - lpos;
					*vzret = (i - lpos) - blen;
					return plugin_settings_mem + lpos;
				}
			}

			lpos = i + 2; /* +2 for 'rn' */
		}

		i++;
	}

	return 0;
}

int plugin_settings_get(const char *splg, const char *sname, char *sval, unsigned int vlength)
{
	char                  tmpbuf[512];
	register unsigned int i = 0, lpos = 0, blen;

	if(!plugin_settings_mem)return -1;
	if(!plugin_settings_init)return -1;

	strcpy(tmpbuf, splg);
	strcat(tmpbuf, ".");
	strcat(tmpbuf, sname);
	strcat(tmpbuf, " = ");

	blen = (unsigned int)strlen(tmpbuf);

	for(;;)
	{
		if(i + 1 >= plugin_settings_size)break;
		
		if(plugin_settings_mem[i] == '\r' && plugin_settings_mem[i + 1] == '\n')
		{
			if(lpos < i)
			{
				if(memicmp(plugin_settings_mem + lpos, tmpbuf, blen) == 0)
				{
					memcpy(sval, plugin_settings_mem + lpos + blen, min(vlength, (i - lpos) - blen));
					sval[i - lpos - blen] = '\0';
					return 0;
				}
			}

			lpos = i + 2; /* +2 for 'rn' */
		}

		i++;
	}
	return -1;
}

int plugin_settings_getnum(const char *splg, const char *sname, int *idata, long *ldata, double *fdata)
{
	double dvalue;
	char   tbuffer[512];
	
	if(plugin_settings_get(splg, sname, tbuffer, sizeof(tbuffer)) < 0)return -1;
	
	dvalue = atof(tbuffer);

	if(idata)*idata = (int)dvalue;
	if(ldata)*ldata = (long)dvalue;
	if(fdata)*fdata = dvalue;
	return 0;
}

/*
 * add/modify setting.
 */
int plugin_settings_set(const char *splg, const char *sname, const char *sval)
{
	char   tmpbuf[1024];

	if(!plugin_settings_mem)return -1;
	if(!plugin_settings_init)return -1;

	strcpy(tmpbuf, splg);
	strcat(tmpbuf, ".");
	strcat(tmpbuf, sname);
	strcat(tmpbuf, " = ");
	strcat(tmpbuf, sval);
	strcat(tmpbuf, "\r\n");
	
	plugin_settings_remove(splg, sname);

	plugin_settings_size += (unsigned int)strlen(tmpbuf);

	plugin_settings_mem = (char*) sys_mem_realloc(plugin_settings_mem, plugin_settings_size + 1);

	strcat(plugin_settings_mem, tmpbuf);
	return 0;
}

int plugin_settings_setfloat(const char *splg, const char *sname, double sval)
{
	char tbuffer[512];

	gcvt(sval, 256, tbuffer);
	return plugin_settings_set(splg, sname, tbuffer);
}

int plugin_settings_setnum(const char *splg, const char *sname, int sval)
{
	char tbuffer[128];

	memset(tbuffer, 0, sizeof(tbuffer));

	itoa(sval, tbuffer, 10);
	return plugin_settings_set(splg, sname, tbuffer);
}

int plugin_settings_remove(const char *splg, const char *sname)
{
	char                  tmpbuf[512];
	register unsigned int i = 0, lpos = 0, blen;

	if(!plugin_settings_mem)return -1;
	if(!plugin_settings_init)return -1;

	strcpy(tmpbuf, splg);
	strcat(tmpbuf, ".");
	strcat(tmpbuf, sname);
	strcat(tmpbuf, " = ");

	blen = (unsigned int)strlen(tmpbuf);

	for(;;)
	{
		if(i + 1 >= plugin_settings_size)break;
		
		if(plugin_settings_mem[i] == '\r' && plugin_settings_mem[i + 1] == '\n')
		{
			if(lpos < i)
			{
				if(memicmp(plugin_settings_mem + lpos, tmpbuf, blen) == 0)
				{
					memcpy(plugin_settings_mem + lpos,
						   plugin_settings_mem + i + 2,
						   plugin_settings_size - i + 2);
					
					plugin_settings_size -=  i - lpos + 2;
					return 0;
				}
			}

			lpos = i + 2; /* +2 for 'rn' */
		}

		i++;
	}

	return -1;
}
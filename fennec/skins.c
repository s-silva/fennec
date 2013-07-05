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

#include "fennec_main.h"
#include "plugins.h"

/* prototypes ---------------------------------------------------------------*/

int local_tips_displayex(int x, int y, int deft, string ttxt);





/* data ---------------------------------------------------------------------*/

struct skin_data_return  current_skin_data;
int                      skin_initialized = 0;
t_sys_library_handle     current_skin_handle = 0;




/* code ---------------------------------------------------------------------*/


/*
 * initialize skins engine.
 */
int skins_initialize(const string fname, struct skin_data *sdata)
{
	t_sys_library_handle       fhandle;
	fennec_skins_initialize    hfunc;

	if(skin_initialized)skins_uninitialize();

	fhandle = sys_library_load(fname);

	if(fhandle == v_error_sys_library_load)return -1; /* error: invalid skin file "[fname]" */

	hfunc = (fennec_skins_initialize) sys_library_getaddress(fhandle, skin_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return -2; /* error: cannot load skin file "[fname]" */
	}

	sdata->showtip   = local_tips_displayex;
	sdata->showerror = 0;


	hfunc(sdata, &current_skin_data);

	current_skin_handle = fhandle;

	skin_initialized = 1;
	return 0;
}


/*
 * uninitialize skins engine.
 */
int skins_uninitialize()
{
	if(!skin_initialized)return 1; /* warning: already uninitialized */
	if(current_skin_handle)
	{
		if(current_skin_data.uninitialize)current_skin_data.uninitialize(0, 0);	
		sys_pass();
		sys_library_free(current_skin_handle);
	}
	
	skinproc = 0;
	current_skin_handle = 0;
	skin_initialized = 0;
	return 0;
}


/*
 * analyze skin file.
 */
int skins_checkfile(const string fname)
{
	t_sys_library_handle       fhandle;
	fennec_skins_initialize    hfunc;

	fhandle = sys_library_load(fname);

	if(fhandle == v_error_sys_library_load)return 0;

	hfunc = (fennec_skins_initialize) sys_library_getaddress(fhandle, skin_function_initialize);

	if(hfunc == v_error_sys_library_getaddress)
	{
		sys_library_free(fhandle);
		return 0;
	}

	sys_library_free(fhandle);
	return 1;
}


/*
 * send refresh message to skin.
 */
int skins_refresh(int rlevel)
{
	if(!skin_initialized)return -1;    /* error: not initialized */
	if(!current_skin_handle)return -2; /* error: not initialized */

	if(current_skin_data.refresh)current_skin_data.refresh(rlevel, 0);
	return 0;
}


/*
 * get data.
 */
struct skin_data_return *skins_getdata(void)
{
	if(!skin_initialized)return 0;
	if(!current_skin_handle)return 0;

	return &current_skin_data;
}


/*
 * get themes.
 * tid - theme id (start: 0,  till 'return 0').
 * return 1 = ok.
 * return 2 = selected theme.
 */
int skins_getthemes(int tid, string tname)
{
	if(!skin_initialized)return 0;
	if(!current_skin_handle)return 0;
	if(!current_skin_data.getthemes)return 0;

	return current_skin_data.getthemes(tid, tname, 0);
}


/*
 * set theme.
 */
int skins_settheme(int tid)
{
	if(!skin_initialized)return 0;
	if(!current_skin_handle)return 0;
	if(!current_skin_data.settheme)return 0;

	return current_skin_data.settheme(tid);
}


/*
 * set color.
 */
int skins_setcolor(int hue, int sat, int light)
{
	if(!skin_initialized)return 0;
	if(!current_skin_handle)return 0;
	if(!current_skin_data.setcolor)return 0;

	return current_skin_data.setcolor(hue, MAKELONG(sat, light));
}

int skins_function_getdata(int id, void *rdata, int dsize)
{
	if(!skin_initialized)return 0;
	if(!current_skin_handle)return 0;
	if(!current_skin_data.getdata)return 0;

	return current_skin_data.getdata(id, rdata, dsize);
}


int skins_function_subskins_get(subskins_callback   callfunc)
{
	if(!skin_initialized)return 0;
	if(!current_skin_handle)return 0;
	if(!current_skin_data.subs_get)return 0;

	return current_skin_data.subs_get(callfunc);
}

int skins_function_subskins_select(string fname)
{
	if(!skin_initialized)return 0;
	if(!current_skin_handle)return 0;
	if(!current_skin_data.subs_select)return 0;

	return current_skin_data.subs_select(fname);
}



/* local - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

int local_tips_displayex(int x, int y, int deft, string ttxt)
{
	if(ttxt)
		return tips_display(x, y, ttxt);
	else
		return tips_display(x, y, text(deft));
}

/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

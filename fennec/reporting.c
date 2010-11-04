/**----------------------------------------------------------------------------

 Fennec 7.1.2 Player 1.2
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




/* defines ------------------------------------------------------------------*/

#define    reporting_window_name  uni("fennec debug - reporting")




/* data ---------------------------------------------------------------------*/

int   reporting_initialized = 0;
HWND  reporting_window = 0;




/* code ---------------------------------------------------------------------*/


int reporting_start(void)
{
	reporting_initialized = 1;

	CreateMutex(0, 0, reporting_window_name);
		
	if(GetLastError() == ERROR_ALREADY_EXISTS || GetLastError() == ERROR_ACCESS_DENIED)
	{
		/* the debug-reporter is initialized */

		reporting_window = FindWindow(0, reporting_window_name);

		report("reporting system has initialized", rt_info);
	}
	return 0;
}


int reporting_end(void)
{
	report("terminating reporting system.", rt_info);
	reporting_initialized = 0;
	return 0;
}


void report(char *msg, int etype, ...)
{
	char            mbuffer[4096];
	va_list         vlist;
	COPYDATASTRUCT  cd;

	if(!reporting_initialized) return;
	if(!reporting_window) return;
	if(!IsWindow(reporting_window))
	{
		reporting_window = 0;
		return;
	}

	va_start(vlist, etype);

	vsprintf(mbuffer, msg, vlist);

	cd.cbData = sizeof(mbuffer);
	cd.dwData = etype;
	cd.lpData = mbuffer;

	SendMessage(reporting_window, WM_COPYDATA, 0 /* ANSI */, (LPARAM)&cd);

	va_end(vlist);
}

void reportu(string msg, int etype, ...)
{
	letter          mbuffer[4096];
	va_list         vlist;
	COPYDATASTRUCT  cd;

	if(!reporting_initialized) return;
	if(!reporting_window) return;
	if(!IsWindow(reporting_window))
	{
		reporting_window = 0;
		return;
	}

	va_start(vlist, etype);

	vswprintf(mbuffer, sizeof(mbuffer) / sizeof(letter), msg, vlist);

	cd.cbData = sizeof(mbuffer);
	cd.dwData = etype;
	cd.lpData = mbuffer;

	SendMessage(reporting_window, WM_COPYDATA, sizeof(letter) /* Unicode */, (LPARAM)&cd);

	va_end(vlist);
}

void reportx(char *msg, int etype, ...)
{
	letter          mbuffer[4096];
	letter          msgbuffer[256];
	va_list         vlist;
	COPYDATASTRUCT  cd;

	if(!reporting_initialized) return;
	if(!reporting_window) return;
	if(!IsWindow(reporting_window))
	{
		reporting_window = 0;
		return;
	}

	va_start(vlist, etype);

	MultiByteToWideChar(CP_ACP, 0, msg, -1, msgbuffer, sizeof(msgbuffer) / sizeof(letter));
	vswprintf(mbuffer, sizeof(mbuffer) / sizeof(letter), msgbuffer, vlist);

	cd.cbData = sizeof(mbuffer);
	cd.dwData = etype;
	cd.lpData = mbuffer;

	SendMessage(reporting_window, WM_COPYDATA, sizeof(letter) /* Unicode */, (LPARAM)&cd);

	va_end(vlist);
}




/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


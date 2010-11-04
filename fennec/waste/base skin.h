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


extern unsigned char    GraphicOutput;   /* output graphics?, turned off when minimized etc. */
//extern HINSTANCE        instance_fennec; /* main module handle */
//extern HWND             window_main;     /* main window handle */
extern HWND             Window_Playlist; /* playlist window handle */
//extern HDC              window_main_dc;  /* main window dc handle */
//extern unsigned long    Fennec_Build_ID; /* build id, variable in debug */
//extern unsigned long    SelectedPlayer;  /* selected player */
//extern unsigned long	LastPlayer;      /* last player selected */
extern int              Coord_DisplayX;  /* coordinates to put default display screen */
extern int              Coord_DisplayY;
extern int              Coord_DisplayW;
extern int              Coord_DisplayH;  
extern HDC              ActionDisplay;   /* actions dc handle*/
//extern int              debug_peakenable;
//extern HMENU            menu_main;
extern int              Display_EqFlush;






/*-----------------------------------------------------------------------------
 fennec.
-----------------------------------------------------------------------------*/

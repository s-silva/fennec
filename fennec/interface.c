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

 this file holds the functions which are called by external or internal
 (by other interfaces i.e. audio) interfaces to global fennec (G)UI.

 ----------------------------------------------------------------------------**/

#include "fennec main.h"
#include "fennec audio.h"


/* code ---------------------------------------------------------------------*/


/*
 * refresh (G)UI.
 * flevel - force level.
 */
int fennec_refresh(int flevel)
{
	switch(flevel)
	{
	case fennec_v_refresh_force_full:
	case fennec_v_refresh_force_high:
	case fennec_v_refresh_force_less:
		if(!settings.skins.selected[0])
		{
			/* no base skin */
		}else{
			skins_refresh(flevel);
		}

	case fennec_v_refresh_force_not:
		if(!settings.skins.selected[0])
		{
			/* no base skin */
		}else{
			skins_refresh(flevel);
		}
		
		visualizations_refresh(flevel);
		main_refresh();
		break;
	}
	return 1;
}


/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/

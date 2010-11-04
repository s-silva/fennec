/**----------------------------------------------------------------------------

 Fennec DSP Plug-in 1.0 (Extra Bass).
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

#include "../../../include/fennec.h"
#include "../../../include/system.h"
#include <math.h>

#define effect_set_mode_low  0
#define effect_set_mode_high 1

extern double setting_highpass  ;
extern double setting_lowpass   ;
extern double setting_wet       ;
extern double setting_dry       ;
extern double setting_diffusion ;
extern double setting_delay     ;
extern double setting_phase     ;


void  effect_initialize(void);
void  effect_set(unsigned long frequency, unsigned long channels, int mode);
fennec_sample effect_process(fennec_sample v, unsigned long frequency, unsigned long channels);




/*-----------------------------------------------------------------------------
 eof.
-----------------------------------------------------------------------------*/


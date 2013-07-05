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
#include "keyboard.h"

#ifdef system_microsoft_windows

unsigned short fennec_convertkey(void* ikey)
{
	unsigned short ukey = *(unsigned short*)ikey;

	switch(ukey)
	{
	case VK_F1:  return fennec_key_function1;
	case VK_F2:  return fennec_key_function2;
	case VK_F3:  return fennec_key_function3;
	case VK_F4:  return fennec_key_function4;
	case VK_F5:  return fennec_key_function5;
	case VK_F6:  return fennec_key_function6;
	case VK_F7:  return fennec_key_function7;
	case VK_F8:  return fennec_key_function8;
	case VK_F9:  return fennec_key_function9;
	case VK_F10: return fennec_key_function10;
	case VK_F11: return fennec_key_function11;
	case VK_F12: return fennec_key_function12;
	case VK_F13: return fennec_key_function13;
	case VK_F14: return fennec_key_function14;
	case VK_F15: return fennec_key_function15;
	
	case 'A': return fennec_key_a;
	case 'B': return fennec_key_b;
	case 'C': return fennec_key_c;
	case 'D': return fennec_key_d;
	case 'E': return fennec_key_e;
	case 'F': return fennec_key_f;
	case 'G': return fennec_key_g;
	case 'H': return fennec_key_h;
	case 'I': return fennec_key_i;
	case 'J': return fennec_key_j;
	case 'K': return fennec_key_k;
	case 'L': return fennec_key_l;
	case 'M': return fennec_key_m;
	case 'N': return fennec_key_n;
	case 'O': return fennec_key_o;
	case 'P': return fennec_key_p;
	case 'Q': return fennec_key_q;
	case 'R': return fennec_key_r;
	case 'S': return fennec_key_s;
	case 'T': return fennec_key_t;
	case 'U': return fennec_key_u;
	case 'V': return fennec_key_v;
	case 'W': return fennec_key_w;
	case 'X': return fennec_key_x;
	case 'Y': return fennec_key_y;
	case 'Z': return fennec_key_z;

	case '0': return fennec_key_0;
	case '1': return fennec_key_1;
	case '2': return fennec_key_2;
	case '3': return fennec_key_3;
	case '4': return fennec_key_4;
	case '5': return fennec_key_5;
	case '6': return fennec_key_6;
	case '7': return fennec_key_7;
	case '8': return fennec_key_8;
	case '9': return fennec_key_9;

	case VK_RETURN: return fennec_key_return;
	case VK_ESCAPE: return fennec_key_escape;
	case VK_TAB:    return fennec_key_tab;
	case VK_OEM_3:  return fennec_key_graveaccent;
	case VK_OEM_MINUS:   return fennec_key_minus;
	case VK_OEM_PLUS:    return fennec_key_equals;
	/*case '/':       return fennec_key_forwardslash;
	case '\\':      return fennec_key_backslash;*/
	case VK_OEM_4:  return fennec_key_squarebracket_open;
	case VK_OEM_5:  return fennec_key_squarebracket_close;
	case VK_OEM_1 : return fennec_key_semicolon;
	case VK_OEM_7:  return fennec_key_accent;
	case VK_OEM_COMMA:   return fennec_key_comma;
	case VK_OEM_PERIOD:  return fennec_key_period;
	case VK_SPACE:       return fennec_key_space;

	case VK_SNAPSHOT: return fennec_key_printscreen;
	case VK_SCROLL:   return fennec_key_scrolllock;
	case VK_PAUSE:    return fennec_key_pause;
	case VK_INSERT:   return fennec_key_insert;
	case VK_HOME:     return fennec_key_home;
	case VK_END:      return fennec_key_end;
	case VK_PRIOR:    return fennec_key_pageup;
	case VK_NEXT:     return fennec_key_pagedown;
	case VK_DELETE:   return fennec_key_delete;
	case VK_BACK:     return fennec_key_backspace;
	case VK_UP:       return fennec_key_up;
	case VK_DOWN:     return fennec_key_down;
	case VK_LEFT:     return fennec_key_left;
	case VK_RIGHT:    return fennec_key_right;

	case VK_NUMPAD0:  return fennec_key_num_0;
	case VK_NUMPAD1:  return fennec_key_num_1;
	case VK_NUMPAD2:  return fennec_key_num_2;
	case VK_NUMPAD3:  return fennec_key_num_3;
	case VK_NUMPAD4:  return fennec_key_num_4;
	case VK_NUMPAD5:  return fennec_key_num_5;
	case VK_NUMPAD6:  return fennec_key_num_6;
	case VK_NUMPAD7:  return fennec_key_num_7;
	case VK_NUMPAD8:  return fennec_key_num_8;
	case VK_NUMPAD9:  return fennec_key_num_9;
	case VK_NUMLOCK:  return fennec_key_num_numlock;
	case VK_DIVIDE:   return fennec_key_num_divide;
	case VK_MULTIPLY: return fennec_key_num_multiply;
	case VK_SUBTRACT: return fennec_key_num_minus;
	case VK_ADD:      return fennec_key_num_plus;

	default: return 0;
	}
}

long fennec_convertkeyrev(unsigned short fkey, long *mkey)
{
	unsigned short rkey = fkey & 0xff;

	*mkey = 0;

	if(fkey & fennec_key_shift)      *mkey |= MOD_SHIFT;
	if(fkey & fennec_key_alternative)*mkey |= MOD_ALT;
	if(fkey & fennec_key_control)    *mkey |= MOD_CONTROL;

	switch(rkey)
	{
	case fennec_key_function1 : return VK_F1 ; 
	case fennec_key_function2 : return VK_F2 ; 
	case fennec_key_function3 : return VK_F3 ; 
	case fennec_key_function4 : return VK_F4 ; 
	case fennec_key_function5 : return VK_F5 ; 
	case fennec_key_function6 : return VK_F6 ; 
	case fennec_key_function7 : return VK_F7 ; 
	case fennec_key_function8 : return VK_F8 ; 
	case fennec_key_function9 : return VK_F9 ; 
	case fennec_key_function10: return VK_F10; 
	case fennec_key_function11: return VK_F11; 
	case fennec_key_function12: return VK_F12; 
	case fennec_key_function13: return VK_F13; 
	case fennec_key_function14: return VK_F14; 
	case fennec_key_function15: return VK_F15; 
	
	case fennec_key_a: return 'A';
	case fennec_key_b: return 'B';
	case fennec_key_c: return 'C';
	case fennec_key_d: return 'D';
	case fennec_key_e: return 'E';
	case fennec_key_f: return 'F';
	case fennec_key_g: return 'G';
	case fennec_key_h: return 'H';
	case fennec_key_i: return 'I';
	case fennec_key_j: return 'J';
	case fennec_key_k: return 'K';
	case fennec_key_l: return 'L';
	case fennec_key_m: return 'M';
	case fennec_key_n: return 'N';
	case fennec_key_o: return 'O';
	case fennec_key_p: return 'P';
	case fennec_key_q: return 'Q';
	case fennec_key_r: return 'R';
	case fennec_key_s: return 'S';
	case fennec_key_t: return 'T';
	case fennec_key_u: return 'U';
	case fennec_key_v: return 'V';
	case fennec_key_w: return 'W';
	case fennec_key_x: return 'X';
	case fennec_key_y: return 'Y';
	case fennec_key_z: return 'Z';

	case fennec_key_0: return '0';
	case fennec_key_1: return '1';
	case fennec_key_2: return '2';
	case fennec_key_3: return '3';
	case fennec_key_4: return '4';
	case fennec_key_5: return '5';
	case fennec_key_6: return '6';
	case fennec_key_7: return '7';
	case fennec_key_8: return '8';
	case fennec_key_9: return '9';

	case fennec_key_return             : return VK_RETURN    ;
	case fennec_key_escape             : return VK_ESCAPE    ;
	case fennec_key_tab                : return VK_TAB       ;
	case fennec_key_graveaccent        : return VK_OEM_3     ;
	case fennec_key_minus              : return VK_OEM_MINUS ;
	case fennec_key_equals             : return VK_OEM_PLUS  ;
  //case fennec_key_forwardslash       : return '/'          ;
  //case fennec_key_backslash          : return '\\'         ;
	case fennec_key_squarebracket_open : return VK_OEM_4     ;
	case fennec_key_squarebracket_close: return VK_OEM_5     ;
	case fennec_key_semicolon          : return VK_OEM_1     ;
	case fennec_key_accent             : return VK_OEM_7     ;
	case fennec_key_comma              : return VK_OEM_COMMA ;
	case fennec_key_period             : return VK_OEM_PERIOD;
	case fennec_key_space              : return VK_SPACE     ;

	case fennec_key_printscreen: return VK_SNAPSHOT;
	case fennec_key_scrolllock : return VK_SCROLL  ;
	case fennec_key_pause      : return VK_PAUSE   ;
	case fennec_key_insert     : return VK_INSERT  ;
	case fennec_key_home       : return VK_HOME    ;
	case fennec_key_end        : return VK_END     ;
	case fennec_key_pageup     : return VK_PRIOR   ; 
	case fennec_key_pagedown   : return VK_NEXT    ;
	case fennec_key_delete     : return VK_DELETE  ;
	case fennec_key_backspace  : return VK_BACK    ;
	case fennec_key_up         : return VK_UP      ;
	case fennec_key_down       : return VK_DOWN    ;
	case fennec_key_left       : return VK_LEFT    ;
	case fennec_key_right      : return VK_RIGHT   ;

	case fennec_key_num_0       : return VK_NUMPAD0 ;
	case fennec_key_num_1       : return VK_NUMPAD1 ;
	case fennec_key_num_2       : return VK_NUMPAD2 ;
	case fennec_key_num_3       : return VK_NUMPAD3 ;
	case fennec_key_num_4       : return VK_NUMPAD4 ;
	case fennec_key_num_5       : return VK_NUMPAD5 ;
	case fennec_key_num_6       : return VK_NUMPAD6 ;
	case fennec_key_num_7       : return VK_NUMPAD7 ;
	case fennec_key_num_8       : return VK_NUMPAD8 ;
	case fennec_key_num_9       : return VK_NUMPAD9 ;
	case fennec_key_num_numlock : return VK_NUMLOCK ;
	case fennec_key_num_divide  : return VK_DIVIDE  ;
	case fennec_key_num_multiply: return VK_MULTIPLY;
	case fennec_key_num_minus   : return VK_SUBTRACT;
	case fennec_key_num_plus    : return VK_ADD     ;

	default: return 0;
	}
}

#endif

/*-----------------------------------------------------------------------------
 fennec, may 2007.
-----------------------------------------------------------------------------*/
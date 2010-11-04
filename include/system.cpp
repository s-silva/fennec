/*-----------------------------------------------------------------------------
  system header file.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include "system.h"

/* system specific codes - Microsoft (R) Windows*/

#ifdef system_microsoft_windows




	/* < disc i/o functions > */

	unsigned long sys_file_read(HANDLE h, void* a, unsigned long z)
	{
		unsigned long nbr = 0;
		if(ReadFile(h, a, z, &nbr, 0))return nbr;
		return v_error_sys_file_read;
	}

	unsigned long sys_file_write(HANDLE h, void* a, unsigned long z)
	{
		unsigned long nbr = 0;
		if(WriteFile(h, a, z, &nbr, 0))return nbr;
		return v_error_sys_file_write;
	}

	/* </ disc i/o functions > */


	


#endif /* </ system_microsoft_windows > */

/*-----------------------------------------------------------------------------
  system header file.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

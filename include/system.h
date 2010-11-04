/*-----------------------------------------------------------------------------
  system header file.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#ifndef header_system
#define header_system

/* system detection */

#ifdef _WIN32
#define system_microsoft_windows
#endif

/* system specific codes - Microsoft (R) Windows*/

#ifdef system_microsoft_windows

	#include <windows.h>




	/* types (C99) */

	#ifndef _STDINT_H

	typedef signed char          int8_t;
	typedef unsigned char        uint8_t;
	typedef short                int16_t;
	typedef unsigned short       uint16_t;
	typedef int                  int32_t;
	typedef unsigned             uint32_t;
	typedef long long            int64_t;
	typedef unsigned long long   uint64_t;

	typedef signed char          int_least8_t;
	typedef unsigned char        uint_least8_t;
	typedef short                int_least16_t;
	typedef unsigned short       uint_least16_t;
	typedef int                  int_least32_t;
	typedef unsigned             uint_least32_t;
	typedef long long            int_least64_t;
	typedef unsigned long long   uint_least64_t;

	typedef char                 int_fast8_t;
	typedef unsigned char        uint_fast8_t;
	typedef short                int_fast16_t;
	typedef unsigned short       uint_fast16_t;
	typedef int                  int_fast32_t;
	typedef unsigned  int        uint_fast32_t;
	typedef long long            int_fast64_t;
	typedef unsigned long long   uint_fast64_t;

	typedef INT_PTR              intptr_t;
	typedef UINT_PTR             uintptr_t;

	typedef long long            intmax_t;
	typedef unsigned long long   uintmax_t;

	#endif
	
	typedef wchar_t *string;
	typedef wchar_t letter;

	#define uni(x) L ## x
	#define str_len(s)          wcslen(s)
	#define str_cpy(d, s)       wcscpy(d, s)
	#define str_cat(d, s)       wcscat(d, s)
	#define str_ncpy(d, s, n)   wcsncpy(d, s, n)
	#define str_ncat(d, s, n)   wcsncat(d, s, n)
	#define str_cmp(a, b)       wcscmp(a, b)
	#define str_ncmp(a, b, n)   wcsncmp(a, b, n)
	#define str_icmp(a, b)      wcsicmp(a, b)
	#define str_lower(s)        wcslwr(s)
	#define str_upper(s)        wcsupr(s)
	#define str_inc(s)          wcsinc(s)
	#define str_dec(s)          wcsdec(s)
	#define str_mcpy(d, s, n)   wcsncpy(d, s, n)
	#define str_mmov(d, s, n)   memmove((char*)(d), (const char*)(s), (n) * sizeof(letter))
	#define str_size(s)         (wcslen(s) * sizeof(letter))
	#define str_incmp(a, b, n)  _memicmp(a, b, n * sizeof(letter))
	#define str_itos(i, b, n)   _itow(i, b, n)
	#define str_stoi(s)         _wtoi(s)
	#define str_chr(s, c)       wcschr(s, c)
	#define str_rchr(s, c)      wcsrchr(s, c)
	#define str_clower(c)       towlower(c)
	#define str_cupper(c)       towupper(c)
	#define str_ispunct(c)      iswpunct(c)





	/* < general values > */

	#define v_sys_maxpath 260
	
	/* </ general values > */
	



	/* < general functions > */

	#define sys_pass()  (Sleep(0))
	#define sys_sleep(n)(Sleep(n))
	#define sys_beep()  (Beep(1000,100))
	#define sys_feep()  (Beep(2000,100))

	/* </ general functions > */




	/* < disc i/o functions > */

	/* types */

	typedef HANDLE t_sys_file_handle;

	/* functions */

	#define sys_file_open(a, s)              (CreateFile(a, s, 0, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), 0, 0))
	#define sys_file_openshare(a, s)         (CreateFile(a, s, FILE_SHARE_READ, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), 0, 0))
	#define sys_file_openforce(a, s)         (CreateFile(a, s, 0, 0, (s == v_sys_file_forread ? OPEN_ALWAYS : CREATE_ALWAYS), 0, 0))
	#define sys_file_openforceshare(a, s)    (CreateFile(a, s, FILE_SHARE_READ, 0, (s == v_sys_file_forread ? OPEN_ALWAYS : CREATE_ALWAYS), 0, 0))
	#define sys_file_openbuffering(a, s)     (CreateFile(a, s, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), FILE_FLAG_SEQUENTIAL_SCAN, 0))
	#define sys_file_openstream(a, s)        (CreateFile(a, s, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), 0, 0))
	#define sys_file_openforcestream(a, s)   (CreateFile(a, s, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, (s == v_sys_file_forread ? OPEN_ALWAYS : CREATE_NEW), 0, 0))

	#define sys_file_create(a, s)            (CreateFile(a, s, 0, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), 0, 0))
	#define sys_file_createshare(a, s)       (CreateFile(a, s, FILE_SHARE_READ, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), 0, 0))
	#define sys_file_createforce(a, s)       (CreateFile(a, s, 0, 0, (s == v_sys_file_forread ? OPEN_ALWAYS : CREATE_ALWAYS), 0, 0))
	#define sys_file_createforceshare(a, s)  (CreateFile(a, s, FILE_SHARE_READ, 0, (s == v_sys_file_forread ? OPEN_ALWAYS : CREATE_ALWAYS), 0, 0))
	#define sys_file_createbuffering(a, s)   (CreateFile(a, s, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), FILE_FLAG_SEQUENTIAL_SCAN, 0))
	#define sys_file_createstream(a, s)      (CreateFile(a, s, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, (s == v_sys_file_forread ? OPEN_EXISTING : CREATE_NEW), 0, 0))
	#define sys_file_createforcestream(a, s) (CreateFile(a, s, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, (s == v_sys_file_forread ? OPEN_ALWAYS : CREATE_ALWAYS), 0, 0))

	#define sys_file_tell(h)                 (SetFilePointer(h, 0, 0, FILE_CURRENT))
	#define sys_file_seek(h, p)              (SetFilePointer(h, p, 0, FILE_BEGIN))
	#define sys_file_getsize(h)              (GetFileSize(h, 0))
	#define sys_file_seteof(h)               (SetEndOfFile(h))
	
	#define sys_file_close(h)                (CloseHandle(h) ? 1 : v_error_sys_file_close);
	
	unsigned long sys_file_read (t_sys_file_handle h, void* a, unsigned long z);
	unsigned long sys_file_write(t_sys_file_handle h, void* a, unsigned long z);

	/* error codes */

	#define v_error_sys_file_open   INVALID_HANDLE_VALUE
	#define v_error_sys_file_create INVALID_HANDLE_VALUE
	#define v_error_sys_file_read   0
	#define v_error_sys_file_write  0
	#define v_error_sys_file_close  0

	/* flags */

	#define v_sys_file_forwrite     GENERIC_WRITE
	#define v_sys_file_forread      GENERIC_READ

	/* </ disc i/o functions > */
	
	
	

#endif /* </ system_microsoft_windows > */




/* system independent codes */

/* < memory (de/re)allocation > */

#define sys_mem_alloc(a)     (malloc(a))
#define sys_mem_realloc(a, z)(realloc(a,z))
#define sys_mem_free(a)      (free(a))

/* error codes */

#define v_error_sys_mem_alloc 0

/* </ memory (de/re)allocation > */




#endif /* </ !header_system > */

/*-----------------------------------------------------------------------------
  system header file.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

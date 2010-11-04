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
	#include <wchar.h>




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
	#define str_icmp(a, b)      _wcsicmp(a, b)
	#define str_lower(s)        _wcslwr(s)
	#define str_upper(s)        _wcsupr(s)
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
	#define str_mkdir(s)        _wmkdir(s)
	#define str_str(s, f)       wcsstr(s, f)


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

	#define sys_file_open(a, s)              (CreateFile((a), (s), 0,                                  0, OPEN_EXISTING,  0, 0))
	#define sys_file_openshare(a, s)         (CreateFile((a), (s), FILE_SHARE_READ,                    0, OPEN_EXISTING,  0, 0))
	#define sys_file_openforce(a, s)         (CreateFile((a), (s), 0,                                  0, OPEN_ALWAYS,    0, 0))
	#define sys_file_openforceshare(a, s)    (CreateFile((a), (s), FILE_SHARE_READ,                    0, OPEN_ALWAYS,    0, 0))
	#define sys_file_openstream(a, s)        (CreateFile((a), (s), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,  0, 0))
	#define sys_file_openforcestream(a, s)   (CreateFile((a), (s), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_ALWAYS,    0, 0))
	#define sys_file_openbuffering(a, s)     (CreateFile((a), (s), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING,  FILE_FLAG_SEQUENTIAL_SCAN, 0))

	#define sys_file_create(a, s)            (CreateFile((a), (s), 0,                                  0, CREATE_NEW,     0, 0))
	#define sys_file_createshare(a, s)       (CreateFile((a), (s), FILE_SHARE_READ,                    0, CREATE_NEW,     0, 0))
	#define sys_file_createforce(a, s)       (CreateFile((a), (s), 0,                                  0, CREATE_ALWAYS,  0, 0))
	#define sys_file_createforceshare(a, s)  (CreateFile((a), (s), FILE_SHARE_READ,                    0, CREATE_ALWAYS,  0, 0))
	#define sys_file_createstream(a, s)      (CreateFile((a), (s), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW,     0, 0))
	#define sys_file_createforcestream(a, s) (CreateFile((a), (s), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_ALWAYS,  0, 0))
	#define sys_file_createbuffering(a, s)   (CreateFile((a), (s), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, CREATE_NEW,     FILE_FLAG_SEQUENTIAL_SCAN, 0))


	#define sys_file_tell(h)    (SetFilePointer((h),   0, 0, FILE_CURRENT))
	#define sys_file_seek(h, p) (SetFilePointer((h), (p), 0, FILE_BEGIN))
	#define sys_file_getsize(h) (GetFileSize((h), 0))
	#define sys_file_seteof(h)  (SetEndOfFile((h)))
	#define sys_file_rename(s, d) (_wrename(s, d))
											  
	#define sys_file_close(h)   (CloseHandle((h)) ? 1 : v_error_sys_file_close);
											  

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
	
	
	
	
	/* <timers> */
	
	/* functions */
	
	#define sys_timer_getms()  (timeGetTime())    /* get time (ms) */
	#define sys_timer_uninit(h)(timeEndPeriod(h))

	int     sys_timer_init(void);
	
	/* error codes */
	
	#define v_error_sys_timer_init 0
             
	/* </timers> */
	
	
	
	
	/* <multimedia> */

	/* types */
	
	typedef unsigned long t_sys_media_handle;
	
	/* flags */
	
	#define v_sys_media_infinite_repeats    INFINITE
	#define v_sys_media_driver_default      WAVE_MAPPER
	
	/* error codes */
			
	#define v_error_sys_media_main_init     0
	#define v_error_sys_media_main_write    0
	#define v_error_sys_media_main_play     0
	#define v_error_sys_media_main_pause    0
	#define v_error_sys_media_main_stop     0
	#define v_error_sys_media_main_uninit   0
	          
	#define v_error_sys_media_stream_init   0
	#define v_error_sys_media_stream_write  0
	#define v_error_sys_media_stream_play   0
	#define v_error_sys_media_stream_pause  0
	#define v_error_sys_media_stream_stop   0
	#define v_error_sys_media_stream_uninit 0

	#define v_error_sys_media_getinfo_nodev  -1
	#define v_error_sys_media_getinfo_badid  -2
	#define v_error_sys_media_getinfo_other  -3
             
	/* functions */
		   
	#define sys_media_handlemultiple()      (1)  /* support multiple sound streams */
	
	#define sys_media_main_init(d, f, c, b, r)() /* initialize main engine (if no multiple streams supported) ; params: device id, freq, channels, bits per sample, repeats */
	#define sys_media_main_write(h, b, z)()      /* write main buffer */
	#define sys_media_main_play(h)()             /* start main engine */
	#define sys_media_main_pause(h)()            /* pause main engine */
	#define sys_media_main_stop(h)()             /* stop main engine */
	#define sys_media_main_uninit(h)()           /* uninitialize main engine */
	
	unsigned int  sys_media_stream_init            (unsigned int d, unsigned int freq, unsigned int channels, unsigned int bps);
	unsigned int  sys_media_stream_write           (unsigned int h, void* b, unsigned int z, unsigned int reps);
	int           sys_media_stream_play            (unsigned long h);
	int           sys_media_stream_pause           (unsigned long h);
	int           sys_media_stream_stop            (unsigned long h);
	int           sys_media_stream_clear           (unsigned long h);
	int           sys_media_stream_uninit          (unsigned long h);
	unsigned long sys_media_stream_buffertimer_byte(unsigned long h);
	int           sys_media_check                  (unsigned int dvid, unsigned long freq, unsigned int bps, unsigned int channels);
	int           sys_media_getinfo                (unsigned int dvid, unsigned long *sformats, int *maxchannels);
	
	
	
	
	/* </multimedia> */
	
	
	

	/* <threading> */
	
	/* types */
	
	typedef HANDLE                 t_sys_thread_handle;
	typedef LPTHREAD_START_ROUTINE t_sys_thread_function;
	typedef CRITICAL_SECTION       t_sys_thread_share;
	
	/* flags */
	
	#define v_sys_thread_idle        THREAD_PRIORITY_IDLE
	#define v_sys_thread_low         THREAD_PRIORITY_LOWEST
	#define v_sys_thread_belownormal THREAD_PRIORITY_BELOW_NORMAL
	#define v_sys_thread_normal      THREAD_PRIORITY_NORMAL
	#define v_sys_thread_abovenormal THREAD_PRIORITY_ABOVE_NORMAL
	#define v_sys_thread_high        THREAD_PRIORITY_HIGHEST
	#define v_sys_thread_highest     THREAD_PRIORITY_TIME_CRITICAL
	        	                            
	/* error codes */
	
	#define v_error_sys_thread_exit 0
	#define v_error_sys_thread_call 0
	          
	/* functions */
	
	#define sys_thread_function_header(a) unsigned long a(LPVOID lpData)
	#define sys_thread_function_caller(a) a(0);
	#define sys_thread_function_return()  return 0
	
	t_sys_thread_handle sys_thread_call       (t_sys_thread_function cfunc);
	int                 sys_thread_exit       (t_sys_thread_handle thand);
	int                 sys_thread_setpriority(t_sys_thread_handle thand, int plevel);

	int sys_thread_share_create (t_sys_thread_share* cs);
	int sys_thread_share_close  (t_sys_thread_share* cs);
	int sys_thread_share_request(t_sys_thread_share* cs);
	int sys_thread_share_release(t_sys_thread_share* cs);




	/* </threading> */
	
	
	
	
	/* <file system> */
	
	/* types */
	
	typedef HANDLE t_sys_fs_find_handle;

	/* functions */
	int sys_fs_file_exist(const string fpath);
	int sys_fs_find_start(const string inpath, string outpath, size_t osize, t_sys_fs_find_handle* shandle);
	int sys_fs_find_next (string outpath, size_t osize, t_sys_fs_find_handle shandle);
	int sys_fs_find_close(t_sys_fs_find_handle shandle);

	/* </file system> */
	
	
	
	
	/* <dynamic link libraries> */
	
	/* types */
	
	typedef HMODULE t_sys_library_handle;
	typedef char*   t_sys_library_function;
	               
	/* error codes */
	
	#define v_error_sys_library_load 0
	#define v_error_sys_library_getaddress 0
	#define v_error_sys_library_free 0
	
	/* data */
	
	#define v_sys_lbrary_extension uni(".dll")
	
	/* functions */
	
	#define sys_library_load(p)              (LoadLibrary(p))
	#define sys_library_getaddress(h, f)     (GetProcAddress(h, f))
	#define sys_library_free(h)              (FreeLibrary(h))
	#define sys_library_getfilename(h, b, m) (GetModuleFileName(h, b, m))
	#define sys_library_getcurrenthandle()   (GetModuleHandle(0))

	/* </dynaimc link libraries> */
	
	
	

#endif /* </ system_microsoft_windows > */




/* system independent codes */

/* < memory (de/re)allocation > */

/*
#define sys_mem_alloc(a)     (malloc(a))
#define sys_mem_realloc(a, z)(realloc(a,z))
*/

void *sys_mem_alloc(size_t sz);
void *sys_mem_realloc(void *imem, size_t sz);

#define sys_mem_free(a)      (free(a))

/* error codes */

#define v_error_sys_mem_alloc 0

/* </ memory (de/re)allocation > */




#endif /* </ !header_system > */

/*-----------------------------------------------------------------------------
  system header file.
  copyright (c) 2007 chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

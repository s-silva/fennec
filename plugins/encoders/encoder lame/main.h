#include "../../../include/system.h"
#include "../../../include/fennec.h"
#include "lame/include/lame.h"
#include "data/system/windows/resource.h"

extern HINSTANCE dll_instance;

struct plugin_stream
{
	int                 initialized;
	int                 bmode;         /* base mode, fennec_plugintype... */
	int                 smode;         /* sub mode, fenne_encoder_file etc. */

	letter              filepath[v_sys_maxpath];
	t_sys_file_handle   fhandle;

	unsigned long       cfrequency;    /* current frequency */
	unsigned long       cbitspersample;
	unsigned long       cchannels;

	lame_global_flags  *lameflags;
	char               *outdata;
};

extern struct plugin_stream   *pstreams;
extern struct plugin_settings  fsettings;

/* declarations -------------------------------------------------------------*/

int callc fennec_plugin_initialize(void);
int callc fennec_plugin_uninitialize(void);

int proc_settings(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

unsigned long callc file_create(string fname, unsigned long fcreatemode);
int           callc file_write(unsigned long id, void* pcmin, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize);
int           callc file_close(unsigned long id); 
int           callc file_flush(unsigned long id);
int           callc file_seek(unsigned long id, double spos);
double        callc file_tell(unsigned long id, unsigned long tellmode);
unsigned long callc encode_initialize(void);
int           callc encode_encode(unsigned long id, void* pcmin, void* eout, unsigned long sfreq, unsigned long sbps, unsigned long schannels, unsigned long dsize, unsigned long* osize, encode_realloc reallocfunc);
int           callc encode_uninitialize(unsigned long id);
int           callc about(unsigned long id, void* odata);
int           callc settings_fileencoder(unsigned long id, void* odata);
int           callc settings_encoder(unsigned long id, void* odata);
int           callc get_initialization_info(unsigned long inforreq, void* outinfo, unsigned long* outsize);

int encoder_setdefaults(unsigned long id);
int encoder_appendextension(unsigned long id, string fpath);
int encoder_deleteextension(unsigned long id, string fpath);
int encoder_initialize(unsigned long id, int fcreatemode);
int encoder_uninitialize(unsigned long id);
int encoder_set(unsigned long id);
int encoder_write(unsigned long id, void* buffer, unsigned long bsize);
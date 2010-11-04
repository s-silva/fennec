#define _WIN32_DCOM

#include "../../../include/system.h"
#include "../../../include/fennec.h"
#include "../../../include/text ids.h"
#include "../../../include/ids.h"
#include "workspaces/visual studio.net/resource.h"
#include <dsound.h>
#include <objbase.h>
#include <math.h>
#include <Mmreg.h>
#include <Ks.h>
#include <Ksmedia.h>
#include <commctrl.h>

int    show_settings(void* vdata);
int    show_about(void* vdata);
int    audio_initialize(void);
int    audio_uninitialize(void);
int    audio_load(const string spath);
int    audio_add(dword idx, const string spath);
int    audio_play(void);
int    audio_playpause(void);
int    audio_pause(void);
int    audio_stop(void);
int    audio_setposition(double cpos);
int    audio_setposition_ms(double mpos);
int    audio_getplayerstate(void);
double audio_getposition(void);
double audio_getposition_ms(void);
double audio_getduration_ms(void);
int    audio_getpeakpower(DWORD* pleft, DWORD* pright);
int    audio_setvolume(double vl, double vr);
int    audio_getvolume(double* vl, double* vr);
int    audio_getcurrentbuffer(void* dout, dword* dsize);
int    audio_getfloatbuffer(float* dout, dword scount, dword channel);
int    audio_getdata(unsigned int di, void* ret);
int    audio_preview_action(int actionid, void *adata);


struct audioplayer
{
	int                   initialized;
	int                   state;
	int                   loaded;
	int                   input_plugin;
	
	unsigned long         stream_handle;

	unsigned long         frequency, channels, bps, outchannels;

	LPDIRECTSOUND         ds; 
	DSBUFFERDESC          dsb;
	WAVEFORMATEXTENSIBLE  wfx;

	unsigned long         buffersize;
	unsigned long         prebuffersize;
	LPDIRECTSOUNDBUFFER   buffer;

	letter                current_file[v_sys_maxpath];

	int                   main_volume;
	unsigned long         dur_ms, pos_ms;
	int64_t               written_samples;

	char                 *prebuffer;

	char                 *public_prebuffer;
	unsigned long         public_prebuffer_length;
	unsigned long         public_prebuffer_clength;

	int                   stopping;
	unsigned long         stop_at;
	unsigned long         stop_start_time;

	
	int                   output_bps;
	int                   output_float;
	int                   output_signed;

	double                nrc[16];
	double                buffer_times;

	unsigned long         start_time;
};

struct output_player
{
	struct audioplayer    audio;

	struct
	{
		CRITICAL_SECTION     cst;
		HWND                 window_audio;
		unsigned long        thread_wlpos;
		GUID                 audio_out_device;
		int                  audio_out_default;
	} engine;
};



DWORD WINAPI thread_audio(LPVOID lpParam);
int local_readdata(unsigned long pid, unsigned long fid, unsigned long *dsize, void* odata, int posset, struct audioplayer *pl);
int CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
int setting_priority_to_sys(int sp);
int local_downsample(void* dout, void *idata, unsigned long dsize);
int local_def_downsample(void* dout, void *idata, unsigned long dsize);
int local_def_readdata(int playerid, unsigned long pid, unsigned long fid, unsigned long *dsize, void* odata);


extern GUID                 audio_out_device;
extern int                  audio_out_default;
extern GUID                 preview_audio_out_device;
extern int                  preview_audio_out_default;

extern struct audioplayer   splayer;
extern struct fennec        sfennec;
extern CRITICAL_SECTION     cst;
extern HWND                 window_audio;
extern HANDLE               hthread_audip;
extern HINSTANCE            plugin_instance;

extern int                  thread_audio_terminate;
extern unsigned long        thread_wlpos;
extern int                  device_combo_index;

extern struct output_player players[1];

#define public_prebuffer_size 0x19000 /* 100 kb */
#define upsampled_val(v)   (( (v) * splayer.bps) / splayer.output_bps)
#define downsampled_val(v) (( (v) * splayer.output_bps) / splayer.bps)

int subengine_initialize(void);
int subengine_uninitialize(void);
int subengine_load(int pid, const string spath);
int subengine_play(int pid);
int subengine_pause(int pid);
int subengine_stop(int pid);
int subengine_seek(int pid, double pos);
double subengine_getduration(int pid);
double subengine_getposition(int pid);
double subengine_getvolume(int pid, int cid);
int subengine_setvolume(int pid, int cid, double vol);
int subengine_thread(int tdata);
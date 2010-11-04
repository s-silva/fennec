#include "..\..\..\include\system.h"
#include "..\..\..\include\fennec.h"
#include <windows.h>
#include <math.h>

int glvis_init(HWND pwnd);
int glvis_display(void);
int glvis_uninit(void);

int visualization_get_rel_path(string buf, const string relpath, int csize);

extern float     wave_buffer[2][1024];
extern float     fft_buffer [2][1024];
extern float     bars_buffer [2][1024];
extern HWND      window_vis_in;
extern HWND      window_vis;
extern WNDPROC   window_vis_proc;
extern float     z;
extern int       gl_lights;
extern float     values[16];

extern struct    fennec                  sfennec;
extern struct    general_videoout_data   vdata;

int glvis_init(HWND pwnd);
int glvis_display(void);
int glvis_uninit(void);

void fft_float(
    unsigned  NumSamples,       /* must be a power of 2          */
    int       InverseTransform,	/* 0=forward FFT, 1=inverse FFT  */
    float    *RealIn,			/* array of input's real samples */
    float    *ImagIn,			/* array of input's imag samples */
    float    *RealOut,			/* array of output's reals       */
    float    *ImagOut);		/* array of output's imaginaries */
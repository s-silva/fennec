HWND BaseWindows_ShowRipping(int wmodal);
HWND BaseWindows_ShowJoining(int wmodal);

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void   AddDirectory(const string fpath);
int    OpenFileList(const string fpaths);
int    OpenFileListOld(string fpaths);

	/* general menus.c */

int Menu_MainProc(long mi);

	/* Fennec global environment.c */

int GlobalFunction(unsigned long fid, ...);

	/* settings interface */

void EqualizeBufferVariable(void* dbuf, int chan, int freq, int bps, unsigned long dlen, float *veqbands, int *vi, int *vj, int *vk, eq_bands_data *eqhistory);

int      png_file_blit(HDC dc, const char *path, int x, int y, unsigned int w, unsigned int h, int sx, int sy);
int      png_file_blit(HDC dc, const char *path, int x, int y, unsigned int w, unsigned int h, int sx, int sy);
int      png_free_bmp(unsigned long *dbitmap);
int      png_res_blit(HDC dc, HINSTANCE hinst, const char *sdir, const char *sname, int x, int y, unsigned int w, unsigned int h, int sx, int sy);
int      png_file_load_dc(const char *path, unsigned int *w, unsigned int *h, HDC *odc);
int      png_res_load_bmp(HINSTANCE hinst, const char *sdir, const char *sname, unsigned int *w, unsigned int *h, unsigned long *dbitmap);
int      png_res_load_dc(HINSTANCE hinst, const string sdir, const string sname, unsigned int *w, unsigned int *h, HDC *dc);
HBITMAP  png_res_load_winbmp(HINSTANCE hinst, const string sdir, const string sname);

#define Function_SetVolumePercent    0x0001
#define Function_Mute                0x0002
#define Function_OpenFileDialog      0x0003
#define Function_Play                0x0004
#define Function_Pause               0x0005
#define Function_Stop                0x0006
#define Function_Previous            0x0007
#define Function_Next                0x0008
#define Function_Rewind              0x0009
#define Function_Forward             0x000a
#define Function_SelectThemePanel    0x000b
#define Function_MainPanel           0x000c
#define Function_EqualizerPanel      0x000d
#define Function_ShowCredits         0x000e
#define Function_RefreshDisplay      0x000f
#define Function_AddFileDialog       0x0010

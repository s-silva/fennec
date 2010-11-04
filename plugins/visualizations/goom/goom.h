#ifndef _GOOM_H
#define _GOOM_H

/* in file about.c */
void about() ;

/* in file readme.c */
extern const char readme[];

/* in file configmenu.c */
typedef struct t_config {
    int xres,yres ;
} t_config ;

void plug_configure(void) ;
void plug_load_prefs(void) ;
extern t_config extern_config ;
int goom_main(HWND hwnd, HDC dc);
void jeko_render_pcm(short data[2][512]);

#endif

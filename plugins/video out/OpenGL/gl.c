#include "main.h"
#include <stdio.h>
#include <gl\gl.h>
#include <gl\glu.h>
//#include <gl\glaux.h>

extern int       w, h;
extern void     *current_frame;
extern double    aspect_ratio;

HDC			hDC=NULL;
HGLRC		hRC=NULL;
HWND		hWnd=NULL;
HINSTANCE	hInstance;
HWND        window_vis_in;

BOOL	    keys[256];
BOOL	    active=TRUE;
BOOL	    fullscreen=TRUE;

int         screen_w, screen_h;
int         imgz_w = 1024;
int         imgz_h = 512;

GLuint	    texture[4];
GLuint	    base;	

HDC         video_window_dc;
HDC         windowdc;
HBITMAP     membitmap;
void       *mempoints;

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#ifndef GL_CLAMP_TO_EDGE
#   define GL_CLAMP_TO_EDGE 0x812F
#endif

GLYPHMETRICSFLOAT  agmf[256]; 
HFONT              font_subtitles = 0;
extern string      subtitle_text;
int                subs_fontsize = 18;
void              *fdata = 0;

#define subtitle_font_face uni("Arial")

int nearst_powertwo(int a)
{
    int r = 1;
    while(r < a)
    {
        r *= 2;
    }
    return r;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if(width != screen_w || height != screen_h)
		glViewport(0, 0, width,height);

	screen_w = width ;
	screen_h = height;

	if(font_subtitles)
		DeleteObject(font_subtitles);

	subs_fontsize = (height * 18) / 500;
	if(subs_fontsize < 8)subs_fontsize = 8;

	//glDeleteLists(base, 96);
	//base = glGenLists(96);
    
	font_subtitles = CreateFont(-MulDiv(subs_fontsize, GetDeviceCaps(video_window_dc, LOGPIXELSY), 72),
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                DEFAULT_PITCH, subtitle_font_face);

	SelectObject(video_window_dc, font_subtitles);
	/*wglUseFontBitmaps(video_window_dc, 0, 256, 1000); */


		
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();

	//gluPerspective(0.0f,(GLfloat)width / (GLfloat)height,0.0f,0.0f);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
}

int InitGL(GLvoid)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_2D);

	//base = glGenLists(96);
    
	font_subtitles = CreateFont(-MulDiv(subs_fontsize, GetDeviceCaps(video_window_dc, LOGPIXELSY), 72),
                                0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                                DEFAULT_PITCH, subtitle_font_face);

	SelectObject(video_window_dc, font_subtitles);

	//wglUseFontBitmaps(video_window_dc, 0, 256, 1000); 
					//glShadeModel(GL_SMOOTH); 

			// move bottom left, southwest of the red triangle  
			
			 
			// set up for a string-drawing display list call  
	///glListBase(1000); 
	return TRUE;
}

int DrawGLScene(GLvoid)
{
	static int    drtime = 0, fsize = 0, lastw, lasth;
	int           j, k, v1, v2;
	float         fr;
	char          subtext[1024];
	BOOL          useddef = 1;


	if((!drtime || (lastw != w) || (lasth != h)) && current_frame)
	{
		int i =0;
		glGenTextures(1, &texture[i]);

		if(fdata) free(fdata);

		imgz_w = nearst_powertwo(w);
		imgz_h = nearst_powertwo(h);

		fsize = imgz_w * imgz_h * 3;
		fdata = malloc(fsize);

		lasth = h;
		lastw = w;


		memset(fdata, 0, fsize);

		for(j = 0; j< h; j++)
		{
			for(k = 0; k< w; k++)
			{
				v1 = ((j * imgz_w) + k) * 3;
				v2 = ((j * w) + k) * 4;

				((char*)fdata)[v1]     = ((char*)current_frame)[v2 + 2];
				((char*)fdata)[v1 + 1] = ((char*)current_frame)[v2 + 1];
				((char*)fdata)[v1 + 2] = ((char*)current_frame)[v2];
				//((char*)fdata)[v1 + 3] = ((char*)current_frame)[v2 + 3];
			}
		}

		glBindTexture(GL_TEXTURE_2D, texture[i]);

        /* Set the texture parameters */
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1.0 );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glTexImage2D(GL_TEXTURE_2D, 0, 3, imgz_w, imgz_h, 0, GL_RGB, GL_UNSIGNED_BYTE, fdata);
		drtime = 1;
	}

	
	if(current_frame)
	{
		for(j = 0; j< h; j++)
		{
			for(k = 0; k< w; k++)
			{
				v1 = ((j * imgz_w) + k) * 3;
				v2 = ((j * w) + k) * 4;

				((char*)fdata)[v1]     = ((char*)current_frame)[v2 + 2];
				((char*)fdata)[v1 + 1] = ((char*)current_frame)[v2 + 1];
				((char*)fdata)[v1 + 2] = ((char*)current_frame)[v2];
				//((char*)fdata)[v1 + 3] = ((char*)current_frame)[v2 + 3];
			}
		}
	}

	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	glOrtho (0, screen_w, screen_h, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
	glTranslatef(0,0,0);		

	
	
	glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, imgz_w, imgz_h, GL_RGB, GL_UNSIGNED_BYTE, fdata );
	glBindTexture( GL_TEXTURE_2D, texture[0]);


	{
		int ips, xps;
		float fc, fa, ar = 1.0f;
		int   nh = h, nw = w;

		if(!h || !w) return 0;

		fa = (float)imgz_h / (float)imgz_w;

		aspect_ratio = 0;

		if(aspect_ratio > 0.0)
		{
			ar = 1.0f / aspect_ratio;
			nh = (double)nw / aspect_ratio;
		}

		fc = (float)screen_w / (float)nw;


		if((fc *(float)nh) > ((float)screen_h))
		{
			fc = (float)screen_h / (float)nh;

			fr = (float)screen_h + ((float)(imgz_w - nh) * fc);
		}else{
			fr = (float)screen_w + ((float)(imgz_w - nw) * fc);
		}

		if(nh < h)
		{
			ips = ((float)screen_h - ((float)(fr * ar) ))/2;
		}else{
			ips = ((float)screen_h - ((float)(nh * fc) ))/2;
			ar = 1.0;
		}

		xps = ((float)screen_w - ((float)nw * fc))/2;

		glBegin(GL_POLYGON);
			glTexCoord2f(0, 0);  glVertex2f( xps,      ips);
			glTexCoord2f(1, 0);  glVertex2f( xps + fr, ips);
			glTexCoord2f(1, 1);  glVertex2f( xps + fr, ips + (fr * fa * ar));
			glTexCoord2f(0, 1);  glVertex2f( xps,      ips + (fr * fa * ar));
		glEnd();

		if(subtitle_text)
		{
			HDC hdc = video_window_dc;
			// make the color a deep blue hue  

			/*glDisable(GL_TEXTURE_2D);
			// make the shading smooth 

			 
			WideCharToMultiByte(CP_ACP, 0, subtitle_text, -1, subtext, sizeof(subtext), "?", &useddef);

			glRasterPos2f(30.0F, screen_h - subs_fontsize - (100.0 * ((float)screen_h / 1024.0f))); 
			glColor3f(1.0F, 1.0F, 1.0F); 
			// draw a string using font display lists  
			glCallLists(str_len(subtitle_text), GL_UNSIGNED_BYTE, subtext); 

			// get all those commands to execute  
			glFlush(); 

			glEnable(GL_TEXTURE_2D);*/
			
		}

		
	}
	
	

	current_frame = 0;

	return 1;
}

GLvoid KillGLWindow(GLvoid)
{
	if(fullscreen)
	{
		ChangeDisplaySettings(0, 0);
		ShowCursor(TRUE);
	}

	if(hRC)
	{
		wglMakeCurrent(0, 0);
		wglDeleteContext(hRC);
		hRC = 0;
	}

	ReleaseDC(hWnd, hDC);
	DestroyWindow(hWnd);
	UnregisterClass(uni("OpenGL"), hInstance);
}

BOOL CreateGLWindow(string title, int width, int height, int bits, BOOL fullscreenflag)
{
	GLuint		PixelFormat;
	WNDCLASS	wc;
	RECT		WindowRect;
	HWND        hwndp;
	RECT        rct;

	static	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		0,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	pfd.cColorBits = 32;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

	vdata.getdata(get_window_video, 0, &hwndp, 0);

	WindowRect.left    = 0;
	WindowRect.right   = width;
	WindowRect.top     = 0;
	WindowRect.bottom  = height;

	screen_w = width;
	screen_h = height;

	fullscreen = fullscreenflag;

	hInstance			= GetModuleHandle(NULL);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= 0;
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= uni("OGLvideoOutputFennec12");

	if(!RegisterClass(&wc)) return 0;

	
	vdata.getdata(get_window_video_rect, 0, &rct, 0);

	width  = rct.right - rct.left;
	height = rct.bottom - rct.top;
	
	screen_w = rct.right - rct.left;
	screen_h = rct.bottom - rct.top;

	hWnd = CreateWindowEx(0, uni("OGLvideoOutputFennec12"), title,
						  WS_CHILD, rct.left, rct.top, width, height, 
						  hwndp, 0, hInstance, 0);


	hDC = video_window_dc = windowdc = GetDC(hWnd);
	
	PixelFormat = ChoosePixelFormat(hDC,&pfd);
	SetPixelFormat(hDC,PixelFormat,&pfd);
	DescribePixelFormat(hDC, PixelFormat, sizeof(PIXELFORMATDESCRIPTOR),&pfd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC,hRC);
	
	ShowWindow(hWnd,SW_SHOW);
	ReSizeGLScene(width, height);
	InitGL();

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		if(!HIWORD(wParam)) active=TRUE;
		else                active=FALSE;
		return 0;

	case WM_PAINT:
		glvis_display();
		break;

	case WM_CLOSE:
		glvis_uninit();
		return 0;

	case WM_SIZE:
		ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
		glvis_display();
		return 0;

	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		{
			RECT rct, rctp;
			GetWindowRect(GetParent(hWnd), &rctp);
			GetWindowRect(hWnd, &rct);
			lParam = MAKELONG(LOWORD(lParam) + (rct.left - rctp.left), HIWORD(lParam) + (rct.top - rctp.top));

			SendMessage(GetParent(hWnd), uMsg, wParam, lParam);
		}
		break;

	case WM_KEYDOWN:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		SendMessage(GetParent(hWnd), uMsg, wParam, lParam);
		break;
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int glvis_init(HWND pwnd)
{
	fullscreen = 0;
	srand(GetTickCount());
	if(!CreateGLWindow(uni("Cube"),imgz_w,768,16,fullscreen)) return -1;
	return 0;
}

int glvis_display(void)
{
	DrawGLScene();
	SwapBuffers(hDC);
	if(subtitle_text)
	{
		RECT rct;
		rct.top = screen_h - subs_fontsize - (100.0 * ((float)screen_h / 1024.0f));
		rct.bottom = screen_h;
		rct.left = 0;
		rct.right = screen_w;
		SetBkMode(video_window_dc, TRANSPARENT);
		SetTextColor(video_window_dc, 0xffffff);
		BeginPath(video_window_dc);
		DrawText(video_window_dc, subtitle_text, str_len(subtitle_text), &rct, DT_CENTER);
		EndPath(video_window_dc);
		//WidenPath(video_window_dc);
		StrokePath(video_window_dc);
		DrawText(video_window_dc, subtitle_text, str_len(subtitle_text), &rct, DT_CENTER);
		
	}

	return 0;
}


int glvis_uninit(void)
{
	KillGLWindow();
	if(fdata)
		free(fdata);
	return 0;
}

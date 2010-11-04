/*-----------------------------------------------------------------------------
  sample visualization for fennec (cube).
  -chase <c-h@users.sf.net>
-----------------------------------------------------------------------------*/

#include "main.h"
#include <stdio.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h>

HDC			hDC=NULL;
HGLRC		hRC=NULL;
HWND		hWnd=NULL;
HINSTANCE	hInstance;
HWND        window_vis_in;

BOOL	    keys[256];
BOOL	    active=TRUE;
BOOL	    fullscreen=TRUE;

GLfloat	    xrot;
GLfloat	    yrot;
GLfloat	    zrot;

GLuint	    texture[4];

float       last_peak_x = 0, last_peak_y = 0, last_peak_z = 0;
int         rside_x = 0, rside_y = 0, rside_z = 0;
GLfloat	    z = -50.0f;
int         gl_lights = 1;
float       values[16] = {  1.0f, 0.5f, 1.0f, 0.5f,
							0.05f, 1.0f, 1.0f, 1.0f,
							1.0f, 1.0f, 1.0f, 1.0f,
							1.0f, 1.0f, 1.0f, 1.0f};

GLfloat LightAmbient[]  =  { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightDiffuse[]  =  { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat LightPosition1[] = { 0.0f, 0.0f, +10.0f, 1.0f };
GLfloat LightPosition2[] = { 0.0f, 0.0f, -10.0f, 1.0f };
GLfloat LightCutoff[]   = {0.0f, 1.0f, 1.0f, 1.0f};

LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#define  cube_count 10
#define  cube_space 3

AUX_RGBImageRec *LoadBMP(string Filename)
{
	FILE *File=NULL;

	if (!Filename)
	{
		return NULL;
	}

	File=_wfopen(Filename,uni("r"));

	if (File)
	{
		fclose(File);
		return auxDIBImageLoad(Filename);
	}

	return NULL;
}

int LoadGLTextures()
{
	int     Status=FALSE, i;
	string  fstr;
	letter  cbuf[v_sys_maxpath];

	AUX_RGBImageRec *TextureImage[3];

	memset(TextureImage,0,sizeof(void *)*1);

	for(i=0; i<3; i++)
	{
		switch(i)
		{
		case 0:
			visualization_get_rel_path(cbuf, uni("data\\vis\\cube.1.bmp"), sizeof(cbuf));
			fstr = cbuf;
			break;

		case 2:
			visualization_get_rel_path(cbuf, uni("data\\vis\\cube.2.bmp"), sizeof(cbuf));
			fstr = cbuf;
			break;

		case 1:
			visualization_get_rel_path(cbuf, uni("data\\vis\\cube.3.bmp"), sizeof(cbuf));
			fstr = cbuf;
			break;
		}

		if (TextureImage[i]=LoadBMP(fstr))
		{
			Status=TRUE;

			glGenTextures(1, &texture[i]);


			glBindTexture(GL_TEXTURE_2D, texture[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureImage[i]->sizeX, TextureImage[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		if (TextureImage[i])
		{
			if (TextureImage[i]->data)
			{
				free(TextureImage[i]->data);
			}

			free(TextureImage[i]);
		}
	}

	return Status;
}

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)
{
	if (height==0)
	{
		height=1;
	}

	glViewport(0,0,width,height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();


	gluPerspective(45.0f,(GLfloat)width/(GLfloat)height,0.1f,300.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

int InitGL(GLvoid)
{

	if (!LoadGLTextures())
	{
		return FALSE;
	}

	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glLightfv(GL_LIGHT1, GL_AMBIENT,  LightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE , LightDiffuse);
	glLightfv(GL_LIGHT2, GL_AMBIENT,  LightAmbient);
	glLightfv(GL_LIGHT2, GL_DIFFUSE , LightDiffuse);

	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	//glEnable(GL_LIGHTING);
	return TRUE;
}

int DrawGLScene(GLvoid)
{
	float v, d, m = 50.0f, cmul = (float)(cube_count * cube_space), cs = (float)cube_space;
	int   i, j;

	m = 100.0f * values[3];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();



	glTranslatef(0.0f, 0.0f, z);
	
	glTranslatef(0.0f, 0.0f, -100.0f);

	glDisable(GL_DEPTH_TEST);
	
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	v = 40.0f;
    d = 150.0f;
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-v,  v, -v); glTexCoord2f(0.0f, 0.0f); glVertex3f(-v,  v,  d); glTexCoord2f(1.0f, 0.0f); glVertex3f( v,  v,  d); glTexCoord2f(1.0f, 1.0f); glVertex3f( v,  v, -v);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-v, -v, -v); glTexCoord2f(1.0f, 1.0f); glVertex3f(-v,  v, -v); glTexCoord2f(0.0f, 1.0f); glVertex3f( v,  v, -v); glTexCoord2f(0.0f, 0.0f); glVertex3f( v, -v, -v);
		glTexCoord2f(1.0f, 0.0f); glVertex3f( v, -v, -v); glTexCoord2f(1.0f, 1.0f); glVertex3f( v,  v, -v); glTexCoord2f(0.0f, 1.0f); glVertex3f( v,  v,  d); glTexCoord2f(0.0f, 0.0f); glVertex3f( v, -v,  d);
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-v, -v, -v); glTexCoord2f(1.0f, 0.0f); glVertex3f(-v, -v,  d); glTexCoord2f(1.0f, 1.0f); glVertex3f(-v,  v,  d); glTexCoord2f(0.0f, 1.0f); glVertex3f(-v,  v, -v);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[2]);

	glBegin(GL_QUADS);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-v, -v, -v); glTexCoord2f(0.0f, 1.0f); glVertex3f( v, -v, -v); glTexCoord2f(0.0f, 0.0f); glVertex3f( v, -v,  d); glTexCoord2f(1.0f, 0.0f); glVertex3f(-v, -v,  d);
	glEnd();

	glEnable(GL_DEPTH_TEST);


	glTranslatef(0.0f, 0.0f, +100.0f);
	
	glTranslatef(0.0f, 0.0f, -85.0f);
	
	glRotatef(xrot,1.0f,0.0f,0.0f);
	glRotatef(yrot,0.0f,1.0f,0.0f);
	glRotatef(zrot,0.0f,0.0f,1.0f);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition1);
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition2);

	glTranslatef(0.0f, 0.0f, +85.0f);
	
	glTranslatef(-(cmul / 2.0f) + cs, -cmul / 2.0f, -(85.0f - (cmul / 2.0f)));

	/* front */
	for(i=0; i<cube_count; i++){
		for(j=0; j<cube_count; j++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_QUADS);
				v = (bars_buffer[0][(i * 50) + (j * 10)] / 2.0f) * m + 0.5f;
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f);
			glEnd();
			glTranslatef(cs, 0.0f, 0.0f);
		}
		glTranslatef(-cmul, cs, 0.0f);
	}

	/* back */

	glTranslatef(-cs, -cmul, -cmul);

	for(i=0; i<cube_count; i++){
		for(j=0; j<cube_count; j++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_QUADS);
				v = -((bars_buffer[0][(i * 50) + (j * 10) + 1] / 2.0f) * m + 0.5f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,     v + 2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v+2.0f);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v+2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v+2.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,     v); glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,     v);    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,     v); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,     v);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,     v); glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  v+2.0f);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  v+2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,     v);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f,     v); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f,     v);    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v+2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v+2.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,     v); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,     v);    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v+2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v+2.0f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,     v); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v+2.0f);    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v+2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,     v);
			glEnd();
			glTranslatef(cs, 0.0f, 0.0f);
		}
		glTranslatef(-cmul, cs, 0.0f);
	}


	/* left */

	glRotatef(90.0f,0.0f,1.0f,0.0f);
	glTranslatef(-cmul, -cmul, 0.0f);

	for(i=0; i<cube_count; i++){
		for(j=0; j<cube_count; j++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_QUADS);
				v = -((bars_buffer[0][(i * 50) + (j * 10) + 2] / 2.0f) * m + 0.5f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v+2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v+2.0f);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v+2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v+2.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,     v); glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,     v);    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,     v); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,     v);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,     v); glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  v+2.0f);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  v+2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,     v);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f,     v); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f,     v);    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v+2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v+2.0f);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,     v); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,     v);    glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v+2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v+2.0f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,     v); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v+2.0f);    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v+2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,     v);
			glEnd();
			glTranslatef(cs, 0.0f, 0.0f);
		}
		glTranslatef(-cmul, cs, 0.0f);
	}

	/* right */

	glRotatef(0.0f,0.0f,1.0f,0.0f);
	glTranslatef(cs, -cmul, +cmul);

	for(i=0; i<cube_count; i++){
		for(j=0; j<cube_count; j++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_QUADS);
				v = ((bars_buffer[0][(i * 50) + (j * 10) + 3] / 2.0f) * m + 0.5f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f);
			glEnd();
			glTranslatef(cs, 0.0f, 0.0f);
		}
		glTranslatef(-cmul, cs, 0.0f);
	}

	/* top */

	//glRotatef(-90.0f,0.0f,1.0f,0.0f);
	glRotatef(-90.0f,1.0f,0.0f,0.0f);
	glTranslatef(-cs, 0.0f, 0.0f);

	for(i=0; i<cube_count + 1; i++){
		for(j=0; j<cube_count + 1; j++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_QUADS);
				v = ((bars_buffer[0][(i * 50) + (j * 10) + 4] / 2.0f) * m + 0.5f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f);
			glEnd();
			glTranslatef(cs, 0.0f, 0.0f);
		}
		glTranslatef(-(cmul + cs), cs, 0.0f);
	}

	/* bottom */

	//glRotatef(-90.0f,0.0f,1.0f,0.0f);
	glRotatef(-180.0f,1.0f,0.0f,0.0f);
	glTranslatef(cs, cs + cs, cmul);

	for(i=1; i<cube_count; i++){
		for(j=1; j<cube_count; j++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[0]);
			glBegin(GL_QUADS);
				v = ((bars_buffer[0][(i * 50) + (j * 10) + 5] / 2.0f) * m + 0.5f);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f);
				glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f);
				glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);
				glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, v-2.0f); glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  v);
				glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, v-2.0f); glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  v);    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  v);    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, v-2.0f);
			glEnd();
			glTranslatef(cs, 0.0f, 0.0f);
		}
		glTranslatef(-(float)((cube_count - 1) * cube_space), cs, 0.0f);
	}



	if(fabs(fft_buffer[0][20]) > last_peak_x)
	{
		last_peak_x = fabs(fft_buffer[0][20]) * 6.0f;
		rside_x = rand() > (RAND_MAX / 2) ? 1 : 0;
	}

	if(fabs(fft_buffer[0][2]) > last_peak_y)
	{
		last_peak_y = fabs(fft_buffer[0][2]) * 6.0f;
		rside_y = rand() > (RAND_MAX / 2) ? 1 : 0;
	}

	if(fabs(fft_buffer[0][500]) > last_peak_z)
	{
		last_peak_z = fabs(fft_buffer[0][500]) * 4.0f;
		rside_z = rand() > (RAND_MAX / 2) ? 1 : 0;
	}

	if(rside_x)
		xrot += last_peak_x;
	else
		xrot -= last_peak_x;

	if(rside_y)
		yrot += last_peak_y;
	else
		yrot -= last_peak_y;

	if(rside_z)
		zrot += last_peak_z;
	else
		zrot -= last_peak_z;

	LightDiffuse[0] = min(fabs(last_peak_y) + 0.2f, 1.0f);
	LightDiffuse[1] = min(fabs(last_peak_y) + 0.2f, 1.0f);
	LightDiffuse[2] = min(fabs(last_peak_y) + 0.2f, 1.0f);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  LightDiffuse);

	LightDiffuse[0] = min(fabs(last_peak_z) + 0.2f, 1.0f);
	LightDiffuse[1] = min(fabs(last_peak_z) + 0.2f, 1.0f);
	LightDiffuse[2] = min(fabs(last_peak_z) + 0.2f, 1.0f);
	glLightfv(GL_LIGHT2, GL_DIFFUSE,  LightDiffuse);

	last_peak_x -= values[4];
	last_peak_y -= values[4];
	last_peak_z -= values[4];

	if(values[0] != 0.0f)
	{
		if(last_peak_y < values[1])
		{
			z += 1.2f;
			if(z > 90.0)z = 90.0;
		}else if(last_peak_y > values[2]){
			z -= 1.0f;

			if(z < -80.0)z = -80;
		}
	}

	if(last_peak_x < 0)last_peak_x = 0;
	if(last_peak_y < 0)last_peak_y = 0;
	if(last_peak_z < 0)last_peak_z = 0;

	return TRUE;
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

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
 *	title			- Title To Appear At The Top Of The Window				*
 *	width			- Width Of The GL Window Or Fullscreen Mode				*
 *	height			- Height Of The GL Window Or Fullscreen Mode			*
 *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
 *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(string title, int width, int height, int bits, BOOL fullscreenflag)
{
	GLuint		PixelFormat;
	WNDCLASS	wc;
	RECT		WindowRect;
	static	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER,
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

	pfd.cColorBits = bits;


	WindowRect.left=(long)0;
	WindowRect.right=(long)width;
	WindowRect.top=(long)0;
	WindowRect.bottom=(long)height;

	fullscreen=fullscreenflag;

	hInstance			= GetModuleHandle(NULL);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= uni("OpenGL");

	if(!RegisterClass(&wc))
	{
		MessageBox(NULL,uni("Failed To Register The Window Class."),uni("ERROR"),MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	//dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	//dwStyle=WS_OVERLAPPEDWINDOW;

	//AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);


	if (!(hWnd=CreateWindowEx(	WS_EX_APPWINDOW,
								uni("OpenGL"),
								title,
								WS_CHILD,
								0, 0,
								WindowRect.right-WindowRect.left,
								WindowRect.bottom-WindowRect.top,
								window_vis,
								NULL,
								hInstance,
								NULL)))
	{
		KillGLWindow();
		MessageBox(NULL,uni("Window Creation Error."),uni("ERROR"),MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	window_vis_in = hWnd;

	if (!(hDC=GetDC(hWnd)))
	{
		KillGLWindow();

		return FALSE;
	}

	if (!(PixelFormat=ChoosePixelFormat(hDC,&pfd)))
	{
		KillGLWindow();
		return FALSE;
	}

	if(!SetPixelFormat(hDC,PixelFormat,&pfd))
	{
		KillGLWindow();
		return FALSE;
	}

	if (!(hRC=wglCreateContext(hDC)))
	{
		KillGLWindow();
		return FALSE;
	}

	if(!wglMakeCurrent(hDC,hRC))
	{
		KillGLWindow();
		return FALSE;
	}

	ShowWindow(hWnd,SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ReSizeGLScene(width, height);

	if (!InitGL())
	{
		KillGLWindow();
		MessageBox(NULL,uni("Initialization Failed."),uni("ERROR"),MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

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

		case WM_CLOSE:
			glvis_uninit();
			return 0;

		case WM_SIZE:
			ReSizeGLScene(LOWORD(lParam),HIWORD(lParam));
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

				if(window_vis_proc)
					return (int)window_vis_proc(GetParent(hWnd), uMsg, wParam, lParam);
			}
			break;

		case WM_KEYDOWN:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDBLCLK:
			if(window_vis_proc)
				return (int)window_vis_proc(GetParent(hWnd), uMsg, wParam, lParam);
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int glvis_init(HWND pwnd)
{
	fullscreen = 0;

	srand(GetTickCount());

	if (!CreateGLWindow(uni("Cube"),640,480,16,fullscreen))
	{
		return -1;
	}
	return 0;
}

int glvis_display(void)
{
	if(gl_lights)glEnable(GL_LIGHTING);
	else         glDisable(GL_LIGHTING);
	DrawGLScene();
	SwapBuffers(hDC);
	SendMessage(window_vis, WM_USER, 0, (LPARAM)&hDC);
	return 0;
}


int glvis_uninit(void)
{
	KillGLWindow();
	return 0;
}

#ifndef GRAPHIC_H
#define GRAPHIC_H

typedef unsigned int Uint;
typedef struct {
	unsigned short r,v,b;
	} Color;

extern const Color BLACK;
extern const Color WHITE;
extern const Color RED;
extern const Color BLUE;
extern const Color GREEN;
extern const Color YELLOW;
extern const Color ORANGE;
extern const Color VIOLET;

#ifdef XMMS_BUILD
inline void setPixelRGB (Uint *buffer, Uint x, Uint y, Color c) ;
inline void getPixelRGB (Uint *buffer, Uint x, Uint y, Color *c) ;
#else
void setPixelRGB (Uint *buffer, Uint x, Uint y, Color c) ;
void getPixelRGB (Uint *buffer, Uint x, Uint y, Color *c) ;
#endif


/*
#ifndef USE_ASM
#define WIDTH 240
#define HEIGHT 180
#else
#define WIDTH 400
#define HEIGHT 300
#endif

#define SIZE HEIGHT*WIDTH
*/

extern unsigned int HEIGHT ;
extern unsigned int WIDTH ;
extern unsigned int SIZEGOOM ;


#endif /*GRAPHIC_H*/

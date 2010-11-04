/* filter.c version 0.7
 * contient les filtres applicable a un buffer
 * creation : 01/10/2000
 *  -ajout de sinFilter()
 *  -ajout de zoomFilter()
 *  -copie de zoomFilter() en zoomFilterRGB(), gérant les 3 couleurs
 *  -optimisation de sinFilter (utilisant une table de sin)
 *	-asm
 *	-optimisation de la procedure de génération du buffer de transformation
 *		la vitesse est maintenant comprise dans [0..128] au lieu de [0..100]
*/


#include "filters.h"
#include "math.h"
#include "graphic.h"
#include <stdlib.h>

/*
#ifdef USE_ASM
#define EFFECT_DISTORS 0.00006515963712f
#else
#define EFFECT_DISTORS 0.00015915963712f
#endif
*/
#ifdef USE_ASM
#define EFFECT_DISTORS 4
#else
#define EFFECT_DISTORS 10
#endif


#ifdef USE_ASM
#include "asm.h"
unsigned int *coeffs ;
Uint *expix1, *expix2;
#endif


int sintable [0xffff] ;
int vitesse = 127;
char theMode = NORMAL_MODE ;
static int middleX , middleY ;
static unsigned char sqrtperte = 16 ;


// retourne x>>s , en testant le signe de x
#ifdef XMMS_BUILD
inline int ShiftRight (int x, const unsigned char s)
#else
int ShiftRight (int x, const unsigned char s)
#endif
{
	if (x<0)
		return -(-x >> s) ;
	else
		return x >> s ;
}

/*
 calculer px et py en fonction de x,y,middleX,middleY et theMode
 px et py indique la nouvelle position (en sqrtperte ieme de pixel) (valeur * 16)
*/
#ifdef XMMS_BUILD
inline void calculatePXandPY (int x, int y, int *px, int *py)
#else
void calculatePXandPY (int x, int y, int *px, int *py)
#endif
{
			// soit O (middleX, middleY)
			// et M (x, y)

			int dist ;

			int vx,vy ; // V = OM
			int fvitesse = vitesse << 4 ;

			// V = OM
			vx = (x - middleX) << 9 ;
			vy = (y - middleY) << 9 ;

			switch (theMode)
			{
			case WAVE_MODE:
				dist = ShiftRight(vx,9) * ShiftRight(vx,9)
					+ ShiftRight(vy,9) * ShiftRight(vy,9) ;
			 	fvitesse *= 
					1024 + ShiftRight (
						sintable [(unsigned short)(0xffff*dist*EFFECT_DISTORS)],
						6) ;
				fvitesse /= 1024 ;
				break ;
			case CRYSTAL_BALL_MODE:
				dist = ShiftRight(vx,9) * ShiftRight(vx,9) + 
					ShiftRight(vy,9) * ShiftRight(vy,9) ;
				fvitesse += (dist * EFFECT_DISTORS >> 10) ;
				break ;
			case AMULETTE_MODE:
				dist = ShiftRight(vx,9) * ShiftRight(vx,9)
					+ ShiftRight(vy,9) * ShiftRight(vy,9) ;
				fvitesse -= (dist * EFFECT_DISTORS >> 4) ;
				break ;
			case SCRUNCH_MODE:
				dist = ShiftRight(vx,9) * ShiftRight(vx,9)
					+ ShiftRight(vy,9) * ShiftRight(vy,9) ;
				fvitesse -= (dist * EFFECT_DISTORS >> 9) ;
				break ;
			}

			if (vx<0)
				*px = (middleX << 4) - (-(vx * fvitesse) >> 16) ;
			else
				*px = (middleX << 4) + ((vx * fvitesse) >> 16) ;

			if (vy<0)
				*py = (middleY << 4) - (-(vy * fvitesse) >> 16) ;
			else
				*py = (middleY << 4) + ((vy * fvitesse) >> 16) ;
}

//#define _DEBUG

#ifdef XMMS_BUILD
inline void setPixelRGB (Uint *buffer, Uint x, Uint y, Color c)
#else
void setPixelRGB (Uint *buffer, Uint x, Uint y, Color c)
#endif
{
	#ifdef _DEBUG
	if ( x+y*WIDTH>=SIZEGOOM )
	{
#ifdef XMMS_BUILD
		printf ("setPixel ERROR : hors du tableau... %i, %i\n", x,y) ;
#endif
		exit (1) ;
	}
	#endif
	buffer[ y*WIDTH + x ] = (c.r<<16)|(c.v<<8)|c.b ;
}

#ifdef XMMS_BUILD
inline void getPixelRGB (Uint *buffer, Uint x, Uint y, Color *c)
#else
void getPixelRGB (Uint *buffer, Uint x, Uint y, Color *c)
#endif
{
	register unsigned char *tmp8;

	#ifdef _DEBUG
	if ( x+y*WIDTH>=SIZEGOOM )
	{
#ifdef XMMS_BUILD
		printf ("getPixel ERROR : hors du tableau... %i, %i\n", x,y) ;
#endif
		exit (1) ;
	}
	#endif

#ifdef WORDS_BIGENDIAN
	c->b = *(unsigned char *)(tmp8 = (unsigned char*)(buffer + (x + y*WIDTH)));
    c->r = *(unsigned char *)(++tmp8);
    c->v = *(unsigned char *)(++tmp8);
	c->b = *(unsigned char *)(++tmp8);
			
#else
	/* ATTENTION AU PETIT INDIEN  */
	c->b = *(unsigned char *)(tmp8 = (unsigned char*)(buffer + (x + y*WIDTH)));
	c->v = *(unsigned char *)(++tmp8);
	c->r = *(unsigned char *)(++tmp8);
//	*c = (Color) buffer[x+y*WIDTH] ;
#endif
}

/*===============================================*/
void sinFilter(Uint *pix1,Uint *pix2, Uint cycle,
	Uint mode,
	Uint rate,
	char	lenght, Uint speed)
{
	static char firstTime = 1;
	static int sinTable[257];
	static int *dist ;
	int x,y;
	Color col;
	int diste;
	static unsigned int PREVIOUS_SIZE ;
	
	if (PREVIOUS_SIZE!=SIZEGOOM) {
		PREVIOUS_SIZE = SIZEGOOM ;
		if (dist) free (dist) ;
		firstTime = 1 ;
		middleX = WIDTH / 2 ;
		middleY = HEIGHT - 1;
	}
	
	if (firstTime) {
		firstTime=0;
		
		dist = (int *) malloc(SIZEGOOM*sizeof(int)) ;
		
		for (x=0;x<257;x++) {
			sinTable[x] = (int)(126.0f + 
				126.0f * sin((float)x * 2.0f * 3.1415f / 255.0f));
		}
		
		for (y=0;y<HEIGHT;y++) for (x=0;x<WIDTH;x++) dist[x+y*WIDTH]=(int)sqrt (x*x+y*y) ;
	}

	for (y=0;y<HEIGHT;y++) for (x=0;x<WIDTH;x++) {
		int vx, vy, svx,svy ;
		vx = (x-middleX) ;
		vy = (y-middleY) ;
		
		svx = vx>0 ? vx : -vx ;
		svy = vy>0 ? vy : -vy ;

		diste = dist [svx+svy*WIDTH] << 3 ;

		getPixelRGB (pix1,x,y,&col) ;

		diste = sinTable[(unsigned char)(diste-cycle*speed)] ;

		if (mode==SIN_MUL) {
			col.r = col.r * diste;
			col.r >>= 8;
			col.v = col.v * diste;
			col.v >>= 8;
			col.b = col.b * diste;
			col.b >>= 8;
		}
		if (mode==SIN_ADD) {
			diste = diste * rate >> 7 ;
			col.r = col.r + diste ;
			col.v = col.v + diste ;
			col.b = col.b + diste ;
			if (col.r>255) col.r=255 ;
			if (col.v>255) col.v=255 ;
			if (col.b>255) col.b=255 ;
		}
		setPixelRGB(pix2,x,y,col);
	}
}

/*===============================================================*/
void zoomFilterFastRGB
	(Uint *pix1, Uint *pix2,
	ZoomFilterData *zf)
{
	static char reverse = 0 ; //vitesse inversé..(zoom out)
//	static int perte = 100; // 100 = normal
	static unsigned char pertedec = 8 ;
	static char firstTime = 1;

	Uint x,y;

	static unsigned int PREVIOUS_SIZE ;
	
#ifdef USE_ASM
	expix1 = pix1 ;
	expix2 = pix2 ;
#else
	Color couleur;
	Color col1,col2,col3,col4;
	Uint position ;
	static unsigned int *pos10 ;
	static unsigned int *c1,*c2,*c3,*c4;
#endif

	if (PREVIOUS_SIZE!=SIZEGOOM) {
		PREVIOUS_SIZE = SIZEGOOM ;
		#ifndef USE_ASM
		if (c1) free (c1) ;
		if (c2) free (c2) ;
		if (c3) free (c3) ;
		if (c4) free (c4) ;
		if (pos10) free (pos10) ;
		#else
		if (coeffs) free (coeffs) ;
		#endif
		middleX = WIDTH / 2 ;
		middleY = HEIGHT - 1;
		firstTime = 1 ;
	}

	if (zf)
	{
		reverse = zf->reverse ;
		vitesse = zf->vitesse ;
		if (reverse) vitesse = 256 - vitesse ;
		#ifndef USE_ASM
		sqrtperte = zf->sqrtperte ;
		#endif
		pertedec = zf->pertedec ;
		middleX = zf->middleX ;
		middleY = zf->middleY ;
		theMode = zf->mode ;
	}
	
	if (firstTime || zf) {
	
		// generation d'une table de sinus
		if (firstTime) {
			unsigned short us ;

			firstTime = 0;
			#ifdef USE_ASM
			coeffs = (unsigned int *) malloc (SIZEGOOM*2*sizeof(unsigned int)) ;
			#else
			pos10 = (unsigned int *) malloc (SIZEGOOM*sizeof(unsigned int)) ;
			c1 = (unsigned int *) malloc (SIZEGOOM*sizeof(unsigned int)) ;
			c2 = (unsigned int *) malloc (SIZEGOOM*sizeof(unsigned int)) ;
			c3 = (unsigned int *) malloc (SIZEGOOM*sizeof(unsigned int)) ;
			c4 = (unsigned int *) malloc (SIZEGOOM*sizeof(unsigned int)) ;
			#endif
			for (us=0; us<0xffff; us++) {
				sintable [us] = (int)(1024.0f * sin (us*2*3.31415f/0xffff)) ;
			}
		}

		// generation du buffer
		for (y=0;y<HEIGHT;y++) for (x=0;x<WIDTH;x++) {
			int px,py;
			unsigned char coefv,coefh;

			// calculer px et py en fonction de x,y,middleX,middleY et theMode
			calculatePXandPY (x,y,&px, &py) ;
						
			if ( (py<0) || (px<0)
				|| (py>=(HEIGHT-1)*sqrtperte) || (px>=(WIDTH-1)*sqrtperte)) {
				
				#ifdef USE_ASM
				coeffs[(y*WIDTH+x)*2]=0 ;
				coeffs[(y*WIDTH+x)*2+1]=0;
				#else
				pos10[y*WIDTH+x]=0 ;
				c1[y*WIDTH+x] = 0 ;
				c2[y*WIDTH+x] = 0 ;
				c3[y*WIDTH+x] = 0 ;
				c4[y*WIDTH+x] = 0 ;
				#endif
			}
			else {
				int npx10 ;
				int npy10 ;

				npx10 = (px/sqrtperte) ;
				npy10 = (py/sqrtperte) ;

				coefh = px - npx10 * sqrtperte ;
				coefv = py - npy10 * sqrtperte ;

				#ifdef USE_ASM
				coeffs[(y*WIDTH+x)*2] = (npx10 + WIDTH * npy10) * 4;
				
				if (!(coefh || coefv))
					coeffs[(y*WIDTH+x)*2+1] = sqrtperte*sqrtperte-1 ;
				else
					coeffs[(y*WIDTH+x)*2+1] = (sqrtperte-coefh) * (sqrtperte-coefv) ;
				coeffs[(y*WIDTH+x)*2+1] |= (coefh * (sqrtperte-coefv)) << 8 ;
				coeffs[(y*WIDTH+x)*2+1] |= ((sqrtperte-coefh) * coefv) << 16 ;
				coeffs[(y*WIDTH+x)*2+1] |= (coefh * coefv)<<24 ;
				#else
				pos10[y*WIDTH+x]= npx10 + WIDTH * npy10 ;

				if (!(coefh || coefv))
					c1[y*WIDTH+x] = sqrtperte*sqrtperte-1 ;
				else
					c1[y*WIDTH+x] = (sqrtperte-coefh) * (sqrtperte-coefv) ;
				c2[y*WIDTH+x] = coefh * (sqrtperte-coefv) ;
				c3[y*WIDTH+x] = (sqrtperte-coefh) * coefv ;
				c4[y*WIDTH+x] = coefh * coefv ;
				#endif
			}
		}
	}
	
	#ifdef USE_ASM
		// printf ("%i , %i\n", mmx_zoom () , pos10) ;
		mmx_zoom () ;
	#else
	for (position=0; position<SIZEGOOM; position++)
	{
		getPixelRGB(pix1,pos10[position],0,&col1);
		getPixelRGB(pix1,pos10[position]+1,0,&col2);
		getPixelRGB(pix1,pos10[position]+WIDTH,0,&col3);
		getPixelRGB(pix1,pos10[position]+WIDTH+1,0,&col4);

		couleur.r = col1.r * c1[position]
			+ col2.r * c2[position]
			+ col3.r * c3[position]
			+ col4.r * c4[position];
		couleur.r >>= pertedec ;

		couleur.v = col1.v * c1[position]
			+ col2.v * c2[position]
			+ col3.v * c3[position]
			+ col4.v * c4[position];
		couleur.v >>= pertedec ;

		couleur.b = col1.b * c1[position]
			+ col2.b * c2[position]
			+ col3.b * c3[position]
			+ col4.b * c4[position];
		couleur.b >>= pertedec ;
    
		setPixelRGB(pix2,position,0,couleur);
	}
	#endif
}


void pointFilter(Uint *pix1, Color c,float t1, float t2, float t3, float t4,
Uint cycle)
{
	Uint x = (Uint)((int)middleX + (int)(t1*cos((float)cycle/t3)));
	Uint y = (Uint)((int)middleY + (int)(t2*sin((float)cycle/t4)));
	if ((x>1) && (y>1) && (x<WIDTH-2) && (y<HEIGHT-2)) {
		setPixelRGB(pix1, x+1, y, c);
		setPixelRGB(pix1, x, y+1, c);
		setPixelRGB(pix1, x+1, y+1, WHITE);
		setPixelRGB(pix1, x+2, y+1, c);
		setPixelRGB(pix1, x+1, y+2, c);
	}
}



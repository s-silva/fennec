#ifndef FILTERS_H
#define FILTERS_H

#include "graphic.h"
#include "math.h"

typedef struct
{
	int vitesse ;
	unsigned char pertedec ;
	unsigned char sqrtperte ;
	int middleX,middleY ;
	char reverse ;
	char mode ;
} ZoomFilterData ;

#define NORMAL_MODE 0
#define WAVE_MODE 1
#define CRYSTAL_BALL_MODE 2
#define SCRUNCH_MODE 3
#define AMULETTE_MODE 4

void pointFilter(Uint *pix1, Color c,
	float t1, float t2, float t3, float t4,
	Uint cycle);

/* filtre de zoom :
 le contenu de pix1 est copie dans pix2, avec l'effet appliqué
 midx et midy represente le centre du zoom
*/
void zoomFilter(Uint *pix1, Uint *pix2, Uint middleX, Uint middleY);
void zoomFilterRGB(Uint *pix1, Uint *pix2, Uint middleX, Uint middleY);
void zoomFilterFastRGB
	(Uint *pix1, Uint *pix2,
	ZoomFilterData *zf);


/* filtre sin :
 le contenu de pix1 est copie dans pix2, avec l'effet appliqué
 cycle est la variable de temps.
 mode vaut SIN_MUL ou SIN_ADD
 rate est le pourcentage de l'effet appliqué
 lenght : la longueur d'onde (1..10) [5]
 speed : la vitesse (1..100) [10]
*/
void sinFilter(Uint *pix1,Uint *pix2,
	Uint cycle,
	Uint mode,
	Uint rate,
	char	lenght,
	Uint speed);
#define SIN_MUL 1
#define SIN_ADD 2

#endif

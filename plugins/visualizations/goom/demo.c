#include <windows.h>
#include <stdlib.h>
#include "tinyptc.h"
#include <math.h>
#include "goom.h"
#include "graphic.h"
#include "filters.h"


#define STOP_SPEED 128


unsigned int POINT_WIDTH ;
unsigned int POINT_HEIGHT ;
HWND         hwnd;
HDC          hdc;

static void jeko_init(void);
static void jeko_cleanup(void);
static void jeko_playback_start(void);
static void jeko_playback_stop(void);
void jeko_update_config ();

/*-----------------------------------------------------
SHARED DATA
-------------------------------------------------------*/

Uint *pixel ;
Uint *back ;
Uint *p1,*p2,*tmp;
Uint x,y;
Uint cycle;
Uint debut, fin;
char fast ;



t_config extern_config = {320,240} ;

int goom_main(HWND hwndx, HDC dcx)
{
	hwnd = hwndx;
	hdc  = dcx;

    jeko_init();
	jeko_playback_start();
	return 1;
}


/*======================*/
/*===============================*/

void jeko_init(void)
{	
	jeko_update_config () ;

	/* ouverture */
	if (!ptc_open_ex("What a GOOM!",WIDTH,HEIGHT, hwnd, hdc)) exit (1);

	pixel = (Uint *) malloc (SIZEGOOM * sizeof(Uint)) ;
	back = (Uint *) malloc (SIZEGOOM * sizeof(Uint)) ;
}

static void jeko_cleanup(void)
{
	/* fermeture */
	ptc_close();
	free (pixel) ;
	free (back) ;
}

static void jeko_playback_start(void)
{
	cycle = 0;

	/* remplissage de la grille */
	for (y=0;y<HEIGHT;y++) for (x=0;x<WIDTH;x++) {
		if ( (x/8)%2 == (y/8)%2 )
			setPixelRGB(pixel,x,y,BLACK);
		else
			setPixelRGB(pixel,x,y,VIOLET);
	}
	
	/* boucle */
	p1=pixel;
	p2=back;

	srand ((unsigned int)cycle * (unsigned int)p1) ;
}

static void jeko_playback_stop(void)
{

}


void jeko_render_pcm(short data[2][512])
{
	static int lockvar = 0 ; // pour empecher de nouveaux changements

	static int goomvar = 0 ; // boucle des gooms
	static int totalgoom = 0 ; // nombre de gooms par seconds
	static int totalgrosgoom = 0 ; // nombre de grosgooms par seconds
	static char goomlimit = 2 ; // sensibilité du goom
	static char grosgoomlimit = 2 ; // sensibilite ajoutée pour gros goom
	
	static int agoom = 0 ; // un goom a eu lieu..
	
	static int loopvar = 0 ; // mouvement des points
	int incvar ; // volume du son
	
	static int speedvar = 0 ; // vitesse des particules

	int accelvar ; // acceleration des particules
	int i ;
	float largfactor ; // elargissement de l'intervalle d'évolution des points
	
	static ZoomFilterData zfd = {128,8,16,1,1,0, WAVE_MODE} ;
	ZoomFilterData *pzfd ;
	
	static int sinrate = 0 ;
	static int sintimeleft = 0 ;
	
	static unsigned int PREVIOUS_SIZE ;
	short shrand;

	/* test if the config has changed, update it if so */
	PREVIOUS_SIZE = SIZEGOOM ;
	jeko_update_config () ;
	if (PREVIOUS_SIZE != SIZEGOOM) {
		PREVIOUS_SIZE = SIZEGOOM ;
		jeko_cleanup () ;

		Sleep(1000);

		jeko_init () ;
		jeko_playback_start () ;
	}

	incvar = 0 ;
	for (i=0;i<512;i++)
	{
		if (incvar<data[0][i]) incvar = data[0][i] ;
	}

	accelvar = incvar / 5000 ;
	if (speedvar>5)
	{
		accelvar-- ;
		if (speedvar>20) accelvar -- ;
		if (speedvar>40) speedvar = 40 ;
	}
	accelvar -- ;
	speedvar += accelvar ;
	
	if (speedvar<0) speedvar=0;

	largfactor = ((float)speedvar / 40.0f + (float)incvar / 50000.0f) / 1.5f ;
	if (largfactor>1.0f) largfactor = 1.0f ;

	for (i = 1 ; i*15 <= speedvar + 15; i ++) {
		loopvar += speedvar + 1 ;
		
		pointFilter(p1, YELLOW,
			((POINT_WIDTH-6.0f)*largfactor+5.0f),
			((POINT_HEIGHT-6.0f)*largfactor+5.0f),
			i*76.0f, 64.0f, loopvar+i*2032);
		pointFilter(p1, ORANGE,
			((POINT_WIDTH/2)*largfactor)/i+10.0f*i,
			((POINT_HEIGHT/2)*largfactor)/i+10.0f*i,
			48.0f, i*40.0f, loopvar/i);
		pointFilter(p1, VIOLET,
			((POINT_HEIGHT/3+5.0f)*largfactor)/i+10.0f*i,
			((POINT_HEIGHT/3+5.0f)*largfactor)/i+10.0f*i,
			i+61.0f, 67.0f, loopvar/i);
		pointFilter(p1, BLACK,
			((POINT_HEIGHT/3)*largfactor+20.0f),
			((POINT_HEIGHT/3)*largfactor+20.0f),
			29.0f, i*33.0f, loopvar/i);
		pointFilter(p1, WHITE,
			(POINT_HEIGHT*largfactor+10.0f*i)/i,
			(POINT_HEIGHT*largfactor+10.0f*i)/i,
			33.0f, 37.0f, loopvar+i*500);
	}

	// par défaut pas de changement de zoom
	pzfd = NULL ;
	
	// diminuer de 1 le temps de lockage
	lockvar -- ;
	if (lockvar==-1) lockvar = 0 ;

	// temps du goom
	agoom -- ;
	if (agoom<0) agoom = 0 ;
	if ((accelvar>goomlimit) || (accelvar<-goomlimit))
	{
		totalgoom ++ ;
		agoom = 5 ;
		
		// changement eventuel de mode
		// if (cycle%1==0)
			switch ((int)(10.0f*rand()/RAND_MAX))
			{
			case 0:
			case 1:
			case 2:
				zfd.mode=WAVE_MODE;
				zfd.vitesse=STOP_SPEED-1;
				zfd.reverse=0;
				break;
			case 3:
			case 4:
				zfd.mode=CRYSTAL_BALL_MODE;
				break;
			case 5:
				zfd.mode=AMULETTE_MODE;
				break;
			case 6:
			case 7:
				zfd.mode=SCRUNCH_MODE;
				break;
			default:
				zfd.mode=NORMAL_MODE;
			}
	}

	// gros goom : on active le filtre sin
	if (((accelvar>goomlimit+grosgoomlimit)
		|| (accelvar<-goomlimit-grosgoomlimit))
		&& (sinrate<40) )
	{
		if (totalgrosgoom<3) {
			sinrate += accelvar * 10 ;
			sintimeleft = 20 ;
		}
		totalgrosgoom ++ ;
	}
	
	// filtre sin activé ? : on diminue son intensité...
	if ((sinrate>0) && (cycle%2==0))
	{
		sinrate -- ;
		if (sinrate==0) {
			zfd.vitesse += 10 ;
			if (zfd.vitesse>STOP_SPEED-3) zfd.vitesse=STOP_SPEED-3 ;
			pzfd = &zfd ;
		}
	}
	
	// ... jusqu'à extinction
	if (sintimeleft==0) {
		if (sinrate!=0) sinrate = sinrate * 4 / 5 ;
	}
	else
		sintimeleft -- ;

	// tout ceci ne sera fait qu'en cas de non-blocage
	if (lockvar==0)
	{
		// reperage de goom (acceleration forte de l'acceleration du volume)
		// -> coup de boost de la vitesse si besoin..
		if ( (accelvar>goomlimit) || (accelvar<-goomlimit) )
		{
			goomvar ++ ;
			if (goomvar%1 == 0)
			{
				int newvit ;
				newvit = STOP_SPEED - speedvar / 2 ;

				// retablir le zoom avant..
				if ((zfd.reverse)&&(cycle%12)) {
					zfd.reverse = 0 ;
					zfd.vitesse = STOP_SPEED-2 ;
					lockvar = 50 ;
				}
				
				// changement de milieu..
				switch (rand()%20)
				{
				case 0:
					zfd.middleY = HEIGHT - 1 ;
					zfd.middleX = WIDTH / 2 ;
					break ;
				case 1:
					zfd.middleX = WIDTH - 1 ;
					break ;
				case 2:
					zfd.middleX = 1 ;
					break ;
				default:
					zfd.middleY = HEIGHT / 2 ;
					zfd.middleX = WIDTH / 2 ;
				}


				if (newvit<zfd.vitesse) // on accelere
				{
					pzfd = &zfd ;
					if ((newvit<STOP_SPEED-8) && (zfd.vitesse<STOP_SPEED-6) && (cycle%3==0))
					{
						zfd.vitesse = STOP_SPEED-1 ;
						zfd.reverse = ! zfd.reverse ;
					}
					else {
						zfd.vitesse = (newvit + zfd.vitesse * 3) / 4 ;
					}

					lockvar = 50 ;
				}
			}
		}
		
		// mode mega-lent
		if (rand()%1200==0) {

			pzfd = &zfd ;
			zfd.vitesse = STOP_SPEED-1 ;
			zfd.pertedec = 8 ;
			zfd.sqrtperte=16 ;
			goomvar = 1 ;
			lockvar = 100 ;
		}
	}
	
	// gros frein si la musique est calme
	if ((speedvar<1) && (zfd.vitesse<STOP_SPEED-4) && (cycle%16==0))
	{
		pzfd = &zfd ;
		zfd.vitesse += 3 ;
		zfd.pertedec = 8 ;
		zfd.sqrtperte=16 ;
		goomvar = 0 ;
	}
	
	// baisser regulierement la vitesse...
	if ( (cycle%73==0) && (zfd.vitesse<STOP_SPEED-5)) {
		pzfd = &zfd ;
		zfd.vitesse ++ ;
	}
	
	// arreter de decrémenter au bout d'un certain temps
	if ((cycle%101==0) && (zfd.pertedec==7))
	{
		pzfd = &zfd ;
		zfd.pertedec=8 ;
		zfd.sqrtperte=16 ;
	}
	
	// passage du zoomFilter..
	if (pzfd) {

	}
	zoomFilterFastRGB (p1, p2, pzfd) ;

	// si on est dans un goom : afficher l'oscillo...
	if (agoom>0)
	{
		Uint *p ;
		Uint vcolor, rcolor ;
		if (agoom>1) {
			p=p2;
			switch (agoom) {
			case 4 :
				vcolor = 0x0014ff82;
				rcolor = 0x00ff4228;
				break ;
			case 3 :
				vcolor = 0x0014cc82;
				rcolor = 0x00cc4228;
				break ;
			case 2 :
				vcolor = 0x00ddffdd ;
				rcolor = 0x00ffdddd ;
				break ;
			default :
				vcolor = 0x0066ee66 ;
				rcolor = 0x00ee6666 ;
			}
		}
		else {
			p=p1 ;
			vcolor = 0x0014aa82;
			rcolor = 0x00aa4228;
		}

		for (i=0;i<512;i++)
		{
			unsigned int plot ;

			shrand = (short)rand();

			#ifndef USE_ASM
				plot=i*WIDTH/512 + (HEIGHT/4+data[0][i]/2000)*WIDTH ;
				if (plot<SIZEGOOM)
					p[plot] = vcolor;
				plot=i*WIDTH/512 + (HEIGHT*3/4-data[1][i]/2000)*WIDTH ;
				if (plot<SIZEGOOM)
					p[plot] = rcolor;
			#else
				plot=i*WIDTH/512 + (HEIGHT/4+data[0][i]/1600)*WIDTH ;
				if (plot<SIZEGOOM-1) {
					p[plot] = vcolor;
					p[plot+1] = vcolor;
				}
				plot=i*WIDTH/512 + (HEIGHT*3/4-data[1][i]/1600)*WIDTH ;
				if (plot<SIZEGOOM-1) {
					p[plot] = rcolor;
					p[plot+1] = rcolor;
				}
			#endif

		}
	}

	// si l'intensité du filter sin est > 0 : faire un passage...
#ifndef USE_ASM
	if (sinrate>0) {
		sinFilter(p2, p1, cycle, SIN_ADD, sinrate, 5 , 17);
		ptc_update (p1) ;
		tmp=p1;
		p1=p2;
		p2=tmp;
	}
#else
	if (sinrate>0) {
		sinFilter(p2, p1, cycle, SIN_ADD, sinrate , 5 , 17);
		sinrate=0;
		ptc_update (p1) ;
	}
#endif
	else {
		ptc_update (p2) ;
		tmp=p1;
		p1=p2;
		p2=tmp;
	}

	// affichage et swappage des buffers..
	cycle++;
	
	// tous les 100 cycles : vérifier si le taux de goom est correct
	// et le modifier sinon..
	if (!(cycle%100))
	{
		if (totalgoom>15)
		{
			goomlimit ++ ;
		}
		else
		{
			if ((totalgoom==0) && (goomlimit>1))
			{
				goomlimit -- ;
			}
		
			if (totalgrosgoom>2)
			{
				grosgoomlimit ++ ;
			}
		}
		if ((totalgrosgoom==0) && (goomlimit>2))
		{
			grosgoomlimit -- ;
		}
		
		totalgoom = 0 ;
		totalgrosgoom = 0 ;
	}

	ptc_draw();
}

void jeko_update_config ()
{
	extern_config.xres = 320;
	extern_config.yres = 240;

	WIDTH = extern_config.xres ;
	HEIGHT = extern_config.yres ;
	SIZEGOOM = HEIGHT * WIDTH ;

	POINT_WIDTH = 2 * WIDTH / 5 ;
	POINT_HEIGHT = 2 * HEIGHT / 5 ;
}

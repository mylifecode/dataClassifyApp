#include "BloodEffuse.h"
#include "stdlib.h"
#include <math.h>
#define IX(i,j) ((i)+(H+2)*(j))
#define SWAP(x0,x) {float * tmp=x0;x0=x;x=tmp;}
#define FOR_EACH_CELL for ( i=1 ; i<=W ; i++ ) { for ( j=1 ; j<=H ; j++ ) {
#define END_FOR }}

BloodEffuse::BloodEffuse()
{
	W = 62;

	H = 62;

	int size = (W+2)*(H+2);

	dens		= (float *) malloc ( size*sizeof(float) );

	dens_prev	= (float *) malloc ( size*sizeof(float) );

	clear_data ();

	m_actived = false;

	lastupdatetime = 0;

	m_speed = 1.0f;
}

float BloodEffuse::GetCurrDensity(int x, int y)
{
	float density = dens[IX(x,y)];
	float densprev = dens_prev[IX(x,y)];
	
	float densfinal = advancepercent*density+(1-advancepercent)*densprev;

	return densfinal;
}

void  BloodEffuse::setSpeed(float speed)
{
	m_speed = speed;
	if(m_speed < 1) m_speed = 1;
	if(m_speed > 1000) m_speed = 1000;
}
void BloodEffuse::clear_data ( void )
{
	int i, size=(W+2)*(H+2);

	for ( i=0 ; i<size ; i++ )
		dens[i] = dens_prev[i] = 0.0f ;
}

BloodEffuse::~BloodEffuse()
{
	if ( dens ) free ( dens );
	if ( dens_prev ) free ( dens_prev );
}

static bool s_istestinitialized = false;


void BloodEffuse::addcenterdensity(float centerdensity)
{
	int i, size = (W+2)*(H+2);

	for ( i=0 ; i<size ; i++ )
		dens[i] =0.0f;

	float initradius = 5.0f;

	float centerx = (W+2)*0.5f;
	float centery = (H+2)*0.5f;
	

	for(int r = 0 ; r < 6; r++)
	{
		float cx = centerx+(rand()%7-4)*3;
		float cy = centery+(rand()%7-4)*3;
		int startx = cx-initradius-1;
		int starty = cy-initradius-1;
		int endx = cx+initradius+1;
		int endy = cy+initradius+1;

		if(startx < 0) 
			startx = 0;
		if(endx >= W)
			endx = W-1;

		if(starty < 0) 
			starty = 0;
		if(endy >= H) 
			endy = H-1;

		for(int y = starty ; y <= endy ; y++)
		{
			for(int x = startx ; x <= endx ; x++)
			{
				float dx = x-centerx;

				float dy = y-centery;

				float radius = sqrtf(dx*dx+dy*dy);

				float percent = radius / initradius;

				if(percent > 1)
					percent = 1;

				if(percent < 0)
					percent = 0;

				float density = (1-percent)*centerdensity;

				if(density > dens[IX(x,y)])
					dens[IX(x,y)] = density;
			}
		}
	}
	
	m_actived = true;
	m_centerx = centerx;//startx;
	m_centery = centery;//starty;

	Ogre::Timer * timer = Ogre::Root::getSingleton().getTimer();
	lastupdatetime = (float)timer->getMilliseconds();

	advancepercent = 0;

	startblooddiffusetime = lastupdatetime;
}

static int framecount = 0;

void BloodEffuse::frameUpdate()
{
	if (m_actived == false)
		return;

	float updateinterval = 1000.0f / m_speed;

	Ogre::Timer * timer = Ogre::Root::getSingleton().getTimer();

	float currtime = (float)timer->getMilliseconds();

	while(currtime - lastupdatetime >= updateinterval)
	{
		static int seed = 0;
		seed++;
		srand(seed);

		int size = (W+2)*(H+2);
		
		for (int i = 0 ; i < size ; i++ )
			dens_prev[i] = dens[i];

		for(size_t y = 1 ; y <= H ; y++)
		{
			for(size_t x = 1 ; x <= W ; x++)
			{
				float aroundmin = FLT_MAX;

				float arounddist = 0;

				int   minx = 0;

				int   miny = 0;

				float cdens = dens[IX(x , y)];

				cdens = cdens * (0.5f+0.5f * float(rand() % 10) / 10.0f);

				for(int dy = -1 ; dy <= 1 ; dy++)
				{
					for(int dx = -1 ; dx <= 1 ; dx++)
					{
						float arounddens = dens[IX(x+dx,y+dy)];

						float multifactor = 1.0f / sqrtf(dx*dx+dy*dy);

						arounddens = cdens*(1-multifactor)+arounddens*(multifactor);

						if(arounddens < cdens && (dx != 0 || dy !=0))
						{
							float dd = cdens-arounddens;

							float maxdiffuse = dd*0.5f;

							float diffuse = maxdiffuse * 1.2f;

							float attunation = 1.0f;

							dens[IX(x+dx,y+dy)] += diffuse * attunation;

							dens[IX(x,y)] -= diffuse * attunation;
						}
					}
				}
			}
		}
		lastupdatetime += updateinterval;
		//advancepercent = 0;
	}
	advancepercent = (currtime-lastupdatetime) / updateinterval;

	if(advancepercent < 0) advancepercent = 0;

	if(advancepercent > 1) advancepercent = 1;

	if(currtime-startblooddiffusetime > 20000.0f / m_speed)
		m_actived = false;
}



#ifndef _SMOKESOLVER_
#define  _SMOKESOLVER_
#include <ogre.h>
class BloodEffuse
{
public:
	BloodEffuse();

	~BloodEffuse();

	void clear_data();

	void frameUpdate();

	void addcenterdensity(float density);

	float GetCurrDensity(int x, int y);
	
	void  setSpeed(float speed);

	bool m_actived;

	int  m_centerx;
	
	int  m_centery;
	
	size_t  W;

	size_t  H;
	
	float m_speed;

	float * dens;

	float * dens_prev;

	float * denssolid;
	float * composedens;

	float lastupdatetime;

	float advancepercent;
	
	float startblooddiffusetime;
};
#endif
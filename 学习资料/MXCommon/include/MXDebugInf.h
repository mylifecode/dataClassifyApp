#ifndef __RHDEBUGINF__
#define __RHDEBUGINF__

#include <fstream>
#include <string>

#include <tchar.h> 
#include <wchar.h> 
#include <string.h>
#include <windows.h>
#include "MXCommon.h"
#include "Ogre.h"
class MXCOMMON_API  MXDebugInf
{

public:
	static MXDebugInf *instance(void);
	void output(std::string inf );
	void output(Ogre::Real mReal,std::string inf="");
	void output(const Ogre::Vector3 vec3,std::string inf="");
	void output(const Ogre::Vector2 vec2,std::string inf="");
	void output(const Ogre::Vector4 vec4,std::string inf="");
	void output(const Ogre::Quaternion quater,std::string inf="");	
private:
	std::string GetStrCurrentTime();

	std::ofstream  m_Ofstream;
	MXDebugInf(void) ;
	virtual~MXDebugInf(void);

};


#endif

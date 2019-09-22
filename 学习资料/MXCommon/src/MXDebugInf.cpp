#include "stdafx.h"
#include "MXDebugInf.h"
#include <sstream>
MXDebugInf *MXDebugInf::instance(void)
{
	static MXDebugInf tmp ;
	return &tmp;
}

void MXDebugInf::output(std::string inf )
{
	////打印调用线程
	/*int id = GetCurrentThreadId();
	std::stringstream ss;
	ss<<id ;*/
	std::string ss=GetStrCurrentTime();
	m_Ofstream.open( "debugInf.txt" ,std::ios::out | std::ios::app );
	m_Ofstream<<ss.c_str()<<"  "<<inf.c_str()<<"\n";   //, inf.size());
	m_Ofstream.close();
}

void MXDebugInf::output(Ogre::Real mReal,std::string inf/* ="" */)
{
	std::string ssTime=GetStrCurrentTime();
	std::stringstream ss;
	ss<<mReal;
	m_Ofstream.open( "debugInf.txt" ,std::ios::out | std::ios::app );
	m_Ofstream<<ssTime.c_str()<<"  "<<inf.c_str()<<"  "<<ss.str()<<"\n";
	m_Ofstream.close();
}

void MXDebugInf::output(const Ogre::Vector2 vec2,std::string inf/* ="" */)
{
	std::string ssTime=GetStrCurrentTime();
	std::stringstream ss;
	ss<<"x:"<<vec2.x<<"  y:"<<vec2.y;
	m_Ofstream.open( "debugInf.txt" ,std::ios::out | std::ios::app );
	m_Ofstream<<ssTime.c_str()<<"  "<<inf.c_str()<<"  "<<ss.str()<<"\n";
	m_Ofstream.close();
}

void MXDebugInf::output(const Ogre::Vector3 vec3,std::string inf/* ="" */)
{
	std::string ssTime=GetStrCurrentTime();
	std::stringstream ss;
	ss<<"x:"<<vec3.x<<"  y:"<<vec3.y<<"  z:"<<vec3.z;
	m_Ofstream.open( "debugInf.txt" ,std::ios::out | std::ios::app );
	m_Ofstream<<ssTime.c_str()<<"  "<<inf.c_str()<<"  "<<ss.str()<<"\n";
	m_Ofstream.close();
}

void MXDebugInf::output(const Ogre::Vector4 vec4,std::string inf/* ="" */)
{
	std::string ssTime=GetStrCurrentTime();
	std::stringstream ss;
	ss<<"x:"<<vec4.x<<"  y:"<<vec4.y<<"  z:"<<vec4.z<<"  w:"<<vec4.w;
	m_Ofstream.open( "debugInf.txt" ,std::ios::out | std::ios::app );
	m_Ofstream<<ssTime.c_str()<<"  "<<inf.c_str()<<"  "<<ss.str()<<"\n";
	m_Ofstream.close();
}

void MXDebugInf::output(const Ogre::Quaternion quater,std::string inf/* ="" */)
{
	std::string ssTime=GetStrCurrentTime();
	std::stringstream ss;
	ss<<"x:"<<quater.x<<"  y:"<<quater.y<<"  z:"<<quater.z<<"  w:"<<quater.w;
	m_Ofstream.open( "debugInf.txt" ,std::ios::out | std::ios::app );
	m_Ofstream<<ssTime.c_str()<<"  "<<inf.c_str()<<"  "<<ss.str()<<"\n";
	m_Ofstream.close();
}



std::string MXDebugInf::GetStrCurrentTime()
{
	SYSTEMTIME sys; 
	GetLocalTime( &sys );
	std::stringstream ss;
	ss<<"当前时间："<<sys.wHour<<":"<<sys.wMinute<<":"<<sys.wSecond<<":"<<sys.wMilliseconds;
	return ss.str();
}

MXDebugInf::MXDebugInf(void) 
{	
	m_Ofstream.open( "debugInf.txt" );
	m_Ofstream<< "MXDebugInf  构造"<<"\n";
	m_Ofstream.close();
}

MXDebugInf::~MXDebugInf(void)
{
	m_Ofstream.open("debugInf.txt",std::ios::out | std::ios::app );
	m_Ofstream<< "MXDebugInf  析构"<<"\n";
	m_Ofstream.close();
}

#include "MxSoundManager.h"
#include <fstream>
#include <algorithm>

#pragma comment(lib,"Winmm.lib")

MxSoundManager::MxSoundManager(void)
{
	//m_pDevice = audiere::AudioDevicePtr(audiere::OpenDevice("directsound"));
	::BASS_Init(-1, 44100, 0, 0, 0);
	m_DeviceVolume = ::BASS_GetVolume();
}

MxSoundManager::~MxSoundManager(void)
{
	Clear();
	::BASS_Free();
}

MxSoundManager* MxSoundManager::GetInstance()
{
	static MxSoundManager soundManager;
	return &soundManager;
}

bool MxSoundManager::Play(const std::string& fileName,bool stopOldSound)
{
	//first stop all sound not this stream
	HSTREAM PlayStreamPtr = 0;

	std::map<std::string , HSTREAM>::iterator itor = m_StreamInPlay.begin();
	while(itor != m_StreamInPlay.end())
	{
		if(itor->first != fileName)
		{
		   HSTREAM StreamPtr = itor->second;
		   if(StreamPtr != NULL )
			  ::BASS_ChannelStop(StreamPtr);
		}
		else
		{
			PlayStreamPtr = itor->second;
		}
		itor++;
	}

	if(PlayStreamPtr == NULL)//already in play list
	{
		PlayStreamPtr = BASS_StreamCreateFile(false, fileName.c_str() , 0, 0, BASS_SAMPLE_LOOP);//audiere::OutputStreamPtr(audiere::OpenSound(m_pDevice, fileName.c_str(), false));;//audiere::OutputStreamPtr(audiere::OpenSound(m_pDevice, fileName.c_str(), false));
		
		if(PlayStreamPtr != NULL)
		{
		   m_StreamInPlay.insert(std::make_pair(fileName , PlayStreamPtr));
		}
	}

	if(PlayStreamPtr != 0)
	{
	   ::BASS_ChannelPlay(PlayStreamPtr,false);
	}

	return true;
}


void MxSoundManager::StopSound(const std::string& fileName)
{
	std::map<std::string , HSTREAM>::iterator itor = m_StreamInPlay.find(fileName);
	
	if(itor != m_StreamInPlay.end())
	{
		HSTREAM StreamPtr = itor->second;
		
		if(StreamPtr != NULL )
		   ::BASS_ChannelStop(StreamPtr);
	}
}

void MxSoundManager::StopAllSound()
{
	std::map<std::string , HSTREAM>::iterator itor = m_StreamInPlay.begin();

	while(itor != m_StreamInPlay.end())
	{
		HSTREAM StreamPtr = itor->second;

		if(StreamPtr != NULL )
			::BASS_ChannelStop(StreamPtr);

		itor++;
	}
}

void MxSoundManager::Clear()
{
	//stop all sound
	std::map<std::string , HSTREAM>::iterator itor = m_StreamInPlay.begin();
	while(itor != m_StreamInPlay.end())
	{
		HSTREAM StreamPtr = itor->second;
		if(StreamPtr != NULL )
		{
			::BASS_ChannelStop(StreamPtr);
			::BASS_StreamFree(StreamPtr);
		}
		itor++;
	}
	m_StreamInPlay.clear();
	//
}
void MxSoundManager::MuteDevice(bool mute)
{
	std::map<std::string , HSTREAM>::iterator itor = m_StreamInPlay.begin();

	while(itor != m_StreamInPlay.end())
	{
		HSTREAM StreamPtr = itor->second;

		if(StreamPtr != NULL )
		{
			if(mute)
			   BASS_ChannelSetAttribute(StreamPtr, BASS_ATTRIB_VOL, 0);
			else
			   BASS_ChannelSetAttribute(StreamPtr, BASS_ATTRIB_VOL, 1);
		}

		itor++;
	}
}
/*
bool MxSoundManager::IsPlaying(const std::string& fileName)
{
	PlayState ps = GetSoundState(fileName);

	return ps == PS_Playing;
}


bool MxSoundManager::CheckCmdError(MCIERROR error)
{
	if(error)
	{
		char buffer[256] = {0};
		mciGetErrorStringA(error,buffer,256);
		MessageBoxA(NULL,buffer,"open error:",MB_OK);
		return true;
	}

	return false;
}

int MxSoundManager::ExtractFileNameStartIndex(const std::string& fullFileName)
{
	int index = 0;
	int i1 = fullFileName.find_last_of('\\');
	int i2 = fullFileName.find_last_of('/');

	if(i1 > 0)
		++i1;
	if(i2 > 0)
		++i2;

	index = max(i1,i2);
	if(index < 0)
		index = 0;
	else if(index == fullFileName.size())
		index = -1;

	return index;
}

MxSoundManager::PlayState MxSoundManager::GetSoundState(const std::string& fileName)
{
	PlayState ps = PS_Error;

	if(FileExists(fileName))
	{
		std::string hashValue = GenHashValue(fileName);
		if(m_loadedSoundSet.find(hashValue) != m_loadedSoundSet.end())
		{
			ps = _GetSoundState(hashValue);
		}
	}

	return ps;
}

MxSoundManager::PlayState MxSoundManager::_GetSoundState(const std::string& hashValue)
{
	PlayState ps = PS_Error;
	std::string cmd;
	char result[32] = {0};

	MCIERROR error = mciSendStringA(("status  " + hashValue + " mode").c_str(),result,sizeof(result) / sizeof(result[0]),NULL);
	if(CheckCmdError(error) == false)
	{
		if(result == std::string("playing"))
			ps = PS_Playing;
		else if(result == std::string("stopped"))
			ps = PS_Stopped;
		else if(result == std::string("paused"))
			ps = PS_Paused;
		else if(result == std::string("not ready"))
			ps = PS_NotReady;
	}

	return ps;
}

void MxSoundManager::Clear()
{
	//stop
	for(std::set<std::string>::iterator itr = m_curPlaySet.begin(); itr != m_curPlaySet.end();++itr)
	{	
		mciSendStringA(("seek " + *itr + " to start ").c_str(),NULL,0,NULL);
	}
	
	//close
	for(std::set<std::string>::iterator itr = m_loadedSoundSet.begin(); itr != m_loadedSoundSet.end();++itr)
	{
		mciSendStringA(("close  " + *itr).c_str(),NULL,0,NULL);
	}

	m_curPlaySet.clear();
	m_loadedSoundSet.clear();
}
*/
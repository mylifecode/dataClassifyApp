#pragma once
#include "MXCommon.h"
#include<string>
#include<map>
#include "bass.h"


/**
	该类主要用于播放音频文件，可支持同时播放多个不同的音频文件，但不支持同时播放相同的音频文件
*/
class MXCOMMON_API MxSoundManager
{
private:
	MxSoundManager(void);
	~MxSoundManager(void);

public:
	enum PlayState
	{
		PS_Playing,
		PS_Stopped,
		PS_NotReady,
		PS_Paused,
		PS_Error
	};

	static MxSoundManager* GetInstance();

	/**
		播放一个音频文件
		fileName：音频文件的名字
		stopOldSound:决定是否停止先前播放的音频。如果为false，如果再次播放先前已经打开的，并且当前正在播放的音频，那么不错任何事情
	*/
	bool Play(const std::string& fileName,bool stopOldSound = false);

	void StopSound(const std::string& fileName);

	void StopAllSound();

	/** 清空所有导入的音频资源 */
	void Clear();

	void MuteDevice(bool mute);
private:
	
	/** 如果发生错误，则返回true */
	//bool CheckCmdError(MCIERROR error);

	//int ExtractFileNameStartIndex(const std::string& fullFileName);

	//PlayState _GetSoundState(const std::string& hashValue);

private:
	
	std::map<std::string , HSTREAM> m_StreamInPlay;
	
	//audiere::AudioDevicePtr m_pDevice;
	float m_DeviceVolume;
};

#pragma once
#include "MXCommon.h"
#include<string>
#include<map>
#include "bass.h"


/**
	������Ҫ���ڲ�����Ƶ�ļ�����֧��ͬʱ���Ŷ����ͬ����Ƶ�ļ�������֧��ͬʱ������ͬ����Ƶ�ļ�
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
		����һ����Ƶ�ļ�
		fileName����Ƶ�ļ�������
		stopOldSound:�����Ƿ�ֹͣ��ǰ���ŵ���Ƶ�����Ϊfalse������ٴβ�����ǰ�Ѿ��򿪵ģ����ҵ�ǰ���ڲ��ŵ���Ƶ����ô�����κ�����
	*/
	bool Play(const std::string& fileName,bool stopOldSound = false);

	void StopSound(const std::string& fileName);

	void StopAllSound();

	/** ������е������Ƶ��Դ */
	void Clear();

	void MuteDevice(bool mute);
private:
	
	/** ������������򷵻�true */
	//bool CheckCmdError(MCIERROR error);

	//int ExtractFileNameStartIndex(const std::string& fullFileName);

	//PlayState _GetSoundState(const std::string& hashValue);

private:
	
	std::map<std::string , HSTREAM> m_StreamInPlay;
	
	//audiere::AudioDevicePtr m_pDevice;
	float m_DeviceVolume;
};

#pragma once
#include "MxCommon.h"
#include <windows.h>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include "SYUserInfo.h"
#include "MxProcessCommunicaMsg.h"
class TiXmlDocument;
class MXCOMMON_API MxProcessCommunicator// : public QObject
{
	//Q_OBJECT
public:
	
	class CommunicatorMsgListener
	{
	public:
		virtual void OnReceiveCommunicatorMessage(MXPCMessageType type , MxProcessCommunicateMsg * msg) = 0;

		virtual void OnReceiveSubModuleTerminate() = 0;
	};

	MxProcessCommunicator(const std::string& pipeName,bool isServer = true);

	~MxProcessCommunicator();

	void  AddMessageListener(MxProcessCommunicator::CommunicatorMsgListener *);

	void  RemoveMessageListener(MxProcessCommunicator::CommunicatorMsgListener *);

	DWORD LaunchProcess(const char * modulename , const char * commandStr, void * shareBuff, int shareByte);

	void  TerminateChildProcess();

	int   ReadFromSharedMemory(void * content, int maxbyte);

	bool  Startup();

	void  SetBufferSize(std::size_t bufferSize) { m_bufferSize = bufferSize; }

	bool SendMessage(MxProcessCommunicateMsg * msg);

	void ProcessMessages(float secondSinceLast , int maxProcessNum = 10);

	/** 仅用于接收端，如果连接断开，那么将停止接受消息，要想再次接受消息，应再次调用此函数 */
	void StartReceiveMessage();

	//signals:
	

private:
	void _ReceiveMessage();

	int WaitFinish(HANDLE hPipe, OVERLAPPED &tagOver);
	//当前最大100KB
	void WriteShareMemory(void * content, int totalbyte);

	
private:
	MxProcessMsgCodec m_CommunicateMsgCodec;
	std::string m_pipeName;
	std::string m_realPipeName;

	bool m_isServer;
	bool m_hasInitialized;
	bool m_hasConnected;
	bool m_canReceive;
	
	HANDLE m_hPipe;

	DWORD m_bufferSize;

	std::thread* m_msgReceiveThread;

	HANDLE  m_ShareMemHandle;

	STARTUPINFOA m_si;
	PROCESS_INFORMATION m_pi;
	bool m_HasChildProcess;

	std::vector<CommunicatorMsgListener*> m_MsgListeners;
	std::vector<MxProcessCommunicateMsg*> m_ReceivedMsgQueue;
	CRITICAL_SECTION m_MsgMutex;
	float m_TimeElapseSinceLastProceed;
};

class MXCOMMON_API ShareMemoryXMLStream
{
public:
	std::string StreamUserAndTrainInfo(SYUserInfo & userInfo,
		                               const std::string & trainCat,
		                               const std::string & trainName,
		                               const std::string & trainChName,
									   const std:: string & trainCode);

	void DeStreamUserAndTrainInfo(char * strBuffer,
		                        SYUserInfo & userInfo,
		                        std::string & trainCat,
		                        std::string & trainName,
		                        std::string & trainChName,
								std::string & trainCode);

	
protected:

	std::string EncodePushOnLineCodeMsg(const char * str, int startTime, int endTime);
	void DecodePushOnLineCodeMsg(std::string & str, int & startTime, int & endTime);

	std::string EncodePullScoreTableMsg (const char * strtablename);
	void DecodePullScoreTableMsg(std::string & str);
};

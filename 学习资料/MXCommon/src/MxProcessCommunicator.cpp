#include "MxProcessCommunicator.h"
#include "tinyxml.h"
#include <iostream>

#define LOG(msg) MessageBoxA(nullptr, msg, "MxProcessCommunicator", MB_OK)
//#define LOG(msg) std::cerr<<msg<<std::endl

MxProcessCommunicator::MxProcessCommunicator(const std::string& pipeName,bool isServer)
	:m_pipeName(pipeName),
	m_realPipeName("\\\\.\\pipe\\" + m_pipeName),
	m_isServer(isServer),
	m_hasInitialized(false),
	m_hasConnected(false),
	m_canReceive(false),
	m_hPipe(INVALID_HANDLE_VALUE),
	m_bufferSize(10240),
	m_msgReceiveThread(nullptr)
{
	m_ShareMemHandle = 0;
	m_HasChildProcess = false;
	InitializeCriticalSection(&m_MsgMutex);//初始化临界区
	//qRegisterMetaType<MxProcessCommunicator::MessageType>("MessageType");
}

MxProcessCommunicator::~MxProcessCommunicator()
{
	if(m_hPipe != INVALID_HANDLE_VALUE)
	{
		if(m_isServer && !m_hasConnected)
		{
			HANDLE hTempPipe = CreateFileA(m_realPipeName.c_str(),
										   GENERIC_WRITE,
										   0,
										   nullptr,
										   OPEN_EXISTING,
										   0,
										   nullptr);
			CloseHandle(hTempPipe);
		}

		DisconnectNamedPipe(m_hPipe);
		CloseHandle(m_hPipe);
	}
	DeleteCriticalSection(&m_MsgMutex);
}

bool MxProcessCommunicator::Startup()
{
	if(m_hasInitialized)
		return true;

	if(m_isServer)
	{
		m_hPipe = CreateNamedPipeA(m_realPipeName.c_str(),
								//PIPE_ACCESS_INBOUND,
								  PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
								   PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
								   PIPE_UNLIMITED_INSTANCES,
								   m_bufferSize,
								   m_bufferSize,
								   3000,
								   nullptr);

		if(m_hPipe == INVALID_HANDLE_VALUE)
		{
			LOG("client startup the process communicator fail.");
			return false;
		}
		m_canReceive = true;
	}
	else
	{
		m_hPipe = CreateFileA(m_realPipeName.c_str(),
							GENERIC_READ | GENERIC_WRITE,
							  0,
							  nullptr,
							  OPEN_EXISTING,
							  FILE_FLAG_OVERLAPPED,
							  nullptr);

		if(m_hPipe == INVALID_HANDLE_VALUE)
		{
			LOG("client startup the process communicator fail.");
			return false;
		}

		m_canReceive = true;
	}

	m_ReceivedMsgQueue.clear();
	m_TimeElapseSinceLastProceed = 0;

	m_hasInitialized = true;
	return true;
}

bool MxProcessCommunicator::SendMessage(MxProcessCommunicateMsg * msg)
{
	if(m_hasInitialized == false)
		return false;

	//if(m_isServer)
	//	return false;

	BOOL bRet = false;

	//std::string msg;
	//msg.reserve(content.size() + sizeof(MXPCMessageType));

	//msg.append(reinterpret_cast<const char*>(&messageType), sizeof(messageType));
	//msg.append(content);

	OVERLAPPED tagOver;
	memset(&tagOver, 0x0, sizeof(tagOver));

	tagOver.hEvent = CreateEvent(NULL,//默认属性  
		TRUE,//手工reset  
		FALSE,//初始状态signaled  
		NULL);//未命名  

	int EncodeByte = 0;
	const char * msgBuff = m_CommunicateMsgCodec.EnCodeMxProcessMsg(msg, EncodeByte);
	DWORD writtenLen;
	bRet = WriteFile(m_hPipe,
		             msgBuff,
					 EncodeByte,
					 &writtenLen,
					 &tagOver);

	if (bRet)
	{
		qDebug() << "WriteMessage Finish";		
	}

	else
	{
		if (WaitFinish(m_hPipe, tagOver) != 0)
		{
			qDebug() << "Receive IOPending Error Occured";

			bRet = false;
		}
		else
		{
			bRet = true;
			qDebug() << "WaitWrite Finish";
		}
		
	}


	if (msg)
		qDebug() << "@@@@@@@@@@Send  Message @@@@@@@@@@" << msg->m_MsgHeader.m_MsgType << m_isServer;
	else
		qDebug() << "@@@@@@@@@@Send  Message  is Empty@@@@@@@@@@" << m_isServer;

	m_CommunicateMsgCodec.FreeEncodeMsgBuf(msgBuff);
 	return (bRet != 0);
}
//=====================================================================
void MxProcessCommunicator::ProcessMessages(float dt , int maxProcessNum)
{
	m_TimeElapseSinceLastProceed += dt;

	if (m_TimeElapseSinceLastProceed < 0.1f)//process every 0.1 second
		return;
	else
		m_TimeElapseSinceLastProceed = 0;

	EnterCriticalSection(&m_MsgMutex);

	int processNum = (int)m_ReceivedMsgQueue.size();
	
	if (processNum > maxProcessNum)
		processNum = maxProcessNum;

	for (int c = 0; c < processNum; c++)
	{
		MxProcessCommunicateMsg * msg = m_ReceivedMsgQueue[c];
		
		if (msg->m_MsgHeader.m_MsgType == MT_TERMINATE_TRAINING)
		{
			//listener
			for (int c = 0; c < m_MsgListeners.size(); c++)
			{
				m_MsgListeners[c]->OnReceiveSubModuleTerminate();
			}
			//listener
		}
		else
		{
			//listener
			for (int c = 0; c < m_MsgListeners.size(); c++)
			{
				m_MsgListeners[c]->OnReceiveCommunicatorMessage((MXPCMessageType)msg->m_MsgHeader.m_MsgType, msg);
			}
			//listener
		}

		//free this message after use
		m_CommunicateMsgCodec.FreeDecodedMsg(msg);
	}

	m_ReceivedMsgQueue.erase(m_ReceivedMsgQueue.begin(), m_ReceivedMsgQueue.begin() + processNum);

	LeaveCriticalSection(&m_MsgMutex);
}
//=====================================================================
void MxProcessCommunicator::StartReceiveMessage()
{
	if(m_canReceive)
	{
		m_canReceive = false;
		if(m_msgReceiveThread)
		{
			m_msgReceiveThread->join();
			delete m_msgReceiveThread;
		}

		m_msgReceiveThread = new std::thread(&MxProcessCommunicator::_ReceiveMessage, this);
	}
}

void MxProcessCommunicator::_ReceiveMessage()
{
	OVERLAPPED tagOver;
	memset(&tagOver, 0x0, sizeof(tagOver));

	tagOver.hEvent = CreateEvent(NULL,//默认属性  
		TRUE,//手工reset  
		FALSE,//初始状态signaled  
		NULL);//未命名  

	BOOL bRet = false;
	if (m_isServer)
	{
		bRet = ConnectNamedPipe(m_hPipe, &tagOver);

		if (bRet)
			return;

		if (WaitFinish(m_hPipe, tagOver) != 0)
			return;
		else
			bRet = true;
	}
		
	else
	{
		bRet = true;
	}
		
	if(bRet)// || GetLastError() == ERROR_PIPE_CONNECTED)
	{
		m_hasConnected = true;

		DWORD readSize;
		std::string msg(m_bufferSize, 0);
		
		while(true)
		{
			msg.resize(m_bufferSize);
			char* pBuffer = const_cast<char*>(msg.data());
			bRet = ReadFile(m_hPipe,
							pBuffer,
							msg.size(),
							&readSize,
							&tagOver);

			if (bRet)
			{
				qDebug() << "Read Finished" << m_isServer;
			}
			
			else
			{
				
				if (WaitFinish(m_hPipe, tagOver) != 0)
				{
					qDebug() << "Receive IOPending Error Occured";

					bRet = false;
				}
				else
				{
					readSize = tagOver.InternalHigh;
					bRet = true;
				}


				qDebug() << "WaitRead Finish" << m_isServer;
				
			}

			if(bRet)
			{
				//decode message
				MxProcessCommunicateMsg * msg = m_CommunicateMsgCodec.DecodeMxProcessMsg(pBuffer, readSize);
				
				if (msg)
					qDebug() << "@@@@@@@@@@Receive  Message  @@@@@@@@@@" << msg->m_MsgHeader.m_MsgType << m_isServer;
				else
				{
					qDebug() << "@@@@@@@@@@Receive  Message is Empty @@@@@@@@@@"<<m_isServer;
					continue;
				}
				

				EnterCriticalSection(&m_MsgMutex);

				m_ReceivedMsgQueue.push_back(msg);

				LeaveCriticalSection(&m_MsgMutex);
				
				
				/*
				
				//listener
				for (int c = 0; c < m_MsgListeners.size(); c++)
				{
					m_MsgListeners[c]->OnReceiveCommunicatorMessage(messageType, msg);
				}
				//listener

				//free this message after use
				m_CommunicateMsgCodec.FreeDecodedMsg(msg);

				*/
				//if(m_listener)
				/*{
					if (readSize >= sizeof(MXPCMessageType))
					{
						msg.resize(readSize);
						messageType = *reinterpret_cast<MXPCMessageType*>(pBuffer);
						readSize -= sizeof(MXPCMessageType);

						if(readSize >= 0)
						{
							msg.erase(0, sizeof(MXPCMessageType));

							for (int c = 0; c < m_MsgListeners.size(); c++)
							{
							   m_MsgListeners[c]->OnReceiveCommunicatorMessage(messageType, msg);
							}
							//m_listener->OnMessage(messageType, msg);
							//emit ReceiveMessage(messageType, QString::fromStdString(msg));
						}
					}
					else
					{
						LOG("ProcessCommunicator: read message error.");
					}
				}*/
			}
			else
			{
				DisconnectNamedPipe(m_hPipe);

				TerminateTrainMsg * msg = new TerminateTrainMsg();
				
				EnterCriticalSection(&m_MsgMutex);

				m_ReceivedMsgQueue.push_back(msg);

				LeaveCriticalSection(&m_MsgMutex);

				//emit ReceiveStop();
				/*for (int c = 0; c < m_MsgListeners.size(); c++)
				{
					m_MsgListeners[c]->OnReceiveSubModuleTerminate();
				}*/
				break;
			}
		}
	}

	m_canReceive = true;
}
void MxProcessCommunicator::AddMessageListener(MxProcessCommunicator::CommunicatorMsgListener * listener)
{
	for (int c = 0; c < m_MsgListeners.size(); c++)
	{
		if (m_MsgListeners[c] == listener)
			return;
	}
	m_MsgListeners.push_back(listener);
}

void MxProcessCommunicator::RemoveMessageListener(MxProcessCommunicator::CommunicatorMsgListener * listener)
{
	for (int c = 0; c < m_MsgListeners.size(); c++)
	{
		if (m_MsgListeners[c] == listener)
		{
			m_MsgListeners.erase(m_MsgListeners.begin()+c);
			return;
		}
	}
}
DWORD MxProcessCommunicator::LaunchProcess(const char * modulename, const char * commandStr, void * shareBuff, int shareByte)
{ 
	if (m_HasChildProcess)
		return 1000;

	if (shareBuff && shareByte > 0)
	    WriteShareMemory(shareBuff, shareByte);//write share memory 

	StartReceiveMessage();
	
	char buffer[MAX_PATH + 1];

	GetModuleFileNameA(NULL, buffer, MAX_PATH + 1);

	std::string moulePath = buffer;
	int nPos = moulePath.rfind('\\');
	moulePath = moulePath.substr(0 , nPos + 1);


	std::string subModuleFileName = moulePath + std::string(modulename);

	nPos = subModuleFileName.rfind('\\');
	std::string submoodulePath = subModuleFileName.substr(0, nPos + 1);

	std::string temp = "\"" + subModuleFileName + "\" " + "empty";

	ZeroMemory(&m_si, sizeof(m_si));
	m_si.cb = sizeof(m_si);
	ZeroMemory(&m_pi, sizeof(m_pi));

	if (!CreateProcessA(NULL,
		(char*)temp.c_str(),
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		submoodulePath.c_str(),
		&m_si,
		&m_pi)
		)
	{
		return GetLastError();
	}
	else
	{
		m_HasChildProcess = true;
		return 0;
	}
}

void MxProcessCommunicator::TerminateChildProcess()
{
	if (m_HasChildProcess)
	{
		TerminateProcess(m_pi.hProcess, 0);
		CloseHandle(m_pi.hProcess);
		CloseHandle(m_pi.hThread);

		ZeroMemory(&m_si, sizeof(m_si));
		m_si.cb = sizeof(m_si);
		ZeroMemory(&m_pi, sizeof(m_pi));

		m_HasChildProcess = false;
	}
}
void MxProcessCommunicator::WriteShareMemory(void * content , int totalbyte)
{
	//创建共享内存给训练进程使用
	if (m_ShareMemHandle == 0)
	{
		//100 KB shared Memory
		m_ShareMemHandle = ::CreateFileMapping((HANDLE)-1, NULL, PAGE_READWRITE, 0, /*strlen(xmlcstr) + 1*/1024 * 100 + sizeof(int), L"MisLap_shareMem");
	}

	if (m_ShareMemHandle != NULL)
	{
		void * strContent = (void*)::MapViewOfFile(m_ShareMemHandle, FILE_MAP_WRITE, 0, 0, 0);

		(*((int*)strContent)) = totalbyte;//write total byte number

		memcpy((char*)strContent + sizeof(int), content, totalbyte);

		::UnmapViewOfFile(strContent);
	}
}

int MxProcessCommunicator::ReadFromSharedMemory(void * content, int maxbyte)
{
	HANDLE hmap = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, L"MisLap_shareMem");
	if (hmap != 0)
	{
		void * memContent = (void*)::MapViewOfFile(hmap, FILE_MAP_WRITE, 0, 0, 0);

		int ByteNum = *((int*)memContent);

		if (ByteNum > maxbyte)
			ByteNum = maxbyte;

		memcpy(content, (char*)memContent + sizeof(int), ByteNum);

		::UnmapViewOfFile(memContent);

		return ByteNum;
	}
	else
		return 0;
}

std::string ShareMemoryXMLStream::StreamUserAndTrainInfo(SYUserInfo & userInfo,
	                                                     const std::string & trainCat,
	                                                     const std::string & trainName,
	                                                     const std::string & trainChName,
														 const std::string & trainCode)
{
	QString username = userInfo.GetUserName();
	QString realname = userInfo.GetRealName();
	int userid = userInfo.GetUserId();
	int userpermission = userInfo.GetUserPermission();
	int groupId = userInfo.GetGroupId();
	//user data organize in XML
	TiXmlElement userNode("userdata");
	userNode.SetAttribute("username", username.toStdString());
	userNode.SetAttribute("realName", realname.toStdString());
	userNode.SetAttribute("userid", userid);
	userNode.SetAttribute("userpermission", userpermission);
	userNode.SetAttribute("groupId", groupId);

	TiXmlElement trainNode("traindata");
	trainNode.SetAttribute("trainName", trainName);
	trainNode.SetAttribute("trainChName", trainChName);
	trainNode.SetAttribute("trainCategry", trainCat);
	trainNode.SetAttribute("trainCode", trainCode);

	TiXmlElement rootNode("shareData");
	rootNode.InsertEndChild(userNode);
	rootNode.InsertEndChild(trainNode);

	TiXmlPrinter printer;
	rootNode.Accept(&printer);
	const char * xmlcstr = printer.CStr();

	return xmlcstr;
}

void ShareMemoryXMLStream::DeStreamUserAndTrainInfo(char * strBuffer,
	                                                SYUserInfo & userInfo,
	                                                std::string & trainCat,
	                                                std::string & trainName,
	                                                std::string & trainChName,
													std::string & trainCode)
{
	TiXmlDocument xmldoc;
	xmldoc.Parse(strBuffer);
	TiXmlElement * rootElement = xmldoc.RootElement();

	TiXmlElement * userData = rootElement->FirstChildElement("userdata");

	//user data
	int userId;
	int permission;
	std::string username = userData->Attribute("username");
	userData->Attribute("userid", &userId);
	userData->Attribute("userpermission", &permission);
	std::string userName = userData->Attribute("username");
	std::string realName = userData->Attribute("realName");

	userInfo.m_userId = userId;
	userInfo.m_userName = QString::fromStdString(userName);
	userInfo.m_realName = QString::fromStdString(realName);
	userInfo.m_permission = (UserPermission)permission;

	//train data
	TiXmlElement * trainData = rootElement->FirstChildElement("traindata");
	if (trainData)
	{
		trainCat = trainData->Attribute("trainCategry");
		trainName = trainData->Attribute("trainName");
		trainChName = trainData->Attribute("trainChName");
		trainCode = trainData->Attribute("trainCode");
		//QtrainCategoryName = QString::fromStdString(trainCat);
		//QtrainEnName = QString::fromStdString(trainName);
		//QtrainChName = QString::fromStdString(trainChName);
	}
}

int MxProcessCommunicator::WaitFinish (HANDLE hPipe, OVERLAPPED &tagOver)
{
	bool bLog = false;
	bool bPendingIO = false;
	switch (GetLastError())
	{
		//等待IO完成
	case ERROR_IO_PENDING:
		bPendingIO = true;
		break;
		//已经连接  
	case ERROR_PIPE_CONNECTED:
		SetEvent(tagOver.hEvent);
		break;

	default:
		break;
	}

	DWORD dwWait = -1;
	DWORD dwTransBytes = -1;

	//等待读写操作完成  
	dwWait = WaitForSingleObject(tagOver.hEvent, INFINITE);
	
	switch (dwWait)
	{
	case 0:
		if (bPendingIO)
		{
			//获取Overlapped结果  
			if (GetOverlappedResult(hPipe, &tagOver, &dwTransBytes, FALSE) == FALSE)
			{
				printf("ConnectNamedPipe  failed   %d\n", GetLastError());
				return -1;
			}
		}
		break;
		//  读写完成  
	case WAIT_IO_COMPLETION:
		break;

	default:
		break;
	}

	return 0;
}
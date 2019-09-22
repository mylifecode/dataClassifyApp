#ifndef _MXPROCESSCOMMMSG_
#define _MXPROCESSCOMMMSG_
#include <iostream>
#include "MXCommon.h"
#pragma pack(push,1)

enum MXPCMessageType
{
	//system preserved message
	MT_EXIT = 0,
	MT_TERMINATE_TRAINING = 1,
	MT_SHOW_PARENT_WINDOW = 2,
	MT_PING = 3,
	

	//user define message type start
	MT_PULLONLINEGRADSCROETABLE = 1000,//拉信息化评分表
	MT_STARTONLINEGRAD = 1001,//信息化评测开始
	MT_PUSHONLINEGRADSCORE = 1002,//推送信息化评分项
	MT_STOPONLINEGRAD= 1003,//信息化评测结束
	MT_ONLINEGRADCALLBACK= 1004,//得到分数页面

	MT_STARTRECORDSCREEN = 1005,
	MT_STOPTRECORDSCREEN = 1006
};
struct MxMsgHeader
{
	int m_MsgType;
	int m_MsgSize;
};

struct MxProcessCommunicateMsg
{
	MxMsgHeader m_MsgHeader;
};

struct PingMsg : public MxProcessCommunicateMsg
{
	PingMsg()
	{
		m_MsgHeader.m_MsgType = MT_PING;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}
};

struct ShowParentWindowMsg : public MxProcessCommunicateMsg
{
	ShowParentWindowMsg(int showWinType)
	{
		m_MsgHeader.m_MsgType = MT_SHOW_PARENT_WINDOW;
		m_MsgHeader.m_MsgSize = sizeof(*this);
		m_ShowWinType = showWinType;
	}
	int m_ShowWinType;
};

struct TerminateTrainMsg : public MxProcessCommunicateMsg
{
	TerminateTrainMsg()
	{
		m_MsgHeader.m_MsgType = MT_TERMINATE_TRAINING;
		m_MsgHeader.m_MsgSize = sizeof(*this);
		m_ShowWinType = -1;
	}

	int m_ShowWinType;
};

struct StartRecordTrainMsg : public MxProcessCommunicateMsg
{
	StartRecordTrainMsg()
	{
		m_MsgHeader.m_MsgType = MT_STARTRECORDSCREEN;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}
	char m_RecordFileName[256];
};

struct StopRecordTrainMsg : public MxProcessCommunicateMsg
{
	StopRecordTrainMsg()
	{
		m_MsgHeader.m_MsgType = MT_STOPTRECORDSCREEN;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}
	char m_RecordFileName[256];
};

struct StartOnlineGradeMsg : public MxProcessCommunicateMsg
{
	StartOnlineGradeMsg()
	{
		m_MsgHeader.m_MsgType = MT_STARTONLINEGRAD;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}

	char m_sheetCode[30];
};

struct StopOnlineGradeMsg : public MxProcessCommunicateMsg
{
	StopOnlineGradeMsg()
	{
		m_MsgHeader.m_MsgType = MT_STOPONLINEGRAD;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}
};

struct OnlineGradeCallBackMsg : public MxProcessCommunicateMsg
{
	OnlineGradeCallBackMsg()
	{
		m_MsgHeader.m_MsgType = MT_ONLINEGRADCALLBACK;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}

	int  m_retCode;
	char m_strUrl[200];
};

struct PushOnlineCodeMsg : public MxProcessCommunicateMsg
{
	PushOnlineCodeMsg()
	{
		m_MsgHeader.m_MsgType = MT_PUSHONLINEGRADSCORE;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}
	char m_Code[20];
	int  m_StartTime;
	int  m_EndTime;
};

struct PullScoreTableMsg : public MxProcessCommunicateMsg
{
	PullScoreTableMsg()
	{
		m_MsgHeader.m_MsgType = MT_PULLONLINEGRADSCROETABLE;
		m_MsgHeader.m_MsgSize = sizeof(*this);
	}
	char m_scoreTable[40];
};
#pragma pack( pop ) 


class MXCOMMON_API MxProcessMsgCodec
{
public:
	MxProcessMsgCodec();

	~MxProcessMsgCodec();

	//@EnCodeMxProcessMsg
	//@Encode one message to byte stream
	//@return the byte stream encoded ,
	//@note you should call "FreeEncodeMsgBuf" after the buffer use finish!!
	const char * EnCodeMxProcessMsg(MxProcessCommunicateMsg * msg , int & encodeByte);

	//@DecodeMxProcessMsg
	//@decode a byte stream to a message
	//@return the message decoded
	//@note you should call "FreeEncodeMsgBuf" after the message use finish!!
	MxProcessCommunicateMsg * DecodeMxProcessMsg(const char * msgBuff , int maxByteNum);


	void FreeDecodedMsg(MxProcessCommunicateMsg *msg);

	void FreeEncodeMsgBuf(const char * msgBuff);

	//
	void TestCodec();

protected:

	void GetMsgHead(const char * msgBuff, MxMsgHeader & header);
};
#endif
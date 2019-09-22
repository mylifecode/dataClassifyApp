#include "MxProcessCommunicaMsg.h"

MxProcessMsgCodec::MxProcessMsgCodec()
{
	
}
//==========================================================================
MxProcessMsgCodec::~MxProcessMsgCodec()
{
	
}
//==========================================================================
void MxProcessMsgCodec::FreeDecodedMsg(MxProcessCommunicateMsg *msg)
{
	delete msg;
}
//==========================================================================
void MxProcessMsgCodec::FreeEncodeMsgBuf(const char * msgBuff)
{
	free((void*)msgBuff);
}
//==========================================================================
const char * MxProcessMsgCodec::EnCodeMxProcessMsg(MxProcessCommunicateMsg * msg, int & encodeByte)
{
	int MsgSize = msg->m_MsgHeader.m_MsgSize;
	char * msgBuf = (char*)malloc(MsgSize);
	memcpy(msgBuf, msg, MsgSize);
	encodeByte = MsgSize;
	return msgBuf;
}
//==========================================================================
void MxProcessMsgCodec::GetMsgHead(const char * msgBuff, MxMsgHeader & header)
{
	header.m_MsgType = *((int*)msgBuff);
	header.m_MsgSize = *((int*)(msgBuff + sizeof(int)));
}
//==========================================================================
MxProcessCommunicateMsg * MxProcessMsgCodec::DecodeMxProcessMsg(const char * msgBuff, int maxByteNum)
{
	if (maxByteNum <= 0)
		return 0;

	MxMsgHeader msgHead;
	GetMsgHead(msgBuff, msgHead);

	if (maxByteNum < msgHead.m_MsgSize)
		return 0;

	MxProcessCommunicateMsg * msg = 0;
	switch (msgHead.m_MsgType)
	{
	    case MT_PULLONLINEGRADSCROETABLE:
			 msg = new PullScoreTableMsg();
			 break;
		case MT_PUSHONLINEGRADSCORE:
			 msg = new PushOnlineCodeMsg();
			 break;
		
		case MT_STARTONLINEGRAD:
			msg = new StartOnlineGradeMsg();
			break;

		case MT_STOPONLINEGRAD:
			msg = new StopOnlineGradeMsg();
			break;

		case MT_ONLINEGRADCALLBACK:
			msg = new OnlineGradeCallBackMsg();
			break;

		case MT_PING:
			 msg = new PingMsg();
			 break;
		case MT_SHOW_PARENT_WINDOW:
			 msg = new ShowParentWindowMsg(-1);
			 break;
		case MT_STARTRECORDSCREEN:
			msg = new StartRecordTrainMsg();
			break;
		case MT_STOPTRECORDSCREEN:
			msg = new StopRecordTrainMsg();
			break;
	}
	//
	if (msg)
	    memcpy(msg, msgBuff, msgHead.m_MsgSize);
	return msg;
}
//=========================================================================================
void MxProcessMsgCodec::TestCodec()
{
	PushOnlineCodeMsg * msg = new PushOnlineCodeMsg();
	msg->m_StartTime = 12345;
	msg->m_EndTime = 54321;
	char * testCode = "TestCodec Fun@@";
	strcpy(msg->m_Code, testCode);

	//encode message
	int encodeByte;
	const char * encodeBuf = EnCodeMxProcessMsg(msg, encodeByte);
	
	//decode message
	PushOnlineCodeMsg * outmsg = (PushOnlineCodeMsg*)DecodeMxProcessMsg(encodeBuf, sizeof(PushOnlineCodeMsg));
	
	//free buffer
	FreeDecodedMsg(outmsg);
	FreeEncodeMsgBuf(encodeBuf);
}
#ifndef GLOBALVARMGR_H
#define GLOBALVARMGR_H

/**
	TODO
	�Ժ��ֹ�����������κγ�Ա��������Ϊ�༰���ļ�����ɾ��������ʹ���µ�ȫ�������ࣺMxGlobalConfig
*/
class GlobalVarMgr
{
public:
	static GlobalVarMgr& Instance()
	{
		static GlobalVarMgr pVarMgr;
		return pVarMgr;
	}
private:
	GlobalVarMgr()
	{
		checkdocbrowser = 0;
	}
	~GlobalVarMgr()
	{
		
	}

public:

	int checkdocbrowser;
};
#endif
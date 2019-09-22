#ifndef GLOBALVARMGR_H
#define GLOBALVARMGR_H

/**
	TODO
	以后禁止向该类中添加任何成员变量，因为类及其文件将被删除，建议使用新的全局配置类：MxGlobalConfig
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
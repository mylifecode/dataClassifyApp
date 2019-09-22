#ifndef RBTOOLMGR_H
#define RBTOOLMGR_H

#define TOOL(x) RbToolMgr::INS(x)

struct Tool_Unit 
{		
	Tool_Unit()
		:name(""),
		picFile(""),
		type(""),
		subType("")
	{
	}
	QString name;
	QString picFile;
	QString type;
	QString subType;
};

class RbToolMgr
{
public:
	static RbToolMgr& GetInstance();
	static Tool_Unit  INS(const QString & name);
	static const Tool_Unit& GetToolUnit(const QString& toolTypeName,const QString& toolSubTypeName);

private:
	RbToolMgr();
	~RbToolMgr();

private:
	bool readXML();

private:
	/// DeviceName to tool unit
	QMap<QString,Tool_Unit> m_toolMap;
	/// ToolType to tool unit
	QMap<QString,Tool_Unit> m_toolUnitMap;
};

#endif // RBTOOLMGR_H

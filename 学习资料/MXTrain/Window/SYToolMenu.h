#ifndef SYToolMenu_H
#define SYToolMenu_H

#include <QWidget>
#include <QPushButton>
#include <QSound>
#include "ui_toolMenu.h"
#include "RbToolMgr.h"
#include "XMLWrapperToolForTask.h"

class SYToolMenuEventListener
{
public:
	SYToolMenuEventListener(){}
	
	virtual ~SYToolMenuEventListener(){}

	virtual void OnToolMenuSelected(int side , QString & picFile, QString &type, QString &subType) = 0;
};
class SYToolMenuButton : public QPushButton
{
	Q_OBJECT
public:

	SYToolMenuButton(QWidget *parent = NULL)
		:QPushButton(parent)
	{
	}

signals:

	void hover();

	void leave();

protected:

	void enterEvent(QEvent * event);

	void leaveEvent(QEvent * event);

	void clicked(bool checked = false);

	void mousePressEvent(QMouseEvent  * e);
};

class MenuHistory
{
public:
	MenuHistory(int index, int startTime)
	{
		m_Index = index;
		m_StartTime = startTime;
		m_EndTime = startTime;
	}
	int m_Index;
	int m_StartTime;
	int m_EndTime;
};

class SYToolMenu : public QWidget
{
	Q_OBJECT

public:

	SYToolMenu(const QString & taskName ,vector<CXMLWrapperToolForTask *> & ToolInTask , bool bLeft,QWidget *parent = 0);
	~SYToolMenu();
	bool addTools(vector<CXMLWrapperToolForTask *> & ToolConfigs);

	void AddMenuEventListener(SYToolMenuEventListener * listener);

	void RemoveMenuEventListener(SYToolMenuEventListener * listener);

private:
	bool readXML(const QString & taskName);
	bool createMenu();
	bool selectType();
	bool changeType(int subindex);
	bool setNomalBtnBK(QPushButton * pBtn);
	bool setHoverBtnBK(QPushButton * pBtn);
	void addEmptyMenuItem(QHBoxLayout * pVLayout,bool left);
	void addOneMenutItem(Tool_Unit & unit);
public:
	void handleMsg(int type ,int subindex);
    void show();
    void hide();
	void setPositon(const QPoint & positon);
	
void keyPressEvent( QKeyEvent *event );

private slots:
	void  selectTool();
    void timeout();
	void timeout_later_hide();
	void onSelectEmptyMenuItem();

private:
	Ui::SYToolMenu ui;
	QList<Tool_Unit> m_toolList;
	QList<QPushButton *> m_btnList;
	QPushButton * m_SelectedBtn;
	int m_curIndex;
	QString m_SingleHand;
	bool m_bLeft;
	QTimer * m_TickTimer;
	QTimer * m_HideTimer;
	QPoint m_SrcPositon;
	QPoint m_DstPositon;
	bool bShowState;
	bool m_bHide;
	QHBoxLayout * m_ItemLayOut;
	std::vector<SYToolMenuEventListener*> m_MenuEventListener;
	std::vector<MenuHistory> m_MenuHistory;
};

#endif // SYToolMenu_H

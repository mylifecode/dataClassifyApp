#pragma once
#include <qwidget.h>
#include <qstring.h>
#include <QVector>
#include "RbRichButton.h"
#include "qlabel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QVector>
#include <qevent.h>

struct MENUITEMATTRIBUTE
{
	MENUITEMATTRIBUTE(QString strMenuType = "",QString strContent = "",QString objName = "",int id = -1,
		QString strCaseFile = "",QString strTrainEnName = "",QString strTrainingChName = "")
		:m_strMenuType(strMenuType),
		m_strContent(strContent),
		m_objName(objName),
		m_id(id),
		m_strTrainEnName(strTrainEnName),
		m_strTrainChName(strTrainingChName),
		m_strCaseFile(strCaseFile)
	{
	}
	QString m_strMenuType;		//m_strMenuType is menu name. identify the current menu
	QString m_strContent;
	QString m_objName;
	QString	m_id;	
	//XML property
	QString m_strCaseFile;
	QString m_strAviFile;//操作视频
	QString m_strTrainEnName;
	QString m_strTrainChName;
};

struct CLICKEDINFO
{
	CLICKEDINFO() 
	{
	}
	RbRichButton::CLICKCONTROLTYPE controlType;
	MENUITEMATTRIBUTE itemAttr;
};


class MenuContent: public QWidget
{
	Q_OBJECT;
public:
	MenuContent(QString& m_strMenuType,QWidget * parent = NULL,int numRow = 3,int numCol = 2);
	~MenuContent();
	bool addItem(MENUITEMATTRIBUTE & menuItemAttr);
	CLICKEDINFO getClickedInfo();
	void setMenuContentName(QString &strMenuName);
	bool hasPressedForLong();										//是否被长按
	QVector<RbRichButton*>& getRichBtns();							//返回richBtn
	QVector<QString>& getBtnItemID();	
	void subtractionNextBtnIndex();									//--m_nextBtnIndex
signals:
	void clickedItem();
public slots:
	void onClickedBtn();
	void onBtnPressed();											//菜单项被按下
	void onBtnRelease();											//菜单项被弹起
protected:
	virtual bool event(QEvent* e);
	void mousePressEvent ( QMouseEvent * event );
	void mouseReleaseEvent(QMouseEvent * event);
private:
	//--
	void calculateRichButtonPos();
	void updateRichButtonPos();
private:
	QVector<RbRichButton*> m_vecBtnItem;
	QVector<QString> m_vecBtnItemID;
	QVector<QLabel*> m_vecLabelItem;

	int m_numRow;
	int m_numCol;
	int m_nextBtnIndex;
	
	QString m_strMenuType;	//m_strMenuType is menu name. identify the current menu
	CLICKEDINFO m_clickedInfo;

	//new
	QGridLayout * m_pLayout;
	QVector<QSpacerItem*>	m_vecSpacerItem;
	QVector<QVBoxLayout*> m_vecItemLayout;

	QVector<QPoint> m_vecBtnPos;

	bool m_bHasPressdForLong;						//是否被长按

	unsigned long m_dwMousePressdTime;
	unsigned long m_dwMouseRelaseTime;
};

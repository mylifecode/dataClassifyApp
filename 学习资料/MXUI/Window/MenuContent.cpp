#include "MenuContent.h"
#include "RbRichButton.h"
#include <qgridlayout.h>
#include <qpushbutton.h>
#include "RbRichButton.h"
#include <windows.h>
#include <QDebug>
#include "MxDefine.h"

MenuContent::MenuContent(QString& strMenuType,QWidget * parent,int numRow,int numCol)
:QWidget(parent),
m_strMenuType(strMenuType),
m_numRow(numRow),
m_numCol(numCol),
m_nextBtnIndex(0),
m_bHasPressdForLong(false)
{
	Mx::setWidgetStyle(this,"qss:MenuContent.qss");
}

MenuContent::~MenuContent()
{
	for(int i = 0;i<m_vecBtnItem.size();++i)
		delete m_vecBtnItem[i];
	m_vecBtnItem.clear();
	for(int i = 0;i<m_vecLabelItem.size();++i)
		delete m_vecLabelItem[i];
	m_vecLabelItem.clear();
	for(int i = 0;i<m_vecSpacerItem.size();++i)
	{
		delete m_vecSpacerItem[i];
	}
	m_vecSpacerItem.clear();
}

bool MenuContent::event(QEvent * e)
{
	QSize size;
	switch(e->type())
	{
	case QEvent::Resize:
		calculateRichButtonPos();
		updateRichButtonPos();
		break;
	case QEvent::Show:
		calculateRichButtonPos();
		updateRichButtonPos();
		break;
	}
	return __super::event(e);
}

void MenuContent::setMenuContentName(QString &strMenuName)
{
	if(m_strMenuType != "")
		throw "MenuName != """;
	m_strMenuType = strMenuName;
}


bool MenuContent::addItem(MENUITEMATTRIBUTE & menuItemAttr)
{
	if(m_strMenuType != "" && menuItemAttr.m_strMenuType != m_strMenuType)
		throw "menuItemAttr.m_strMenuType != m_strMenuType";
	else if(m_nextBtnIndex == m_numRow * m_numCol)
		return false;
	else
	{
//new
		RbRichButton * pBtn = new RbRichButton(this,menuItemAttr.m_objName,menuItemAttr.m_id,true,false);
		m_vecBtnItem.push_back(pBtn);
		m_vecBtnItemID.push_back(menuItemAttr.m_id);

		pBtn->setObjectName(menuItemAttr.m_objName);
		pBtn->setId(menuItemAttr.m_id);
		pBtn->setBottomLabelText(menuItemAttr.m_strContent);
		pBtn->hideRightBottomBtn();		//���ص����½ǵļӺŰ�ť

		//set property
		pBtn->setProperty("CaseFile",menuItemAttr.m_strCaseFile);
		pBtn->setProperty("AviFile",menuItemAttr.m_strAviFile);
		pBtn->setProperty("TrainChName",menuItemAttr.m_strTrainChName);
		pBtn->setProperty("TrainEnName",menuItemAttr.m_strTrainEnName);

		//�ж��Ƿ�Ϊ�γ̼ƻ��˵�
		if (menuItemAttr.m_strMenuType == QString("CoursePlan"))
		{
			//ɾ������ġ���ӡ���ť
			for (int i = 0; i < m_vecBtnItem.size(); ++i)
			{
				if (m_vecBtnItemID[i] == "-1" && i < m_vecBtnItem.size()-1)
				{
					//��ע�͵Ļ����������г���+�Ű�ť���������
					//if (m_vecBtnItem[i])
					//{
					//	delete m_vecBtnItem[i];
					//}

					m_vecBtnItem.erase(m_vecBtnItem.begin()+i);
					m_vecBtnItemID.erase(m_vecBtnItemID.begin()+i);
					--m_nextBtnIndex;
					break;
				}
			}
			//���ÿγ̼ƻ��˵��в˵������ʽ
			pBtn->hideRightBottomBtn();										//�����ڿγ̼ƻ��˵��еĲ˵������Ҫ���½ǵļӺŰ�ť����˽�������
			//pBtn->setAttribute(Qt::WA_TransparentForMouseEvents);			//�γ̼ƻ��˵�����ʱ��֪ͨ������
			connect(pBtn, SIGNAL(pressBtn()),this, SLOT(onBtnPressed()));
			connect(pBtn, SIGNAL(releaseBtn()),this, SLOT(onBtnRelease()));
		}
		else
			connect(m_vecBtnItem[m_nextBtnIndex],SIGNAL(clickedBtn()),this,SLOT(onClickedBtn()));
		if(!m_nextBtnIndex)
			calculateRichButtonPos();
		updateRichButtonPos();
		++m_nextBtnIndex;
	}
	return true;
}

void MenuContent::calculateRichButtonPos()
{
	for(int row = 0;row < m_numRow;++row)
	{
		for(int col = 0;col < m_numCol;++col)
		{
			QPoint pt(col * 158,row * 162);
			m_vecBtnPos.push_back(pt);
		}
	}
}

void MenuContent::updateRichButtonPos()
{
	if(m_vecBtnPos.size() < m_vecBtnItem.size())
		throw "m_vecBtnPos.size() < m_vecBtnItem.size()";
	for(int i = 0;i<m_vecBtnItem.size();++i)
	{
		m_vecBtnItem[i]->move(m_vecBtnPos[i]);
		m_vecBtnItem[i]->show();
	}
}

void MenuContent::onClickedBtn()
{
	RbRichButton * pBtn = static_cast<RbRichButton*>(sender());
	m_clickedInfo.itemAttr.m_strMenuType = m_strMenuType;
	m_clickedInfo.itemAttr.m_id = pBtn->getId();
	m_clickedInfo.itemAttr.m_objName = pBtn->objectName();

	m_clickedInfo.itemAttr.m_strContent = pBtn->getBottomLabelText();
	m_clickedInfo.itemAttr.m_strCaseFile = pBtn->property("CaseFile").toString();
	m_clickedInfo.itemAttr.m_strAviFile = pBtn->property("AviFile").toString();
	m_clickedInfo.itemAttr.m_strTrainEnName = pBtn->property("TrainEnName").toString();
	m_clickedInfo.itemAttr.m_strTrainChName = pBtn->property("TrainChName").toString();

	for(int i = 0;i<m_vecBtnItem.size();++i)
	{
		if(m_vecBtnItem[i] == pBtn)
		{
			//m_clickedInfo.itemAttr.m_strContent = m_vecLabelItem[i]->text();
			m_clickedInfo.itemAttr.m_strContent = pBtn->getBottomLabelText();
			break;
		}
	}
	m_clickedInfo.controlType = pBtn->getClickType();
	emit clickedItem();
}


CLICKEDINFO MenuContent::getClickedInfo()
{
	return m_clickedInfo;
}


void MenuContent::mousePressEvent(QMouseEvent *event)
{
	m_dwMousePressdTime = GetTickCount();
}

void MenuContent::mouseReleaseEvent(QMouseEvent *event)
{
	m_dwMouseRelaseTime = GetTickCount();
	RbRichButton * pBtn = NULL;
	QPoint pt = event->pos();
	int x = pt.x();
	int y = pt.y();
	for (int i = 0; i != m_vecBtnItem.size(); ++i)
	{
		int left = m_vecBtnItem[i]->pos().x();
		int right = m_vecBtnItem[i]->pos().x()+m_vecBtnItem[i]->width();
		int bottom = m_vecBtnItem[i]->pos().y()+m_vecBtnItem[i]->height();
		int top = m_vecBtnItem[i]->pos().y();
		if (x <= right && x >= left && y <= bottom && y >= top)
		{
			pBtn = m_vecBtnItem[i];
			break;
		}
	}
	if (pBtn)
	{
		m_clickedInfo.itemAttr.m_strMenuType = m_strMenuType;
		m_clickedInfo.itemAttr.m_id = pBtn->getId();
		m_clickedInfo.itemAttr.m_objName = pBtn->objectName();

		m_clickedInfo.itemAttr.m_strContent = pBtn->getBottomLabelText();
		m_clickedInfo.itemAttr.m_strCaseFile = pBtn->property("CaseFile").toString();
		m_clickedInfo.itemAttr.m_strTrainEnName = pBtn->property("TrainEnName").toString();
		m_clickedInfo.itemAttr.m_strTrainChName = pBtn->property("TrainChName").toString();

		for(int i = 0;i<m_vecBtnItem.size();++i)
		{
			if(m_vecBtnItem[i] == pBtn)
			{
				m_clickedInfo.itemAttr.m_strContent = pBtn->getBottomLabelText();
				break;
			}
		}
		m_clickedInfo.controlType = RbRichButton::CLICK_PARENT;
	}
	else
		m_clickedInfo.controlType = RbRichButton::CLICK_NONE;			//��ֹ���ѵ��ģ���հ״�������һ��ѵ������

	//�ж��Ƿ�Ϊ����,��Ϊ��������ʾ�γ̼ƻ������޸Ľ���,������ʾ�γ̼ƻ�����
	if (m_dwMouseRelaseTime-m_dwMousePressdTime >= 1000)
		m_bHasPressdForLong = true;
	else
		m_bHasPressdForLong = false;
	emit clickedItem();
}

bool MenuContent::hasPressedForLong()
{
	return m_bHasPressdForLong;
}

QVector<RbRichButton*>& MenuContent::getRichBtns()
{
	return m_vecBtnItem;
}

QVector<QString>& MenuContent::getBtnItemID()
{
	return m_vecBtnItemID;
}

void MenuContent::subtractionNextBtnIndex()
{
	--m_nextBtnIndex;
}


void MenuContent::onBtnPressed()
{
	m_dwMousePressdTime = GetTickCount();
}

void MenuContent::onBtnRelease()
{
	m_dwMouseRelaseTime = GetTickCount();
	RbRichButton * pBtn = static_cast<RbRichButton*>(sender());
	if (pBtn)
	{
		m_clickedInfo.itemAttr.m_strMenuType = m_strMenuType;
		m_clickedInfo.itemAttr.m_id = pBtn->getId();
		m_clickedInfo.itemAttr.m_objName = pBtn->objectName();

		m_clickedInfo.itemAttr.m_strContent = pBtn->getBottomLabelText();
		m_clickedInfo.itemAttr.m_strCaseFile = pBtn->property("CaseFile").toString();
		m_clickedInfo.itemAttr.m_strTrainEnName = pBtn->property("TrainEnName").toString();
		m_clickedInfo.itemAttr.m_strTrainChName = pBtn->property("TrainChName").toString();

		for(int i = 0;i<m_vecBtnItem.size();++i)
		{
			if(m_vecBtnItem[i] == pBtn)
			{
				m_clickedInfo.itemAttr.m_strContent = pBtn->getBottomLabelText();
				break;
			}
		}
		m_clickedInfo.controlType = RbRichButton::CLICK_PARENT;

	}
	//�ж��Ƿ�Ϊ����,��Ϊ��������ʾ�γ̼ƻ������޸Ľ���,������ʾ�γ̼ƻ�����
	if (m_dwMouseRelaseTime-m_dwMousePressdTime >= 1000)
		m_bHasPressdForLong = true;
	else
		m_bHasPressdForLong = false;
	emit clickedItem();
}
#include "MxDefine.h"
#include "SYToolMenu.h"
#include "RbToolMgr.h"
#include "ToolsMgr.h"
#include "MXApplication.h"
#include "MxGlobalConfig.h"

#include <QLabel>
#define HIDE_TIME 15000
#define  TOOL_BK_PIC  "/toolIcon.png"
#define  TOOL_BK_PIC_P  "/toolIcon_p.png"


void SYToolMenuButton::enterEvent(QEvent * event)
{
	emit hover();
}

void SYToolMenuButton::leaveEvent(QEvent * event)
{
	emit leave();
}

void SYToolMenuButton::clicked(bool checked)
{
	emit __super::clicked();
}

void SYToolMenuButton::mousePressEvent(QMouseEvent  * e)
{
	QSound::play(MxGlobalConfig::Instance()->GetAudioDir() + "/Button54.wav");
	__super::mousePressEvent(e);
}

SYToolMenu::SYToolMenu( const QString & taskName , vector<CXMLWrapperToolForTask *> & ToolInTask , bool bLeft,QWidget *parent)
: QWidget(parent)
,m_curIndex(0)
,m_bLeft(bLeft)
,m_bHide(true)
,m_SelectedBtn(NULL)
,m_ItemLayOut(0)
{
	ui.setupUi(this);
	setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground,true);
	readXML(taskName);
	addTools(ToolInTask);
	createMenu();
	//adjustSize();

	m_TickTimer = new QTimer(this);
	m_HideTimer = new QTimer(this);
	connect(m_TickTimer, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(m_HideTimer, SIGNAL(timeout()), this, SLOT(timeout_later_hide()));
	
	Mx::setWidgetStyle(this,"qss:SYToolMenu.qss");
	setVisible(true);
}

SYToolMenu::~SYToolMenu()
{

}

bool SYToolMenu::addTools(std::vector<CXMLWrapperToolForTask *> & ToolConfigs)
{
	for(size_t c = 0 ; c < ToolConfigs.size() ; c++)
	{
		CXMLWrapperToolForTask * toolxml = ToolConfigs[c];
		Tool_Unit unit = TOOL(QString::fromUtf8(toolxml->m_DeviceName.c_str()));
		m_toolList.push_back(unit);

		/*RbPushButton * pButton= new RbPushButton(this);
		m_ItemLayOut->addWidget(pButton);
		m_btnList.push_back(pButton);

		pButton->setProperty( "strPixmap", MxGlobalConfig::Instance()->GetToolIconDir()+QString("/%1").arg(unit.picFile) );
		pButton->setProperty( "ToolType", unit.type );
		pButton->setProperty( "SubType", unit.subType );

		connect( pButton , SIGNAL( clicked() ) , this , SLOT( selectTool() ) );
		setNomalBtnBK(pButton);
		*/
		//addOneMenutItem(unit);
	}
	return true;
}

void SYToolMenu::AddMenuEventListener(SYToolMenuEventListener * listener)
{
	for (int c = 0; c < m_MenuEventListener.size(); c++)
	{
		if (m_MenuEventListener[c] == listener)
			return;
	}
	m_MenuEventListener.push_back(listener);
}

void SYToolMenu::RemoveMenuEventListener(SYToolMenuEventListener * listener)
{
	for (int c = 0; c < m_MenuEventListener.size(); c++)
	{
		if (m_MenuEventListener[c] == listener)
		{
			m_MenuEventListener.erase(m_MenuEventListener.begin() + c);
			break;
		}
	}

}

bool SYToolMenu::readXML(const QString & Name)
{
	return true;
}

bool SYToolMenu::createMenu()
{
	m_ItemLayOut = new QHBoxLayout( ui.toolBkFrame );
	m_ItemLayOut->setSpacing(20);
	m_ItemLayOut->setContentsMargins(20, 0, 20, 0);
	//m_ItemLayOut->setContentsMargins (0,0,20,2);
	
	QLabel * pLabel = new QLabel(ui.toolBkFrame);
	pLabel->setAlignment(Qt::AlignCenter);
	QString showText = m_bLeft ? QString::fromLocal8Bit("左") : QString::fromLocal8Bit("右");
	pLabel->setText(showText);
	pLabel->setStyleSheet("color:white; font: bold 18px;");
	
	QPushButton * pButton = NULL;
	
	if (!m_bLeft)
	{
		m_ItemLayOut->addWidget(pLabel);
		addEmptyMenuItem(m_ItemLayOut, m_bLeft);		//增加空置按钮
	}
	foreach(Tool_Unit unit,m_toolList)
	{
		addOneMenutItem(unit);
	}

	if (m_bLeft)
	{
		addEmptyMenuItem(m_ItemLayOut, m_bLeft);		//增加空置按钮
		m_ItemLayOut->addWidget(pLabel);
	}
	
	return true;
}

void SYToolMenu::handleMsg(int type, int subindex)
{
	if (!m_btnList.size()) 
		return;
	
	m_bHide = false;

	int AngleInterval = 5;
	
	int range = AngleInterval * m_btnList.size();
	
	int halfRange = range * 0.5f;

	int selItemIndex = (-subindex + halfRange) / AngleInterval;

	switch(type)
	{
	case SHOW_EVT:
		 show(); //setHoverBtnBK(m_btnList[m_curIndex]);
		 if (selItemIndex >= 0 && selItemIndex < m_btnList.size())
			 changeType(selItemIndex);
		 break;
	
	case CHANGE_EVT:
		 if (selItemIndex >= 0 && selItemIndex < m_btnList.size())
		 {
			bool succed = changeType(selItemIndex);
			if (succed)
			{
				int currTick = GetTickCount();
				if (m_MenuHistory.size() == 0)
				{
					m_MenuHistory.push_back(MenuHistory(selItemIndex, currTick));
				}
				else
				{
					int numItem = m_MenuHistory.size();

					m_MenuHistory[numItem - 1].m_EndTime = currTick;

					if (m_MenuHistory[numItem - 1].m_Index != selItemIndex)
					{
						m_MenuHistory.push_back(MenuHistory(selItemIndex, currTick));
					}
				}

				if (m_MenuHistory.size() > 3)
				{
					m_MenuHistory.erase(m_MenuHistory.begin(),m_MenuHistory.end() - 3);
				}
			}
		 }
		 break;
	
	case SELECT_EVT:
		
		 if (isVisible())
		 {
			//select property one
			int currTick = GetTickCount();
			for (int c = m_MenuHistory.size() - 1; c >= 0; c++)
			{
				int delta = currTick - m_MenuHistory[c].m_StartTime;
				if (delta > 250)
				{
					m_curIndex = m_MenuHistory[c].m_Index;
					break;
				}
			}
			selectType();
		 }
		 m_MenuHistory.clear();
		 break;
	default:
		hide();
		m_MenuHistory.clear();
		break;
	}
}

bool SYToolMenu::selectType()
{
	ITraining * pTraining= (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	CToolsMgr * pToolsMgr = pTraining->m_pToolsMgr;
	if (m_SingleHand == "true")
	{
		pToolsMgr->RemoveAllCurrentTool();
	}
	pToolsMgr->SwitchTool(m_bLeft, m_toolList.at(m_curIndex).type.toStdString(),m_toolList.at(m_curIndex).subType.toStdString());
	hide();


	QPushButton * pButton = (m_btnList.at(m_curIndex));
	QString strPixmap = pButton->property("strPixmap").toString();
	QString toolType  = pButton->property("ToolType").toString();
	QString subType   = pButton->property("SubType").toString();

	for (int c = 0; c < m_MenuEventListener.size(); c++)
	{
		m_MenuEventListener[c]->OnToolMenuSelected(m_bLeft ? 0 : 1, strPixmap, toolType, subType);
	}

	return true;
}

bool SYToolMenu::changeType(int subindex)
{
	int size = m_btnList.size();
	
	if (subindex < 0 || subindex >= size) 
		return false;

	if (m_curIndex < 0 || m_curIndex >= size)
		return false;

	setNomalBtnBK(m_btnList[m_curIndex]);

	setHoverBtnBK(m_btnList[subindex]);

	m_curIndex = subindex;
	
	return true;
}

bool SYToolMenu::setNomalBtnBK( QPushButton * pBtn )
{
	if ( pBtn)
	{
		QString strPixmap = pBtn->property( "strPixmap" ).toString();
		//QString strBackPixmap =  MxGlobalConfig::Instance()->GetSkinDir() + TOOL_BK_PIC;
		pBtn->setStyleSheet(QString("image:url(%1);border:1px solid #46526D;").arg(strPixmap));
	}
	return true;
}

bool SYToolMenu::setHoverBtnBK( QPushButton * pBtn )
{
	if ( pBtn)
	{
		QString strPixmap = pBtn->property( "strPixmap" ).toString();
		//QString strBackPixmap =  MxGlobalConfig::Instance()->GetSkinDir() + TOOL_BK_PIC_P;
		pBtn->setStyleSheet(QString("image:url(%1);border:3px solid #1691EB;").arg(strPixmap));
	}
	return true;
}

void SYToolMenu::selectTool()
{
	QPushButton * pButton = (QPushButton *)sender();
	QString strPixmap = pButton->property("strPixmap").toString();
	QString toolType  = pButton->property( "ToolType").toString();
	QString subType   = pButton->property( "SubType").toString();

	ITraining * pTraining= (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	CToolsMgr * pToolsMgr = pTraining->m_pToolsMgr;
	if (m_SingleHand == "true")
	{
		pToolsMgr->RemoveAllCurrentTool();
	}
	setNomalBtnBK(m_SelectedBtn);
	setHoverBtnBK(pButton);	
	m_SelectedBtn = pButton;
	pToolsMgr->SwitchTool(m_bLeft, toolType.toStdString(),subType.toStdString());

	for (int c = 0; c < m_MenuEventListener.size(); c++)
	{
		m_MenuEventListener[c]->OnToolMenuSelected(m_bLeft ? 0 : 1 ,strPixmap, toolType, subType);
	}
	hide();
}

void SYToolMenu::show()
{
	__super::show();
	/*if (!m_TickTimer->isActive())
	{
		m_TickTimer->start(50);
		bShowState = true;
		QTimer::singleShot(HIDE_TIME,this,SLOT(timeout_later_hide()));
	}*/
	move(m_DstPositon);

	ITraining * pTraining = (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	CToolsMgr * pToolsMgr = pTraining->m_pToolsMgr;

	if ( m_bLeft )
	{
		pToolsMgr->RemoveLeftCurrentTool();
	}
	else
	{
		pToolsMgr->RemoveRightCurrentTool();
	}
}

void SYToolMenu::hide()
{
	__super::hide();
	return;

	if (!m_TickTimer->isActive())
	{
		m_TickTimer->start(50);
		bShowState = false;
		m_HideTimer->stop();
	}
}

void SYToolMenu::timeout()
{
	int x_dst = m_DstPositon.x();
	int x_src = m_SrcPositon.x();
	int x = pos().x();
	int y = pos().y();

	int step =bShowState? (x_dst-x_src)/10:(x_src-x_dst)/10;
	int x_direction = bShowState ? x_dst:x_src;
	if ( qAbs(x_direction-x) < qAbs(step) || step==0)
	{
		m_TickTimer->stop();
		if (!bShowState)
		{		
			__super::hide();
		}
		clearFocus();
	}
	else
	{
		  move(x+step,y);
	}
}

void SYToolMenu::setPositon(const QPoint & positon)
{
	m_DstPositon = positon;
	if (m_bLeft)
	{
		m_SrcPositon = QPoint(m_DstPositon.x()-width()*1.5,m_DstPositon.y());
		m_DstPositon.setX(m_DstPositon.x()+5);
	}
	else
	{
		m_SrcPositon = QPoint(m_DstPositon.x()+width()*1.5,m_DstPositon.y());
		m_DstPositon.setX(m_DstPositon.x()-5);
	}
	move(m_SrcPositon);
}

void SYToolMenu::keyPressEvent( QKeyEvent *event )
{
	if (!m_btnList.size()) return;
	m_bHide = false;
	switch(event->key())
	{
	case Qt::Key_F2:
	case Qt::Key_F5:
		{
			if(isVisible())
			{
				hide();
			}
			else
			{
				if (m_HideTimer->isActive())
				{
					m_HideTimer->stop();
				}
				m_HideTimer->start(HIDE_TIME);
				show();
			}
		}
		break;
	case Qt::Key_F3:
	case Qt::Key_F6:
		//if(!isVisible()) return;
		//handleMsg(CHANGE_EVT);
		break;
	case Qt::Key_F4:
	case Qt::Key_F7:
		//if(!isVisible()) return;
		//handleMsg(SELECT_EVT);
		break;
	}
}

void SYToolMenu::timeout_later_hide()
{
	if	(m_bHide)
	{
		m_HideTimer->stop();
		hide();
	}
	m_bHide = true;
}

void SYToolMenu::addOneMenutItem(Tool_Unit & unit)
{
	if(m_ItemLayOut == 0)
	   return;

	if (unit.type.isEmpty())  
		return;

	SYToolMenuButton * pButton = new SYToolMenuButton(this);
	m_ItemLayOut->addWidget(pButton);
	m_btnList.push_back(pButton);

	pButton->setProperty( "strPixmap", MxGlobalConfig::Instance()->GetToolIconDir()+QString("/%1").arg(unit.picFile) );
	pButton->setProperty( "ToolType", unit.type );
	pButton->setProperty( "SubType", unit.subType );

	connect( pButton , SIGNAL( clicked() ) , this , SLOT( selectTool() ) );
	setNomalBtnBK(pButton);
}
void SYToolMenu::addEmptyMenuItem(QHBoxLayout * pVLayout, bool left)
{
	QPushButton * pButton = NULL;

	pButton = new SYToolMenuButton(this);
	pVLayout->addWidget(pButton);
	m_btnList.push_back(pButton);
	Tool_Unit unit;
	unit.name = "";
	unit.picFile = "";
	unit.type = "";
	unit.subType = "";
	pButton->setProperty( "strPixmap", MxGlobalConfig::Instance()->GetToolIconDir());
	pButton->setProperty( "ToolType", unit.type );
	pButton->setProperty( "SubType", unit.subType );

	if (left)
		m_toolList.push_back(unit);
	else
	    m_toolList.push_front(unit);
	setNomalBtnBK(pButton);
	setHoverBtnBK(pButton);	
	m_SelectedBtn = pButton;
	connect(pButton,SIGNAL(clicked()),this,SLOT(onSelectEmptyMenuItem()));
}

void SYToolMenu::onSelectEmptyMenuItem()
{
	QPushButton * pButton = (QPushButton *)sender();
	/*QString toolType = pButton->property( "ToolType").toString();
	QString subType = pButton->property( "SubType").toString();*/
	
	ITraining * pTraining= (static_cast<MXApplication*>(qApp))->GetEngineCore()->GetTrainingMgr()->GetCurTraining();
	if(!pTraining)
	{
		MessageBoxA(NULL,"pTraining is nullptr!",NULL,MB_OK);
		return;
	}
	CToolsMgr * pToolsMgr = pTraining->m_pToolsMgr;
	if(!pToolsMgr)
	{
		MessageBoxA(NULL,"pToolsMgr is nullptr",NULL,MB_OK);
		return ;
	}
	if (m_SingleHand == "true")
	{
		pToolsMgr->RemoveAllCurrentTool();
	}
	setNomalBtnBK(m_SelectedBtn);
	setHoverBtnBK(pButton);	
	m_SelectedBtn = pButton;
	//pToolsMgr->SwitchTool(m_bLeft, toolType.toStdString(),subType.toStdString());
	hide();

	QString emptystr("");

	for (int c = 0; c < m_MenuEventListener.size(); c++)
	{
		m_MenuEventListener[c]->OnToolMenuSelected(m_bLeft ? 0 : 1, emptystr, emptystr, emptystr);
	}
}
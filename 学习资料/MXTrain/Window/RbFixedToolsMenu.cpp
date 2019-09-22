#include "RbFixedToolsMenu.h"
#include "ToolsMgr.h"
#include "SYTrainWindow.h"
#include "RbToolMgr.h"
#include "MxGlobalConfig.h"
RbFixedToolsMenu::RbFixedToolsMenu(QWidget* pParent)
:QWidget(pParent,Qt::Window | Qt::FramelessWindowHint),
m_caseOperationWidget(NULL)
{
	m_ui.setupUi(this);
	
	setAttribute(Qt::WA_TranslucentBackground,true);

	Mx::setWidgetStyle(this,"qss:RbFixedToolsMenu.qss");
}

RbFixedToolsMenu::~RbFixedToolsMenu(void)
{
	m_fixedToolsBtn.clear();
}

void RbFixedToolsMenu::on_fixLeftBtn_clicked()
{
	if(m_caseOperationWidget)
	{
		CToolsMgr* pToolsMgr = m_caseOperationWidget->GetToolsMgr();
		if(pToolsMgr)
		{
			ITool* fixedTool = pToolsMgr->FixTool(true);
			if(fixedTool)
				AddOneFixedTool(fixedTool,true);
		}
	}
}

void RbFixedToolsMenu::on_fixRightBtn_clicked()
{
	if(m_caseOperationWidget)
	{
		CToolsMgr* pToolsMgr = m_caseOperationWidget->GetToolsMgr();
		if(pToolsMgr)
		{
			ITool* fixedTool = pToolsMgr->FixTool(false);
			if(fixedTool)
				AddOneFixedTool(fixedTool,false);
		}
	}
}

void RbFixedToolsMenu::AddOneFixedTool(ITool* fixedTool,bool isLeftHand)
{
	QPushButton* toolBtn = new QPushButton;
	QString toolName = fixedTool->GetName().c_str();
	toolBtn->setProperty("toolName",toolName);
	//toolBtn->setText(toolName);
	toolBtn->setObjectName("fixedToolBtn");
	//connect(toolBtn,SIGNAL(clicked()),this,SLOT(onFixedToolsBtnClicked()));

	m_fixedToolsBtn.insert(toolBtn);
	//限制最多只能固定2个器械
	if(m_fixedToolsBtn.size() >= 2)
	{
		m_ui.fixLeftBtn->setEnabled(false);
		m_ui.fixRightBtn->setEnabled(false);
	}

	m_ui.fixedBtnLayout->addWidget(toolBtn);

	//set btn icon
	QString toolType = fixedTool->GetType().c_str();
	QString toolSubType = fixedTool->GetSubType().c_str();
	const Tool_Unit& toolUnit = RbToolMgr::GetInstance().GetToolUnit(toolType,toolSubType);
	QString qss = QString("border-image:url(%1);").arg(MxGlobalConfig::Instance()->GetToolIconDir()+ "/" + toolUnit.picFile);
	toolBtn->setStyleSheet(qss);
}

void RbFixedToolsMenu::on_clearToolsBtn_clicked()
{
	ClearAllFixedTool();
}

void RbFixedToolsMenu::ClearAllFixedTool()
{
	CToolsMgr* pToolsMgr = m_caseOperationWidget->GetToolsMgr();
	if(pToolsMgr)
	{
		pToolsMgr->RemoveAllFixedTools();
	}

	for(QSet<QPushButton*>::iterator itr = m_fixedToolsBtn.begin();itr != m_fixedToolsBtn.end();++itr)
		delete *itr;

	m_fixedToolsBtn.clear();

	m_ui.fixLeftBtn->setEnabled(true);
	m_ui.fixRightBtn->setEnabled(true);
}
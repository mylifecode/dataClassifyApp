#include "SYAdminPortalWindow.h"
#include"ui_SYAdminPortalWindow.h"
#include "SYMonitorWindow.h"
#include <QMessageBox>
#include "SYUserInfo.h"
#include "SYMessageBox.h"
#include "MxGlobalConfig.h"
#include "MXApplication.h"


SYAdminPortalWindow::SYAdminPortalWindow(QWidget *parent) : QWidget(parent),
m_userName(""),
m_realName("")
{
	m_userName = SYUserInfo::Instance()->GetUserName();
	m_realName = SYUserInfo::Instance()->GetRealName();

	ui.setupUi(this);
	//QObject::connect(ui.bt_theoryTestmgr, &QPushButton::clicked, this, &SYAdminPortalWindow::on_bt_theoryTestmgr_clicked);
  //  QObject::connect(ui.bt_knowledgemgr, &QPushButton::clicked, this, &SYAdminPortalWindow::on_bt_knowledgemgr_clicked);
	
	/*
	if(m_realName.size())
		ui.adminidlabel->setText(m_userName + QString("(%1)").arg(m_realName));
	else
		ui.adminidlabel->setText(m_userName);

	m_permission = SYUserInfo::Instance()->GetUserPermission();
	m_mapLoginModule = Mx::addModuleItemFromXML();
	if (MxGlobalConfig::Instance()->onLineConfig())
	{
		m_permission = UP_Error;
	}

	if (m_mapLoginModule.size())
	{
		for (std::multimap<UserPermission, QString>::iterator itr = m_mapLoginModule.begin(); itr != m_mapLoginModule.end(); ++itr)
		{
			if (itr->first == m_permission )
			{
				QPushButton* pBtn = new QPushButton(this);
				QString str = itr->second;
				pBtn->setObjectName(str);
				pBtn->setProperty("window", str);
				int index = ui.horizontalLayout->count() - 1;
				if (index > 1)
				{
					ui.horizontalLayout->insertSpacerItem(index, new QSpacerItem(115, 20));
				}
				index = ui.horizontalLayout->count() - 1;
				ui.horizontalLayout->insertWidget(index, pBtn);
				m_buttons.push_back(pBtn);
				connect(pBtn, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
			}
			
		}
	}

	
	*/
	//MXApplication* app = static_cast<MXApplication*>(qApp);
	//connect(app, &MXApplication::ReceiveMessage, this, &RbAdminWindow::OnReceiveMessage);
	//connect(app, &MXApplication::ReceiveStop, this, &RbAdminWindow::OnReceiveStop);
	Mx::setWidgetStyle(this, "qss:SYAdminPortalWindow.qss");
}

SYAdminPortalWindow::~SYAdminPortalWindow(void)
{

}


void SYAdminPortalWindow::on_bt_personmgr_clicked()
{
	emit showNextWindow(WT_AdminPersonWindow);
}

void SYAdminPortalWindow::on_bt_studymgr_clicked()
{
	emit showNextWindow(WT_AdminTaskWindow);   //人员管理
}
//理论测试集管理
void SYAdminPortalWindow::on_bt_theoryTestmgr_clicked()
{
	emit showNextWindow(WT_AdminTheoryTestWindow);
}
//知识集管理
void SYAdminPortalWindow::on_bt_knowledgemgr_clicked()
{
	emit showNextWindow(WT_AdminKnowledgeSetManageWindow);
}
void SYAdminPortalWindow::on_bt_datacenter_clicked()
{
	emit showNextWindow(WT_AdminDataCenterWindow);
}
void SYAdminPortalWindow::on_bt_traincenter_clicked()
{
	emit showNextWindow(WT_AdminTrainingCenterWindow);
}

void SYAdminPortalWindow::on_bt_personcenter_clicked()
{
	emit showNextWindow(WT_PersonCenterWindow);
}

void SYAdminPortalWindow::on_bt_networkCenter_clicked()
{
	emit showNextWindow(WT_MonitorWindow);
}
#include "SYMessageSendWindow.h"
#include <MXDefine.h>
#include "SYUserListWindow.h"
#include <SYDBMgr.h>
#include <SYUserInfo.h>

SYMessageSendWindow::SYMessageSendWindow(QWidget* parent)
	:RbShieldLayer(parent),
	m_userListWindow(nullptr),
	m_groupId(0)
{
	ui.setupUi(this);

	ui.receiverLineEdit->setEnabled(false);

	hideCloseButton();
	hideOkButton();

	Mx::setWidgetStyle(this, "qss:/SYMessageSendWindow.qss");
}

SYMessageSendWindow::~SYMessageSendWindow()
{

}

void SYMessageSendWindow::SetReceiveGroup(int groupId)
{
	m_groupId = groupId;
}

void SYMessageSendWindow::on_userListBtn_clicked()
{
	if(m_userListWindow == nullptr)
		m_userListWindow = new SYUserListWindow(this);

	if(m_groupId > 0)
		m_userListWindow->SetGroupFilter(m_groupId);

	m_userListWindow->showFullScreen();
	if(m_userListWindow->exec() == RbShieldLayer::RC_Ok){
		m_receiverIds.clear();
		const auto& userInfos = m_userListWindow->GetSelectedUserInfos();

		QString editContent("   ");
		for(auto itr = userInfos.begin(); itr != userInfos.end(); ++itr){
			m_receiverIds.push_back(itr.key());

			editContent += itr.value();
			editContent += "   ";
		}

		//set line edit content
		ui.receiverLineEdit->setText(editContent);
	}
}

void SYMessageSendWindow::on_cancelBtn_clicked()
{
	m_receiverIds.clear();
	done(RbShieldLayer::RC_Cancel);
}

void SYMessageSendWindow::on_okBtn_clicked()
{
	//receivers
	if(m_receiverIds.size() == 0){
		InternalMessageBox(CHS("提示"), CHS("请选择消息接受者"));
		return;
	}

	//subject
	QString subject = ui.subjectLineEdit->text();
	if(subject.size() == 0){
		InternalMessageBox(CHS("提示"), CHS("主题不能为空"));
		return;
	}

	//content
	QString content = ui.contentTextEdit->toPlainText();
	if(content.size() == 0){
		InternalMessageBox(CHS("提示"), CHS("消息内容不能为空"));
		return;
	}

	//send message
	int senderId = SYUserInfo::Instance()->GetUserId();
	for(auto receiverId : m_receiverIds){
		SYDBMgr::Instance()->AddMessageInfo(senderId, receiverId, subject, content);
	}

	done(RbShieldLayer::RC_Ok);
}

void SYMessageSendWindow::showEvent(QShowEvent* event)
{
	m_receiverIds.clear();
	ui.receiverLineEdit->clear();
	ui.subjectLineEdit->clear();
	ui.contentTextEdit->clear();
}
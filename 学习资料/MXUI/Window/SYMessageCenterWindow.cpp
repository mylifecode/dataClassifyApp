#include "SYMessageCenterWindow.h"
#include <MXDefine.h>
#include <SYUserInfo.h>
#include <SYDBMgr.h>
#include "SYMessageWindow.h"

SYMessageCenterWindow::SYMessageCenterWindow(QWidget* parent)
	:QWidget(parent)
{
	ui.setupUi(this);

	LoadMessages();

	//需要排序窗口
	SortMessageWindowByDateTimeDesc();
	for(auto* messageWindow : m_messageWindows)
		AddMessageWindow(messageWindow);

	auto layout = static_cast<QVBoxLayout*>(ui.scrollAreaWidgetContents->layout());
	if(layout)
		layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));

	Mx::setWidgetStyle(this, "qss:/SYMessageCenterWindow.qss");
}

SYMessageCenterWindow::~SYMessageCenterWindow()
{

}

void SYMessageCenterWindow::LoadMessages()
{
	const int curUserId = SYUserInfo::Instance()->GetUserId();

	//1 对于当前用户是接受者
	std::vector<QSqlRecord> records1,records2;
	SYDBMgr::Instance()->QueryMessagesByReceiverId(curUserId, records1);

	std::size_t nRecord = records1.size();
	QString subject;
	QString content;
	SYMessageWindow* messageWindow = nullptr;
	for(const auto& originalMessageRecord : records1){
		
		auto originalMessageStatus = static_cast<SYMessageWindow::MessageStatus>(originalMessageRecord.value("status").toInt());
		if(originalMessageStatus & SYMessageWindow::MS_DeletedByReceiver)
			continue;

		messageWindow = new SYMessageWindow;

		int originalMessageSenderId = originalMessageRecord.value("senderId").toInt();
		int originalMessageId = originalMessageRecord.value("id").toInt();

		//original message info
		QDateTime dateTime = originalMessageRecord.value("sendTime").toDateTime();
		messageWindow->SetOriginalMessageInfo(originalMessageSenderId,
											  originalMessageId,
											  originalMessageStatus,
											  dateTime);

		//set sender and receiver
		messageWindow->SetSenderId(curUserId);
		messageWindow->SetReceiverId(originalMessageSenderId);

		//subject
		subject = originalMessageRecord.value("subject").toString();
		messageWindow->SetSubject(subject);

		//content
		content = originalMessageRecord.value("content").toString();
		messageWindow->AddContent(originalMessageId,
								  originalMessageSenderId, 
								  content,
								  dateTime,
								  originalMessageStatus);

		//query all reply message of this subject
		records2.clear();
		SYDBMgr::Instance()->QueryMessageReply(originalMessageId, records2);
		for(const auto& replyRecord : records2){
			int replyId = replyRecord.value("replyerId").toInt();
			content = replyRecord.value("content").toString();
			messageWindow->AddContent(replyRecord.value("id").toInt(),
									  replyId, 
									  content,
									  replyRecord.value("time").toDateTime(),
									  static_cast<SYMessageWindow::MessageStatus>(replyRecord.value("status").toInt()));
		}

		m_messageWindows.push_back(messageWindow);
	}
	

	//2 对于当前用户是发送者
	records1.clear();
	SYDBMgr::Instance()->QueryMessagesBySenderId(curUserId, records1);
	for(const auto& originalMessageRecord : records1){
		auto originalMessageStatus = static_cast<SYMessageWindow::MessageStatus>(originalMessageRecord.value("status").toInt());

		if(originalMessageStatus & SYMessageWindow::MS_DeletedBySender)
			continue;

		int originalMessageSenderId = originalMessageRecord.value("senderId").toInt();
		int originalMessageId = originalMessageRecord.value("id").toInt();
		QDateTime dateTime = originalMessageRecord.value("sendTime").toDateTime();
		records2.clear();
		messageWindow = nullptr;

		SYDBMgr::Instance()->QueryMessageReply(originalMessageId, records2);
		for(const auto& replyRecord : records2){
			if(messageWindow == nullptr){
				messageWindow = new SYMessageWindow;
				
				//original message info
				messageWindow->SetOriginalMessageInfo(originalMessageSenderId,
													  originalMessageId,
													  originalMessageStatus,
													  dateTime);

				//set sender and receiver
				messageWindow->SetSenderId(curUserId);
				int id = originalMessageRecord.value("receiverId").toInt();
				messageWindow->SetReceiverId(id);

				//subject
				subject = originalMessageRecord.value("subject").toString();
				messageWindow->SetSubject(subject);

				//content
				content = originalMessageRecord.value("content").toString();
				messageWindow->AddContent(originalMessageId,
										  curUserId, 
										  content,
										  dateTime,
										  originalMessageStatus);
			}

			int replyId = replyRecord.value("replyerId").toInt();
			content = replyRecord.value("content").toString();
			messageWindow->AddContent(replyRecord.value("id").toInt(),
									  replyId, 
									  content,
									  replyRecord.value("time").toDateTime(),
									  static_cast<SYMessageWindow::MessageStatus>(replyRecord.value("status").toInt()));
		}

		if(messageWindow)
			m_messageWindows.push_back(messageWindow);
	}
}

void SYMessageCenterWindow::AddMessageWindow(SYMessageWindow* messageWindow)
{
	QVBoxLayout* layout = static_cast<QVBoxLayout*>(ui.scrollAreaWidgetContents->layout());
	if(layout == nullptr){
		layout = new QVBoxLayout;
		layout->setContentsMargins(0,0,0,0);
		layout->setSpacing(10);
		ui.scrollAreaWidgetContents->setLayout(layout);
	}

	//layout->insertWidget(0, messageWindow);
	layout->addWidget(messageWindow);
}

void SYMessageCenterWindow::SortMessageWindowByDateTimeDesc()
{
	std::size_t nMessageWindow = m_messageWindows.size();
	if(nMessageWindow == 0)
		return;

	for(std::size_t i = 0; i < nMessageWindow - 1; ++i){
		for(std::size_t j = i + 1; j < nMessageWindow; ++j){
			if(m_messageWindows[j]->GetOriginalMessageDateTime() > m_messageWindows[i]->GetOriginalMessageDateTime()){
				SYMessageWindow* temp = m_messageWindows[i];
				m_messageWindows[i] = m_messageWindows[j];
				m_messageWindows[j] = temp; 
			}
		}
	}
}

void SYMessageCenterWindow::on_selectAllBtn_clicked()
{
	for(auto messageWindow : m_messageWindows)
		messageWindow->SetSelected(true);
}

void SYMessageCenterWindow::on_reverseSelecteBtn_clicked()
{
	for(auto messageWindow : m_messageWindows){
		bool isSelected = messageWindow->IsSelected();
		messageWindow->SetSelected(!isSelected);
	}
}

void SYMessageCenterWindow::on_deleteBtn_clicked()
{
	if(m_messageWindows.size() == 0)
		return;

	//
	for(auto& messageWindow : m_messageWindows){
		if(messageWindow->IsSelected()){
			messageWindow->DeleteMessage();
			messageWindow->deleteLater();
			messageWindow = nullptr;
		}
	}

	//
	for(auto itr = m_messageWindows.begin(); itr != m_messageWindows.end();){
		if(*itr == nullptr)
			itr = m_messageWindows.erase(itr);
		else
			++itr;
	}
}
#include "SYMessageWindow.h"
#include <MXDefine.h>
#include <SYDBMgr.h>
#include <QScrollBar>

SYMessageWindow::SYMessageWindow(QWidget* parent)
	:QWidget(parent),
	m_originalMessageSender(-1),
	m_originalMessageId(-1),
	m_originalMessageStatus(MS_Unread),
	m_receiverId(-1),
	m_senderId(-1),
	m_statusLabel(nullptr)
{
	ui.setupUi(this);

	m_statusLabel = new QLabel(ui.backgroundFrame);
	m_statusLabel->setObjectName("statusLabel");
	m_statusLabel->setVisible(false);

	ui.checkBox->installEventFilter(this);

	ui.contentFrame->setVisible(false);
	setFixedHeight(90);

	connect(ui.scrollArea->verticalScrollBar(), &QScrollBar::rangeChanged, this, &SYMessageWindow::onVerticalScrollBarRangeChanged);

	Mx::setWidgetStyle(this, "qss:/SYMessageWindow.qss");
}

SYMessageWindow::~SYMessageWindow()
{

}

void SYMessageWindow::onVerticalScrollBarRangeChanged(int min, int max)
{
	QScrollBar* scrollBar = ui.scrollArea->verticalScrollBar();
	int maxValue = scrollBar->maximum();
	scrollBar->setValue(max);
}

bool SYMessageWindow::IsSelected() const
{
	return ui.checkBox->isChecked();
}

void SYMessageWindow::SetSelected(bool selected)
{
	ui.checkBox->setChecked(selected);
}

void SYMessageWindow::SetSubject(const QString& subject)
{
	ui.subjectLabel->setText(subject);
}

void SYMessageWindow::SetOriginalMessageInfo(int originalMessageSender,
											 int originalMessageId,
											 MessageStatus originalMessageStatus,
											 const QDateTime& originalTime)
{
	m_originalMessageSender = originalMessageSender;
	m_originalMessageId = originalMessageId;
	m_originalMessageStatus = originalMessageStatus;
	m_originalMessageDateTime = originalTime;
	ui.dateTimeLabel->setText(originalTime.toString(CHS("yyyy年MM月dd日  HH:mm")));
}

void SYMessageWindow::SetSenderId(int id)
{
	m_senderId = id;
}

void SYMessageWindow::SetReceiverId(int id)
{
	m_receiverId = id;
}

void SYMessageWindow::AddContent(int messageId, int userId, const QString& content, const QDateTime& curDateTime, MessageStatus status)
{
	QVBoxLayout* contentsLayout = static_cast<QVBoxLayout*>(ui.scrollAreaWidgetContents->layout());
	QSpacerItem* lastSpacerItem = nullptr;
	if(contentsLayout == nullptr){
		contentsLayout = new QVBoxLayout;
		contentsLayout->setSpacing(10);
		contentsLayout->setContentsMargins(0, 0, 0, 0);
		ui.scrollAreaWidgetContents->setLayout(contentsLayout);
	}
	else{
		int nItem = contentsLayout->count();
		if(nItem > 0){
			QLayoutItem* layoutItem = contentsLayout->itemAt(nItem - 1);
			lastSpacerItem = dynamic_cast<QSpacerItem*>(layoutItem);
			if(lastSpacerItem){
				contentsLayout->takeAt(nItem - 1);
			}
		}
	}

// 	QVBoxLayout* vLayout = new QVBoxLayout;
// 	QLabel* nameLabel = new QLabel;
// 	vLayout->addWidget(nameLabel);
// 	vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
// 
// 	QHBoxLayout* hLayout = new QHBoxLayout;
// 	hLayout->setSpacing(6);
// 	QLabel* contentLabel = new QLabel(content);
// 
// 	if(userId == m_senderId){
// 		nameLabel->setText(CHS("你"));
// 		nameLabel->setObjectName("leftNameLabel");
// 		nameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
// 		contentLabel->setObjectName("leftContentLabel");
// 		contentLabel->setAlignment(Qt::AlignLeft);
// 
// 		hLayout->addLayout(vLayout);
// 		hLayout->addWidget(contentLabel);
// 		hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
// 	}
// 	else{
// 		nameLabel->setText(CHS("对方"));
// 		nameLabel->setObjectName("rightNameLabel");
// 		nameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
// 		contentLabel->setObjectName("rightContentLabel");
// 		contentLabel->setAlignment(Qt::AlignRight);
// 
// 		hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
// 		hLayout->addWidget(contentLabel);
// 		hLayout->addLayout(vLayout);
// 	}
// 
// 	contentsLayout->addLayout(hLayout);

	//content
	QHBoxLayout* hLayout1 = new QHBoxLayout;
	QLabel* contentLabel = new QLabel(content);
	contentLabel->setObjectName("contentLabel");
	hLayout1->addWidget(contentLabel);
	hLayout1->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	
	//name
	QHBoxLayout* hLayout2 = new QHBoxLayout;
	QLabel* nameLabel = new QLabel();
	nameLabel->setObjectName("nameLabel");
	if(m_senderId == userId){
		nameLabel->setText(CHS("你"));
	}
	else{
		nameLabel->setText(CHS("对方"));
	}

	hLayout2->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout2->addWidget(nameLabel);

	//date
	QLabel* dateLabel = new QLabel;
	dateLabel->setObjectName("dateLabel");
	dateLabel->setText(curDateTime.toString(CHS("yyyy年MM月dd日  HH:mm")));
	QHBoxLayout* hLayout3 = new QHBoxLayout;
	hLayout3->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout3->addWidget(dateLabel);

	//frame and frame's layout
	QVBoxLayout* frameLayout = new QVBoxLayout;
	frameLayout->addLayout(hLayout1);
	frameLayout->addLayout(hLayout2);
	frameLayout->addLayout(hLayout3);

	QFrame* frame = new QFrame;
	frame->setObjectName("recordFrame");
	frame->setLayout(frameLayout);

	//add frame to contents layout
	contentsLayout->addWidget(frame);

	if(lastSpacerItem == nullptr)
		lastSpacerItem = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	contentsLayout->addSpacerItem(lastSpacerItem);

	//set status label visibility
	bool labelIsVisible = false;

	if(messageId == m_originalMessageId){
		if(status == MS_Unread && m_senderId != m_originalMessageSender)
			labelIsVisible = true;
	}
	else{
		if(status == MS_Unread && userId != m_senderId){
			labelIsVisible = true;
			m_unreadMessageReplyIds.push_back(messageId);
		}
	}

	if(labelIsVisible)
		m_statusLabel->setVisible(true);
}

void SYMessageWindow::DeleteMessage()
{
	int newStatus = static_cast<int>(m_originalMessageStatus);
	if(m_originalMessageSender == m_senderId)
		newStatus |= MS_DeletedBySender;
	else
		newStatus |= MS_DeletedByReceiver;

	if(SYDBMgr::Instance()->UpdateMessageStatus(m_originalMessageId, newStatus))
		m_originalMessageStatus = static_cast<MessageStatus>(newStatus);
}

void SYMessageWindow::on_sendBtn_clicked()
{
	if(m_receiverId == -1){
		qDebug() << "receiver id is -1";
		return;
	}

	QString content = ui.textEdit->toPlainText();
	if(content.size() == 0)
		return;

	//write data to database
	if(m_originalMessageId == -1){
		qDebug() << "message id is -1";
		return;
	}

	SYDBMgr::Instance()->AddMessageReply(m_originalMessageId, m_senderId, content);

	if(m_originalMessageStatus & (MS_DeletedBySender | MS_DeletedByReceiver)){
		int newStatus = static_cast<int>(m_originalMessageStatus)& (~(MS_DeletedBySender | MS_DeletedByReceiver));
		if(SYDBMgr::Instance()->UpdateMessageStatus(m_originalMessageId, newStatus))
			m_originalMessageStatus = static_cast<MessageStatus>(newStatus);
	}

	//show
	AddContent(0, m_senderId, content, QDateTime::currentDateTime(), MS_BeRead);
	
	//clear
	ui.textEdit->clear();
}

void SYMessageWindow::on_foldBtn_clicked()
{
	bool visible = !ui.contentFrame->isVisible();
	ui.contentFrame->setVisible(visible);

	if(visible){
		setFixedHeight(570);

		//update status
		if(m_originalMessageStatus == MS_Unread && m_originalMessageSender != m_senderId){
			//更新消息为已读状态
			if(SYDBMgr::Instance()->UpdateMessageStatus(m_originalMessageId, MS_BeRead)){
				m_originalMessageStatus = MS_BeRead;
			}
		}

		for(auto itr = m_unreadMessageReplyIds.begin(); itr != m_unreadMessageReplyIds.end();){
			int id = *itr;
			if(SYDBMgr::Instance()->UpdateMessageReplyStatus(id, MS_BeRead)){
				itr = m_unreadMessageReplyIds.erase(itr);
			}
			else
				++itr;
		}

		m_statusLabel->setVisible(false);
	}
	else
		setFixedHeight(90);
}

bool SYMessageWindow::eventFilter(QObject* obj, QEvent* event)
{
	if(event->type() == QEvent::Move && obj == ui.checkBox){
		if(m_statusLabel){
			static const QSize offset(25, -7);
			QPoint ptCheckBox = ui.checkBox->pos();
			QSize statusLabelSize = m_statusLabel->size();

			m_statusLabel->move(ptCheckBox.x() - statusLabelSize.width() - offset.width(),
								ptCheckBox.y() - offset.height());
		}
	}

	return QWidget::eventFilter(obj, event);
}

void SYMessageWindow::showEvent(QShowEvent* event)
{
//  	if(m_originalMessageStatus == 0 && m_originalMessageSender != m_senderId)
//  		m_statusLabel->setVisible(true);
}
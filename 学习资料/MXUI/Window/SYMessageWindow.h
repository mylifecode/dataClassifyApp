#pragma once
#include <QWidget>
#include <QDateTime>
#include "ui_SYMessageWindow.h"


class SYMessageWindow : public QWidget
{
	Q_OBJECT
public:
	enum MessageStatus
	{
		MS_Unread								= 0,
		MS_BeRead								= 0x01,
		MS_DeletedBySender						= 0x01 << 1,
		MS_DeletedByReceiver					= 0x01 << 2,
		MS_BeRead_DeletedBySender				= MS_BeRead | MS_DeletedBySender,
		MS_BeRead_DeletedByReceiver				= MS_BeRead | MS_DeletedByReceiver,
		MS_BeRead_DeletedBySenderAndReceiver	= MS_BeRead | MS_DeletedBySender | MS_DeletedByReceiver
	};

	SYMessageWindow(QWidget* parent = nullptr);

	~SYMessageWindow();

	bool IsSelected() const;

	void SetSelected(bool selected);

	void SetSubject(const QString& subject);

	void SetOriginalMessageInfo(int originalMesageSender,
								int originalMessageId,
								MessageStatus originalMessageStatus,
								const QDateTime& originalTime);

	void SetSenderId(int id);

	void SetReceiverId(int id);

	const QDateTime& GetOriginalMessageDateTime() const { return m_originalMessageDateTime; }

	/**
		status:标记该条消息或回复是否已经被读，0表示未读，否则已读
	*/
	void AddContent(int messageId, int userId, const QString& content, const QDateTime& curDateTime, MessageStatus status);

	void DeleteMessage();

private slots:
	void on_sendBtn_clicked();

	void on_foldBtn_clicked();

	void onVerticalScrollBarRangeChanged(int min, int max);

protected:
	bool eventFilter(QObject* obj, QEvent* event);

	void showEvent(QShowEvent* event);

private:
	Ui::MessageWindow ui;

	/// 消息发起者id
	int m_originalMessageSender;
	/// 消息发起者的第一条消息id
	int m_originalMessageId;
	/// 消息发起者的第一条消息是否被读
	MessageStatus m_originalMessageStatus;
	/// 消息发起者的第一条的发送时间
	QDateTime m_originalMessageDateTime;


	/// 本次消息回复中发送者id
	int m_senderId;
	/// 本次消息回复中接受者id
	int m_receiverId;

	/// 用于标记消息是否未读
	QLabel* m_statusLabel;

	std::list<int> m_unreadMessageReplyIds;
};


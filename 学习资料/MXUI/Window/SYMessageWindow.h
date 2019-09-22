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
		status:��Ǹ�����Ϣ��ظ��Ƿ��Ѿ�������0��ʾδ���������Ѷ�
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

	/// ��Ϣ������id
	int m_originalMessageSender;
	/// ��Ϣ�����ߵĵ�һ����Ϣid
	int m_originalMessageId;
	/// ��Ϣ�����ߵĵ�һ����Ϣ�Ƿ񱻶�
	MessageStatus m_originalMessageStatus;
	/// ��Ϣ�����ߵĵ�һ���ķ���ʱ��
	QDateTime m_originalMessageDateTime;


	/// ������Ϣ�ظ��з�����id
	int m_senderId;
	/// ������Ϣ�ظ��н�����id
	int m_receiverId;

	/// ���ڱ����Ϣ�Ƿ�δ��
	QLabel* m_statusLabel;

	std::list<int> m_unreadMessageReplyIds;
};


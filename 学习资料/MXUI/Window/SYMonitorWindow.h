#pragma once
#include "SYMonitorSubWindow.h"
#include "ui_SYMonitorWindow.h"
#include <QVector>
#include <QMap>
#include <MxRtpSession.h>
#include <MxSessionListener.h>
#include <MxDecodeListener.h>
#include <MxVideoDecoder.h>


class SYMonitorButton;

class SYMonitorWindow : public QWidget,
						public MxSessionListener,
						public MxDecodeListener
{
	Q_OBJECT
public:
	SYMonitorWindow(QWidget * parent = NULL);
	~SYMonitorWindow(void);

	struct UserInfo
	{
		UserInfo()
			:m_userIp(0),
			m_monitorButton(nullptr),
			m_monitorWindow(nullptr),
			m_decoderId(0)
		{

		}

		QString GetDisplayInfo()
		{
			if(m_realName.size())
				return m_userName + "(" + m_realName + ")";
			else
				return m_userName;
		}

		bool HasDisplayInfo() { return m_userName.size(); }

		uint32_t  m_userIp;
		std::shared_ptr<MxRtpPacketBuffer> m_bufferPtr;
		SYMonitorButton* m_monitorButton;
		SYMonitorSubWindow* m_monitorWindow;
		uint32_t m_decoderId;
		QString m_userName;
		QString m_realName;
	};

signals:
	void RequestAddMonitorButtonToLayout(SYMonitorButton* button);
	void RequestRemoveMonitorButtonFromLayout(SYMonitorButton* button);
	void RequestDestroyMonitorButton(SYMonitorButton* button);
	void RequestUpdateDisplayInfo(unsigned int ip);

protected slots:

	void onWatch();
	void onDeleteMonitorButton();
	void onExitWatch();

	/** 改变监控子窗口的布局：6窗口 <<===>> 9窗口 */
	void OnChangedSubWindowLayout();

	/** 更新控件的显示信息 */
	void OnUpdateDisplayInfo(unsigned int sourceIp);

	void AddMonitorButtonToLayout(SYMonitorButton* button);
	void RemoveMonitorButtonFromLayout(SYMonitorButton* button);

	void DestroyMonitorButton(SYMonitorButton* button);

	

private:

	SYMonitorButton* CreateMonitorButton(uint32_t ip);

	SYMonitorSubWindow* CreateMonitorSubWindow(uint32_t ip);
	void DestroyMonitorSubWindow(SYMonitorSubWindow* window);

	virtual void OnNewSource(uint32_t sourceIp, const std::shared_ptr<MxRtpPacketBuffer>& bufferPtr);
	virtual void OnRemoveSource(uint32_t sourceIp);
	virtual void OnReceiveCommand(uint32_t sourceIp, const uint8_t* data, uint32_t length);
	virtual void OnDecodeFinish(uint32_t decoderId, const MxVideoFrame& videoFrame);

protected:
	virtual bool eventFilter(QObject *pObject, QEvent * pEvent);

signals:
	void AddButton();
private:
	Ui::SYMonitorWindow ui;
	MxRtpSession* m_rtpSession;
// 	RbMonitorTipsWindow *m_pTipsWindow;																	//提示窗口

	QPoint m_startPoint;
	QPoint m_endPoint;
	bool m_canMove;
	int m_value;
	QGridLayout* layout;

	QMap<uint32_t, UserInfo*> m_userInfoMap;
	QMutex m_userInfoMapMutex;

	QVBoxLayout* m_monitorButtonsLayout;
	QMutex m_layoutMutex;

	bool m_hasInitLayout;
	/// default: false
	bool m_isSixGridLayout;
	QVector<SYMonitorSubWindow*> m_allSubWindow;
	QMutex m_subWindowMutex;

	/// 剩余可用的监控按钮
	QVector<SYMonitorButton*> m_remainMonitorButtons;
	QMutex m_monitorButtonMutex;

	MxVideoDecoder* m_decoder;
	uint32_t m_decodeDelayTime;
};
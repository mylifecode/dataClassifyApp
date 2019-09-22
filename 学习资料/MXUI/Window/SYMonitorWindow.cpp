#include "SYMonitorWindow.h"
#include "SYMonitorButton.h"
#include "SYMonitorSubWindow.h"
#include "SYMessageBox.h"
#include "MxDefine.h"
#include "MXGlobalConfig.h"
#include "MxSessionCommand.h"
#include <QDebug>
#include <QMutexLocker>
#include <QScrollBar>
#include <MxRTPConfig.h>


SYMonitorWindow::SYMonitorWindow(QWidget * parent)
	:QWidget(parent),
	m_rtpSession(nullptr),
	m_canMove(false),
	m_monitorButtonsLayout(nullptr),
	m_hasInitLayout(false),
	m_isSixGridLayout(false),
	m_decoder(nullptr),
	m_decodeDelayTime(0)
{
	ui.setupUi(this);

	Mx::setWidgetStyle(this, "qss:SYMonitorWindow.qss");

	//初始化滚动区域的布局
	m_monitorButtonsLayout = new QVBoxLayout;  //
	m_monitorButtonsLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	m_monitorButtonsLayout->setMargin(25);
	ui.scrollAreaWidgetContents->setLayout(m_monitorButtonsLayout);  //按钮列表设置布局

	//初始化监控按钮
 	const int nMonitorButton = 20;		//待改进
 	m_remainMonitorButtons.reserve(nMonitorButton);
 	for(int i = 0; i < nMonitorButton; ++i)
 		m_remainMonitorButtons.push_back(new SYMonitorButton);

	//创建监控子窗口
	SYMonitorSubWindow* subWindow;
	for(int i = 0; i < 9; ++i)
	{
		subWindow = new SYMonitorSubWindow(nullptr);
		subWindow->SetExtraData(0);
		m_allSubWindow.push_back(subWindow);
	}
 	OnChangedSubWindowLayout();
 
 	//connect signals
 	connect(this, SIGNAL(RequestAddMonitorButtonToLayout(SYMonitorButton*)), this, SLOT(AddMonitorButtonToLayout(SYMonitorButton*)));
 	connect(this, SIGNAL(RequestRemoveMonitorButtonFromLayout(SYMonitorButton*)), this, SLOT(RemoveMonitorButtonFromLayout(SYMonitorButton*)));
 	connect(this, SIGNAL(RequestDestroyMonitorButton(SYMonitorButton*)), this, SLOT(DestroyMonitorButton(SYMonitorButton*)));
 	connect(this, SIGNAL(RequestUpdateDisplayInfo(unsigned int)), this, SLOT(OnUpdateDisplayInfo(unsigned int)));
 	//connect(ui.layoutBtn, SIGNAL(clicked()), this, SLOT(OnChangedSubWindowLayout()));
  	ui.scrollArea->viewport()->installEventFilter(this);  //滚动条
	ui.scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  	ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

 	//显示提示窗口
//  	m_pTipsWindow = new RbMonitorTipsWindow(this);
//  	m_pTipsWindow->show();

	int serverPort = MxGlobalConfig::Instance()->GetServerPort();
	const QString& multicastAddr = MxGlobalConfig::Instance()->GetMulticastGroupAddress();
	m_decodeDelayTime = MxGlobalConfig::Instance()->GetDecodeDelayTime();

	//create rtp session
	m_rtpSession = new MxRtpSession;
	m_rtpSession->SetLocalSessionPort(serverPort);

 	if(m_rtpSession->Initialize())
 	{
 		m_rtpSession->JoinMulticastGroup(multicastAddr.toStdString(), serverPort);
 		m_rtpSession->SetSessionListener(this);
 	}
 	else
 	{
 		SYMessageBox* msgBox = new SYMessageBox(this, CHS("提示"), CHS("初始化失败，请重试"));
 		msgBox->showFullScreen();
 		msgBox->exec();
 	}

	//create decoder
	m_decoder = new MxVideoDecoder;
	m_decoder->AddListener(this);
}

SYMonitorWindow::~SYMonitorWindow(void)
{
	//1 close session
	m_rtpSession->BYEDestroy(1, "exit", 4);
	m_rtpSession->ClearDestinations();
	m_rtpSession->LeaveAllMulticastGroups();
	m_rtpSession->SetSessionListener(nullptr);
	delete m_rtpSession;

	//2 clear user info map
	m_userInfoMapMutex.lock();
	for(auto e : m_userInfoMap)
	{
		e->m_bufferPtr.reset();

		if(e->m_monitorButton)
			DestroyMonitorButton(e->m_monitorButton);

		if(e->m_monitorWindow)
		{
			DestroyMonitorSubWindow(e->m_monitorWindow);
			m_decoder->StopDecode(e->m_decoderId);
		}

		if(e->m_decoderId)
			m_decoder->DestroyH264Decoder(e->m_decoderId);

		//last free memory
		delete e;
	}
	m_userInfoMap.clear();
	m_userInfoMapMutex.unlock();

	//3 close
	m_decoder->RemoveListener(this);
	delete m_decoder;

	//
	m_monitorButtonMutex.lock();
	for(auto e : m_remainMonitorButtons)
		delete e;
	m_remainMonitorButtons.clear();
	m_monitorButtonMutex.unlock();

	//
	m_subWindowMutex.lock();
	for(auto e : m_allSubWindow)
		DestroyMonitorSubWindow(e);
	m_allSubWindow.clear();
	m_subWindowMutex.unlock();

	// 	删除提示窗口的指针
	// 		if (m_pTipsWindow)
	// 		{
	// 			delete m_pTipsWindow;
	// 			m_pTipsWindow = NULL;
	// 		}
}

void SYMonitorWindow::onWatch() //显示学生监控信息到子控制窗口
{
	QMutexLocker locker(&m_userInfoMapMutex);
	SYMonitorButton * pButton = static_cast<SYMonitorButton *>(sender());
	uint32_t ip = pButton->GetExtraData();
	
	if(ip == 0)
		return;

	//如果查找未成功我们不必再移除该对象
	auto itr = m_userInfoMap.find(ip);
	if(itr != m_userInfoMap.end())
	{
		UserInfo* userInfo = itr.value();

		if(userInfo->m_monitorButton == nullptr)
			return;

		userInfo->m_monitorWindow = CreateMonitorSubWindow(ip);

		if(userInfo->m_monitorWindow)
		{
			if(userInfo->m_decoderId == 0)
			{
				userInfo->m_decoderId = m_decoder->CreateH264Decoder();						//总会创建成功
				m_decoder->SetDecodeDelayTime(userInfo->m_decoderId, m_decodeDelayTime);
			}
				
			m_decoder->SetDynamicBuffer(userInfo->m_decoderId, userInfo->m_bufferPtr);		//记得关闭先前的缓存
			m_decoder->StartDecode(userInfo->m_decoderId);

			//1 销毁监控按钮
			userInfo->m_monitorButton = nullptr;
			RemoveMonitorButtonFromLayout(pButton);
			DestroyMonitorButton(pButton);
			//emit RequestRemoveMonitorButtonFromLayout(pButton);
			//emit RequestDestroyMonitorButton(pButton);

			//2 set display info
			userInfo->m_monitorWindow->setName(userInfo->GetDisplayInfo());  //设置监控视频文本信息
		}
		else
		{
			locker.unlock();
			SYMessageBox* box = new SYMessageBox(this, CHS("提示"), CHS("当前无窗口用于显示"));
			box->showFullScreen();
			box->exec();
		}
	}
}

void SYMonitorWindow::onDeleteMonitorButton()
{
	return;

	QMutexLocker locker(&m_userInfoMapMutex);
	UserInfo* userInfo = nullptr;
	SYMonitorButton * pButton = static_cast<SYMonitorButton * >(sender());
	uint32_t ip = pButton->GetExtraData();

	if(ip == 0)
		return;

	auto itr = m_userInfoMap.find(ip);
	if(itr != m_userInfoMap.end())
	{
		userInfo = itr.value();
		//remove from layout
		RemoveMonitorButtonFromLayout(userInfo->m_monitorButton);
		DestroyMonitorButton(pButton);
		//emit RequestRemoveMonitorButtonFromLayout(pButton);
		//emit RequestDestroyMonitorButton(pButton);
		userInfo->m_monitorButton = nullptr;
	}
}

void SYMonitorWindow::onExitWatch()
{
	QMutexLocker locker(&m_userInfoMapMutex);
	SYMonitorSubWindow * pSubWindow = (SYMonitorSubWindow*)sender();
	uint32_t ip = pSubWindow->GetExtraData();

	if(ip == 0)
		return;
	
	auto itr = m_userInfoMap.find(ip);
	if(itr != m_userInfoMap.end())
	{
		UserInfo* userInfo = itr.value();
		//1 add button
		userInfo->m_monitorButton = CreateMonitorButton(userInfo->m_userIp);
		Q_ASSERT(userInfo->m_monitorButton && "no more available monitor button.");
		userInfo->m_monitorButton->setInfo(userInfo->GetDisplayInfo());
		AddMonitorButtonToLayout(userInfo->m_monitorButton);
		//2 stop decode
		m_decoder->StopDecode(userInfo->m_decoderId);
		//3 hide sub window
		DestroyMonitorSubWindow(userInfo->m_monitorWindow);
		userInfo->m_monitorWindow = nullptr;
	}
}

void SYMonitorWindow::OnChangedSubWindowLayout()  //6-》9
{
	//the first
	if(m_hasInitLayout)//默认false
	{
		SYMessageBox * messageBox = new SYMessageBox(this, "", CHS("是否切换布局？"), 2);
		messageBox->showFullScreen();
		if(messageBox->exec() != 2)
			return;
	}
	else
		m_hasInitLayout = true;

	//destroy all sub window
	QMutexLocker locker(&m_userInfoMapMutex);  //线程锁
	for(auto e : m_userInfoMap)
	{
		UserInfo* userInfo = e;
		if(userInfo->m_monitorWindow)
		{
			//1 add button
			userInfo->m_monitorButton = CreateMonitorButton(userInfo->m_userIp);  //添加控制按键额外的数据
			Q_ASSERT(userInfo->m_monitorButton && "no more available monitor button.");//都为true通过
			userInfo->m_monitorButton->setInfo(userInfo->GetDisplayInfo());
			AddMonitorButtonToLayout(userInfo->m_monitorButton);
			//2 stop decode
			m_decoder->StopDecode(userInfo->m_decoderId);  //解码看不懂
			//3 hide sub window
			DestroyMonitorSubWindow(userInfo->m_monitorWindow);
			userInfo->m_monitorWindow = nullptr;//
		}
	}

	//lock
	m_subWindowMutex.lock();

	layout = static_cast<QGridLayout*>(ui.monitorWidget->layout());  //获得当前窗口的布局
	if(layout == nullptr)
	{
		layout = new QGridLayout(ui.monitorWidget);//网格布局指定监控窗口为父类
	}
	else
	{
		for(auto widget : m_allSubWindow)
		{
			widget->SetExtraData(0);
			layout->removeWidget(widget);
			widget->move(10000, 10000);
		}
	}
	
	int n = 2;
	if(m_isSixGridLayout) //false
	{
		//convert to nine grid
		n = 3;
		//ui.layoutBtn->setChecked(true);
	}
	else
	{
		//ui.layoutBtn->setChecked(false);
	}
		
	for(int row = 0; row < n; ++row)
	{
		for(int col = 0; col < 3; ++col)
		{
			int index = row * 3 + col;
			Q_ASSERT(index < m_allSubWindow.size());//索引不能超过9
			SYMonitorSubWindow* subWindow = m_allSubWindow[index];
			layout->addWidget(m_allSubWindow[row * 3 + col],row, col);//网格布局
		}
	}
	layout->setContentsMargins(92, 0,0, 0);
	layout->setSpacing(80);
	m_isSixGridLayout = !m_isSixGridLayout;
	//unlock
	m_subWindowMutex.unlock();
}

void SYMonitorWindow::OnUpdateDisplayInfo(unsigned int sourceIp)
{
	m_userInfoMapMutex.lock();
	auto itr = m_userInfoMap.find(sourceIp);
	if(itr != m_userInfoMap.end())
	{
		UserInfo* userInfo = itr.value();
		QString displayInfo = userInfo->GetDisplayInfo();

		if(userInfo->m_monitorButton)
		{
			userInfo->m_monitorButton->setInfo(displayInfo);
			userInfo->m_monitorButton->setVisible(true);		//当有用户信息到达的时候才显示此按钮
		}

		if(userInfo->m_monitorWindow)
			userInfo->m_monitorWindow->setName(displayInfo);
	}
	m_userInfoMapMutex.unlock();
}

bool SYMonitorWindow::eventFilter(QObject *pObject, QEvent * pEvent)
{
	if (pObject == ui.scrollArea->viewport())  //提取内容
	{
		QWidget* pViewport = static_cast<QWidget*>(pObject);
		QMouseEvent* pMouseEvent = NULL;
		switch(pEvent->type())
		{
		case QEvent::MouseButtonPress:
			pMouseEvent = static_cast<QMouseEvent*>(pEvent);
			m_canMove = true;
			m_startPoint = pMouseEvent->pos();
			break;
		case QEvent::MouseMove:
			pMouseEvent = static_cast<QMouseEvent*>(pEvent);
			
			if (m_canMove)
			{	
				QScrollBar* pScrollBar = ui.scrollArea->verticalScrollBar();
				m_value = pScrollBar->value();
				if (pScrollBar)
				{
					m_endPoint = pMouseEvent->pos();
					int dy = m_endPoint.y() - m_startPoint.y();
					float moveUnit = 0.01f;
					int value = m_value - dy * moveUnit;
					if (value < pScrollBar->minimum())
					{
						m_startPoint = m_endPoint;
						value = pScrollBar->minimum();
					}
					else if (value > pScrollBar->maximum())
					{
						m_startPoint = m_endPoint;
						value = pScrollBar->maximum();
					}
					pScrollBar->setValue(value);
				}
			}
			break;
		case QEvent::MouseButtonRelease:
			m_canMove = false;
			break;
		default:
			break;
		}
	}

	return __super::eventFilter(pObject,pEvent);
}

void SYMonitorWindow::OnNewSource(uint32_t sourceIp, const std::shared_ptr<MxRtpPacketBuffer>& bufferPtr)
{
	m_userInfoMapMutex.lock();

	auto itr = m_userInfoMap.find(sourceIp);
	UserInfo* userInfo = nullptr;

	if(itr == m_userInfoMap.end())
	{
		//init user info
		userInfo = new UserInfo;
		m_userInfoMap.insert(sourceIp, userInfo);
	}
	else
		userInfo = itr.value();

	userInfo->m_userIp = sourceIp;
	userInfo->m_bufferPtr = bufferPtr;

	//add monitor button
	if(userInfo->m_monitorButton)
		userInfo->m_monitorButton->SetExtraData(sourceIp);
	else
	{
		userInfo->m_monitorButton = CreateMonitorButton(sourceIp);
		Q_ASSERT(userInfo->m_monitorButton && "no more available monitor button.");
		if(userInfo->HasDisplayInfo())
			userInfo->m_monitorButton->setInfo(userInfo->GetDisplayInfo());
		else
			userInfo->m_monitorButton->setVisible(false);
		//AddMonitorButtonToLayout(userInfo->m_monitorButton);
		emit RequestAddMonitorButtonToLayout(userInfo->m_monitorButton);
	}

	if(userInfo->m_monitorWindow)
	{
		DestroyMonitorSubWindow(userInfo->m_monitorWindow);
		userInfo->m_monitorWindow = nullptr;
	}

	if(userInfo->m_decoderId)
		m_decoder->StopDecode(userInfo->m_decoderId);

	m_userInfoMapMutex.unlock();
}

void SYMonitorWindow::OnRemoveSource(uint32_t sourceIp)
{
	m_userInfoMapMutex.lock();
	auto itr = m_userInfoMap.find(sourceIp);
	UserInfo* userInfo = nullptr;
	
	if(itr != m_userInfoMap.end())
	{
		userInfo = itr.value();
		userInfo->m_bufferPtr.reset();

		if(userInfo->m_monitorButton)
		{
			//RemoveMonitorButtonFromLayout(userInfo->m_monitorButton);
			//DestroyMonitorButton(userInfo->m_monitorButton);
			emit RequestRemoveMonitorButtonFromLayout(userInfo->m_monitorButton);
			emit RequestDestroyMonitorButton(userInfo->m_monitorButton);
		}

		if(userInfo->m_monitorWindow)
			DestroyMonitorSubWindow(userInfo->m_monitorWindow);

		if(userInfo->m_decoderId)
			m_decoder->DestroyH264Decoder(userInfo->m_decoderId);

		delete userInfo;

		m_userInfoMap.erase(itr);
	}
	m_userInfoMapMutex.unlock();
}

void SYMonitorWindow::OnReceiveCommand(uint32_t sourceIp, const uint8_t* data, uint32_t length)
{
	QMutexLocker locker(&m_userInfoMapMutex);

	auto itr = m_userInfoMap.find(sourceIp);
	if(itr == m_userInfoMap.end())
		return;

	bool isOk = false;
	QByteArray byteData((const char*)data, length);
	MxSessionCommand sessionCommand = MxSessionCommand::FromByteArray(byteData,&isOk);
	UserInfo* userInfo = itr.value();

	if(isOk)
	{
		switch(sessionCommand.Type())
		{
		case MxSessionCommand::CT_Update:
			{
				bool changed = false;
				QString data = sessionCommand.GetData("UserName");
				if(data != userInfo->m_userName)
				{
					userInfo->m_userName = data;
					changed = true;
				}

				data = sessionCommand.GetData("RealName");
				if(data != userInfo->m_realName)
				{
					userInfo->m_realName = data;
					changed = true;
				}

				if(changed)
					emit RequestUpdateDisplayInfo(sourceIp);
			}
			break;
		case MxSessionCommand::CT_Exit:
			{
				m_userInfoMap.erase(itr);
				locker.unlock();

				userInfo->m_bufferPtr.reset();

				if(userInfo->m_monitorButton)
				{
					//RemoveMonitorButtonFromLayout(userInfo->m_monitorButton);
					//DestroyMonitorButton(userInfo->m_monitorButton);
					emit RequestRemoveMonitorButtonFromLayout(userInfo->m_monitorButton);
					emit RequestDestroyMonitorButton(userInfo->m_monitorButton);
				}

				if(userInfo->m_monitorWindow)
					DestroyMonitorSubWindow(userInfo->m_monitorWindow);

				if(userInfo->m_decoderId)
					m_decoder->DestroyH264Decoder(userInfo->m_decoderId);

				delete userInfo;
			}
			break;
		}
	}
}

void SYMonitorWindow::OnDecodeFinish(uint32_t decoderId, const MxVideoFrame& videoFrame)
{
	QMutexLocker locker(&m_userInfoMapMutex);

	auto itr = std::find_if(m_userInfoMap.begin(), m_userInfoMap.end(), [decoderId](UserInfo* userInfo)->bool{
		if(userInfo && userInfo->m_decoderId == decoderId)
			return true;
		else
			return false;
	});

	if(itr != m_userInfoMap.end())
	{
		UserInfo* userInfo = itr.value();
		if(userInfo->m_monitorWindow)
		{
			userInfo->m_monitorWindow->updateWindow(videoFrame);
		}
	}
}

SYMonitorButton* SYMonitorWindow::CreateMonitorButton(uint32_t ip)
{
	SYMonitorButton* button = nullptr;

	m_monitorButtonMutex.lock();

	if(m_remainMonitorButtons.size())
	{
		button = m_remainMonitorButtons.takeLast();
		button->SetExtraData(ip);
		connect(button, SIGNAL(watch()), this, SLOT(onWatch()));
		connect(button, SIGNAL(stop()), this, SLOT(onDeleteMonitorButton()));
	}

	m_monitorButtonMutex.unlock();

	return button; 
}

void SYMonitorWindow::DestroyMonitorButton(SYMonitorButton* button)
{
	if(button)
	{
		m_monitorButtonMutex.lock();

		button->setInfo("");
		button->SetExtraData(0);
		disconnect(button, SIGNAL(watch()), this, SLOT(onWatch()));
		disconnect(button, SIGNAL(stop()), this, SLOT(onDeleteMonitorButton()));
		
		m_remainMonitorButtons.push_back(button);

		m_monitorButtonMutex.unlock();
	}
}

void SYMonitorWindow::AddMonitorButtonToLayout(SYMonitorButton* button)
{
	if(button)
	{
		m_layoutMutex.lock();
		m_monitorButtonsLayout->insertWidget(0, button);
		m_layoutMutex.unlock();
	}
}

void SYMonitorWindow::RemoveMonitorButtonFromLayout(SYMonitorButton* button)
{
	if(button)
	{
		m_layoutMutex.lock();
		m_monitorButtonsLayout->removeWidget(button);
		button->move(10000, 10000);
		m_layoutMutex.unlock();
	}
}

SYMonitorSubWindow* SYMonitorWindow::CreateMonitorSubWindow(uint32_t ip)
{
	SYMonitorSubWindow* subWindow = nullptr;
	int n = 0;

	//lock
	m_subWindowMutex.lock();

	if(m_isSixGridLayout)
		n = 6;
	else
		n = 9;

	//get available sub window
	for(int i = 0; i < n; ++i)
	{
		if(m_allSubWindow[i]->GetExtraData() == 0)
		{
			subWindow = m_allSubWindow[i];
			break;
		}
	}

	if(subWindow)
	{
		subWindow->SetExtraData(ip);
		subWindow->showControl();
		connect(subWindow, SIGNAL(exitWatch()), this, SLOT(onExitWatch()));
	}
		
	//unlock
	m_subWindowMutex.unlock();

	return subWindow;
}

void SYMonitorWindow::DestroyMonitorSubWindow(SYMonitorSubWindow* window)
{
	if(window)
	{
		window->SetExtraData(0);
		window->setName("");
		window->hideControl();
		disconnect(window, SIGNAL(exitWatch()), this, SLOT(onExitWatch()));
	}
}
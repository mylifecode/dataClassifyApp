#include "Network.h"
#include "MxDefine.h"
#include <QDebug>
#include "QInputDialog"
#include "MxGlobalConfig.h"
char *w2c(char *pcstr,const wchar_t *pwstr, size_t len)

{

	int nlength=wcslen(pwstr);

	//获取转换后的长度

	int nbytes = WideCharToMultiByte( 0, // specify the code page used to perform the conversion

		0,         // no special flags to handle unmapped characters

		pwstr,     // wide character string to convert

		nlength,   // the number of wide characters in that string

		NULL,      // no output buffer given, we just want to know how long it needs to be

		0,

		NULL,      // no replacement character given

		NULL );    // we don't want to know if a character didn't make it through the translation

	// make sure the buffer is big enough for this, making it larger if necessary

	if(nbytes>len)   nbytes=len;

	// 通过以上得到的结果，转换unicode 字符为ascii 字符

	WideCharToMultiByte( 0, // specify the code page used to perform the conversion

		0,         // no special flags to handle unmapped characters

		pwstr,   // wide character string to convert

		nlength,   // the number of wide characters in that string

		pcstr, // put the output ascii characters at the end of the buffer

		nbytes,                           // there is at least this much space there

		NULL,      // no replacement character given

		NULL );

	return pcstr ;

}

Network::Network(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_strBackPixmap_log = MxGlobalConfig::Instance()->GetSkinDir();

	m_layout = new QVBoxLayout;
	ui.scrollAreaWidgetContents_2->setLayout(m_layout);
	ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint); //设置窗体无边框
	connect(ui.openWIFI,SIGNAL(clicked()),this,SLOT(onOpenWIFI()));
	connect(ui.colseWIFI,SIGNAL(clicked()),this,SLOT(onCloseWIFI()));
	


	timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(getNework()) );
	timer->start(1000); // 1秒单触发定时器
	timer->stop();


	Mx::setWidgetStyle(this, "qss:Network.qss");
}
void Network::onClose()
{
	close();
	timer->stop();
}
void Network::onCloseWIFI()
{
	HANDLE hClient=NULL;
	DWORD dwMaxClient=2;
	DWORD dwCurVersion=0;
	DWORD dwResult=0;
	PWLAN_INTERFACE_INFO_LIST pIfList=NULL;
	dwResult=WlanOpenHandle(dwMaxClient,NULL,&dwCurVersion,&hClient);
	dwResult=WlanEnumInterfaces(hClient,NULL,&pIfList);
		WLAN_PHY_RADIO_STATE state;

		state.dwPhyIndex=0;

		state.dot11SoftwareRadioState=dot11_radio_state_off;

		PVOID pData=&state;

		dwResult=WlanSetInterface(hClient,&pIfList->InterfaceInfo[0].InterfaceGuid,wlan_intf_opcode_radio_state,sizeof(WLAN_PHY_RADIO_STATE),pData,NULL);
		if(dwResult!=ERROR_SUCCESS){
			wprintf(L"setstatefailed!erris%d\n",dwResult);
		}

	dwResult=WlanCloseHandle(hClient,NULL);
	if(dwResult!=ERROR_SUCCESS){wprintf(L"WlanCloseHandlefailed%lu.\r\n",dwResult);}
	//listenStatus();
	if(pIfList!=NULL)
	{
		WlanFreeMemory(pIfList);
		pIfList=NULL;
	}
	return ;
}
void Network::onOpenWIFI()
{
	HANDLE hClient=NULL;
	DWORD dwMaxClient=2;
	DWORD dwCurVersion=0;
	DWORD dwResult=0;
	PWLAN_INTERFACE_INFO_LIST pIfList=NULL;
	dwResult=WlanOpenHandle(dwMaxClient,NULL,&dwCurVersion,&hClient);
	dwResult=WlanEnumInterfaces(hClient,NULL,&pIfList);
	WLAN_PHY_RADIO_STATE state;

	state.dwPhyIndex=0;

	state.dot11SoftwareRadioState=dot11_radio_state_on;

	PVOID pData=&state;

	dwResult=WlanSetInterface(hClient,&pIfList->InterfaceInfo[0].InterfaceGuid,wlan_intf_opcode_radio_state,sizeof(WLAN_PHY_RADIO_STATE),pData,NULL);
	if(dwResult!=ERROR_SUCCESS){
		wprintf(L"setstatefailed!erris%d\n",dwResult);
	}

	dwResult=WlanCloseHandle(hClient,NULL);
	if(dwResult!=ERROR_SUCCESS){wprintf(L"WlanCloseHandlefailed%lu.\r\n",dwResult);}
	//listenStatus();
	if(pIfList!=NULL)
	{
		WlanFreeMemory(pIfList);
		pIfList=NULL;
	}
	return ;
}
void Network::getNework()
{
	LANListS.clear();
	netList.clear();
	int id=0;
	char *lang = setlocale(LC_CTYPE,NULL);//获取当前的本地语言
	char clang[24];
	strcpy(clang,lang);
	setlocale(LC_ALL, "chs");//设置本地语言
	HANDLE ClientHandle;DWORD nv,i,c;PWLAN_INTERFACE_INFO_LIST ilist;PWLAN_AVAILABLE_NETWORK_LIST nlist;static char ssid[36];//static char  name[36];
	if(WlanOpenHandle(1,0,&nv,&ClientHandle)==0)
	{
		if(WlanEnumInterfaces(ClientHandle,0,&ilist)==0)
		{
			for (i = 0; i< ilist->dwNumberOfItems; i++) {
				wprintf(L"\n\n  %s%s\n\n",L"网卡:",ilist->InterfaceInfo[i].strInterfaceDescription);
				if(WlanGetAvailableNetworkList(ClientHandle,&ilist->InterfaceInfo[i].InterfaceGuid,0,0,&nlist)==0){
					for(c=0;c<nlist->dwNumberOfItems;c++)
					{
						
						memcpy(ssid,nlist->Network[c].dot11Ssid.ucSSID,nlist->Network[c].dot11Ssid.uSSIDLength);
						//char *name;
						//name=w2c(name,nlist->Network[c].strProfileName,sizeof(nlist->Network[c].strProfileName)/sizeof(WCHAR));
						ssid[nlist->Network[c].dot11Ssid.uSSIDLength]=0;
						//name[sizeof(nlist->Network[c].strProfileName)/sizeof(WCHAR)]=0;
						pBssEntry = (WLAN_AVAILABLE_NETWORK *) & nlist->Network[c];
						char ss[256];
						sprintf(ss,"   %3d. SSID:  %-25s  信号强度:  %5d\n",id++,ssid,nlist->Network[c].wlanSignalQuality);
						QString key;
						key.append(ssid);
						int _value=nlist->Network[c].wlanSignalQuality;
						qDebug()<<ss;
						//DOT11_AUTH_ALGORITHM ss= pBssEntry->dot11DefaultAuthAlgorithm;
						if (LANListS.find(key) == LANListS.end())
						{
							LANListS.insert(key,_value);
							
							//LANList.insert(key,pBssEntry);
							NetInfo netinfo;
							netinfo.netName=ssid;
							netinfo.SSID=ssid;
							netinfo.SignalQuality=_value;
							if(pBssEntry->dot11DefaultAuthAlgorithm == DOT11_AUTH_ALGORITHM::DOT11_AUTH_ALGO_80211_OPEN)
							{
								netinfo.sock=false;
							}
							else{

								netinfo.sock=true;
							}
							if(pBssEntry->dwFlags == 0x3)
							{
								netinfo.isConn=true;
							}
							else
							{
								netinfo.isConn=false;
							}
							netList.push_back(netinfo);
						}
						//delete name;
					}

					WlanFreeMemory(nlist);
				}
			}
			WlanFreeMemory(ilist);
		}
		//system("pause>nul");
		WlanCloseHandle(ClientHandle,0);
	}
	setlocale(LC_ALL, clang);//恢复语言
	updateList();
}
void Network::init()
{
	timer->start(1000); // 1秒单触发定时器
	return;
}
Network::~Network()
{
	
}
void Network::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::white);
	painter.drawPixmap(this->rect(), QPixmap(QString::fromLocal8Bit("%1/qtlogin/background.png").arg(m_strBackPixmap_log)));
	QPainter painter2(this);
	painter2.setPen(Qt::SolidLine);
	painter2.setBrush(QColor(151,151,151));
	int w=width();
	int h=height();
	painter2.drawLine(QPoint(0,0),QPoint(w,0));
	painter2.drawLine(QPoint(0,0),QPoint(0,h));
	painter2.drawLine(QPoint(w-1,0),QPoint(w-1,h-1));
	painter2.drawLine(QPoint(0,h-1),QPoint(w-1,h-1));
}
void Network::updateList()
{
	foreach(QObject* p,objList)
	{
		delete p;
	}
	objList.clear();
	foreach(QLayoutItem* p,LayoutList)
	{
		m_layout->removeItem(p);
		//delete p;
	}
	LayoutList.clear();
	addLine();
}
void Network::addLine()
{
	//for(QMap<QString,int>::iterator iter=LANListS.begin();iter!=LANListS.end();iter++)
	int i = 0;
	foreach(NetInfo info,netList)
	{
		QString lan=info.netName;
		int qiang=info.SignalQuality;
		QHBoxLayout *hv=new QHBoxLayout;
		objList.push_back(hv);
		QLabel *name=new QLabel;
		if(lan.isEmpty())
			lan=info.SSID;
		name->setText(lan);
		
		QLabel *zh=new QLabel;//youshuo
		zh->setFixedSize(13,16);
		if(info.sock)
		{
			zh->setStyleSheet(QString::fromLocal8Bit("border-image:url(%1/qtlogin/password01.png);").arg(m_strBackPixmap_log));
		}

		QLabel *con=NULL;
		con=new QLabel;
		con->setFixedSize(24,16);
		if(info.isConn)
		{
			con->setStyleSheet(QString::fromLocal8Bit("border-image:url(%1/qtlogin/right.png);").arg(m_strBackPixmap_log));
		}
		MyLabel *lian=new MyLabel;
		lian->setText(QString::fromLocal8Bit(""));
		lian->setObjectName(info.SSID);
		//lian->setStyleSheet("color: rgb(0, 85, 255)");
		lian->setFixedSize(23,16);
		lian->setStyleSheet(QString::fromLocal8Bit("border-image:url(%1/qtlogin/WiFi01.png);").arg(m_strBackPixmap_log));
		connect(lian,SIGNAL(clicked()),this,SLOT(onLain()));
		
		QSpacerItem* p=new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
		QSpacerItem* p1=new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		QSpacerItem* p2=new QSpacerItem(16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		QSpacerItem* p3=new QSpacerItem(16, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		QSpacerItem* p4=new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);
		hv->addSpacerItem(p1);

		
		hv->addWidget(con);
		hv->addSpacerItem(p2);
		hv->addWidget(name);
		hv->addSpacerItem(p);
		hv->addWidget(zh);
		hv->addSpacerItem(p3);
		hv->addWidget(lian);
		hv->addSpacerItem(p4);


		//hv->addWidget(tu);
		
		LayoutList.push_back(p);
		LayoutList.push_back(p1);
		LayoutList.push_back(p2);
		LayoutList.push_back(p3);
		LayoutList.push_back(p4);
		objList.push_back(con);
		objList.push_back(zh);
		//objList.push_back(tu);
		objList.push_back(lian);
		objList.push_back(name);

		m_layout->addLayout(hv);
	}
}
void Network::onLain()
{
	QString name=sender()->objectName();
	QString password;
	QString qname;
	QString qssid;
	QString oldName;
	foreach(NetInfo info,netList)
	{
		if(info.isConn)
		{
			oldName=info.netName;
		}
		if(info.SSID == name)
		{
			bool ok;
			QString text = QInputDialog::getText(this, QString::fromLocal8Bit("输入密码"),
				QString::fromLocal8Bit("请输入密码:"), QLineEdit::Password,"", &ok);
			if (ok)
			{
				password=text;
				qname=info.netName;
				qssid=info.SSID;
				//break;
			}
			else{
				return;
			}
				
		}
	}
	//WLAN_CONNECTION_PARAMETERS pp;
	//PWLAN_AVAILABLE_NETWORK ss=LANList.find(name).value();
	//pp.dot11BssType=ss->dot11BssType;
	//pp.pDot11Ssid=&ss->dot11Ssid;
	//pp.strProfile=ss->strProfileName;
	//pp.dwFlags=2;
	//pp.wlanConnectionMode=wlan_connection_mode_auto;
	//pp.pDesiredBssidList=NULL;
	//DWORD dd=WlanConnect(hCilent,&(pIfList->InterfaceInfo[0].InterfaceGuid),&pp,NULL);//pIfInfo->InterfaceGuid
	//if (dd != ERROR_SUCCESS)
	//{
	//	//wprintf(L"WlanGetAvailableNetworkList failed with error: %u\n",dwResult);
	//	dd = 1;
	//	// You can use FormatMessage to find out why the function failed
	//} else {
	//	qDebug()<<dd;
	//}
	HANDLE hClient=NULL;
	DWORD dwMaxClient=2;
	DWORD dwCurVersion=0;
	DWORD dwResult=0;
	PWLAN_INTERFACE_INFO_LIST pIfList=NULL;
	dwResult=WlanOpenHandle(dwMaxClient,NULL,&dwCurVersion,&hClient);
	dwResult=WlanEnumInterfaces(hClient,NULL,&pIfList);
	LPCWSTR profileXml;
	char cStr[1028];
	std::wstring strHead=L"<?xml version=\"1.0\" encoding=\"US-ASCII\" ?>\<WLANProfile xmlns=\"http://www.microsoft.com/networking/WLAN/profile/v1\">\<name>";
	strHead.append(qname.toStdWString());
	strHead.append(L"</name>\<SSIDConfig>\<SSID>\<name>");
	strHead.append(qssid.toStdWString());
	strHead.append(L"</name>\</SSID>\</SSIDConfig>\<connectionType>ESS</connectionType>\<connectionMode>auto</connectionMode>\<autoSwitch>false</autoSwitch>\<MSM>\<security>\<authEncryption>\<authentication>WPA2PSK</authentication>\<encryption>AES</encryption>\<useOneX>false</useOneX>\</authEncryption>\<sharedKey>\<keyType>passPhrase</keyType>\<protected>false</protected>\<keyMaterial>");
	strHead.append(password.toStdWString());
	strHead.append(L"</keyMaterial>\</sharedKey>\</security>\</MSM>\</WLANProfile>");
	//wprintf(strHead,L"  Profile Name[%u]:  %ws\n", j, pBssEntry->strProfileName);
	profileXml=strHead.c_str();
	WLAN_REASON_CODE Wlanreason;//如果<connectionMode>auto</connectionMode>，为自动连接，则下面的一步可以连接上无线
	dwResult=WlanSetProfile(hClient,&(pIfList->InterfaceInfo[0].InterfaceGuid),0,profileXml,NULL,TRUE,NULL,&Wlanreason);
	if(ERROR_SUCCESS!=dwResult)
	{
		printf("wlansetprofilefailed%lu.\r\n",dwResult);
		return;
		
	}//删除无线配置文件/*LPCWSTRprofileName;
	if(!oldName.isEmpty())
	{
		LPCWSTR profileName;
		std::wstring strprofileName=oldName.toStdWString();
		profileName=strprofileName.c_str();
		dwResult=WlanDeleteProfile(hClient,&(pIfList->InterfaceInfo[0].InterfaceGuid),profileName,NULL);
		if(ERROR_SUCCESS!=dwResult){
			printf("wlandeleteprofilefailed%lu.\r\n",dwResult);
		}
	}
	onClose();
}

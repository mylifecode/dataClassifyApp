#include "HttpRequest.h"
#include <QDebug>
#include "MxDefine.h"
#if(0)
HttpRequest* HttpRequest::m_httpRequstInstance = NULL;
HttpRequest::HttpRequest(QObject* parent /* = 0 */):QObject(parent)
{
	//checkRegisterURL=QString("http://openlab.dev.cd.misrobot.com/msc/api/eq/checkRegister");
	//getRegisterDataURL=QString("http://openlab.dev.cd.misrobot.com/msc/api/eq/getRegisterData");
	//putRegisterDataURL=QString("http://openlab.dev.cd.misrobot.com/msc/api/eq/putRegisterData");
	//getQrUrlURL = QString("http://openlab.dev.cd.misrobot.com/msc/api/eq/getQrUrl");
	//loginURL=QString("http://openlab.dev.cd.misrobot.com/msc/api/eq/login");
	//logoutURL=QString("http://openlab.dev.cd.misrobot.com/msc/api/eq/logout");
	m_networkManager = new QNetworkAccessManager(this);
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(HttpReturn(QNetworkReply*)));
}


void HttpRequest::senddata(QString url,QString dataget,QByteArray datapost,int sendtype)
{
	QNetworkRequest request;
	request.setUrl(QUrl(url));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	if(sendtype == 0)
	{

	}
	else if(sendtype == 1)
	{
		qDebug()<<"data:"<<datapost;
		m_networkManager->post(request,datapost);
	}
}

void HttpRequest::HttpReturn(QNetworkReply* reply)
{
	QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
	int requestOption = (int)reply->operation();

	int m_interface_requestType=UnknownInterface;
	if(requestOption==QNetworkAccessManager::GetOperation)
	{

	}
	if(requestOption==QNetworkAccessManager::PostOperation)
	{
		QString url = (reply->url()).toString();

		if(url.compare(checkRegisterURL) == 0)
		{
			m_interface_requestType=checkRegisterInterface;
		}
		if(url.compare(getQrUrlURL) == 0)
		{
			m_interface_requestType=getQrUrlInterface;
		}
		if(url.compare(getRegisterDataURL) == 0)
		{
			m_interface_requestType=getRegisterDataInterface;
		}
		if(url.compare(loginURL) == 0)
		{
			m_interface_requestType=loginInterface;
		}

		if(url.compare(logoutURL) == 0)
		{
			m_interface_requestType=logoutInterface;
		}
		if(url.compare(putRegisterDataURL) == 0)
		{
			m_interface_requestType=putRegisterDataInterface;
		}
	}
	
	emit receveData(statusCodeV.toInt(), reply->readAll(),m_interface_requestType);
	reply->deleteLater();
}


//½Ó¿Ú
void HttpRequest::checkRegister(QString mac,int requestType)//post mac±ØÐë
{
	QSettings *cfg=new QSettings(HTTP_URL_INI,QSettings::IniFormat);
	QString httpurl=cfg->value("http/url").toString();
	httpurl.append(cfg->value("http/urlend").toString());
	delete cfg;
	cfg=NULL;
	checkRegisterURL=httpurl+"checkRegister";
	QString szMac = QString("mac=") + mac.trimmed();
	QString send = szMac;
	QByteArray data = send.toUtf8();
	if(requestType == 0)
	{

	}
	else if(requestType == 1)
	{
		senddata(checkRegisterURL,"",data,requestType);
	}
}

void HttpRequest::getRegisterData(int requestType)//post
{
	QSettings *cfg=new QSettings(HTTP_URL_INI,QSettings::IniFormat);
	QString httpurl=cfg->value("http/url").toString();
	httpurl.append(cfg->value("http/urlend").toString());
	delete cfg;
	cfg=NULL;
	getRegisterDataURL=httpurl+"getRegisterData";
	QByteArray data;
	if(requestType == 0)
	{

	}
	else if(requestType == 1)
	{
		senddata(getRegisterDataURL,"",data,requestType);
	}
}
void HttpRequest::putRegisterData(int eq_type_id,QString name,QString type,int eq_resouce,QString mac,int lab_id,int admin_id,QString begin_time,QString end_time,QByteArray week,int requestType)//post
{
	QSettings *cfg=new QSettings(HTTP_URL_INI,QSettings::IniFormat);
	QString httpurl=cfg->value("http/url").toString();
	httpurl.append(cfg->value("http/urlend").toString());
	delete cfg;
	cfg=NULL;
	putRegisterDataURL=httpurl+"putRegisterData";
	QString szEq_type_id = QString("eq_type_id=") +QString::number(eq_type_id,10);
	QString szName = QString("name=")+name.trimmed();
	QString szType = QString("type=")+type.trimmed();
	QString szEq_resouce = QString("eq_resouce=")+QString::number(eq_resouce,10);
	QString szMac=QString("mac=")+mac.trimmed();
	QString szLab_id=QString("lab_id=")+QString::number(lab_id,10);
	QString szAdmin_id=QString("admin_id=")+QString::number(admin_id,10);
	QString szBegin_time = QString("begin_time=")+begin_time.trimmed();
	QString szEnd_time=QString("end_time=")+end_time.trimmed();
	QString szWeek;
	if(week.size() == 0)
	{
		szWeek = QString("week[]=")+QString("");
	}
	else
	{
		for(int i=0;i<week.size();i++)
		{
			if(i == week.size()-1)
			{
				szWeek += QString("week[]=")+QString(week.at(i));
			}
			else
			{
				szWeek += QString("week[]=")+QString(week.at(i))+QString("&");
			}

		}
	}

	QString send= szEq_type_id+QString("&")+szName+QString("&")+szType+QString("&")+szEq_resouce+QString("&")+szMac+QString("&")+szLab_id+QString("&")+szAdmin_id+QString("&")+szBegin_time+QString("&")+szEnd_time+QString("&")+szWeek;
	QByteArray data = send.toUtf8();

	if(requestType == 0)
	{

	}
	else if(requestType == 1)
	{
		senddata(putRegisterDataURL,"",data,requestType);
	}
}

//void HttpRequest::putRegisterData(int eq_type_id,QString name,QString type,int eq_resouce,QString mac,int lab_id,int admin_id,QString begin_time,QString end_time,QString week,int requestType)//post
//{
//	QSettings *cfg=new QSettings(HTTP_URL_INI,QSettings::IniFormat);
//	QString httpurl=cfg->value("http/url").toString();
//	delete cfg;
//	cfg=NULL;
//	putRegisterDataURL=httpurl+"putRegisterData";
//	QString szEq_type_id = QString("eq_type_id=") +QString::number(eq_type_id,10);
//	QString szName = QString("name=")+name.trimmed();
//	QString szType = QString("type=")+type.trimmed();
//	QString szEq_resouce = QString("eq_resouce=")+QString::number(eq_resouce,10);
//	QString szMac=QString("mac=")+mac.trimmed();
//	QString szLab_id=QString("lab_id=")+QString::number(lab_id,10);
//	QString szAdmin_id=QString("admin_id=")+QString::number(admin_id,10);
//	QString szBegin_time = QString("begin_time=")+begin_time.trimmed();
//	QString szEnd_time=QString("end_time=")+end_time.trimmed();
//	QByteArray m_week;
//	m_week.push_back('1');
//	m_week.push_back('2');
//	m_week.push_back('3');
//	m_week.push_back('4');
//
//	qDebug()<<m_week.data()<<endl;
//	QString szWeek = QString("week=")+week.trimmed();
//	
//	QString send= szEq_type_id+QString("&")+szName+QString("&")+szType+QString("&")+szEq_resouce+QString("&")+szMac+QString("&")+szLab_id+QString("&")+szAdmin_id+QString("&")+szBegin_time+QString("&")+szEnd_time+QString("&")+szWeek;
//	QByteArray data = send.toUtf8();
//	qDebug()<<data<<endl;
//	if(requestType == 0)
//	{
//
//	}
//	else if(requestType == 1)
//	{
//		senddata(putRegisterDataURL,"",data,requestType);
//	}
//}
void HttpRequest::getQrUrl(QString mac,int requestType)//post
{
	QSettings *cfg=new QSettings(HTTP_URL_INI,QSettings::IniFormat);
	QString httpurl=cfg->value("http/url").toString();
	httpurl.append(cfg->value("http/urlend").toString());
	delete cfg;
	cfg=NULL;
	getQrUrlURL=httpurl+"getQrUrl";
	QString szMac = QString("mac=") + mac.trimmed();
	QString send = szMac;
	QByteArray data = send.toUtf8();
	if(requestType == 0)
	{

	}
	else if(requestType == 1)
	{
		senddata(getQrUrlURL,"",data,requestType);
	}
}

void HttpRequest::login(QString username,QString password,QString mac,int requestType)//post
{
	QSettings *cfg=new QSettings(HTTP_URL_INI,QSettings::IniFormat);
	QString httpurl=cfg->value("http/url").toString();
	httpurl.append(cfg->value("http/urlend").toString());
	delete cfg;
	cfg=NULL;
	loginURL=httpurl+"login";
	QString szUsername = QString("username=")+username.trimmed();
	QString szPassword = QString("password=")+password.trimmed();
	QString szMac = QString("mac=") + mac.trimmed();
 	QString send = szUsername+QString("&")+szPassword+QString("&")+szMac;
	QByteArray data = send.toUtf8();
	if(requestType == 0)
	{

	}
	else if(requestType == 1)
	{
		senddata(loginURL,"",data,requestType);
	}
}

void HttpRequest::logout(QString mac,int requestType)//post
{
	QSettings *cfg=new QSettings(HTTP_URL_INI,QSettings::IniFormat);
	QString httpurl=cfg->value("http/url").toString();
	httpurl.append(cfg->value("http/urlend").toString());
	QString uid=cfg->value("Hardware/uid").toString();
	QString sign_id=cfg->value("Hardware/sign_id").toString();
	delete cfg;
	cfg=NULL;
	logoutURL=httpurl+"logout";
	QString szMac = QString("mac=") + mac.trimmed();
	QString szUid = QString("uid=")+uid.trimmed();
	QString szSignid = QString("sign_id=")+sign_id.trimmed();
	QString send = szMac+QString("&")+szUid+QString("&")+szSignid;
	QByteArray data = send.toUtf8();
	if(requestType == 0)
	{

	}
	else if(requestType == 1)
	{
		senddata(logoutURL,"",data,requestType);
	}
}
#endif
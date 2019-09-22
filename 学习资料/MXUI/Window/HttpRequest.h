#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <QString>
#include <QObject>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSettings>

/*
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>

���ӿ� Qt5Networkd.lib  �����5�汾��
ʹ��˵��
QString str2;
QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
str2 = list.at(1).hardwareAddress(); 

QString mac3=QString("3C:46:D8:77:24:2B");
HttpRequest* http_instancde=HttpRequest::getInstance();
//http_instancde->getRegisterData(); ---����
connect(http_instancde,SIGNAL(receveData(int,QString,int)),this,SLOT(receveDataSlot(int,QString,int)));
//http_instancde->checkRegister(mac3);---����

int eq_type_id =111;
QString name = QString("������");
QString type =QString("FQJ(��ǻ��)");
int eq_resouce=0;
int lab_id=504;
int admin_id=100;
QString mac2=QString("95:90:96:B2:C6:06");

//void putRegisterData(int eq_type_id,QString name,QString type,int eq_resouce,QString mac,int lab_id,int admin_id,QString begin_time,QString end_time,QString week,int requestType=1);//post---����
//http_instancde->putRegisterData(eq_type_id,name,type,eq_resouce,mac3,lab_id,admin_id,"","","");

http_instancde->getQrUrl(mac3);---����
connect(http_instancde,SIGNAL(receveData(int,QString,int)),this,SLOT(receveDataSlot(int,QString,int))); ----------------------------

void receveDataSlot(int stat, QString data,int interface_requestType)
{

if(interface_requestType ==getQrUrlInterface)
{
if(stat == 200)
{
qDebug()<<"---------------------------"<<endl;
qDebug()<<"request httpcode="<<stat<<endl;
qDebug()<<data<<endl;


QJsonParseError error;
QJsonDocument jsonDocument = QJsonDocument::fromJson(data.toUtf8(), &error);

if (error.error == QJsonParseError::NoError) {
if (!(jsonDocument.isNull() || jsonDocument.isEmpty()))
{
if (jsonDocument.isObject())
{
QVariantMap result = jsonDocument.toVariant().toMap();
QString message = result["message"].toString();

qDebug()<<message<<endl;
}
else
{

}
}
}
else {
//qFatal(error.errorString().toUtf8().constData());
//exit(1);
}
}
}

*/
typedef enum InterfaceRequestType
{
	UnknownInterface=0,
	checkRegisterInterface,
	getRegisterDataInterface,
	putRegisterDataInterface,
	getQrUrlInterface,
	loginInterface,
	logoutInterface
}InterfaceRequestType;
/*
requestType 0��get 1��post
*/
/*
class HttpRequest :public QObject
{
	Q_OBJECT
public:
	void checkRegister(QString mac,int requestType=1);//post mac����
	void getRegisterData(int requestType=1);//post

		//					��������	�Ƿ����	��			��ע

		//eq_type_id		int			Y					�豸����ID
		//name				string		Y					�豸����
		//type				string		Y		FQJ(��ǻ��)	�豸�ͺ�
		//eq_resouce		int  		N		1	        ע����Դ
		//mac				string		Y					MAC��ַ
		//lab_id			int			N					ʵ����ID
		//admin_id			int			N					����ԱID
		//begin_time		string		N		11:00	    ���ŵ�ԤԼ��ʼʱ��
		//end_time			string		N		11:30	    ����ԤԼ�Ľ���ʱ��
		//week				array		N		[1,2,3,4]	ÿ�����ڿ��ŵĶ�Ӧ��

	void putRegisterData(int eq_type_id,QString name,QString type,int eq_resouce,QString mac,int lab_id,int admin_id,QString begin_time,QString end_time,QByteArray week,int requestType=1);//post
	//void putRegisterData(int eq_type_id,QString name,QString type,int eq_resouce,QString mac,int lab_id,int admin_id,QString begin_time,QString end_time,QString week,int requestType=1);//post
	void getQrUrl(QString mac,int requestType=1);//post
	void login(QString username,QString password,QString mac,int requestType=1);//post
	void logout(QString mac,int requestType=1);//post
private:
	void senddata(QString url,QString dataget,QByteArray datapost,int sendtype);//0 get=>datapost �� 1 post=>dataget��
public slots:
	void HttpReturn(QNetworkReply*);
signals:
	void receveData(int stat, QByteArray data,int interface_requestType);
public:
	static HttpRequest* getInstance()//ͨ����̬���к�����ø����ʵ������
	{
		if(m_httpRequstInstance==NULL)
			m_httpRequstInstance=new HttpRequest();
		return m_httpRequstInstance;
	}
private:
	explicit HttpRequest(QObject* parent = 0);
	static HttpRequest* m_httpRequstInstance;
	class Garbo//ɾ��Singletonʵ���Ķ���
	{
	public:
		~Garbo()
		{
			if(HttpRequest::m_httpRequstInstance)
			{
				delete HttpRequest::m_httpRequstInstance;
			}
		}
	};
	static Garbo gb;//�ڳ������ʱ��ϵͳ�����������������
private:
	QNetworkAccessManager* m_networkManager;

	QString severIP;
	QString checkRegisterURL;
	QString getRegisterDataURL;
	QString putRegisterDataURL;
	QString getQrUrlURL;
	QString loginURL;
	QString logoutURL;

};
*/
#endif

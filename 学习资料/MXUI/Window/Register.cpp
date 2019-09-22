#include "Register.h"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonArray"
#include "qDebug"
#include "MxDefine.h"
#include "MxGlobalConfig.h"
Register::Register(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	m_strBackPixmap_log = MxGlobalConfig::Instance()->GetSkinDir();

	pRegInfo=NULL;
	pSchoolInfo=NULL;
	pRegInfo=NULL;
	pFloorInfo=NULL;
	pFInfo=NULL;
	setWindowFlags(Qt::FramelessWindowHint ); //ÉèÖÃ´°ÌåÎÞ±ß¿ò
	connect(ui.xiaoqu,SIGNAL(currentIndexChanged(int)),this,SLOT(onXiaoqu(int)));
	connect(ui.dalou,SIGNAL(currentIndexChanged(int)),this,SLOT(onDalou(int)));
	connect(ui.loucheng,SIGNAL(currentIndexChanged(int)),this,SLOT(onLoucheng(int)));
	connect(ui.jiaoshi,SIGNAL(currentIndexChanged(int)),this,SLOT(onJiaoshi(int)));
	connect(ui.deviceType,SIGNAL(currentIndexChanged(int)),this,SLOT(onType(int)));
	connect(ui.closeBtn,SIGNAL(clicked()),this,SLOT(close()));
	connect(ui.no,SIGNAL(clicked()),this,SLOT(close()));
	connect(ui.ok,SIGNAL(clicked()),this,SLOT(onOK()));

	Mx::setWidgetStyle(this, "qss:Register.qss");
}

Register::~Register()
{
	if(pRegInfo!=NULL)
	{
		delete pRegInfo;
	}
}
void Register::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::white);
	painter.drawPixmap(this->rect(), QPixmap(QString::fromLocal8Bit("%1/qtlogin/registerBackground.png").arg(m_strBackPixmap_log)));
}
void Register::init(RegInfo *regInfo)
{
	if(pRegInfo!=NULL)
	{
		delete pRegInfo;
	}
 	pRegInfo=regInfo;
	//pFloorInfo=NULL;
	//pFInfo=NULL;
	//pRoom=NULL;
	ui.xiaoqu->clear();
	int n=pRegInfo->school.count();
	for(int i=0;i<n;i++)
	{
		ui.xiaoqu->addItem(pRegInfo->school.at(i).schoolName);
	}
	if(n>0)
	{
		ui.xiaoqu->setCurrentText(pRegInfo->school.at(0).schoolName);
		pSchoolInfo = &(pRegInfo->school.at(0));
	}
	else
	{
		pSchoolInfo=NULL;
	}
	ui.deviceType->clear();
	int n2=pRegInfo->type.count();
	for(int i=0;i<n2;i++)
	{
		ui.deviceType->addItem(pRegInfo->type.at(i).typeName);
	}
	if(n2>0)
	{
		ui.deviceType->setCurrentText(pRegInfo->type.at(0).typeName);
		pTypeInfo = &(pRegInfo->type.at(0));
	}
	else
	{
		pTypeInfo=NULL;
	}
	
}
void Register::onXiaoqu(int idx)
{
	//QString text=ui.xiaoqu->
	//if(text == "")
	//{
	//	//ui.dalou->clear();
	//	return;
	//}
	//pSchoolInfo=NULL;
	//int n=pRegInfo->school.count();
	//for(int i=0;i<n;i++)
	//{
	//	if(pRegInfo->school.at(i).schoolName == text)
	//	{
	//		pSchoolInfo = &(pRegInfo->school.at(i));
	//		//int n2=pRegInfo->school.at(i).floor.count();
	//		//for(int i2=0;i2<n2;i2++)
	//		//{
	//		//	ui.dalou->addItem(pRegInfo->school.at(i).floor.at(i2).floorName);
	//		//}
	//		//if(n2>0)
	//		//{
	//		//	ui.dalou->setCurrentText(pRegInfo->school.at(i).floor.at(0).floorName);
	//		//}
	//		//return;
	//		break;
	//	}
	//}
	if(idx<0)
	{
		return;
	}
	pSchoolInfo = &(pRegInfo->school.at(idx));
	ui.dalou->clear();
	if(pSchoolInfo==NULL)
	{
		return;
	}
	int n2=pSchoolInfo->floor.count();
	
	for(int i2=0;i2<n2;i2++)
	{
		ui.dalou->addItem(pSchoolInfo->floor.at(i2).floorName);
	}
	if(n2>0)
	{
		ui.dalou->setCurrentText(pSchoolInfo->floor.at(0).floorName);
		pFloorInfo=&pSchoolInfo->floor.at(0);
	}
	else
	{
		pFloorInfo=NULL;
	}
}
void Register::onDalou(int idx)
{
	//if(text == "")
	//{
	//	//ui.loucheng->clear();
	//	return;
	//}
	//pFloorInfo=NULL;
	//int n=pSchoolInfo->floor.count();
	//for(int i=0;i<n;i++)
	//{
	//	if(pSchoolInfo->floor.at(i).floorName == text)
	//	{
	//		pFloorInfo = &(pSchoolInfo->floor.at(i));
	//		break;
	//	}
	//}
	if(idx<0)
	{
		return;
	}
	pFloorInfo = &(pSchoolInfo->floor.at(idx));
	ui.loucheng->clear();
	if(pFloorInfo==NULL)
	{
		return;
	}
	int lc=pFloorInfo->floor_bottom+pFloorInfo->floor_top;
	for(int i=0;i<lc;i++)
	{
		int k=i;
		if(i>=pFloorInfo->floor_bottom)
		{
			k++;
		}
		ui.loucheng->addItem(QString("%1").arg(k-pFloorInfo->floor_bottom));
	}
	if(lc>0)
	{
		
		pFInfo=&pFloorInfo->fInfo.at(0);
		ui.loucheng->setCurrentText(QString(pFInfo->id));
	}
	else
	{
		pFInfo=NULL;
	}
}
void Register::onLoucheng(int idx)
{
	//if(text == "")
	//{
	//	//ui.jiaoshi->clear();
	//	return;
	//}
	//pFInfo=NULL;
	//int n=pFloorInfo->fInfo.count();
	//for(int i=0;i<n;i++)
	//{
	//	if(pFloorInfo->fInfo.at(i).id == text.toInt())
	//	{
	//		pFInfo = &(pFloorInfo->fInfo.at(i));
	//		break;
	//	}
	//}
	if(idx<0)
	{
		return;
	}
	pFInfo = &(pFloorInfo->fInfo.at(idx));
	ui.jiaoshi->clear();
	if(pFInfo==NULL)
	{
		return;
	}
	int n2=pFInfo->room.count();
	
	for(int i2=0;i2<n2;i2++)
	{
		ui.jiaoshi->addItem(pFInfo->room.at(i2).code);
	}
	if(n2>0)
	{
		ui.jiaoshi->setCurrentText(pFInfo->room.at(0).code);
	}
	else
	{
		pRoom=NULL;
	}
}
void Register::onJiaoshi(int idx)
{
	//if(text == "")
	//{
	//	pRoom=NULL;
	//	return;
	//}
	//pRoom=NULL;
	//int n=pFInfo->room.count();
	//for(int i=0;i<n;i++)
	//{
	//	if(pFInfo->room.at(i).roomName == text)
	//	{
	//		pRoom = &(pFInfo->room.at(i));
	//		break;
	//	}
	//}
	if(idx<0)
	{
		pRoom=NULL;
		return;
	}
	pRoom = &(pFInfo->room.at(idx));
	ui.jiaoshi->setToolTip(pRoom->roomName);
}
void Register::onType(int idx)
{
	//if(text == "")
	//{
	//	pTypeInfo=NULL;
	//	return;
	//}
	//pTypeInfo=NULL;
	//int n=pRegInfo->type.count();
	//for(int i=0;i<n;i++)
	//{
	//	if(pRegInfo->type.at(i).typeName == text)
	//	{
	//		pTypeInfo = &(pRegInfo->type.at(i));
	//		break;
	//	}
	//}
	if(idx<0)
	{
		return;
	}
	pTypeInfo = &(pRegInfo->type.at(idx));
	if(pTypeInfo!=NULL /*&& ui.deviceName->text().isEmpty()*/)
		ui.deviceName->setText(pTypeInfo->adviseName);
}
void Register::onOK()
{
	PutRegInfo *putdata=new PutRegInfo;
	if(pTypeInfo != NULL)
		putdata->eq_type_id=pTypeInfo->id;
	else
		putdata->eq_type_id=-1;
	putdata->name=ui.deviceName->text();
	//putdata->type=pTypeInfo->enName;
	putdata->type=ui.xinghao->text();
	putdata->eq_resouce=1;
	if(pRoom != NULL)
	{
		putdata->lab_id=pRoom->id;
	}
	else
	{
		putdata->lab_id=-1;
	}
	putdata->admin_id=0;
	putdata->begin_time=ui.beginTime->text();
	putdata->end_time=ui.endTime->text();

	//QJsonArray json;
	
	//json.insert("DepartureCity", fq.DepartureCity);
	//json.insert("ArrivalCity", fq.ArrivalCity);
	//QJsonDocument document;
	//document.setObject(json);
	//QByteArray byte_array = document.toJson(QJsonDocument::Compact);
	QByteArray _weekd;
// 	QString _week="[";
// 	bool first=true;
	if(ui.week_1->checkState()==Qt::Checked)
	{
		_weekd.push_back("1");
// 		json.append(1);
// 		_week+="1";
// 		first=false;
	}
	if(ui.week_2->checkState()==Qt::Checked)
	{
		_weekd.push_back("2");
// 		json.append(2);
// 		if(!first)
// 			_week+=",";
// 		_week+="2";
// 		first=false;
	}
	if(ui.week_3->checkState()==Qt::Checked)
	{
		_weekd.push_back("3");
		//json.append(3);
		//if(!first)
		//	_week+=",";
		//_week+="3";
		//first=false;
	}
	if(ui.week_4->checkState()==Qt::Checked)
	{
		_weekd.push_back("4");
		//json.append(4);
		//if(!first)
		//	_week+=",";
		//_week+="4";
		//first=false;
	}
	if(ui.week_5->checkState()==Qt::Checked)
	{
		_weekd.push_back("5");
		//json.append(5);
		//if(!first)
		//	_week+=",";
		//_week+="5";
		//first=false;
	}
	if(ui.week_6->checkState()==Qt::Checked)
	{
		_weekd.push_back("6");
		//json.append(6);
		//if(!first)
		//	_week+=",";
		//_week+="6";
		//first=false;
	}
	if(ui.week_7->checkState()==Qt::Checked)
	{
		_weekd.push_back("7");
		//json.append(7);
		//if(!first)
		//	_week+=",";
		//_week+="7";
	}
	//_week+="]";
	//QJsonDocument document(json);
	////document.setObject(json);
	//QByteArray byte_array = document.toJson(QJsonDocument::Compact);
	//QString dd=QString(byte_array);
	//qDebug()<<"dd:"<<dd<<endl;
	putdata->week=_weekd;
	emit RegisterData(putdata);
	close();
}

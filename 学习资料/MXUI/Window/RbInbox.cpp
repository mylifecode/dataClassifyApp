#include "RbInbox.h"
#include <QSqlRecord>
#include "SYDBMgr.h"
#include "SYUserInfo.h"
#include "MxDefine.h"
#include <QScrollBar>

RbInbox::RbInbox(QWidget *parent):RbShieldLayer(parent),
	m_pMessageView(NULL)
{
	ui.setupUi(this);
	hideOkButton();
	hideCloseButton();
	ui.teaBtn->setProperty("index", 0);
	ui.adminBtn->setProperty("index", 1);

	QStringList header;  
	header<<QString::fromLocal8Bit("来自")
		<<QString::fromLocal8Bit("主题")
		<<QString::fromLocal8Bit("日期");

	ui.adminMessageTableWidget->setColumnCount(3); 
	ui.adminMessageTableWidget->setShowGrid(true);
	ui.adminMessageTableWidget->setHorizontalHeaderLabels(header); 
	ui.adminMessageTableWidget->horizontalHeader()->setFixedHeight(50);
	ui.adminMessageTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);  
	ui.adminMessageTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.adminMessageTableWidget->setRowCount(0); 
	ui.adminMessageTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.adminMessageTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);


	ui.TeaMessageTableWidget->setColumnCount(3);
	ui.TeaMessageTableWidget->setShowGrid(true);
	ui.TeaMessageTableWidget->setHorizontalHeaderLabels(header); 
	ui.TeaMessageTableWidget->horizontalHeader()->setFixedHeight(50);
	ui.TeaMessageTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.TeaMessageTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.TeaMessageTableWidget->setRowCount(0); 
	ui.TeaMessageTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.TeaMessageTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	connect(ui.adminMessageTableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(onOpenaContent(int,int)));
	connect(ui.TeaMessageTableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(onOpenpContent(int,int)));
	connect(ui.teaBtn, SIGNAL(clicked()), this, SLOT(onButtonClicked()));
	connect(ui.adminBtn, SIGNAL(clicked()), this, SLOT(onButtonClicked()));

	BuildInboxWindow();
	connect(ui.closeBtn,SIGNAL(clicked()),this,SLOT(onClickedcloseBtn()));
	Mx::setWidgetStyle(this,"qss:RbInbox.qss");
}

RbInbox::~RbInbox()
{
	if (m_pMessageView)
	{
		delete m_pMessageView;
		m_pMessageView = NULL;
	}
}

void RbInbox::BuildInboxWindow()
{
	QTableWidget * aTableWidget = ui.adminMessageTableWidget;
	aTableWidget->clearContents();

	QTableWidget * pTableWidget = ui.TeaMessageTableWidget;
	pTableWidget->clearContents();
	
	int loginGroupid = SYUserInfo::Instance()->GetGroupId();  //登录学员的小组id
	if(loginGroupid)
	{
		QSqlRecord groupName_record;
		bool ret = SYDBMgr::Instance()->QueryGroupInfo(loginGroupid,groupName_record);
		if(ret)
		{
				m_groupName = groupName_record.value("name").toString();
		}
		else
		{
			m_groupName = "";
		}
		
	}
	QString loginName = SYUserInfo::Instance()->GetUserName();  //用户登录名
	QVector<QSqlRecord> records;
	QVector<QSqlRecord> aRecords;
	QVector<QSqlRecord> pRecords;

	QVector<QTableWidgetItem*> atableWidgetItems;
	QTableWidgetItem *aItem;
	QVector<QTableWidgetItem*> ptableWidgetItems;
	QTableWidgetItem *pItem;
	QString sql = "select * from message";
	bool ret = SYDBMgr::Instance()->Query(sql,records);

	for(QVector<QSqlRecord>::iterator iter = records.begin();iter != records.end();iter++)
	{
		if(((*iter).value("isGroup").toInt() == 0 && loginName == (*iter).value("receiverName").toString()) || ((*iter).value("isGroup").toInt() == 1 && loginGroupid != 0 && m_groupName == (*iter).value("receiverName").toString()))
		{
			const QString& userInfoTable = SYDBMgr::Instance()->GetTableName(SYDBMgr::DT_UserInfo);
			QVector<QSqlRecord> result;
			QString sql = QString("select * from %1 where userName = '%2'").arg(userInfoTable).arg((*iter).value("senderName").toString());
			bool ret = SYDBMgr::Instance()->Query(sql,result);

			for(int r = 0;r < result.size();++r)
			{
				UserPermission permission = SYUserInfo::ConvertValueToPermission(result[r].value("permission").toInt());//发送者权限
				if (permission == UP_SuperManager)
				{
					aRecords.push_back(*iter);
				}

				if (permission == UP_Teacher)
				{
					pRecords.push_back(*iter);
				}
			}
		}	
	}

	aTableWidget->setRowCount(aRecords.size());
	
	for(int r = 0;r < aRecords.size();++r)
	{
		aTableWidget->setItem(r,0,aItem = new QTableWidgetItem(aRecords[r].value("senderName").toString()));
		atableWidgetItems.push_back(aItem);

		aTableWidget->setItem(r,1,aItem = new QTableWidgetItem(aRecords[r].value("subject").toString()));
		atableWidgetItems.push_back(aItem);

		aTableWidget->setItem(r,2,aItem = new QTableWidgetItem(aRecords[r].value("sendTime").toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
		atableWidgetItems.push_back(aItem);

		aTableWidget->setRowHeight(r, 50);
	}
	

	pTableWidget->setRowCount(pRecords.size());
	
	for(int r = 0;r < pRecords.size();++r)
	{
		pTableWidget->setItem(r,0,pItem = new QTableWidgetItem(pRecords[r].value("senderName").toString()));
		ptableWidgetItems.push_back(pItem);

		pTableWidget->setItem(r,1,pItem = new QTableWidgetItem(pRecords[r].value("subject").toString()));
		ptableWidgetItems.push_back(pItem);

		pTableWidget->setItem(r,2,pItem = new QTableWidgetItem(pRecords[r].value("sendTime").toDateTime().toString("yyyy-MM-dd hh:mm:ss")));
		ptableWidgetItems.push_back(pItem);
		
		pTableWidget->setRowHeight(r, 50);
	}
	

	for(int i = 0;i < atableWidgetItems.size();++i)
		atableWidgetItems[i]->setTextAlignment(Qt::AlignCenter);

	for(int i = 0;i < ptableWidgetItems.size();++i)
		ptableWidgetItems[i]->setTextAlignment(Qt::AlignCenter);

	ui.stackedWidget->setCurrentIndex(0);
	ui.teaBtn->setStyleSheet(QString("background-color:white;color:rgb(51,51,51);"));
	ui.adminBtn->setStyleSheet(QString("color:white;"));
	
}

void RbInbox::onOpenaContent(int row,int column)
{
	QPoint curPos = QCursor::pos();//鼠标点击位置
	m_subject = ui.adminMessageTableWidget->item(row, 1)->text();
	m_sendName = ui.adminMessageTableWidget->item(row, 0)->text();
	m_sendTime = ui.TeaMessageTableWidget->item(row, 2)->text();

	QVector<QSqlRecord> arecord;
	QString sql = "select * from message ";
	bool ret = SYDBMgr::Instance()->Query(sql,arecord);

	for(int r = 0;r < arecord.size();++r)
	{
		if (m_subject == arecord[r].value("subject").toString() && m_sendName == arecord[r].value("senderName").toString() && m_sendTime == arecord[r].value("sendTime").toDateTime().toString("yyyy-MM-dd hh:mm:ss"))
		{
			m_content = arecord[r].value("content").toString();

			if (m_pMessageView)
			{
				delete m_pMessageView;
				m_pMessageView = NULL;
			}
			m_pMessageView = new RbMessageViewWindow(this, m_subject, m_content);
			connect(m_pMessageView, SIGNAL(messageClose()), this, SLOT(onMessageClose()));
			int i = (curPos.y() - 328) / 50;
			m_pMessageView->move(1360, 328 + 50 * i);
			m_pMessageView->show();
		}
	}
}

void RbInbox::onOpenpContent(int row,int column)
{
	QPoint curPos = QCursor::pos();//鼠标点击位置
	m_subject = ui.TeaMessageTableWidget->item(row, 1)->text();
	m_sendName = ui.TeaMessageTableWidget->item(row, 0)->text();
	m_sendTime = ui.TeaMessageTableWidget->item(row, 2)->text();

	QVector<QSqlRecord> precord;
	QString sql = "select * from message ";
	bool ret = SYDBMgr::Instance()->Query(sql,precord);

	for(int r = 0;r < precord.size();++r)
	{
		if (m_subject == precord[r].value("subject").toString() && m_sendName == precord[r].value("senderName").toString() && m_sendTime == precord[r].value("sendTime").toDateTime().toString("yyyy-MM-dd hh:mm:ss"))
		{
			m_content = precord[r].value("content").toString();

			if (m_pMessageView)
			{
				delete m_pMessageView;
				m_pMessageView = NULL;
			}
			m_pMessageView = new RbMessageViewWindow(this, m_subject, m_content);
			connect(m_pMessageView, SIGNAL(messageClose()), this, SLOT(onMessageClose()));
			int i = (curPos.y() - 328) / 50;
			m_pMessageView->move(1360, 328 + 50 * i);
			m_pMessageView->show();
		}
	}

}

void RbInbox::onButtonClicked()
{
	QPushButton* pBtn = (QPushButton*)sender();
	int index = pBtn->property("index").toInt();
	ui.stackedWidget->setCurrentIndex(index);
	if (index)
	{
		ui.teaBtn->setStyleSheet(QString("color:white;"));
		ui.adminBtn->setStyleSheet(QString("background-color:white;color:rgb(51,51,51);"));
	}
	else
	{
		ui.teaBtn->setStyleSheet(QString("background-color:white;color:rgb(51,51,51);"));
		ui.adminBtn->setStyleSheet(QString("color:white;"));
	}
}

void RbInbox::onClickedcloseBtn()
{
	this->close();
}

void RbInbox::closeEvent(QCloseEvent * event)
{
	emit indexClose();
}


void RbInbox::mouseMoveEvent(QMouseEvent *event)
{
	if (Qt::LeftButton)
	{
		QPoint end_point = event->pos();
		QPoint startPos = QCursor::pos();
		int x = end_point.rx();
		int y = end_point.ry();

		int dis = end_point.ry() - startPos.ry();

		if (dis > 0)
		{
			Scroll_up();
		}

		if (dis < 0)
		{
			Scroll_down();
		}
	}
}


void RbInbox::Scroll_up()
{
	int index = ui.stackedWidget->currentIndex();
	int barpos = 0;
	if (index)
	{
		barpos = ui.adminMessageTableWidget->verticalScrollBar()->value();
		ui.adminMessageTableWidget->verticalScrollBar()->setSliderPosition(barpos + 1);
	}
	else
	{
		barpos = ui.TeaMessageTableWidget->verticalScrollBar()->value();
		ui.TeaMessageTableWidget->verticalScrollBar()->setSliderPosition(barpos + 1);
	}
}

void RbInbox::Scroll_down()
{
	int index = ui.stackedWidget->currentIndex();
	int barpos = 0;
	if (index)
	{
		barpos = ui.adminMessageTableWidget->verticalScrollBar()->value();
		ui.adminMessageTableWidget->verticalScrollBar()->setSliderPosition(barpos - 1);
	}
	else
	{
		barpos = ui.TeaMessageTableWidget->verticalScrollBar()->value();
		ui.TeaMessageTableWidget->verticalScrollBar()->setSliderPosition(barpos - 1);
	}
}

void RbInbox::onMessageClose()
{
	if (m_pMessageView)
	{
		disconnect(m_pMessageView, SIGNAL(messageClose()), this, SLOT(onMessageClose()));
		delete m_pMessageView;
		m_pMessageView = NULL;
	}
}

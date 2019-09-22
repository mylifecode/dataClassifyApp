#include "SYAdminQuestionsMgrWindow.h"
#include "ui_SYAdminQuestionsMgrWindow.h"
#include"MxDefine.h"
//#include"SYAdminPaperContentFrameWindow.h"
#include"SYDBMgr.h"
#include"qlistwidget.h"
#include"SYStringTable.h"
#include "SYNewBuiltQuestionWindow.h"
#include"SYMessageBox.h"
#include<QFileDialog>
#include"SYStringTable.h"

SYAdminQuestionsMgrWindow::SYAdminQuestionsMgrWindow(int moduleType,QWidget *parent) :
RbShieldLayer(parent), m_curBtn(NULL)
   
{
    ui.setupUi(this);
	hideOkButton();
	hideCloseButton();

	Mx::setWidgetStyle(this, QString("qss:SYAdminQuestionsMgrWindow.qss"));
	InitPage();

	//初始化页面信息
	for (std::size_t i = 0; i < m_btnVector.size(); i++)
	{
		if (m_btnVector[i]->property("type").toInt() == moduleType)
		{
			m_curBtn = m_btnVector[i];
		}
	}
	//m_curBtn = m_btnVector[0];
	m_curBtn->setChecked(true);
	int type = m_curBtn->property("type").toInt();
	QWidget* curWidget = SetListWidget(type);
	QWidget* oldWidget = ui.stackedWidget->currentWidget();
	if (oldWidget)
		ui.stackedWidget->removeWidget(oldWidget);
	ui.stackedWidget->addWidget(curWidget);
	if (curWidget)
		ui.stackedWidget->setCurrentWidget(curWidget);

	connect(ui.batchImportBtn, &QPushButton::clicked, this, &SYAdminQuestionsMgrWindow::do_batchImportData);
	connect(ui.deleteBtn, &QPushButton::clicked, this, &SYAdminQuestionsMgrWindow::do_deleteQuestions);
	connect(ui.newBuiltQuestion, &QPushButton::clicked, this, [=]()
	{
		do_newBuiltQuestion();
	});
	connect(ui.returnBtn, &QPushButton::clicked, this, [=]()
	{
		done(RC_Ok);
	});
	connect(ui.chooseAllBox, &QCheckBox::stateChanged, this, [=](bool state)
	{
		for (std::size_t i = 0; i < m_listWidgetVector.size(); i++)
		{
			QListWidget* s_listWidget = m_listWidgetVector[i];

			for (std::size_t j = 0; j < s_listWidget->count(); j++)
			{
				QListWidgetItem*item = s_listWidget->item(j);
				ListWidgetItemFrame*widget = dynamic_cast<ListWidgetItemFrame*>(s_listWidget->itemWidget(item));
				if (widget)
				{
					widget->setChecked(state);
				}
			}
		}
	}
	);
}

SYAdminQuestionsMgrWindow::~SYAdminQuestionsMgrWindow()
{
   
}

void SYAdminQuestionsMgrWindow::InitPage()
{

	QStringList modulesInfoArray = ConfigFileOperation::getInstance()->readFile();
	m_btnVector.clear();
	
	QListWidget* btnList = new QListWidget;
	btnList->setObjectName("btnList");

	for (std::size_t i = 0; i < modulesInfoArray.size(); i++)
	{
		QString moduleName = modulesInfoArray[i].split(',')[0];
		QString moduleIconPath = modulesInfoArray[i].split(',')[1];
		int type = modulesInfoArray[i].split(',')[2].toInt();

		QHBoxLayout* hLayout = new QHBoxLayout();
		QHBoxLayout* contentLayout = new QHBoxLayout();
		QLabel* iconLabel;
		if (moduleIconPath.size() > 0)
		{
			iconLabel = new QLabel;
			iconLabel->setPixmap(moduleIconPath);
			iconLabel->setFixedSize(QSize(60, 60));
			contentLayout->addWidget(iconLabel);
			//contentLayout->addItem(new QSpacerItem(10, QSizePolicy::Fixed));
		}
		QLabel* moduleText = new QLabel(moduleName);
		moduleText->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		moduleText->setStyleSheet("font-size:20px;color:#ffffff;");
		contentLayout->addWidget(moduleText);

		QPushButton* btn = new QPushButton;
		btn->setProperty("type", type);
		btn->setObjectName(moduleName);
		//btn->setFixedSize(280, 90);
		btn->setCheckable(true);
		hLayout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
		hLayout->addLayout(contentLayout);
		hLayout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
		btn->setLayout(hLayout);
		m_btnVector.push_back(btn);
		connect(btn, &QPushButton::clicked, this, &SYAdminQuestionsMgrWindow::do_module_Btn_Clicked);
	
		//insert btnList
		QListWidgetItem* item = new QListWidgetItem(btnList);
		item->setSizeHint(QSize(280, 90));
		btnList->setItemWidget(item, btn);

		QListWidget* listWidget = new QListWidget;
		listWidget->setProperty("type", type);
		m_listWidgetVector.push_back(listWidget);
		listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		QVector<QSqlRecord> records;  //占位
		m_allRecords.push_back(records);


		connect(listWidget, &QListWidget::itemClicked, this, [=](QListWidgetItem* item)
		{
			bool flag = item->data(Qt::UserRole).toBool();
			if (!flag)
			{
				item->setData(Qt::UserRole, 1);  //更新状态
				ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(listWidget->itemWidget(item));
				widget->setChecked(true);
			}
			else
			{
				item->setData(Qt::UserRole, 0);  //更新状态
				//item->setBackgroundColor(QColor("#31394c"));
				ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(listWidget->itemWidget(item));
				widget->setChecked(false);
			}
		});
	}

	QVBoxLayout*vLayout = new QVBoxLayout();
	vLayout->addWidget(btnList);
	vLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
	vLayout->setSpacing(0);
	vLayout->setContentsMargins(QMargins(0,0,0,0));
	ui.btnBgWidget->setLayout(vLayout);
	btnList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	btnList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

QListWidget* SYAdminQuestionsMgrWindow::SetListWidget(int type)
{
	//读取数据并显示
	QVector<QSqlRecord> records;
	QListWidget* listWidget;
	bool flag = true;
	SYDBMgr::Instance()->QueryPaperInfo(type, records);


	////新建试题需置顶
	//if (newQuest)
	//{
	//	QSqlRecord lastRecord = records[records.size() - 1];
	//	records.pop_back();
	//	records.insert(records.begin(), lastRecord);
	//}

	for (std::size_t i = 0; i < m_listWidgetVector.size(); i++)
	{
		QListWidget* s_listWidget = m_listWidgetVector[i];
		if (s_listWidget)
		{
			auto t = s_listWidget->property("type");
			int s_type = t.toInt();
			if (s_type == type)
			{
				listWidget = m_listWidgetVector[i];
				m_allRecords[i] = records;
				flag = false;
				break;
			}
		}	
	}
	if (flag)
	{
		listWidget = new QListWidget;
	}
	listWidget->clear();
	//重写listwidget
	int index = 0;
	for (std::size_t i = 0; i<records.size();i++)
	{
		QSqlRecord record = records[i];
		
		ListWidgetItemFrame* widget = new ListWidgetItemFrame(listWidget, record, index,false);
		//widget->setFixedWidth(1200);
		widget->setQuestionFrameHintSize(1214);
		QListWidgetItem *item = new QListWidgetItem(listWidget);
		item->setData(Qt::UserRole, 0);
		int itemHeight = widget->getItemHeight();
		item->setSizeHint(QSize(1200, itemHeight));
		listWidget->setItemWidget(item, widget);
		listWidget->setSpacing(5);
		//listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	
		QuestionFrame* queFrame = widget->getQuestionFrame();
		QPushButton* titleBtn = queFrame->GetBtn();

		connect(titleBtn, &QPushButton::clicked, this, [=]
		{
			QSqlRecord* t_record = new QSqlRecord(record);
			do_newBuiltQuestion(t_record);

		});
		QCheckBox* box = widget->getCheckBox();
		connect(box, &QCheckBox::stateChanged, this, [=](int state)
		{
			bool flag = state;
			widget->setChecked(flag);
		});	

		index++;
	}
	return listWidget;
}

void SYAdminQuestionsMgrWindow::do_module_Btn_Clicked()
{
	QPushButton* btn = static_cast<QPushButton*>(sender());
	if (m_curBtn->objectName() == btn->objectName())
	{
		m_curBtn->setChecked(true);
		return;
	}
	m_curBtn->setChecked(false);
	m_curBtn = btn;
	m_curBtn->setChecked(true);

	int type = m_curBtn->property("type").toInt();
	QListWidget* curwidget = SetListWidget(type);

	QWidget* oldWidget = ui.stackedWidget->currentWidget();
	ui.stackedWidget->removeWidget(oldWidget);

	ui.stackedWidget->addWidget(curwidget);
	ui.stackedWidget->setCurrentWidget(curwidget);
}

void SYAdminQuestionsMgrWindow::do_batchImportData()
{

	QString inputFilePath = SYStringTable::GetString(SYStringTable::STR_UploadFileTips);
	QStringList fileNames = QFileDialog::getOpenFileNames(this, inputFilePath, "", "xls (*.xls *.xlsx)");

	for (std::size_t i = 0; i < fileNames.size(); i++)
	{
		QString fileName = fileNames[i];
		QVector<QVector<QVariant>> datas;
		bool ok = Mx::readDataFromExcelFile(fileName, datas, 9);

		//cols:3 -answer
		QVector<QVector<QString>> paperQuestions;
		for (int i = 0; i < datas.size(); i++)
		{
			if (i == 0) //表格表头
				continue;

			QVector<QString> question;
			QString titleChooseType = datas[i][0].toString();  //单选or多选

			QString title = datas[i][2].toString();
			QString answer = datas[i][3].toString();
			QString a = datas[i][4].toString();
			QString b = datas[i][5].toString();
			QString c = datas[i][6].toString();
			QString d = datas[i][7].toString();
			QString e = datas[i][8].toString();
		

			QString questionType = datas[i][1].toString();//本文映射类型
			QString type;
			int j;
			for ( j = 0;j< m_btnVector.size(); j++)
			{
				if (m_btnVector[j]->objectName()==questionType)
				{
					type = m_btnVector[j]->property("type").toString();
					break;
				}
			}
			if (j == m_btnVector.size())
			{
				continue;
				//不存在的类型过滤
			}

			question.push_back(a);
			question.push_back(b);
			question.push_back(c);
			question.push_back(d);
			question.push_back(e);
			question.push_back(title);
			question.push_back(answer);
			question.push_back(type);

			paperQuestions.push_back(question);
		}

		SYDBMgr::Instance()->FromExcelUpLoadPaperInfo(paperQuestions);
	}

	RefreshWidget();
}
void SYAdminQuestionsMgrWindow::do_deleteQuestions()
{
	QVector<QSqlRecord> delRecords;

	for (std::size_t i = 0; i < m_listWidgetVector.size(); i++)
	{
		QListWidget* s_listWidget = m_listWidgetVector[i];

		for (std::size_t j = 0; j < s_listWidget->count(); j++)
		{
			QListWidgetItem*item = s_listWidget->item(j);
			ListWidgetItemFrame*widget = dynamic_cast<ListWidgetItemFrame*>(s_listWidget->itemWidget(item));
			if (widget)
			{
				if (widget->IsChecked())
				{
					widget->setChecked(false);
					QSqlRecord record = m_allRecords[i][j];
					delRecords.push_back(record);
				}
			}
			
		}
	}

	if (delRecords.size() == 0)
	{
		SYMessageBox* box = new SYMessageBox(this, "", CHS("请选择需要删除的试题！"), 1);
		box->showFullScreen();
		box->exec();
		return;
	}

	SYMessageBox* box = new SYMessageBox(this, "", CHS("是否确认删除选中的试题?"), 2);
	box->showFullScreen();
	int ret=box->exec();
	if (ret==2)
		SYDBMgr::Instance()->DeleteQuestions(delRecords);
	//页面刷新
	RefreshWidget();
}

void SYAdminQuestionsMgrWindow::RefreshWidget()
{

	//页面刷新
	int type = m_curBtn->property("type").toInt();
	QListWidget* curWidget = SetListWidget(type);
	QWidget* oldWidget = ui.stackedWidget->currentWidget();
	if (oldWidget)
		ui.stackedWidget->removeWidget(oldWidget);


	if (curWidget)
	{
		ui.stackedWidget->addWidget(curWidget);
		ui.stackedWidget->setCurrentWidget(curWidget);
	}


}
void SYAdminQuestionsMgrWindow::do_newBuiltQuestion(QSqlRecord* record)
{
	int type = m_curBtn->property("type").toInt();
	SYNewBuiltQuestionWindow* newWin = new SYNewBuiltQuestionWindow(type, record, this);
	newWin->showFullScreen();
	bool flag;
	flag = newWin->exec();
	if (flag)
	{
		SYMessageBox* box = new SYMessageBox(this, "", CHS("试题创建成功！"), 1);
		box->showFullScreen();
		box->exec();
	}
	/*else
	{
		SYMessageBox* box = new SYMessageBox(this, "", CHS("题目创建失败或者题目已经存在！"), 1);
		box->showFullScreen();
		box->exec();
	}*/

	RefreshWidget();

}
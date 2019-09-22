#include "SYAdminNewPaperWindow.h"
#include "ui_SYAdminNewPaperWindow.h"
#include"SYAdminPaperContentShowWindow.h"
#include"SYAdminPaperInfoEditWindow.h"
#include "SYAdminShowCreatingPaperWindow.h"
#include "SYAdminPaperRandomChooseWindow.h"
#include"SYUserInfo.h"
#include "MxDefine.h"
#include <SYDBMgr.h>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <initializer_list>
#include<qlistwidget.h>
#include"SYStringTable.h"
#include <SYMessageBox.h>
#include "SYMainWindow.h"
#include<qpushbutton.h>

#include <Windows.h>
#include<Qmutex>
#include<qthread.h>


SYAdminNewPaperWindow::SYAdminNewPaperWindow(QString t_paperName , QWidget *parent) :
RbShieldLayer(parent), m_paperName(t_paperName)

{
    ui.setupUi(this);

	hideOkButton();
	hideCloseButton();

	SetMetaType();
	LoadData();
	//设置所有页面
	{
		for (std::size_t i = 0; i < m_listWidgets.size(); i++)
		{
			int type = m_listWidgets[i]->property("type").toInt();
			SetWidgetContent(type);
		}
	}
	//初始化设置
	m_curBtn = m_btnVector[0];
	m_curBtn->setChecked(true);
	RefreshTable();
	if (m_paperName.size() > 0)
	{
		ui.newTitle->setText(CHS("编辑试卷"));
		ui.randomSearch->setVisible(false);
		ui.addFinished->setText(CHS("编辑完成"));
		SetPaperEditInfo();
	}
	Mx::setWidgetStyle(this, "qss:SYAdminNewPaperWindow.qss");
}

SYAdminNewPaperWindow::~SYAdminNewPaperWindow()
{
}

void SYAdminNewPaperWindow::SetMetaType()
{
	m_btnVector.clear();

	QStringList modulesInfoArray = ConfigFileOperation::getInstance()->readFile();
	QListWidget* btnList = new QListWidget;
	btnList->setObjectName("btnList");
	btnList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	for (std::size_t i = 0; i < modulesInfoArray.size(); i++)
	{
		QString moduleName = modulesInfoArray[i].split(',')[0];
		QString moduleIconPath = modulesInfoArray[i].split(',')[1];
		int type = modulesInfoArray[i].split(',')[2].toInt();
		m_typeList.push_back(type);
		QPushButton* btn = new QPushButton();
		QHBoxLayout* hLayout = new QHBoxLayout();
		QHBoxLayout*contentLayout = new QHBoxLayout();

		QLabel* label1 = new QLabel();
		if (moduleIconPath.size() > 0)
		{
			label1->setPixmap(QPixmap(moduleIconPath));
			label1->setFixedSize(60, 60);
			contentLayout->addWidget(label1);
			//contentLayout->addItem(new QSpacerItem(10, QSizePolicy::Fixed));
			
		}
		QLabel* label2 = new QLabel();
		label2->setText(moduleName);
		label2->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		label2->setStyleSheet("font-size:20px;color:#ffffff;");
		contentLayout->addWidget(label2);

		hLayout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
		hLayout->addLayout(contentLayout);
		hLayout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
		//hLayout->setSpacing(4);
		btn->setLayout(hLayout);
		btn->setProperty("type", type);
		btn->setObjectName("moduleBtn");
		//btn->setFixedSize(280, 90);
		btn->setCheckable(true);
		m_btnVector.push_back(btn);
		connect(btn, &QPushButton::clicked, this, &SYAdminNewPaperWindow::ShowPaperContent);
		QListWidgetItem* item = new QListWidgetItem(btnList);
		item->setSizeHint(QSize(284, 90));
		btnList->setItemWidget(item, btn);

		QListWidget* listWidget = new QListWidget;
		listWidget->setProperty("type", type);
		m_listWidgets.push_back(listWidget);
		listWidget->setContentsMargins(0, 15, 0, 0);

		connect(listWidget, &QListWidget::doubleClicked, this, [=]()
		{
			QListWidgetItem *item = listWidget->currentItem();
			QuestionFrame* frame = dynamic_cast<QuestionFrame*>(listWidget->itemWidget(item));
			QSqlRecord record = frame->getItemRecord();
			int id = record.value("id").toInt();
			bool checkState = item->data(Qt::UserRole).toBool();
			bool flag = !checkState;
			int row = listWidget->currentRow();
			if (flag)
			{
				item->setData(Qt::UserRole, 1);  //更新状态
				item->setBackgroundColor(QColor(39,74,112)); 
				int flag = true;
				QuestionInfo checkedQuestionInfo(row, type, record); //选中试卷信息
				for (std::size_t i = 0; i < m_chosedQuestions.size(); i++)
				{
					QuestionInfo queInfo = m_chosedQuestions[i];
					QSqlRecord t_record = queInfo.record;
					int t_id = t_record.value("id").toInt();
					if (id == t_id)
					{
						flag = false;
					}
				}
				if (flag)
				{
					m_chosedQuestions.push_back(checkedQuestionInfo);
				}
			}
			else
			{
				item->setData(Qt::UserRole, 0);  //更新状态
				item->setBackgroundColor(QColor("#2c3345"));


				QuestionInfo checkedQuestionInfo(row, type, record); //选中试卷信息
				for (std::size_t i = 0; i < m_chosedQuestions.size(); i++)
				{
					QuestionInfo queInfo = m_chosedQuestions[i];
					QSqlRecord t_record = queInfo.record;
					int t_id = t_record.value("id").toInt();
					if (id == t_id)
					{
						m_chosedQuestions.removeAt(i);
						break;
					}
				}
			

			}
			RefreshItemStyle();

		});
	}

	QVBoxLayout* vLayout = new QVBoxLayout();
	vLayout->addWidget(btnList);
	vLayout->setSpacing(0);
	vLayout->setContentsMargins(QMargins(0, 0, 0, 0));
	ui.btnWidget->setLayout(vLayout);

	btnList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//添加完成
	connect(ui.addFinished, &QPushButton::clicked, this, [=]
	{	//编辑试卷信息，确认上传试卷。

		int chosedPaperNum = m_chosedQuestions.size();
		if (chosedPaperNum <= 0)
		{
			//已选试题为空！
			QString notChosedNumLabel = SYStringTable::GetString(SYStringTable::STR_QuestionNotChosed);
			SYMessageBox* messageBox = new SYMessageBox(this, "", notChosedNumLabel, 1);
			messageBox->setPicture("Shared/personnel_pic_.png");  //图片需要修改
			messageBox->showFullScreen();
			bool ok = messageBox->exec();  //阻塞
			return;
		}
		PaperInfoDetail* t_paperInfoDetail=0;
		QVector<QSqlRecord > records;
		if (m_paperName.size() > 0)
		{

			SYDBMgr::Instance()->QueryPaperInfo(m_paperName, &t_paperInfoDetail, records);

		}
		
		SYAdminPaperInfoEditWindow *editWin = new SYAdminPaperInfoEditWindow(chosedPaperNum, this,t_paperInfoDetail);
		connect(editWin, SIGNAL(createPaper(QString, QString, QString)), this, SLOT(doCreatePaper(QString, QString, QString)));

		editWin->showFullScreen();
		bool ok=editWin->exec();
		if (ok)
		{
			done(RC_Ok);
		}

	});
	//已选试题
	connect(ui.chosedPaperBtn, &QPushButton::clicked, this, &SYAdminNewPaperWindow::ShowChosedQuestions);

	//随机抽题
	connect(ui.randomSearch, &QPushButton::clicked, this, [=]{

		SYAdminPaperRandomChooseWindow* randomChoseWin = new SYAdminPaperRandomChooseWindow(m_allQueryPaperInfo, this);
		randomChoseWin->showFullScreen();
		bool flag=randomChoseWin->exec();
		if (flag)
		{
			done(RC_Ok);
		}
	});

	connect(ui.searchBtn, &QPushButton::clicked, this, [=]()
	{
		RefreshTable();
	});

	connect(ui.returnBtn, &QPushButton::clicked, this, [=]()
	{
		int queNumber = m_chosedQuestions.size();
		if (queNumber > 0 && m_paperName.isEmpty())
		{
			SYMessageBox* confirmRetWin = new SYMessageBox(this, CHS(""), CHS("已经选中试题，是否确定退出？"), 2);
			confirmRetWin->showFullScreen();
			int ret = confirmRetWin->exec();
			if (ret == 1)
			{
				return;
			}
		}
		if (!m_paperName.isEmpty())
		{
			SYMessageBox* confirmRetWin = new SYMessageBox(this, CHS(""), CHS("正在编辑试卷，是否确定退出？"), 2);
			confirmRetWin->showFullScreen();
			int ret = confirmRetWin->exec();
			if (ret == 1)
			{
				return;
			}
		}
		done(RC_Ok);

	});

	connect(ui.searchLineEdit, &QLineEdit::textChanged, this, [=](QString txt)
	{
		RefreshTable();
		RefreshItemStyle();
	
	});

}


void SYAdminNewPaperWindow::SetPaperEditInfo()
{
	QVector<QSqlRecord> records;
	PaperInfoDetail* paperInfoDetail = 0;
	SYDBMgr::Instance()->QueryPaperInfo(m_paperName, &paperInfoDetail, records);
	QVector<int> record_idVector;
	for (std::size_t i = 0; i < records.size(); i++)
	{
		record_idVector.push_back(records[i].value("id").toInt());
	}

	for (std::size_t i = 0; i < m_listWidgets.size(); i++)
	{
		for (std::size_t j = 0; j < m_allQueryPaperInfo[i].size(); j++)
		{
			QSqlRecord record = m_allQueryPaperInfo[i][j];
			int id = record.value("id").toInt();
			int row = j;
			int type = m_listWidgets[i]->property("type").toInt();
			if (record_idVector.contains(id))
			{
				QListWidgetItem* item = m_listWidgets[i]->item(j);

				if (item)
				{

					item->setData(Qt::UserRole, 1);  //更新状态
					item->setBackgroundColor(QColor(39, 74, 112));
					int flag = false;
					QuestionInfo checkedQuestionInfo(row, type, record); //选中试卷信息
					for (std::size_t i = 0; i < m_chosedQuestions.size(); i++)  //避免重复被选中
					{
						QuestionInfo queInfo = m_chosedQuestions[i];
						QSqlRecord t_record = queInfo.record;
						int t_id = t_record.value("id").toInt();
						if (id == t_id)
						{
							flag = true;
							break;
						}
					}
					if (!flag)
					{
						m_chosedQuestions.push_back(checkedQuestionInfo);
					}


				}
			}
		}

	}
	//QVector<QuestionInfo>* createPaperInfo = GetCreatePaperInfo();
	RefreshItemStyle();

}

int SYAdminNewPaperWindow::GetBtnType(QPushButton* Btn)
{
	if (Btn)
	{
		return Btn->property("type").toInt();
	}
	return -1;

}

void SYAdminNewPaperWindow::LoadData()
{	
	auto FromMysqlLoadData = [=](int i, QVector<QSqlRecord> &results)
	{
		int type = m_typeList[i];
		SYDBMgr::Instance()->QueryPaperInfo(type, results);	
	};

	m_allQueryPaperInfo.resize(m_typeList.size());   //resize预留空间，但没有初始化

	for (std::size_t i = 0; i < m_typeList.size(); i++)
	{
		QVector<QSqlRecord> &results= m_allQueryPaperInfo[i];
		FromMysqlLoadData(i,results);
	}

}


QListWidget* SYAdminNewPaperWindow::SetWidgetContent(int type)
{
	int index=0;
	for (std::size_t i = 0; i < m_listWidgets.size(); i++)
	{
		if (m_listWidgets[i]->property("type").toInt() == type)
		{
			index = i;
			break;
		}
	}

	QVector<QSqlRecord> records = m_allQueryPaperInfo[index];

	//int rows = m_listWidgets[index]->count();
	//if ( rows>0&&ui.searchLineEdit->text()==0)  //减少页面的重复读写操作
	//{
	//	return m_listWidgets[index];
	//}

	m_listWidgets[index]->clear();  //页面内容清除
	int row = 0;
	for (std::size_t j = 0; j < records.size(); j++)
	{
		QSqlRecord record = records[j];
		if (FilterRecord(record) == false)   //搜索过滤器
			continue;
		QuestionFrame* frame = new QuestionFrame(record, row);
		frame->setObjectName("questionFrame");
		QListWidgetItem* item = new QListWidgetItem(m_listWidgets[index]);
		item->setData(Qt::UserRole, 0); //是否被选中
		int itemHeight = frame->getHeight();
		item->setSizeHint(QSize(1250, itemHeight));

		m_listWidgets[index]->setItemWidget(item, frame);
		m_listWidgets[index]->setSpacing(5);
		m_listWidgets[index]->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

		row++;
	}
	return m_listWidgets[index];
	
}

void SYAdminNewPaperWindow::ShowPaperContent()  //创建显示窗口并添加到widget
{
	QPushButton* Btn = (QPushButton*)sender();  //按钮每次点击状态都会改变
	if (Btn == m_curBtn)
	{
		Btn->setChecked(true);
		return;
	}
	m_curBtn->setChecked(false);
	m_curBtn = Btn;
	m_curBtn->setChecked(true);

	RefreshTable();

}


void SYAdminNewPaperWindow::RefreshTable()
{
	int type = GetBtnType(m_curBtn);

	QWidget* curWidget = dynamic_cast<QWidget*>( SetWidgetContent(type));
	QWidget* oldWidget = ui.contentStackedWidget->currentWidget();
	if (oldWidget)
		ui.contentStackedWidget->removeWidget(oldWidget);

	ui.contentStackedWidget->addWidget(curWidget);
	if (curWidget)
		ui.contentStackedWidget->setCurrentWidget(curWidget);
	RefreshItemStyle();

}

void SYAdminNewPaperWindow::RefreshItemStyle()
{
	QMultiMap<int, int> ques_id_type;
	QStringList strList;
	
	for (auto it = m_chosedQuestions.begin(); it != m_chosedQuestions.end(); it++)
	{
		QString str = (*it).this_type;
		strList = str.split("_", QString::SkipEmptyParts);
		int type = strList[0].toInt();
		int id = strList[1].toInt();
		ques_id_type.insert(type, id);
	}
	int typeSize = m_allQueryPaperInfo.size();
	for (std::size_t i = 0; i < typeSize; i++)
	{
		for (std::size_t j = 0; j < m_allQueryPaperInfo[i].size(); j++)
		{
			int type = m_typeList[i];
			int id = j;
			bool flag = ques_id_type.contains(type, id);
			int itemNumber = m_listWidgets[i]->count();
			if (itemNumber - 1<j)
				continue;
			QListWidgetItem* item = m_listWidgets[i]->item(j);
			if (item)
			{
				if (flag)
				{
					item->setData(Qt::UserRole, 1);  //更新状态
					item->setBackgroundColor(QColor(39, 74, 112));
				}
				else
				{
					item->setData(Qt::UserRole, 0);  //更新状态
					item->setBackgroundColor(QColor("#2c3345"));
				}
			}
		}
	}
	//QString chosedNumLabel = SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum);
	int itemChosedNumber = m_chosedQuestions.size();
	ui.chosedPaperBtn->setText(QString::fromLocal8Bit("%1").arg(itemChosedNumber));
}


void SYAdminNewPaperWindow::keyReleaseEvent(QKeyEvent *event)
{

	int key = event->key();
	if (key == Qt::Key_Enter || key == Qt::Key_Return)
	{
		RefreshTable();
		RefreshItemStyle();
	}
}

bool SYAdminNewPaperWindow::FilterRecord(QSqlRecord &record)
{
	QString text = ui.searchLineEdit->text();

	if (text.size() == 0)
		return true;
	QString title = record.value("title").toString();
	if (title.contains(text, Qt::CaseInsensitive))
		return true;

	if (text == "0")
		return true;
	return false;


}

void SYAdminNewPaperWindow::doCreatePaper(QString t_paperName, QString examTime, QString  examScore)
{
	QString paperName = t_paperName; 
	QVector<QSqlRecord> questions;  
	//QVector<QuestionInfo>* createPaperInfo = GetCreatePaperInfo();

	for (auto it = m_chosedQuestions.begin(); it != m_chosedQuestions.end(); it++)
	{
		questions.push_back((*it).record);
	} 
	//记录新试卷信息到Paperlist数据库中
	QString midLevel = SYStringTable::GetString(SYStringTable::STR_MidLevel);
	QString label = midLevel;
	QString questionNum = QString::number(questions.size());
	QString creator = SYUserInfo::s_Instance->GetRealName();
	QString paperTotalScore=examScore, examTotalTime=examTime;   
	std::vector<QString> paperImpInfo = { paperName, label, questionNum, creator, examTotalTime, paperTotalScore };
	bool ret=SYDBMgr::Instance()->CreatePaperInfo(paperImpInfo, questions,m_paperName);

	//QVector<QuestionInfo>* chosedPaperInfo = GetCreatePaperInfo();
	//页面更新
	RefreshItemStyle();
}

void SYAdminNewPaperWindow::ShowChosedQuestions()
{
	//QVector<QuestionInfo>* createPaperInfo = GetCreatePaperInfo();
	int chosedPaperNum = m_chosedQuestions.size();
	if (chosedPaperNum <= 0)
	{
		//已选试题为空！
		QString notChosedNumLabel = SYStringTable::GetString(SYStringTable::STR_QuestionNotChosed);
		SYMessageBox* messageBox = new SYMessageBox(this, "", notChosedNumLabel, 1);
		messageBox->setPicture("Shared/personnel_pic_.png");  //图片需要修改
		messageBox->showFullScreen();

		bool ok = messageBox->exec();
		return;
	}

	SYAdminShowCreatingPaperWindow* createPaperWin = new SYAdminShowCreatingPaperWindow(m_chosedQuestions, this);

	//connect(createPaperWin, &SYAdminShowCreatingPaperWindow::on_CreatingPaperUpdate, this, &SYAdminNewPaperWindow::on_do_CreatingPaperUpdate);

	createPaperWin->showFullScreen();
	bool flag = createPaperWin->exec();

	//页面更新
	RefreshItemStyle();

}

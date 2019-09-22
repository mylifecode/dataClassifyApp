#include "SYAdminTheoryTestWindow.h"
#include"ui_SYAdminTheoryTestWindow.h"
#include "SYAdminShowPaperDetailWindow.h"
#include"SYAdminSendPaperWindow.h"
#include"SYAdminModifyPaperInfoWindow.h"
#include"SYAdminNewPaperWindow.h"
#include"SYAdminUpLoadPaperWindow.h"
#include "SYAdminQuestionsMgrWindow.h"
#include"SYStringTable.h"
#include "MxDefine.h"
#include <SYDBMgr.h>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <initializer_list>

#include <SYMessageBox.h>
#include"SYAddModuleWindow.h"
#include "SYMainWindow.h"
#include "qfile.h"


SYAdminTheoryTestWindow::SYAdminTheoryTestWindow(QWidget *parent) :
QWidget(parent),
m_curSelectedBtn(nullptr),
m_paperSetTable(nullptr),
m_questionBankTable(nullptr)

{
	ui.setupUi(this);

	Initialize();

	Mx::setWidgetStyle(this, "qss:SYAdminTheoryTestWindow.qss");
}

SYAdminTheoryTestWindow::~SYAdminTheoryTestWindow()
{

}


void SYAdminTheoryTestWindow::Initialize()
{

	InitTables();  //����stackwidget���ű�ͷ����ͱ���ʾ����

	LoadData();    //�������ݲ���ѧ������ʦ���ݷ���

	SetTableContent();

	//ѧԱ��������
	m_curSelectedBtn = ui.paperMgrBtn;   //Ĭ����ʾ��һҳ

	ui.paperMgrBtn->setChecked(true);
	ui.questionBankMgrBtn->setChecked(false);
	ui.stackedWidget->setCurrentIndex(0);  //��ʾ��ջ�е�һ����

	connect(ui.leftMenuWindow, &SYMenuWindow::showNextWindow, this, &SYAdminTheoryTestWindow::showNextWindow);
	connect(ui.paperMgrBtn, &QPushButton::clicked, this, &SYAdminTheoryTestWindow::on_paperMgrBtn_clicked);
	connect(ui.questionBankMgrBtn, &QPushButton::clicked, this, &SYAdminTheoryTestWindow::on_questionBankMgrBtn_clicked);
	connect(ui.searchBtn, &QPushButton::clicked, this, [=]
	{
		RefreshTable();

	});
	connect(ui.sendPaperBtn, &QPushButton::clicked, this, [=]()
	{
		SYAdminSendPaperWindow* sendPaperWin = new SYAdminSendPaperWindow(this);
		sendPaperWin->showFullScreen();
		sendPaperWin->exec();
	});
	connect(ui.searchLineEdit, &QLineEdit::textEdited, this, [=]()
	{
		SetTableContent();

	});

}

void SYAdminTheoryTestWindow::InitTables()//lambda����   //�ȶ�����
{
	auto InitImpl = [](QTableWidget* table, int columnCount, std::initializer_list<QString> horizontalLabels, std::vector<int> sectionWidths){
		table->setColumnCount(columnCount);   //std::initializer_list�ɱ��������

		QStringList labels;
		for (const auto& lb : horizontalLabels)
			labels.append(lb);
		table->setHorizontalHeaderLabels(labels);   //���ñ�ͷ

		table->setFrameShape(QFrame::NoFrame);  //�ޱ߿�
		table->setShowGrid(false);  //����ʾ����
		table->setSelectionMode(QAbstractItemView::SingleSelection);  //���õ�ѡģʽ
		table->setSelectionBehavior(QAbstractItemView::SelectRows);//ѡ������ģʽ
		table->setAlternatingRowColors(true);
		table->setFocusPolicy(Qt::NoFocus);

		QHeaderView* headerView = table->horizontalHeader();  //����б�ͷ
		headerView->setStretchLastSection(true);   //���һ����չ
		headerView->setHighlightSections(false);
		headerView->setObjectName("horizontalHeader");
		//headerView->setSectionsMovable(false);
		headerView->setEnabled(false);

		//set section width
		for (std::size_t i = 0; i < sectionWidths.size(); ++i){
			table->setColumnWidth(i, sectionWidths[i]);   //�����п�
		}

		headerView = table->verticalHeader();
		headerView->hide();  //��ֱ��ͷ����
	};
	QStringList headerLabels;

	//��ʦ��
	m_paperSetTable = ui.paperSetTable;

	InitImpl(m_paperSetTable,
		6,
		{ SYStringTable::GetString(SYStringTable::STR_PAPERNAME), SYStringTable::GetString(SYStringTable::STR_PAPERLABEL), \
		SYStringTable::GetString(SYStringTable::STR_QUESTIONNUM), SYStringTable::GetString(SYStringTable::STR_CREATEDATE), \
		SYStringTable::GetString(SYStringTable::STR_CREATROR), SYStringTable::GetString(SYStringTable::STR_OPERATION) },
		{ 212, 300, 217, 297, 280, 75 });  //fromLocal8Bit GB תΪunicode���п���ʱ����
	//ui.stackedWidget->addWidget(m_paperSetTable);

	//���ģ����ʱ��д
}  

void SYAdminTheoryTestWindow::LoadData()
{


	m_paperSetVec.clear();	
	SYDBMgr::Instance()->QueryInfo(SYDBMgr::DatabaseTable::DT_ExamPaperList, m_paperSetVec);
}

void SYAdminTheoryTestWindow::SetTableContent()
{
	const int rowHeight = 70;
	const QString dateTimeFormat = SYStringTable::GetString(SYStringTable::STR_DATETIMEFORMAT);
		//QString("yyyy%1MM%2dd%3 hh:mm:ss").arg(QString::fromLocal8Bit("��")).arg(QString::fromLocal8Bit("��")).arg(QString::fromLocal8Bit("��"));
	auto SetItemAttribute = [](QTableWidgetItem* item, const QString& text, Qt::Alignment align){  //����ÿ����ÿһ������
		item->setText(text);
		item->setTextAlignment(align); //�ı�ͣ��
		item->setTextColor(QColor(0xb7, 0xc5, 0xd8));

		QFont font = item->font();
		font.setPixelSize(16);  //�����ı����ش�С
		item->setFont(font);   //������������

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);
	};

	bool canOperate = true;

	//�Ծ����
	if (m_paperSetTable){

		int nPaperRecord = m_paperSetVec.size();  //sql��ȡ���ݼ�¼
		m_paperSetTable->setRowCount(nPaperRecord);  //�������� 

		m_paperSetTable->clearContents();
		QString name,label;
		QString questionNum,datetime;
		int row = 0;
		for (int i= 0; i < nPaperRecord; ++i){	
			const auto& record = m_paperSetVec[i];
			int userId = record.value("id").toInt();
		
			if (FilterRecord(record) == 0)
				continue;

			//col 0
			QTableWidgetItem* item = new QTableWidgetItem;
			item->setData(Qt::UserRole, userId);		//ɾ��ʱ������ɾ����һ�е�����
			name = record.value("paperName").toString();
			if (name.size() == 0)
				name.push_back("-");
			SetItemAttribute(item, name, Qt::AlignHCenter | Qt::AlignVCenter);  //lamda�������޷���ֵ��
			m_paperSetTable->setItem(row, 0, item);

			//col 1
			item = new QTableWidgetItem;
			label = record.value("paperRank").toString();
			if (label.size() == 0)
				label.push_back("-");
			SetItemAttribute(item, label, Qt::AlignHCenter | Qt::AlignVCenter); //�������ݼ�¼item����ʾ����
			m_paperSetTable->setItem(row, 1, item);  //�ڹ̶��в�ͬ����������

			//col 2
			item = new QTableWidgetItem;
			questionNum = record.value("questionNum").toString();
			if (questionNum.size() == 0)
				questionNum.push_back("-");
			SetItemAttribute(item, questionNum, Qt::AlignHCenter | Qt::AlignVCenter);
			m_paperSetTable->setItem(row, 2, item);

			//col 3
			item = new QTableWidgetItem;
			QDateTime dataTime = record.value("createDatetime").toDateTime();
			datetime = dataTime.toString(dateTimeFormat);

		
			if (datetime.size() == 0)
				datetime.push_back("-");
			SetItemAttribute(item, datetime, Qt::AlignHCenter | Qt::AlignVCenter);
			m_paperSetTable->setItem(row, 3, item);

			//col 4
			item = new QTableWidgetItem;
			SetItemAttribute(item, record.value("creator").toString(), Qt::AlignHCenter | Qt::AlignVCenter);
			m_paperSetTable->setItem(row, 4, item);

			//col 5
			QWidget* widget = new QWidget;
			QHBoxLayout* hLayout = new QHBoxLayout;

			QPushButton* showBtn = new QPushButton();
			showBtn->setStyleSheet(QString("background-color:transparent;border:none;font-size:16px;color:#00ff00;"));
			showBtn->setText(SYStringTable::GetString(SYStringTable::STR_Check));  //�鿴
			showBtn->setObjectName(QString("showBtn_%1").arg(row));
			showBtn->setEnabled(canOperate);
			showBtn->setProperty("userId", userId);    //
			connect(showBtn, &QPushButton::clicked, this, &SYAdminTheoryTestWindow::onShowPaperBtn_clicked);
			

			QPushButton* deleteBtn = new QPushButton();
			deleteBtn->setStyleSheet(QString("background-color:transparent;border:none;font-size:16px;color:#ff0000;"));
			deleteBtn->setText(SYStringTable::GetString(SYStringTable::STR_Delete));  //ɾ��
			deleteBtn->setObjectName(QString("deleteBtn_%1").arg(row));   
			deleteBtn->setEnabled(canOperate);
			deleteBtn->setProperty("userId", userId);       //
			connect(deleteBtn, &QPushButton::clicked, this, &SYAdminTheoryTestWindow::onDeleteRecord);


			QPushButton* modifyPaperBtn = new QPushButton();
			modifyPaperBtn->setStyleSheet(QString("background-color:transparent;border:none;font-size:16px;color:#00ff00;"));
			modifyPaperBtn->setText(SYStringTable::GetString(SYStringTable::STR_Modify));  //�޸�
			modifyPaperBtn->setObjectName(QString("modifyPaperBtn_%1").arg(row));
			modifyPaperBtn->setEnabled(canOperate);
			modifyPaperBtn->setProperty("userId", userId);
			connect(modifyPaperBtn, &QPushButton::clicked, this, [=]()
			{	
			
				QString paperName="";
				int userId = modifyPaperBtn->property("userId").toInt();

				for (int i = 0; i < m_paperSetTable->rowCount(); i++)
				{
					QTableWidgetItem* item = m_paperSetTable->item(i, 0);
					if (item->data(Qt::UserRole).toInt() == userId)
					{
						paperName = item->text();
						break;
					}
				}

				SYAdminNewPaperWindow* newPaperWin = new SYAdminNewPaperWindow(paperName, this);
				newPaperWin->showFullScreen();
				newPaperWin->exec();
				ui.searchLineEdit->clear();
				RefreshTable();  //��ʾ����
			});

			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(showBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(deleteBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(modifyPaperBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			widget->setLayout(hLayout);
			m_paperSetTable->setCellWidget(row, 5, widget);
			m_paperSetTable->setRowHeight(row, rowHeight);

			row++;
		}
	}		

}


void SYAdminTheoryTestWindow::RefreshTable()  //��ʾ����
{
	LoadData();
	SetTableContent();
}

//ҳ���л�


void SYAdminTheoryTestWindow::on_paperMgrBtn_clicked()
{
	ui.paperMgrBtn->setChecked(true);  //setCheckable(true)Ϊ����,��ʾ����ѡ�� setChecked(true)Ϊ���Ե�ֵ,��ʾ�Ѿ�ѡ��

	if (m_curSelectedBtn == ui.paperMgrBtn)
		return;

	if (m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = ui.paperMgrBtn;

	SetTableContent();
	//update page
	ui.stackedWidget->setCurrentIndex(0);  //��ʾ��ջ�е�һ����

	
}

void SYAdminTheoryTestWindow::on_questionBankMgrBtn_clicked()
{

	if (m_curSelectedBtn == ui.questionBankMgrBtn)
	{
		ui.questionBankMgrBtn->setChecked(true);
		return;
	}
	if (m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	ui.searchLineEdit->clear();

	m_curSelectedBtn = ui.questionBankMgrBtn;
	m_curSelectedBtn->setChecked(true);

	SetModulePage();

	ui.stackedWidget->setCurrentIndex(1);  //��ʾ��ջ�е�һ����

}

void SYAdminTheoryTestWindow::SetModulePage()
{

	m_moduleVector.clear();

	//�����ļ���ȡ
	QStringList modulesInfoArray = ConfigFileOperation::getInstance()->readFile();

	for (std::size_t i = 0; i < modulesInfoArray.size(); i++)
	{
		QString moduleName = modulesInfoArray[i].split(',')[0];
		QString moduleIconPath = modulesInfoArray[i].split(',')[1];
		QString moduleType = modulesInfoArray[i].split(',')[2];

		QuestionModule* new_module = new QuestionModule(moduleName, moduleIconPath);
		new_module->setFixedSize(QSize(290,108));
		m_moduleVector.push_back(new_module);

		QPushButton* btn = new_module->GetReturnBtn();

		connect(btn, &QPushButton::clicked, this, [=]()
		{
			int type = moduleType.toInt();
			SYAdminQuestionsMgrWindow* questionMgrWin = new SYAdminQuestionsMgrWindow(type, this);
			questionMgrWin->showFullScreen();
			questionMgrWin->exec();

		});

		QPushButton*delBtn = new_module->GetDelBtn();
		delBtn->setProperty("type", moduleType.toInt());

		connect(delBtn, &QPushButton::clicked, this, [=]()
		{
			QStringList modulesInfoArray = ConfigFileOperation::getInstance()->readFile();
			delete_questionModule(delBtn, modulesInfoArray);
		});
	}
	QString moduleName = QString::fromLocal8Bit("���ģ��");
	QuestionModule* addModule = new QuestionModule(moduleName, "", false);
	addModule->setFixedSize(QSize(290, 108));
	//addModule->SetText(QString::fromLocal8Bit("���ģ��"));
	QPushButton*delBtn = addModule->GetDelBtn();
	delBtn->setProperty("type", -1);
	addModule->SetAddModule();
	m_moduleVector.push_back(addModule);

	QPushButton* addBtn = addModule->GetReturnAddBtn();

	connect(addBtn, &QPushButton::clicked, this, [=]()
	{
		SYAddModuleWindow*addModuleWin = new SYAddModuleWindow(this);

		connect(addModuleWin, &SYAddModuleWindow::on_confirm_Btn_Clicked, this, &SYAdminTheoryTestWindow::do_addModule);
		addModuleWin->showFullScreen();
		bool flag = addModuleWin->exec();
		if (flag)
		{
			SetModulePage();//ˢ��
		}
	});

	QLayout* layout = ui.moduleWidget->layout();

	for (std::size_t i = 0; i < m_oldModuleVector.size(); i++)
	{
		m_oldModuleVector[i]->setParent(NULL);
		delete m_oldModuleVector[i];
	}
	delete layout;

	QVBoxLayout* vLayout = new QVBoxLayout();

	QGridLayout* moduleGridLayout = new QGridLayout();

	int index = 0;
	for (std::size_t i = 0; i < 3; i++)
	{
		for (std::size_t j = 0; j < 4; j++)
		{
			if (index >= m_moduleVector.size())
				break;
			moduleGridLayout->addWidget(m_moduleVector[index], i, j);
			index++;
		}
	}


	moduleGridLayout->setHorizontalSpacing(80);
	moduleGridLayout->setVerticalSpacing(60);
	moduleGridLayout->setContentsMargins(0, 0, 0, 0);
	vLayout->addLayout(moduleGridLayout);
	vLayout->addItem(new QSpacerItem(1, QSizePolicy::Preferred));
	vLayout->setContentsMargins(10, 0, 0, 0);
	vLayout->setSpacing(0);
	ui.moduleWidget->setLayout(vLayout);

	m_oldModuleVector.clear();

	for (std::size_t i = 0; i < m_moduleVector.size(); i++)
	{
		m_oldModuleVector.push_back(m_moduleVector[i]);
	}


}

void SYAdminTheoryTestWindow::do_addModule(QString iconPath, QString&moduleName)
{
	QString saveText = "";
	QStringList modulesInfoArray = ConfigFileOperation::getInstance()->readFile();
	
	QVector<int> typeVector;
	for (std::size_t i = 0; i < modulesInfoArray.size(); i++)
	{
		saveText += modulesInfoArray[i]+"#";
		int type = modulesInfoArray[i].split(',')[2].toInt();
		typeVector.push_back(type);
		QString t_moduleName = modulesInfoArray[i].split(',')[0];
		if (t_moduleName == moduleName)
		{
			SYMessageBox* box = new SYMessageBox(this, "", CHS("��ģ���Ѵ��ڣ�"), 1);
			box->showFullScreen();
			box->exec();
			return;
		}
	}
	if (modulesInfoArray.size() + 1 >= 12)
	{
		SYMessageBox* box = new SYMessageBox(this, "", CHS("����ӵ�ģ���Ѿ���!"), 1);
		box->showFullScreen();
		box->exec();
		return;

	}

	int type;
	while (true)
	{
		type = rand() % 100;
		if (!typeVector.contains(type))
		{
			break;
		}
	}

	QuestionModule* newModule = new QuestionModule(moduleName, iconPath);

	newModule->setFixedSize(QSize(290, 108));

	//���������Ŀ����ӳ��
	QString newAddModuleInfo = moduleName + QString(",") + iconPath + QString(",") + QString::number(type) + QString("#");
	saveText += newAddModuleInfo;
	ConfigFileOperation::getInstance()->saveFile(saveText);
	
	QPushButton*delBtn = newModule->GetDelBtn();
	delBtn->setProperty("type", type);
	connect(delBtn, &QPushButton::clicked, this, [=]()
	{
		QStringList modulesInfoArray = ConfigFileOperation::getInstance()->readFile();
		delete_questionModule(delBtn, modulesInfoArray);
	});	
}

void SYAdminTheoryTestWindow::delete_questionModule(QPushButton*delBtn, QStringList modulesInfoArray)
{

	int delType = delBtn->property("type").toInt();

	auto GetSaveText = [=](QString& saveContent, QStringList &modulesInfoArray)
	{
		for (std::size_t i = 0; i < modulesInfoArray.size(); i++)
		{
			int moduleType = modulesInfoArray[i].split(',')[2].toInt();
			if (delType == moduleType)
			{
				continue;
			}
			saveContent += modulesInfoArray[i] + QString("#");
		}
	};


	QVector<QSqlRecord> records;
	bool ok = SYDBMgr::Instance()->QueryPaperInfo(delType, records);

	if (ok)
	{
		if (records.size() > 0)
		{

			SYMessageBox* box = new SYMessageBox(this, "", CHS("�Ƿ���ģ������Ŀһͬɾ����"), 2);
			box->showFullScreen();
			int ret = box->exec();

			if (ret == 2)
			{
				QString saveContent = "";
				GetSaveText(saveContent, modulesInfoArray);
				SYDBMgr::Instance()->DeleteQuestions(delType);//��������ɾ����Ŀ
				ConfigFileOperation::getInstance()->saveFile(saveContent);
			}

		}
		else
		{

			SYMessageBox* box = new SYMessageBox(this, "", CHS("�Ƿ�ɾ����ģ�飿"), 2);
			box->showFullScreen();
			int ret = box->exec();
			if (ret == 2)
			{
				QString saveContent = "";
				GetSaveText(saveContent, modulesInfoArray);
				ConfigFileOperation::getInstance()->saveFile(saveContent);
			}
		}

	}


	SetModulePage();
}


void SYAdminTheoryTestWindow::on_newBtn_clicked()
{

	SYAdminNewPaperWindow* newPaperWin = new SYAdminNewPaperWindow("",this);
	newPaperWin->showFullScreen();
	newPaperWin->exec();
	RefreshTable();
}

void SYAdminTheoryTestWindow::on_upLoadBtn_clicked()
{

	SYAdminUpLoadPaperWindow* uploadWin = new SYAdminUpLoadPaperWindow(this);
	uploadWin->showFullScreen();
	uploadWin->exec();

}

void SYAdminTheoryTestWindow::onDeleteRecord()
{
	QPushButton* button = static_cast<QPushButton*>(sender());
	bool ok = false;
	int userId = button->property("userId").toInt(&ok);

	if (ok){
		SYMessageBox* messageBox = new SYMessageBox(this, "",SYStringTable::GetString(SYStringTable::STR_ConfirmRomovePaper), 2);
		messageBox->setPicture("Shared/personnel_pic_.png");
		messageBox->showFullScreen();
		if (messageBox->exec() == 2){
		
			QTableWidget* table = m_paperSetTable;
			QString paperName;
			for (int i = 0; i < table->rowCount(); ++i)
			{
				QTableWidgetItem* item = table->item(i, 0);
				if (item)
				{
					int id = item->data(Qt::UserRole).toInt(&ok);
					if (ok && id == userId){
						paperName = item->text();
						break;
					}
				}
			}
			SYDBMgr::Instance()->DeletePaperInfo(paperName);
		}
	}

	RefreshTable();  //��ʾ����
}

void SYAdminTheoryTestWindow::onShowPaperBtn_clicked()  //�鿴�Ծ�
{
	QPushButton* Btn = static_cast<QPushButton*>(sender());   //ȷ�����а�������
	QString paperName;
	int userId = Btn->property("userId").toInt();
	QTableWidget *widget = m_paperSetTable;
	for (std::size_t i = 0; i < widget->rowCount(); i++)
	{
		QTableWidgetItem* item = widget->item(i, 0);
		if (item->data(Qt::UserRole) == userId)
		{
			paperName = item->text();
			break;
		}
	}

	SYAdminShowPaperDetailWindow* paperWin = new SYAdminShowPaperDetailWindow(paperName,this);
	paperWin->showFullScreen();
	paperWin->exec();
	RefreshTable();
}


void SYAdminTheoryTestWindow::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();
	if (key == Qt::Key_Enter || key == Qt::Key_Return)
		RefreshTable();
}

bool  SYAdminTheoryTestWindow::FilterRecord(QSqlRecord record)
{
	QString text = ui.searchLineEdit->text();

	if (text.size() == 0)
		return true;

	if (record.value("paperName").toString().contains(text, Qt::CaseInsensitive))
		return true;
	if (record.value("paperRank").toString().contains(text, Qt::CaseInsensitive))
		return true;
	if (record.value("questionNum").toString().contains(text, Qt::CaseInsensitive))
		return true;
	if (record.value("createDatetime").toDateTime().toString(SYStringTable::GetString(SYStringTable::STR_DATETIMEFORMAT)).contains(text, Qt::CaseInsensitive))
		return true;
	if (record.value("creator").toString().contains(text, Qt::CaseInsensitive))
		return true;

	if (text == "0")
		return true;
	return false;
}

void SYAdminTheoryTestWindow::showEvent(QShowEvent* event)
{
	if (m_curSelectedBtn)
		m_curSelectedBtn->setChecked(true);

	ui.leftMenuWindow->setCurSelectedItem(WT_AdminTheoryTestWindow);   //
}

void SYAdminTheoryTestWindow::onBackToWidget(QWidget*widget)
{
	if (this == widget)
	{
		ui.searchLineEdit->clear();
		RefreshTable();  //��ʾ����
	}
}



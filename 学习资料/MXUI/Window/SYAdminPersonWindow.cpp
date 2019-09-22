#include "SYAdminPersonWindow.h"
#include "MxDefine.h"
#include <SYDBMgr.h>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <initializer_list>
#include <SYUserInfo.h>
#include <SYMessageBox.h>
#include "SYAddUserWindow.h"
#include "SYImportUserWindow.h"
#include "SYAdminGroupMgrWindow.h"
#include "SYMessageSendWindow.h"
#include "SYProcessingDlg.h"
#include "SYMainWindow.h"

SYAdminPersonWindow::SYAdminPersonWindow(QWidget* parent)
	:QWidget(parent),
	m_curSelectedBtn(nullptr),
	m_teacherTable(nullptr),
	m_studentTable(nullptr),
	m_groupWindow(nullptr),
	m_addUserWindow(nullptr),
	m_importUserWindow(nullptr),
	m_messageSendWindow(nullptr),
	m_processingDlg(nullptr)
{
	ui.setupUi(this);

	m_curSelectedBtn = ui.teacherMgrBtn;
	ui.sendMessageBtn->setVisible(false);

	connect(ui.leftMenuWindow, &SYMenuWindow::showNextWindow, this, &SYAdminPersonWindow::showNextWindow);

	InitTables();

	LoadData();

	SetTableContent();

	Mx::setWidgetStyle(this, "qss:SYAdminPersonWindow.qss");
}

SYAdminPersonWindow::~SYAdminPersonWindow()
{

}

void SYAdminPersonWindow::InitTables()
{
	auto InitImpl = [](QTableWidget* table,int columnCount,std::initializer_list<QString> horizontalLabels,std::vector<int> sectionWidths){
		table->setColumnCount(columnCount);

		QStringList labels;
		for(const auto& lb : horizontalLabels)
			labels.append(lb);
		table->setHorizontalHeaderLabels(labels);

		table->setFrameShape(QFrame::NoFrame);
		table->setShowGrid(false);
		table->setSelectionMode(QAbstractItemView::SingleSelection);
		table->setSelectionBehavior(QAbstractItemView::SelectRows);
		table->setAlternatingRowColors(true);
		table->setFocusPolicy(Qt::NoFocus);

		QHeaderView* headerView = table->horizontalHeader();
		headerView->setStretchLastSection(true);
		headerView->setHighlightSections(false);
		headerView->setObjectName("horizontalHeader");
		//headerView->setSectionsMovable(false);
		headerView->setEnabled(false);

		//set section width
		for(std::size_t i = 0; i < sectionWidths.size(); ++i){
			table->setColumnWidth(i, sectionWidths[i]);
		}

		headerView = table->verticalHeader();
		headerView->hide();
	};
	QStringList headerLabels;

	//教师表
	m_teacherTable = new QTableWidget;

	InitImpl(m_teacherTable,
			 6,
			 {QString::fromLocal8Bit("教师姓名"), QString::fromLocal8Bit("标签"), QString::fromLocal8Bit("首次登录日期"), QString::fromLocal8Bit("上次登录日期"), QString::fromLocal8Bit("登录次数"), QString::fromLocal8Bit("操作")},
			 {212, 300, 217, 297, 280, 75});
	ui.stackedWidget->addWidget(m_teacherTable);

	//学生表
	m_studentTable = new QTableWidget;
	InitImpl(m_studentTable,
			 7,
			 {QString::fromLocal8Bit("学员姓名"),QString::fromLocal8Bit("学员账号"),QString::fromLocal8Bit("班级分类"),QString::fromLocal8Bit("分组"),QString::fromLocal8Bit("任务分配"),QString::fromLocal8Bit("课程分配"),QString::fromLocal8Bit("操作")},
			 {212, 270, 214, 214, 110, 252,100});
	ui.stackedWidget->addWidget(m_studentTable);

	//小组表
	m_groupWindow = new SYAdminGroudpMgrWindow();
	ui.stackedWidget->addWidget(m_groupWindow);
}

void SYAdminPersonWindow::LoadData()
{
	m_allUserInfos.clear();
	m_teacherInfoIndexPairs.clear();
	m_studentInfoIndexPairs.clear();

	//load user info
	SYDBMgr::Instance()->QueryAllUserInfo(m_allUserInfos);

	std::pair<int,bool> tempPair;
	for(std::size_t i = 0; i < m_allUserInfos.size();++i){
		QSqlRecord& record = m_allUserInfos[i];
		int permission = record.value("permission").toInt();

		//访客
		if(permission == 0)
			continue;

		//学生、教师
		tempPair.first = i;
		tempPair.second = true;
		if(permission == 1)
			m_studentInfoIndexPairs.push_back(tempPair);
		else if(permission == 2)
			m_teacherInfoIndexPairs.push_back(tempPair);
	}
}

void SYAdminPersonWindow::SetTableContent()
{
	SetTeacherTableContent();

	SetStudentTableContent();
}

void SYAdminPersonWindow::SetTeacherTableContent()
{
	const int rowHeight = 70;
	const QString dateTimeFormat = QString("yyyy%1MM%2dd%3").arg(QString::fromLocal8Bit("年")).arg(QString::fromLocal8Bit("月")).arg(QString::fromLocal8Bit("日"));
	auto SetItemAttribute = [](QTableWidgetItem* item, const QString& text, Qt::Alignment align){
		item->setText(text);
		item->setTextAlignment(align);
		item->setTextColor(QColor(0xb7, 0xc5, 0xd8));

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);
	};

	bool canOperate = true;// SYUserInfo::Instance()->IsSuperManager();

	//教师表
	if(m_teacherTable){
		int nTeacherRecord = m_teacherInfoIndexPairs.size();
		m_teacherTable->setRowCount(nTeacherRecord);

		QString name;
		QString firstTime, lastTime;
		int row = 0;
		for(int i = 0; i < nTeacherRecord; ++i){
			auto& indexPair = m_teacherInfoIndexPairs[i];
			if(indexPair.second == false)
				continue;

			int index = indexPair.first;
			const auto& record = m_allUserInfos[index];
			int userId = record.value("id").toInt();

			//col 0
			QTableWidgetItem* item = new QTableWidgetItem;
			item->setData(Qt::UserRole, userId);		//删除时，便于删除这一行的数据
			name = record.value("realName").toString();
			if(name.size() == 0)
				name.push_back("-");
			SetItemAttribute(item, name, Qt::AlignHCenter | Qt::AlignVCenter);
			m_teacherTable->setItem(row, 0, item);

			//col 1
			item = new QTableWidgetItem;
			//item->setData(Qt::UserRole, i);			//第一列存储索引，用于筛选功能
			SetItemAttribute(item, "-", Qt::AlignHCenter | Qt::AlignVCenter);
			m_teacherTable->setItem(row, 1, item);

			//col 2
			item = new QTableWidgetItem;
			firstTime = record.value("firstLoginTime").toDateTime().toString(dateTimeFormat);
			if(firstTime.size() == 0)
				firstTime.push_back("-");
			SetItemAttribute(item, firstTime, Qt::AlignHCenter | Qt::AlignVCenter);
			m_teacherTable->setItem(row, 2, item);

			//col 3
			item = new QTableWidgetItem;
			lastTime = record.value("lastLoginTime").toDateTime().toString(dateTimeFormat);
			if(lastTime.size() == 0)
				lastTime.push_back("-");
			SetItemAttribute(item, lastTime, Qt::AlignHCenter | Qt::AlignVCenter);
			m_teacherTable->setItem(row, 3, item);

			//col 4
			item = new QTableWidgetItem;
			SetItemAttribute(item, record.value("loginTimes").toString(), Qt::AlignHCenter | Qt::AlignVCenter);
			m_teacherTable->setItem(row, 4, item);

			//col 5
			QWidget* widget = new QWidget;
			QHBoxLayout* hLayout = new QHBoxLayout;

			QPushButton* deleteBtn = new QPushButton();
			deleteBtn->setText(QString::fromLocal8Bit("删除"));
			deleteBtn->setObjectName("deleteBtn");
			deleteBtn->setEnabled(canOperate);
			//deleteBtn->setProperty("index", index);
			deleteBtn->setProperty("userId", userId);
			deleteBtn->setProperty("isTeacher", 1);
			connect(deleteBtn, &QPushButton::clicked, this, &SYAdminPersonWindow::onDeleteRecord);


			QPushButton* resetPasswordBtn = new QPushButton();
			resetPasswordBtn->setText(QString::fromLocal8Bit("重置密码"));
			resetPasswordBtn->setObjectName("resetPasswordBtn");
			resetPasswordBtn->setEnabled(canOperate);
			//resetPasswordBtn->setProperty("index", index);
			resetPasswordBtn->setProperty("userId", userId);
			connect(resetPasswordBtn, &QPushButton::clicked, this, &SYAdminPersonWindow::onResetPassword);

			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(deleteBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(resetPasswordBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			widget->setLayout(hLayout);

			m_teacherTable->setCellWidget(row, 5, widget);

			m_teacherTable->setRowHeight(row, rowHeight);

			row++;
		}

		m_teacherTable->setRowCount(row);
	}

}

void SYAdminPersonWindow::SetStudentTableContent()
{
	const int rowHeight = 70;
	const QString dateTimeFormat = QString("yyyy%1MM%2dd%3").arg(QString::fromLocal8Bit("年")).arg(QString::fromLocal8Bit("月")).arg(QString::fromLocal8Bit("日"));
	auto SetItemAttribute = [](QTableWidgetItem* item, const QString& text, Qt::Alignment align){
		item->setText(text);
		item->setTextAlignment(align);
		item->setTextColor(QColor(0xb7, 0xc5, 0xd8));

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);
	};

	bool canOperate = true;// SYUserInfo::Instance()->IsSuperManager();

	//学生表
	if(m_studentTable){
		int nStudentRecord = m_studentInfoIndexPairs.size();
		m_studentTable->setRowCount(nStudentRecord);

		QString groupName;
		int row = 0;
		for(int i = 0; i < nStudentRecord; ++i){
			auto& indexPair = m_studentInfoIndexPairs[i];
			if(indexPair.second == false)
				continue;

			int index = indexPair.first;
			const auto& record = m_allUserInfos[index];
			int userId = record.value("id").toInt();

			//col 0
			QTableWidgetItem* item = new QTableWidgetItem;
			item->setData(Qt::UserRole, userId);		//删除时，便于删除这一行的数据
			SetItemAttribute(item, record.value("realName").toString(), Qt::AlignHCenter | Qt::AlignVCenter);
			m_studentTable->setItem(row, 0, item);

			//col 1
			item = new QTableWidgetItem;
			//item->setData(Qt::UserRole, i);				//第一列存储索引，用于筛选功能
			SetItemAttribute(item, record.value("userName").toString(), Qt::AlignHCenter | Qt::AlignVCenter);
			m_studentTable->setItem(row, 1, item);

			//col 2
			item = new QTableWidgetItem;
			SetItemAttribute(item, "-", Qt::AlignHCenter | Qt::AlignVCenter);
			m_studentTable->setItem(row, 2, item);

			//col 3
			item = new QTableWidgetItem;
			groupName = record.value("groupname").toString();
			if(groupName.size() == 0)
				groupName.push_back("-");
			SetItemAttribute(item, groupName, Qt::AlignHCenter | Qt::AlignVCenter);
			m_studentTable->setItem(row, 3, item);

			//col 4
			item = new QTableWidgetItem;
			SetItemAttribute(item, "0", Qt::AlignHCenter | Qt::AlignVCenter);
			m_studentTable->setItem(row, 4, item);

			//col 5
			item = new QTableWidgetItem;
			SetItemAttribute(item, "0", Qt::AlignHCenter | Qt::AlignVCenter);
			m_studentTable->setItem(row, 5, item);

			//col 6
			QWidget* widget = new QWidget;
			QHBoxLayout* hLayout = new QHBoxLayout;

			QPushButton* deleteBtn = new QPushButton();
			deleteBtn->setText(QString::fromLocal8Bit("删除"));
			deleteBtn->setObjectName("deleteBtn");
			deleteBtn->setEnabled(canOperate);
			//deleteBtn->setProperty("index", index);
			deleteBtn->setProperty("userId", userId);
			deleteBtn->setProperty("isTeacher", 0);
			connect(deleteBtn, &QPushButton::clicked, this, &SYAdminPersonWindow::onDeleteRecord);


			QPushButton* resetPasswordBtn = new QPushButton();
			resetPasswordBtn->setText(QString::fromLocal8Bit("重置密码"));
			resetPasswordBtn->setObjectName("resetPasswordBtn");
			resetPasswordBtn->setEnabled(canOperate);
			//resetPasswordBtn->setProperty("index", index);
			resetPasswordBtn->setProperty("userId", userId);
			connect(resetPasswordBtn, &QPushButton::clicked, this, &SYAdminPersonWindow::onResetPassword);

			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(deleteBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			hLayout->addWidget(resetPasswordBtn);
			hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			widget->setLayout(hLayout);

			m_studentTable->setCellWidget(row, 6, widget);

			m_studentTable->setRowHeight(row, rowHeight);

			++row;
		}

		m_studentTable->setRowCount(row);
	}
}

void SYAdminPersonWindow::ResetTableRecordShowStatus()
{
	for(auto& indexPair : m_teacherInfoIndexPairs)
		indexPair.second = true;

	for(auto& indexPair : m_studentInfoIndexPairs)
		indexPair.second = true;
}

void SYAdminPersonWindow::RefreshTable()
{
	//reload
	LoadData();
	SetTableContent();
}

void SYAdminPersonWindow::on_teacherMgrBtn_clicked()
{
	ui.teacherMgrBtn->setChecked(true);

	if(m_curSelectedBtn == ui.teacherMgrBtn)
		return;

	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = ui.teacherMgrBtn;
	ui.stackedWidget->setCurrentIndex(0);

	//
	ui.newBtn->setVisible(true);
	ui.importBtn->setVisible(true);
	ui.sendMessageBtn->setVisible(false);
	ui.searchLineEdit->clear();

	ResetTableRecordShowStatus();
	SetTableContent();
}

void SYAdminPersonWindow::on_studentMgrBtn_clicked()
{
	ui.studentMgrBtn->setChecked(true);

	if(m_curSelectedBtn == ui.studentMgrBtn)
		return;

	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = ui.studentMgrBtn;
	ui.stackedWidget->setCurrentIndex(1);

	//
	ui.newBtn->setVisible(true);
	ui.importBtn->setVisible(true);
	ui.sendMessageBtn->setVisible(true);
	ui.searchLineEdit->clear();

	ResetTableRecordShowStatus();
	SetTableContent();
}

void SYAdminPersonWindow::on_groupMgrBtn_clicked()
{
	ui.groupMgrBtn->setChecked(true);

	if(m_curSelectedBtn == ui.groupMgrBtn)
		return;

	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = ui.groupMgrBtn;
	ui.stackedWidget->setCurrentIndex(2);

	//
	ui.newBtn->setVisible(true);
	ui.importBtn->setVisible(false);
	ui.sendMessageBtn->setVisible(false);
	ui.searchLineEdit->clear();

	//ResetTableRecordShowStatus();
	//SetTableContent();
	//m_groupWindow->FilterRecord("");
	m_groupWindow->SetFilterText("");
	m_groupWindow->RefreshGroupList();
}

void SYAdminPersonWindow::on_searchBtn_clicked()
{
	const QString dateTimeFormat = QString("yyyy%1MM%2dd%3").arg(QString::fromLocal8Bit("年")).arg(QString::fromLocal8Bit("月")).arg(QString::fromLocal8Bit("日"));

	QString text = ui.searchLineEdit->text();
	
	if(m_curSelectedBtn == ui.groupMgrBtn){
		m_groupWindow->SetFilterText(text);
		m_groupWindow->RefreshGroupList();
	}
	else{

		if(text.size() == 0){
			RefreshTable();
			return;
		}
		
		if(m_curSelectedBtn == ui.teacherMgrBtn){
			QString name;
			QString firstTime, lastTime;

			for(auto& indexPair : m_teacherInfoIndexPairs){
				indexPair.second = false;
				int recordIndex = indexPair.first;
				auto& record = m_allUserInfos[recordIndex];

				// col 0
				name = record.value("realName").toString();
				if(name.size() == 0)
					name.push_back("-");
				if(name.contains(text, Qt::CaseInsensitive)){
					indexPair.second = true;
					continue;
				}

				//col 1
				if(text == "-"){
					indexPair.second = true;
					continue;
				}

				//col 2
				firstTime = record.value("firstLoginTime").toDateTime().toString(dateTimeFormat);
				if(firstTime.size() == 0)
					firstTime.push_back("-");

				if(firstTime.contains(text,Qt::CaseInsensitive)){
					indexPair.second = true;
					continue;
				}

				//col 3
				lastTime = record.value("lastLoginTime").toDateTime().toString(dateTimeFormat);
				if(lastTime.size() == 0)
					lastTime.push_back("-");

				if(lastTime.contains(text, Qt::CaseInsensitive)){
					indexPair.second = true;
					continue;
				}

				//col 4
				if(record.value("loginTimes").toString().contains(text)){
					indexPair.second = true;
					continue;
				}
			}

			SetTeacherTableContent();
		}
		else if(m_curSelectedBtn == ui.studentMgrBtn){
			QString groupName;
			for(auto& indexPair : m_studentInfoIndexPairs){
				indexPair.second = false;
				int recordIndex = indexPair.first;
				auto& record = m_allUserInfos[recordIndex];

				//col 0
				if(record.value("realName").toString().contains(text, Qt::CaseInsensitive)){
					indexPair.second = true;
					continue;
				}

				//col 1
				if(record.value("userName").toString().contains(text,Qt::CaseInsensitive)){
					indexPair.second = true;
					continue;
				}

				//col 2
				if(text == "-"){
					indexPair.second = true;
					continue;
				}

				//col 3
				groupName = record.value("groupname").toString();
				if(groupName.size() == 0)
					groupName.push_back("-");
				if(groupName.contains(text, Qt::CaseInsensitive)){
					indexPair.second = true;
					continue;
				}

				//col 4 & col 5
				if(text == "0"){
					indexPair.second = true;
					continue;
				}
			}

			SetStudentTableContent();
		}
	}
}

void SYAdminPersonWindow::on_newBtn_clicked()
{
	if(m_curSelectedBtn == ui.groupMgrBtn){
		m_groupWindow->on_bt_creategroup_clicked();
	}
	else{
		if(m_addUserWindow == nullptr)
			m_addUserWindow = new SYAddUserWindow(this);

		if(m_curSelectedBtn == ui.teacherMgrBtn)
			m_addUserWindow->setUserPermission(UP_Teacher);
		else
			m_addUserWindow->setUserPermission(UP_Student);

		m_addUserWindow->showFullScreen();
		if(m_addUserWindow->exec() == RbShieldLayer::RC_Ok){
			//刷新表的内容
			RefreshTable();
		}
	}
}

void SYAdminPersonWindow::on_importBtn_clicked()
{
	if(m_importUserWindow == nullptr)
		m_importUserWindow = new SYImportUserWindow(this);

	if(m_curSelectedBtn == ui.teacherMgrBtn)
		m_importUserWindow->setUserPermission(UP_Teacher);
	else
		m_importUserWindow->setUserPermission(UP_Student);

	m_importUserWindow->showFullScreen();
	if(m_importUserWindow->exec() == RbShieldLayer::RC_Ok){
		RefreshTable();
	}
}

void SYAdminPersonWindow::on_sendMessageBtn_clicked()
{
	if(m_messageSendWindow == nullptr)
		m_messageSendWindow = new SYMessageSendWindow(this);

	m_messageSendWindow->showFullScreen();

	if(m_messageSendWindow->exec() == RbShieldLayer::RC_Ok){
		if(m_processingDlg == nullptr){
			m_processingDlg = new SYProcessingDlg(SYMainWindow::GetInstance());
			m_processingDlg->setAttribute(Qt::WA_DeleteOnClose,false);
		}

		m_processingDlg->SetAutoProcess(2000);
		m_processingDlg->showFullScreen();
	}
}

void SYAdminPersonWindow::onDeleteRecord()
{
	QPushButton* button = static_cast<QPushButton*>(sender());
	bool ok = false;
	int userId = button->property("userId").toInt(&ok);

	if(ok){
		SYMessageBox* messageBox = new SYMessageBox(this, "", QString::fromLocal8Bit("是否删除该用户"), 2);
		messageBox->setPicture("Shared/personnel_pic_.png");
		messageBox->showFullScreen();
		if(messageBox->exec() == 2){
			//delete data from database
			SYDBMgr::Instance()->DeleteUserInfo(userId);

			bool isTeacher = button->property("isTeacher").toBool();
			QTableWidget* table = m_teacherTable;
			if(!isTeacher)
				table = m_studentTable;

			//delete data from tableWidget
			for(int i = 0; i < table->rowCount(); ++i){
				QTableWidgetItem* item = table->item(i, 0);
				if(item){
					int id = item->data(Qt::UserRole).toInt(&ok);
					if(ok && id == userId){
						table->removeRow(i);
						break;
					}
				}
			}
		}
	}
}

void SYAdminPersonWindow::onResetPassword()
{
	QPushButton* button = static_cast<QPushButton*>(sender());
	bool ok = false;
	int userId = button->property("userId").toInt(&ok);

	if(ok){
		SYMessageBox* messageBox = new SYMessageBox(this, "", QString::fromLocal8Bit("是否重置为初始密码"), 2);
		messageBox->setSubContent(QString::fromLocal8Bit("初始密码：123456"));
		messageBox->setPicture("Shared/password_pic_.png");
		messageBox->showFullScreen();
		if(messageBox->exec() == 2){
			SYDBMgr::Instance()->UpdateUserPassword(userId, "123456");
		}
	}
}


void SYAdminPersonWindow::showEvent(QShowEvent* event)
{
	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(true);

	ui.leftMenuWindow->setCurSelectedItem(WT_AdminPersonWindow);
}

void SYAdminPersonWindow::keyReleaseEvent(QKeyEvent* event)
{
	int key = event->key();
	if(key == Qt::Key_Enter || key == Qt::Key_Return)
		on_searchBtn_clicked();
}
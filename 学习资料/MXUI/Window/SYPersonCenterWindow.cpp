#include "SYPersonCenterWindow.h"
#include "SYDBMgr.h"
#include <QSqlRecord>
#include <vector>
#include "SYUserInfo.h"
#include "MxDefine.h"
#include "SYEditPersonPasswordWindow.h"
#include "SYMainWindow.h"
#include "SYModifyTrainTaskWindow.h"

SYPersonCenterWindow::SYPersonCenterWindow(QWidget *parent)
	: QWidget(parent),
	m_userId(0),
	m_editPasswordWindow(nullptr)
{
	ui.setupUi(this);

	ui.rankTableWidget->setColumnCount(4);		//第一列显示图标
	ui.rankTableWidget->setColumnWidth(0, 74);
	ui.rankTableWidget->setColumnWidth(1, 160);
	ui.rankTableWidget->setColumnWidth(2, 100);
	ui.rankTableWidget->setColumnWidth(3, 50);
	ui.rankTableWidget->setFrameShape(QFrame::NoFrame);
	ui.rankTableWidget->setShowGrid(false);
	ui.rankTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
	//ui.rankTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.rankTableWidget->setFocusPolicy(Qt::NoFocus);

	QHeaderView* headerView = ui.rankTableWidget->horizontalHeader();
	headerView->setStretchLastSection(true);
	headerView->setFixedHeight(46);
	headerView->setObjectName("rankHeaderView");
	headerView->setEnabled(false);
// 	QFont font = headerView->font();
// 	font.setPixelSize(16);
// 	headerView->setFont(font);

	ui.rankTableWidget->verticalHeader()->hide();

	QStringList headerInfos;
	headerInfos << ""
		<< QString::fromLocal8Bit("榜上有名")
		<< QString::fromLocal8Bit("上榜分数")
		<< QString::fromLocal8Bit("训练次数");
	ui.rankTableWidget->setHorizontalHeaderLabels(headerInfos);



	SYUserInfo::Instance()->Refresh();

	m_userId = SYUserInfo::Instance()->GetUserId();

	//1 设置个人信息
	const QString& userName = SYUserInfo::Instance()->GetUserName();
	const QString& realName = SYUserInfo::Instance()->GetRealName();

	if(realName.size())
		ui.nameLabel->setText(realName);
	else
		ui.nameLabel->setText(userName);

	//访客无法编辑个人信息
	if(SYUserInfo::Instance()->IsVisitor())
		ui.editBtn->setVisible(false);

	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();
	ui.headLabel->setPixmap(headIcon.pixmap(108, 108));

	//2 设置班级信息与小组信息
	const QString& className = SYUserInfo::Instance()->GetClassName();
	if(className.size() > 0)
		ui.classLabel->setText(className);
	else
		ui.classLabel->setText("-");

	const QString& groupName = SYUserInfo::Instance()->GetGroupName();
	if(groupName.size() > 0)
		ui.groupLabel->setText(groupName);
	else
		ui.groupLabel->setText("-");

	const QString& teacherName = SYUserInfo::Instance()->GetTeacherName();
	if(teacherName.size() > 0)
		ui.teacherLabel->setText(teacherName);
	else
		ui.teacherLabel->setText("-");

	//3 设置登录时间及在线时长
	//content = QString::fromLocal8Bit("首次登陆  ") + SYUserInfo::Instance()->GetFirstLoginTime().toString("yyyy/MM/dd HH:mm:ss");
	//ui.firstLoginTimeLabel->setText(content);

	//content = QString::fromLocal8Bit("最近一次登陆  ") + SYUserInfo::Instance()->GetLastLoginTime().toString("yyyy/MM/dd HH:mm:ss");
	//ui.lastLoginTimeLabel->setText(content);

	int h, m;
	std::size_t totalOnlineTime = SYUserInfo::Instance()->GetTotalOnlineTime();
	h = totalOnlineTime / 3600;
	m = (totalOnlineTime - h * 3600) / 60;
	//content = QString::fromLocal8Bit("累计在线时间%1时%2分").arg(h).arg(m);
	//ui.totalOnlineTimeLabel->setText(content);
	QString content;
	content.setNum(h);
	ui.hourLabel11->setText(content);
	content.setNum(m);
	ui.minuteLabel11->setText(content);

	std::size_t totalTrainTime = SYUserInfo::Instance()->GetTotalTrainTime();
	h = totalTrainTime / 3600;
	m = (totalTrainTime - h * 3600) / 60;
	//content = QString::fromLocal8Bit("累计训练时间%1时%2分").arg(h).arg(m);
	//ui.totalTrainTimeLabel->setText(content);
	content.setNum(h);
	ui.hourLabel21->setText(content);
	content.setNum(m);
	ui.minuteLabel21->setText(content);

	setTheoryTestInfo();

	setSkillingTrainInfo();

	setTrainTimesInfo();

	setRankInfo();

	setCourseAndTaskInfo();

	refreshMyTasks();

	Mx::setWidgetStyle(this,"qss:SYPersonCenterWindow.qss");

	connect(ui.courseTableWidget2, &QTableWidget::cellClicked, this, &SYPersonCenterWindow::onCourseListTableItemClicked);

}

SYPersonCenterWindow::~SYPersonCenterWindow(void)
{
	if(m_editPasswordWindow)
		delete m_editPasswordWindow;
}

void SYPersonCenterWindow::setTheoryTestInfo()
{
	QString textTimes("-");
	QString textAvgScore("-");
	QString textPassRate("-");
	
	int userId = SYUserInfo::Instance()->GetUserId();

	QSqlRecord resultRecord;

	int AvgScore = 0;
	int Count = 0;
	int PassCount = 0;

	bool succed0  = SYDBMgr::Instance()->QueryExamNumAndAvgScore(userId , resultRecord);
	if (succed0)
	{
		AvgScore = resultRecord.value("AvgScore").toInt();
		Count = resultRecord.value("Count").toInt();
		textTimes = QString::number(Count);

		if (Count > 0)
		    textAvgScore = QString::number(AvgScore);

		bool succed1 = SYDBMgr::Instance()->QueryExamPassNum(userId, 60, resultRecord);
		if (succed1 && Count > 0)
		{
			PassCount = resultRecord.value("PassCount").toInt();

			int PassRate = 100 * (Count > 0 ? (float)PassCount / (float)Count : 0.0f);
			textPassRate = QString::number(PassRate) + QString("%");
		}
	}
	

	ui.theoryTestTimesLabel->setText(textTimes);
	ui.theoryTestAvgScoreLabel->setText(textAvgScore);
	ui.theoryTestPassRateLabel->setText(textPassRate);
}

void SYPersonCenterWindow::setSkillingTrainInfo()
{
	const int numRm = 20;		//入门
	const int numSl = 20;		//熟练
	const int numJt = 20;		//精通

	int rmCount = 0;
	int slCount = 0;
	int jtCount = 0;
	std::vector<QSqlRecord> records;

	QString suffix;
	int userId = SYUserInfo::Instance()->GetUserId();
	SYDBMgr::Instance()->QueryAllScoreRecordOrderByDateDescend(userId,records);

	for(const auto& record : records){
		suffix = record.value("trainName").toString().right(1);
		if(suffix == QString::fromLocal8Bit("Ι"))
			++rmCount;
		else if(suffix == QString::fromLocal8Bit("Ⅱ"))
			++slCount;
		else if(suffix == QString::fromLocal8Bit("Ⅲ"))
			++jtCount;
	}

	//set text
	//QString content("<font color=#289fff>%1</font><font color=#85909e>/%2</font>");
	QString content("%1/%2");
	ui.rmLabel->setText(content.arg(rmCount).arg(numRm));
	ui.slLabel->setText(content.arg(slCount).arg(numSl));
	ui.jtLabel->setText(content.arg(jtCount).arg(numJt));
}

void SYPersonCenterWindow::on_editBtn_clicked()
{
	if(m_editPasswordWindow == nullptr)
		m_editPasswordWindow = new SYEditPersonPasswordWindow(SYMainWindow::GetInstance());

	m_editPasswordWindow->show();
	m_editPasswordWindow->exec();

	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();
	ui.headLabel->setPixmap(headIcon.pixmap(108, 108));
}

void SYPersonCenterWindow::on_messageBtn_clicked()
{
	emit showNextWindow(WT_MessageCenterWindow);
}

void SYPersonCenterWindow::on_addTaskBtn_clicked()
{
	SYModifyTrainTaskWindow * addTaskWindow = new SYModifyTrainTaskWindow(this);
	addTaskWindow->showFullScreen();
	int button = addTaskWindow->exec();
	if (button == 2)
	{
		refreshMyTasks();
	}
}
void SYPersonCenterWindow::onCourseListTableItemClicked(int row, int col)
{
	if (row >= 0 && row < m_allTasks.size())
	{
		SYTaskTrainDataMgr::GetInstance().SetSelectedTaskID(m_allTasks[row].m_ID);
		emit showNextWindow(WT_PersonTasksWindow);
	}
}
void SYPersonCenterWindow::setTrainTimesInfo()
{
	const int numLocalUser = SYDBMgr::Instance()->QueryNumberOfUser();
	const int numGlobalUser = numLocalUser;

	std::vector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryAllUserTrainTimesOrderByDescend(records);
	
	int rank = -1;
	for(std::size_t index = 0; index < records.size(); ++index){
		if(records[index].value("userId") == m_userId){
			rank = index + 1;
			break;
		}
	}

	if(rank == -1)
		rank = records.size() + 1;

	//set text
	//QString content("<font color=#289fff>%1</font><font color=#85909e>/%2</font>");
	QString content("%1/%2");
	ui.localTrainTimesLabel->setText(content.arg(rank).arg(numLocalUser));
	ui.globalTrainTimes->setText(content.arg(rank).arg(numGlobalUser));
}

void SYPersonCenterWindow::setRankInfo()
{
	const QColor itemColor = QColor(0x85, 0x90, 0x9e);
	auto SetItemAttribute = [](QTableWidgetItem* item){
		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);

		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		item->setTextColor(QColor(0x46, 0x4d, 0x5f));
	};

	ui.rankTableWidget->clearContents();
	std::vector<QSqlRecord> records;

	SYDBMgr::Instance()->QueryAvgScoreRankInfo(records);
	const std::size_t nRow = records.size();
	
	QString iconNames[3] = {"icons:/SYPersonCenterWindow/1.png", "icons:/SYPersonCenterWindow/2.png", "icons:/SYPersonCenterWindow/3.png"};
	QSqlRecord userInfoRecord, classInfoRecord;
	QString name;
	QTableWidgetItem* item = nullptr;

	int n = 5;
	ui.rankTableWidget->setRowCount(5);

	for(int i = 0; i < n; ++i)
		ui.rankTableWidget->setRowHeight(i, 46);

	if(nRow < n)
		n = nRow;

	for(std::size_t i = 0; i < n /*nRow*/; ++i){
		const QSqlRecord& record = records[i];

		//col 0
		if(i < 3){
			//item = new QTableWidgetItem(QIcon(iconNames[i]), "");
			//SetItemAttribute(item);
			//ui.rankTableWidget->setItem(i, 0, item);
			QWidget* widget = new QWidget;

			QHBoxLayout* layout = new QHBoxLayout;
			layout->setMargin(0);
			layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			QLabel* label = new QLabel;
			label->setPixmap(QPixmap(iconNames[i]));
			layout->addWidget(label);
			layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
			widget->setLayout(layout);

			ui.rankTableWidget->setCellWidget(i, 0, widget);
		}
		
		//col 1
		int userId = record.value("userId").toInt();
		userInfoRecord.clear();
		SYDBMgr::Instance()->QueryUserInfo(userId, userInfoRecord);

		int classId = userInfoRecord.value("classId").toInt();
		classInfoRecord.clear();
		SYDBMgr::Instance()->QueryClassInfo(classId, classInfoRecord);

		name = userInfoRecord.value("realName").toString();
		if(name.size() == 0)
			name = userInfoRecord.value("userName").toString();
		item = new QTableWidgetItem(classInfoRecord.value("name").toString() + " " + name);
		SetItemAttribute(item);
		ui.rankTableWidget->setItem(i, 1, item);

		//col 2
		int score = record.value("score").toInt();
		item = new QTableWidgetItem(QString().setNum(score));
		SetItemAttribute(item);
		ui.rankTableWidget->setItem(i, 2, item);
		
		//col 3
		int times = record.value("times").toInt();
		item = new QTableWidgetItem(QString().setNum(times));
		SetItemAttribute(item);
		ui.rankTableWidget->setItem(i, 3, item);

		//ui.rankTableWidget->setRowHeight(i,46);
	}
}

void SYPersonCenterWindow::setCourseAndTaskInfo()
{
	ui.courseTableWidget1->setRowCount(2);
	ui.courseTableWidget1->setColumnCount(4);

	ui.courseTableWidget1->setColumnWidth(0, 230);
	ui.courseTableWidget1->setColumnWidth(1, 183);
	ui.courseTableWidget1->setColumnWidth(2, 181);
	ui.courseTableWidget1->setFrameShape(QFrame::NoFrame);
	ui.courseTableWidget1->setShowGrid(false);
	ui.courseTableWidget1->setSelectionMode(QAbstractItemView::NoSelection);
	ui.courseTableWidget1->setFocusPolicy(Qt::NoFocus);

	QHeaderView* headerView = ui.courseTableWidget1->horizontalHeader();
	headerView->setStretchLastSection(true);
	headerView->setFixedHeight(65);
	headerView->setObjectName("courseTableHeaderView1");
	headerView->hide();
	ui.courseTableWidget1->verticalHeader()->hide();
	//headerView->setEnabled(false);

	QTableWidgetItem* item = nullptr;
	QString headerTexts[3] = {QString::fromLocal8Bit("已完成"), QString::fromLocal8Bit("进行中"), QString::fromLocal8Bit("已失效")};
	QString headerIconNames[3] = {"icons:/SYPersonCenterWindow/blue.png", "icons:/SYPersonCenterWindow/green.png", "icons:/SYPersonCenterWindow/yellow.png"};

	//set content
	for(int i = 0; i < 3; ++i){
		QWidget* widget = new QWidget;
		QHBoxLayout* layout = new QHBoxLayout;
		QIcon icon(headerIconNames[i]);
		QPushButton* button = new QPushButton(icon, headerTexts[i]);
		button->setObjectName("headerButton");

		layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
		layout->addWidget(button);
		layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
		widget->setLayout(layout);
		ui.courseTableWidget1->setCellWidget(0, i + 1,widget);

		ui.courseTableWidget1->setRowHeight(0, 65);
	}

	QString testContents1[4] = {QString::fromLocal8Bit("任务"), "5", "4", "3"};

	for(int col = 0; col < 4; ++col){
		item = new QTableWidgetItem(testContents1[col]);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);

		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		item->setTextColor(QColor(0x46, 0x4d, 0x5f));

		ui.courseTableWidget1->setItem(1, col, item);

		ui.courseTableWidget1->setRowHeight(1, 60);
	}

	/*
	QString testContents2[4] = {QString::fromLocal8Bit("任务"), "5", "4", "3"};
	for(int col = 0; col < 4; ++col){
		item = new QTableWidgetItem(testContents2[col]);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);

		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		item->setTextColor(QColor(0x46, 0x4d, 0x5f));

		ui.courseTableWidget1->setItem(2, col, item);

		ui.courseTableWidget1->setRowHeight(2, 60);
	}
	*/
}

void SYPersonCenterWindow::refreshMyTasks()
{
	//
	m_allTasks.clear();

	int userId = SYUserInfo::Instance()->GetUserId();
	
	SYTaskTrainDataMgr::GetInstance().PullAllTasksBelongsToReceiver(userId, m_allTasks);
	
	ui.courseTableWidget2->clear();
	ui.courseTableWidget2->setRowCount(m_allTasks.size());
	ui.courseTableWidget2->setColumnCount(5);

	ui.courseTableWidget2->setColumnWidth(0, 340);
	ui.courseTableWidget2->setColumnWidth(1, 280);
	ui.courseTableWidget2->setColumnWidth(2, 280);
	ui.courseTableWidget2->setColumnWidth(3, 280);
	ui.courseTableWidget2->setFrameShape(QFrame::NoFrame);
	ui.courseTableWidget2->setShowGrid(false);
	ui.courseTableWidget2->setSelectionMode(QAbstractItemView::NoSelection);
	ui.courseTableWidget2->setFocusPolicy(Qt::NoFocus);
	ui.courseTableWidget2->setAlternatingRowColors(true);

	//
	//ui.courseTableWidget2->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QHeaderView* headerView = ui.courseTableWidget2->horizontalHeader();
	headerView->setStretchLastSection(true);
	headerView->setFixedHeight(78);
	headerView->setObjectName("courseTableHeaderView2");
	//headerView->hide();
	ui.courseTableWidget2->verticalHeader()->hide();
	//headerView->setEnabled(false);

	QStringList headerLabels;
	headerLabels << QString::fromLocal8Bit("名称")
		<< QString::fromLocal8Bit("属性")
		<< QString::fromLocal8Bit("发送人")
		<< QString::fromLocal8Bit("发送时间")
		<< QString::fromLocal8Bit("完成情况");

	ui.courseTableWidget2->setHorizontalHeaderLabels(headerLabels);

	for (int row = 0; row < m_allTasks.size(); row++)
	{
		QString Contents[5];
		Contents[0] = m_allTasks[row].m_TaskName;
		Contents[1] = QString::fromLocal8Bit("任务计划");
		Contents[2] = m_allTasks[row].m_Assignor;
		Contents[3] = m_allTasks[row].m_SendDate.toString("yyyy-MM-dd");
		Contents[4] = QString::fromLocal8Bit("进行中");

		for (int col = 0; col < 5; col++)
		{
			QTableWidgetItem* item = new QTableWidgetItem(Contents[col]);
			Qt::ItemFlags flags = item->flags();
			flags = flags & (~Qt::ItemIsEditable);
			item->setFlags(flags);

			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

			QFont font = item->font();
			font.setPixelSize(16);

			if (col == 0)
			    font.setUnderline(true);
	
			item->setFont(font);

			if (col == 0)
				item->setTextColor(QColor(0x28, 0x9f, 0xff));
			else
				item->setTextColor(QColor(0xb7, 0xc5, 0xd8));
//
			ui.courseTableWidget2->setItem(row, col, item);

			ui.courseTableWidget2->setRowHeight(row, 70);
		}
	}

	ui.courseTableWidget1->item(1, 1)->setText(QString::number(0));
	ui.courseTableWidget1->item(1, 2)->setText(QString::number(m_allTasks.size()));
	ui.courseTableWidget1->item(1, 3)->setText(QString::number(0));
	//set content
	/*
	QString testContents1[5] = {QString::fromLocal8Bit("第一阶段训练"), QString::fromLocal8Bit("任务计划"), QString::fromLocal8Bit("张老师"), QString::fromLocal8Bit("2018-10-12"),QString::fromLocal8Bit("进行中")};

	for(int col = 0; col < 5; ++col){
		item = new QTableWidgetItem(testContents1[col]);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);

		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		item->setTextColor(QColor(0x46, 0x4d, 0x5f));

		ui.courseTableWidget2->setItem(0, col, item);

		ui.courseTableWidget2->setRowHeight(0, 70);
	}

	QString testContents2[5] = {QString::fromLocal8Bit("日常练习1"), QString::fromLocal8Bit("课程计划"), QString::fromLocal8Bit("高原"), QString::fromLocal8Bit("2018-9-12"), QString::fromLocal8Bit("未开始")};
	for(int col = 0; col < 5; ++col){
		item = new QTableWidgetItem(testContents2[col]);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);

		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		item->setTextColor(QColor(0x46, 0x4d, 0x5f));

		ui.courseTableWidget2->setItem(1, col, item);

		ui.courseTableWidget2->setRowHeight(1, 70);
	}

	QString testContents3[5] = {QString::fromLocal8Bit("日常练习2"), QString::fromLocal8Bit("课程计划"), QString::fromLocal8Bit("高原"), QString::fromLocal8Bit("2018-8-9"), QString::fromLocal8Bit("已完成")};
	for(int col = 0; col < 5; ++col){
		item = new QTableWidgetItem(testContents3[col]);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);

		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		item->setTextColor(QColor(0x46, 0x4d, 0x5f));

		ui.courseTableWidget2->setItem(2, col, item);

		ui.courseTableWidget2->setRowHeight(2, 70);
	}
	*/
}
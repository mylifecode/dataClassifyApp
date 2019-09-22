#include "SYTrainRecordWindow.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDateTime>
#include <MXDefine.h>
#include <SYMainWindow.h>
#include "SYTrainRecordDetailWindow.h"
#include "SYEntityTrainReportFormWindow.h"
#include "SYScreenshotDisplayer.h"
#include "SYUserInfo.h"
#include "SYDBMgr.h"
#include "SYScoreTable.h"
#include "RbMoviePlayer.h"

SYTrainRecordWindow::SYTrainRecordWindow(const std::vector<QSqlRecord>& records,QWidget* parent)
	:QWidget(parent),
	m_records(records),
	m_tableWidget(nullptr),
	m_trainRecordDetailWindow(nullptr),
	m_entityTrainReportFormWindow(nullptr),
	m_screenshotDisplayer(nullptr),
	m_moviePlayer(nullptr)
{
	ui.setupUi(this);

	//remove widget and delete it
	QWidget* widget1 = ui.stackedWidget->widget(0);
	QWidget* widget2 = ui.stackedWidget->widget(1);
	ui.stackedWidget->removeWidget(widget1);
	ui.stackedWidget->removeWidget(widget2);
	delete widget1;
	delete widget2;

	//addd tableWidget
	m_tableWidget = new QTableWidget;
	m_tableWidget->setObjectName("tableWidget");
	m_tableWidget->setColumnCount(6);
	m_tableWidget->setColumnWidth(0, 324);
	m_tableWidget->setColumnWidth(1, 110);
	m_tableWidget->setColumnWidth(2, 242);
	m_tableWidget->setColumnWidth(3, 148);
	m_tableWidget->setColumnWidth(4, 148);
	//m_tableWidget->setColumnWidth(5, 300);
	m_tableWidget->setFrameShape(QFrame::NoFrame);
	m_tableWidget->setShowGrid(false);
	m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tableWidget->setAlternatingRowColors(true);
	m_tableWidget->setFocusPolicy(Qt::NoFocus);

	connect(m_tableWidget, &QTableWidget::cellClicked, this, &SYTrainRecordWindow::onItemClicked);

	//hide header view
	QHeaderView* headerView = m_tableWidget->horizontalHeader();
	headerView->setStretchLastSection(true);
	headerView->hide();
	headerView = m_tableWidget->verticalHeader();
	headerView->hide();

	ui.stackedWidget->addWidget(m_tableWidget);

	//m_trainRecordDetailWindow = new SYTrainRecordDetailWindow(ui.stackedWidget);
	//ui.stackedWidget->addWidget(m_trainRecordDetailWindow);
	//connect(m_trainRecordDetailWindow, &SYTrainRecordDetailWindow::back, this, &SYTrainRecordWindow::onBackToScoreTableWidget);

	m_screenshotDisplayer = new SYScreenshotDisplayer(ui.stackedWidget);
	ui.stackedWidget->addWidget(m_screenshotDisplayer);
	connect(m_screenshotDisplayer, &SYScreenshotDisplayer::back, this, &SYTrainRecordWindow::onBackToScoreTableWidget);

	updateTableWidget();

	Mx::setWidgetStyle(this, "qss:SYTrainRecordWindow.qss");
}

SYTrainRecordWindow::~SYTrainRecordWindow()
{

}

void SYTrainRecordWindow::showEvent(QShowEvent* event)
{
	
}

void SYTrainRecordWindow::onActiveComboBoxItem(int index)
{
	addRecord(index - 1);
}

void SYTrainRecordWindow::updateTableWidget()
{
	addRecord(-1);
}

void SYTrainRecordWindow::addRecord(int displayTrainTypeCode)
{
	const QColor headerTextColor = QColor(0xff, 0xff, 0xff);
	const QColor contentTextColor = QColor(0x85, 0x90, 0x9e);
	auto SetItemAttribute = [](QTableWidgetItem* item,const QColor& textColor,int sqlRecordIndex,bool needUnderline){
		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);

		QFont font = item->font();
		font.setUnderline(needUnderline);
		font.setPixelSize(16);
		item->setFont(font);

		item->setTextColor(textColor);

		item->setData(Qt::UserRole, QVariant(sqlRecordIndex));
	};

	m_tableWidget->clearContents();
	const std::size_t nRecord = m_records.size() + 1;
	m_tableWidget->setRowCount(nRecord);
	m_tableWidget->setRowHeight(0, 79);


	//col 0
	QTableWidgetItem* item = new QTableWidgetItem(QString::fromLocal8Bit("训练日期"));
	SetItemAttribute(item,headerTextColor,-1,false);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_tableWidget->setItem(0, 0, item);

	//col 1
	QHBoxLayout* layout = new QHBoxLayout;
	layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

	QComboBox* comboBox = new QComboBox;
	comboBox->setObjectName("comboBox");
	comboBox->setEditable(false);
	comboBox->addItem(QString::fromLocal8Bit("所有训练"));
	comboBox->addItem(QString::fromLocal8Bit("实物训练"));
	comboBox->addItem(QString::fromLocal8Bit("技能训练"));
	comboBox->addItem(QString::fromLocal8Bit("手术训练"));
	comboBox->setCurrentIndex(displayTrainTypeCode + 1);
	connect(comboBox, SIGNAL(activated(int)), this, SLOT(onActiveComboBoxItem(int)));
	layout->addWidget(comboBox);

	layout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

	QWidget* widget = new QWidget;
	widget->setLayout(layout);
	m_tableWidget->setCellWidget(0, 1, widget);

	//col 2
	item = new QTableWidgetItem(QString::fromLocal8Bit("训练项目"));
	SetItemAttribute(item, headerTextColor,-1,false);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_tableWidget->setItem(0, 2, item);

	//col 3
	item = new QTableWidgetItem(QString::fromLocal8Bit("分数"));
	SetItemAttribute(item, headerTextColor,-1,false);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_tableWidget->setItem(0, 3, item);

	//col 4
	item = new QTableWidgetItem(QString::fromLocal8Bit("截图"));
	SetItemAttribute(item, headerTextColor, -1,false);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_tableWidget->setItem(0, 4, item);

	//col 5
	item = new QTableWidgetItem(QString::fromLocal8Bit("视频"));
	SetItemAttribute(item, headerTextColor, -1,false);
	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	m_tableWidget->setItem(0, 5, item);

	int nRow = 1;
	for(std::size_t i = 1; i < nRecord; ++i){
		const QSqlRecord& record = m_records[i - 1];
		int trainTypeCode = record.value("trainTypeCode").toInt();
		if(displayTrainTypeCode != -1 && trainTypeCode != displayTrainTypeCode)
			continue;

		//col 0
		QTableWidgetItem* item = new QTableWidgetItem(record.value("beginTime").toDateTime().toString("yyyy/MM/dd hh:mm"));
		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		SetItemAttribute(item, contentTextColor, i - 1,false);
		m_tableWidget->setItem(nRow, 0, item);

		//col 1
		item = new QTableWidgetItem();
		if(trainTypeCode == 0)
			item->setText(QString::fromLocal8Bit("实物训练"));
		else if(trainTypeCode == 1)
			item->setText(QString::fromLocal8Bit("技能训练"));
		else
			item->setText(QString::fromLocal8Bit("手术训练"));

		SetItemAttribute(item, contentTextColor, i - 1,false);
		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		m_tableWidget->setItem(nRow, 1, item);

		//col 2
		item = new QTableWidgetItem(record.value("trainName").toString());
		SetItemAttribute(item, contentTextColor, i - 1,false);
		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		m_tableWidget->setItem(nRow, 2, item);

		//col 3
		item = new QTableWidgetItem(record.value("score").toString());
		SetItemAttribute(item, contentTextColor, i - 1,true);
		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		item->setForeground(QBrush(QColor(0x28, 0x9f, 0xff)));
		m_tableWidget->setItem(nRow, 3, item);

		//col 4
		item = new QTableWidgetItem(QString::fromLocal8Bit("查看截图"));
		SetItemAttribute(item, contentTextColor, i - 1,true);
		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		item->setForeground(QBrush(QColor(0x28, 0x9f, 0xff)));
		m_tableWidget->setItem(nRow, 4, item);

		//col 5
		item = new QTableWidgetItem(QString::fromLocal8Bit("查看视频"));
		SetItemAttribute(item, contentTextColor, i - 1,true);
		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		item->setForeground(QBrush(QColor(0x28, 0x9f, 0xff)));
		m_tableWidget->setItem(nRow, 5, item);

		//set row height
		m_tableWidget->setRowHeight(nRow, 70);
		++nRow;
	}

	m_tableWidget->setRowCount(nRow);
}

void SYTrainRecordWindow::onItemClicked(int row,int col)
{
	//第0行为表头
	if(row == 0)
		return;

	QTableWidgetItem* item = m_tableWidget->item(row, col);
	if(item == nullptr)
		return;

	int sqlRecordIndex = item->data(Qt::UserRole).toInt();
	if(sqlRecordIndex < 0 || (std::size_t)sqlRecordIndex >= m_records.size())
		return;

	const QSqlRecord& record = m_records[sqlRecordIndex];
	int scoreId = record.value("id").toInt();

	//第3列为查看成绩详情
	//第4列为查看截图
	//第5列为查看视频
	if(col == 3){
		QString trainTypeCode = record.value("trainTypeCode").toString();

		//实物训练
		if(trainTypeCode == "0"){
			if(m_entityTrainReportFormWindow == nullptr)
				m_entityTrainReportFormWindow = new SYEntityTrainReportFormWindow(SYMainWindow::GetInstance());

			m_entityTrainReportFormWindow->SetTrainInfo(record);
			m_entityTrainReportFormWindow->UpdateTable();
			m_entityTrainReportFormWindow->showFullScreen();
		}//技能训练和手术训练
 		else{
			if(m_trainRecordDetailWindow == nullptr)
				m_trainRecordDetailWindow = new SYTrainRecordDetailWindow(SYMainWindow::GetInstance());

			QString trainCode = record.value("trainCode").toString();
			QString scoreTableCode = SYScoreTable::GetScoreTableCode(trainTypeCode, trainCode);

			m_trainRecordDetailWindow->setShowErrorItemOnly(false);
			m_trainRecordDetailWindow->setContent(SYUserInfo::Instance()->GetUserId(),
												  record.value("id").toInt(),
												  record.value("trainName").toString(),
												  record.value("score").toInt(),
												  scoreTableCode);
			m_trainRecordDetailWindow->setTrainUsedTime(record.value("costTime").toInt());
			//ui.stackedWidget->setCurrentWidget(m_trainRecordDetailWindow);
			m_trainRecordDetailWindow->showFullScreen();
		}
	}
	else if(col == 4){
		//if(m_screenshotDisplayer == nullptr){
		//	m_screenshotDisplayer = new SYScreenshotDisplayer;
		//	ui.stackedWidget->addWidget(m_screenshotDisplayer);
		//}

		//m_screenshotDisplayer->show();
		m_screenshotDisplayer->setScoreId(scoreId);
		ui.stackedWidget->setCurrentWidget(m_screenshotDisplayer);
	}
	else if(col == 5){
		QVector<QSqlRecord> records;
		SYDBMgr::Instance()->QueryVideoFiles(scoreId, records);
		if(records.size() > 0){
			QSqlRecord& record = records[0];
			QString videoFileName = record.value("videoFileName").toString();

			if(m_moviePlayer == nullptr)
				m_moviePlayer = new RbMoviePlayer(SYMainWindow::GetInstance());

			m_moviePlayer->loadMoviePath(videoFileName);
			m_moviePlayer->Play();
			m_moviePlayer->show();
		}
		else
			QMessageBox::information(this, "", QString::fromLocal8Bit("无视频文件"));
	}
}

void SYTrainRecordWindow::onBackToScoreTableWidget()
{
	ui.stackedWidget->setCurrentWidget(m_tableWidget);
}
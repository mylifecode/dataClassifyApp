#include "SYAdminEntityTrainAnalysisWindow.h"
#include <MxDefine.h>
#include "SYStringTable.h"
#include <SYDBMgr.h>
#include <QHeaderView>
#include "SYEntityTrainReportFormWindow.h"
#include "SYMainWindow.h"
#include "SYAdminScreenshotDisplayer.h"
#include <RbMoviePlayer.h>

const QColor g_normalColor = QColor(0xb7, 0xc5, 0xd8);
const QColor g_blue = QColor(0x28, 0x9f, 0xff);

SYAdminEntityTrainAnalysisWindow::SYAdminEntityTrainAnalysisWindow(QWidget* parent)
	:QWidget(nullptr),
	m_canUpdateData(true),
	m_trainReportFormWindow(nullptr),
	m_screenshotDisplayer(nullptr),
	m_moviePlayer(nullptr)
{
	ui.setupUi(this);

	ui.userFilter->addItem(SYStringTable::GetString(SYStringTable::STR_FilterByUser), 0);
	ui.classFilter->addItem(SYStringTable::GetString(SYStringTable::STR_FilterByClass), 0);
	ui.groupFilter->addItem(SYStringTable::GetString(SYStringTable::STR_FilterByGroup), 0);

	connect(ui.trainRecordTableWidget, &QTableWidget::cellClicked, this, &SYAdminEntityTrainAnalysisWindow::onClickedTrainRecordCell);

	Mx::setWidgetStyle(this, "qss:SYAdminEntityTrainAnalysisWindow.qss");
}

SYAdminEntityTrainAnalysisWindow::~SYAdminEntityTrainAnalysisWindow()
{

}

void SYAdminEntityTrainAnalysisWindow::LoadTrainStatisticData()
{
	//1
	ui.trainStatisticWindow->SetTrainType(TT_EntityTrain);
	ui.trainStatisticWindow->SetChartTitle(SYStringTable::GetString(SYStringTable::STR_MODULETRAINTIMES),
										   SYStringTable::GetString(SYStringTable::STR_MODULETRAINTIME));
	ui.trainStatisticWindow->LoadData();

	//2
	LoadRankData();

	//3
	InitTrainRecordTableWidget();
	LoadTrainRecords();
}

void SYAdminEntityTrainAnalysisWindow::SetTableWidgetItemAttribute(QTableWidgetItem* item,const QColor& color,int recordIndex,bool needUnderline)
{
	Qt::ItemFlags flags = item->flags();
	flags = flags & (~Qt::ItemIsEditable);
	item->setFlags(flags);

	item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

	QFont font = item->font();
	font.setPixelSize(16);
	font.setUnderline(needUnderline);
	item->setFont(font);

	item->setTextColor(color);

	item->setData(Qt::UserRole, recordIndex);
	//item->setBackgroundColor(QColor(27, 32, 42));
}

void SYAdminEntityTrainAnalysisWindow::ClearRankTable()
{
	QTableWidget* tables[] = {ui.trainCountRankTableWidget, ui.trainTimeRankTableWidget, ui.clampPeaCountRankTableWidget};
	QString iconNames[3] = {"icons:/SYPersonCenterWindow/1.png", "icons:/SYPersonCenterWindow/2.png", "icons:/SYPersonCenterWindow/3.png"};
	const QColor col0Color = QColor(255, 255, 255);
	const QColor normalColor = QColor(0xb7, 0xc5, 0xd8);
	QTableWidgetItem* item = nullptr;

	for(auto table : tables){
		table->clearContents();
		table->setRowCount(5);
		table->setColumnCount(3);		//第一列显示图标
		table->setColumnWidth(0, 174);
		table->setColumnWidth(1, 146);
		table->setColumnWidth(2, 100);
		table->setFrameShape(QFrame::NoFrame);
		table->setShowGrid(false);
		table->setSelectionMode(QAbstractItemView::NoSelection);
		table->setFocusPolicy(Qt::NoFocus);
		table->horizontalHeader()->setStretchLastSection(true);

		table->horizontalHeader()->hide();
		table->verticalHeader()->hide();

		for(int row = 0; row < 5; ++row){
			//col 0
			if(row < 3){
				QWidget* widget = new QWidget;

				QHBoxLayout* layout = new QHBoxLayout;
				layout->setMargin(0);
				layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
				QLabel* label = new QLabel;
				label->setPixmap(QPixmap(iconNames[row]));
				layout->addWidget(label);
				layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
				widget->setLayout(layout);

				table->setCellWidget(row, 0, widget);
			}
			else{
				item = new QTableWidgetItem();
				item->setText(SYStringTable::GetString(SYStringTable::STR_FirstPrize + row));
				SetTableWidgetItemAttribute(item,col0Color,row);
				table->setItem(row, 0, item);
			}

			//col 1 and col 2
			for(int i = 0; i < 2; ++i){
				item = new QTableWidgetItem();
				item->setText("-");
				SetTableWidgetItemAttribute(item, normalColor,row);
				table->setItem(row, 1 + i, item);
			}

			table->setRowHeight(row, 70);
		}
	}
}

void SYAdminEntityTrainAnalysisWindow::LoadRankData()
{
	const QColor col0Color = QColor(255, 255, 255);
	const QColor normalColor = QColor(0xb7, 0xc5, 0xd8);

	auto SetTableContent = [&](QTableWidget* table,const std::vector<QSqlRecord>& records){
		QTableWidgetItem* item = nullptr;
		const size_t nMaxRow = 5;
		std::size_t n = 0;

		n = std::min(nMaxRow, records.size());
		for(std::size_t i = 0; i < n; ++i){
			const auto& record = records[i];
			//col 1
			item = new QTableWidgetItem();
			item->setText(record.value(0).toString());
			SetTableWidgetItemAttribute(item, normalColor,i);
			table->setItem(i, 1, item);

			//col 2
			item = new QTableWidgetItem();
			item->setText(record.value(1).toString());
			SetTableWidgetItemAttribute(item,normalColor,i);
			table->setItem(i, 2, item);
		}
	};


	ClearRankTable();

	std::vector<QSqlRecord> records;
	

	//train count rank
	SYDBMgr::Instance()->QueryTrainCountRank(TT_EntityTrain, records);
	SetTableContent(ui.trainCountRankTableWidget, records);

	//train time rank
	records.clear();
	SYDBMgr::Instance()->QueryTrainTimeRank(TT_EntityTrain, records);
	SetTableContent(ui.trainTimeRankTableWidget, records);

	//clamp pea times rank
	records.clear();
	SYDBMgr::Instance()->QueryClampPeaTrainCountRank(records);
	SetTableContent(ui.clampPeaCountRankTableWidget, records);
}

void SYAdminEntityTrainAnalysisWindow::LoadTrainRecords()
{
	m_allRecords.clear();
	m_recordIndices.clear();
	ui.trainRecordTableWidget->clearContents();

	SYDBMgr::Instance()->QueryAllScoreRecordOrderByDateDescend(TT_EntityTrain, m_allRecords,true);

	std::size_t nRecord = m_allRecords.size();
	ui.trainRecordTableWidget->setRowCount(nRecord);
	QTableWidgetItem* item = nullptr;
	QString name,className,groupName;
	std::vector<int> tempIndices;

	for(std::size_t row = 0; row < nRecord; ++row){
		const auto& record = m_allRecords[row];
		int userId = record.value("userId").toInt();

		//col 0
		item = new QTableWidgetItem();
		item->setText(record.value("beginTime").toDateTime().toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT2)));
		SetTableWidgetItemAttribute(item, g_normalColor, row);
		ui.trainRecordTableWidget->setItem(row, 0, item);

		//col 1
		item = new QTableWidgetItem();
		item->setText(record.value("trainName").toString());
		SetTableWidgetItemAttribute(item, g_normalColor, row);
		ui.trainRecordTableWidget->setItem(row, 1, item);

		//col 2
		name = record.value("realName").toString();
		item = new QTableWidgetItem();
		item->setText(name);
		SetTableWidgetItemAttribute(item, g_normalColor, row);
		ui.trainRecordTableWidget->setItem(row, 2, item);

		//col 3
		//content = record.value("class").toString();
		item = new QTableWidgetItem();
		className = "-";
		item->setText(className);
		SetTableWidgetItemAttribute(item, g_normalColor, row);
		ui.trainRecordTableWidget->setItem(row, 3, item);

		//col 4
		groupName = record.value("groupName").toString();
		item = new QTableWidgetItem();
		item->setText(groupName);
		SetTableWidgetItemAttribute(item, g_normalColor, row);
		ui.trainRecordTableWidget->setItem(row, 4, item);

		//col 5
		item = new QTableWidgetItem();
		item->setText(SYStringTable::GetString(SYStringTable::STR_Check));
		SetTableWidgetItemAttribute(item, g_blue, row, true);
		ui.trainRecordTableWidget->setItem(row, 5, item);

		//col 6
		item = new QTableWidgetItem();
		item->setText(SYStringTable::GetString(SYStringTable::STR_Check));
		SetTableWidgetItemAttribute(item, g_blue, row, true);
		ui.trainRecordTableWidget->setItem(row, 6, item);

		//col 7
		item = new QTableWidgetItem();
		item->setText(SYStringTable::GetString(SYStringTable::STR_Check));
		SetTableWidgetItemAttribute(item, g_blue, row, true);
		ui.trainRecordTableWidget->setItem(row, 7, item);

		m_recordIndices.push_back(row);
		ui.trainRecordTableWidget->setRowHeight(row, 70);

		//
		tempIndices.clear();
		tempIndices.push_back(row);

		auto itr1 = m_userRecordMapById.find(userId);
		if(itr1 == m_userRecordMapById.end()){
			m_userRecordMapById.insert(std::pair<int,std::vector<int>>(userId,tempIndices));
			ui.userFilter->addItem(name, userId);
		}
		else{
			itr1->second.push_back(row);
		}

		auto itr2 = m_userRecordMapByClass.find(className);
		if(itr2 == m_userRecordMapByClass.end()){
			m_userRecordMapByClass.insert(std::pair<QString, std::vector<int>>(className, tempIndices));
			ui.classFilter->addItem(className,1);
		}
		else{
			itr2->second.push_back(row);
		}

		auto itr3 = m_userRecordMapByGroup.find(groupName);
		if(itr3 == m_userRecordMapByGroup.end()){
			m_userRecordMapByGroup.insert(std::pair<QString, std::vector<int>>(groupName, tempIndices));
			ui.groupFilter->addItem(groupName,1);
		}
		else{
			itr3->second.push_back(row);
		}
	}
}

void SYAdminEntityTrainAnalysisWindow::InitTrainRecordTableWidget()
{
	const int nColumn = 8;
	ui.trainRecordTableWidget->clearContents();
	ui.trainRecordTableWidget->setColumnCount(nColumn);
	ui.trainRecordTableWidget->setColumnWidth(0, 308);
	ui.trainRecordTableWidget->setColumnWidth(1, 200);
	ui.trainRecordTableWidget->setColumnWidth(2, 290);
	ui.trainRecordTableWidget->setColumnWidth(3, 146);
	ui.trainRecordTableWidget->setColumnWidth(4, 244);
	ui.trainRecordTableWidget->setColumnWidth(5, 164);
	ui.trainRecordTableWidget->setColumnWidth(6, 164);
	ui.trainRecordTableWidget->setColumnWidth(7, 164);
	ui.trainRecordTableWidget->setFrameShape(QFrame::NoFrame);
	ui.trainRecordTableWidget->setShowGrid(false);
	ui.trainRecordTableWidget->setSelectionMode(QAbstractItemView::NoSelection);
	ui.trainRecordTableWidget->setFocusPolicy(Qt::NoFocus);
	ui.trainRecordTableWidget->setAlternatingRowColors(true);
	ui.trainRecordTableWidget->horizontalHeader()->setObjectName("trainRecordHeaderView");
	ui.trainRecordTableWidget->horizontalHeader()->setEnabled(false);
	ui.trainRecordTableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.trainRecordTableWidget->verticalHeader()->hide();

	QStringList headerLabels;
	for(int i = 0; i < nColumn; ++i){
		headerLabels << SYStringTable::GetString(SYStringTable::STR_TrainDate + i);
	}
	ui.trainRecordTableWidget->setHorizontalHeaderLabels(headerLabels);
}

void SYAdminEntityTrainAnalysisWindow::on_userFilter_currentIndexChanged(int index)
{
	if(!m_canUpdateData)
		return;

	if(index == 0){
		updateTable(m_recordIndices);
	}
	else{
		int userId = ui.userFilter->itemData(index, Qt::UserRole).toInt();
		auto itr = m_userRecordMapById.find(userId);
		if(itr != m_userRecordMapById.end())
			updateTable(itr->second);
	}

	m_canUpdateData = false;
	ui.classFilter->setCurrentIndex(0);
	ui.groupFilter->setCurrentIndex(0);
	m_canUpdateData = true;
}

void SYAdminEntityTrainAnalysisWindow::on_classFilter_currentIndexChanged(int index)
{
	if(!m_canUpdateData)
		return;

	if(index == 0){
		updateTable(m_recordIndices);
	}
	else{
		QString className = ui.classFilter->itemText(index);
		auto itr = m_userRecordMapByClass.find(className);
		if(itr != m_userRecordMapByClass.end())
			updateTable(itr->second);
	}

	m_canUpdateData = false;
	ui.userFilter->setCurrentIndex(0);
	ui.groupFilter->setCurrentIndex(0);
	m_canUpdateData = true;

}

void SYAdminEntityTrainAnalysisWindow::on_groupFilter_currentIndexChanged(int index)
{
	if(!m_canUpdateData)
		return;

	if(index == 0){
		updateTable(m_recordIndices);
	}
	else{
		QString groupName = ui.groupFilter->itemText(index);
		auto itr = m_userRecordMapByGroup.find(groupName);
		if(itr != m_userRecordMapByGroup.end())
			updateTable(itr->second);
	}

	m_canUpdateData = false;
	ui.classFilter->setCurrentIndex(0);
	ui.userFilter->setCurrentIndex(0);
	m_canUpdateData = true;

}

void SYAdminEntityTrainAnalysisWindow::updateTable(const std::vector<int>& recordIndices)
{
	if(recordIndices.size() == 0)
		return;

	std::size_t nRecord = recordIndices.size();
	ui.trainRecordTableWidget->clearContents();
	ui.trainRecordTableWidget->setRowCount(nRecord);
	QTableWidgetItem* item = nullptr;
	QString name, className, groupName;

	for(std::size_t row = 0; row < nRecord; ++row){
		int index = recordIndices[row];
		const auto& record = m_allRecords[index];
		int userId = record.value("userId").toInt();

		//col 0
		item = new QTableWidgetItem();
		item->setText(record.value("beginTime").toDateTime().toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT2)));
		SetTableWidgetItemAttribute(item, g_normalColor, index);
		ui.trainRecordTableWidget->setItem(row, 0, item);

		//col 1
		item = new QTableWidgetItem();
		item->setText(record.value("trainName").toString());
		SetTableWidgetItemAttribute(item, g_normalColor, index);
		ui.trainRecordTableWidget->setItem(row, 1, item);

		//col 2
		name = record.value("realName").toString();
		item = new QTableWidgetItem();
		item->setText(name);
		SetTableWidgetItemAttribute(item, g_normalColor, index);
		ui.trainRecordTableWidget->setItem(row, 2, item);

		//col 3
		//content = record.value("class").toString();
		item = new QTableWidgetItem();
		className = "-";
		item->setText(className);
		SetTableWidgetItemAttribute(item, g_normalColor, index);
		ui.trainRecordTableWidget->setItem(row, 3, item);

		//col 4
		groupName = record.value("groupName").toString();
		item = new QTableWidgetItem();
		item->setText(groupName);
		SetTableWidgetItemAttribute(item, g_normalColor, index);
		ui.trainRecordTableWidget->setItem(row, 4, item);

		//col 5
		item = new QTableWidgetItem();
		item->setText(SYStringTable::GetString(SYStringTable::STR_Check));
		SetTableWidgetItemAttribute(item, g_blue, index, true);
		ui.trainRecordTableWidget->setItem(row, 5, item);

		//col 6
		item = new QTableWidgetItem();
		item->setText(SYStringTable::GetString(SYStringTable::STR_Check));
		SetTableWidgetItemAttribute(item, g_blue, index, true);
		ui.trainRecordTableWidget->setItem(row, 6, item);

		//col 7
		item = new QTableWidgetItem();
		item->setText(SYStringTable::GetString(SYStringTable::STR_Check));
		SetTableWidgetItemAttribute(item, g_blue, index, true);
		ui.trainRecordTableWidget->setItem(row, 7, item);

		ui.trainRecordTableWidget->setRowHeight(row, 70);
	}
}

void SYAdminEntityTrainAnalysisWindow::onClickedTrainRecordCell(int row, int col)
{
	QTableWidgetItem* item = ui.trainRecordTableWidget->item(row, col);
	if(item == nullptr)
		return;
	
	bool ok = false;
	int recordIndex = item->data(Qt::UserRole).toInt(&ok);
	if(!ok)
		return;

	const QSqlRecord& record = m_allRecords[recordIndex];
	//qDebug() << record;
	int scoreId = 0;
	//训练报表
	if(col == 5){
		if(m_trainReportFormWindow == nullptr)
			m_trainReportFormWindow = new SYEntityTrainReportFormWindow(SYMainWindow::GetInstance());

		m_trainReportFormWindow->SetTrainInfo(record);
		m_trainReportFormWindow->UpdateTable();
		m_trainReportFormWindow->showFullScreen();
	}//截图
	else if(col == 6){
		if(m_screenshotDisplayer == nullptr){
// 			int width = 1260;
// 			int height = 1006;
// 			QSize parentSize = SYMainWindow::GetInstance()->size();
// 			m_screenshotDisplayer = new SYScreenshotDisplayer(SYMainWindow::GetInstance());
// 			m_screenshotDisplayer->resize(width, height);
// 			m_screenshotDisplayer->move((parentSize.width() - width) / 2, (parentSize.height() - height) / 2);
			m_screenshotDisplayer = new SYAdminScreenshotDisplayer(SYMainWindow::GetInstance());
		}
		
		scoreId = record.value("scoreId").toInt();
		m_screenshotDisplayer->SetScoreId(scoreId);
		m_screenshotDisplayer->showFullScreen();
	}//视频
	else if(col == 7){
		QVector<QSqlRecord> records;
		scoreId = record.value("scoreId").toInt();

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
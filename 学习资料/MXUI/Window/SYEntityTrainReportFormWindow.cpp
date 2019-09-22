#include "SYEntityTrainReportFormWindow.h"
#include <MxDefine.h>
#include <SYDBMgr.h>
#include "SYStringTable.h"
#include <QPixmap>
#include <QScrollBar>
#include <QSqlField>
#include "tinyxml.h"


static const int g_tableRowHeight = 60;

SYEntityTrainReportFormWindow::SYEntityTrainReportFormWindow(QWidget* parent)
	:RbShieldLayer(parent),
	m_nScreenshot(0)
{
	ui.setupUi(this);

	hideCloseButton();
	hideOkButton();

	QHBoxLayout* hLayout = new QHBoxLayout;
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout->addWidget(ui.backgroundFrame);
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout->setMargin(0);

	ui.scrollAreaWidgetContents->setLayout(hLayout);

	InitTable();

	Mx::setWidgetStyle(this, "qss:/SYEntityTrainReportFormWindow.qss");
}

SYEntityTrainReportFormWindow::~SYEntityTrainReportFormWindow()
{

}

void SYEntityTrainReportFormWindow::SetTableWidgetItemAttribute(QTableWidgetItem* item, const QString& text,const QColor& color)
{
	Qt::ItemFlags flags = item->flags();
	flags = flags & (~Qt::ItemIsEditable);
	item->setFlags(flags);

	item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	QFont font = item->font();
	font.setPixelSize(20);
	item->setFont(font);

	item->setTextColor(color);
	item->setText(text);
}

void SYEntityTrainReportFormWindow::SetTrainInfo(const QSqlRecord& scoreRecord)
{
	m_Details.clear();

	//学生端传入的record中的字段全为score表中的字段，而教师段传入的record中，id既是score的id又是学生的id，所以通过id获取会值无法准确的得到想要的
	int scoreId = 0;
	QSqlField field = scoreRecord.field("scoreId");

	if(field.isValid())
		scoreId = field.value().toInt();//scoreRecord.value("scoreId").toInt();
	else
		scoreId = scoreRecord.value("id").toInt();

	const QSize imageSize(390, 310);
	m_record = scoreRecord;
	m_trainCode = scoreRecord.value("trainCode").toString();
	m_trainName = scoreRecord.value("trainName").toString();
	m_trainTime = scoreRecord.value("beginTime").toDateTime();
	QString trainDescXML = scoreRecord.value("scoredetail").toString();
	QString qcontent = "";
	QString qtime = "";
	std::string contentstr = trainDescXML.toUtf8().toStdString();

	TiXmlDocument * pDoc = new TiXmlDocument();
	pDoc->Parse(contentstr.c_str());
	
	TiXmlNode * pRootNode = pDoc->RootElement();
	if (pRootNode)
	{
		TiXmlElement* element = pRootNode->FirstChildElement();
		for (; element != NULL; element = element->NextSiblingElement())
		{
			std::string  content = element->Attribute("content");
			std::string time = element->Attribute("time");
			
			qcontent = QString::fromUtf8(content.c_str());
			qtime = QString::fromUtf8(time.c_str());

			m_Details.push_back(std::make_pair(qtime, qcontent));
		}
	}
	delete pDoc;
	
	//trian name & train time
	ui.titleLabel->setText(m_trainName);
	ui.trainTimeLabel->setText(m_trainTime.toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT3)));

	//train times
	int times = 0;
	SYDBMgr::Instance()->QueryTrainTimes(TT_EntityTrain, m_trainCode, times);
	ui.trainTimes->setText(QString().setNum(times) + SYStringTable::GetString(SYStringTable::STR_Times));

	//pictures
	QVector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryScreenshots(scoreId, records);
	int nRecord = records.size();
	if(nRecord > 0){
		bool ok;
		QPixmap pixmap1;
		ok = pixmap1.loadFromData(records[0].value("data").toByteArray());
		if(ok)
			ui.sceneLabel1->setPixmap(pixmap1.scaled(imageSize));
		
		if(nRecord > 1){
			QPixmap pixmap2;
			ok = pixmap2.loadFromData(records.last().value("data").toByteArray());
			if(ok)
				ui.sceneLable2->setPixmap(pixmap2.scaled(imageSize));
		}
	}
	else{
		QPixmap pixmap;
		ui.sceneLabel1->setPixmap(pixmap);
		ui.sceneLable2->setPixmap(pixmap);
	}

	m_nScreenshot = nRecord;
}

void SYEntityTrainReportFormWindow::UpdateTable()
{
	QString content;
	QTableWidgetItem* item = nullptr;
	const QColor itemColor1(0xc4, 0xc3, 0xd5);
	const QColor itemColor2(0x28, 0x9f, 0xff);

	//train info table 1
	ui.trainInfoTable1->clearContents();
	
	//row 0
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_TrainUseTime), itemColor1);
	ui.trainInfoTable1->setItem(0, 1, item);

	int costTime = m_record.value("costTime").toInt();
	int h = costTime / 3600;
	int m = (costTime % 3600) / 60;
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, QString("%1:%2").arg(h).arg(m), itemColor1);
	ui.trainInfoTable1->setItem(0, 2, item);

	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, "-", itemColor1);
	ui.trainInfoTable1->setItem(0, 3, item);

	//row 1
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_ToolMoveDistance), itemColor1);
	ui.trainInfoTable1->setItem(1, 1, item);

	float moveDistanceLeft = m_record.value("movedistanceleft").toFloat();
	float moveDistanceRight = m_record.value("movedistanceright").toFloat();
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, content.setNum(moveDistanceLeft + moveDistanceRight, 'g', 2), itemColor1);
	ui.trainInfoTable1->setItem(1, 2, item);

	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, "-", itemColor1);
	ui.trainInfoTable1->setItem(1, 3, item);

	//row 2
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_CameraMoveDistance), itemColor1);
	ui.trainInfoTable1->setItem(2, 1, item);

	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, "-", itemColor1);
	ui.trainInfoTable1->setItem(2, 2, item);

	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, "-", itemColor1);
	ui.trainInfoTable1->setItem(2, 3, item);

	//row 3
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_ToolMoveAvgVelocity), itemColor1);
	ui.trainInfoTable1->setItem(3, 1, item);

	float vl = m_record.value("movespeedleft").toFloat();
	float vr = m_record.value("movespeedright").toFloat();
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, content.setNum((vl + vr) / 2, 'g', 2), itemColor1);
	ui.trainInfoTable1->setItem(3, 2, item);

	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, "-", itemColor1);
	ui.trainInfoTable1->setItem(3, 3, item);


	//train info table 2
	ui.trainInfoTable2->clearContents();

	//row 0
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_SavePicture), itemColor2);
	ui.trainInfoTable2->setItem(0, 1, item);

	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, content.setNum(m_nScreenshot), itemColor2);
	ui.trainInfoTable2->setItem(0, 2, item);

	//row 1
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_VideoTotalTime), itemColor2);
	ui.trainInfoTable2->setItem(1, 1, item);

	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, "-", itemColor2);
	ui.trainInfoTable2->setItem(1, 2, item);

	//row 2
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_CameraUseTime), itemColor1);
	ui.trainInfoTable2->setItem(2, 1, item);

	float holdCameraUseTime = m_record.value("holdcamerausetime").toFloat();
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, content.setNum(holdCameraUseTime, 'g', 2), itemColor1);
	ui.trainInfoTable2->setItem(2, 2, item);

	//row 3
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, SYStringTable::GetString(SYStringTable::STR_FixdCameraUseTime), itemColor1);
	ui.trainInfoTable2->setItem(3, 1, item);

	float fixedCameraUseTime = m_record.value("fixcamerausetime").toFloat();
	item = new QTableWidgetItem;
	SetTableWidgetItemAttribute(item, content.setNum(fixedCameraUseTime, 'g', 2), itemColor1);
	ui.trainInfoTable2->setItem(3, 2, item);


	if (m_Details.size() > 0)
	{ 
		ui.trainRecordTable->setRowCount(m_Details.size());
		ui.trainRecordTable->setMinimumHeight(g_tableRowHeight * (m_Details.size() + 1));
		ui.trainRecordTable->clearContents();

		for (std::size_t c = 0; c < m_Details.size(); c++)
		{
			QString time = m_Details[c].first;
			QString content = m_Details[c].second;
			item = new QTableWidgetItem;
			//item->setText(time);
			SetTableWidgetItemAttribute(item, time, itemColor1);
			ui.trainRecordTable->setItem(c, 1, item);

			item = new QTableWidgetItem;
			//item->setText(content);
			SetTableWidgetItemAttribute(item, content, itemColor1);
			ui.trainRecordTable->setItem(c, 2, item);

			ui.trainRecordTable->setRowHeight(c, g_tableRowHeight);
		}
	}
}

void SYEntityTrainReportFormWindow::InitTable()
{
	struct HeaderInfo{
		int width;
		QString text;
	};

	auto InitTableImpl = [](QTableWidget* table,const QVector<HeaderInfo>& headerInfos){
		table->clearContents();
		table->setFrameShape(QFrame::NoFrame);
		table->setShowGrid(false);
		table->setSelectionMode(QAbstractItemView::NoSelection);
		table->setFocusPolicy(Qt::NoFocus);
		table->setAlternatingRowColors(true);
		table->horizontalHeader()->setObjectName("headerView");
		table->horizontalHeader()->setEnabled(false);
		table->horizontalHeader()->setStretchLastSection(true);
		table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		table->verticalHeader()->hide();

		int n = headerInfos.size();
		table->setColumnCount(n);

		QStringList headerLabels;
		for(int i = 0; i < n; ++i){
			const auto& headerInfo = headerInfos[i];
			table->setColumnWidth(i, headerInfo.width);
			headerLabels << headerInfo.text;
		}
		
		table->setHorizontalHeaderLabels(headerLabels);
	};

	QVector<HeaderInfo> headerInfos1 = {{80, QString("")}, {280, QString("")}, {240, QString::fromLocal8Bit("所得数据")}, {100, QString::fromLocal8Bit("参考数据")}};
	QVector<HeaderInfo> headerInfos2 = {{80, QString("")}, {430, QString::fromLocal8Bit("项目")}, {100, QString::fromLocal8Bit("数值")}};
	QVector<HeaderInfo> headerInfos3 = {{100, QString("")}, {860, QString::fromLocal8Bit("时刻")}, {100, QString::fromLocal8Bit("事件")}};

	ui.trainInfoTable1->setRowCount(4);
	InitTableImpl(ui.trainInfoTable1, headerInfos1);

	ui.trainInfoTable2->setRowCount(4);
	InitTableImpl(ui.trainInfoTable2, headerInfos2);

	ui.trainRecordTable->setRowCount(7);		//for test
	ui.trainRecordTable->setMinimumHeight(g_tableRowHeight * (7 + 1));
	InitTableImpl(ui.trainRecordTable, headerInfos3);

	for(int row = 0; row < 4; ++row){
		ui.trainInfoTable1->setRowHeight(row, g_tableRowHeight);
		ui.trainInfoTable2->setRowHeight(row, g_tableRowHeight);
	}

	for(int row = 0; row < 7; ++row){
		ui.trainRecordTable->setRowHeight(row, g_tableRowHeight);
	}
}

void SYEntityTrainReportFormWindow::showEvent(QShowEvent* event)
{
	ui.scrollArea->verticalScrollBar()->setValue(0);
}

void SYEntityTrainReportFormWindow::on_backBtn_clicked()
{
	onOkButton();
}
#include "SYAdminTrainAnalysisWindow.h"
#include <MxDefine.h>
#include <SYDBMgr.h>
#include "SYAdminSkillTrainAnalysisWindow.h"
#include <QTableWidgetItem>
#include <QMap>
#include <SYUserData.h>

SYAdminTrainAnalysisWindow::SYAdminTrainAnalysisWindow(QWidget* parent)
	:QWidget(parent),
	m_trainType(TT_EntityTrain),
	m_trainAbilityCounter(nullptr),
	m_bottomBlueLabel(nullptr),
	m_nMaxRowOfScoreRankTable(4)
{
	m_bottomBlueLabel = new QLabel(this);
	m_bottomBlueLabel->setObjectName("bottomBlueLabel");

	ui.setupUi(this);

	ui.eliteFrame->installEventFilter(this);

	InitTable();

	Mx::setWidgetStyle(this, "qss:/SYAdminTrainAnalysisWindow.qss");
}

SYAdminTrainAnalysisWindow::~SYAdminTrainAnalysisWindow()
{

}

void SYAdminTrainAnalysisWindow::UpdateContent()
{
	ClearTableContent();

	SetEliteListInfo();

	SetScoreRankInfo();

	SetErrorInfo();
}

void SYAdminTrainAnalysisWindow::InitTable()
{
	//score rank table
	ui.scoreRankTable->setRowCount(m_nMaxRowOfScoreRankTable);
	ui.scoreRankTable->setColumnCount(3);
	ui.scoreRankTable->horizontalHeader()->setObjectName("horizontalHeader1");
	ui.scoreRankTable->horizontalHeader()->setStretchLastSection(true);
	ui.scoreRankTable->horizontalHeader()->setHighlightSections(false);
	ui.scoreRankTable->horizontalHeader()->setEnabled(false);
	ui.scoreRankTable->verticalHeader()->hide();
	ui.scoreRankTable->setColumnWidth(0, 240);
	ui.scoreRankTable->setColumnWidth(1, 200);
	ui.scoreRankTable->setColumnWidth(2, 100);
	ui.scoreRankTable->setFrameShape(QFrame::NoFrame);
	ui.scoreRankTable->setShowGrid(false);
	ui.scoreRankTable->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.scoreRankTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.scoreRankTable->setAlternatingRowColors(true);
	ui.scoreRankTable->setFocusPolicy(Qt::NoFocus);

	QStringList headerInfos;
	headerInfos << QString::fromLocal8Bit("榜上有名")
		<< QString::fromLocal8Bit("训练项目")
		<< QString::fromLocal8Bit("上榜分数");
	ui.scoreRankTable->setHorizontalHeaderLabels(headerInfos);

	//error table
	ui.errorTable->horizontalHeader()->setObjectName("horizontalHeader2");
	ui.errorTable->horizontalHeader()->setStretchLastSection(true);
	ui.errorTable->horizontalHeader()->setHighlightSections(false);
	ui.errorTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	ui.errorTable->horizontalHeader()->setEnabled(false);
	ui.errorTable->verticalHeader()->hide();
	ui.errorTable->setColumnCount(4);
	ui.errorTable->setColumnWidth(0, 80);	//第一列为空，仅为了item左对齐显示好看
	ui.errorTable->setColumnWidth(1, 340);
	ui.errorTable->setColumnWidth(2, 240);
	ui.errorTable->setColumnWidth(3, 100);
	ui.errorTable->setFrameShape(QFrame::NoFrame);
	ui.errorTable->setShowGrid(false);
	ui.errorTable->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.errorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.errorTable->setAlternatingRowColors(true);
	ui.errorTable->setFocusPolicy(Qt::NoFocus);

	headerInfos.clear();
	headerInfos << ""
		<< QString::fromLocal8Bit("评分内容")
		<< QString::fromLocal8Bit("评分项")
		<< QString::fromLocal8Bit("发生次数");
	ui.errorTable->setHorizontalHeaderLabels(headerInfos);

	//ClearTableContent();
}

void SYAdminTrainAnalysisWindow::ClearTableContent()
{
	//score rank table
	QTableWidgetItem* item;
	for(std::size_t i = 0; i < m_nMaxRowOfScoreRankTable; ++i){
		for(int col = 0; col < 3; ++col){
			item = new QTableWidgetItem();
			SetItemAttribute(item, "-", Qt::AlignHCenter | Qt::AlignVCenter);
			ui.scoreRankTable->setItem(i, col, item);
		}

		ui.scoreRankTable->setRowHeight(i, 70);
	}

	//error table
	ui.errorTable->clearContents();
}

void SYAdminTrainAnalysisWindow::SetItemAttribute(QTableWidgetItem* item, const QString& text, Qt::Alignment align)
{
	if(item == nullptr)
		return;

	item->setText(text);
	item->setTextAlignment(align);

	//flags
	auto flags = item->flags();
	item->setFlags(flags & (~Qt::ItemIsEditable));

	//font
	QFont font = item->font();
	font.setPixelSize(16);
	item->setFont(font);
}

void SYAdminTrainAnalysisWindow::SetEliteListInfo()
{
	if(m_trainAbilityCounter == nullptr)
		return;
	std::vector<QSqlRecord> records;
	//SYDBMgr::Instance()->QueryRankInfo(m_trainType,records,true);
	SYDBMgr::Instance()->QueryAvgScoreRankInfo(m_trainType, m_trainAbilityCounter->m_trainCode, records, true);

	const int nMaxLabel = 5;
	QLabel* labels[nMaxLabel] = {ui.firstLabel, ui.secondLabel, ui.thirdLabel, ui.fourthLabel, ui.fifthLabel};

	int n = records.size();
	if(n > nMaxLabel)
		n = nMaxLabel;

	for(int i = 0; i < nMaxLabel; ++i)
		labels[i]->clear();

	QString spacing("   ");
	QString content;
	for(int i = 0; i < n; ++i){
		const auto& record = records[i];

		content += spacing;
		content += spacing;
		
		content = record.value("groupName").toString();

		content += spacing;
		content += record.value("realName").toString();

		content += spacing;
		content += QString().setNum(record.value("score").toInt());
		
		content += spacing;
		content += QString().setNum(record.value("times").toInt());

		labels[i]->setText(content);
	}
}

void SYAdminTrainAnalysisWindow::SetScoreRankInfo()
{
	if(m_trainAbilityCounter == nullptr)
		return;

	std::vector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryScoreRankInfo(m_trainType, m_trainAbilityCounter->m_trainCode, records, true);

	std::size_t n = records.size();
	if(n > m_nMaxRowOfScoreRankTable)
		n = m_nMaxRowOfScoreRankTable;

	QTableWidgetItem* item = nullptr;
	for(std::size_t row = 0; row < n; ++row){
		const auto& record = records[row];

		//col 0
		item = new QTableWidgetItem;
		SetItemAttribute(item, 
						 record.value("groupName").toString() + "  " + record.value("realName").toString(),
						 Qt::AlignHCenter | Qt::AlignVCenter);
		ui.scoreRankTable->setItem(row, 0, item);

		//col 1
		item = new QTableWidgetItem;
		SetItemAttribute(item, 
						 m_trainAbilityCounter->m_trainName,
						 Qt::AlignHCenter | Qt::AlignVCenter);
		ui.scoreRankTable->setItem(row, 1, item);

		//col 2
		item = new QTableWidgetItem;
		SetItemAttribute(item, 
						 record.value("score").toString(),
						 Qt::AlignHCenter | Qt::AlignVCenter);
		ui.scoreRankTable->setItem(row, 2, item);
	}
}

void SYAdminTrainAnalysisWindow::SetErrorInfo()
{
	if(m_trainAbilityCounter == nullptr)
		return; 

	struct Node
	{
		const SYScorePointDetailData* spdd;
		int times;
	};

	Node node;
	QList<Node> errorDataList;

	for(auto itr = m_trainAbilityCounter->m_spddCounterMap.begin(); itr != m_trainAbilityCounter->m_spddCounterMap.end(); ++itr){
		node.spdd = itr.key();
		const QString& code = node.spdd->Code();
		if(code == "0")
			continue;

		node.times = itr.value();
		auto errorDataItr = errorDataList.begin();
		for(; errorDataItr != errorDataList.end(); ++errorDataItr){
			if(errorDataItr->times < node.times){
				errorDataList.insert(errorDataItr, node);
				break;
			}
		}

		if(errorDataItr == errorDataList.end())
			errorDataList.push_back(node);
	}

	int nNode = errorDataList.size();
	ui.errorTable->setRowCount(nNode);

	QTableWidgetItem* item = nullptr;
	QString content;
	int row = 0;
	for(auto errorData : errorDataList){
		//col 1
		item = new QTableWidgetItem;
		SYScoreItemData* scoreItemData = static_cast<SYScoreItemData*>(errorData.spdd->GetParent());
		SetItemAttribute(item,
						 scoreItemData->GetScoreContent(),
						 Qt::AlignLeft | Qt::AlignVCenter);
		ui.errorTable->setItem(row, 1, item);

		//col 2
		item = new QTableWidgetItem;
		SetItemAttribute(item,
						 errorData.spdd->GetDescription(),
						 Qt::AlignLeft | Qt::AlignVCenter);
		ui.errorTable->setItem(row, 2, item);

		//col 3
		item = new QTableWidgetItem;
		SetItemAttribute(item,
						 content.setNum(errorData.times),
						 Qt::AlignLeft | Qt::AlignVCenter);
		ui.errorTable->setItem(row, 3, item);

		ui.errorTable->setRowHeight(row, 70);
		++row;
	}
}

void SYAdminTrainAnalysisWindow::showEvent(QShowEvent* showEvent)
{
	//UpdateContent();
}

bool SYAdminTrainAnalysisWindow::eventFilter(QObject* object, QEvent* event)
{
	if(event->type() == QEvent::Resize){
		if(object == m_bottomBlueLabel || object == ui.eliteFrame){
			QSize parentSize = ui.eliteFrame->size();
			QSize labelSize = m_bottomBlueLabel->size();
			m_bottomBlueLabel->move(ui.eliteFrame->x()/*(parentSize.width() - labelSize.width()) / 2*/,
									parentSize.height() - labelSize.height());

			ui.fifthLabel->repaint();
		}
	}

	return __super::eventFilter(object, event);
}
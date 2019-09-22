#include "SYDataCenterWindow.h"
#include <MxDefine.h>
#include <SYDBMgr.h>
#include <SYUserInfo.h>
#include <QValueAxis>
#include "SYTrainRecordWindow.h"
#include "SYTrainAnalysisWindow.h"

const int SYDataCenterWindow::m_finishProgressUpper = 40;

const int SYDataCenterWindow::m_trainTimesUpper = 50;

const int SYDataCenterWindow::m_trainTimeUpper = 100;

SYDataCenterWindow::SYDataCenterWindow(QWidget* parent)
	:QWidget(parent),
	m_userId(-1),
	m_trainTimesGraphView(nullptr),
	m_trainTimesChart(nullptr),
	m_trainTimesBarSet(nullptr),
	m_trainTimesBarSeries(nullptr),
	m_simulationTrainTimes(0),
	m_skillingTrainTimes(0),
	m_surgeryTrainTimes(0),
	m_totalTrainTimes(0),
	m_pieGraphView(nullptr),
	m_pieGraph(nullptr),
	m_trainTimesPieSeries(new QPieSeries),
	m_skillingTrainScorePieSeries(new QPieSeries),
	m_surgeryTrainScorePieSeries(new QPieSeries),
	m_trainRecordWindow(nullptr),
	m_skillingTrainAnalysisWindow(nullptr),
	m_surgeryTrainAnalysisWindow(nullptr)
{
	ui.setupUi(this);

	m_userId = SYUserInfo::Instance()->GetUserId();
	
	//1 条形图
	m_trainTimesBarSet = new QBarSet(QString::fromLocal8Bit("训练次数"));
	m_trainTimesBarSet->setColor(QColor(0x28, 0x9f, 0xff));
	m_trainTimesBarSet->setBorderColor(QColor(0x46, 0x52, 0x6d));	//QColor(0x28, 0x9f, 0xff) //QColor(0x31, 0x39, 0x4c)
	m_trainTimesBarSeries = new QBarSeries();
	m_trainTimesBarSeries->append(m_trainTimesBarSet);

	//creat char view
	m_trainTimesChart = new QChart();
	//m_trainTimesChart->setTitle("chart1");
	//m_trainTimesChart->addSeries(m_trainTimesBarSeries);
	//m_trainTimesChart->setMargins(QMargins(10, 0, 0, 10));
	//m_trainTimesChart->createDefaultAxes();

	m_trainTimesChart->setAnimationOptions(QChart::SeriesAnimations);
	m_trainTimesChart->legend()->setVisible(false);
	m_trainTimesChart->legend()->setAlignment(Qt::AlignBottom);
	m_trainTimesChart->legend()->setColor(QColor(40, 129, 255));
	m_trainTimesChart->setBackgroundVisible(false);

	m_trainTimesGraphView = new QChartView(ui.trainTimesGraphFrame);
	m_trainTimesGraphView->setObjectName("trainTimesGraphView");
	m_trainTimesGraphView->setChart(m_trainTimesChart);
	m_trainTimesGraphView->move(-20, 0);
	//m_trainTimesGraphView->setContentsMargins(0, 0, 0, 0);
	
	QLabel* label = new QLabel(QString::fromLocal8Bit("训练次数"), m_trainTimesGraphView);
	label->setObjectName("trainTimesLabel");
	label->move(40, 10);

	//label = new QLabel(QString::fromLocal8Bit("月份"), m_trainTimesGraphView);
	//label->setObjectName("trainMonthLabel");
	//label->move(500, 252);

	//2 饼状图
	m_pieGraph = new QChart();
	m_pieGraph->setAnimationOptions(QChart::SeriesAnimations);
	m_pieGraph->legend()->setVisible(false);
	m_pieGraph->setBackgroundVisible(false);

	m_pieGraphView = new QChartView(ui.pieGraphFrame);
	m_pieGraphView->setObjectName("pieGraphView");
	m_pieGraphView->setChart(m_pieGraph);
	m_pieGraphView->setRenderHint(QPainter::Antialiasing);

	m_trainTimesPieSeries->setHorizontalPosition(0.109f);
	m_trainTimesPieSeries->setPieSize(0.99f);
	m_trainTimesPieSeries->setHoleSize(0.5f);
	
	m_skillingTrainScorePieSeries->setHorizontalPosition(0.465f);
	m_skillingTrainScorePieSeries->setPieSize(0.99f);
	m_skillingTrainScorePieSeries->setHoleSize(0.5f);
	
	m_surgeryTrainScorePieSeries->setHorizontalPosition(0.779f);
	m_surgeryTrainScorePieSeries->setPieSize(0.99f);
	m_surgeryTrainScorePieSeries->setHoleSize(0.5f);

	setPieLabels();
	
	addTrainAnalysisWindow();

	Mx::setWidgetStyle(this, "qss:SYDataCenterWindow.qss");
}

SYDataCenterWindow::~SYDataCenterWindow()
{

}

void SYDataCenterWindow::showEvent(QShowEvent* event)
{
	updateRecords();

	updateTrainOverview();

	updateTrainTimesGraph();

	updatePieGraph();

	updateTrainAnalysisWindow();
}

void SYDataCenterWindow::setPieLabels()
{
	//pie 1
	QLabel* label = new QLabel(QString::fromLocal8Bit("训练结构"), m_pieGraphView);
	label->setObjectName("pieNameLabel1");
	label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	label->setWordWrap(true);
	label->move(120, 123);

	int x = 290, y = 63;
	int dx = 40, dy = 66;
	QLabel* pie1Slice1Icon = new QLabel(m_pieGraphView);
	pie1Slice1Icon->setObjectName("pie1Slice1Icon");
	pie1Slice1Icon->move(x, y);
	QLabel* pie1Slice1Text = new QLabel(QString::fromLocal8Bit("实物训练"), m_pieGraphView);
	pie1Slice1Text->setObjectName("pie1Slice1Text");
	pie1Slice1Text->move(x + dx, y);

	QLabel* pie1Slice2Icon = new QLabel(m_pieGraphView);
	pie1Slice2Icon->move(x, y + dy);
	pie1Slice2Icon->setObjectName("pie1Slice2Icon");
	QLabel* pie1Slice2Text = new QLabel(QString::fromLocal8Bit("技能训练"), m_pieGraphView);
	pie1Slice2Text->setObjectName("pie1Slice2Text");
	pie1Slice2Text->move(x + dx, y + dy);

	QLabel* pie1Slice3Icon = new QLabel(m_pieGraphView);
	pie1Slice3Icon->move(x, y + 2 * dy);
	pie1Slice3Icon->setObjectName("pie1Slice3Icon");
	QLabel* pie1Slice3Text = new QLabel(QString::fromLocal8Bit("手术训练"), m_pieGraphView);
	pie1Slice3Text->setObjectName("pie1Slice3Text");
	pie1Slice3Text->move(x + dx, y + 2 * dy);

	//pie 2
	label = new QLabel(QString::fromLocal8Bit("技能训练分数"), m_pieGraphView);
	label->setObjectName("pieNameLabel2");
	label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	label->setWordWrap(true);
	label->move(548, 112);

	x = 720;
	y = 43;
	dy = 50;
	QLabel* pie2Slice1Icon = new QLabel(m_pieGraphView);
	pie2Slice1Icon->setObjectName("pie2Slice1Icon");
	pie2Slice1Icon->move(x, y);
	QLabel* pie2Slice1Text = new QLabel(QString::fromLocal8Bit("90～100"), m_pieGraphView);
	pie2Slice1Text->setObjectName("pie2Slice1Text");
	pie2Slice1Text->move(x + dx, y);

	QLabel* pie2Slice2Icon = new QLabel(m_pieGraphView);
	pie2Slice2Icon->move(x, y + dy);
	pie2Slice2Icon->setObjectName("pie2Slice2Icon");
	QLabel* pie2Slice2Text = new QLabel(QString::fromLocal8Bit("80～90"), m_pieGraphView);
	pie2Slice2Text->setObjectName("pie2Slice2Text");
	pie2Slice2Text->move(x + dx, y + dy);

	QLabel* pie2Slice3Icon = new QLabel(m_pieGraphView);
	pie2Slice3Icon->move(x, y + 2 * dy);
	pie2Slice3Icon->setObjectName("pie2Slice3Icon");
	QLabel* pie2Slice3Text = new QLabel(QString::fromLocal8Bit("60～80"), m_pieGraphView);
	pie2Slice3Text->setObjectName("pie2Slice3Text");
	pie2Slice3Text->move(x + dx, y + 2 * dy);

	QLabel* pie2Slice4Icon = new QLabel(m_pieGraphView);
	pie2Slice4Icon->move(x, y + 3 * dy);
	pie2Slice4Icon->setObjectName("pie2Slice4Icon");
	QLabel* pie2Slice4Text = new QLabel(QString::fromLocal8Bit("＜60"), m_pieGraphView);
	pie2Slice4Text->setObjectName("pie2Slice4Text");
	pie2Slice4Text->move(x + dx, y + 3 * dy);

	//pie 3
	label = new QLabel(QString::fromLocal8Bit("手术训练分数"), m_pieGraphView);
	label->setObjectName("pieNameLabel3");
	label->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	label->setWordWrap(true);
	label->move(923, 112);

	x = 1095;
	y = 43;
	dy = 50;
	QLabel* pie3Slice1Icon = new QLabel(m_pieGraphView);
	pie3Slice1Icon->setObjectName("pie3Slice1Icon");
	pie3Slice1Icon->move(x, y);
	QLabel* pie3Slice1Text = new QLabel(QString::fromLocal8Bit("90～100"), m_pieGraphView);
	pie3Slice1Text->setObjectName("pie3Slice1Text");
	pie3Slice1Text->move(x + dx, y);

	QLabel* pie3Slice2Icon = new QLabel(m_pieGraphView);
	pie3Slice2Icon->move(x, y + dy);
	pie3Slice2Icon->setObjectName("pie3Slice2Icon");
	QLabel* pie3Slice2Text = new QLabel(QString::fromLocal8Bit("80～90"), m_pieGraphView);
	pie3Slice2Text->setObjectName("pie3Slice2Text");
	pie3Slice2Text->move(x + dx, y + dy);

	QLabel* pie3Slice3Icon = new QLabel(m_pieGraphView);
	pie3Slice3Icon->move(x, y + 2 * dy);
	pie3Slice3Icon->setObjectName("pie3Slice3Icon");
	QLabel* pie3Slice3Text = new QLabel(QString::fromLocal8Bit("60～80"), m_pieGraphView);
	pie3Slice3Text->setObjectName("pie3Slice3Text");
	pie3Slice3Text->move(x + dx, y + 2 * dy);

	QLabel* pie3Slice4Icon = new QLabel(m_pieGraphView);
	pie3Slice4Icon->move(x, y + 3 * dy);
	pie3Slice4Icon->setObjectName("pie3Slice4Icon");
	QLabel* pie3Slice4Text = new QLabel(QString::fromLocal8Bit("＜60"), m_pieGraphView);
	pie3Slice4Text->setObjectName("pie3Slice4Text");
	pie3Slice4Text->move(x + dx, y + 3 * dy);
}

void SYDataCenterWindow::updateRecords()
{
	m_allRecords.clear();
	m_skillingTrainRecords.clear();
	m_surgeryTrainRecords.clear();

	SYDBMgr::Instance()->QueryAllScoreRecordOrderByDateDescend(m_userId,m_allRecords);

	for(const auto& record : m_allRecords){
		int trainType = record.value("trainTypeCode").toInt();
		if(trainType == 1)
			m_skillingTrainRecords.push_back(record);
		else if(trainType == 2)
			m_surgeryTrainRecords.push_back(record);
	}
}

void SYDataCenterWindow::updateTrainOverview()
{

	int n = SYDBMgr::Instance()->QueryNumberOfCompletedTrain(m_userId);
	int curLength,maxLength;

	//1 训练完成度
	if(n < 0)
		n = 0;
	ui.finishProgressTextLabelRight->setText(QString("%1/%2").arg(n).arg(m_finishProgressUpper));
	maxLength = ui.finishProgressFrame->width();
	curLength = 1.0f * n / m_finishProgressUpper * maxLength;
	if(curLength > maxLength)
		curLength = maxLength;
	ui.finishProgressGraphLabel->setFixedWidth(curLength);

	//2 训练次数 & 训练时间
	m_simulationTrainTimes = 0;
	m_skillingTrainTimes = 0;
	m_surgeryTrainTimes = 0;
	m_totalTrainTimes = 0;
	int totalTime = 0;

	SYDBMgr::Instance()->QueryBasicTrainInfo(m_userId, totalTime, m_simulationTrainTimes, m_skillingTrainTimes, m_surgeryTrainTimes);
	m_totalTrainTimes = m_simulationTrainTimes + m_skillingTrainTimes + m_surgeryTrainTimes;
	//times
	ui.trainTimesTextLabelRight->setText(QString("%1/%2").arg(m_totalTrainTimes).arg(m_trainTimesUpper));
	maxLength = ui.trainTimesFrame->width();
	curLength = 1.0f * m_totalTrainTimes / m_trainTimesUpper * maxLength;
	if(curLength > maxLength)
		curLength = maxLength;
	ui.trainTimesGraphLabel->setFixedWidth(curLength);

	//time
	int hour = totalTime / 3600;
	ui.trainTimeTextLabelRight->setText(QString::fromLocal8Bit("%1/%2小时").arg(hour).arg(m_trainTimeUpper));
	maxLength = ui.trainTimeFrame->width();
	curLength = 1.0f * hour / m_trainTimeUpper * maxLength;
	if(curLength > maxLength)
		curLength = maxLength;
	ui.trainTimeGraphLabel->setFixedWidth(curLength);

	//detail times
	ui.simulationTimesLabel->setText(QString::fromLocal8Bit("%1次").arg(m_simulationTrainTimes));
	ui.skillingTrainTimesLabel->setText(QString::fromLocal8Bit("%1次").arg(m_skillingTrainTimes));
	ui.surgeryTrainTimesLabel->setText(QString::fromLocal8Bit("%1次").arg(m_surgeryTrainTimes));
}

void SYDataCenterWindow::updateTrainTimesGraph()
{
	//remove
	m_trainTimesChart->removeSeries(m_trainTimesBarSeries);

	const int nCategory = 6;
	QDateTime currentTime = QDateTime::currentDateTime();// .addDays(33);
	QString curYear = currentTime.toString("yyyy");
	QString curMonth = currentTime.toString("MM");
	int cm, cy;

	std::vector<int> trainTimesInfo(nCategory,0);
	int index = nCategory - 1;
	int count = 0;

	std::size_t nRecord = m_allRecords.size();

	if(nRecord > 0){
		QDateTime dateTime;
		QString month, year;
		int m, y;
		cm = curMonth.toInt();
		cy = curYear.toInt();

		for(std::size_t i = 0; i < nRecord; ++i){
			QSqlRecord& record = m_allRecords[i];
			dateTime = record.value("beginTime").toDateTime();
			//string value
			month = dateTime.toString("MM");
			year = dateTime.toString("yyyy");
			//int value
			m = month.toInt();
			y = year.toInt();

			if(y == cy){
				int d = cm - m;
				Q_ASSERT(d >= 0 && "d >= 0");

				if(d == 0){
					++count;
				}
				else{
					trainTimesInfo[index] = count;
					count = 0;
					
					--index;
					if(index < 0)
						break;

					--i;
					--cm;
					if(cm == 0){
						cm = 12;
						--cy;
					}
				}
			}
			else
			{
				//1
				trainTimesInfo[index] = count;
				count = 0;
				--index;
				if(index < 0)
					break;

				//2
				--i;
				--cm;
				if(cm == 0){
					cm = 12;
					--cy;
				}
			}
		}

		//
		if(index >= 0)
			trainTimesInfo[index] = count;
	}
	

	//test
 	//for(int i = 0; i < nCategory; ++i)
 	//	trainTimesInfo[i] = i + 1;
	
	//set bar data
	int n = m_trainTimesBarSet->count();
	if(n > 0)
		m_trainTimesBarSet->remove(0, n);
	for(auto t : trainTimesInfo){
		m_trainTimesBarSet->append(t);
	}

	//add
	m_trainTimesChart->addSeries(m_trainTimesBarSeries);
	


	//set x-axis
	auto* xAxis = new QBarCategoryAxis();
	xAxis->setLabelsColor(QColor(0xb7, 0xc5, 0xd8));
	xAxis->setLinePenColor(QColor(0x46, 0x52, 0x6d));
	QFont labelFont = xAxis->labelsFont();
	labelFont.setPixelSize(16);
	xAxis->setLabelsFont(labelFont);

	cm = curMonth.toInt();
	cy = curYear.toInt();

	cm -= 5;
	if(cm <= 0){
		cm += 12;
		--cy;
	}
	for(int i = 0; i < nCategory; ++i){
		//xAxis->append(QString::asprintf("%d/%2d", cy, cm));
		xAxis->append(QString().setNum(cm) + QString::fromLocal8Bit("月"));

		++cm;
		if(cm == 13){
			cm = 1;
			++cy;
		}
	}

	xAxis->setGridLineColor(QColor(0x31, 0x39, 0x4c));
	m_trainTimesChart->setAxisX(xAxis,m_trainTimesBarSeries);

	//set y-axis
	int min = 0;
	int max = 0;

	for(auto v : trainTimesInfo){
		if(v > max)
			max = v;
	}

	//if(max < 10)
	//	max = 10;

	QValueAxis* yAxis = new QValueAxis();
	yAxis->setLabelsColor(QColor(0xb7, 0xc5, 0xd8));
	yAxis->setLinePenColor(QColor(0x31, 0x39, 0x4c));
	yAxis->setGridLineColor(QColor(0x46, 0x52, 0x6d));
	yAxis->setLabelsFont(labelFont);
	yAxis->setLabelFormat(QString("%d"));
	yAxis->setRange(min, max);
	n = max - min + 1;
	if(n > 4)
		n = 4;
	yAxis->setTickCount(n);
	m_trainTimesChart->setAxisY(yAxis, m_trainTimesBarSeries);
}

void SYDataCenterWindow::updatePieGraph()
{
	const int fontSize = 16;//pixel
	static const QColor backgroundColor = QColor(0x31, 0x39, 0x4c);
	static const QColor emptyCircleBorderColor = QColor(0x46, 0x52, 0x6d);
	static const std::vector<QColor> pieColors1 = {QColor(0x6e, 0x94, 0xff), QColor(0x11, 0xd1, 0x76), QColor(0xff, 0xc9, 0x46)};
	static const std::vector<QColor> pieColors2 = {QColor(0xff, 0xc9, 0x46), QColor(0x11, 0xd1, 0x76), QColor(0x5d, 0xb5, 0xff), QColor(0x6e, 0x94, 0xff)};

	//1 训练次数饼状图
	m_trainTimesPieSeries->clear();
	if(m_totalTrainTimes > 0){
		int trainTimes[3] = {m_simulationTrainTimes, m_skillingTrainTimes, m_surgeryTrainTimes};
		int percentages[3] = {0};

		for(int i = 0; i < 3; ++i){
			percentages[i] = 100 * trainTimes[i] / m_totalTrainTimes;
		}

		int d = 100 - percentages[0] - percentages[1] - percentages[2];
		if(d > 0){
			int index = 0;
			int minPercentage = percentages[0];

			//find first
			for(int i = 0; i < 3; ++i){
				if(trainTimes[i] > 0){
					index = i;
					minPercentage = percentages[i];
					break;
				}
			}

			for(int i = 1; i < 3; ++i){
				if(trainTimes[i] > 0 && minPercentage > percentages[i]){
					minPercentage = percentages[i];
					index = i;
				}
			}

			if(trainTimes[index] > 0)
				percentages[index] += d;
			else{
				for(int i = 0; i < 3; ++i){
					if(trainTimes[i] > 0){
						percentages[i] += d;
						break;
					}
				}
			}
		}

		//add pie slices
 		for(int i = 0; i < 3; ++i){
 			if(trainTimes[i] > 0){
 				QPieSlice* pieSlice = new QPieSlice();
 				pieSlice->setLabelVisible(true);
 				pieSlice->setLabel(QString::fromLocal8Bit("%1次").arg(trainTimes[i]));
 				pieSlice->setLabelColor(QColor(0xf9, 0xf9, 0xf9));
 				pieSlice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
 				QFont font = pieSlice->labelFont();
 				font.setWeight(QFont::Bold);
 				font.setPixelSize(fontSize);
 				pieSlice->setLabelFont(font);
 				pieSlice->setValue(percentages[i]);
 				pieSlice->setColor(pieColors1[i]);
 				pieSlice->setBorderColor(pieColors1[i]);
 				*m_trainTimesPieSeries << pieSlice;
 			}
 		}

		m_pieGraph->removeSeries(m_trainTimesPieSeries);
		m_pieGraph->addSeries(m_trainTimesPieSeries);
	}
	else{
		//无训练次数时画一个空心圆
		QPieSlice* pieSlice = new QPieSlice();
		//pieSlice->setLabelVisible(true);
		//pieSlice->setLabel(QString("%1%").arg(percentages[i]));
		//pieSlice->setLabelColor(QColor(0xf9, 0xf9, 0xf9));
		//pieSlice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
		//QFont font = pieSlice->labelFont();
		//font.setWeight(QFont::Bold);
		//font.setPixelSize(fontSize);
		//pieSlice->setLabelFont(font);
		pieSlice->setValue(100);
		pieSlice->setColor(emptyCircleBorderColor);
		pieSlice->setBorderColor(emptyCircleBorderColor);
		*m_trainTimesPieSeries << pieSlice;

		m_pieGraph->removeSeries(m_trainTimesPieSeries);
		m_pieGraph->addSeries(m_trainTimesPieSeries);
	}

	//2 技能训练分数饼状图 & 手术训练分数饼状图
	enum ScoreGrade{
		SG_0_60 = 0,
		SG_60_80,
		SG_80_90,
		SG_90_100
	};

	struct PieSliceInfo{
		int scoreType;		//分数类型：0技能训练分数；1：手术训练分数
		ScoreGrade scoreGrade;
		int count;
		int percentage;
	};

	m_skillingTrainScorePieSeries->clear();
	m_surgeryTrainScorePieSeries->clear();
	std::map<ScoreGrade, PieSliceInfo> sktPieSliceInfoMap;//技能训练
	std::map<ScoreGrade, PieSliceInfo> sutPieSliceInfoMap;//手术训练
	PieSliceInfo temp = {0, SG_0_60, 0, 0};
	bool ok;

	for(int i = 0; i < 4; ++i){
		ScoreGrade scoreGrade = static_cast<ScoreGrade>(i);
		temp.scoreGrade = scoreGrade;
		//1
		temp.scoreType = 0;
		sktPieSliceInfoMap.insert(std::make_pair(scoreGrade, temp));
		//2
		temp.scoreType = 1;
		sutPieSliceInfoMap.insert(std::make_pair(scoreGrade, temp));
	}

	//统计不同等级包含的分数个数
	int nScore1 = 0;
	int nScore2 = 0;
	for(const auto& record : m_allRecords){
		int scoreType = record.value("trainTypeCode").toInt(&ok);
		if(!ok)
			continue;
		
		std::map<ScoreGrade, PieSliceInfo>* mapPointer = nullptr;
		if(scoreType == 1){	//技能训练
			mapPointer = &sktPieSliceInfoMap;
			++nScore1;
		}
		else if(scoreType == 2){	//手术训练
			mapPointer = &sutPieSliceInfoMap;
			++nScore2;
		}
		else
			continue;

		int score = record.value("score").toInt(&ok);
		ScoreGrade scoreGrade;
		if(!ok) score = 0;

		if(score < 60)
			scoreGrade = SG_0_60;
		else if(score < 80)
			scoreGrade = SG_60_80;
		else if(score < 90)
			scoreGrade = SG_80_90;
		else 
			scoreGrade = SG_90_100;


		auto itr = mapPointer->find(scoreGrade);
		auto& pieSliceInfo = itr->second;
		pieSliceInfo.count += 1;

	}

	//计算百分比
	auto ComputPercentage = [](std::map<ScoreGrade, PieSliceInfo>& pieSliceInfoMap, int nScore){
		if(nScore <= 0)
			return;

		int d = 100;
		for(auto& itr : pieSliceInfoMap){
			PieSliceInfo& pieSliceInfo = itr.second;
			pieSliceInfo.percentage = 100 * pieSliceInfo.count / nScore;
			d -= pieSliceInfo.percentage;
		}

		if(d > 0){
			auto itr = pieSliceInfoMap.begin();
			int grade = -1;
			int minPercentage = 100;

			//find fist
			while(itr != pieSliceInfoMap.end()){
				const auto& pieSliceInfo = itr->second;
				if(pieSliceInfo.count > 0 && pieSliceInfo.percentage < minPercentage){
					grade = itr->first;
					minPercentage = pieSliceInfo.percentage;
				}

				++itr;
			}

			Q_ASSERT(grade >= 0 && "grade >= 0");

			ScoreGrade scoreGrade = static_cast<ScoreGrade>(grade);
			pieSliceInfoMap[scoreGrade].percentage += d;
		}
	};

	ComputPercentage(sktPieSliceInfoMap, nScore1);
	ComputPercentage(sutPieSliceInfoMap, nScore2);

	//draw pie slice
	std::map<ScoreGrade, PieSliceInfo>* mapPointers[2] = {&sktPieSliceInfoMap, &sutPieSliceInfoMap};
	QPieSeries* pieSeriess[2] = {m_skillingTrainScorePieSeries, m_surgeryTrainScorePieSeries};
	int nScores[2] = {nScore1, nScore2};

	for(int i = 0; i < 2; ++i){
		pieSeriess[i]->clear();

		auto itr = mapPointers[i]->begin();
		while(itr != mapPointers[i]->end()){
			const auto& pieSliceInfo = itr->second;
			if(pieSliceInfo.count > 0){
				QPieSlice* pieSlice = new QPieSlice;
				pieSlice->setValue(pieSliceInfo.percentage);
				pieSlice->setLabelVisible(true);
				pieSlice->setLabel(QString("%1%").arg(pieSliceInfo.percentage));
				pieSlice->setLabelColor(QColor(0xf9, 0xf9, 0xf9));
				pieSlice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
				QFont font = pieSlice->labelFont();
				font.setWeight(QFont::Bold);
				font.setPixelSize(fontSize);
				pieSlice->setLabelFont(font);
				pieSlice->setColor(pieColors2[pieSliceInfo.scoreGrade]);
				pieSlice->setBorderColor(pieColors2[pieSliceInfo.scoreGrade]);
				pieSeriess[i]->append(pieSlice);
			}

			++itr;
		}

		//无训练次数时画一个空心圆
		if(nScores[i] == 0){
			QPieSlice* pieSlice = new QPieSlice();
			//pieSlice->setLabelVisible(true);
			//pieSlice->setLabel(QString("%1%").arg(percentages[i]));
			//pieSlice->setLabelColor(QColor(0xf9, 0xf9, 0xf9));
			//pieSlice->setLabelPosition(QPieSlice::LabelInsideHorizontal);
			//QFont font = pieSlice->labelFont();
			//font.setWeight(QFont::Bold);
			//font.setPixelSize(fontSize);
			//pieSlice->setLabelFont(font);
			pieSlice->setValue(100);
			pieSlice->setColor(emptyCircleBorderColor);
			pieSlice->setBorderColor(emptyCircleBorderColor);
			pieSeriess[i]->append(pieSlice);
		}

		m_pieGraph->removeSeries(pieSeriess[i]);
		m_pieGraph->addSeries(pieSeriess[i]);
	}
}

void SYDataCenterWindow::addTrainAnalysisWindow()
{
	if(m_trainRecordWindow)
		return;

	//remove old widget
	int n = ui.stackedWidget->count();
	while(n > 0){
		QWidget* oldWidget = ui.stackedWidget->widget(0);
		ui.stackedWidget->removeWidget(oldWidget);
		delete oldWidget;
		--n;
	}

	//add new widget. i.e SYTrainRecordWindow
	m_trainRecordWindow = new SYTrainRecordWindow(m_allRecords);
	ui.stackedWidget->addWidget(m_trainRecordWindow);

	m_skillingTrainAnalysisWindow = new SYTrainAnalysisWindow;
	ui.stackedWidget->addWidget(m_skillingTrainAnalysisWindow);

	m_surgeryTrainAnalysisWindow = new SYTrainAnalysisWindow;
	ui.stackedWidget->addWidget(m_surgeryTrainAnalysisWindow);


	ui.stackedWidget->setCurrentIndex(0);
}

void SYDataCenterWindow::updateTrainAnalysisWindow()
{
	if(m_trainRecordWindow)
		m_trainRecordWindow->updateTableWidget();
}

void SYDataCenterWindow::on_trainRecordBtn_clicked()
{
	ui.trainRecordBtn->setChecked(true);
	ui.skillingTrainAnalysisBtn->setChecked(false);
	ui.surgeryTrainAnalisisBtn->setChecked(false);

	ui.stackedWidget->setCurrentWidget(m_trainRecordWindow);
}

void SYDataCenterWindow::on_skillingTrainAnalysisBtn_clicked()
{
	ui.trainRecordBtn->setChecked(false);
	ui.skillingTrainAnalysisBtn->setChecked(true);
	ui.surgeryTrainAnalisisBtn->setChecked(false);

	m_skillingTrainAnalysisWindow->updateContent(m_userId, 1, m_skillingTrainRecords);
	ui.stackedWidget->setCurrentWidget(m_skillingTrainAnalysisWindow);
}

void SYDataCenterWindow::on_surgeryTrainAnalisisBtn_clicked()
{
	ui.trainRecordBtn->setChecked(false);
	ui.skillingTrainAnalysisBtn->setChecked(false);
	ui.surgeryTrainAnalisisBtn->setChecked(true);

	m_surgeryTrainAnalysisWindow->updateContent(m_userId, 2, m_surgeryTrainRecords);
	ui.stackedWidget->setCurrentWidget(m_surgeryTrainAnalysisWindow);
}
#include "SYAdminTrainStatisticWindow.h"
#include <MxDefine.h>
#include <SYDBMgr.h>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include "SYStringTable.h"

SYAdminTrainStatisticWindow::SYAdminTrainStatisticWindow(QWidget* parent)
	:QWidget(parent),
	m_trainType(TT_EntityTrain)
{
	ui.setupUi(this);

	for(int i = 0; i < 2; ++i){
		m_charts[i] = new QChart();
		//m_charts[i]->setTitle("1111111111111");
		m_charts[i]->legend()->setLabelColor(QColor(255, 255, 255));
		//m_charts[i]->legend()->setBorderColor(QColor(49, 57, 76));
		m_charts[i]->setBackgroundVisible(false);
		m_charts[i]->setAnimationOptions(QChart::SeriesAnimations);

		m_chartViews[i] = new QChartView(ui.dataAnalysisFrame);
		m_chartViews[i]->setObjectName("chartView");
		m_chartViews[i]->setChart(m_charts[i]);

		m_barSets[i] = new QBarSet("title");
		QFont font = m_barSets[i]->labelFont();
		font.setPixelSize(20);
		m_barSets[i]->setLabelFont(font);
		m_barSets[i]->setLabelColor(QColor(255, 255, 255));
		m_barSets[i]->setColor(QColor(127, 127, 127));
		m_barSets[i]->setBorderColor(QColor(0x46, 0x52, 0x6d));

		m_barSeries[i] = new QBarSeries;
		m_barSeries[i]->append(m_barSets[i]);
	}
	
	//add chart view to layout of ui.dataAnalysisFrame
	QHBoxLayout* hLayout = new QHBoxLayout;
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout->addWidget(m_chartViews[0]);
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout->addWidget(m_chartViews[1]);
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

	ui.dataAnalysisFrame->setLayout(hLayout);

	//vertical line
	//for(int i = 0; i < 2; ++i){
		//QLabel* label = new QLabel(m_chartViews[i]);
		//label->setObjectName("verticalLine");
		//label->move(65, 90);
	//}

	Mx::setWidgetStyle(this, "qss:SYAdminTrainStatisticWindow.qss");
}

SYAdminTrainStatisticWindow::~SYAdminTrainStatisticWindow()
{

}

void SYAdminTrainStatisticWindow::SetTrainType(TrainType type)
{
	m_trainType = type;
}

void SYAdminTrainStatisticWindow::LoadData()
{
	//1 设置训练总数
	QString content;
	int times = 0;
	int totalTime = 0;
	int nPeople = 0;

	SYDBMgr::Instance()->QueryBasicTrainInfo(m_trainType, times, totalTime, nPeople);

	ui.trainTimesLabel1->setText(QString().setNum(times));
	ui.trainHourLabel1->setText(QString().setNum(totalTime / 3600));
	ui.trainNumberOfPeopleLabel1->setText(QString().setNum(nPeople));

	//2 charts
	if(m_trainType == TT_SurgeryTrain){
		UpdateChart2();
	}
	else{
		UpdateChart1();
	}
	
}

void SYAdminTrainStatisticWindow::UpdateChart1()
{
	std::vector<QSqlRecord> records;
	m_charts[0]->removeAllSeries();

	SYDBMgr::Instance()->QueryTrainDataDistribution(m_trainType, records);

	int n = m_barSets[0]->count();
	if(n > 0)
		m_barSets[0]->remove(0, n);

	int minTimes = 0, maxTimes = 0;
	int minTotalTime = 0, maxTotalTime = 0;
	std::vector<int> times;
	std::vector<int> totalTime;
	std::vector<QString> trainNames;
	QString trainName;

	for(const auto& record : records){
		//left
		int ts = record.value("times").toInt();
		//m_barSets[0]->append(times);
		times.push_back(ts);

		//right
		int tt = record.value("totalTime").toInt();
		//m_barSets[1]->append(totalTime);
		totalTime.push_back(tt);

		trainName = record.value("trainName").toString();
		trainNames.push_back(trainName);

		//max number
		if(ts > maxTimes)
			maxTimes = ts;

		if(tt > maxTotalTime)
			maxTotalTime = tt;
	}

	SetAxisData(times, totalTime, trainNames,
				minTimes, maxTimes,
				minTotalTime, maxTotalTime);
}

void SYAdminTrainStatisticWindow::UpdateChart2()
{
	std::vector<QSqlRecord> records;
	m_charts[0]->removeAllSeries();

	//query records
	SYDBMgr::Instance()->QueryAllScoreRecordOrderByDateDescend(m_trainType, records);

	const int nCategory = 6;
	QDateTime currentTime = QDateTime::currentDateTime();
	QString curYear = currentTime.toString("yyyy");
	QString curMonth = currentTime.toString("MM");
	int cm, cy;

	std::vector<int> trainTimesInfo(nCategory, 0);
	std::vector<int> trainTotalTime(nCategory, 0);
	//std::vector<int> recordedMonths(nCategory, 0);		//数据所属的月份
	int index = nCategory - 1;
	int count = 0;
	int totalTime = 0;

	std::size_t nRecord = records.size();
	if(nRecord > 0){
		QDateTime dateTime;
		QString month, year;
		int m, y;
		cm = curMonth.toInt();
		cy = curYear.toInt();

		for(std::size_t i = 0; i < nRecord; ++i){
			QSqlRecord& record = records[i];
			dateTime = record.value("beginTime").toDateTime();
			int costTime = record.value("costTime").toInt();

			//string value
			month = dateTime.toString("MM");
			year = dateTime.toString("yyyy");

			//int value
			m = month.toInt();
			y = year.toInt();

			//if(recordedMonths[index] == 0)
			//	recordedMonths[index] = cm;

			if(y == cy){
				int d = cm - m;
				Q_ASSERT(d >= 0 && "d >= 0");

				if(d == 0){
					++count;
					totalTime += costTime;
				}
				else{
					trainTimesInfo[index] = count;
					count = 0;

					trainTotalTime[index] = totalTime;
					totalTime = 0;


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

				trainTotalTime[index] = totalTime;
				totalTime = 0;

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
		if(index >= 0){
			trainTimesInfo[index] = count;
			trainTotalTime[index] = totalTime;
		}
	}


	std::vector<QString> barLabels;

	cm = curMonth.toInt();
	cy = curYear.toInt();

	cm -= 5;
	if(cm <= 0){
		cm += 12;
		--cy;
	}

	for(int i = 0; i < nCategory; ++i){
		//xAxis->append(QString::asprintf("%d/%2d", cy, cm));
		barLabels.push_back(QString().setNum(cm) + QString::fromLocal8Bit("月"));

		++cm;
		if(cm == 13){
			cm = 1;
			++cy;
		}
	}

	//set y-axis
	int min1 = 0,min2 = 0;
	int max1 = 0,max2 = 0;

	for(auto v : trainTimesInfo){
		if(v > max1)
			max1 = v;
	}

	for(auto& v : trainTotalTime){
		//v /= 3600;
		if(v > max2)
			max2 = v;
	}

	SetAxisData(trainTimesInfo, trainTotalTime, barLabels,
				min1, max1,
				min2, max2,
				true);
	
}

void SYAdminTrainStatisticWindow::SetAxisData(const std::vector<int>& timesDatas,const std::vector<int>& totalTimeDatas,const std::vector<QString>& barLabels,
											  int axisY1RangLow,int axisY1RangUp,
											  int axisY2RangLow,int axisY2RangUp,
											  bool enforceBarLabels)
{
	//remove old data
	for(int i = 0; i < 2; ++i){
		m_charts[i]->removeAllSeries();
		int n = m_barSets[i]->count();
		if(n > 0)
			m_barSets[i]->remove(0, n);
	}
	
	//
	const std::size_t maxColumnIndex = 5;
	const std::size_t n = std::min(timesDatas.size(), maxColumnIndex);
	QBarCategoryAxis* xAxis1 = new QBarCategoryAxis;
	QBarCategoryAxis* xAxis2 = new QBarCategoryAxis;

	for(std::size_t i = 0; i < n; ++i){
		//left chart
		m_barSets[0]->append(timesDatas[i]);
		m_barSets[0]->setColor(QColor(40, 159, 255));
		xAxis1->append(barLabels[i]);
		//xAxis1->append(QString().setNum(i));

		//right chart
		m_barSets[1]->append(totalTimeDatas[i]);
		m_barSets[1]->setColor(QColor(5, 193, 98));
		xAxis2->append(barLabels[i]);
		//xAxis2->append(QString().setNum(i));
	}

	//last one bar
	int times = 0, totalTime = 0;
	const int d = timesDatas.size() - maxColumnIndex;
	if(d){
		if(d < 0){
			for(std::size_t i = timesDatas.size(); i < maxColumnIndex; ++i){
				//left chart
				m_barSets[0]->append(0);
				//xAxis1->append("");
				//xAxis1->append(SYStringTable::GetString(SYStringTable::STR_OTHER) + QString().setNum(i));
				QString content;
				for(std::size_t k = 0; k < i; k++)	//避免有重复的列，而Qt会认为是一列，造成bar不能和文字对齐的问题
					content.append("  ");
				xAxis1->append(content);

				//right chart
				m_barSets[1]->append(0);
				//xAxis2->append(" -");
				//xAxis2->append(SYStringTable::GetString(SYStringTable::STR_OTHER) + QString().setNum(i));
				xAxis2->append(content);
			}
		}
		else if(d > 0){
			for(std::size_t i = n; i < timesDatas.size(); ++i){
				times += timesDatas[i];
				totalTime += totalTimeDatas[i];
			}
		}
	}

	//left chart & right chart
	m_barSets[0]->append(times);
	m_barSets[1]->append(totalTime);

	if(enforceBarLabels){
		QString lastBarLabel;
		if(d > 0)
			lastBarLabel = barLabels[maxColumnIndex];
		xAxis1->append(lastBarLabel);
		xAxis2->append(lastBarLabel);
	}
	else{
		if(times > 0)
			xAxis1->append(SYStringTable::GetString(SYStringTable::STR_OTHER));
		else
			xAxis1->append("");

		if(totalTime > 0)
			xAxis2->append(SYStringTable::GetString(SYStringTable::STR_OTHER));
		else
			xAxis2->append("");
	}

	//m_barSeries->setBarWidth(0.1);
	m_charts[0]->setAxisX(xAxis1, m_barSeries[0]);
	m_charts[1]->setAxisX(xAxis2, m_barSeries[1]);

	//set y axis
	QBarCategoryAxis* xAxiss[2] = {xAxis1, xAxis2};
	int mins[2] = {axisY1RangLow, axisY2RangLow};
	int maxs[2] = {axisY1RangUp,axisY2RangUp};
	for(int i = 0; i < 2; ++i){
		QValueAxis* yAxis = new QValueAxis();
		//yAxis->setLabelsFont(labelFont);
		yAxis->setLabelFormat(QString::fromLocal8Bit("%d"));
		yAxis->setRange(mins[i], maxs[i]);
		int d = maxs[i] - mins[i] + 1;
		if(d > 4)
			d = 4;
		yAxis->setTickCount(d);

		m_charts[i]->setAxisY(yAxis, m_barSeries[i]);
		m_charts[i]->addSeries(m_barSeries[i]);

		SetAxisColor(xAxiss[i], yAxis);
	}
}

void SYAdminTrainStatisticWindow::SetAxisColor(QBarCategoryAxis* xAxis, QValueAxis* yAxis)
{
	//x axis
	xAxis->setLabelsColor(QColor(0xb7, 0xc5, 0xd8));
	xAxis->setLinePenColor(QColor(0x46, 0x52, 0x6d));
	xAxis->setGridLineColor(QColor(0x31, 0x39, 0x4c));
	QFont labelFont = xAxis->labelsFont();
	labelFont.setPixelSize(16);
	xAxis->setLabelsFont(labelFont);

	//y axis
	yAxis->setLabelsColor(QColor(0xb7, 0xc5, 0xd8));
	yAxis->setLinePenColor(QColor(0x31, 0x39, 0x4c));
	yAxis->setGridLineColor(QColor(0x46, 0x52, 0x6d));
}

void SYAdminTrainStatisticWindow::SetChartTitle(const QString& leftTitle, const QString& rightTitle)
{
	if(m_barSets[0]) m_barSets[0]->setLabel(leftTitle);
	if(m_barSets[1]) m_barSets[1]->setLabel(rightTitle);
}
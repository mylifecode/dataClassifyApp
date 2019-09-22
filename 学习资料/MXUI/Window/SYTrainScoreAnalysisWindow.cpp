#include "SYTrainScoreAnalysisWindow.h"
#include <SYDBMgr.h>
#include <MxDefine.h>

SYTrainScoreAnalysisWindow::SYTrainScoreAnalysisWindow(QWidget* parent)
	:QWidget(parent)
{
	ui.setupUi(this);

	ui.trainTimesDescLabel->setVisible(false);
	ui.goodRateDescLabel->setVisible(false);

	Mx::setWidgetStyle(this, "qss:SYTrainScoreAnalysisWindow.qss");
}

SYTrainScoreAnalysisWindow::~SYTrainScoreAnalysisWindow()
{

}

void SYTrainScoreAnalysisWindow::updateContent(int userId, int trainTypeCode,const std::vector<QSqlRecord>& records)
{
	std::size_t nRecord = records.size();
	
	//1 训练次数
	ui.trainTimesLabel->setText(QString::fromLocal8Bit("%1次").arg(nRecord));

	//2 排名
	std::vector<QSqlRecord> allUserTrainTimesRecords;
	SYDBMgr::Instance()->QueryAllUserTrainTimesOrderByDescend(trainTypeCode, allUserTrainTimesRecords);
	
	int ranking = -1;
	for(std::size_t i = 0; i < allUserTrainTimesRecords.size(); ++i){
		const QSqlRecord& record = allUserTrainTimesRecords[i];
		int id = record.value("userId").toInt();
		int times = record.value("times").toInt();
		if(id == userId){
			ranking = i + 1;
		}
	}

	ui.rankingLabel->setText("-");
	ui.rankingDescriptionLabel->clear();
	if(ranking != -1){
		ui.rankingLabel->setText(QString::fromLocal8Bit("第%1名").arg(ranking));
		//
	}

	//3 最高分、最低分
	int minScore = -1;
	int maxScore = -1;
	int sum = 0;
	int passCount = 0;	//及格个数
	int goodCount = 0;	//优秀个数

	if(nRecord > 0){
		for(const auto& record : records){
			int score = record.value("score").toInt();
			if(minScore == -1){
				minScore = score;
				maxScore = score;
			}
			else{
				if(maxScore < score)
					maxScore = score;
				else if(minScore > score)
					minScore = score;
			}

			//
			sum += score;

			//
			if(score >= 80){
				goodCount++;
				passCount++;
			}
			else if(score >= 60)
				passCount++;
		}

		ui.maxScoreLabel->setText(QString::fromLocal8Bit("%1分").arg(maxScore));
		ui.minScoreLabel->setText(QString::fromLocal8Bit("%1分").arg(minScore));
		ui.avgScoreLabel->setText(QString::fromLocal8Bit("%1分").arg(sum / nRecord));
		ui.passRateLabel->setText(QString::fromLocal8Bit("%1%").arg(passCount * 100 / nRecord));
		ui.goodRateLabel->setText(QString::fromLocal8Bit("%1%").arg(goodCount * 100 / nRecord));
	}
	else{
		ui.maxScoreLabel->clear();
		ui.minScoreLabel->clear();
		ui.avgScoreLabel->clear();
		ui.passRateLabel->clear();
		ui.goodRateLabel->clear();
		ui.goodRateDescLabel->clear();
	}

	//4 最近训练
	QLabel* trainLabels[3] = {ui.recentTrainLabel1, ui.recentTrainLabel2, ui.recentTrainLabel3};
	for(auto* label : trainLabels)
		label->setVisible(false);

	for(std::size_t i = 0; (i < 3) && (i < nRecord); ++i){
		const auto& record = records[i];
		int score = record.value("score").toInt();
		trainLabels[i]->setText(record.value("trainName").toString() + QString::fromLocal8Bit("：%1分").arg(score));
		trainLabels[i]->setVisible(true);
	}

	ui.verticalLineLabel1->setVisible(false);
	ui.verticalLineLabel2->setVisible(false);
	if(nRecord >= 3){
		ui.verticalLineLabel1->setVisible(true);
		ui.verticalLineLabel2->setVisible(true);
	}
	else if(nRecord >= 2){
		ui.verticalLineLabel1->setVisible(true);
	}
	
}
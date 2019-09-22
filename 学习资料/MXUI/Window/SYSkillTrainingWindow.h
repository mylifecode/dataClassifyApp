#pragma once
#include <QWidget>
#include "MxDefine.h"
#include<QPair>
#include<QVector>
//#include "ui_SkillTrainingWindow.h"
#include "ui_SYSkillingTrainWindow.h"
#include <qt5/poppler-qt5.h>
class RbMoviePlayer;

class SYSkillTrainingWindow :	public QWidget
{
	Q_OBJECT
public:
	SYSkillTrainingWindow(QWidget* parent = nullptr);
	
	~SYSkillTrainingWindow();
	void SetAttributeParam();
	void SetItemStyle();

signals:
	void showNextWindow(WindowType type);
	
public slots:
    void textAreaChanged();

	void openUrl(QString url);

	void openVideo(QString url);

	void onButtonClicked();

	void on_knowledgeLibBtn_clicked();

	void on_dataCenterBtn_clicked();

	void on_answerBtn_clicked();

	void on_personCenterBtn_clicked();

	void on_startTrainBtn_clicked();

	void onTreeWidgetItemClicked(QTreeWidgetItem* item, int column);

	void on_playBtn_clicked();

	void setVideoInfo();

protected:
	void showEvent(QShowEvent* event);
	
private:
	void LoadDocuments(const QString& fileName);

	RbMoviePlayer * m_MoviePlayer;

	QVBoxLayout * m_imageLayout;

	Poppler::Document * m_pDocument;

	QString m_CurrVideoFile;
	QString m_CurrTrainShowName;
	QString m_CurrTrainGlobalName;
	QString m_CurrTrainCaseFile;
	QString m_CurrTrainCode;

	//���òü����
	unsigned int m_cullLeftWidth;			//���ü���ȣ�default��0
	unsigned int m_cullRightWidth;			//�Ҳ�ü���ȣ�default��0
	unsigned int m_cullUpHeight;			//�ϲ�ü��߶ȣ�default��0
	unsigned int m_cullDownHeight;			//�²�ü��߶ȣ�default��0

	int m_lastId;
	QPushButton* m_preButton;

	Ui::SYSkillingTrainWindow ui;
};


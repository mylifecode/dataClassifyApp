#ifndef _SYEXAMRECORDITEM_
#define _SYEXAMRECORDITEM_

#include "ui_SYExamRecordItem.h"
#include "ui_SYExamPaperItem.h"

class SYExamRecordItem : public QWidget
{
	Q_OBJECT

public:
	explicit SYExamRecordItem(QWidget *parent = 0);
	
	~SYExamRecordItem();

	void SetContent(const QString & date , const QString & name , int QuestNum , int AccuracyNum , int UseSeconds);


	//int  m_MissionID;
	//int  m_PaperID;
	int  m_missionentry;
signals:
	
	void OnDescButtonClicked(SYExamRecordItem * item);

	void OnReDoButtonClicked(SYExamRecordItem * item);

private slots:
	
	void on_bt_desc_clicked();

	void on_bt_test_clicked();

private:
	Ui::SYExamRecordItem ui;
};

class SYExamPaperItem : public QWidget
{
	Q_OBJECT

public:
	explicit SYExamPaperItem(QWidget *parent = 0);
	~SYExamPaperItem();

	void SetContent(QString & col1 , QString & col2 ,QString & col3);

	int GetPaperId();

	void SetColumn3Content(QString & clo3);

	void SetColumnLabelStyleSheet(int colum , const QString & style);

	QPushButton* GetReturnBtn();
	QString GetPaperName();

private:
	Ui::SYExamPaperItem ui;
};

#endif // THEWIDGETITEM_H
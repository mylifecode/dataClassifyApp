#pragma once

#include <QWidget>
#include "ui_SYAdminNewPaperWindow.h"
#include"SYAdminTheoryTestWindow.h" 
#include"SYAdminQuestionsMgrWindow.h"
#include "SYMainWindow.h"
#include<qstring.h>
#include<qsqlrecord.h>
#include<qvector.h>
#include<mutex>
#include<thread>
#include"RbShieldLayer.h"

struct QuestionInfo  //问题信息
{
	QSqlRecord record;
	QString this_type;

	QuestionInfo(int paperId, int btnType, QSqlRecord record)
	{
		int type = static_cast<int>(btnType);
		this_type = QString::number(type) + QString("_") + QString::number(paperId);
		this->record = record;
	}
	QuestionInfo()
	{
	}
};

class SYAdminNewPaperWindow :public RbShieldLayer
{
	Q_OBJECT;
public:
	explicit SYAdminNewPaperWindow(QString paperName = "", QWidget *parent = 0);
    ~SYAdminNewPaperWindow();
	void LoadData();
	void SetMetaType();
	QListWidget* SetWidgetContent(int type);

	void RefreshTable();
	void RefreshItemStyle();
	int GetBtnType(QPushButton* Btn);
	//QVector<QuestionInfo>* GetCreatePaperInfo();
	void ShowChosedQuestions();
	void SetPaperEditInfo();
private:
    Ui::SYAdminNewPaperWindow ui;

	bool FilterRecord(QSqlRecord &record);
	void keyReleaseEvent(QKeyEvent *event);

	QPushButton *m_curBtn;
	QVector<QVector<QSqlRecord>> m_allQueryPaperInfo;
	QVector<QListWidget*> m_listWidgets;
	std::mutex m_mutex;
	QVector<int> m_typeList;
	QVector<QPushButton*>m_btnVector;
	QVector<QListWidget*> m_listWidgetVector;
	QString m_paperName;
	QVector<QuestionInfo> m_chosedQuestions;

signals:
	void showCreatingPaper(QVector<QuestionInfo> *createPaperInfo,WindowType type );
	void showNextWindow(WindowType);

private slots:
	void ShowPaperContent();
	void doCreatePaper(QString t_tableName, QString examTime, QString examScore);
};



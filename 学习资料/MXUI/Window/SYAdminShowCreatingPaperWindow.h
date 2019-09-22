#pragma once
#include <QWidget>
#include "ui_SYAdminShowCreatingPaperWindow.h"
#include "SYAdminNewPaperWindow.h"
#include"RbShieldLayer.h"
//#include"SYAdminQuestionsMgrWindow.h"

struct QuestionInfo;




class SYAdminShowCreatingPaperWindow : public RbShieldLayer
{
    Q_OBJECT

public:
	SYAdminShowCreatingPaperWindow(QVector<QuestionInfo> &createPaperInfo, QWidget *parent = 0);
    ~SYAdminShowCreatingPaperWindow();
	void Initialize();
	void showContent();
	void RefreshTable();
	int GetItemSelectedNumber();
	//QVector<QuestionInfo>* GetCreatePaperInfo();

private slots:
		void on_delete_Btn_Clicked();
		void on_return_Btn_Clicked();

//signals:void on_CreatingPaperUpdate();

private:
    Ui::SYAdminShowCreatingPaperWindow ui;
	QVector<QuestionInfo> *m_createPaperInfo;
	int m_itemSelectedNumber;
};

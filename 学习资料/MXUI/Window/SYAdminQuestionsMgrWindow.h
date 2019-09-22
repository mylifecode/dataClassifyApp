#pragma once
#include "SYAdminTheoryTestWindow.h"
#include "ui_SYAdminQuestionsMgrWindow.h"
#include <QWidget>
#include<qpushbutton.h>
#include<qlistwidget.h>
#include<qsqlrecord.h>
#include<qpushbutton.h>
//#include"SYAdminPaperContentFrameWindow.h"
#include"MxDefine.h"
#include"RbShieldLayer.h"
namespace Ui {
class SYAdminQuestionsMgrWindow;
}

class QuestionFrame :public QFrame
{
	Q_OBJECT;
public:
	QuestionFrame(QSqlRecord& cur_record,int cur_index,QWidget* parent=nullptr)
		:QFrame(parent), m_record(cur_record)
	{
		QVBoxLayout* vLayout = new QVBoxLayout();

		QString title = cur_record.value("title").toString();
		titleLabel = new QPushButton(QString("%1.").arg(cur_index + 1) + title);
		titleLabel->setObjectName("title");

		vLayout->addWidget(titleLabel);
		QString opta = cur_record.value("opta").toString();
		optaLabel = new QLabel(QString("A.") + opta);
		optaLabel->setObjectName("opta");
		vLayout->addWidget(optaLabel);
		QString optb = cur_record.value("optb").toString();
		optbLabel=new QLabel(QString("B.") + optb);
		optbLabel->setObjectName("optb");
		vLayout->addWidget(optbLabel);

		m_height = 2;

		QString optc = cur_record.value("optc").toString();
		if (optc.size() > 0)
		{
			optcLabel = new QLabel(QString("C.") + optc);
			optcLabel->setObjectName("optc");
			vLayout->addWidget(optcLabel);
			m_height = 3;
		}
		
		QString optd = cur_record.value("optd").toString();
		if (optd.size() > 0)
		{
			optdLabel = new QLabel(QString("D.") + optd);
			optdLabel->setObjectName("optd");
			vLayout->addWidget(optdLabel);
			m_height = 4;
		}
	
		QString opte = cur_record.value("opte").toString();
		if (opte.size() > 0)
		{
			opteLabel = new QLabel(QString("E.") + opte);
			opteLabel->setObjectName("opte");
			vLayout->addWidget(opteLabel);
			m_height = 5;
		}

		QString answer = cur_record.value("answer").toString();
		answerLabel = new QLabel(QString::fromLocal8Bit("ÕýÈ·´ð°¸: ") + answer);
		answerLabel->setObjectName("answer");
		vLayout->addWidget(answerLabel);

		m_height++;

		vLayout->setSpacing(15);
		vLayout->setContentsMargins(28,10,0,10);
		setLayout(vLayout);
		//this->setStyleSheet(QString("border-style:solid;border-width:1px;border-color:#46526d;border-radius:4px;"));
		Mx::setWidgetStyle(this, QString("qss:QuestionFrame.qss"));
	}
	void setFixedSize(int w, int h)
	{
		if (w)
		{
			this->setFixedWidth(w);
		}
		if (h)
		{
			this->setFixedHeight(h);
		}
	}

	QSqlRecord getItemRecord()
	{

		return m_record;
	}
	int getHeight()
	{	
		int height = (m_height)*15+ 24 + (m_height) * 20+10*2;
		return height;
	}
	QPushButton* GetBtn()
	{
		if (titleLabel == NULL)
			titleLabel = new QPushButton;

		return titleLabel;
	}
	
private:
	QPushButton *titleLabel;
	QLabel *optaLabel;
	QLabel *optbLabel;
	QLabel *optcLabel;
	QLabel *optdLabel;
	QLabel *opteLabel;
	QLabel* answerLabel;
	int m_height;
	QSqlRecord m_record;
};

class ListWidgetItemFrame :public QFrame
{
public:
	ListWidgetItemFrame(QWidget*parent, QSqlRecord record, int index,bool flag=true)
	{
		QHBoxLayout *hLayout = new QHBoxLayout();
		if (flag)
		{
		//	hLayout->addItem(new QSpacerItem(78, 22, QSizePolicy::Fixed, QSizePolicy::Fixed));
		}
		QVBoxLayout *vLayout = new QVBoxLayout();
		vLayout->addSpacerItem(new QSpacerItem(22, 15, QSizePolicy::Fixed, QSizePolicy::Fixed));
		box = new QCheckBox(this);
		vLayout->addWidget(box);
		vLayout->addSpacerItem(new QSpacerItem(22, 22, QSizePolicy::Fixed, QSizePolicy::Expanding));

		hLayout->addLayout(vLayout);
		hLayout->addItem(new QSpacerItem(10, 22, QSizePolicy::Fixed, QSizePolicy::Fixed));
		frameWin = new QuestionFrame(record, index);
		hLayout->addWidget(frameWin);
		hLayout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
		hLayout->setSpacing(0);
		hLayout->setContentsMargins(0, 0, 0, 0);
		setLayout(hLayout);

	//	Mx::setWidgetStyle(this, QString("qss:ListWidgetItemFrame.qss"));
		//setStyleSheet(QString("background-color:rgb(255,0,0);box-shadow:0px 5px 10px 0px #1c202b;border-radius:4px;border:solid 1px #46526d;"));
	
	}
	void setQuestionFrameHintSize(int w=0,int h=0)
	{
		if (w)
			frameWin->setFixedWidth(w);
		if (h)
			frameWin->setFixedWidth(h);
	}
	int getItemHeight()
	{
		int height = frameWin->getHeight();
		return height;
	}
	QuestionFrame* getQuestionFrame()
	{
		return frameWin;
	}
	QCheckBox* getCheckBox()
	{
		return box;
	}
	void setChecked(bool state)
	{
		box->setChecked(state);
	}
	bool  IsChecked()
	{
		if (box->isChecked())

			return true;
		else

			return false;
	}
	void SetBackGroundColor(bool flag)
	{
		if (flag)
		{
			setStyleSheet(QString("background-color:rgb(255,0,0)"));

		}
		else

			setStyleSheet(QString("background-color:#31394c"));

	}
		void ItemStateChange(bool flag)
		{
			setChecked(flag);
			//SetBackGroundColor(flag);
		}
private:

	QCheckBox *box;
	QuestionFrame* frameWin;
};


class SYAdminQuestionsMgrWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYAdminQuestionsMgrWindow(int moduleType,QWidget *parent = 0);
    ~SYAdminQuestionsMgrWindow();

	void InitPage();
	void RefreshWidget();
	QListWidget* SetListWidget(int type);

private slots:
	void do_module_Btn_Clicked();
	void do_batchImportData();
	void do_deleteQuestions();
	void do_newBuiltQuestion(QSqlRecord* record=nullptr);

private:
    Ui::SYAdminQuestionsMgrWindow ui;
	QPushButton* m_curBtn;
	QVector<QPushButton*> m_btnVector;
	QVector<QListWidget*> m_listWidgetVector;
	QVector<QVector<QSqlRecord>> m_allRecords;
};



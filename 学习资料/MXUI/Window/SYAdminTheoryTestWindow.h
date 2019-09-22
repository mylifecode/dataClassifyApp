#pragma once
#include <QWidget>
#include"ui_SYAdminTheoryTestWindow.h"  //camke±àÒëÉú³Éui.hÎÄ¼þ
#include <vector>
#include <QSqlRecord>
#include <QTableWidget>
#include<qpushbutton.h>
#include<qlist.h>
#include<qfile.h>
#include<qlistwidget.h>
#include<qstringlist.h>
#include<QTextStream>
#include<qlabel.h>

class QuestionModule :public QWidget
{
	Q_OBJECT;
public:
	QuestionModule(QString t_moduleName, QString t_iconPath = "", bool flag = true, QWidget* parent = 0)
		:QWidget(parent), deleteBtn(NULL), moduleName(t_moduleName), iconPath(t_iconPath)
	{
		QHBoxLayout* hLayout = new QHBoxLayout();
		QVBoxLayout*vLayout = new QVBoxLayout();
		vLayout->addItem(new QSpacerItem(1, 18, QSizePolicy::Expanding, QSizePolicy::Fixed));
		bgBtn = new QPushButton(this);
		bgBtn->setFixedSize(QSize(262, 90));
		bgBtn->setStyleSheet(QString("background-color:rgb(0,0,0);font-size:20px;color:#ffffff;"));

		if (flag)
		{
			SetBtnLayout();
		}
		else
		{
			bgBtn->setText(t_moduleName);
		}
		vLayout->addWidget(bgBtn);
		hLayout->addLayout(vLayout);
		hLayout->addItem(new QSpacerItem(18, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
		setLayout(hLayout);

		deleteBtn = new QPushButton(this);
		deleteBtn->setGeometry(262, 16, 28, 28);
		deleteBtn->setStyleSheet(QString("border-image:url(icons:/SYAdminTheoryTestWindow/delete_icon.png); \
										 										 		border:none; "));
	}


	~QuestionModule()
	{
	}

	void SetAddModule()
	{
		deleteBtn->setVisible(false);

		addBtn = new QPushButton(this);
		addBtn->setGeometry(50,55,40, 40);
		addBtn->setStyleSheet(QString("border-image:url(icons:/SYAdminTheoryTestWindow/article_icon.png); \
										 		border:none; "));
	}

	QPushButton* GetReturnBtn()
	{
		return bgBtn;
	}
	QPushButton* GetReturnAddBtn()
	{
		return addBtn;
	}
	QPushButton*GetDelBtn()
	{
		return deleteBtn;
	}

private:
	void SetBtnLayout()
	{
		QHBoxLayout* hLayout = new QHBoxLayout();
		QHBoxLayout* contentLayout = new QHBoxLayout();
		QLabel* label1;
		QLabel* label2 = new QLabel();
		if (iconPath.size() > 0)
		{
			label1 = new QLabel();
			label1->setPixmap(QPixmap(iconPath));
			label1->setFixedSize(45, 80);
		}
		
		label2->setText(moduleName);
		label2->setStyleSheet("font-size:20px;color:#ffffff;");
		label2->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
		
		if (iconPath.size() > 0)
		{
			contentLayout->addWidget(label1);
			//contentLayout->addItem(new QSpacerItem(20, QSizePolicy::Fixed));
		}
		contentLayout->addWidget(label2);
		hLayout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
		hLayout->addLayout(contentLayout);
		hLayout->addItem(new QSpacerItem(1, QSizePolicy::Expanding));
		//hLayout->setSpacing(4);
		bgBtn->setLayout(hLayout);
	}

private:

	QPushButton* bgBtn;
	QPushButton* deleteBtn;
	QPushButton* addBtn;
	QString moduleName;
	QString iconPath;

};

const QString filePath = "..\\Config\\questionModuleInfo.txt";

class ConfigFileOperation
{
public:

	static QStringList readFile()
	{
		QFile infile(filePath);
		bool flag = infile.open(QIODevice::ReadOnly | QIODevice::Text);
		QStringList modulesInfoArray;
		if (flag)
		{
			QByteArray t = infile.readAll();
			infile.close();
			QString modulesInfo = QString::fromLocal8Bit(t);
			modulesInfoArray = modulesInfo.split('#', QString::SkipEmptyParts);
		}
		return modulesInfoArray;

	}

	static bool saveFile(QString saveContent)
	{
		bool ret = false;
		QFile outfile(filePath);
		bool flag = outfile.open(QIODevice::WriteOnly | QIODevice::Text);
		if (flag)
		{
			QTextStream out(&outfile);
			out << saveContent;
			outfile.close();
			ret = true;
		}
		return ret;
	}

	static ConfigFileOperation* getInstance()
	{
		static ConfigFileOperation* m_instance = new ConfigFileOperation;
		return m_instance;
	}
private:
	ConfigFileOperation()
	{
		
	}
protected:

	//static QString filePath;	
};


class SYAdminTheoryTestWindow : public QWidget
{
	Q_OBJECT

public:
	explicit SYAdminTheoryTestWindow(QWidget *parent = 0);
	~SYAdminTheoryTestWindow();

signals:
	void showNextWindow(WindowType windowType);
	void onModifyPaperBtn_clicked(QString tableName);//ÐÞ¸ÄÊÔ¾í;
	
private slots:
	void onBackToWidget(QWidget*);
	void do_addModule(QString iconPath, QString&moduleName);
	
private:
	void Initialize();
	void InitTables();
	void LoadData();
	void SetTableContent();
	void RefreshTable();
	void keyReleaseEvent(QKeyEvent* event);
	bool FilterRecord(QSqlRecord record);
	void SetModulePage();
	void delete_questionModule(QPushButton*, QStringList);


private slots:
	void on_paperMgrBtn_clicked();
	void on_questionBankMgrBtn_clicked();
	void on_newBtn_clicked();
	void on_upLoadBtn_clicked();
//	void on_sendPaperBtn_clicked();  //·¢ËÍÊÔ¾í
	void onShowPaperBtn_clicked();  //²é¿´ÊÔ¾í	
	void onDeleteRecord(); //É¾³ýÊÔ¾í
protected:
	void showEvent(QShowEvent *event);
private:
	Ui::SYAdminTheoryTestWindow ui;
	QPushButton* m_curSelectedBtn;
	QTableWidget* m_paperSetTable;
	QTableWidget* m_questionBankTable;
	QVector<QSqlRecord> m_paperSetVec;
	QVector<QuestionModule*> m_moduleVector;
	QVector<QuestionModule*> m_oldModuleVector;
};
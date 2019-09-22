#pragma once
#include "ui_SYAdminSendPaperWindow.h"
#include <QWidget>
#include"RbShieldLayer.h"
#define  CHS(txt) QString::fromLocal8Bit(txt)

class SYListWidgetFrame :public QFrame
{
public:
	SYListWidgetFrame(int t_ItemId, QWidget* parent) :QFrame(parent)
	{
		itemId = t_ItemId;
	}
	~SYListWidgetFrame()
	{

	}
	void Create(int height, QString& text0, QString &text1, QString &text2)
	{
		setFixedHeight(height);
		int offset = 31;
		int itemWidth = 207;

		m_box = new QCheckBox("", this);
		m_box->setFixedHeight(height);
		m_box->setFixedWidth(itemWidth);
		m_box->move(offset, 0);
		offset += itemWidth;

		m_label0 = new QLabel(text0, this);
		m_label0->setFixedHeight(height);
		m_label0->setFixedWidth(itemWidth);
		m_label0->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		m_label0->setStyleSheet(QString("font-size:16px; color:#b7c5d8;"));
		m_label0->move(offset, 0);
		offset += itemWidth;

		
		m_label1 = new QLabel (this);
		m_label1->setFixedHeight(height);
		m_label1->setFixedWidth(itemWidth);
		m_label1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		if (text1 == "0")
		{
			m_label1->setText(CHS("Î´·ÖÅä"));
			m_label1->setStyleSheet(QString("font-size:16px; color:#289fff;text-align:left;"));
		}
		else
		{
			m_label1->setText(text1);
			m_label1->setStyleSheet(QString("font-size:16px; color:#b7c5d8;text-align:left;"));
		}
		m_label1->move(offset, 0);
		offset += itemWidth;

		m_label2 = new QLabel(text2, this);
		m_label2->setFixedHeight(height);
		m_label2->setFixedWidth(itemWidth);
		m_label2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

		if (text2 == "0")
		{
			m_label2->setText(CHS("Î´·ÖÅä"));
			m_label2->setStyleSheet(QString("font-size:16px; color:#289fff;text-align:left;"));
		}
		else
		{
			m_label2->setText(text2);
			m_label2->setStyleSheet(QString("font-size:16px; color:#b7c5d8;text-align:left;"));
		}
		m_label2->move(offset, 0);
		offset += itemWidth;
	}
	bool IsChecked()
	{
		if (m_box->isChecked())
			return true;
		else
			return false;
	}
	void SetChecked(bool state)
	{
			m_box->setChecked(state);
	}

	QString GetFrameImInfo()
	{
		return m_label0->text();
	}

private:
	QCheckBox* m_box;
	QLabel* m_label0;
	QLabel* m_label1;
	QLabel* m_label2;
	int itemId;
public:
	int m_ID;
	QString m_UserName;
};





class SYAdminSendPaperWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYAdminSendPaperWindow(QWidget *parent = 0);
    ~SYAdminSendPaperWindow();

	void Initialize();
	void refreshTaskReceivedList();

private slots:
   // void on_SendTaskInfo();
	void on_confirm_Btn_Clicked();
	void on_checkbox_statechanged(int state);
	void on_btn_clicked();
private:
    Ui::SYAdminSendPaperWindow ui;
	QPushButton* m_curBtn;
	
};



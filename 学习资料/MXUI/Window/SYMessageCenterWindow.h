#pragma once
#include"ui_SYMessageCenterWindow.h"
#include <vector>

class SYMessageWindow;

class SYMessageCenterWindow : public QWidget
{
	Q_OBJECT
public:
	SYMessageCenterWindow(QWidget* parent = nullptr);

	~SYMessageCenterWindow();

private:
	void LoadMessages();

	void AddMessageWindow(SYMessageWindow* messageWindow);

	void SortMessageWindowByDateTimeDesc();

private slots:
	void on_selectAllBtn_clicked();
	
	void on_reverseSelecteBtn_clicked();

	void on_deleteBtn_clicked();

private:
	Ui::MessageCenterWindow ui;

	std::vector<SYMessageWindow*> m_messageWindows;
};


#pragma once
#include <QWidget>
#include "ui_SYMenuWindow.h"

enum WindowType;

class SYMenuWindow : public QWidget
{
	Q_OBJECT
public:
	SYMenuWindow(QWidget* parent = nullptr);

	~SYMenuWindow();

	void setCurSelectedItem(WindowType windowType);

signals:
	void showNextWindow(WindowType windowType);

private slots:
	void onClickedBtn();

private:
	Ui::SYMenuWindow ui;
	QPushButton* m_curSelectedBtn;
};


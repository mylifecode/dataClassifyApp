#pragma once
#include "ui_SYLoginWindow.h"
#include <MxDefine.h>

class SYLoginWindow : public QWidget
{
	Q_OBJECT
public:
	SYLoginWindow(QWidget *parent = 0, Qt::WindowFlags flag = 0);

	~SYLoginWindow(void);

protected:
	void showEvent(QShowEvent*);

public slots:
    void on_userloginBtn_clicked();
	void on_guestloginBtn_clicked();

signals:
	void showNextWindow(WindowType type);

private:
	Ui::UserLogin ui;

};
 

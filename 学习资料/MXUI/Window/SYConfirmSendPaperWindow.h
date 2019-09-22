#pragma once
#include "ui_SYConfirmSendPaperWindow.h"
#include"RbShieldLayer.h"


class SYConfirmSendPaperWindow : public RbShieldLayer
{
    Q_OBJECT

public:
	explicit SYConfirmSendPaperWindow(QVector<QString> sender, QVector<QString> receiver, QWidget *parent);
    ~SYConfirmSendPaperWindow();
	void SetContent(QVector<QString> senders, QVector<QString> receivers);
	

private:
    Ui::SYConfirmSendPaperWindow ui;
};



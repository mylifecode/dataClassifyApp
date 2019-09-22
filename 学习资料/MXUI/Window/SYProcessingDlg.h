#ifndef _SYProcessingDlg_H
#define _SYProcessingDlg_H
#include "ui_SYProcessingDlg.h"
#include "RbShieldLayer.h"
#include <QVector>
#include <QPushButton>
#include <QTimer>
#include "SYTrainTaskStruct.h"

class  SYProcessingDlg : public RbShieldLayer
{
	Q_OBJECT

public:

	SYProcessingDlg(QWidget *parent);
	
	~SYProcessingDlg();


	void SetProcess(int percent);

	void SetCompleted();

	void SetAutoProcess(int waitTime);

signals:
	void SendRefreshBar(int);
	void SendComplete();

private slots:

    void InternalRefreshBar(int);

	void InternalComplete();

	void on_bt_conform_clicked();

	void onCloseTimer();

	void onAutoProcessTimer();

protected:
	void showEvent(QShowEvent* event);

private:
	bool m_HasShowed;
	int  m_MaxBarLength;

	QTimer* m_closeTimer;
	QTimer* m_autoProcessTimer;
	int m_processValue;
	int m_maxProcessValue;

	Ui::SYProcessingDlg  ui;
};

#endif
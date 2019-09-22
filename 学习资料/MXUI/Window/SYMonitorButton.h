#pragma once
#include <QWidget>
#include "ui_RbMonitorButton.h"
#include <QPoint>
#include <QMutex>
#include <cstdint>

class SYMonitorButton : public QWidget
{
	Q_OBJECT
public:
	SYMonitorButton(QWidget * parent = NULL);
	~SYMonitorButton(void);

	void setInfo(const QString& info)
	{
		//ui.infoLabel->setText(info);
		ui.watchBtn->setText(info);
	}

	void SetExtraData(uint32_t data);

	uint32_t GetExtraData();

signals:
	void watch();
	void stop();

protected slots:
	void onClickedWatchButton();

protected:
	bool eventFilter(QObject *, QEvent *);

private:

	Ui::MonitorButton ui;
	QPoint m_ptStart;
	QPoint m_ptEnd;
	bool m_bMoved;
	uint32_t m_extraData;
	QMutex m_propertyMutex;
};

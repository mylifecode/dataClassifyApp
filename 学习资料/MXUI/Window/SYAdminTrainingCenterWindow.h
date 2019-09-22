#pragma once
#include <QWidget>
#include "ui_SYAdminTrainingCenterWindow.h"

enum WindowType;

class SYAdminTrainingCenterWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminTrainingCenterWindow(QWidget* parent = nullptr);

	~SYAdminTrainingCenterWindow();

signals:
	void showNextWindow(WindowType type);

protected:
	void showEvent(QShowEvent* event);

	void mousePressEvent(QMouseEvent* event);

	void mouseReleaseEvent(QMouseEvent* event);

	void LaunchRealTrainModule();
private:
	Ui::SYAdminTrainingCenterWindow ui;

	int m_clickedFrameIndex;
};


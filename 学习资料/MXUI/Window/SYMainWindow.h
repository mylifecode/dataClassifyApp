#pragma once
#include <QWidget>
#include <QTimer>
#include "ui_mainWindow.h"
#include "MxDefine.h"


class RbShutdownBox;
class RbAbout;
class MxDemonstrationWindow;


class SYMainWindow : public QWidget
{
	Q_OBJECT
private:
	SYMainWindow(QWidget* parent = nullptr);

	~SYMainWindow();

public:
	static SYMainWindow* GetInstance() { return m_instance; }

	static void CreateInstance();

	static void DestoryInstance();

	void ReturnToLoginWindow();

	int  GetWindowDepthLevel(WindowType type);


protected:
	void mousePressEvent(QMouseEvent* event);

signals:

	void BackToWidget(QWidget * widget);


public slots:
	void on_exitBtn_clicked();

	void on_backBtn_clicked();

	void on_shutdownBtn_clicked();

	void on_aboutBtn_clicked();

	void onShowNextWindow(WindowType type);

	void onExitCurrentWindow(WindowType type);

	void onTimer();

	void showEvent(QShowEvent *e);

private:
	void Initialize();

	void SetWindowType(QWidget* widget, WindowType type);

	WindowType GetWindowType(QWidget* widget);

private:
	

	Ui::MainWindow m_ui;
	int m_curWindowIndex;

	RbShutdownBox* m_shutdownWindow;
	RbAbout* m_aboutWindow;
	MxDemonstrationWindow* m_demonstrationWindow;

	QTimer* m_timer;
	

public:static SYMainWindow* m_instance;
};


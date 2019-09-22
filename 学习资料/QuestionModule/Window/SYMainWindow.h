#pragma once
#include <QWidget>
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

signals:

	void BackToWidget(QWidget * widget);

protected:
	void mousePressEvent(QMouseEvent* event);

private slots:
	void on_exitBtn_clicked();

	void on_backBtn_clicked();

	void on_shutdownBtn_clicked();

	void on_aboutBtn_clicked();

	void onShowNextWindow(WindowType type);

	void onReplaceCurrentWindow(WindowType type);

	void onExitCurrentWindow();

private:
	void Initialize();

	void SetWindowType(QWidget* widget, WindowType type);

	WindowType GetWindowType(QWidget* widget);

private:
	static SYMainWindow* m_instance;

	Ui::MainWindow m_ui;
	int m_curWindowIndex;

	RbShutdownBox* m_shutdownWindow;
	RbAbout* m_aboutWindow;

};


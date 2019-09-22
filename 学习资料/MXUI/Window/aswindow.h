#ifndef AsWindow_H
#define AsWindow_H

#if defined(EXPORT_DLL)
#define AB_EXPORT __declspec(dllexport)
#else
#define AB_EXPORT __declspec(dllimport)
#endif

#include <QDialog>
#include <QFrame>
#include <QMenu>
#include <QTimer>
#include <Windows.h>
#include <QDesktopWidget>
#include <QMutex>
#include "qtlog.h"
#include "ui_aswindow.h"

class  AsWindow : public QDialog
{
	Q_OBJECT
public:
	enum AsWindowHit {
		AsWindowTitlebarHint = 0x00000001,
		AsWindowMenuHint = 0x00000002,
		AsWindowMinimizeButtonHint = 0x00000004,
		AsWindowMaximizeButtonHint = 0x00000008,
		AsWindowCloseButtonHint = 0x00000010,
	};
	AsWindow(QWidget *parent = 0, Qt::WindowFlags flags = NULL);
	~AsWindow();

public:
	void showOnHideParent(QWidget * pWidget);
	void CloseMe(int delay);
public slots:
		virtual void on_exitBtnClicked();
		void on_aboutBtnClicked();
		virtual void on_retBtnClicked();
	    virtual  void on_msgBtnClicked();
		virtual  void on_perCenterClicked();
		void onClickedtipBtn();

protected:
	virtual bool event(QEvent *event);
	bool eventFilter(QObject *watched, QEvent *event);
	void closeEvent(QCloseEvent * event);
	enum SizingDirect {
		NoSizing = 0, sizingN, sizingS, sizingW, sizingE, sizingNW, sizingNE, sizingSW, sizingSE,
	};

	QToolButton *getRetBtn();
	QToolButton *getAboutBtn();
	QToolButton *getExitBtn();
	QToolButton  *getPerCenterBtn();
	QToolButton  *getMsgBtn();

protected:
	QFrame* m_bg;
	QWidget * m_parent;

private:
	Ui::aswindow ui;
};

#endif // AsWindow_H

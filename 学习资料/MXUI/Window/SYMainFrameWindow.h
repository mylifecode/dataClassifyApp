#pragma once
#include <QWidget.h>
#include <QToolButton.h>
#include "ui_SYMainFrameWindow.h"
class RbAbout;

class SYMainFrameWindow : public QWidget
{
public:
	Q_OBJECT
public:
	SYMainFrameWindow(QWidget * parent = NULL, Qt::WindowFlags flag = 0);
	virtual ~SYMainFrameWindow(void);

	bool event(QEvent * e);
	QToolButton * getBackToolBtn();

	void fullShowAndHideParent();
	QWidget * getParent();
	void hidePersonBtn();
	void showPersonBtn();
	void hidebackBtn();
	void hideabout();
	void hidepowerBtn();
	void hideaboutBtn();
	int retCheckWindow(){return m_checkWindow;}

	Ui::MainFrameWindow& getUI(void) {return ui;}

protected:
	void closeEvent(QCloseEvent *);
	void mousePressEvent(QMouseEvent *);
	void showEvent(QShowEvent*);
	int m_checkWindow;  //判断是在哪个界面

public slots:

	virtual void onClickedShutdownBtn();
	virtual void onClickedAboutBtn();
	virtual void onClickedBackBtn();
	virtual void onClickedPerson();


private:
	Ui::MainFrameWindow ui;
	QFrame * m_pBgFrame;
	QWidget * m_pParentWidget;
	RbAbout * m_pAbout;
};

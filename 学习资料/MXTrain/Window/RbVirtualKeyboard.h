#pragma once
#include <QWidget>
#include <QSignalMapper>
#include <QMouseEvent>
#include "ui_RbVirtualKeyboard.h"


class RbVirtualKeyboard : public QWidget
{
	Q_OBJECT
public:
	RbVirtualKeyboard();
	~RbVirtualKeyboard();

	QString getBuffer(){ return m_content; }
	
public Q_SLOTS:
	void clear();

protected:
	void showEvent(QShowEvent *);

private:
	void setMapper();
	void connectMapper();

signals:
	void contentChanged(const QString& text);

private Q_SLOTS:
	void setDispText(const QString& text);
	void onBackspace();

private:
	Ui::RbVirtualKeyboard ui;

	QSignalMapper *m_signalMapper;
	QString m_content;
};
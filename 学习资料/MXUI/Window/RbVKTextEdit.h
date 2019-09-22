#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QString>
#include <QMouseEvent>
#include <QProcess>
#include <Windows.h>

class RbVKTextEdit : public QTextEdit
{
	Q_OBJECT
public:
	RbVKTextEdit(QWidget * parent = NULL);
	~RbVKTextEdit(void);
	void mousePressEvent(QMouseEvent * event);
	virtual void focusOutEvent(QFocusEvent * event);
private:
	QProcess* m_process;

};

#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QMouseEvent>
#include <QProcess>
#include <Windows.h>

class RbVKLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	RbVKLineEdit(QWidget * parent = NULL);
	~RbVKLineEdit(void);
	void mousePressEvent(QMouseEvent * event);
	bool closeKeyboard();
private:
	QProcess* m_process;

};

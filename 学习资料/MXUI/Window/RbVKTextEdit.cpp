#include "RbVKTextEdit.h"
#include <QEvent>
#include <Windows.h>

RbVKTextEdit::RbVKTextEdit(QWidget * parent) :QTextEdit(parent),
m_process(NULL)
{
}

RbVKTextEdit::~RbVKTextEdit(void)
{
	
}

void RbVKTextEdit::mousePressEvent(QMouseEvent * event)
{	
	m_process = new QProcess(this);
	m_process->start("mxosk.exe");
}

void RbVKTextEdit::focusOutEvent(QFocusEvent * event)
{
	if (m_process)
	{
		m_process->close();
		delete m_process;
		m_process = NULL;
	}
	
}

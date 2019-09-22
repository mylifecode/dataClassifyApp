#include "RbVKLineEdit.h"
#include <QEvent>
#include <Windows.h>

RbVKLineEdit::RbVKLineEdit(QWidget * parent) :QLineEdit(parent),
m_process(NULL)
{
	m_process = new QProcess(this);
}

RbVKLineEdit::~RbVKLineEdit(void)
{

}

void RbVKLineEdit::mousePressEvent(QMouseEvent * event)
{		
	//m_process->start("mxosk.exe");

	//QString         strCmd;
	//strCmd(_T("/adminoption %d"), idd);

// 	SHELLEXECUTEINFOA execinfo;
// 	memset(&execinfo, 0, sizeof(execinfo));
// 	execinfo.lpFile = "mxosk.exe";
// 	execinfo.cbSize = sizeof(execinfo);
// 	execinfo.lpVerb = "runas";
// 	execinfo.fMask = SEE_MASK_NO_CONSOLE;
// 	execinfo.nShow = SW_SHOWDEFAULT;
// 	execinfo.lpParameters = "/adminoption 1";
// 
// 	ShellExecuteExA(&execinfo);
}

bool RbVKLineEdit::closeKeyboard()
{
// 	if (m_process)
// 	{
// 		m_process->close();
// 		return true;
// 	}
 	return false;
}

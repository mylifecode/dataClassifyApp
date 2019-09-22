#include "SYLineEdit.h"
#include <QHBoxLayout>
#include <QSpacerItem>
#include <Windows.h>
#include "MxGlobalConfig.h"

#define MAKESKINDIR (MxGlobalConfig::Instance()->GetSkinDir())

SYLineEdit::SYLineEdit(QWidget * parent, QString strNormalBg, QString strChangedBg, QString strClearBtnNormalStyle, QString strClearBtnPressedStyle)
	:QLineEdit(parent),
m_strNormalStyle(strNormalBg),
m_strTextChangedStyle(strChangedBg),
m_strClearBtnNormalStyle(strClearBtnNormalStyle),
m_strClearBtnPressedStyle(strClearBtnPressedStyle),
m_strCurBtnStyle(strClearBtnNormalStyle),
m_oldCursor(NULL),
mouselisten(0)
{
	changed = false;
	m_pBtnClearContent = new QPushButton("",this);
	int x = width();
	int y = height();
	m_pBtnClearContent->setObjectName("ClearButton");
	m_pBtnClearContent->setGeometry(0,0,30,30);
	m_pBtnClearContent->setMinimumWidth(30);
	m_pBtnClearContent->setMaximumWidth(30);

	m_pBtnClearContent->setMinimumHeight(30);
	m_pBtnClearContent->setMaximumHeight(30);
	m_pBtnClearContent->move(x,0);
	m_pBtnClearContent->hide();
	m_oldCursor = GetCursor();

	QHBoxLayout * pHLayout = new QHBoxLayout(this);
	pHLayout->addSpacerItem(new QSpacerItem(250,y,QSizePolicy::Expanding));
	pHLayout->addWidget(m_pBtnClearContent);
		
	connect(m_pBtnClearContent,SIGNAL(clicked()),this,SLOT(clearEditText()));
	connect(this,SIGNAL(textChanged(const QString&)),this,SLOT(textChanged(const QString&)));
	m_pBtnClearContent->setMouseTracking(true);
	m_pBtnClearContent->installEventFilter(this);
	//resize(300,70);
	setStyleSheet(m_strNormalStyle);
	
	setClearBtnNormalStyleSheet(QString(MAKESKINDIR + "/Shared/edit_clear_nor.png"));
	setClearBtnPressedStyleSheet(QString(MAKESKINDIR + "/Shared/edit_clear_Pre.png"));
	
	m_pBtnClearContent->setFocusPolicy(Qt::NoFocus);
}

SYLineEdit::~SYLineEdit(void)
{
	delete m_pBtnClearContent;
	m_pBtnClearContent = NULL;
}


void SYLineEdit::showClearBtn()
{
	m_pBtnClearContent->show();
}

void SYLineEdit::hideClearBtn()
{
	m_pBtnClearContent->hide();
}

void SYLineEdit::clearEditText()
{
	setText("");
}

void SYLineEdit::textChanged(const QString & strText)
{
	if(strText != "" )
	{
		setStyleSheet(m_strTextChangedStyle);
		m_pBtnClearContent->show();
	}
	else
	{
		setStyleSheet(m_strNormalStyle);
		m_pBtnClearContent->hide();
	}
	m_pBtnClearContent->setStyleSheet(m_strCurBtnStyle);
}

void SYLineEdit::setNormalPic(QString & strNormal)
{
	m_strNormalStyle = QString("border-image: url(%1)0 0 0 0;").arg(strNormal);
	textChanged("");
}

void SYLineEdit::setTextChangedPic(QString& picChanged)
{
	m_strTextChangedStyle = QString("border-image: url(%1)0 0 0 0;").arg(picChanged);
	textChanged("");
}

void SYLineEdit::setClearBtnNormalStyleSheet(QString & strNormal)
{
	m_strClearBtnNormalStyle = QString("border-image: url(%1);").arg(strNormal);
	m_strCurBtnStyle = m_strClearBtnNormalStyle;
	textChanged("");
}

void SYLineEdit::setClearBtnPressedStyleSheet(QString& strPressed)
{
	m_strClearBtnPressedStyle = QString("border-image: url(%1);").arg(strPressed);
	textChanged("");
}

bool SYLineEdit::event(QEvent * e)
{
	bool flag = false;
	switch(e->type())
	{
	case QEvent::MouseMove:
		setStyleSheet(m_strTextChangedStyle);
		m_pBtnClearContent->show();
		flag = true;
		break;
	case QEvent::Leave:
		setStyleSheet(m_strNormalStyle);
		m_pBtnClearContent->hide();
	default:
		break;
	}
	return QLineEdit::event(e);
}

bool SYLineEdit::eventFilter(QObject* watched, QEvent * event)
{
	int k = 1;
	if(watched == m_pBtnClearContent)
	{
		switch(event->type())
		{
		case QEvent::Enter:
			k = 2;
			m_pBtnClearContent->setStyleSheet(m_strClearBtnPressedStyle);
			m_oldCursor = GetCursor();
			//TODO
			//setCursor(QCursor((HCURSOR)LoadCursor(NULL,IDC_ARROW)));
			break;
		case QEvent::Leave:
			k = 3;
			m_pBtnClearContent->setStyleSheet(m_strClearBtnNormalStyle);
			//TODO
			//setCursor(QCursor(m_oldCursor));
			break;
		}
	}
	return __super::eventFilter(watched,event);
}



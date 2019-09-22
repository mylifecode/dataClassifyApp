#include "AsLang.h"
#include <QApplication>
#include <QTranslator>
#include <QFile>
AsLang* AsLang::self = NULL;
AsLang* AsLang::instance()
{
	if (self == NULL)
	{
		self = new AsLang;
	} 
	//else cont.

	return self;
}

AsLang::AsLang(QObject* parent):
QObject(parent)
,m_abTranslator(NULL)
,m_appTranslator(NULL)
,m_qtTranslator(NULL)
,m_currentLang("zh_CN")
{
//	m_abTranslator = new QTranslator();
//	m_appTranslator = new QTranslator();
	m_qtTranslator = new QTranslator( this );

	//ChangeLanguage("zh_CN");
}  

AsLang::~AsLang()
{
	if ( m_qtTranslator )
	{
		delete m_qtTranslator;
		m_qtTranslator = NULL;
	}
}

void AsLang::Destroy()
{
	if ( self )
	{
		delete self;
		self = NULL;
	}
}

void AsLang::ChangeLanguage(QString strLanguage)
{
// 	if (!m_abTranslator->isEmpty())
// 	{
// 		qApp->removeTranslator(m_abTranslator);
// 	}
// 
// 	if(!m_appTranslator->isEmpty()) 
// 	{ 
// 		qApp->removeTranslator(m_appTranslator);
// 	}

	if(!m_qtTranslator->isEmpty())
	{
		qApp->removeTranslator(m_qtTranslator);
	}

	QString strLanguageFilePath = ":/language/";

	bool bRet = QFile::exists(strLanguageFilePath + QString("qt_") + strLanguage + ".qm");
	bool b = m_qtTranslator->load(strLanguageFilePath + QString("qt_") + strLanguage + ".qm");
//	QString strApp = qApp->applicationName() + "_";
//	m_abTranslator->load(strLanguageFilePath + strApp + strLanguage + ".qm");
//	m_appTranslator->load(strLanguageFilePath + QString("ab_") + strLanguage + ".qm");
	
	qApp->installTranslator(m_qtTranslator);
	//qApp->installTranslator(m_abTranslator);
	//qApp->installTranslator(m_appTranslator);
}

QString AsLang::getCurrentLang()
{
	return m_currentLang;
}

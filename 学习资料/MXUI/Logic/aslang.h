#ifndef ABLANG_H
#define ABLANG_H

#include <QObject>


class QTranslator;
class  AsLang:public QObject
{
public:
	AsLang(QObject *parent = 0);
	~AsLang();
	void Destroy();

	static AsLang* instance();
public:
	void ChangeLanguage(QString strLanguage);
	QString getCurrentLang();
protected:
	static AsLang* self;
private:
	QString m_currentLang;
	QTranslator *m_abTranslator;
	QTranslator *m_appTranslator;
	QTranslator *m_qtTranslator;

};
#define g_lang (AsLang::instance())

#endif // ABLANG_H

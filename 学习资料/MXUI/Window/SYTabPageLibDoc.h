#pragma once
#include "ui_SYTabPageLibDoc.h"
#include <QString>
#include <QStringList>
#include <QTextBrowser>

class SYTabPageLibDoc : public QWidget
{
	Q_OBJECT
public:
	SYTabPageLibDoc(QWidget * parent);

	~SYTabPageLibDoc(void);

	//void setContents(const QVector<QString>& contents);

	void setContent(const QString& content);

private slots:

private:
	Ui::SYTabPageLibDoc ui;

	//QVector<QTextBrowser*> m_textBrowsers;
	QTextBrowser* m_textBrowser;
};
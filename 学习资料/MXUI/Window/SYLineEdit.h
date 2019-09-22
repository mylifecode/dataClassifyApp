#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QEvent>
#include <Windows.h>

class SYLineEdit : public QLineEdit
{
	Q_OBJECT
public:
	SYLineEdit(QWidget * parent = NULL, QString strEditNormalStyle = "", QString strEditTextChangedStyle = "", QString strClearBtnNormalStyle = "", QString strClearBtnPressedStyle = "");
	~SYLineEdit(void);

	void showClearBtn();
	void hideClearBtn();

	void setNormalPic(QString & picNormal);	//设置无文本是的风格
	void setTextChangedPic(QString & picChanged);	//设置有文本时的风格
	void setClearBtnNormalStyleSheet(QString & strNormal);
	void setClearBtnPressedStyleSheet(QString& strPressed);

	virtual bool event(QEvent * e);
public slots:
	void clearEditText();
	void textChanged(const QString &);

protected:
	bool eventFilter(QObject *watched, QEvent *event);
	
private:
	QPushButton * m_pBtnClearContent;
	bool changed;
	QString m_strNormalStyle;
	QString m_strTextChangedStyle;
	QString m_strClearBtnNormalStyle;
	QString m_strClearBtnPressedStyle;
	QString m_strCurBtnStyle;
	HCURSOR m_oldCursor;
	int mouselisten;
};

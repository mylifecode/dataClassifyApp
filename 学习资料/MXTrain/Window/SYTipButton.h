#pragma once

#include <QPushButton.h>


class SYTipButton : public QPushButton
{
	Q_OBJECT
public:
	SYTipButton(QWidget *parent);

	virtual ~SYTipButton(void);

	virtual void paintEvent(QPaintEvent * event);

	void SetTipStr(const QString & tipStr);

	QString m_TipStr;
};

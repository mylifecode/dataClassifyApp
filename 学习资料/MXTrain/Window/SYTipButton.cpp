#include "SYTipButton.h"
#include <qpainter>
#define LABEL_WIDTH 20
#define LABEL_OFFSET -2

SYTipButton::SYTipButton(QWidget *parent)
	: QPushButton(parent)
{
	m_TipStr = "";
}


SYTipButton::~SYTipButton(void)
{
}

void SYTipButton::paintEvent(QPaintEvent * event)
{
	QPushButton::paintEvent(event);

	if (m_TipStr != "")
	{
		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing, true);
		QRect rt = rect();
		QRect rtCircle = QRect(rt.right() + LABEL_OFFSET - LABEL_WIDTH, rt.top() - LABEL_OFFSET, LABEL_WIDTH, LABEL_WIDTH);


		painter.setPen(QColor(34, 200, 116, 255));
		painter.setBrush(QBrush(QColor(34, 200, 116, 255)));
		painter.drawEllipse(rtCircle);

		painter.setPen(Qt::white);
		painter.drawText(rtCircle, Qt::AlignCenter, m_TipStr);
	}
}

void SYTipButton::SetTipStr(const QString & tipStr)
{
	m_TipStr = tipStr;
	update();
}
#pragma once
#include "RbShieldLayer.h"
#include "ui_RbScreenshotDisplayer.h"
#include <QVector>
#include <QPixmap>
#include <QSet>
#include <QPoint>

class MxScreenshotDisplayer : public RbShieldLayer
{
	Q_OBJECT
public:
	MxScreenshotDisplayer(QWidget* parent = nullptr);

	~MxScreenshotDisplayer();

	void setScoreId(int scoreId);

	void updateContent();

	void clearContent();

protected:
	bool eventFilter(QObject *, QEvent *);

private slots:
	void onClickedSmallPicture();

private:
	Ui::ScreenshotDisplayer ui;
	int m_scoreId;
	QSet<QPushButton*> m_allButtons;
	QVector<QPixmap> m_pixmaps;
	int m_selectedPixmapIndex;

	bool m_hasMoved;
	QPoint m_pressedPoint;
	int m_oldValue;
};


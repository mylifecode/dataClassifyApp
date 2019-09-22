#pragma once
#include <QWidget>
#include "ui_SYScreenshotDisplayer.h"
#include "ui_SYFullPictureViewWindow.h"
#include <QVector>
#include <QPixmap>
#include "RbShieldLayer.h"

class SYFullPictureViewWindow;

class SYScreenshotDisplayer : public QWidget
{
	Q_OBJECT
public:
	SYScreenshotDisplayer(QWidget* parent = nullptr);

	~SYScreenshotDisplayer();

	void setScoreId(int scoreId);

signals:
	void back();

private slots:
	void on_backBtn_clicked();

	void on_pictureBtn_clicked();

protected:
	bool eventFilter(QObject* obj, QEvent* event);

private:
	Ui::SYScreenshotDisplayer ui;
	int m_scoreId;

	QVector<QPushButton*> m_buttons;
	QVector<QPixmap> m_pixmaps;

	SYFullPictureViewWindow* m_fullPictureViewWindow;
};

class SYFullPictureViewWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	SYFullPictureViewWindow(QWidget* parent = nullptr);

	~SYFullPictureViewWindow();

	void setPixmaps(const QVector<QPixmap>& pixmaps,int curIndex = 0);

protected:
	bool eventFilter(QObject* obj, QEvent* event);

private slots:
	void on_prePicBtn_clicked();

	void on_nextPicBtn_clicked();

	void on_closeBtn_clicked();

private:
	Ui::FullPictureViewWindow ui;

	QVector<QPixmap> m_pixmaps;
	int m_curPixmapIndex;

	QPushButton* m_closeBtn;
};
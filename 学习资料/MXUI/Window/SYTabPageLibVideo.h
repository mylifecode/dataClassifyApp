#pragma once
#include "ui_SYTabPageLibVideo.h"
#include <QPushButton>

class RbMoviePlayer;

class SYTabPageLibVideo : public QWidget
{
	Q_OBJECT
public:
	SYTabPageLibVideo(QWidget * parent);

	~SYTabPageLibVideo(void);

	void setVideos(const QVector<QString>& videoFiles);

private slots:
	void onClieckedVideoBtn();

private:
	RbMoviePlayer* m_videoPlayer;

	QVector<QPushButton*> m_videoButtons;

	Ui::SYTabPageLibVideo ui;
};
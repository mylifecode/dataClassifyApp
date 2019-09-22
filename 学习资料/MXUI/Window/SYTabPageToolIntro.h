#pragma once
#include "ui_SYTabPageToolIntro.h"
#include  <QDialog>

class RbMoviePlayer;

class SYTabPageToolIntro : public QWidget
{
	Q_OBJECT
public:
	SYTabPageToolIntro(QWidget * parent);

	~SYTabPageToolIntro(void);

	void setToolInfo(const QString& toolName, const QString& videoFileName, const QString& description);

private slots:
	void on_playBtn_clicked();


private:
	Ui::SYTabPageToolIntro ui;

	QString m_toolName;
	QString m_videoFileName;
	QString m_description;

	RbMoviePlayer* m_moviePlayer;
};
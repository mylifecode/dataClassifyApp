#ifndef RBTOOLBUTTON_H
#define RBTOOLBUTTON_H

#include "StdAfx.h"
#include <QToolButton>
#include <QString>
#include <QApplication>
#include <QSound>
#include "MxGlobalConfig.h"

class RbToolButton : public QToolButton
{

	Q_OBJECT

public:

	RbToolButton( QWidget *parent = NULL )
		:QToolButton(parent)
	{
	}

	inline void setTaskPicFile( QString & strTaskPicFile ) {  m_strTaskPicFile = strTaskPicFile; }
	inline void setTrainingName( QString & strTrainingName ) {  m_strTrainingName = strTrainingName; }
	inline void setHelp( QString & strHelp ) {  m_strHelp = strHelp; }

	inline QString getTaskPicFile() { return m_strTaskPicFile; }
	inline QString getTrainingName() { return m_strTrainingName; }
	inline QString getHelp() { return m_strHelp; }

signals:

	void hover();

	void leave();

protected:

	void enterEvent ( QEvent * event )
	{
		emit hover();
	}

	void leaveEvent ( QEvent * event )
	{
		emit leave();
	}

	void mousePressEvent (  QMouseEvent  * e )
	{
		QSound::play(MxGlobalConfig::Instance()->GetAudioDir() + "/Button54.wav" ); 
		__super::mousePressEvent( e );
	}

private:
	QString m_strTaskPicFile;
	QString m_strTrainingName;
	QString m_strHelp;
};
#endif
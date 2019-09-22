#ifndef RBPUSHBUTTON_H
#define RBPUSHBUTTON_H
#include  <QSound>
#include <QPushButton>
#include <QApplication>
#include "MxGlobalConfig.h"


class RbPushButton : public QPushButton
{
	Q_OBJECT
public:
	
	RbPushButton( QWidget *parent = NULL )
		:QPushButton( parent )
	{
	}

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

	void clicked ( bool checked = false )
	{
		emit __super::clicked();
	}

	void mousePressEvent (  QMouseEvent  * e )
	{
		QSound::play(MxGlobalConfig::Instance()->GetAudioDir() + "/Button54.wav" ); 
		__super::mousePressEvent( e );
	}
};
#endif
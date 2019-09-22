#ifndef RBCLICKLABEL_H
#define RBCLICKLABEL_H


#include <QLabel>

class MyLabel : public QLabel
{
	Q_OBJECT
public:
	MyLabel(QWidget *parent = NULL):QLabel(parent)
	{
		//setWindowFlags(Qt::FramelessWindowHint);
		//setAttribute(Qt::WA_TranslucentBackground);
	}
	~MyLabel()
	{
		
	}

signals:
	void clicked();
protected:
	void showEvent( QShowEvent * event )
	{
		if ( this->isVisible())
			this->repaint();
	}
	void mouseReleaseEvent ( QMouseEvent * ev )
	{
		emit clicked();
	}
};
#endif
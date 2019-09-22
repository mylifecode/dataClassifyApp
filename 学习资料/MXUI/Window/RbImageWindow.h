#pragma once
#include <QFrame>
#include <QVector>

class QImage;
class QPaintEvent;
class QScrollBar;
class QPixmap;

class RbImageWindow : public QFrame
{
	Q_OBJECT
public:
	RbImageWindow(QWidget * parent = NULL);
	~RbImageWindow(void);

	bool addImage(const QString& fileName);
	void scrollaction(int);
protected slots:
	void onHSliderValueChanged(int value);
	void onVSliderValueChanged(int value);
protected:
	bool event(QEvent *);
	void paintEvent(QPaintEvent *);
private:
	QScrollBar * m_pHScrollBar;
	QScrollBar * m_pVScrollBar;
	
	//QVector<QImage*> m_vecImage;
	QVector<QPixmap*> m_vecImage;
	int m_offsetX;
	int m_offsetY;
	int m_contentWidth;
	int m_contentHeight;
};

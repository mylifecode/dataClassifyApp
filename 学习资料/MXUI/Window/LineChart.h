#ifndef LINECHART_H
#define LINECHART_H

#include <QWidget>
#include <QPen>
#include <QPainter> 
#include <QTimer>
class LineChart : public QWidget
{
	Q_OBJECT

public:
	LineChart(QWidget *parent = 0);
	~LineChart();

private:
	void DrawAxis();

	void DrawGrid();

	void DrawLines(const QList<QPointF> & datalist,const QColor & color);
	void DrawMarks(const QString name,const QColor & color,int index);
    //void DrawData(const QPoint & point , const QColor & color , float data);

public:
	void setAxisPen( const QPen & pen );
	void setScalePen( const QPen & pen );
	void setGridPen( const QPen & pen );
	void setBackImage(const QString & file);
	void setFont(const QFont & font);
    void setGradual(bool bGradual);
	void setRangeValueX(float min,float max );
	void setRangeValueY(float min,float max );
	void addObject(const QString & name, const QList<QPointF> list,const QColor & color);
    void clear();

	void Draw(float interval=50);
	void setCurrentDot(int id){ m_CurScoreDot = id;}
private:
	void axisRange();
    void paintEvent(QPaintEvent * evt);
    void showEvent(QShowEvent * evt);

private slots: 
	void on_TimeOut();

private:
	QList<QList<QPointF>> objList;
	QList<QColor> colorList;
	QList<QString> nameList;
	
    QPen m_AxisPen;
    QPen m_ScalePen;
	QPen m_GridPen;
	QPoint m_StartPos;
	QRect m_Area;

	QFont m_font;

	float x_unit;
	float y_unit;

	float  x_min;
	float  x_max;
	float  y_min;	
	float  y_max;

	QTimer * m_TickTimer;
	int m_nPoint;

	bool m_bGradual;
	bool m_bTimerEvent;
	bool m_bStartDraw;
	int m_CurScoreDot;
};

#endif // LINECHART_H

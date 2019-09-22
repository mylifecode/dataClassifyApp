#include "LineChart.h"
#include <QLabel>
#include <QDebug>
#include <QHBoxLayout>

#define  POINTS 10

LineChart::LineChart(QWidget *parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::Widget);
	m_AxisPen.setColor(QColor(213,113,113,255));
	m_AxisPen.setWidth(3); 
	m_AxisPen.setJoinStyle (Qt::RoundJoin);
	m_ScalePen.setColor(QColor(0,0,0));
	m_ScalePen.setWidth(2); 
	m_ScalePen.setJoinStyle (Qt::RoundJoin);
	
	m_GridPen.setColor(QColor(100,100,200,255));
	m_GridPen.setWidth(1); 
	m_GridPen.setJoinStyle (Qt::RoundJoin);

	m_TickTimer = new QTimer(this);
	m_nPoint = 0;
	connect(m_TickTimer, SIGNAL(timeout()), this, SLOT(on_TimeOut()));

	m_font = QFont("Times", 12, QFont::Light);	

	m_bGradual = true;
	m_bTimerEvent = true;
    m_bStartDraw = false;
	 x_unit = 1;
	 y_unit = 1;
	 x_min = x_max = 1;
	x_max =y_min=y_max= 0;
	 m_CurScoreDot = 9;
}

LineChart::~LineChart()
{

}

void LineChart::DrawAxis()
{
	QPainter painter(this);
	painter.setFont (m_font);
	painter.setPen(m_AxisPen);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.drawLine(m_Area.topLeft(),QPoint(m_Area.topLeft().x()-5,m_Area.topLeft().y()+5));
	painter.drawLine(m_Area.topLeft(),QPoint(m_Area.topLeft().x()+5,m_Area.topLeft().y()+5));
	painter.drawLine(m_Area.topLeft(),m_Area.bottomLeft());
	painter.drawLine(m_Area.bottomLeft(),m_Area.bottomRight());

	painter.drawLine(m_Area.bottomRight(),QPoint(m_Area.bottomRight().x()-5,m_Area.bottomRight().y()-5));
	painter.drawLine(m_Area.bottomRight(),QPoint(m_Area.bottomRight().x()-5,m_Area.bottomRight().y()+5));


	//not yet start draw
	if(!m_bStartDraw)  return; 

	//draw  x axis Scale
	for (int i=0;i<POINTS;i++)
	{
		int x = m_Area.bottomLeft().x()+i*m_Area.width()/POINTS;
		int y =  m_Area.bottomLeft().y();
		painter.drawLine(QPoint(x,y),QPoint(x,y+5));

		//if (i%2==0)
		{
			painter.drawText(QPoint(x-3,y+20), QString("%1").arg(x_min+x_unit*i));
		}
	}
	//draw  y axis Scale
	for (int i=1;i<POINTS;i++)
	{
	    int x =  m_Area.bottomLeft().x();
		int y = m_Area.bottomLeft().y()-i*m_Area.height()/POINTS;
		painter.drawLine(QPoint(x,y),QPoint(x-5,y));

		if (i%2)
		{
            painter.drawText(QPoint(x-40,y+3), QString("%1").arg(y_min+y_unit*i));
		}
	}
}

void LineChart::paintEvent( QPaintEvent * evt )
{
	m_Area = QRect(50,10,rect().width()-220,rect().height()-50);

	DrawAxis();
	DrawGrid();
	for(int i =0;i<objList.size();i++)
	{
		DrawLines(objList[i],colorList[i]);
		DrawMarks(nameList[i],colorList[i],i);
	}
}

void  LineChart::axisRange()
{
	if(objList.isEmpty() )  return;
	if(objList[0].isEmpty() )  return;

	x_min = x_max = objList[0][0].x();
	y_min = y_max = objList[0][0].y();
	foreach(QList<QPointF> list,objList)
	{
		foreach(QPointF point,list)
		{
			if (point.x()<x_min) x_min = point.x();
			if (point.x()>x_max) x_max = point.x();
			if (point.y()<y_min) y_min = point.y();
			if (point.y()>y_max) y_max = point.y();
		}
	}

	float lenght = (int)(x_max-x_min)+(int)(x_max-x_min)%POINTS;
    x_unit = (int)lenght/POINTS;
	x_unit = (x_unit==0) ? 1:x_unit;

	lenght = (int)(y_max-y_min)+(int)(y_max-y_min)%POINTS;

	if (lenght<10.0f)
	{
		y_unit = lenght/POINTS;
	}
	else
	{
		y_unit = (int)lenght/POINTS;
	}
	y_unit =((int)y_unit%5==0)?y_unit:(y_unit+5-(int)y_unit%5);
	y_min = y_min-(int)y_min%5;
}

void LineChart::on_TimeOut()
{
	repaint();
	if (m_nPoint>POINTS-1 || objList.isEmpty())  
	{
		m_nPoint = 0;
		m_bTimerEvent = false;
		m_TickTimer->stop();
	}
	else
	{
		m_nPoint++;
		m_bTimerEvent = true;
	}
}

void LineChart::DrawGrid()
{
	QPainter painter(this);
	painter.setFont (m_font);
	painter.setPen(m_GridPen);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(QBrush(QColor(0,0,0)));

    QPointF ogrin = m_Area.bottomLeft();
	int dx_unit = m_Area.width()/POINTS;
	int dy_unit = m_Area.height()/POINTS;
	for (int i=1;i<POINTS;i++)
	{
		painter.drawLine(QPointF(ogrin.x()+dx_unit*i,ogrin.y()),QPointF(ogrin.x()+dx_unit*i,ogrin.y()-dy_unit*POINTS));
		painter.drawLine(QPointF(ogrin.x(),ogrin.y()-dy_unit*i),QPointF(ogrin.x()+dx_unit*POINTS,ogrin.y()-dy_unit*i));
	}
}

void LineChart::DrawMarks(const QString name,const QColor & color,int index)
{
	QPainter painter(this);
	QPen pen;
	painter.setFont (m_font);
	pen.setColor(color);
	pen.setWidth(5); 
	pen.setJoinStyle (Qt::RoundJoin);
	painter.setPen(pen);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(QBrush(color));

	painter.drawRect(QRect(m_Area.topRight().x()+10,m_Area.topRight().y()+30*index,10,10));
	painter.drawText(QPoint(m_Area.topRight().x()+30,m_Area.topRight().y()+30*index+10), name);
}

void LineChart::DrawLines(const QList<QPointF> & dataList,const QColor & color)
{
	QPainter painter(this);
	QPen pen;
	painter.setFont (m_font);
	pen.setColor(color);
	pen.setWidth(3); 
	pen.setJoinStyle (Qt::RoundJoin);
	painter.setPen(pen);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.translate(m_Area.bottomLeft());
	painter.setBrushOrigin (QPoint(x_min,y_min));
	painter.rotate(270.0);

	//draw line
	if (dataList.size()<2) return;

	int max = m_nPoint>dataList.size() ? dataList.size() :m_nPoint;
    if (!m_bTimerEvent)  max = dataList.size();
	if (!m_bGradual) max = dataList.size();

	for (int i=1;i<max;i++)
	{
		QPointF A,B;
		A.setY(m_Area.width()*(dataList[i].x()-x_min)*0.1/x_unit);
		A.setX(m_Area.height()*(dataList[i].y()-y_min)*0.1/y_unit);
		B.setY(m_Area.width()*(dataList[i-1].x()-x_min)*0.1/x_unit);
		B.setX(m_Area.height()*(dataList[i-1].y()-y_min)*0.1/y_unit);
		painter.drawLine(A,B);

		int radius = 2;
		if ( i==m_CurScoreDot) radius = 6.4;
		painter.drawEllipse(A,radius,radius);
	}
	if (m_CurScoreDot==0)
	{
		QPointF A;
		A.setY(m_Area.width()*(dataList[0].x()-x_min)*0.1/x_unit);
		A.setX(m_Area.height()*(dataList[0].y()-y_min)*0.1/y_unit);
		int radius = 6.4;
		painter.drawEllipse(A,radius,radius);
	}
}

void LineChart::addObject( const QString & name,const QList<QPointF> list,const QColor & color )
{
	objList.push_back(list);
	colorList.push_back(color);
	nameList.push_back(name);
}

void LineChart::Draw(float interval )
{
	axisRange();

	m_TickTimer->start(interval);

	m_bStartDraw = true;
}

void LineChart::setAxisPen( const QPen & pen )
{
	m_AxisPen = pen;
}

void LineChart::setScalePen( const QPen & pen )
{
	m_ScalePen = pen;
}

void LineChart::setBackImage( const QString & file )
{
	setStyleSheet(QString("border-image:url(%1)").arg(file));
}

void LineChart::setGridPen( const QPen & pen )
{
	m_GridPen = pen;
}

void LineChart::showEvent( QShowEvent * evt )
{
     Draw();
}

void LineChart::setFont( const QFont & font )
{
	m_font = font;
}

void LineChart::setGradual( bool bGradual )
{
    m_bGradual = bGradual;
}

void LineChart::clear()
{
	objList.clear();
	colorList.clear();
	nameList.clear();
	m_TickTimer->stop();
	m_nPoint = 0;
	m_font = QFont("Times", 12, QFont::Light);	
	m_bGradual = true;
	m_bTimerEvent = true;
}

void LineChart::setRangeValueX( float min,float max )
{
	x_min = min;
	x_max = max	;
}

void LineChart::setRangeValueY( float min,float max )
{
	y_min = min;
	y_max = max	;
}
//void LineChart::DrawData( const QPoint & point , const QColor & color , float data )
//{
//	QPainter painter(this);
//	QPen pen;
//	pen.setColor(color);
//	painter.setPen(pen);
//	painter.setFont (m_font);
//	painter.setRenderHint(QPainter::Antialiasing, true);
//	painter.translate(m_Area.topLeft());
//	painter.drawText(point, QString("%1").arg(data));
//}
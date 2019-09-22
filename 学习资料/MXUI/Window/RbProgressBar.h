#ifndef RBPROGRESSBAR_H
#define RBPROGRESSBAR_H

#include <QWidget>
#include <QString>
#include <QVector>

class QPixmap;
class QEvent;
class QPaintEvent;

class RbProgressBar : public QWidget
{
    Q_OBJECT
		typedef void (*CALLBACKFUNCTION)();
public:
    explicit RbProgressBar(const QString & strBackBarPictureName,
                           const QString& strFrontBarPictrueName,
                           int progressNum = 10,
						   CALLBACKFUNCTION pCallBack = NULL,
                           QWidget *parent = 0);

signals:

public slots:

public:
	bool addProgress(unsigned int numStep = 1);
protected:
    bool event(QEvent *pEvent);
    void paintEvent(QPaintEvent  * pEvent);
private:
    int m_numCurProgress;
    int m_numTotalProgress;
    QPixmap * m_pPixmapBackBar;
    QPixmap * m_pPixmapFrontBar;

    int m_pixmapWidth;
    int m_pixmapHeight;
    QVector<bool> m_vecBDrawBackBar;
	CALLBACKFUNCTION m_pCallBack;
};

#endif // RBPROGRESSBAR_H

#ifndef OGREWIDGET_H
#define OGREWIDGET_H

#include <OgreCommon.h>
#include <QWidget>
#include <QVector>

#define GLOBAL_VIEW     0
#define PART_VIEW    1

class QSettings;

/**
 * Widget holding Ogre
 * 
 * This widget is used to hold and contain the Ogre RenderWindow
 * 
 * \author David Williams
 */
class OgreWidgetEventListener
{
public:
	virtual void OnMouseReleased(char button , int x , int y) = 0;
	virtual void OnMousePressed(char button , int x , int y) = 0;
	virtual void OnMouseMoved(char button , int x , int y) = 0;
	virtual void OnWheelEvent(int delta) {}
	virtual void OnKeyPress(int whichButton){};
	virtual void OnKeyRelease(int whichButton){};
};

class OgreWidget : public QWidget
{
	Q_OBJECT

public:
	OgreWidget(QWidget* parent=0, Qt::WindowFlags f=0);
	~OgreWidget();

	Ogre::RenderWindow* getOgreRenderWindow() const;

	//Other
	void initialise(int viewType, std::string renderWindowName, const Ogre::NameValuePairList *miscParams = 0);
	int  GetViewType();

	void Destroy();//not delete just destory ogre relevant resource and mark as invalid
public:
	QPaintEngine *paintEngine() const;
	void paintEvent(QPaintEvent* evt);
	void resizeEvent(QResizeEvent* evt);
	void keyPressEvent (QKeyEvent * evt);
	void keyReleaseEvent (QKeyEvent * evt);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);

	void AddListener(OgreWidgetEventListener * listener);
	void RemoveListener(OgreWidgetEventListener* listener);

public:
	Ogre::RenderWindow* m_pOgreRenderWindow;

private:
	bool mIsInitialised;
	int mViewType;
	bool m_lockmouse;

	QVector<OgreWidgetEventListener*> m_eventListener;
};

#endif /*QTOGRE_OGREWIDGET_H_*/

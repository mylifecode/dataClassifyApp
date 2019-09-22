#include "StdAfx.h"
#include "OgreWidget.h"
#include <qpainter.h>

//#include "Application.h"
//#include "EventHandler.h"
//#include "Log.h"

#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreStringConverter.h>

#include <QSettings>
#include <QKeyEvent>
#include "InputSystem.h"
#include "KeyboardInput.h"
#include "MouseInput.h"
#include "PathputTool.h"

// #define GLOBAL_VIEW     1
// #define PART_VIEW   0

#if defined(Q_WS_X11)
#include <QX11Info>
#endif
#include "SimpleUI.h"

OgreWidget::OgreWidget(QWidget* parent, Qt::WindowFlags f)
	:QWidget(parent, f | Qt::MSWindowsOwnDC)
	,m_pOgreRenderWindow(0)
	,mIsInitialised(false)
	,mViewType(0)
	//,m_windoweventlistener(0)
	//,m_windowevent_listener_veineditor(0)
	//,m_windowevent_listener_veineditorV2(0)
	//,m_listener_for_training(0)
{
	QPalette colourPalette = palette();
	colourPalette.setColor(QPalette::Active, QPalette::WindowText, Qt::black);
	colourPalette.setColor(QPalette::Active, QPalette::Window, Qt::black);
	setPalette(colourPalette);

	setAutoFillBackground(true);
	setMouseTracking(true); 

	m_lockmouse = false;
}

OgreWidget::~OgreWidget()
{
	Destroy();
	MXOgreWrapper::Instance()->DetachOgreWidget(this);
}
void OgreWidget::Destroy()
{
	mIsInitialised = false;
}

void OgreWidget::initialise(int viewType, std::string renderWindowName, const Ogre::NameValuePairList *miscParams)
{
	if(MXOgreWrapper::Instance()->GetOgreRoot()==NULL)
	{
		MXOgreWrapper::Instance()->CreateOgreRoot();
	}
	mViewType = viewType;
	//These attributes are the same as those use in a QGLWidget
	//setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

	//Parameters to pass to Ogre::Root::createRenderWindow()
	Ogre::NameValuePairList params;
	params["useNVPerfHUD"] = "false";
	params["FSAA"]= "0";

	//If the user passed in any parameters then be sure to copy them into our own parameter set.
	//NOTE: Many of the parameters the user can supply (left, top, border, etc) will be ignored
	//as they are overridden by Qt. Some are still useful (such as FSAA).
	if(miscParams != 0)
	{
		params.insert(miscParams->begin(), miscParams->end());
	}

	//The external windows handle parameters are platform-specific
	Ogre::String externalWindowHandleParams;

	//Accept input focus
	//setFocusPolicy(Qt::StrongFocus);
	setFocus();

	//positive integer for W32 (HWND handle) - According to Ogre Docs
	externalWindowHandleParams = Ogre::StringConverter::toString((unsigned int)(winId()));
	params["externalWindowHandle"] = externalWindowHandleParams;

	//Finally create our window.
	try
	{
		m_pOgreRenderWindow = Ogre::Root::getSingletonPtr()->createRenderWindow(renderWindowName, width(), height(), false, &params);
		Ogre::Root::getSingletonPtr()->getRenderSystem()->attachRenderTarget(*m_pOgreRenderWindow);

		mIsInitialised = true;
	}
	catch (Ogre::Exception &e)
	{
		QString error
			(
			"Failed to create the RenderWindow.\n\n"
			"The message returned by Ogre was:\n\n"
			);
		error += QString::fromStdString(e.getFullDescription().c_str());

		qCritical(error.toStdString().c_str());

		mIsInitialised = false;
	}
}

Ogre::RenderWindow* OgreWidget::getOgreRenderWindow() const
{
	return m_pOgreRenderWindow;
}

QPaintEngine *OgreWidget:: paintEngine() const
{
	return 0;
}

void OgreWidget::paintEvent(QPaintEvent* evt)
{
	if(mIsInitialised)
	{
		//Ogre::Root::getSingleton()._fireFrameStarted();
		//m_pOgreRenderWindow->update();
		//Ogre::Root::getSingleton()._fireFrameRenderingQueued();
		//Ogre::Root::getSingleton()._fireFrameEnded();
	}
	//RECT rect;
	//GetClientRect((HWND)(this->winId()),&rect);
	//int width = this->width();
	//int height = this->height();
	//QPainter painter(this);
	//painter.setPen(Qt::blue);
	//painter.drawText(0,height- 30,QString("111111111111111111111111111111111111111111111111111111"));
}

void OgreWidget::resizeEvent(QResizeEvent* evt)
{
	if(m_pOgreRenderWindow)
	{
		int width = this->width();
		int height = this->height();
		m_pOgreRenderWindow->resize(width,height);
		m_pOgreRenderWindow->windowMovedOrResized();

		for(int ct = 0; ct < m_pOgreRenderWindow->getNumViewports(); ++ct)
		{
			Ogre::Viewport* pViewport = m_pOgreRenderWindow->getViewport(ct);
			Ogre::Camera* pCamera = pViewport->getCamera();
			pCamera->setAspectRatio(static_cast<Ogre::Real>(pViewport->getActualWidth()) / static_cast<Ogre::Real>(pViewport->getActualHeight()));
		}
	}
}

void OgreWidget::keyPressEvent (QKeyEvent * evt)
{
	if(mIsInitialised == false)
	   return;

	if(InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice()->KeyPress(evt->key());
	
	if(InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->KeyPress(evt->key());

	if(InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->KeyPress(evt->key());

	if(InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetDevice()->KeyPress(evt->key());

	if(evt->key() == Qt::Key_Space)
		m_lockmouse = !m_lockmouse;

	for (int i = 0; i < m_eventListener.size(); ++i)
		m_eventListener[i]->OnKeyPress(evt->key());

}

void OgreWidget::keyReleaseEvent (QKeyEvent * evt)
{
	if(mIsInitialised == false)
		return;

	if(InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice()->KeyRelease(evt->key());

	if(InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->KeyRelease(evt->key());

	if(InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->KeyRelease(evt->key());

	if(InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetDevice())
	   InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetDevice()->KeyRelease(evt->key());
	
	if (evt->key() == Qt::Key_Delete && PathPutTool::GetCurrentTool())
	{
		PathPutTool::GetCurrentTool()->RemoveSelectedCluster();
	}

	else if (evt->key() == Qt::Key_H && PathPutTool::GetCurrentTool())
	{
		PathPutTool::GetCurrentTool()->HideSelectedOgran();
		//cout << "Key Minus pressed " << endl;
	}

	for (int i = 0; i < m_eventListener.size(); ++i)
		m_eventListener[i]->OnKeyRelease(evt->key());
}
int  OgreWidget::GetViewType()
{
   return mViewType;
}

void OgreWidget::mousePressEvent( QMouseEvent *event )
{
	if(mIsInitialised == false)
		return;

	MouseInput::GetInstance()->mousePressEvent(event->buttons(), event->x(), event->y());

	for(int i = 0;i < m_eventListener.size();++i)
		m_eventListener[i]->OnMousePressed(event->buttons(), event->x(),  event->y());

	CSimpleUIManger::Instance()->OnMousePressed(event->buttons(), event->x(),  event->y());
}

void OgreWidget::mouseReleaseEvent(QMouseEvent *event)
{
	if(mIsInitialised == false)
		return;

	for(int i = 0;i < m_eventListener.size();++i)
		m_eventListener[i]->OnMouseReleased(event->buttons(), event->x(), event->y());

	CSimpleUIManger::Instance()->OnMouseReleased(event->buttons(), event->x(),  event->y());
}

void OgreWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	if(mIsInitialised == false)
		return;

	if (/*mViewType == GLOBAL_VIEW &&*/ m_lockmouse == false)
	{
		MouseInput::GetInstance()->mouseDoubleClickEvent(event->buttons(), event->x(), event->y());
	}

}

void OgreWidget::mouseMoveEvent( QMouseEvent *event )
{
	if(mIsInitialised == false)
		return;

	if (/*mViewType == GLOBAL_VIEW && */m_lockmouse == false)
	{
			MouseInput::GetInstance()->mouseMoveEvent(event->buttons(), event->x(), event->y());
	}

	for(int i = 0;i < m_eventListener.size();++i)
		m_eventListener[i]->OnMouseMoved(event->buttons(), event->x(),  event->y());
	
}

void OgreWidget::wheelEvent( QWheelEvent *event )
{
	if(mIsInitialised == false)
		return;

	if (/*mViewType == GLOBAL_VIEW && */m_lockmouse == false)
	{
		MouseInput::GetInstance()->wheelEvent(event->delta());
	}
	for(int i = 0;i < m_eventListener.size();++i)
		m_eventListener[i]->OnWheelEvent(event->delta());
}

void OgreWidget::AddListener(OgreWidgetEventListener * listener)
{
	m_eventListener.push_back(listener);
}

void OgreWidget::RemoveListener(OgreWidgetEventListener* listener)
{
	for(int i = 0;i < m_eventListener.size();++i)
	{
		if(m_eventListener[i] == listener)
		{
			m_eventListener.remove(i);
			break;
		}
	}
}
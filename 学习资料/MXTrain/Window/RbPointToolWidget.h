#ifndef _RBMATERIALDEBUGWIDGET_
#define _RBMATERIALDEBUGWIDGET_

#include "ogrewidget.h"
#include <QTime>
#include <QDialog>
#include <QLabel>
#include <QToolButton>
#include "ui_addpointputedit.h"
#include "OgreVector3.h"

class QStandardItemModel;
class ObjectLoadReleaseListener;

class CBasicTraining;
//class CDynamicObject;
class ToolEventListenr;


class RbPointToolWidget : public QDialog
{
	Q_OBJECT
public:
	RbPointToolWidget(QWidget *parent = NULL);

	~RbPointToolWidget();

	void updateObjectList();

	void updateConnectionList();

	void onDynamicObjectChanged();

	void onCurrentTrainChanged(CBasicTraining * ptrain);

	void keyPressEvent(QKeyEvent *event);

	void InitLightSet();

    void InitCameraSet();

	void showEvent(QShowEvent *);
public:
	ObjectLoadReleaseListener * m_listener;

	ToolEventListenr * m_tooleventlistener;

	OgreWidget *smallGLWidget;

	QStandardItemModel * m_objlistmodel;

	QStandardItemModel * m_pathlistmodel;

	QStandardItemModel * m_connectlistmodel;

	CBasicTraining * m_registerdtrain;

	//CDynamicObject * m_dynobjsel;

	Ogre::String m_selshaderparam;

	bool m_inreadlight;
public slots:
		virtual void on_close(){};
		virtual void on_saveClicked();
		virtual void on_autoRebuildConnect();
        virtual void on_AutoGenConnect();
		virtual void on_EditSelButtonClicked();

		virtual void seralizestrip();
		virtual void genTexmap();
		virtual void reset();
		virtual void on_delselconnect();
		virtual void on_refreshpathClicked();
		virtual void on_deletepathClicked();

		virtual void Objlistclicked(const QModelIndex &index);
		virtual void PointPathlistclicked(const QModelIndex &index);
		virtual void ConnectListclicked(const QModelIndex &index);

		virtual void on_putdensityTextChanged(const QString &);

		virtual void on_valueChanged(int value);
		virtual void on_changeAdheHeight(int value);
		virtual void on_changeAdheHeightA(int value);
		virtual void on_changeAdheHeightB(int value);
		virtual void cb_unselstateChanged(int value);
		virtual void cb_selstateChanged(int value);
//light
		virtual void SetLight();
		virtual void onSetPosX(int value);
		virtual void onSetPosY(int value);
		virtual void onSetPosZ(int value);
		virtual void onSetDirX(int value);
		virtual void onSetDirY(int value);
		virtual void onSetDirZ(int value);
		virtual void onSetDiffuseR(int value);
		virtual void onSetDiffuseG(int value);
		virtual void onSetDiffuseB(int value);
		virtual void onSetSpecularR(int value);
		virtual void onSetSpecularG(int value);
		virtual void onSetSpecularB(int value);
		virtual void onSetAtteRange(int value);
		virtual void onSetAtteConstant(int value);
		virtual void onSetAtteLinear(int value);
		virtual void onSetAttequadraticui(int value);
		virtual void onSetInnerAngle(int value);
		virtual void onSetOuterAngle(int value);
		virtual void onSetFallOff(int value);
		virtual void onLightTypeChanged(int value);

		virtual void onSetAmbientR(int value);
		virtual void onSetAmbientG(int value);
		virtual void onSetAmbientB(int value);

//camera
public slots:
    void onSetCamPosX(int value);
	void onEditCamPosX();
    void onSetCamPosY(int value);
    void onSetCamPosZ(int value);

    void onSetCamDirX(int value);
    void onSetCamDirY(int value);
    void onSetCamDirZ(int value);

    void onSetFovy(int value);

    void onRestoreButtonClicked();

    void onSetOrthoWindow_w(int value);
    void onSetOrthoWindow_h(int value);

    void onSetAspectRatio(int value);


private:
    Ogre::Camera* m_pCamera;
    Ogre::Vector3 m_initCamPos;
    Ogre::Vector3 m_initCamDir;
    float m_fInitAspectRatio;
    Ogre::Radian m_camFovy_radian;


protected:
		Ui::addpointputedit ui;

		QFrame* m_bg;

};

#endif
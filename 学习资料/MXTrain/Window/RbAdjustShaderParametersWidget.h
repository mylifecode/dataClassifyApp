#pragma once
#include "../MXRobot/Event/MxEventsDump.h"
#include "../MXRobot/NewTrain/MisMedicOrganOrdinary.h"
#include "../MXRobot/NewTrain/MisNewTraining.h"

#include "ogrewidget.h"
#include <QTime>
#include <QDialog>
#include <QLabel>
#include <QToolButton>
#include "ui_RbAdjustShaderParametersWindow.h"
#include "OgreVector3.h"

#include "./collision/BroadPhase/GoPhysDynBVTree.h"

class QStandardItemModel;
class ObjectLoadReleaseListener;

class CBasicTraining;

struct ShaderParams
{
	Ogre::ColourValue AmbientColor;
	Ogre::ColourValue DiffuseColor;
	Ogre::ColourValue SpecularColor;

	float Rhod;
	float Rhos;
	float Rhots;
	float TransPower;

	float Ward_Alphax;
	float Ward_Alphay;

	float EnvFresnelScale;
	float EnvFresnelExp;
	float EnvScale;

	std::map<Ogre::String , float> m_Params;
};


class RbAdjustShaderParametersWidget: public QDialog
{
	Q_OBJECT
public:
	RbAdjustShaderParametersWidget(QWidget *parent = NULL);

	~RbAdjustShaderParametersWidget();

	void UpdateDynObjList();

	void ShowDebugInfo(const QString& str);
//	void onDynamicObjectChanged();

	void OnCurrentTrainChanged(CBasicTraining * ptrain);
	
public:
	QStandardItemModel * m_dynobj_list_model;

	CBasicTraining * m_registerd_train;

	ObjectLoadReleaseListener *m_pObjectLoadListener;


public slots:
	void OnDynObjListClicked(const QModelIndex &index);
	
	void OnFrameVisibleBtnClicked();

	void OnParamsValueChanged(int value);

	void OnLineEditTextChanged(const QString &text);

	void timerUpdate();

protected:
	Ui::ShaderParamsEditor ui;

private:
	void GetShaderParams(Ogre::MaterialPtr matPtr);

	void ShowShaderParams();

	void SetFpParameterValue(Ogre::MaterialPtr matPtr , const char * paramName, float paramValue);

	void DrawSelectedOrgan();

	int m_SelectedOrganID;

	std::map<int , Ogre::MaterialPtr> m_OrganMaterialPtrs;

	Ogre::MaterialPtr m_CurrMatPtr;

	Ogre::MaterialPtr m_FinalToneMappingMatPtr;

	Ogre::MaterialPtr m_BloomMatPtr;

	//ssss
	Ogre::MaterialPtr m_SSSSBlurXMatPtr;
	Ogre::MaterialPtr m_SSSSBlurYMatPtr;




	Ogre::ManualObject *m_pManualForSelectedOrgan;

	MisMedicOrgan_Ordinary *m_pSelectedOrgan;

	QTimer *m_Timer;

	bool m_IsShowOrganFrame;

	ShaderParams m_CurrShaderParams;

	bool m_IsGetMat;

	float m_CommonScale;

};

#pragma once
#include "../MXRobot/NewTrain/MisMedicOrganOrdinary.h"
#include "../MXRobot/NewTrain/MisNewTraining.h"
#include "ogrewidget.h"
#include <QTime>
#include <QDialog>
#include <QLabel>
#include <QToolButton>
#include "ui_RbHotchpotchEditorWindow.h"
#include "OgreVector3.h"

#include "../MXRobot/EditorTool/HotchpotchEditor.h"
#include "../MXRobot/EditorTool/DrawTool.h"


class QStandardItemModel;
class ObjectLoadReleaseListener;
class CBasicTraining;


class RbHotchpotchEditorWidget: public QDialog
{
	Q_OBJECT
public:
	RbHotchpotchEditorWidget(QWidget *parent = NULL);

	~RbHotchpotchEditorWidget();

	void UpdateDynObjList();

	void UpdateViewDetectionObjList();

	void ShowDebugInfo(const QString& str);
//	void onDynamicObjectChanged();

	void OnCurrentTrainChanged(CBasicTraining * ptrain);
	
public:
	QStandardItemModel * m_DynObjListModel;

	QStandardItemModel * m_ViewDetectionObjListModel;

	CBasicTraining * m_pRegisterdTraining;

	//ObjectLoadReleaseListener *m_pObjectLoadListener;


public slots:
//list
	void OnDynObjListClicked(const QModelIndex &index);

	void OnViewDetectionObjListClicked(const QModelIndex &index);

//btn
	void OnFrameVisibleBtnClicked();

	void OnOpenEditorBtnClicked();

	void OnOpenTrainingEditModeBtnClicked();
	
	//about organ
	void OnExportOrganObjBtnClicked();


	//about vd
	void OnAddVdBtnClicked();

	void OnChangeVdBtnClicked();

	void OnSetPosBtnClicked();

	//about doodle
	void OnAddDoodleBtnClicked();

	void OnSaveDoodleBtnClicked();

	void OnLoadImageIntoDoodleBtnClicked();

	void OnToggleShowEdgeBtnClicked();


//slider
	void OnStepSliderValueChanged(int value);

//LineEdit
	void OnStepScaleLineEditTextChanged(const QString &text);
	void OnPosLineEditTextChanged(const QString &text);
	void OnBrushRadiusSliderChanged(int value);


	void timerUpdate();

protected:
	Ui::HotchpotchEditorWidget ui;

private:
	int m_SelectedOrganID;

	int m_VdObjIndex;

	Ogre::Vector3 m_CommonPos;

	float m_CommonStep;

	float m_StepBasic;

	float m_StepScale;

	Ogre::ColourValue m_BrushColor;
	uint32 m_RComponent;
	uint32 m_GComponent;
	uint32 m_BComponent;
	uint32 m_AComponent;





	MisMedicOrgan_Ordinary *m_pSelectedOrgan;

	QTimer *m_Timer;

};

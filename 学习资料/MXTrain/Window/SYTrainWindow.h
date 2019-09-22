#ifndef _SYTRAINWINDOW_
#define _SYTRAINWINDOW_

#include <QProgressDialog>
#include "ui_SYTrainWindow.h"
#include "../MXRobot/Include/MXOgreWrapper.h"
#include "DeviceUpdateThread.h"
#include "Inception.h"
#include "MxDefine.h"
#include <QPixmap>
#include <stdint.h>
#include "ScoreBoardWeb.h"
#include "SYToolMenu.h"
#include"MxScreenDataSender.h"
#include"SYLoading.h"
#include"RbFixedToolsMenu.h"
#include"RbPhantomBoxTest.h"
#include"RbAdjustGyroTool.h"
#include"MxTrainingDebugWindow.h"


struct SYTrainResultRecord;
class RbScreenRecorder;
class RbCaseReturnWidget;
class RbMoviePlayer;
class SYLoading;
class CToolsMgr;
class CBasicTraining;
class SYToolMenu;
//class RbDegreeMenu;
//class RbTimerWindow;
//class RbProgressBar;
//class RbTipWindow;
class RbPhantomBoxTest;
class RbAdjustCamera;
class RbAdjustShaft;
class RbAdjustGyroTool;
class RbFixedToolsMenu;
class MxTrainingDebugWindow;
class MxScreenDataSender;
class MxSessionCommand;
class RbConnectStatusWindow;
class MisNewTraining;


class SYTrainWindow : public QWidget, public SYToolMenuEventListener
{
	Q_OBJECT
public:
	struct CaseInfoParam
	{
		QString trainCategoryName;
		QString trainEnName;//unique 
		QString trainChName;
		QString nextTrainName;
		QString trainCode;
	};

	class TrainModelDlgToShow
	{
	public:
		int mode;
		std::string title;
		std::string description;
		std::string leftButtonText;
		std::string rightButtongText;
		int m_Type;
	};

public:
	SYTrainWindow(CaseInfoParam& param,QWidget* parent = NULL );
	~SYTrainWindow();
	
	
	void ReStartScene();
	void EnableTimer(void);
	void DisableTimer(void);
	void StartVideoRecording();
	std::string StopVideoRecording();//return file path
	CToolsMgr* GetToolsMgr();

	void StartOnlineGrade(const char* sheedCode);
	void StopOnlineGrade();
	static void PushScoreCode(const char code[20], int begintime, int endtime);

	
	static void StopDataServiceCallBack(int, char* pUrl);
	static QWidget* s_widgetSelf;

	//overriden �û��Ҽ��������رճ���
	void closeEvent(QCloseEvent* event);

	void SetCameraPullOutImgVisible(bool visible)
	{
		if (m_CameraPullOutImg && (m_CameraPullOutImg->isVisible() != visible))
		{
			m_CameraPullOutImg->setVisible(visible);
		}
	}
	bool IsDisplaySkillImproveWindow(QVector<QPair<QString, QString>>& trainInfos, QVector<QPair<QString, bool>> &trainLockStates);

protected:
	QString FormatSeconds(float totalSeconds);
	void HideTimeFrame();
	void ShowTimeFrame();
	void showEvent( QShowEvent* event );
	void keyPressEvent(QKeyEvent* event);
	void keyReleaseEvent (QKeyEvent* evt);
	void mouseMoveEvent(QMouseEvent* event);
	void customEvent(QEvent* event);
    void wheelEvent(QWheelEvent* event);

	void ShowModleDlg(int mode, 
					  const std::string& title, 
					  const std::string& description, 
					  const std::string& leftButtonText, 
					  const std::string& rightButtongText);

	void SkillImproveWin();

	QVector<QPair<QString, QString>> FromXMLReadTrainModuleInfo();
public slots:

	void SelectTool(int handType,int msgType,int submsgType);
	void SelectDegree(int msgType);
	void QuitScene();

	void on_picBtn_clicked();
	void on_bt_capscreen_clicked();
	void on_tb_zerodegree_toggled(bool check);
	void on_tb_30degree_toggled(bool check);
	void on_tb_anatomy_clicked();

	void onDemonstrationBeginBtn();
	void onDemonstrationStopBtn();
	void onSwitchRecordBtn(int check);

	void on_ShowMovie();
	void on_LoadMovie(QString strMovieName);
	//void on_ShowMovieAndFinish();
	void on_PostShowMovieEvent(QString strMovieName);
	void on_PostLoadMovieEvent(QString strMovieName);
	void on_PostShowMovieAndFinishEvent();
	void on_completeBtn_clicked();
	void on_tipBtn_clicked();
	//void on_StashPopupWidget(int mode, const std::string& title, const std::string& description, const std::string& leftButtonText, const std::string& rightButtongText);

	void onReceiveSessionCommand(unsigned int ip, const MxSessionCommand &command);

	void ExitModule();

	void ShowProgressRegion();
	void HideProgressRegion();
	void ShowTrainCompletness(int curr , int total);

	

private slots:
    void onTimingStart();
	void OnTrainTimeElapsed();
	void on_ShowTip(TipInfo tipInfo);
	void on_ShowFixToolMenu();

	void On_ShowPhantomBoxDebug();
	void On_DestroyPhantomBoxDebug();
	void On_GetPhantomBoxDebugInfo(QString& strinfo);

	
	//////////////////////////////////////////////////////////////////////////
	void On_ShowAdjustGyro();
	void On_GetAdjustGyroInfo(QString& strinfo);

	void on_CapScreenAnime_Finished();

	void StartLoadingTrainScene();//��ʼ����ѵ������ʼ��app

	void FinishLoadingTrainScene();

private :
	bool InitializeScene();
	bool TerminateScene();	

	void ActiveUI();
	void DeactiveUI();

	void OnToolMenuSelected(int side , QString & picFile, QString &type, QString &subType);

	MisNewTraining* GetCurrentTraining();
	void SaveResult();

	const std::string& MakeVideoFileName();

	void ShowCapScreenAnimation(const QPixmap & pixmap , int mseconds);
private:
	bool m_bTerminated;
	bool m_bFirstShow;
	//bool m_IsTimingStart;

	SYToolMenu* m_toolMenu_l;//������е�˵�
	SYToolMenu* m_toolMenu_r;//������е�˵�
	//RbDegreeMenu *  m_degreeMenu;//���Ӳ˵�
	RbFixedToolsMenu* m_pFixedToolsMenu;

	/*���Թ��߽���*/
	RbPhantomBoxTest* m_phantomboxdebug;
	MxTrainingDebugWindow* m_trainingDebugWindow;

	//RbAdjustCamera* m_adjustcamera;
	//RbAdjustShaft* m_adjustshaft;
	RbAdjustGyroTool* m_adjustGyro;

	QLabel* m_CameraPullOutImg;//
	//˫����е��ز���
	QString m_strLeftDeviceName;
	QString m_strLeftDeviceFile;
	QString m_strLeftToolType;
	QString m_strLeftSubType;
	QString m_strRightDeviceName;
	QString m_strRightDeviceFile;
	QString m_strRightToolType;
	QString m_strRightSubType;

	//�ܺ�ʱ
	//int m_remainTime;
	QTimer* m_TimerTrain;//ѵ���ļ�ʱ��//mTimer_1s;
	//QTimer *m_pTipTimer;//tip ��ʾ�ļ�����
	//int m_nCameraTotalSec;

	DeviceUpdateThread* mDeviceUpdateThread;

	QDateTime m_trainingStartTime;

	RbScreenRecorder* m_pScreenRecorder;

	bool   m_InRecordNew;

	CaseInfoParam m_caseParam;

	float m_TimeFromTrainActive;//��������������Ϊֹ������ʱ��(��)
	float m_TimeVideoRecord;//¼�����ʱ��
	float m_TrainConfigedTime;//���õ�ѵ������ʱ��

	QString m_strMovieName;
	RbMoviePlayer* m_pDlgMoviePlayer;
	SYLoading* m_LoadingUI;

	bool m_bFinished;
	QString m_strTipGuide;

	//RbTimerWindow * m_pTimeWindow;		//��ʾʣ��ѵ��ʱ��
	//RbTipWindow * m_pTipWindow;
	//QPushButton * m_pBackBtn;			//�½��淵�ذ�ť
   // QPushButton * m_ptipBtn;
    //QPushButton * m_pScreenShotBtn;     //������ť
	QPushButton* m_pDemonstrationBeginBtn;	//ʾ�̿�ʼ��ť
	QPushButton* m_pDemonstrationStopBtn;	//ʾ��ֹͣ��ť
	//QPushButton* m_pRecordBtn;			//¼��ť
	RbConnectStatusWindow* m_pConnectStatusWindow;
	
	//float m_videoRecordingStopTime;

	bool m_isAdministrator;

	MxScreenDataSender* m_screenDataSender;

	//std::vector<TrainModelDlgToShow> m_ModelDlgToShow;

	SYTrainReportRecord m_reportRecord;
	QMap<QString, QPixmap> m_screenshots;
	QVector<QString> m_videoFiles;

	bool m_bMissionTerminate;
	std::string m_RecordVideoString;
	std::string m_videoFileName;
	
	
	bool m_bFlagForLock;

	bool m_ModuleExisted;

	Ui::SYTrainWindow ui;
};

#endif
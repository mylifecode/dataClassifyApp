#pragma once
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料

#include <QtCore>
#include <QFile>
#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QDialog>
#include <QDomDocument>
#include <QFile>
#include <QFrame>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QTreeView>
#include <QtDebug>
#include <QTextStream>
#include <QTimer>
#include <QMessageBox>
#include <QMenu>
#include <QVariant>
#include <QDebug>
#include <vector>
#include "IMXDefine.h"
#include "MxOperateItem.h"
#include "SYScoreItemDetail.h"



#define SHOW_MOVIE_EVENT		QEvent::User+1
#define FINISHED_LOADING_EVENT	QEvent::User+2
#define LOAD_MOVIE_EVENT		QEvent::User+3
//#define SHOW_MOVIE_AND_FINISH	QEvent::User+4

#define CHS(text) QString::fromLocal8Bit(text)
#define Message(title,content) QMessageBox::information(NULL,title,content,CHS("返回"))

#define InternalMessageBox(title,content) QMessageBox::information(this,title,content,CHS("确定"))

/// 用于华西医院预约的网络配置文件
#define HTTP_URL_INI "../config/httpurl_huaxi.ini"

enum TrainType
{
	/// 实物训练
	TT_EntityTrain,
	/// 技能训练
	TT_SkillTrain,
	/// 手术训练
	TT_SurgeryTrain
};

/** 用户权限 */
enum UserPermission
{
	/// 访问者
	UP_Visitor,
	/// 学生权限
	UP_Student,
	/// 教师权限,相当于普通管理员
	UP_Teacher,
	/// 超级管理员权限
	UP_SuperManager,
	/// 非法权限
	UP_Error
};

enum ToolEvent
{
	SHOW_EVT = 0,
	CHANGE_EVT,
	SELECT_EVT
};

enum WindowType
{
	WT_None = -1,
	WT_LoginWindow,
	WT_UserWindow,
	WT_TrainingCenterWindow,
	WT_AdminTrainingCenterWindow,
	WT_WeChatLoginWindow,
	WT_AdminPortalWindow,
	WT_AdminPersonWindow,
	WT_AdminGroupMgrWindow,
	WT_AdminTaskWindow,
	WT_AdminSendTaskWindow,
	WT_CameraWindow,
	WT_CourseTrainWindow,
	WT_UserScoreWindow,
	WT_AdminScoreWindow,
	WT_AdminDataCenterWindow,
	WT_AdminTheoryTestWindow,   //增加理论测试窗口
	WT_AdminKnowledgeSetManageWindow,//知识集管理
	//WT_AdminNewPaperWindow,  //新建试卷
	//WT_AdminQuestionsMgrWindow,
	WT_ScoreBoardWebWindow,
	WT_NewDocWindow,
	WT_DemonstrationWindow,
	WT_PersonCenterWindow,
	WT_PersonTasksWindow,
	WT_MessageCenterWindow,
	WT_UserManageWindow,
	WT_MonitorWindow,

	/// 知识集窗口
	WT_KnowLibWindow,
	WT_SkillTrainingWindow,
	WT_SurgeyTrainWindow,
	WT_DataCenterWindow,

	///答题模块窗口
	WT_ExamListWindow,
	WT_ExamDoWindow,
	WT_ExamResultWindow
};

/**
用报告单的形式表示分数记录等,主要增加了包括操作项的描述
*/
struct MXCOMMON_API SYTrainReportRecord
{
	SYTrainReportRecord()
		:m_userId(-1),
		m_trainTypeName(""),
		m_trainTypeCode(""),
		m_trainName(""),
		m_trainCode(""),
		m_beginTime(""),
		m_costedTime(0),
		m_score(0),
		m_LeftToolSpeed(0),
		m_RightToolSpeed(0),
        m_LeftToolMovDist(0),
		m_RightToolMovDist(0)
	{

	}

	void reset()
	{
		m_userId = -1;
		m_trainTypeName.clear();
		m_trainTypeCode.clear();
		m_trainName.clear();
		m_trainCode.clear();
		m_beginTime.clear();
		m_costedTime = 0;
		m_score = 0;
		m_LeftToolSpeed = 0;
		m_RightToolSpeed = 0;
		m_LeftToolMovDist = 0;
		m_RightToolMovDist = 0;
		m_scoreItemDetails.clear();
	}

	void updateScore()
	{
// 		operateScore = 0.f;
// 		timeScore = 0.f;
// 		totalScore = 0.f;
// 
// 		for(size_t i = 0; i < operateItems.size(); ++i)
// 		{
// 			operateScore += operateItems[i].GetTotalOperateScore();
// 			timeScore += operateItems[i].GetTotalTimeScore();
// 		}
// 
// 		totalScore = operateScore + timeScore;
// 		if(totalScore < 0.f)
// 			totalScore = 0.f;
	}

	int m_userId;
	QString m_trainTypeName;
	QString m_trainTypeCode;
	QString m_trainName;
	QString m_trainCode;
	QString m_beginTime;
	int m_costedTime;
	int m_score;

	float m_LeftToolSpeed;
	float m_RightToolSpeed;

	float m_LeftToolMovDist;
	float m_RightToolMovDist;
	std::vector<SYScoreItemDetail> m_scoreItemDetails;
};

namespace Mx
{
	void MXCOMMON_API setWidgetStyle(QWidget* w, const QString& qssFile);

	void MXCOMMON_API setGlobleStyle(const QString& qssFile);

	bool MXCOMMON_API writeDataToExcelFile(const QString& fileName, const QVector<QVector<QVariant>> & datas);

	bool MXCOMMON_API readDataFromExcelFile(const QString& fileName, QVector<QVector<QVariant>> & datas, int maxCol);

	std::multimap<UserPermission, QString> MXCOMMON_API addModuleItemFromXML();
}

#define MENTORMODULE_MUTEXID   "SYLAPMENTOR-{BE2B769F-C0E6-4923-BF57-2C47EC80021C}"
#define QUESTIONMODULE_MUTEXID "SYLAPQUESTION-{BE2B769F-C0E6-4923-BF57-2C47EC80021C}"

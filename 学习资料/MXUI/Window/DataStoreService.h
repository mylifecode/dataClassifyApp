#pragma once

#define DSS_LIB

#include <QtCore>
#include <QtNetwork>
#include <QDataStream>
#include <QIODevice>
#include <QTcpServer>
#include <QNetworkSession>

typedef std::function<void(const QVariantMap&)> ReplyCallback;
typedef void(*LoginCallBack)(bool ok, int mode, char* username, char* name, char* trainitem);
typedef void(*LogoutCallBack)();
typedef void(*StoppedCallBack)(int scoreid, char* url);

namespace MX
{

	extern "C"
	{
		/** 初始化
		*/
		void DSS_LIB init(LoginCallBack loginCallBack, StoppedCallBack stoppedCallBack,LogoutCallBack logoutCallBack);

		/** 启动“数据生产存储上传服务”，后台开始录制视频、音频。进入训练场景时开始调用,仅调用一次。
		*@param scoresheetcode, 评分表id
		*/
		void DSS_LIB start(char * scoresheetcode);

		/** 设置训练模式
		*@param  mode, 0:演示，1:考试，2:训练，3:大赛
		*/
		void DSS_LIB setMode(int mode);


		/** 停止“数据生产存储上传服务”，后台上传评分、视频、音频、图片。退出训练场景时开始调用,仅调用一次。
		*/
		void DSS_LIB stop(StoppedCallBack callback = nullptr);

		/** 程序退出 释放资源
		*/
		void DSS_LIB release();

		/** 对出错时进行截图取证。
		@param code, 评分编码
		 */
		void DSS_LIB prtScr(const char code[20]);

		/** 步骤信息存储.步骤发生改变时进行调用。
		@param stepid, 步骤id
		@param info, 步骤说明
		*/
		void DSS_LIB pushStep(int stepid, const char info[256]);

		/** 评分编码存储.步骤发生改变时进行调用。
		@param code, 评分编码
		@param begintime, 评分编码产生时刻的毫秒数
		@param endtime, 评分编码结束时刻的毫秒数.如果使用endtime=0的缺省参数, 则评分编码对应的时间区间为begintime±2000
		*/
		void DSS_LIB pushScoreCode(const char code[20], int begintime, int endtime = 0);

		/** “开关数据”存储.例如：罗伯特夹子的夹闭传感器数据。
		@param type, 器械类型
		@param state, 开关状态
		*/
		void DSS_LIB pushSwitchData(int type, bool state);

		/** “器械数据”存储.需要间隔10ms~30ms实时存储。例如：穿刺针相关数据。
		@param type, 器械类型
		@param num, 存储数据个数
		@param data, 数据存储数组
		*/
		void DSS_LIB pushToolData(int type, int num, const float data[12]);

		/**
		@return, 得到“数据存储服务” 内部时钟时间。（毫秒数）
		*/
		int DSS_LIB getTime();

		/** 登录
		* callback:登录回调
		*@param ok, 登录结果
		*@param mode, 登录模式，0表示用户名密码登录，1表示web登录
		*@param username, 返回的用户名
		void callback(bool ok, int mode, char* username, char* name)
		{
			// do sth
		}
		login("user", "123", callback);
		*/
		void DSS_LIB login(char *username, char *pwd, LoginCallBack callback = nullptr);

		/** 注销
		*/
		void DSS_LIB logout(char *username);
	}

	class ClientNet;
	class DSS : public QObject
	{
		Q_OBJECT
	public:
		DSS(QObject* parent=nullptr);

		static DSS *Get()
		{
			static DSS *dss = nullptr;
			if (dss == nullptr)
			{
				dss = new DSS;
			}
			return dss;
		}

	Q_SIGNALS:
		void stoppedReplyed(int scoreid, const QString & url);
		void loginReplyed(bool ok, int mode, const QString & username, const QString & name, const QString & trainitem, const QString & url);
		void logoutReplyed(bool ok);

	public Q_SLOTS:
		void init(LoginCallBack login, StoppedCallBack stopped, LogoutCallBack logout);
		void Start(const QString & scoresheetcode, ReplyCallback reply = nullptr);
		void SetMode(int mode, ReplyCallback reply = nullptr);
		void Release(ReplyCallback reply = nullptr);
		void pushScoreCode(const QString & code, int begintime, int endtime);
		void Login(const QString & username, const QString & pwd, ReplyCallback reply=nullptr);

		void Logout(const QString & username, ReplyCallback reply = nullptr);
		int GetTime();

		void Stop(ReplyCallback reply = nullptr);

		void request(const QVariantMap& data, ReplyCallback reply = nullptr);

	private Q_SLOTS:
		void received(const QVariantMap &data);

	private:
		void timerEvent(QTimerEvent *event);

	private:
		class ScoreData
		{
		public:
			QString code;
			int begintime;
			int endtime;
		};
		qint32 mSecCnt;
		ClientNet * mClientNet;
		QMap<QString, ReplyCallback> mReplys;
		QList<ScoreData> mScoreData;
	};
}

#include "DataStoreService.h"
#include "ClientNet.h"
#include <windows.h>
#include <psapi.h>

using namespace std;

#define FN_TIME "C:/MISData/Bin/time.ini"

LoginCallBack loginCallBack = nullptr;
LogoutCallBack logoutCallBack = nullptr;
StoppedCallBack stoppedCallBack = nullptr;

namespace MX
{
	void DSS_LIB init(LoginCallBack loginCallBack, StoppedCallBack stoppedCallBack,LogoutCallBack logout)
	{
		DSS::Get()->init(loginCallBack, stoppedCallBack,logout);
	}

	void DSS_LIB start(char *scoresheetcode)
	{
		DSS::Get()->Start(scoresheetcode);
	}

	void DSS_LIB stop(StoppedCallBack callback)
	{
		DSS::Get()->Stop([=](const QVariantMap& data)->void {
			int scoreid = data["Scoreid"].toInt();
			char* url = data["Url"].toString().toLocal8Bit().data();
			callback(scoreid, url);
		});
	}

	void DSS_LIB setMode(int mode)
	{
		DSS::Get()->SetMode(mode);
	}

	void DSS_LIB release()
	{
	}

	void DSS_LIB prtScr(const char code[20])
	{
	}

	int DSS_LIB getTime()
	{
		return DSS::Get()->GetTime();
	}

	void DSS_LIB pushStep(int stepid, const char info[256])
	{
	}

	void DSS_LIB pushScoreCode(const char code[20], int begintime, int endtime)
	{
		DSS::Get()->pushScoreCode(code, begintime, endtime);
	}

	void DSS_LIB pushSwitchData(int type, bool state)
	{
	}

	void DSS_LIB pushToolData(int type, int num, const float data[12])
	{
	}

	void DSS_LIB login(char *username, char *pwd, LoginCallBack callback)
	{
		QVariantMap data;
		data["Command"] = "login";
		data["UserName"] = username;
		data["Password"] = pwd;
		DSS::Get()->request(data, [=](const QVariantMap& reply)->void
		{
			bool ok = reply["ErrorCode"].toBool();
			int mode = reply["Mode"].toInt();
			char* username = reply["UserName"].toString().toLocal8Bit().data();
			char* name = reply["Name"].toString().toLocal8Bit().data();
			char* trainitem = reply["Trainitem"].toString().toLocal8Bit().data();
			callback(ok, mode, username, name, trainitem);
		});
	}

	void DSS_LIB logout(char *username)
	{
		DSS::Get()->Logout(username);
	}

	DSS::DSS(QObject* parent)
		:QObject(parent),mSecCnt(0), mClientNet(new ClientNet(this))
	{
		//startTimer(1000);
		QObject::connect(mClientNet, SIGNAL(received(const QVariantMap&)), SLOT(received(const QVariantMap&)));
	}

	void DSS::timerEvent(QTimerEvent *)
	{
		QVariantMap data;
		data["Command"] = "gettime";
		request(data, [&](const QVariantMap& received)->void
		{
			mSecCnt = received["Time"].toInt();
		});
	}

	void DSS::request(const QVariantMap& data, std::function<void(const QVariantMap&)> reply)
	{
		if (reply != nullptr)
		{
			mReplys[data["Command"].toString()] = reply;
		}
		mClientNet->send(data);
	}

	void DSS::init(LoginCallBack _loginCallBack, StoppedCallBack _stoppedCallBack, LogoutCallBack logout)
	{
		loginCallBack = _loginCallBack;
		stoppedCallBack = _stoppedCallBack;
		logoutCallBack = logout;
		QVariantMap data;
		data["Command"] = "init";
		mClientNet->send(data);
	}

	void DSS::Start(const QString &scoresheetcode, ReplyCallback reply)
	{
		QDir dir;
		dir.mkpath("c:/MISData");
		dir.mkpath("c:/MISData/Data");

		mScoreData.clear();

		QVariantMap data;
		data["Command"] = "starttraining";
		data["Code"] = scoresheetcode;
		mClientNet->send(data);
		mReplys[data["Command"].toString()] = reply;
	}

	void DSS::SetMode(int mode, ReplyCallback reply)
	{
		QVariantMap data;
		data["Command"] = "setmode";
		data["Mode"] = QString::number(mode);
		data["ActionId"] = 1008;
		mClientNet->send(data);
		mReplys[data["Command"].toString()] = reply;
	}

	void DSS::Stop(ReplyCallback reply)
	{
		mSecCnt = 0;

		QVariantMap data;
		data["Command"] = "setscorecode";
		QVariantMap scoreCodeList;
		foreach(ScoreData sd, mScoreData)
		{
			QVariantList var = {sd.code, sd.begintime, sd.endtime};
			scoreCodeList[sd.code] = var;
		}
		data["Data"] = scoreCodeList;
		mClientNet->send(data);

		// stop training
		data.clear();
		data["Command"] = "stoptraining";
		mClientNet->send(data);
		mReplys[data["Command"].toString()] = reply;
	}

	void DSS::Release(ReplyCallback reply)
	{
		QVariantMap data;
		data["Command"] = "release";
		mClientNet->send(data);
		mReplys[data["Command"].toString()] = reply;
	}

	void DSS::pushScoreCode(const QString &code, int begintime, int endtime)
	{
		ScoreData sd;
		sd.code = code;
		sd.begintime = begintime;
		sd.endtime = endtime;
		mScoreData.push_back(sd);
	}


	void DSS::Login(const QString& username, const QString & pwd, ReplyCallback reply)
	{
		QVariantMap data;
		data["Command"] = "login";
		data["ActionId"] = 1000;
		data["UserName"] = username;
		data["Password"] = pwd;
		mClientNet->send(data);
		mReplys[data["Command"].toString()] = reply;
	}

	void DSS::Logout(const QString &username, ReplyCallback reply)
	{
		QVariantMap data;
		data["Command"] = "logout";
		data["UserName"] = username;
		mClientNet->send(data);
		mReplys[data["Command"].toString()] = reply;
	}

	int DSS::GetTime()
	{
		return mSecCnt;
	}

	void DSS::received(const QVariantMap &data)
	{
		QString command = data["Command"].toString();
		if (mReplys.contains(command))
		{
			ReplyCallback reply = mReplys[command];
			if (reply != nullptr)
			{
				reply(data);
			}
		}

		if (command == "login")
		{
			bool ok = data["ErrorCode"].toInt() == 0 ? true : false;
			int mode = data["Mode"].toInt();
			QString username = data["UserName"].toString();
			QString name = data["Name"].toString();
			QString trainitem = data["Trainitem"].toString();
			QString url = data["Url"].toString();
			if (loginCallBack != nullptr)
			{
				loginCallBack(ok, mode, username.toLocal8Bit().data(), name.toLocal8Bit().data(), trainitem.toLocal8Bit().data());
			}

			Q_EMIT loginReplyed(ok, mode, username, name, trainitem, url);
			qDebug() << "DSS::login reply: " << ok << username << name << trainitem << url;
		}
		else if (command == "stoptraining")
		{
			int scoreid = data["Scoreid"].toInt();
			QString url = data["Url"].toString();
			if (stoppedCallBack != nullptr)
			{
				stoppedCallBack(scoreid, url.toLatin1().data());
			}
			Q_EMIT stoppedReplyed(scoreid, url);
			qDebug() << "DSS::stoptraining reply: " << scoreid << url;
		}
		else if (command == "logout")
		{
			 if (logoutCallBack != nullptr)
			 {
				logoutCallBack();
			 }
		}
	}
}

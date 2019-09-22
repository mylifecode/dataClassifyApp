#include "TipMgr.h"
#include "Inception.h"
#include "EffectManager.h"
#include "Helper.h"

CTipMgr::CTipMgr(void)
{
	m_bTerminate = false;
	m_bQueueInited = false;
	m_FrameCount = 0;
	m_LastTipDuration = 0;
//	m_waitTime = 0;
}

CTipMgr::~CTipMgr(void)
{
	m_mapTips.clear();
}

void CTipMgr::LoadPresetTips()
{
	Tip tip;
	tip.m_bCustomPos = false;
	tip.m_nCustomPosX = 0;
	tip.m_nCustomPosY = 0;
	tip.m_nLastSeconds = 0;

	{
		QString str = QString::fromLocal8Bit("请将双手器械垂直拉至顶端。");
		tip.m_wstrDescription = str.toStdWString();
		tip.m_strIconType = "Guide";
		m_mapTips["TrainingReset"] = tip;
	}

	{
		QString str = QString::fromLocal8Bit("左手钛夹钳内无钛夹,请重置器械。");
		tip.m_wstrDescription = str.toStdWString();
		tip.m_strIconType = "Tip";
		m_mapTips["NoClipInHolder_Left"] = tip;
	}

	{
		QString str = QString::fromLocal8Bit("右手钛夹钳内无钛夹,请重置器械");
		tip.m_wstrDescription = str.toStdWString();
		tip.m_strIconType = "Tip";
		m_mapTips["NoClipInHolder_Right"] = tip;
	}

	{
		QString str = QString::fromLocal8Bit("训练已完成");
		tip.m_wstrDescription = str.toStdWString();
		tip.m_strIconType = "Guide";
		m_mapTips["TrainingFinish"] = tip;
	}

	{
		QString str = QString::fromLocal8Bit("训练已超时，当前操作不计分");
		tip.m_wstrDescription = str.toStdWString();
		tip.m_strIconType = "Guide";
		m_mapTips["TimeOver"] = tip;
	}

	{
		QString str = QString::fromLocal8Bit("严重操作失误");
		tip.m_wstrDescription = str.toStdWString();
		tip.m_strIconType = "Guide";
		m_mapTips["fatalError"] = tip;
	}

}
void CTipMgr::LoadTips(const vector<CXMLWrapperTip *> & vtTips)
{
	m_mapTips.clear();
	
	LoadPresetTips();
	
	for (vector<CXMLWrapperTip *>::const_iterator it = vtTips.begin(); it != vtTips.end(); it++)
	{
		Tip tip;
		//tip.m_wstrDescription = QApplication::translate("TipMgr", (*it)->m_Description.c_str(), 0, QApplication::UnicodeUTF8).toStdWString();
		tip.m_wstrDescription = QApplication::translate("TipMgr", (*it)->m_Description.c_str()).toStdWString();
		tip.m_strIconType = (*it)->m_IconType;
		if ((*it)->m_flag_CustomPosX && (*it)->m_flag_CustomPosY)
		{
			tip.m_bCustomPos = true;
			tip.m_nCustomPosX = (*it)->m_CustomPosX;
			tip.m_nCustomPosY = (*it)->m_CustomPosY;
		}
		else
		{
			tip.m_bCustomPos = false;
			tip.m_nCustomPosX = 0;
			tip.m_nCustomPosY = 0;
		}
		tip.m_nLastSeconds = (*it)->m_LastSeconds;

		if (tip.m_nLastSeconds > 0)
		{
			int ppp = 0;
			int qqq = ppp + 1;
		}
		m_mapTips[(*it)->m_Name] = tip;
	}
}

bool CTipMgr::ShowTip(Ogre::String strTipName, ...)
{
 	if ( m_bTerminate )
 		return false;

	Tip tip = m_mapTips[strTipName];

	TipInfo::TipIconType eIconType;
	
	if (tip.m_strIconType == "Warning")
	{
		eIconType = TipInfo::TIT_TIP_WARNING;
	}
	else if (tip.m_strIconType == "Guide")
	{
		eIconType = TipInfo::TIT_GUIDE;
	}
	else if (tip.m_strIconType == "Tip")
	{
		eIconType = TipInfo::TIT_TIP;
	}
	else
	{
		eIconType = TipInfo::TIT_TIP_INFO;
	}

	va_list args;
	TipInfo tipInfo;
	va_start(args, strTipName);
	wchar_t szBuf[1024];
	vswprintf(szBuf, tip.m_wstrDescription.c_str(), args);
	va_end(args);
	tipInfo.title = strTipName;
	tipInfo.str = QString::fromWCharArray(szBuf);
	tipInfo.eIconType = eIconType;
	tipInfo.nDuration = tip.m_nLastSeconds;
	
	if (m_bQueueInited)//cache into queue
	{
		bool AddToQueue = true;
			
		if( eIconType == TipInfo::TIT_TIP && (m_lastTip == strTipName))
			AddToQueue = false;
			
		if(eIconType == TipInfo::TIT_TIP_WARNING  && m_tipqueue.size() > 0)
		{
			Ogre::String QueueStr = (m_tipqueue.back()).title;
			if(QueueStr== strTipName)
			   AddToQueue = false;
		}

		if (AddToQueue)
		{
			m_tipqueue.push_back(tipInfo);
			m_lastTip = strTipName;
		}
		return true;
	}
	else//show directly
	{
		Inception::Instance()->EmitShowTip(tipInfo);
		return true;
	}

	return true;
}

bool CTipMgr::ShowTipAndTerminate(Ogre::String strTipName)
{
	return true;
}
void CTipMgr::OnTrainStart()
{
	m_bTerminate = false;

}
void CTipMgr::OnTrainEnd()
{
	m_bTerminate = true;
}

void CTipMgr::OnTrainCreated()
{

}

void CTipMgr::OnTrainDestroyed()
{
    m_mapTips.clear();
}

void CTipMgr::Update(float dt)
{
	if (m_bQueueInited)
	{
		if (m_LastTipDuration > 0)
		{
			m_LastTipDuration -= dt;
		}
		else
		{
			m_LastTipDuration = 0;
			
			if (!m_tipqueue.empty())
			{
			    TipInfo tipInfo = *(m_tipqueue.begin());

				m_tipqueue.pop_front();

				Inception::Instance()->EmitShowTip(tipInfo);

				m_LastTipDuration = tipInfo.nDuration;
			}
		}
	}

	std::map<Ogre::String, float>::iterator iter = m_nextTip.begin();
	for (; iter != m_nextTip.end();)
	{
		if (!(iter->first.empty()) && iter->second > 0)
		{
			iter->second -= dt;
			if (iter->second <= 0)
			{
				map<Ogre::String, Tip>::iterator itr = m_mapTips.find(iter->first);
				if (itr != m_mapTips.end())
				{
					Tip tip = itr->second;

					TipInfo::TipIconType eIconType;

					if (tip.m_strIconType == "Warning")
						eIconType = TipInfo::TIT_TIP_WARNING;
					else if (tip.m_strIconType == "Guide")
						eIconType = TipInfo::TIT_GUIDE;
					else if (tip.m_strIconType == "Tip")
						eIconType = TipInfo::TIT_TIP;
					else
						eIconType = TipInfo::TIT_TIP_INFO;

					TipInfo tipInfo;
					tipInfo.title = iter->first;
					tipInfo.str = QString::fromWCharArray(tip.m_wstrDescription.c_str());
					tipInfo.eIconType = eIconType;
					tipInfo.nDuration = tip.m_nLastSeconds;

					Inception::Instance()->EmitShowTip(tipInfo);
				}
				iter = m_nextTip.erase(iter);
			}
			else
			{
				++iter;
			}
		}
	}
}

void CTipMgr::SetQueeu( bool isOn )
{
	m_bQueueInited = isOn;
	m_lastTip = "notsame";
	m_tipqueue.clear();
}

void CTipMgr::SetNextTip(const Ogre::String& tipName,float waitTime)
{
	m_nextTip[tipName] = waitTime;
}

bool CTipMgr::DeleteNextTip(const Ogre::String& tipName)
{
	std::map<Ogre::String, float>::iterator iter = m_nextTip.begin();
	for (; iter != m_nextTip.end(); ++iter)
	{
		if (iter->first == tipName)
		{
			m_nextTip.erase(iter);
			return true;
		}
	}
	return false;
}

void CTipMgr::DeleteAllNextTip()
{
	m_nextTip.clear();
}
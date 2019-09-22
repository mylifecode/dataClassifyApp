#include "RbAllocCoursePlanWindow.h"
#include "SYDBMgr.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include "MxDefine.h"


RbAllocCoursePlanWindow::RbAllocCoursePlanWindow(QWidget *parent, QVector<QString> vecPlanID, QList<COURSEPLANINFO> listCoursePlanInfo/* = QList<COURSEPLANINFO>()*/):
RbShieldLayer(parent),m_vecPlanID(vecPlanID), m_listCoursePlanInfo(listCoursePlanInfo)
{
	ui.setupUi(this);

	//��������tableWidget����Ϊ����ʽ
	ui.tableWidget_student->horizontalHeader()->resizeSection(0,120);
	ui.tableWidget_student->horizontalHeader()->resizeSection(1,120);
	ui.tableWidget_student->horizontalHeader()->resizeSection(2,200);
	//����vs2013���޶�Ӧ�ĺ��������뱨����ʱע�͵���TODO
	//ui.tableWidget_student->horizontalHeader()->setClickable(false);
	//ui.tableWidget_group->horizontalHeader()->setClickable(false);
	ui.tableWidget_student->setEditTriggers(QAbstractItemView::NoEditTriggers);		//���ò��ɱ༭
	ui.tableWidget_group->setEditTriggers(QAbstractItemView::NoEditTriggers);		//���ò��ɱ༭
	ui.tableWidget_student->setSelectionBehavior ( QAbstractItemView::SelectRows); //����ѡ����Ϊ������Ϊ��λ
	ui.tableWidget_student->setSelectionMode ( QAbstractItemView::MultiSelection); //����ѡ��ģʽ��ѡ����
	ui.tableWidget_group->setSelectionBehavior ( QAbstractItemView::SelectRows); //����ѡ����Ϊ������Ϊ��λ
	ui.tableWidget_group->setSelectionMode ( QAbstractItemView::MultiSelection); //����ѡ��ģʽ��ѡ����

	QSqlQuery query(SYDBMgr::Instance()->GetDatabase());
	QString queryCmd;

	queryCmd = "select distinct id,userName from UserInfoTable where permission = '0'";
	query.exec(queryCmd);
	while(query.next())
	{
		m_mapUserID2UserName.insert(query.value(0).toString(), query.value(1).toString());
	}
	queryCmd = "select groupId,id from UserInfoTable where permission = '0'";
	query.exec(queryCmd);
	while(query.next())
	{
		m_mapGroupID2UserID.insert(query.value(0).toString(), query.value(1).toString());
	}

	//��m_listCoursePlanInfoΪ��ʱ�����ݿ��ж�ȡ���ݣ�����Ӳ����ж�ȡ����
	if (!m_listCoursePlanInfo.empty())
	{
		for (int i = 0; i != m_listCoursePlanInfo.size(); ++i)
		{
			for (QSet<QString>::iterator iter = m_listCoursePlanInfo[i].strUserID.begin(); iter != m_listCoursePlanInfo[i].strUserID.end(); ++iter)
			{
				++m_mapUserID2Count[*iter];
			}
		}
	}
	else
	{
		if (m_vecPlanID.size() == 1)
		{
			queryCmd = QString::fromLocal8Bit("select * from CoursePlanInfoTable where planid = '%1'").arg(m_vecPlanID[0]);
		}
		else
		{
			queryCmd = QString::fromLocal8Bit("select * from CoursePlanInfoTable where planid = '%1'").arg(m_vecPlanID[0]);
			for (int i = 0; i < m_vecPlanID.size(); ++i)
			{
				queryCmd.append(QString::fromLocal8Bit(" or planid = '%1'").arg(m_vecPlanID[i]));
			}
		}
		query.exec(queryCmd);
		while (query.next())
		{
			COURSEPLANINFO cpinfo;
			cpinfo.strPlanID = query.value(0).toString();
			cpinfo.strPlanName = query.value(1).toString();
			QStringList userIDs = query.value(2).toString().split("|", QString::SkipEmptyParts);
			for (int i = 0; i != userIDs.size(); ++i)
			{
				cpinfo.strUserID.insert(userIDs[i]);
				++m_mapUserID2Count[userIDs[i]];
			}
			QStringList trainNames = query.value(3).toString().split("|", QString::SkipEmptyParts);
			for (int i = 0; i != trainNames.size(); ++i)
			{
				cpinfo.strTrainName.insert(trainNames[i]);
			}
			m_listCoursePlanInfo.push_back(cpinfo);
		}
	}

	for (QMap<QString, QString>::iterator iter = m_mapUserID2UserName.begin(); iter != m_mapUserID2UserName.end(); ++iter)
	{
		if (m_mapUserID2Count.end() == m_mapUserID2Count.find(iter.key()))
		{
			m_mapUserID2Count[iter.key()] = 0;
		}
	}

	//����studentWidgetÿ�м�¼���ı����������ɫ����ɫ������Խ��γ̼ƻ���������û�����ɫ������Կγ̼ƻ������ڸ��û�������ѡ�а�ȷ��֮����ԴӸ��û��н��γ̼ƻ�ɾ��
	int i = 0;
	for (QMap<QString, int>::iterator iter = m_mapUserID2Count.begin(); iter != m_mapUserID2Count.end(); ++iter)
	{
		QString userID = iter.key();
		QString userName = m_mapUserID2UserName.value(iter.key());
		int count = m_mapUserID2Count.value(iter.key());
		ui.tableWidget_student->setRowCount(i+1);
		ui.tableWidget_student->setItem(i, 0, new QTableWidgetItem(userID));
		ui.tableWidget_student->item(i, 0)->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_student->setItem(i, 1, new QTableWidgetItem(userName));
		ui.tableWidget_student->item(i, 1)->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_student->setItem(i, 2, new QTableWidgetItem(QString::number(count)));
		ui.tableWidget_student->item(i, 2)->setTextAlignment(Qt::AlignCenter);

		
		//����studentTableWidget�һ�
		bool beGray = true;											//����Ƿ���Խ�������ɫ���óɻ�ɫ
		for (int j = 0; j != m_vecPlanID.size(); ++j)
		{
			for (int k = 0; k != m_listCoursePlanInfo.size(); ++k)
			{
				if (m_listCoursePlanInfo[k].strPlanID == m_vecPlanID[j])
				{
					if (!m_listCoursePlanInfo[k].strUserID.contains(userID))
					{
						beGray = false;
					}
				}
			}
		}
		if (beGray)
		{
			for (int j = 0; j != ui.tableWidget_student->columnCount(); ++j)
			{
				ui.tableWidget_student->item(i,j)->setForeground(QBrush(QColor(149,149,149)));
			}
			m_listGrayRowInStudentTableWidget.push_back(i);			//���ڼ�¼��ɫ���к�
		}
		++i;
	}

	//����groupWidgetÿ�м�¼���ı�ÿ�м�¼���ı����������ɫ����ɫ������Խ��γ̼ƻ��������С�飬��ɫ������Կγ̼ƻ������ڸ�С�飬����ѡ�а�ȷ��֮����ԴӸ�С���н��γ̼ƻ�ɾ��
	QList<QString> listGroupID = m_mapGroupID2UserID.uniqueKeys();
	for (i = 0; i < listGroupID.size(); ++i)
	{
		ui.tableWidget_group->setRowCount(i+1);
		ui.tableWidget_group->setItem(i, 0, new QTableWidgetItem(listGroupID[i]));
		ui.tableWidget_group->item(i, 0)->setTextAlignment(Qt::AlignCenter);
		QList<QString> userIDs = m_mapGroupID2UserID.values(listGroupID[i]);
		int count = 0;
		for (int j = 0; j != userIDs.size(); ++j)
		{
			count += m_mapUserID2Count.value(userIDs[j]);
		}
		ui.tableWidget_group->setItem(i, 1, new QTableWidgetItem(QString::number(count)));
		ui.tableWidget_group->item(i, 1)->setTextAlignment(Qt::AlignCenter);


		//����groupTableWidget�һ�
		bool beGray = true;											//����Ƿ���Խ�������ɫ���óɻ�ɫ
		for (int j = 0; j != m_vecPlanID.size(); ++j)
		{
			for (int k = 0; k != m_listCoursePlanInfo.size(); ++k)
			{
				for (int p = 0; p != userIDs.size(); ++p)
				{
					if (m_listCoursePlanInfo[k].strPlanID == m_vecPlanID[j])
					{
						if (!m_listCoursePlanInfo[k].strUserID.contains(userIDs[p]))
						{
							beGray = false;
						}
					}
				}
			}
		}
		if (beGray)
		{
			for (int j = 0; j != ui.tableWidget_group->columnCount(); ++j)
			{
				ui.tableWidget_group->item(i,j)->setForeground(QBrush(QColor(149,149,149)));
			}
			m_listGrayRowInGroupTableWidget.push_back(i);		//���ڼ�¼��ɫ���к�
		}
	}


	hideCloseButton();
	hideOkButton();

	connect(ui.cancelBtn, SIGNAL(clicked()), this, SLOT(onCancelBtn()));
	connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(onOkButton()));

	Mx::setWidgetStyle(this,"qss:RbAllocCoursePlanWindow.qss");
}

RbAllocCoursePlanWindow::~RbAllocCoursePlanWindow()
{
}

//ȡ����ť�ۺ���
void RbAllocCoursePlanWindow::onCancelBtn()
{
	done(1);
	emit allocPlanWindowClose(CANCEL_ALLOC, "");
}

//ȷ����ť�ۺ���
void RbAllocCoursePlanWindow::onOkButton()
{
	done(1);
	QString queryCmd;
	QVector<QString> vecPlanUserID;

	for (int i = 0; i != m_vecPlanID.size(); ++i)
	{
		QSet<QString> pendingAddUserID;	
		int j = 0;
		for (j = 0; j != m_listCoursePlanInfo.size(); ++j)
		{
			if (m_listCoursePlanInfo[j].strPlanID == m_vecPlanID[i])
			{
				for (QSet<QString>::iterator iter = m_listCoursePlanInfo[j].strUserID.begin(); iter != m_listCoursePlanInfo[j].strUserID.end(); ++iter)
				{
					pendingAddUserID.insert(*iter);
				}
				break;
			}
		}

		//��ȡgroupTableWidget��ѡ����е�����
		QList<QTableWidgetItem*> items = ui.tableWidget_group->selectedItems();
		for(int i=0;i<items.count();i++)
		{
			if (i%2==0)
			{
				int row=ui.tableWidget_group->row(items.at(i));//��ȡѡ�е���
				QTableWidgetItem* item= ui.tableWidget_group->item(row,0);
				QString groupid = item->text();
				QList<QString> userids = m_mapGroupID2UserID.values(groupid);
				for (int j = 0; j != userids.size(); ++j)
				{
					if (m_listGrayRowInGroupTableWidget.contains(row))
					{
						pendingAddUserID.erase(pendingAddUserID.find(userids[j]));
					}
					else
					{
						pendingAddUserID.insert(userids[j]);
					}
				}
			}
		}
		items.clear();

		//��ȡstudentTableWidget��ѡ����е�����
		items = ui.tableWidget_student->selectedItems();
		for(int i=0;i<items.count();i++)
		{
			if (i%3==0)
			{
				int row=ui.tableWidget_student->row(items.at(i));//��ȡѡ�е���
				QTableWidgetItem* item= ui.tableWidget_student->item(row,0);
				QString userid = item->text();
				if (m_listGrayRowInStudentTableWidget.contains(row))
				{
					pendingAddUserID.erase(pendingAddUserID.find(userid));
				}
				else
				{
					pendingAddUserID.insert(userid);
				}
			}
		}

		QString strUserIDs;
		for (QSet<QString>::iterator iter = pendingAddUserID.begin(); iter != pendingAddUserID.end(); ++iter)
		{
			if (strUserIDs.isEmpty())
			{
				strUserIDs.append(*iter);
			}
			else
			{
				strUserIDs.append("|");
				strUserIDs.append(*iter);
			}
		}
		vecPlanUserID.push_back(strUserIDs);
	}

	for (int i = 0; i < m_vecPlanID.size()-1; ++i)
	{
		queryCmd += QString::fromLocal8Bit("update CoursePlanInfoTable set userid = '%1' where planid = '%2'").arg(vecPlanUserID[i]).arg(m_vecPlanID[i]);
		queryCmd += "$";
	}
	queryCmd += QString::fromLocal8Bit("update CoursePlanInfoTable set userid = '%1' where planid = '%2'").arg(vecPlanUserID[m_vecPlanID.size()-1]).arg(m_vecPlanID[m_vecPlanID.size()-1]);
	
	

	emit allocPlanWindowClose(CONFIRM_ALLOC,queryCmd);
}

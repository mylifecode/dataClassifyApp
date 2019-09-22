#include "MxDefine.h"
#include "SYMessageBox.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "SYQuestionExamListWindow.h"
#include "SYExamRecordItem.h"
#include "SYExamDataManager.h"
#include "SYExamGlobal.h"
#include <QFileDialog>
#include <stdio.h>
#include "MxQuestionModuleApp.h"
#include"SYDBMgr.h"
#include"SYStRandomChooseQuestion.h"
#include"SYConfirmDoMissionWindow.h"

SYQuestionExamListWindow::SYQuestionExamListWindow(QWidget *parent)
: QWidget(parent),
m_preButton(nullptr)
{
	ui.setupUi(this);

	//m_userName = SYUserInfo::Instance()->GetUserName();
	//m_realName = SYUserInfo::Instance()->GetRealName();

	//g_UserInfor.m_userid = userId;

	m_userName = g_UserInfor.m_name;

//	m_permission = SYUserInfo::Instance()->GetUserPermission();

	if(m_realName.size())

		ui.headBtn->setText(m_realName);
	else
		ui.headBtn->setText(m_userName);

	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();

	ui.headBtn->setIcon(headIcon);

	
	//knowledge lib
	connect(ui.knowledgeLibBtn, &QToolButton::clicked, this, &SYQuestionExamListWindow::onButtonClicked);

	//data center
	connect(ui.dataCenterBtn, &QToolButton::clicked, this, &SYQuestionExamListWindow::onButtonClicked);

	//answer
	connect(ui.answerBtn, &QToolButton::clicked, this, &SYQuestionExamListWindow::onButtonClicked);

	//person center
	connect(ui.personCenterBtn, &QToolButton::clicked, this, &SYQuestionExamListWindow::onButtonClicked);

	
	Mx::setWidgetStyle(this, "qss:SYQuestionExamListWindow.qss");

	RefreshMissionList();

	ui.answerBtn->setChecked(true);
}

SYQuestionExamListWindow::~SYQuestionExamListWindow(void)
{
	m_timer.stop();
}

void SYQuestionExamListWindow::RefreshMissionList()
{
	ui.list_eaxmpaper->clear();
	ui.list_examrecords->clear();
	m_missionlist.clear();

	SYExamDataManager::GetInstance().SelectAllUserMission(g_UserInfor.m_userid, m_missionlist);   //get multiply items
	
	for (int c = 0; c < m_missionlist.size(); c++)
	{
		if (m_missionlist[c].m_IsFinished == false)
		{
			QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_eaxmpaper);

			SYExamPaperItem * theWidgetItem = new SYExamPaperItem;
		
			theWidgetItem->setProperty("index", c);
			theWidgetItem->SetContent(m_missionlist[c].m_PaperName, QString::number(m_missionlist[c].m_QuestionNum), m_missionlist[c].m_Auther);

			int paperId = theWidgetItem->GetPaperId();
			QPushButton* btn = theWidgetItem->GetReturnBtn();

			connect(btn, &QPushButton::clicked, this, [=]
			{
				emit on_item_Btn_clicked(theWidgetItem);
			});

			//Making sure that the listWidgetItem has the same size as the TheWidgetItem
			listWidgetItem->setSizeHint(QSize(370, 70));

			//Finally adding the itemWidget to the list
			ui.list_eaxmpaper->setItemWidget(listWidgetItem, theWidgetItem);    // mission list where isfinished=0 at left
		}
	}


	for (int c = 0; c < m_missionlist.size(); c++)
	{
		if (m_missionlist[c].m_IsFinished == true)
		{
			QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_examrecords);

			SYExamRecordItem * theWidgetItem = new SYExamRecordItem;

			connect(theWidgetItem, SIGNAL(OnDescButtonClicked(SYExamRecordItem*)), this, SLOT(OnItemDescButtonClicked(SYExamRecordItem*)));
			connect(theWidgetItem, SIGNAL(OnReDoButtonClicked(SYExamRecordItem*)), this, SLOT(OnReDoButtonClicked(SYExamRecordItem*)));

			theWidgetItem->SetContent(m_missionlist[c].m_AnswerData,
				m_missionlist[c].m_PaperName,
				m_missionlist[c].m_QuestionNum,
				m_missionlist[c].m_AccuracyNum,
				m_missionlist[c].m_SecondsUsed);
			theWidgetItem->m_missionentry = c;// paperlist[c].m_ID;
			//theWidgetItem->m_PaperID = paperlist[c].m_PaperID;
			//Making sure that the listWidgetItem has the same size as the TheWidgetItem
			listWidgetItem->setSizeHint(QSize(1200, 70));

			//Finally adding the itemWidget to the list
			ui.list_examrecords->setItemWidget(listWidgetItem, theWidgetItem);

		}
	}

}
void SYQuestionExamListWindow::onButtonClicked()
{
	QPushButton* toolBtn = (QPushButton*)sender();

	if(m_preButton){
		if(toolBtn == m_preButton)
			m_preButton->setChecked(true);	
		else
			m_preButton->setChecked(false);
	}
	m_preButton = toolBtn;
	//m_preButton->setChecked(true);
}

void SYQuestionExamListWindow::OnItemDescButtonClicked(SYExamRecordItem * item)
{
	g_currentMission = m_missionlist[item->m_missionentry];
	emit showNextWindow(WT_ExamResultWindow);
}


void SYQuestionExamListWindow::OnReDoButtonClicked(SYExamRecordItem*item)  //redo  test where isfinished=1
{
	g_currentMission = m_missionlist[item->m_missionentry];
	emit showNextWindow(WT_ExamDoWindow);
} 

void SYQuestionExamListWindow::on_randexam_clicked()  //random choose exam paper, default paperid=-1,missionId=-1
{

	SYStRandomChooseQuestion* RandomChooseWin = new SYStRandomChooseQuestion(this);

	connect(RandomChooseWin, &SYStRandomChooseQuestion::setPaperInfo, this, [=](int questionNumber, int minutes)
	{
		g_currentMission.m_QuestionNum = questionNumber;
		g_currentMission.m_ExamTime = minutes;
		//考试总时间设置

	});

	RandomChooseWin->showFullScreen();
	int ret = RandomChooseWin->exec();
	if (ret == 1)
	{
		g_currentMission.m_PaperID = -1;
		g_currentMission.m_MissionID = -1;

		emit showNextWindow(WT_ExamDoWindow);

	}

}

void SYQuestionExamListWindow::on_item_Btn_clicked(SYExamPaperItem* item) //mission list at left side where item is clicked
{
	//SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("开始答题"), 2);

	int questionNumber;
	int examTime;
	QString paperName = item->GetPaperName();
	SYExamDataManager::GetInstance().QueryMissionInfo(paperName, questionNumber, examTime);
	SYConfirmDoMissionWindow* confirmWindow = new SYConfirmDoMissionWindow(questionNumber,examTime,this);

	confirmWindow->showFullScreen();

	if (confirmWindow->exec() == 1)
	{
	
		int index = item->property("index").toInt();
		
		g_currentMission = m_missionlist[index];
		emit showNextWindow(WT_ExamDoWindow);
		//emit DoExamMission(-1);
	}

}

void SYQuestionExamListWindow::on_knowledgeLibBtn_clicked()
{
	static_cast<MxQuestionModuleApp*>(qApp)->exitModuleAndShowParentWindow(WT_KnowLibWindow);

}

void SYQuestionExamListWindow::on_dataCenterBtn_clicked()
{
	static_cast<MxQuestionModuleApp*>(qApp)->exitModuleAndShowParentWindow(WT_DataCenterWindow);
}

void SYQuestionExamListWindow::on_answerBtn_clicked()
{
	
}

void SYQuestionExamListWindow::on_personCenterBtn_clicked()
{
	static_cast<MxQuestionModuleApp*>(qApp)->exitModuleAndShowParentWindow(WT_PersonCenterWindow);
}

void SYQuestionExamListWindow::showEvent(QShowEvent* event)
{
	if(m_preButton){
		//m_preButton->setChecked(false);
		m_preButton->setChecked(true);
	}

	//update tain info
	int totalTime = 0;
	int trainTimes1 = 0;
	int trainTimes2 = 0;
	int trainTimes3 = 0;
	int userId = 0;// SYUserInfo::Instance()->GetUserId();
	bool ok = 0;// SYDBMgr::Instance()->QueryBasicTrainInfo(userId, totalTime, trainTimes1, trainTimes2, trainTimes3);
	if(!ok)
		return;

	QString info;
	//1
	int h = totalTime / 3600;
	int m = (totalTime - h * 3600) / 60;
	//ui.hourLabel->setText(info.setNum(h));
	//ui.minuteLabel->setText(info.setNum(m));
	//2
	//ui.trainTimesLabel1->setText(info.setNum(trainTimes1));
	//ui.trainTimesLabel2->setText(info.setNum(trainTimes2));
	//ui.trainTimesLabel3->setText(info.setNum(trainTimes3));
}

void SYQuestionExamListWindow::mousePressEvent(QMouseEvent* mouseEvent)
{
	
}

void FlushOneRecord(char questionContent[], char optionA[], char optionB[], char optionC[], char optionD[], char optionE[], bool correctTag[5], int numAnswerRight)
{
	if (numAnswerRight == 1)//暂时过滤掉多选
	{
		QString qcontent = QString::fromLocal8Bit(questionContent);
		QString qoptA = QString::fromLocal8Bit(optionA);
		QString qoptB = QString::fromLocal8Bit(optionB);
		QString qoptC = QString::fromLocal8Bit(optionC);
		QString qoptD = QString::fromLocal8Bit(optionD);
		QString qoptE = QString::fromLocal8Bit(optionE);

		QString correctAnswer = "A";
		if (correctTag[0])
			correctAnswer = "A";
		
		else if (correctTag[1])
			correctAnswer = "B";
		
		else if (correctTag[2])
			correctAnswer = "C";
		
		else if (correctTag[3])
			correctAnswer = "D";
		
		else if (correctTag[4])
			correctAnswer = "E";

		if (correctAnswer == "E")//temp no e option swap optD and optE
		{
			QString tmp = qoptD;
			qoptD = qoptE;
			qoptE = tmp;
			correctAnswer = "D";
		}
		SYExamDataManager::GetInstance().ImportOneQuestionToDB(QString::fromLocal8Bit(questionContent), qoptA, qoptB, qoptC, qoptD, qoptE, correctAnswer);
	}
}
void SYQuestionExamListWindow::onBackToWidget(QWidget * widget)
{
	if (widget == this)
	{
		RefreshMissionList();
	}
}
void SYQuestionExamListWindow::on_bt_importquest_clicked()
{
#if(0)//
    QVector<QVector<QVariant>> datas;

    QStringList strList = QFileDialog::getOpenFileNames(this, "", "", tr("All Excel files (*.xls)"));

	if (strList.size())
	{
		for (int i = 0; i < strList.size(); ++i)				//支持从多个文件中读入数据，最大列数为3
		{
			Mx::readDataFromExcelFile(strList[i], datas, 7);
		}

		QString questionContent;

		QString questionOptionA;

		QString questionOptionB;

		QString questionOptionC;

		QString questionOptionD;

		QString questionOptionE;

		QString questionAnswer;

		for (int i = 1; i < datas.size(); ++i)
		{
			//user name
			questionContent = datas[i][0].toString();

			//real name
			questionOptionA = datas[i][1].toString();

			questionOptionB = datas[i][2].toString();

			questionOptionC = datas[i][3].toString();

			questionOptionD = datas[i][4].toString();

			questionOptionE = datas[i][5].toString();

			questionAnswer = datas[i][6].toString();

			SYExamDataManager::GetInstance().ImportOneQuestionToDB(questionContent, questionOptionA, questionOptionB, questionOptionC, questionOptionD, questionOptionE, questionAnswer);
		}

		//SYMessageBox * messageBox = new SYMessageBox(this, CHS("提示"), CHS("共%1条数据，成功导入%2条").arg(std::max(datas.size() - 1, 0)).arg(nLoadedUser), 1);
		//messageBox->setButtonStyle();
		//messageBox->showFullScreen();
		//ReFresh();
	}
#else
	//fopen("c:\\questlist.txt", "r");
	std::ifstream infile("D:\\questlist.txt");

	bool isfirstquestion = true;


	char questionContent[1000];

	char optionA[1000];
	char optionB[1000];
	char optionC[1000];
	char optionD[1000];
	char optionE[1000];
	char correctAnswer;
	bool answerright[5];//abcde
	int  numAnswerRight;

	while (!infile.eof())
	{
		char linecontent[1000];
		infile.getline(linecontent, 1000);

		int strlength = strlen(linecontent);

		bool isquestnum = false;
		bool isoption   = false;

		int c = 0;
		while (c < strlength)
		{
			if (linecontent[c] >= '0' && linecontent[c] <= '9')
			{
				if (isoption)
				{
					isquestnum = false;
					isoption = false;
					break;
				}
				else
				    isquestnum = true;
			}
			else if ((linecontent[c] >= 'A' && linecontent[c] <= 'E') || (linecontent[c] >= 'a' && linecontent[c] <= 'e'))
			{
				if (isquestnum || isoption)//AA AB like is not allowed so isoption must still be false
				{
					isquestnum = false;
					isoption = false;
					break;
				}
				else
					isoption = true;
			}
			else if (linecontent[c] == '.')
			{
				c++;
				while (c < strlength && (linecontent[c] == ' ' || linecontent[c] == '\t'))
				{
					c++;
				}
				break;
			}
			else
			{
				isquestnum = false;
				isoption = false;
				break;
			}
			c++;
		}

		//read body
		if (isquestnum || isoption)
		{
			if (c < strlength)
			{
				if (isquestnum)//new question 
				{
					if (isfirstquestion == false)//out put old one
					{
						FlushOneRecord(questionContent, optionA, optionB, optionC, optionD, optionE, answerright , numAnswerRight);
					}
					else
					{
						isfirstquestion = false;
					}
					int bitor = strlength-1;
					while(bitor >= 0 && (linecontent[bitor] == ' ' || linecontent[bitor] == '\t'))
					{
						bitor--;
					}
					answerright[0] = answerright[1] = answerright[2] = answerright[3] = answerright[4] = false;

					numAnswerRight = 0;
				
					while (bitor >= 0)
					{
						correctAnswer = linecontent[bitor];
						
						if (correctAnswer == 'A' || correctAnswer == 'a')
						{
							answerright[0] = true;
							linecontent[bitor] = 0;
							numAnswerRight++;
						}
						else if (correctAnswer == 'B' || correctAnswer == 'b')
						{
							answerright[1] = true;
							linecontent[bitor] = 0;
						    numAnswerRight++;
					    }
						else if(correctAnswer == 'C' || correctAnswer == 'c')
						{ 
							answerright[2] = true;	
							linecontent[bitor] = 0;
							numAnswerRight++;
						}
						else if(correctAnswer == 'D' || correctAnswer == 'd')
						{ 
							answerright[3] = true;
							linecontent[bitor] = 0;
							numAnswerRight++;
						}
						else if(correctAnswer == 'E' || correctAnswer == 'e')
						{
							answerright[4] = true;
							linecontent[bitor] = 0;
						    numAnswerRight++;
					    }
						else
						{
							break;
						}
						bitor--;
					}
					strcpy(questionContent, linecontent + c);
					
				}
				else if (isoption)
				{
					char optletter = linecontent[0];
					switch (optletter)
					{
					case 'A':
					case 'a':
						strcpy(optionA, linecontent + c);
						break;
					case 'B':
					case 'b':
						strcpy(optionB, linecontent + c);
						break;
					case 'C':
					case 'c':
						strcpy(optionC, linecontent + c);
						break;
					case 'D':
					case 'd':
						strcpy(optionD, linecontent + c);
						break;
					case 'E':
					case 'e':
						strcpy(optionE, linecontent + c);
						break;
					}
				}
			}
		}
	}
#endif
}
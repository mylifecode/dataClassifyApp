#include "SYSkillTrainingWindow.h"
#include <QPainter>
#include "TrainModuleConfig.h"
#include "MxGlobalConfig.h"
#include "RbMoviePlayer.h"
#include "MXApplication.h"
#include "SYMessageBox.h"
#include <SYUserInfo.h>
#include "SYMainWindow.h"
#include"SYDBMgr.h"
#include<QSqlRecord>
#include<QString>

#define ISLOCK Qt::UserRole+1

SYSkillTrainingWindow::SYSkillTrainingWindow(QWidget* parent)
	:QWidget(parent), m_pDocument(0), m_imageLayout(0), m_preButton(nullptr), m_lastId(-1)
{
	ui.setupUi(this);

	SetAttributeParam();
	SetItemStyle();

	m_MoviePlayer = new RbMoviePlayer(this);
	m_MoviePlayer->setVisible(false);

//	connect(ui.lb_video, SIGNAL(linkActivated(QString)), this, SLOT(openVideo(QString)));

	connect(ui.treeWidget, &QTreeWidget::itemClicked, this, &SYSkillTrainingWindow::onTreeWidgetItemClicked);

	//knowledge lib
	connect(ui.knowledgeLibBtn, &QToolButton::clicked, this, &SYSkillTrainingWindow::onButtonClicked);

	//data center
	connect(ui.dataCenterBtn, &QToolButton::clicked, this, &SYSkillTrainingWindow::onButtonClicked);

	//answer
	connect(ui.answerBtn, &QToolButton::clicked, this, &SYSkillTrainingWindow::onButtonClicked);

	//person center
	connect(ui.personCenterBtn, &QToolButton::clicked, this, &SYSkillTrainingWindow::onButtonClicked);

	connect(ui.casecontent->document(), SIGNAL(contentsChanged()), this, SLOT(textAreaChanged()));

	Mx::setWidgetStyle(this, "qss:SYSkillTrainingWindow.qss");

}

void SYSkillTrainingWindow::SetAttributeParam()
{
	auto& userName = SYUserInfo::Instance()->GetUserName();
	auto& realName = SYUserInfo::Instance()->GetRealName();
	if (realName.size())
		ui.headBtn->setText(realName);
	else
		ui.headBtn->setText(userName);

	ui.headBtn->setIcon(SYUserInfo::Instance()->GetHeadIcon());

	m_cullLeftWidth = 50;
	m_cullRightWidth = 50;
	m_cullUpHeight = 100;
	m_cullDownHeight = 130;
	//LoadDocuments(QString::fromLocal8Bit("../doc/任务说明/基础训练/《套扎训练》任务说明.pdf"));

	ui.treeWidget->clear();
	//ui.treeWidget->setHeaderLabel("");
	ui.treeWidget->header()->hide();
	ui.treeWidget->setIndentation(89);
	ui.treeWidget->setRootIsDecorated(false);
	ui.treeWidget->setIconSize(QSize(33, 33));
}

void SYSkillTrainingWindow::SetItemStyle()
{
	int userId = SYUserInfo::Instance()->GetUserId();

	TrainModuleConfig::Instance()->LoadFromXML(MxGlobalConfig::Instance()->GetCourseTrainXmlConfigFileName());

	QDomElement element = TrainModuleConfig::Instance()->m_document.documentElement();
	QDomNode node = element.firstChild();
	QString strMenuName;
	QString strMenuItemName;
	QString strObjectName;
	QTreeWidgetItem* firstItem = nullptr;

	while (!node.isNull())
	{
		if (node.isElement())
		{
			QDomElement moduleElement = node.toElement();

			if (moduleElement.attribute("ModuleName") == QString("SkillTrain"))
			{
				QDomNode subModuleNode = moduleElement.firstChild();

				while (!subModuleNode.isNull())
				{
					QDomElement subModuleEle = subModuleNode.toElement();
					QString showName = subModuleEle.attribute("ShowName");
					QString iconName = subModuleEle.attribute("IconName");
					QFont font(QString::fromLocal8Bit("微软雅黑"), 16);
					QTreeWidgetItem *itemsLevel2 = new QTreeWidgetItem(ui.treeWidget, QStringList());

					QWidget* widget = new QWidget;
					QHBoxLayout* layout = new QHBoxLayout(widget);
					QLabel* iconLabel = new QLabel;
					QLabel* itemNameLabel = new QLabel;

					QPixmap pixmap;
					pixmap.load("icons:/syskilltrainingwindow/" + iconName);
					iconLabel->setPixmap(pixmap);
					iconLabel->setObjectName("iconLabel");
					layout->addWidget(iconLabel);

					itemNameLabel->setText(showName);
					itemNameLabel->setObjectName("itemNameLabel");
					layout->addWidget(itemNameLabel);

					layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

					ui.treeWidget->setItemWidget(itemsLevel2, 0, widget);

					QDomNode trainItemNode = subModuleEle.firstChild();

					bool ok = true;
					while (!trainItemNode.isNull())
					{
						QDomElement trainItemEle = trainItemNode.toElement();
						QString showName = trainItemEle.attribute("ShowName");
						QString strId = trainItemEle.attribute("id");
						QString trainCode = trainItemEle.attribute("TrainCode");
						QString skillLevel = trainItemEle.attribute("SkillLevel");
						int level = (skillLevel).toInt();
						QTreeWidgetItem *itemsLevel3 = new QTreeWidgetItem(itemsLevel2);// , QStringList(showName));

						//添加技能锁，被添加锁的训练不能被使用
						QWidget* widget = new QWidget;
						QHBoxLayout* hLayout = new QHBoxLayout();
						QLabel* label1 = new QLabel, *label2 = new QLabel(showName);
						label2->setObjectName("showName");
						label2->setStyleSheet(QString("font-size:18;color:#ffffff;"));

						QString IconName;
						int userId = SYUserInfo::Instance()->GetUserId();
						bool ret = SYDBMgr::Instance()->QueryUserTrainIsLock(userId, trainCode);
						//为首节点直接解锁
						ret = ret || (subModuleEle.firstChild() == trainItemNode);

						if (level == 1)
						{
							if (ret)
							{
								IconName = "level1";
							}
							else
								IconName = "level1disabled";
						}
						if (level == 2)
						{
							if (ret)
							{
								IconName = "level2";
							}
							else
								IconName = "level2disabled";
						}

						if (level == 3)
						{
							if (ret)
							{
								IconName = "level3";
							}
							else
								IconName = "level3disabled";
						}

						QPixmap pixmap;
						pixmap.load("icons:/syskilltrainingwindow/" + IconName + ".png");
						label1->setPixmap(pixmap);
						label1->setFixedSize(QSize(60, 60));

						//hLayout->addSpacerItem(new QSpacerItem(10, QSizePolicy::Preferred));
						hLayout->addWidget(label1);
						hLayout->addWidget(label2);
						hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
						hLayout->setContentsMargins(0, 0, 10, 0);
						hLayout->setSpacing(10);
						widget->setLayout(hLayout);
						//itemsLevel3->setText(0, showName);
						itemsLevel3->setData(0, Qt::UserRole, strId.toInt());
						itemsLevel3->setData(0, ISLOCK, ret);
						QFont font = itemsLevel3->font(0);
						font.setPixelSize(16);
						itemsLevel3->setFont(0, font);

						if (firstItem == nullptr)
							firstItem = itemsLevel3;

						ui.treeWidget->setItemWidget(itemsLevel3, 0, widget);

						//  						QLabel * lbTrainItem = new QLabel(this);
						//  						lbTrainItem->setStyleSheet("text-decoration:none;font-size:20px;");
						//  
						//  						QString linkstr = QString("<a href=\"") + strId + QString("\">") + showName + QString("</a>");
						//  						lbTrainItem->setText(linkstr);
						//  
						//  						connect(lbTrainItem, SIGNAL(linkActivated(QString)), this, SLOT(openUrl(QString)));
						// 
						// 						ui.treeWidget->setItemWidget(itemsLevel3, 0, lbTrainItem);

						//itemsLevel3->setData(0, Qt::UserRole, strId);
						//itemsLevel3->setText(0,showName);

						trainItemNode = trainItemNode.nextSibling();
					}
					subModuleNode = subModuleNode.nextSibling();
				}
				break;
			}

		}
		node = node.nextSibling();
	}


	if (firstItem && firstItem->parent()){
		ui.treeWidget->expandItem(firstItem->parent());
		firstItem->setSelected(true);
		onTreeWidgetItemClicked(firstItem, 0);
	}

}

void SYSkillTrainingWindow::textAreaChanged()
{
	QTextDocument *document = qobject_cast<QTextDocument*>(sender());

	document->adjustSize();
	if (document)
	{
		QTextEdit *editor = qobject_cast<QTextEdit*>(document->parent()->parent());
		if (editor)
		{
			int newwidth = document->size().width() + 30;//10
			int newheight = document->size().height() + 20;//20
			if (newwidth != editor->width())
			{
				editor->setFixedWidth(newwidth);
			}
			if (newheight != editor->height())
			{
				editor->setFixedHeight(newheight);
			}
		}
	}
}

SYSkillTrainingWindow::~SYSkillTrainingWindow()
{

}


void SYSkillTrainingWindow::openUrl(QString url)
{
	bool getted = TrainModuleConfig::Instance()->GetMenuAttributeById(url, m_CurrTrainGlobalName, m_CurrTrainShowName,m_CurrTrainCode, m_CurrTrainCaseFile, m_CurrVideoFile);
	
	//ui.lb_trainname->setText(m_CurrTrainShowName);
	ui.trainNameLabel->setText(m_CurrTrainShowName);

	LoadDocuments(QString::fromLocal8Bit("../doc/任务说明/基础训练/") + m_CurrTrainCaseFile);

	setVideoInfo();
}

void SYSkillTrainingWindow::openVideo(QString url)
{
	if(m_CurrVideoFile.size()){
		m_MoviePlayer->loadMoviePath(m_CurrVideoFile);// "c:/test.avi");
		m_MoviePlayer->Play();
		m_MoviePlayer->show();
	}
	else{
		QMessageBox::information(this, "", QString::fromLocal8Bit("无视频文件"), QMessageBox::Ok);
	}
	
}

void SYSkillTrainingWindow::setVideoInfo()
{
	//ui.videoNameLabel->setText(m_CurrVideoFile);
	ui.videoNameLabel->setText(m_CurrTrainShowName);
}

void SYSkillTrainingWindow::onButtonClicked()
{
	QPushButton* toolBtn = (QPushButton*)sender();

	if(m_preButton){
		if(toolBtn == m_preButton)
			m_preButton->setChecked(true);
		else
			m_preButton->setChecked(false);
	}
	m_preButton = toolBtn;
}

void SYSkillTrainingWindow::on_knowledgeLibBtn_clicked()
{
	emit showNextWindow(WT_KnowLibWindow);
}

void SYSkillTrainingWindow::on_dataCenterBtn_clicked()
{
	emit showNextWindow(WT_DataCenterWindow);
}

void SYSkillTrainingWindow::on_answerBtn_clicked()
{
	//理论答题窗口
	int errorCode = (static_cast<MXApplication*>(qApp))->LaunchQuestionModuleProcess();
	if (errorCode != 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("QuestionModuleLaunchFailed"));
		messageBox->showFullScreen();
		messageBox->exec();
	}
}

void SYSkillTrainingWindow::on_personCenterBtn_clicked()
{
	emit showNextWindow(WT_PersonCenterWindow);
}

void SYSkillTrainingWindow::on_startTrainBtn_clicked()
{

	////confirm current train whether is locked
	QTreeWidgetItem* curItem=ui.treeWidget->currentItem();

	bool ok;
	int id = curItem->data(0, Qt::UserRole).toInt(&ok);
//ok is false ,item doesnot chosed
	if (!curItem ||(!ok))
		return;
	bool isLock = curItem->data(0, ISLOCK).toBool();

	if (!isLock)
	{
		SYMessageBox* box = new SYMessageBox(this,"", CHS("当前技能训练还未解锁，请先解锁！"),1);
		box->showFullScreen();
		box->exec();

	}
	else
	{

		std::string modulefile = MxGlobalConfig::Instance()->GetVirtualTrainModule();

		int errorCode = (static_cast<MXApplication*>(qApp))->LaunchModule(modulefile,
			m_CurrTrainGlobalName,
			m_CurrTrainShowName,
			m_CurrTrainGlobalName,
			m_CurrTrainCode,
			-1,
			-1,
			true);

		if (errorCode != 0)
		{
			SYMessageBox * messageBox = new SYMessageBox(this, CHS("技能训练模块启动失败"), CHS("error code : %1").arg(errorCode));
			messageBox->showFullScreen();
			messageBox->exec();
		}

	}
}

void SYSkillTrainingWindow::onTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
{
	bool ok;
 	int id = item->data(0, Qt::UserRole).toInt(&ok);
	if(ok){
		if(id != m_lastId){
			m_lastId = id;
			openUrl(QString().setNum(m_lastId));
		}
		
	}
	else{
		item->setExpanded(!item->isExpanded());
	}
	
	ui.treeWidget->setCurrentItem(item);

}

void SYSkillTrainingWindow::on_playBtn_clicked()
{
	openVideo(m_CurrVideoFile);
}

void SYSkillTrainingWindow::showEvent(QShowEvent* event)
{
	if(m_preButton){
		m_preButton->setChecked(false);
	}

}

void SYSkillTrainingWindow::LoadDocuments(const QString& fileName)
{
	//scrollArea->setBackgroundRole(QPalette::Dark);
	QFile textfile(fileName);
	if (textfile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream textStream(&textfile);
		ui.casecontent->setHtml(textStream.readAll());
	}
	/*
	if (m_pDocument)
	{
		delete m_pDocument;
		m_pDocument = NULL;
	}

	m_pDocument = Poppler::Document::load(fileName);
	if (!m_pDocument)
	{
		qDebug() << "load pdf file failed!!!";
		return;
	}

	//清空PDF的Label布局元素
	if (m_imageLayout)
	{
		QHBoxLayout * hLayout;

		hLayout = dynamic_cast<QHBoxLayout*>(m_imageLayout->takeAt(0)->layout());
		if(hLayout){
			QLayoutItem* layoutItem;
			while(layoutItem = hLayout->takeAt(0)){
				QSpacerItem* spacerItem = layoutItem->spacerItem();
				QWidget* widget;

				if(spacerItem){
					delete spacerItem;
				}
				else if(widget = layoutItem->widget()){
					QLabel* label = dynamic_cast<QLabel*>(widget);
					delete widget;
				}
			}

			delete hLayout;
		}
	}
	else
	{
		m_imageLayout = new QVBoxLayout(ui.scrollArea->widget());
		m_imageLayout->setSpacing(0);
		m_imageLayout->setContentsMargins(0, 0, 0, 0);
	}
	
	
	m_pDocument->setRenderHint(Poppler::Document::Antialiasing, true);			//设置图形抗锯齿
	m_pDocument->setRenderHint(Poppler::Document::TextAntialiasing, true);		//设置文本抗锯齿

	int nPage = m_pDocument->numPages();

	QPainter painter;
	
	bool needCull = m_cullLeftWidth || m_cullRightWidth || m_cullUpHeight || m_cullDownHeight;

	int nWidth = 0;
	int nHeight = 0;
	for (int i = 0; i < nPage; ++i)
	{
		Poppler::Page * pPage = m_pDocument->page(i);
		
		QImage image = pPage->renderToImage(120, 120);		//设置参数对iamge的大小有一定的影响

		if (image.isNull())
		{
			qDebug() << "image is null";
			return;
		}

		QPixmap destPixmap;
		if (needCull)
		{
			int realWidth = 800;//image.width() - m_cullLeftWidth - m_cullRightWidth;
			int realHeight = image.height() - m_cullUpHeight - m_cullDownHeight;
			assert(realWidth > 0 && realHeight > 0);
			QPixmap pixmap(QSize(realWidth, realHeight));

			painter.begin(&pixmap);
			
			painter.drawImage(0, 0, image, (image.width() - realWidth) / 2, m_cullUpHeight, realWidth, realHeight);
			painter.end();
	
			destPixmap = pixmap;
		}
		else
			destPixmap = QPixmap::fromImage(image);

		QLabel * pLabel = new QLabel();
		pLabel->resize(destPixmap.width(), destPixmap.height());
		pLabel->setMargin(0);
		pLabel->setPixmap(destPixmap);

		QHBoxLayout* hLayout = new QHBoxLayout;
		//hLayout->setMargin(0);
		hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
		hLayout->addWidget(pLabel);
		hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

		m_imageLayout->addLayout(hLayout);
		nWidth = pLabel->width();
		nHeight += pLabel->height();
	}

	ui.scrollArea->widget()->setGeometry(0, 0, nWidth, nHeight);
	*/
}
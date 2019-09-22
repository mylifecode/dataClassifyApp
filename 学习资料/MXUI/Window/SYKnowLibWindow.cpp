#include "SYKnowLibWindow.h"
#include "SYTabPageToolIntro.h"
#include "SYTabPageLibDoc.h"
#include "SYTabPageLibVideo.h"
#include <QListWidget>
#include <QStringList>
#include <QDir>
#include "MxDefine.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "MXApplication.h"
#include <QTreeWidget>

SYKnowLibWindow::SYKnowLibWindow(QWidget *parent)
	: QWidget(parent),
	m_preToolButton(nullptr),
	m_preDocumentCatButton(nullptr),
	m_preVideoCatButton(nullptr),
	m_preButton(nullptr),
	m_treeWidget(nullptr),
	m_pageTool(nullptr),
	m_pageDoc(nullptr),
	m_pageVideo(nullptr)
{
	ui.setupUi(this);
	m_preButton = ui.knowledgeLibBtn;

	auto getFilledName = [](const QString& oldName){
		QString newName = oldName;
		newName.insert(0, "      ");
// 		int oldSize = oldName.size();
// 		int d = 6 - oldSize;
// 		if(d > 0){
// 			for(int i = 0; i < d; ++i){
// 				newName.append("--");
// 			}
// 		}
		return newName;
	};

	//parse external config file
	QTreeWidgetItem* firstItem = nullptr;

	const QString& knowledgeConfigFile = MxGlobalConfig::Instance()->GetKnowledgeLibConfigFile();
	if(QFile::exists(knowledgeConfigFile)){
		QFile file(knowledgeConfigFile);
		if(file.open(QIODevice::ReadOnly)){
			QDomDocument document;
			document.setContent(&file);
			file.close();

			QDomElement element = document.documentElement();
			QDomNode node = element.firstChild();

			while(!node.isNull())
			{
				if(node.isElement())
				{
					QDomElement subpageElement = node.toElement();
					QString id = subpageElement.attribute("Id");

					if(id == QString("0"))
					{
						ToolInfo toolInfo;
						QDomNode toolNode = subpageElement.firstChild();
						QVBoxLayout* layout = new QVBoxLayout;
						QFrame* frame = new QFrame;
						frame->setObjectName("toolFrame");
						frame->setContentsMargins(0, 0, 0, 0);
						frame->setLayout(layout);
						layout->setSpacing(0);
						layout->setMargin(0);

						while(!toolNode.isNull())
						{
							QDomElement toolElement = toolNode.toElement();
							toolInfo.button = new QPushButton;
							toolInfo.button->setCheckable(true);
							toolInfo.toolName = toolElement.attribute("ToolName");
							toolInfo.button->setText(getFilledName(toolInfo.toolName));

							QString controlName = toolElement.attribute("ControlObjectName");
							toolInfo.button->setObjectName(controlName);

							toolInfo.videoFileName = toolElement.attribute("VideoFileName");
							toolInfo.description = toolElement.attribute("Description");

							layout->addWidget(toolInfo.button);
							m_toolInfos.push_back(toolInfo);

							connect(toolInfo.button, &QPushButton::clicked, this, &SYKnowLibWindow::onToolButtonClicked);

							toolNode = toolNode.nextSibling();
						}

						layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
						ui.stackedWidget->addWidget(frame);
					}
					else if(id == QString("1")){
						SurgeryDocumentInfo surgeryDocumentInfo;
						QDomNode documentNode = subpageElement.firstChild();
						//QVBoxLayout* layout = new QVBoxLayout;
						m_treeWidget = new QTreeWidget;
						m_treeWidget->setObjectName("surgeryDocumentCategoryFrame");
						m_treeWidget->setContentsMargins(0, 0, 0, 0);
						m_treeWidget->header()->hide();
						m_treeWidget->setIndentation(89);
						m_treeWidget->setRootIsDecorated(false);
						m_treeWidget->setIconSize(QSize(33, 33));
						m_treeWidget->setFocusPolicy(Qt::NoFocus);
						//treeWidget->setLayout(layout);
						//layout->setSpacing(0);
						//layout->setMargin(0);
						connect(m_treeWidget, &QTreeWidget::itemClicked, this, &SYKnowLibWindow::onSurgeryDocumentItemClicked);

						while(!documentNode.isNull())
						{
							QDomElement documentElement = documentNode.toElement();
							//surgeryDocumentInfo.button = new QPushButton;
							//surgeryDocumentInfo.button->setCheckable(true);
							surgeryDocumentInfo.documentCategoryName = documentElement.attribute("SurgeryDocumentCategoryName");
							//surgeryDocumentInfo.button->setText("   " + surgeryDocumentInfo.documentCategoryName);

							QString controlName = documentElement.attribute("ControlObjectName");
							QString iconName = documentElement.attribute("IconName");
							//surgeryDocumentInfo.button->setObjectName(controlName);


							QTreeWidgetItem* item1 = new QTreeWidgetItem(m_treeWidget);
							//item1->setText(0, surgeryDocumentInfo.documentCategoryName);

							QWidget* widget = new QWidget;
							QHBoxLayout* layout = new QHBoxLayout(widget);
							QLabel* iconLabel = new QLabel;
							QLabel* itemNameLabel = new QLabel;

							QPixmap pixmap;
							pixmap.load("icons:/syknowledgelib/" + iconName);
							iconLabel->setPixmap(pixmap);
							iconLabel->setObjectName("iconLabel");
							layout->addWidget(iconLabel);

							itemNameLabel->setText(surgeryDocumentInfo.documentCategoryName);
							itemNameLabel->setObjectName("itemNameLabel");
							layout->addWidget(itemNameLabel);

							layout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

							m_treeWidget->setItemWidget(item1, 0, widget);

							//surgeryDocumentInfo.description = documentElement.attribute("Description");
							QString attr("DocumentFile%1");
							QString docFile;
							surgeryDocumentInfo.documentFiles.clear();
							for(int i = 1; i <= 6; ++i){
								docFile = documentElement.attribute(attr.arg(i));
								if(docFile.size() > 0){
									int n = docFile.size();
									QString showName = docFile;
									int index1 = docFile.lastIndexOf("/");
									int index2 = docFile.lastIndexOf("\\");
									
									if(index1 != -1 || index2 != -1){
										if(index1 > index2)
											showName = docFile.right(n - index1 - 1);
										else
											showName = docFile.right(n - index2 - 1);
									}

									int index = showName.lastIndexOf(".");
									if(index != -1){
										n = showName.size();
										showName = showName.left(index);
									}

									QTreeWidgetItem* item2 = new QTreeWidgetItem(item1);
									item2->setText(0,showName);
									item2->setData(0, Qt::UserRole, docFile);

									//set font size
									QFont font = item2->font(0);
									font.setPixelSize(16);
									item2->setFont(0, font);

									if(firstItem == nullptr)
										firstItem = item2;

									surgeryDocumentInfo.documentFiles.push_back(docFile);
								}
								else
									break;
							}

							//layout->addWidget(surgeryDocumentInfo.button);
							m_surgeryDocumentInfos.push_back(surgeryDocumentInfo);

							//connect(surgeryDocumentInfo.button, &QPushButton::clicked, this, &SYKnowLibWindow::onSurgeryDocumentCatButtonClicked);

							documentNode = documentNode.nextSibling();
						}

						//layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
						ui.stackedWidget->addWidget(m_treeWidget);
					}
					else if(id == QString("2")){
						SurgeryVideoInfo surgeryVideoInfo;
						QDomNode videoNode = subpageElement.firstChild();
						QVBoxLayout* layout = new QVBoxLayout;
						QFrame* frame = new QFrame;
						frame->setObjectName("surgeryVideoCategoryFrame");
						frame->setContentsMargins(0, 0, 0, 0);
						frame->setLayout(layout);
						layout->setSpacing(0);
						layout->setMargin(0);

						while(!videoNode.isNull())
						{
							QDomElement videoElement = videoNode.toElement();
							surgeryVideoInfo.button = new QPushButton;
							surgeryVideoInfo.button->setCheckable(true);
							surgeryVideoInfo.videoCategoryName = videoElement.attribute("SurgeryVideoCategoryName");
							surgeryVideoInfo.button->setText("   " + surgeryVideoInfo.videoCategoryName);

							QString controlName = videoElement.attribute("ControlObjectName");
							surgeryVideoInfo.button->setObjectName(controlName);

							layout->addWidget(surgeryVideoInfo.button);
							
							QString attri("VideoFileName%1");
							QString videoFile;
							surgeryVideoInfo.videoFiles.clear();
							for(int i = 1; i < 12; ++i){
								videoFile = videoElement.attribute(attri.arg(i));
								if(videoFile.size() > 0)
									surgeryVideoInfo.videoFiles.push_back(videoFile);
								else
									break;
							}
							m_surgeryVideoInfos.push_back(surgeryVideoInfo);

							connect(surgeryVideoInfo.button, &QPushButton::clicked, this, &SYKnowLibWindow::onSurgeryVideoCatButtonClicked);

							videoNode = videoNode.nextSibling();
						}

						layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));
						ui.stackedWidget->addWidget(frame);
					}
				}
				node = node.nextSibling();
			}
		}
	}

	//set page
	m_pageTool = new SYTabPageToolIntro(this);
	m_pageDoc = new SYTabPageLibDoc(this);
	m_pageVideo = new SYTabPageLibVideo(this);

	ui.tabWidget->addTab(m_pageTool, QString::fromLocal8Bit("常用器械"));
	ui.tabWidget->addTab(m_pageDoc, QString::fromLocal8Bit("参考文档"));
	ui.tabWidget->addTab(m_pageVideo, QString::fromLocal8Bit("参考视频"));

	// 	ui.tabWidget->setIconSize(QSize(12, 12));
	// 	ui.tabWidget->setTabIcon(0, QIcon("icons:/SYPersonCenterWindow/blue.png"));
	// 	ui.tabWidget->setTabIcon(1, QIcon("icons:/SYPersonCenterWindow/green.png"));
	// 	ui.tabWidget->setTabIcon(2, QIcon("icons:/SYPersonCenterWindow/yellow.png"));

	connect(ui.tabWidget, &QTabWidget::tabBarClicked, this, &SYKnowLibWindow::onTabBarClicked);


	//set button and page info
	if(m_toolInfos.size() > 0){
		m_preToolButton = m_toolInfos[0].button;
		m_preToolButton->setChecked(true);
		m_pageTool->setToolInfo(m_toolInfos[0].toolName,
							  m_toolInfos[0].videoFileName,
							  m_toolInfos[0].description);
	}

	if(m_surgeryDocumentInfos.size() > 0){
		//m_preDocumentCatButton = m_surgeryDocumentInfos[0].button;
		//m_preDocumentCatButton->setChecked(true);

		//QStringList contents;
		//contents.append(m_surgeryDocumentInfos[0].description);
		//pageDoc->setContents(contents);
		for(int i = 0; i < m_surgeryDocumentInfos.size(); ++i){
			auto& documentFiles = m_surgeryDocumentInfos[i].documentFiles;
			if(documentFiles.size() > 0){
				m_pageDoc->setContent(documentFiles[0]);
				break;
			}
		}
	}

	if(firstItem && firstItem->parent()){
		m_treeWidget->expandItem(firstItem->parent());
		firstItem->setSelected(true);
	}

	if(m_surgeryVideoInfos.size() > 0){
		m_preVideoCatButton = m_surgeryVideoInfos[0].button;
		m_preVideoCatButton->setChecked(true);

		//QStringList videos;
		//videos.append(m_surgeryVideoInfos[0].videoCategoryName);
		m_pageVideo->setVideos(m_surgeryVideoInfos[0].videoFiles);
	}

	//right frame
	const QString& userName = SYUserInfo::Instance()->GetUserName();
	const QString& realName = SYUserInfo::Instance()->GetRealName();
	if(realName.size())
		ui.headBtn->setText(realName);
	else
		ui.headBtn->setText(userName);

	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();

	ui.headBtn->setIcon(headIcon);

	//knowledge lib
	connect(ui.knowledgeLibBtn, &QToolButton::clicked, this, &SYKnowLibWindow::onRightButtonClicked);

	//data center
	connect(ui.dataCenterBtn, &QToolButton::clicked, this, &SYKnowLibWindow::onRightButtonClicked);

	//answer
	connect(ui.answerBtn, &QToolButton::clicked, this, &SYKnowLibWindow::onRightButtonClicked);

	//person center
	connect(ui.personCenterBtn, &QToolButton::clicked, this, &SYKnowLibWindow::onRightButtonClicked);

	Mx::setWidgetStyle(this,"qss:SYKnowLibWindow.qss");
}

SYKnowLibWindow::~SYKnowLibWindow(void)
{

}

void SYKnowLibWindow::onTabBarClicked(int index)
{
	ui.stackedWidget->setCurrentIndex(index);
}

void SYKnowLibWindow::onToolButtonClicked()
{
	auto* button = static_cast<QPushButton*>(sender());
	if(m_preToolButton){
		m_preToolButton->setChecked(false);
	}

	if(button != m_preToolButton){
		for(const auto& toolInfo : m_toolInfos){
			if(toolInfo.button == button){
				auto toolWidget = static_cast<SYTabPageToolIntro*>(ui.tabWidget->widget(0));
				toolWidget->setToolInfo(toolInfo.toolName, toolInfo.videoFileName, toolInfo.description);
				break;
			}
		}

		m_preToolButton = button;
	}
	
	button->setChecked(true);
}

void SYKnowLibWindow::onSurgeryDocumentCatButtonClicked()
{
// 	auto* button = static_cast<QPushButton*>(sender());
// 	if(m_preDocumentCatButton){
// 		m_preDocumentCatButton->setChecked(false);
// 	}
// 
// 	if(button != m_preDocumentCatButton){
// 		for(const auto& docInfo : m_surgeryDocumentInfos){
// 			if(docInfo.button == button){
// 				auto docWidget = static_cast<SYTabPageLibDoc*>(ui.tabWidget->widget(1));
// // 				QStringList contents;
// // 				contents.append(docInfo.description);
// // 				contents.append(docInfo.description);
// // 				contents.append(docInfo.description);
// // 				contents.append(docInfo.description);
// // 				contents.append(docInfo.description);
// // 				contents.append(docInfo.description);
// 				docWidget->setContents(docInfo.documentFiles);
// 				break;
// 			}
// 		}
// 
// 		m_preDocumentCatButton = button;
// 	}
// 
// 	button->setChecked(true);
}

void SYKnowLibWindow::onSurgeryDocumentItemClicked(QTreeWidgetItem* item, int col)
{
	QString file = item->data(0, Qt::UserRole).toString();
	if(file.size() > 0){
		m_pageDoc->setContent(file);
	}
	else{
		item->setExpanded(!item->isExpanded());
	}
}

void SYKnowLibWindow::onSurgeryVideoCatButtonClicked()
{
	auto* button = static_cast<QPushButton*>(sender());
	if(m_preVideoCatButton){
		m_preVideoCatButton->setChecked(false);
	}

	if(button != m_preVideoCatButton){
		for(const auto& videoInfo : m_surgeryVideoInfos){
			if(videoInfo.button == button){
				auto docWidget = static_cast<SYTabPageLibVideo*>(ui.tabWidget->widget(2));
				docWidget->setVideos(videoInfo.videoFiles);
				break;
			}
		}

		m_preVideoCatButton = button;
	}

	button->setChecked(true);
}

void SYKnowLibWindow::onRightButtonClicked()
{
	auto* button = static_cast<QPushButton*>(sender());

	if(m_preButton)
		m_preButton->setChecked(false);

	button->setChecked(true);
	m_preButton = button;

	if(button == ui.knowledgeLibBtn)
		return;
	else if(button == ui.dataCenterBtn)
		emit showNextWindow(WT_DataCenterWindow);
	else if(button == ui.personCenterBtn)
		emit showNextWindow(WT_PersonCenterWindow);
	else if (button == ui.answerBtn)
	{
		int errorCode = (static_cast<MXApplication*>(qApp))->LaunchQuestionModuleProcess();
		if (errorCode != 0)
		{
			QMessageBox::information(this, "error", "QuestionMouduleLaunchFailed", "ok");
		}
	}
}

void SYKnowLibWindow::showEvent(QShowEvent* event)
{
	//如果性别有修改则刷新头像
	QIcon headIcon = SYUserInfo::Instance()->GetHeadIcon();
	ui.headBtn->setIcon(headIcon);

	m_preButton = ui.knowledgeLibBtn;
	ui.knowledgeLibBtn->setChecked(true);
	ui.answerBtn->setChecked(false);
	ui.dataCenterBtn->setChecked(false);
	ui.personCenterBtn->setChecked(false);
}
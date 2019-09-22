#include "MxDefine.h"
#include "SYSurgeryTrainWindow.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include <QFile>
#include "SYMessageBox.h"
#include "MXApplication.h"


SYSurgeryTrainModule::SYSurgeryTrainModule(QWidget *parent) 
	: QWidget(parent),
	m_preFlodBtn(nullptr),
	m_preTrainBtn(nullptr),
	m_preCaseBtn(nullptr),
	m_id(0)
{
	ui.setupUi(this);

	connect(ui.bt_start, SIGNAL(clicked()), this, SLOT(onStartButtonClicked()));
}

SYSurgeryTrainModule::~SYSurgeryTrainModule(void)
{

}

void SYSurgeryTrainModule::LoadCaseFile(const QString & htmlfile)
{
	QFile textfile(htmlfile);
	if (textfile.open(QFile::ReadOnly | QFile::Text))
	{
		QTextStream textStream(&textfile);
		ui.casecontent->setHtml(textStream.readAll());
	}
}

void SYSurgeryTrainModule::SetModuleName(const QString& moduleName)
{
	m_moduleName = moduleName;
	ui.casename->setText(moduleName);
}

void SYSurgeryTrainModule::AddTrainInfo(const QString& trainName)
{
	for(const auto& trainInfo : m_trainInfos){
		if(trainInfo.trainName == trainName)
			return;
	}

	TrainInfo trainInfo;
	trainInfo.trainName = trainName;
	m_trainInfos.push_back(trainInfo);
}

void SYSurgeryTrainModule::AddCaseInfo(const QString& trainName,const QString& trainEnName, const QString& caseName, const QString& caseFileName)
{
	for(auto& trainInfo : m_trainInfos){
		if(trainInfo.trainName == trainName){
			CaseInfo caseInfo;
			caseInfo.trainEnName = trainEnName;
			caseInfo.caseName = caseName;
			caseInfo.caseFileName = caseFileName;
			
			trainInfo.caseInfos.push_back(caseInfo);
			break;
		}
	}
}

void SYSurgeryTrainModule::SetTrainLayout()
{
	QVBoxLayout* trainLayout = new QVBoxLayout;
	trainLayout->setMargin(0);
	trainLayout->setContentsMargins(0, 20, 0, 7);
	trainLayout->setSpacing(20);
	ui.rightFrame->setLayout(trainLayout);

	for(std::size_t i = 0; i < m_trainInfos.size();++i){
		auto& trainInfo = m_trainInfos[i];
		QVBoxLayout* vLayout = new QVBoxLayout;
		vLayout->setMargin(0);
		//vLayout->setSpacing(25);
		//vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
		
		QFrame* caseFrame = new QFrame;
		caseFrame->setObjectName("caseFrame" + QString().setNum(m_id));
		caseFrame->setLayout(vLayout);
		//caseFrame->setMaximumHeight((trainInfo.caseInfos.size() + 1) * 60);

		//add train button
		QPushButton* trainBtn = new QPushButton;
		trainBtn->setText(trainInfo.trainName);
		trainBtn->setObjectName("trainBtn");
		trainBtn->setEnabled(false);
		trainBtn->setCheckable(true);
		trainBtn->setChecked(false);
		m_trainButtons.push_back(trainBtn);

		vLayout->addWidget(trainBtn);

		for(std::size_t caseIndex = 0; caseIndex < trainInfo.caseInfos.size();++caseIndex){
			auto& caseInfo = trainInfo.caseInfos[caseIndex];
			QPushButton* caseBtn = new QPushButton();
			caseBtn->setObjectName("caseBtn");
			caseBtn->setText(caseInfo.caseName);
			caseBtn->setCheckable(true);
			caseBtn->setChecked(false);
			caseBtn->setVisible(false);
			caseBtn->setProperty("trainIndex", i);
			caseBtn->setProperty("caseIndex", caseIndex);
			connect(caseBtn, &QPushButton::clicked, this, &SYSurgeryTrainModule::onCaseBtnClicked);

			vLayout->addWidget(caseBtn);
			trainInfo.caseButtons.push_back(caseBtn);
		}

		//add flod button
		QPushButton* flodButton = new QPushButton;
		flodButton->setObjectName("flodBtn");
		flodButton->setCheckable(true);
		flodButton->setChecked(false);
		flodButton->setProperty("trainIndex", i);
		connect(flodButton, &QPushButton::clicked, this, &SYSurgeryTrainModule::onClickedFlodBtn);

		vLayout->addWidget(flodButton);
		//vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
		m_flodButtons.push_back(flodButton);

		trainLayout->addWidget(caseFrame);
	}

	trainLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

void SYSurgeryTrainModule::onClickedFlodBtn()
{
	QPushButton* flodBtn = (QPushButton*)sender();
	std::size_t trainIndex = flodBtn->property("trainIndex").toUInt();
	if(trainIndex < m_trainInfos.size()){
		const auto& trainInfo = m_trainInfos[trainIndex];

		if(flodBtn != m_preFlodBtn){
			if(m_preFlodBtn){
				std::size_t preIndex = m_preFlodBtn->property("trainIndex").toInt();
				const auto& preTrainInfo = m_trainInfos[preIndex];
				for(auto cb : preTrainInfo.caseButtons)
					cb->setVisible(false);
				m_preFlodBtn->setChecked(false);
			}

			for(auto cb : trainInfo.caseButtons)
				cb->setVisible(true);
		}
		else{
			for(auto cb : trainInfo.caseButtons)
				cb->setVisible(!cb->isVisible());
		}


		m_preFlodBtn = flodBtn;
	}
}

void SYSurgeryTrainModule::onCaseBtnClicked()
{
	QPushButton* btn = (QPushButton*)sender();
	std::size_t trainIndex = btn->property("trainIndex").toUInt();
	std::size_t caseIndex = btn->property("caseIndex").toUInt();
	const auto& trainInfo = m_trainInfos[trainIndex];
	const auto& caseInfo = trainInfo.caseInfos[caseIndex];

	if(m_preCaseBtn)
		m_preCaseBtn->setChecked(false);

	btn->setChecked(true);

	m_preCaseBtn = btn;

	
	if(m_preTrainBtn)
		m_preTrainBtn->setChecked(false);
	QPushButton* curTrainBtn = m_trainButtons[trainIndex];
	curTrainBtn->setChecked(true);
	m_preTrainBtn = curTrainBtn;

	LoadCaseFile(caseInfo.caseFileName);
}

void SYSurgeryTrainModule::onStartButtonClicked()
{
	if(m_preCaseBtn){
		std::size_t trainIndex = m_preCaseBtn->property("trainIndex").toUInt();
		std::size_t caseIndex = m_preCaseBtn->property("caseIndex").toUInt();
		const auto& trainInfo = m_trainInfos[trainIndex];
		const auto& caseInfo = trainInfo.caseInfos[caseIndex];

		std::string modulefile = MxGlobalConfig::Instance()->GetVirtualTrainModule();

		int errorCode = (static_cast<MXApplication*>(qApp))->LaunchModule(modulefile,
																		  caseInfo.trainEnName,
																		  caseInfo.caseName,
																		  trainInfo.trainName,
																		  "",
																		  -1, -1, true);

		if(errorCode != 0)
		{
			SYMessageBox * messageBox = new SYMessageBox(this, CHS("¼¼ÄÜÑµÁ·Ä£¿éÆô¶¯Ê§°Ü"), CHS("error code : %1").arg(errorCode));
			messageBox->showFullScreen();
			messageBox->exec();
		}
	}

	
}

void SYSurgeryTrainModule::showEvent(QShowEvent* event)
{
	//ÉèÖÃÄ¬ÈÏµÄ²¡Àú
	if(m_trainInfos.size() > 0){
		TrainInfo& trainInfo = m_trainInfos[0];
		if(trainInfo.caseInfos.size() > 0){
			CaseInfo& caseInfo = trainInfo.caseInfos[0];
			LoadCaseFile(caseInfo.caseFileName);
		}

		m_preTrainBtn = m_trainButtons[0];
		m_preTrainBtn->setChecked(true);

		if(trainInfo.caseButtons.size()){
			for(auto* caseBtn : trainInfo.caseButtons)
				caseBtn->setVisible(true);

			m_preCaseBtn = trainInfo.caseButtons[0];
			m_preCaseBtn->setChecked(true);
		}
		
	}

	if(m_flodButtons.size() > 0){
		m_preFlodBtn = m_flodButtons[0];
		m_preFlodBtn->setChecked(true);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
SYSurgeryTrainWindow::SYSurgeryTrainWindow(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);

	int numCases = 2;

	Mx::setWidgetStyle(this, "qss:SYSurgeryTrainWindow.qss");


	//parse external config file
	const QString& configFile = MxGlobalConfig::Instance()->GetSurgeryTrainConfigFile();
	if(QFile::exists(configFile)){
		QFile file(configFile);
		if(file.open(QIODevice::ReadOnly)){
			QDomDocument document;
			document.setContent(&file);
			file.close();

			QDomElement element = document.documentElement();
			QDomNode node = element.firstChild();
			QDomNode trainNode, caseNode;
			QDomElement trainElement, caseElement;
			QString trainName,trainEnName,caseName,caseFileName;
			int id = 0;

			while(!node.isNull())
			{
				QString moduleName;
				SYSurgeryTrainModule* module = nullptr;

				//module
				if(node.isElement())
				{
					QDomElement moduleElement = node.toElement();
					moduleName = moduleElement.attribute("MouduleName");
					module = GetSurgeryTrainModule(moduleName);
					if(module == nullptr){
						module = new SYSurgeryTrainModule(this);
						module->SetModuleName(moduleName);
						module->SetId(id);
						module->GetUI().bodyframe->setObjectName(QString("bodyframe%1").arg(id++));
						ui.layout_case->addWidget(module);
						m_moduleWindows.push_back(module);
					}
					
					//train
					trainNode = moduleElement.firstChild();
					while(trainNode.isNull() == false){
						if(trainNode.isElement()){
							trainElement = trainNode.toElement();
							trainName = trainElement.attribute("TrainName");

							module->AddTrainInfo(trainName);

							//case
							caseNode = trainElement.firstChild();
							while(caseNode.isNull() == false){
								if(caseNode.isElement()){
									caseElement = caseNode.toElement();
									trainEnName = caseElement.attribute("TrainName");
									caseName = caseElement.attribute("CaseName");
									caseFileName = caseElement.attribute("CaseFile");

									module->AddCaseInfo(trainName, trainEnName, caseName, caseFileName);
								}

								caseNode = caseNode.nextSibling();
							}
						}

						trainNode = trainNode.nextSibling();
					}
				}

				node = node.nextSibling();
			}
		}
	}

	for(auto m : m_moduleWindows)
		m->SetTrainLayout();

// 	for (int c = 0; c < numCases; c++)
// 	{
// 		SYSurgeryTrainModule * caseWin = new SYSurgeryTrainModule(this);
// 		
// 		caseWin->GetUI().bodyframe->setObjectName(QString("bodyframe%1").arg(c));
// 		
// 		caseWin->GetUI().bt_start->setProperty("caseindex", static_cast<int>(c));
// 
// 		connect(caseWin->GetUI().bt_start, SIGNAL(clicked()), this, SLOT(onStartButtonClicked()));
// 
// 
// 		QString caseFile = QString::fromLocal8Bit("../doc/²¡Àú/µ¨ÄÒÇÐ³ýÊõÈ«Ì×ÑµÁ·/") + QString::fromLocal8Bit("case%1.html").arg(c+1);
// 		caseWin->LoadCaseFile(caseFile);
// 
// 		QHBoxLayout * pHLayout = new QHBoxLayout(this);
// 		pHLayout->addWidget(caseWin);
// 		ui.layout_case->addItem(pHLayout);
// 	}

	
}

SYSurgeryTrainWindow::~SYSurgeryTrainWindow(void)
{
	
}


void SYSurgeryTrainWindow::showEvent(QShowEvent* event)
{
	
}

void SYSurgeryTrainWindow::LaunchRealTrainModule()
{
	
}
void SYSurgeryTrainWindow::mousePressEvent(QMouseEvent* mouseEvent)
{
	
}

void SYSurgeryTrainWindow::onStartButtonClicked()
{
	QPushButton* toolBtn = (QPushButton*)sender();

	int caseIndex = static_cast<WindowType>(toolBtn->property("caseindex").toInt());

	QString trainName = (caseIndex == 0 ? QString::fromLocal8Bit("GallTrain1") : QString::fromLocal8Bit("GallTrain2"));

	std::string modulefile = MxGlobalConfig::Instance()->GetVirtualTrainModule();

	int errorCode = (static_cast<MXApplication*>(qApp))->LaunchModule(modulefile,
		                                                              trainName, 
																	  QString::fromLocal8Bit("ÂýÐÔµ¨ÄÒÑ×\\²¡Àý1"),
																	  trainName,"",-1,-1,true);

	if (errorCode != 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS("¼¼ÄÜÑµÁ·Ä£¿éÆô¶¯Ê§°Ü"), CHS("error code : %1").arg(errorCode));
		messageBox->showFullScreen();
		messageBox->exec();
	}
}

SYSurgeryTrainModule* SYSurgeryTrainWindow::GetSurgeryTrainModule(const QString& moduleName)
{
	for(auto m : m_moduleWindows){
		if(m->GetModuleName() == moduleName)
			return m;
	}

	return nullptr;
}
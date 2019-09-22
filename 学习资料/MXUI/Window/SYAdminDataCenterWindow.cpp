#include "SYAdminDataCenterWindow.h"
#include <MxDefine.h>
#include "SYStringTable.h"
#include "SYAdminEntityTrainAnalysisWindow.h"
#include "SYAdminSkillTrainAnalysisWindow.h"
#include "SYAdminSurgeryTrainAnalysisWindow.h"

SYAdminDataCenterWindow::SYAdminDataCenterWindow(QWidget* parent)
	:QWidget(parent),
	m_entityTrainAnalysisBtn(nullptr),
	m_skillTrainAnalysisBtn(nullptr),
	m_surgeryTrainAnalysisBtn(nullptr),
	m_stackedWidget(nullptr),
	m_curSelectedBtn(nullptr),
	m_entityTrainAnalysisWindow(nullptr),
	m_skillTrainAnalysisWindow(nullptr),
	m_surgeryTrainAnalysisWindow(nullptr)
{
	ui.setupUi(this);

	InitLayout();

	Mx::setWidgetStyle(this, "qss:SYAdminDataCenterWindow.qss");
}

SYAdminDataCenterWindow::~SYAdminDataCenterWindow()
{

}

void SYAdminDataCenterWindow::InitLayout()
{
	//training analysis button
	m_entityTrainAnalysisBtn = new QPushButton();
	m_entityTrainAnalysisBtn->setObjectName("entityTrainAnalysisBtn");
	m_entityTrainAnalysisBtn->setCheckable(true);
	m_entityTrainAnalysisBtn->setChecked(true);
	m_entityTrainAnalysisBtn->setText(SYStringTable::GetString(SYStringTable::STR_ENTITYTRAINANALYSIS));
	connect(m_entityTrainAnalysisBtn, &QPushButton::clicked, this, &SYAdminDataCenterWindow::onEntityTrainAnalysisBtn);
	m_curSelectedBtn = m_entityTrainAnalysisBtn;

	m_skillTrainAnalysisBtn = new QPushButton();
	m_skillTrainAnalysisBtn->setObjectName("skillTrainAnalysisBtn");
	m_skillTrainAnalysisBtn->setCheckable(true);
	m_skillTrainAnalysisBtn->setChecked(false);
	m_skillTrainAnalysisBtn->setText(SYStringTable::GetString(SYStringTable::STR_SKILLTRAINANALYSIS));
	connect(m_skillTrainAnalysisBtn, &QPushButton::clicked, this, &SYAdminDataCenterWindow::onSkillTrainAnalysisBtn);

	m_surgeryTrainAnalysisBtn = new QPushButton();
	m_surgeryTrainAnalysisBtn->setObjectName("surgeryTrainAnalysisBtn");
	m_surgeryTrainAnalysisBtn->setCheckable(true);
	m_surgeryTrainAnalysisBtn->setChecked(false);
	m_surgeryTrainAnalysisBtn->setText(SYStringTable::GetString(SYStringTable::STR_SURGERYTRAINANALYSIS));
	connect(m_surgeryTrainAnalysisBtn, &QPushButton::clicked, this, &SYAdminDataCenterWindow::onSurgeryTrainAnalysisBtn);

	//add button into layout
	QHBoxLayout* hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->setSpacing(0);

	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout->addWidget(m_entityTrainAnalysisBtn);
	hLayout->addWidget(m_skillTrainAnalysisBtn);
	hLayout->addWidget(m_surgeryTrainAnalysisBtn);
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

	//stacked widget
	m_stackedWidget = new QStackedWidget();
	m_entityTrainAnalysisWindow = new SYAdminEntityTrainAnalysisWindow;
	m_entityTrainAnalysisWindow->LoadTrainStatisticData();
	m_stackedWidget->addWidget(m_entityTrainAnalysisWindow);

	m_skillTrainAnalysisWindow = new SYAdminSkillTrainAnalysisWindow;
	m_skillTrainAnalysisWindow->LoadTrainStatisticData();
	m_skillTrainAnalysisWindow->LoadAbilityCounterData();
	m_stackedWidget->addWidget(m_skillTrainAnalysisWindow);

	m_surgeryTrainAnalysisWindow = new SYAdminSurgeryTrainAnalysisWindow;
	m_surgeryTrainAnalysisWindow->LoadTrainStatisticData();
	m_surgeryTrainAnalysisWindow->LoadAbilityCounterData();
	m_stackedWidget->addWidget(m_surgeryTrainAnalysisWindow);

	m_stackedWidget->setCurrentWidget(m_entityTrainAnalysisWindow);

	//set vertical layout for scroll area contents
	QVBoxLayout* vLayout = new QVBoxLayout;
	vLayout->setContentsMargins(50, 40, 50, 0);
	vLayout->setSpacing(0);

	vLayout->addLayout(hLayout);
	vLayout->addWidget(m_stackedWidget);
	vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));

	ui.scrollAreaWidgetContents->setLayout(vLayout);
}

void SYAdminDataCenterWindow::onEntityTrainAnalysisBtn()
{
	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = m_entityTrainAnalysisBtn;
	m_curSelectedBtn->setChecked(true);

	m_stackedWidget->setCurrentWidget(m_entityTrainAnalysisWindow);
}

void SYAdminDataCenterWindow::onSkillTrainAnalysisBtn()
{
	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = m_skillTrainAnalysisBtn;
	m_curSelectedBtn->setChecked(true);

	m_stackedWidget->setCurrentWidget(m_skillTrainAnalysisWindow);
}

void SYAdminDataCenterWindow::onSurgeryTrainAnalysisBtn()
{
	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = m_surgeryTrainAnalysisBtn;
	m_curSelectedBtn->setChecked(true);

	m_stackedWidget->setCurrentWidget(m_surgeryTrainAnalysisWindow);
}
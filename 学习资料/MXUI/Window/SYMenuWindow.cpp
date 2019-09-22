#include "SYMenuWindow.h"
#include "MxDefine.h"

SYMenuWindow::SYMenuWindow(QWidget* parent)
	:QWidget(parent),
	m_curSelectedBtn(nullptr)
{
	ui.setupUi(this);

	const int nButton = 8;

	WindowType nextWindowTypes[nButton] = {WT_AdminPersonWindow, 
											WT_AdminTaskWindow,
											WT_AdminTheoryTestWindow,
											WT_AdminTrainingCenterWindow, 
											WT_AdminDataCenterWindow, 
											WT_PersonCenterWindow,
											WT_MonitorWindow,
											WT_AdminKnowledgeSetManageWindow };
	QPushButton* buttons[nButton] = {ui.personMgrBtn, ui.trainingMgrBtn, ui.theoryMgrBtn,ui.trainingCenterBtn, ui.dataCenterBtn, ui.personCenterBtn, ui.networkCenterBtn,ui.knowledgeLibMgrBtn};

	for(int i = 0; i < nButton; ++i){
		QPushButton* button = buttons[i];
		button->setProperty("windowType", static_cast<int>(nextWindowTypes[i]));//���ð�ť����
		connect(button, &QPushButton::clicked, this, &SYMenuWindow::onClickedBtn);  //���õ���¼�
	}

	m_curSelectedBtn = ui.personMgrBtn;    //��ǰѡ�а���
	m_curSelectedBtn->setChecked(true);
	 
	Mx::setWidgetStyle(this, "qss:SYMenuWindow.qss");   //������ʽ
}

SYMenuWindow::~SYMenuWindow()
{

}

void SYMenuWindow::setCurSelectedItem(WindowType type)
{
	QPushButton* button = nullptr;

	switch(type)
	{
	case WT_AdminPersonWindow:
		button = ui.personMgrBtn;
		break;
	case WT_AdminTaskWindow:
		button = ui.trainingMgrBtn;
		break;
	case WT_AdminTrainingCenterWindow:
		button = ui.trainingCenterBtn;
		break;
	case WT_AdminTheoryTestWindow:
		button = ui.theoryMgrBtn;
		break;
	//case WT_AdminKnowledgeSetManageWindow:
	//	button = ui.knowledgeSetManageBtn;
	//	break;
	}

	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = button;
	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(true);
}

void SYMenuWindow::onClickedBtn()
{
	QPushButton* button = static_cast<QPushButton*>(sender());   //ͨ���ź�ȷ���ĸ����������
	if(button == m_curSelectedBtn){   //�����������뵱ǰѡ�а���Ϊͬһ��������ͬ�л�ҳ��
		button->setChecked(true);
		return;
	}

	if(m_curSelectedBtn)
		m_curSelectedBtn->setChecked(false);

	m_curSelectedBtn = button;
	m_curSelectedBtn->setChecked(true);

	WindowType windowType = static_cast<WindowType>(button->property("windowType").toInt());

	emit showNextWindow(windowType);  //������ʾ��Ӧ�����ź�
}

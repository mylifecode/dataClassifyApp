#include "MxTrainingDebugWindow.h"
#include "ITraining.h"
#include <QString>
#include "MxDefine.h"
#include "RbVirtualKeyboard.h"
#include "RbPointToolWidget.h"
#include "RbHotchpotchEditorWidget.h"
#include "RbAdjustShaderParametersWidget.h"
#include "SYTrainWindow.h"
#include <qtooltip.h>
#include "NewTrain\MisNewTraining.h"
#define BTNPROPERTY "toolWindowPtr"


MxTrainingDebugWindow::MxTrainingDebugWindow(QWidget * parent, MisNewTraining * trainInEdit)
:QWidget(parent),
m_pTraining(NULL),
m_pVirtualKeyboard(NULL),
m_pointInputEditWidget(NULL),
m_hotchpotchEditorWidget(NULL),
m_veinConnEditWidget(NULL),
m_veinConnEditorV2Widget(NULL),
m_adjustShaderParamWidget(NULL),
m_nextRow(0),
m_nextCol(0),
m_maxCol(2),
m_numRecord(0),
m_debugMessageShowPrefix(QString::fromLocal8Bit("第%1条：  "))
{
	m_ui.setupUi(this);
	m_pVirtualKeyboard = new RbVirtualKeyboard();
	m_ui.contentLineEdit->installEventFilter(this);

	createSubToolWindow();

	connect(m_ui.sendBtn,SIGNAL(clicked()),SLOT(onSendBtn()));
	connect(m_ui.hideBtn,SIGNAL(clicked()),SLOT(onHideBtn()));
	connect(m_ui.exportMessageBtn,SIGNAL(clicked()),SLOT(onExportMessageBtn()));
	connect(m_pVirtualKeyboard,SIGNAL(contentChanged(const QString&)),m_ui.contentLineEdit,SLOT(setText(const QString &)));
	connect(m_ui.clearListViewBtn,SIGNAL(clicked()),SLOT(onClearListViewBtn()));

	Mx::setWidgetStyle(this,"qss:RbTrainingDebugWindow.qss");

	setTraining(trainInEdit);
}

MxTrainingDebugWindow::~MxTrainingDebugWindow(void)
{
	delete m_pVirtualKeyboard;
}

bool MxTrainingDebugWindow::eventFilter(QObject * object, QEvent * event)
{
	if(object == m_ui.contentLineEdit && event->type() == QEvent::MouseButtonPress)
	{
		m_pVirtualKeyboard->show();
	}
	if (object == m_ui.contentLineEdit && event->type() == QEvent::Enter)
	{
		QToolTip::showText(m_ui.contentLineEdit->pos(),QString::fromLocal8Bit("'savecamera'保存当前相机视点"));
	}

	return __super::eventFilter(object,event);
}

void MxTrainingDebugWindow::onSendBtn()
{
	if(m_pTraining)
	{
		QString text = m_ui.contentLineEdit->text();
		if(text.size())
		{
			m_pTraining->onDebugMessage(m_ui.contentLineEdit->text().toStdString());
			QToolTip::showText(m_ui.contentLineEdit->pos(),QString::fromLocal8Bit("相机视点保存成功"));//相机视点保存失败
		}
	}
}

void MxTrainingDebugWindow::onHideBtn()
{
	hide();
}

void MxTrainingDebugWindow::hideEvent(QHideEvent*)
{
// 	if(m_pVirtualKeyboard)			m_pVirtualKeyboard->hide();
// 	if(m_pointInputEditWidget)		m_pointInputEditWidget->hide();
// 	if(m_hotchpotchEditorWidget)	m_hotchpotchEditorWidget->hide();
// 	if(m_veinConnEditWidget)		m_veinConnEditWidget->hide();
// 	if(m_veinConnEditorV2Widget)	m_veinConnEditorV2Widget->hide();
// 	if(m_adjustShaderParamWidget)	m_adjustShaderParamWidget->hide();
}

void MxTrainingDebugWindow::onClickedSubToolBtn()
{
    EditorCamera::InitGlobalEditorCamera();

	QPushButton * pBtn = static_cast<QPushButton*>(sender());
	QWidget * widget = NULL;

	QVariant variant = pBtn->property(BTNPROPERTY);
	if(variant.isValid())
		widget = (QWidget*)variant.toUInt();

	if(widget)
		widget->setVisible(!widget->isVisible());
}

void MxTrainingDebugWindow::onClickedEditCamera()
{
	EditorCamera::InitGlobalEditorCamera();
}

void MxTrainingDebugWindow::onAddOneDebugInfo(const std::string& debugInfo)
{
	if(m_ui.receiveDebugInfoCheckBox->isChecked())
	{
		m_ui.recordNumberLabel->setText(QString::fromLocal8Bit("共%1条记录").arg(++m_numRecord));
		m_ui.debugInfoView->addItem(m_debugMessageShowPrefix.arg(m_numRecord) + QString::fromLocal8Bit(debugInfo.c_str()));
		m_ui.debugInfoView->setCurrentRow(m_ui.debugInfoView->count() - 1);
	}
}

void MxTrainingDebugWindow::onClearListViewBtn()
{
	m_ui.debugInfoView->clear();
	m_ui.recordNumberLabel->setText(QString::fromLocal8Bit("共0条记录"));
	m_numRecord = 0;
}

void MxTrainingDebugWindow::onExportMessageBtn()
{
	QString content;
	
	if(m_ui.lineCheckBox->isChecked())
	{
		for(int r = 0;r < m_ui.debugInfoView->count();++r)
		{
			content += m_ui.debugInfoView->item(r)->text() + "\r\n";
		}
	}
	else
	{
		int removedSize = m_debugMessageShowPrefix.size();
		for(int r = 1;r < m_ui.debugInfoView->count() + 1;++r)
		{
			int dt = 0;
			if(r < 10)
				dt = -1;
			else if(r >= 100)
			{	
				int temp = r;
				dt = -1;
				while(temp /= 10)
					dt += 1;
			}

			content += m_ui.debugInfoView->item(r - 1)->text().remove(0,removedSize + dt) + "\r\n";
		}
	}

	QFile file("debugMessage.txt");
	if(!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::information(this,"open file fail:","can not open debugMessage.txt");
	}
	else
	{
		file.write(content.toLocal8Bit());
		QMessageBox::information(this,"","export success: debugMessage.txt");
	}
}

/**
	创建自定义的子工具
*/
void MxTrainingDebugWindow::createSubToolWindow()
{
	QPushButton * pBtn = NULL;

	//1 PointTool
 	m_pointInputEditWidget = new RbPointToolWidget(this);
	m_pointInputEditWidget->setVisible(false);
	pBtn = new QPushButton;
 	pBtn->setProperty(BTNPROPERTY,(unsigned int)m_pointInputEditWidget);
	pBtn->setText("PointTool");
	pBtn->setObjectName("subToolBtn");
	m_ui.gridLayout->addWidget(pBtn);
	connect(pBtn,SIGNAL(clicked()),SLOT(onClickedSubToolBtn()));
 
	//2 HotchpotchEditor
  	m_hotchpotchEditorWidget = new RbHotchpotchEditorWidget(this);
 	m_hotchpotchEditorWidget->setVisible(false);
 	pBtn = new QPushButton;
 	pBtn->setProperty(BTNPROPERTY,(unsigned int)m_hotchpotchEditorWidget);
 	pBtn->setText("HotchpotchEditor");
	pBtn->setObjectName("subToolBtn");
 	m_ui.gridLayout->addWidget(pBtn);
 	connect(pBtn,SIGNAL(clicked()),SLOT(onClickedSubToolBtn()));

	//3 VeinConnEditor
//   	m_veinConnEditWidget = new RbVeinConnEditorWidget(this);
//  	m_veinConnEditWidget->setVisible(false);
//  	pBtn = new QPushButton;
//  	pBtn->setProperty(BTNPROPERTY,(unsigned int)m_veinConnEditWidget);
//  	pBtn->setText("VeinConnEditor");
//		pBtn->objectName("subToolBtn");
//  	m_ui.gridLayout->addWidget(pBtn);
//  	connect(pBtn,SIGNAL(clicked()),SLOT(onClickedSubToolBtn()));
  
	//4 VeinConnEditorV2
 // 	m_veinConnEditorV2Widget = new RbVeinConnEditorV2Widget(this);
 //	m_veinConnEditorV2Widget->setVisible(false);
 //	pBtn = new QPushButton;
 //	pBtn->setProperty(BTNPROPERTY,(unsigned int)m_veinConnEditorV2Widget);
 //	pBtn->setText("VeinConnEditorV2");
	//pBtn->setObjectName("subToolBtn");
 //	m_ui.gridLayout->addWidget(pBtn);
 //	connect(pBtn,SIGNAL(clicked()),SLOT(onClickedSubToolBtn()));

	//5   AdjustShaderParameters
  	m_adjustShaderParamWidget = new RbAdjustShaderParametersWidget(this);
 	m_adjustShaderParamWidget->setVisible(false);
 	pBtn = new QPushButton;
 	pBtn->setProperty(BTNPROPERTY,(unsigned int)m_adjustShaderParamWidget);
 	pBtn->setText("AdjustShaderParameters");
	pBtn->setObjectName("subToolBtn");
 	m_ui.gridLayout->addWidget(pBtn);
 	connect(pBtn,SIGNAL(clicked()),SLOT(onClickedSubToolBtn()));

	//test
	pBtn = new QPushButton;
	pBtn->setText("EditCamera");
	pBtn->setObjectName("subToolBtn");
	m_ui.gridLayout->addWidget(pBtn);
	connect(pBtn,SIGNAL(clicked()),SLOT(onClickedEditCamera()));
}

/**
	设置训练
*/
void MxTrainingDebugWindow::setTraining(MisNewTraining * pTraining)
{
	m_pTraining = pTraining;

	if(m_pointInputEditWidget)		m_pointInputEditWidget->onCurrentTrainChanged(pTraining);
	if(m_hotchpotchEditorWidget)	m_hotchpotchEditorWidget->OnCurrentTrainChanged(pTraining);
	if(m_adjustShaderParamWidget)	m_adjustShaderParamWidget->OnCurrentTrainChanged(pTraining);
}

void MxTrainingDebugWindow::hideAllSubWindow()
{
	if(m_pVirtualKeyboard)			m_pVirtualKeyboard->hide();
	if(m_pointInputEditWidget)		m_pointInputEditWidget->hide();
	if(m_hotchpotchEditorWidget)	m_hotchpotchEditorWidget->hide();
	if(m_adjustShaderParamWidget)	m_adjustShaderParamWidget->hide();
}
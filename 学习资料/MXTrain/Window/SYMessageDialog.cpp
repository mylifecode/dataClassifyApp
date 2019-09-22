#include "SYMessageDialog.h"
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QPainter>
#include <QPen>
#include "MxDefine.h"

SYMessageDialog::SYMessageDialog(QWidget *parent, 
	                             const QString title, 
								 int buttonActionMode, 
								 const QString description,
				                 const QString leftButtonText, 
								 const QString rightButtongText) : RbShieldLayer(parent) , m_buttonActionMode(buttonActionMode)

{
	
	ui.setupUi(this);

	hideCloseButton();
	hideOkButton();

	ui.contentLabel->setText(description);
	ui.subContentLabel->setText("");
	
	for (int i = 0; i < 2; ++i)
	{
		QPushButton * pBtn = new QPushButton;
		pBtn->setObjectName(QString("button%1").arg(i + 1));
		pBtn->setProperty("index", i + 1);
		pBtn->setText(i == 0 ? leftButtonText : rightButtongText);
		
		ui.gridLayout->addWidget(pBtn, 0, i);
	
		if (i == 0)
			connect(pBtn, SIGNAL(clicked()), this, SLOT(onPushbutton1()));
		else
			connect(pBtn, SIGNAL(clicked()), this, SLOT(onPushbutton2()));
	}
	ui.gridLayout->setSpacing(80);

	setAttribute(Qt::WA_DeleteOnClose);
	
	Mx::setWidgetStyle(this, "qss:SYMessageDialog.qss");

}

SYMessageDialog::~SYMessageDialog()
{

}


void SYMessageDialog::onPushbutton1()
{
	done(1);
}

void SYMessageDialog::onPushbutton2()
{
	done(0);
}
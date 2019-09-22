#include "SYAddModuleWindow.h"
#include "ui_SYAddModuleWindow.h"
#include<QFileDialog>
#include"MxDefine.h"

SYAddModuleWindow::SYAddModuleWindow(QWidget *parent) :
RbShieldLayer(parent)
   
{
    ui.setupUi(this);

	hideCloseButton();
	hideOkButton();

	Mx::setWidgetStyle(this, QString("qss:SYAddModuleWindow.qss"));
	connect(ui.cancelBtn, &QPushButton::clicked, this, [=]()
	{
		done(RC_Cancel);
	});
	connect(ui.confirmBtn, &QPushButton::clicked, this, [=]()
	{
		QString moduleName = ui.moduleName->text();
		if (moduleName.size() == 0)
		{
			return;
		}

		emit on_confirm_Btn_Clicked(iconPath, moduleName);

		done(RC_Ok);

	});
	connect(ui.addModuleIconBtn, &QPushButton::clicked, this, &SYAddModuleWindow::choosedModueIcon);
}

SYAddModuleWindow::~SYAddModuleWindow()
{
  
}


void SYAddModuleWindow::choosedModueIcon()
{
	QString t_iconPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择模块的图标"), "", QString("Images (*.png *jpg *xpm)"));
	if (t_iconPath.isEmpty())
	{
		iconPath = "";
		return;
	}
	QStringList iconNames = t_iconPath.split('/',QString::SkipEmptyParts);
	QString iconName = iconNames[iconNames.size() - 1];
	QString basePath = "..\\res\\Skin\\SkinConfig\\SYAddModuleWindow\\";
	iconPath = basePath+iconName;
}
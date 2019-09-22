#include "RbAdjustGyroTool.h"
#include "ui_RbAdjustGyroTool.h"
#include <qevent.h>

RbAdjustGyroTool::RbAdjustGyroTool(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RbAdjustGyroTool)
{
    ui->setupUi(this);
	setDragLength(0);
	setLookAtAngle(0);
}

RbAdjustGyroTool::~RbAdjustGyroTool()
{
    delete ui;
}

void RbAdjustGyroTool::initWidgetValue(Ogre::String strData)
{	
	
}

void RbAdjustGyroTool::closeEvent(QCloseEvent *e)
{
	if(e->Close)
	{
		__super::closeEvent(e);
		emit closeExit();
	}
}
void RbAdjustGyroTool::resetWidgetValue()
{

};

Ogre::String RbAdjustGyroTool::getWidgetValueString()
{
	Ogre::String str="aaa";
	return str;
}

void RbAdjustGyroTool::getDragLength(double& dlength)
{
	dlength = ui->dsbdraglength->value();
};
void RbAdjustGyroTool::setDragLength(const double dlength)
{
	ui->dsbdraglength->setValue(dlength);
};

void RbAdjustGyroTool::getLookAtAngle(double& dangle)
{
	dangle = ui->dsblookat->value();
};

void RbAdjustGyroTool::setLookAtAngle(const double dangle)
{
	ui->dsblookat->setValue(dangle);
};
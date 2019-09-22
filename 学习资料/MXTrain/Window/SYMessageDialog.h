#pragma once
#include "ui_SYMessageBox.h"
#include "ScreenEffect.h"
#include <QDialogButtonBox>
#include "RbShieldLayer.h"

class QTimer;
class QGraphicsOpacityEffect;

class SYMessageDialog : public RbShieldLayer
{
	Q_OBJECT
public:
	SYMessageDialog(QWidget *parent, const QString title, int buttonActionMode = 0, const QString description = QString::fromLocal8Bit("是否退出训练"),
		const QString leftButtonText = QString::fromLocal8Bit("是"), const QString rightButtongText = QString::fromLocal8Bit("否"));
	
	~SYMessageDialog(void);

protected:
	
protected slots:
	
    void onPushbutton1();													//左边按钮
	
    void onPushbutton2();													//右边按钮

private:
	
	Ui::SYMessageBox ui;
	
	int m_buttonActionMode;													//0->左按钮为退出训练，右按钮为取消,1->左按钮为退出训练，右按钮为重新开始训练
	
};

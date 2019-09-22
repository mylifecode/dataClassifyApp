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
	SYMessageDialog(QWidget *parent, const QString title, int buttonActionMode = 0, const QString description = QString::fromLocal8Bit("�Ƿ��˳�ѵ��"),
		const QString leftButtonText = QString::fromLocal8Bit("��"), const QString rightButtongText = QString::fromLocal8Bit("��"));
	
	~SYMessageDialog(void);

protected:
	
protected slots:
	
    void onPushbutton1();													//��߰�ť
	
    void onPushbutton2();													//�ұ߰�ť

private:
	
	Ui::SYMessageBox ui;
	
	int m_buttonActionMode;													//0->��ťΪ�˳�ѵ�����Ұ�ťΪȡ��,1->��ťΪ�˳�ѵ�����Ұ�ťΪ���¿�ʼѵ��
	
};

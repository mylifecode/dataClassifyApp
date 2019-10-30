#pragma once

#include <QtWidgets/QWidget>
#include"ui_DataClassifyApp.h"
#include<qvector.h>
#include<qstring.h>

#define CHS(txt) QString::fromLocal8Bit(txt)

class DataClassifyApp : public QWidget
{
	Q_OBJECT

public:
	DataClassifyApp(QWidget *parent = Q_NULLPTR);
	//���û���ƶ���Ĭ���ļ�����·��Ϊ����·�����Զ�����savedir�ļ�����
private slots:
	void on_dataInputClickedBtn();
	void on_dataSaveClickedBtn();
	void on_preImgShowClickedBtn();
	void on_nextImgShowClickedBtn();
	void on_imgClassifyToNum0Btn();
	void on_imgClassifyToNum1Btn();
	void on_imgClassifyToNum2Btn();
	void on_imgLargerShowBtn();


private:
	//�ļ�Ŀ¼Ԥ��������ʽΪ���ƶ�·���£�ȥ������Ŀ¼����ȫ���ļ�����һ��Ŀ¼�У�newData�ļ�����.
	bool imgPathPreprocess();
	void freshImgDisplay();
	
private:
	Ui::DataClassifyAppClass ui;
	QString m_imgPath="";
	QString m_imgSavePath="";
	int curNum=0;
	QString curFileName;
	QVector<QString> m_imgNameVec;
};

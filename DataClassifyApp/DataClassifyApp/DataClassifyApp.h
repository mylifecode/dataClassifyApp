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
	//如果没有制定，默认文件保存路径为数据路径下自动创建savedir文件保存
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
	//文件目录预处理，处理方式为在制定路径下，去掉多余目录，把全部文件放在一个目录中，newData文件夹中.
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

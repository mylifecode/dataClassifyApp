#include "DataClassifyApp.h"
#include<QFileDialog>
#include<qmessagebox.h>
#include<qdebug.h>
#include<string>

DataClassifyApp::DataClassifyApp(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);


	connect(ui.dataInputBtn, &QPushButton::clicked, this, &DataClassifyApp::on_dataInputClickedBtn);
	connect(ui.dataOutputBtn, &QPushButton::clicked, this, &DataClassifyApp::on_dataSaveClickedBtn);
	connect(ui.preBtn, &QPushButton::clicked, this, &DataClassifyApp::on_preImgShowClickedBtn);
	connect(ui.nextBtn, &QPushButton::clicked, this, &DataClassifyApp::on_nextImgShowClickedBtn);
	connect(ui.number_0, &QPushButton::clicked, this, &DataClassifyApp::on_imgClassifyToNum0Btn);
	connect(ui.number_1, &QPushButton::clicked, this, &DataClassifyApp::on_imgClassifyToNum1Btn);
	connect(ui.number_2, &QPushButton::clicked, this, &DataClassifyApp::on_imgClassifyToNum2Btn);


}

//ͼƬԤ����
bool DataClassifyApp::imgPathPreprocess()
{
	return true;
}

void DataClassifyApp::on_dataInputClickedBtn()
{
	QString file_path = QFileDialog::getExistingDirectory(this, CHS("��ѡ����ȷ������·��..."), "./");
	if (file_path.isEmpty())
	{
		QMessageBox::information(NULL, CHS("��ʾ"), CHS("��ѡ����ȷ�������ļ�·��!"), QMessageBox::Yes);
		return;
	}
	else
	{
		m_imgPath = file_path;
		
		QDir dir(m_imgPath);
		QStringList nameFilters;
		nameFilters << "*.jpg" << "*.png";
		QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
		for (auto fileName : files)
		{
			m_imgNameVec.push_back(fileName);
		}
		//�Զ���ʾ��һ��ͼƬ
		if (!m_imgNameVec.isEmpty())
		{
			curNum = 1;
			curFileName = m_imgNameVec[curNum-1];
			//��ʾ��һ��ͼƬ	
			freshImgDisplay();
		}	
	}

}

void DataClassifyApp::freshImgDisplay()
{
	int total = m_imgNameVec.size();
	ui.imgSeq->setText(QString("%1/%2").arg(curNum).arg(total));
	QString curfilePath = m_imgPath + "/" + m_imgNameVec[curNum - 1];
	QFileInfo file(curfilePath);
	if (file.isFile())
	{
		QPalette pal = ui.widget->palette();
		pal.setBrush(QPalette::Background, QBrush(QPixmap(curfilePath)));
		ui.widget->setPalette(pal);
		ui.widget->setAutoFillBackground(true);
	}

}

void DataClassifyApp::on_dataSaveClickedBtn()
{
	QString file_path = QFileDialog::getExistingDirectory(this, CHS("��ѡ����ȷ�����ݱ���·��..."), "./");
	if (file_path.isEmpty())
	{
		QMessageBox::information(NULL, CHS("��ʾ"), CHS("��ѡ����ȷ�������ļ�·��!"), QMessageBox::Yes);
		return;
	}
	else
	{
		m_imgSavePath = file_path;
		//�Զ�����0��1��2����Ŀ¼
		QDir dir(m_imgSavePath);
		if (dir.exists())
		{
			QStringList list;
			list<< "0" << "1" << "2";
			for (auto subdir : list)
			{
				QString newPath = m_imgSavePath + "/" + subdir;
				QDir dir1(newPath);
				if (dir1.exists())
				{
					dir1.rmpath(newPath);
				}
				//������Ŀ¼
				dir1.mkpath(newPath);
			}
		}
	}
}

void DataClassifyApp::on_preImgShowClickedBtn()
{
	if (curNum > 1)
	{
		curNum--;
		freshImgDisplay();
	}
		
}

void DataClassifyApp::on_nextImgShowClickedBtn()
{

	if (curNum <m_imgNameVec.size())
	{
		curNum++;
		freshImgDisplay();
	}
}

void DataClassifyApp::on_imgClassifyToNum0Btn()
{
	
	QString saveFilePath = m_imgSavePath +"/0/"+ m_imgNameVec[curNum-1];
	//ȥ��
	QDir dir(m_imgSavePath + "/0");
	QStringList nameFilters;
	nameFilters << "*.jpg" << "*.png";
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
	bool isExist=false;
	for (auto fileName : files)
	{
		if (fileName == m_imgNameVec[curNum - 1])
		{
			isExist = true;
			break;
		}
	}
	if (!isExist)
	{
		QString readFilePath = m_imgPath + "/" + m_imgNameVec[curNum - 1];
		QImage img;
		if (img.load(readFilePath))
		{

			img.save(saveFilePath);
		}
	}

}

void DataClassifyApp::on_imgClassifyToNum1Btn()
{
	QString saveFilePath = m_imgSavePath + "/1/" + m_imgNameVec[curNum - 1];
	//ȥ��
	QDir dir(m_imgSavePath + "/1");
	QStringList nameFilters;
	nameFilters << "*.jpg" << "*.png";
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
	bool isExist = false;
	for (auto fileName : files)
	{
		if (fileName == m_imgNameVec[curNum - 1])
		{
			isExist = true;
			break;
		}
	}
	if (!isExist)
	{
		QString readFilePath = m_imgPath + "/" + m_imgNameVec[curNum - 1];
		QImage img;
		if (img.load(readFilePath))
		{

			img.save(saveFilePath);
		}
	}
}

void DataClassifyApp::on_imgClassifyToNum2Btn()
{
	QString saveFilePath = m_imgSavePath + "/2/" + m_imgNameVec[curNum - 1];
	//ȥ��
	QDir dir(m_imgSavePath + "/2");
	QStringList nameFilters;
	nameFilters << "*.jpg" << "*.png";
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
	bool isExist = false;
	for (auto fileName : files)
	{
		if (fileName == m_imgNameVec[curNum - 1])
		{
			isExist = true;
			break;
		}
	}
	if (!isExist)
	{
		QString readFilePath = m_imgPath + "/" + m_imgNameVec[curNum - 1];
		QImage img;
		if (img.load(readFilePath))
		{

			img.save(saveFilePath);
		}
	}
}

void DataClassifyApp::on_imgLargerShowBtn()
{

}
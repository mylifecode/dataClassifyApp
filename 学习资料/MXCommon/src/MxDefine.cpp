#include "MxDefine.h"
#include <QApplication>
//#include <QAxObject>
#include "MxGlobalConfig.h"
#include "SYBasicExcel.hpp"
#include <qfile.h>
#include <QDomDocument>

namespace Mx
{
	void setWidgetStyle(QWidget* w, const QString& qssFile)
	{
		static QMap<QString, QString> styleSheetMap;
		QString qss = w->styleSheet();
		QFile file(qssFile);

		auto itr = styleSheetMap.find(qssFile);
		if(itr != styleSheetMap.end())
		{
			if(qss.size())
			{
				qss += itr.value();
				w->ensurePolished();
			}
			else
				qss = itr.value();
			w->setStyleSheet(qss);
		}
		else if(file.open(QFile::ReadOnly))
		{
			QString tempStyleSheet;
			QByteArray bufAll = file.readAll();
			file.close();

			if(!bufAll.isEmpty())
			{
				QTextStream in(&bufAll, QIODevice::ReadOnly);
				//in.setCodec(QTextCodec::codecForName("UTF-8"));

				while(!in.atEnd())
					tempStyleSheet += in.readLine() + "\n";
			}

			tempStyleSheet.replace("$SDIR", MxGlobalConfig::Instance()->GetSkinDir());
			styleSheetMap.insert(qssFile, tempStyleSheet);

			if(qss.size())
			{
				qss += tempStyleSheet;
				w->ensurePolished();
			}
			else
				qss = tempStyleSheet;
			w->setStyleSheet(qss);
		}
	}

	void setGlobleStyle(const QString& qssFile)
	{
		QString qss;
		QFile file(qssFile);

		if(file.open(QFile::ReadOnly))
		{
			QByteArray bufAll = file.readAll();
			file.close();

			if(!bufAll.isEmpty())
			{
				QTextStream in(&bufAll, QIODevice::ReadOnly);

				while(!in.atEnd())
					qss += in.readLine() + "\n";
			}
			qss.replace("$SDIR", MxGlobalConfig::Instance()->GetSkinDir());
			qApp->setStyleSheet(qss);
		}
	}

	// 	bool writeDataToExcelFile_1(const QString& fileName,const QVector<QVector<QVariant>> & datas)
	// 	{
	// 		QAxObject * excelObj = new QAxObject("Excel.Application");
	// 		if(excelObj->isNull())
	// 		{
	// 			Message(CHS("提示"),CHS("未找到Excel相关组件"));
	// 			delete excelObj;
	// 			return false;
	// 		}
	// 		excelObj->setProperty("Visible",false);
	// 		excelObj->setProperty("Caption",fileName);
	// 		excelObj->setProperty("DisplayAlerts", false);
	// 
	// 		QAxObject * workBooks = excelObj->querySubObject("Workbooks");
	// 		if(workBooks == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("获取Excel工作簿失败"));
	// 			return false;
	// 		}
	// 		workBooks->dynamicCall("Add");
	// 
	// 		QAxObject * book = workBooks->querySubObject("Item(const int)", 1);
	// 		if(book == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("添加Excel工作簿失败"));
	// 			return false;
	// 		}
	// 
	// 		QAxObject * sheets = book->querySubObject("Sheets");
	// 		if(sheets == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("获取Excel表单失败"));
	// 			return false;
	// 		}
	// 
	// 		QAxObject * sheet = sheets->querySubObject("Item(int)",1);
	// 		if(sheet == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("获取Excel子表单失败"));
	// 			return false;
	// 		}
	// 
	// 
	// 		for(int r = 0 ;r < datas.size();++r)
	// 		{
	// 			for(int c = 0; c < datas[r].size();++c)
	// 			{
	// 				QAxObject * cell = sheet->querySubObject("Cells(in,int)",r + 1,c + 1);
	// 				if(cell == NULL)
	// 				{
	// 					Message(CHS("提示"),CHS("获取单元格失败：row=%1,col=%2").arg(r+1).arg(c+1));
	// 				}
	// 				else
	// 					cell->dynamicCall("SetValue(QVariant)",datas[r][c]);
	// 			}
	// 		}
	// 
	// 		book->setProperty("DisplayAlerts", 0);
	// 		book->dynamicCall("SaveAs(const QString&,int)",fileName);  //另存为另一个文件 
	// 		book->dynamicCall("Close(bool)", false);  //关闭文件
	// 		workBooks->dynamicCall("Quit()");
	// 		excelObj->dynamicCall("Quit()");
	// 		delete excelObj;
	// 		return true;
	// 	}

	bool writeDataToExcelFile_2(const QString& fileName, const QVector<QVector<QVariant>> & datas)
	{
		bool bRet = false;
		YExcel::BasicExcel excel;
		YExcel::BasicExcelWorksheet *sheet = NULL;

		excel.New(1);
		sheet = excel.GetWorksheet((size_t)0);
		if(sheet)
		{
			int row = datas.size();
			int col = 0;
			for(int r = 0; r < row; ++r)
			{
				col = datas[r].size();
				for(int c = 0; c < col; ++c)
				{
					sheet->Cell(r, c)->SetWString(datas[r][c].toString().toStdWString().c_str());
				}
			}

			excel.SaveAs(fileName.toLocal8Bit().data());
			bRet = true;
		}

		return bRet;
	}

	bool writeDataToExcelFile(const QString& fileName, const QVector<QVector<QVariant>> & datas)
	{
		QString newFileName = fileName;
		newFileName.replace('/', "\\");

		//return writeDataToExcelFile_1(newFileName,datas);
		return writeDataToExcelFile_2(newFileName, datas);
	}

	// 	bool readDataFromExcelFile_1(const QString& fileName,QVector<QVector<QVariant>>& datas,int maxCol)
	// 	{
	// 		QAxObject * excelObj = new QAxObject("Excel.Application");
	// 		if(excelObj->isNull())
	// 		{
	// 			Message(CHS("提示"),CHS("未找到Excel相关组件"));
	// 			delete excelObj;
	// 			return false;
	// 		}
	// 		excelObj->setProperty("Visible",false);
	// 		excelObj->setProperty("Caption",fileName);
	// 		excelObj->setProperty("DisplayAlerts", false);
	// 
	// 		QAxObject * workBooks = excelObj->querySubObject("Workbooks");
	// 		if(workBooks == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("获取Excel工作簿失败"));
	// 			return false;
	// 		}
	// 		QAxObject * book = workBooks->querySubObject("Open(QString)", fileName);
	// 		if(book == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("打开Excel工作簿失败"));
	// 			return false;
	// 		}
	// 		QAxObject * sheets = book->querySubObject("Sheets");
	// 		if(sheets == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("获取Excel表单失败"));
	// 			return false;
	// 		}
	// 		QAxObject * sheet = sheets->querySubObject("Item(int)",1);
	// 		if(sheet == NULL)
	// 		{
	// 			Message(CHS("提示"),CHS("获取Excel子表单失败"));
	// 			return false;
	// 		}
	// 
	// 		QVector<QVariant> record;
	// 		QVariant value;
	// 		bool canStop = false;
	// 
	// 		QString s;
	// 		int row = 1;
	// 		while(!canStop)
	// 		{
	// 			record.clear();
	// 
	// 			int counter = 0;
	// 			for(int c = 0; c < maxCol;++c)
	// 			{
	// 				QAxObject * cell = sheet->querySubObject("Cells(in,int)",row,c + 1);
	// 				if(cell == NULL)
	// 				{
	// 					Message(CHS("提示"),CHS("获取单元格失败：row=%1,col=%2").arg(row).arg(c+1));
	// 					record.push_back(QVariant());
	// 					++counter;
	// 					continue;
	// 				}
	// 				else
	// 					record.push_back(cell->property("Value"));
	// 
	// 				value = cell->property("Value");
	// 				s = value.toString();
	// 				if(value.isNull() || value.isValid() == false)
	// 					++counter;
	// 			}
	// 
	// 			if(counter == maxCol)
	// 				canStop = true;
	// 			else
	// 				datas.push_back(record);
	// 			++row;
	// 		}
	// 
	// 		book->setProperty("DisplayAlerts", 0);
	// 		book->dynamicCall("Close(bool)", false);  //关闭文件
	// 		workBooks->dynamicCall("Quit()");
	// 		excelObj->dynamicCall("Quit()");
	// 		delete excelObj;
	// 
	// 		return true;
	// 	}

	bool readDataFromExcelFile_2(const QString& fileName, QVector<QVector<QVariant>> & datas, int maxCol)
	{
		bool bRet = false;
		YExcel::BasicExcel excel;
		YExcel::BasicExcelWorksheet *sheet = NULL;
		YExcel::BasicExcelCell * cell = NULL;

		excel.Load(fileName.toStdWString().c_str());
		sheet = excel.GetWorksheet((size_t)0);
		if(sheet)
		{
			const int rows = sheet->GetTotalRows();
			const int tc = sheet->GetTotalCols();
			const int cols = std::min(maxCol, tc);

			std::wstring wstr;
			std::string str;
			QVector<QVariant> record;
			for(int r = 0; r < rows; ++r)
			{
				record.clear();
				for(int c = 0; c < cols; ++c)
				{
					cell = sheet->Cell(r, c);
					if(cell)
					{
						switch(cell->Type())
						{
						case YExcel::BasicExcelCell::UNDEFINED:
							record.push_back("");
							break;
						case YExcel::BasicExcelCell::INT:
							record.push_back(cell->GetInteger());
							break;
						case YExcel::BasicExcelCell::DOUBLE:
							record.push_back(cell->GetDouble());
							break;
						case YExcel::BasicExcelCell::STRING:
							str.clear();
							if(cell->GetString())
								str.append(cell->GetString());
							record.push_back(QString::fromStdString(str));
							break;
						case YExcel::BasicExcelCell::WSTRING:
							wstr.clear();
							if(cell->GetWString())
								wstr.append(cell->GetWString());
							record.push_back(QString::fromStdWString(wstr));
							break;
						default:
							record.push_back("");
							break;
						}
					}
					else
						record.push_back("");
				}
				datas.push_back(record);
			}

			int dt = 0;
			if(maxCol > tc)
			{
				int dt = maxCol - tc;
				for(int r = 0; r < rows; ++r)
				{
					for(int c = 0; c < dt; ++c)
					{
						datas[r].push_back("");
					}
				}
			}
			bRet = true;
		}

		return bRet;
	}

	bool readDataFromExcelFile(const QString& fileName, QVector<QVector<QVariant>> & datas, int maxCol)
	{
		if(QFile::exists(fileName) == false)
			return false;
		if(maxCol <= 0)
			return false;

		QString newFileName = fileName;
		newFileName.replace('/', "\\");

		//return readDataFromExcelFile_1(newFileName,datas,maxCol);
		return readDataFromExcelFile_2(newFileName, datas, maxCol);
	}

	std::multimap<UserPermission, QString> addModuleItemFromXML()
	{
		std::multimap<UserPermission, QString> mapLoginModule ;
		const QString& fileName = MxGlobalConfig::Instance()->GetSelectModuleXmlConfigFileName();
		if (!QFile::exists(fileName))
			return mapLoginModule;
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly))
			return mapLoginModule;
		QDomDocument document;
		document.setContent(&file);
		file.close();

		//parse
		QDomElement element = document.documentElement();
		QDomNode node = element.firstChild();
		QString strObjectName;
		UserPermission userPermission;
		
		while (!node.isNull())
		{
			if (node.isElement())
			{
				QDomElement e = node.toElement();
				if (e.attribute("UserPermission") == "UP_SuperManager")
				{
					userPermission = UP_SuperManager;
				}
				else if (e.attribute("UserPermission") == "UP_Teacher")
				{
					userPermission = UP_Teacher;
				}
				else if (e.attribute("UserPermission") == "UP_Student")
				{
					userPermission = UP_Student;
				}
				else if (e.attribute("UserPermission") == "UP_Visitor")
				{
					userPermission = UP_Visitor;
				}
				else if (e.attribute("UserPermission") == "")//代表信息化登录
				{
					userPermission = UP_Error;
				}

				QDomNode dn = e.firstChild();
				while (!dn.isNull())
				{
					e = dn.toElement();
					strObjectName = e.attribute("ObjectName");
					dn = dn.nextSibling();
					mapLoginModule.insert(make_pair(userPermission, strObjectName));
				}
			}
			node = node.nextSibling();
		}
		return mapLoginModule;
	}

}

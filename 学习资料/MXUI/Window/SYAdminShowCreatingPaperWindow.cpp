#include "SYAdminShowCreatingPaperWindow.h"
#include "ui_SYAdminShowCreatingPaperWindow.h"
// #include"SYAdminQuestionsMgrWindow.h"
//#include "SYAdminPaperContentFrameWindow.h"
#include "MxDefine.h"
#include <SYDBMgr.h>
#include"SYStringTable.h"


SYAdminShowCreatingPaperWindow::SYAdminShowCreatingPaperWindow(QVector<QuestionInfo> &t_createPaperInfo, QWidget *parent) :
 m_itemSelectedNumber(0), RbShieldLayer(parent)
{
	ui.setupUi(this);

	m_createPaperInfo = &t_createPaperInfo;
	hideOkButton();
	hideCloseButton();
	setAttribute(Qt::WA_DeleteOnClose);
	Mx::setWidgetStyle(this, QString("qss:SYAdminShowCreatingPaperWindow.qss"));

	Initialize();//导入数据
	RefreshTable();	
}

SYAdminShowCreatingPaperWindow::~SYAdminShowCreatingPaperWindow()
{

}

void  SYAdminShowCreatingPaperWindow:: Initialize()
{
	connect(ui.listWidget, &QListWidget::itemClicked, [=](QListWidgetItem *item)
	{
		ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(ui.listWidget->itemWidget(item));
		bool flag = widget->IsChecked();
		flag = !flag;
		widget->setChecked(flag);
	//	widget->SetBackGroundColor(flag);

		QString chosedQuestionNum = SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum);
		int itemSelectedNumber = GetItemSelectedNumber();
		ui.chosedQuestion->setText(chosedQuestionNum + QString::number(itemSelectedNumber));

	});

	
	connect(ui.all_chosed_checkBox, &QCheckBox::stateChanged, this, [=](int state)
	{
		bool flag = state;

		for (std::size_t i = 0; i < ui.listWidget->count(); i++)
		{
			QListWidgetItem* item = ui.listWidget->item(i);

			ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(ui.listWidget->itemWidget(item));

			if (widget)
			{
				widget->setChecked(flag);
			//	widget->SetBackGroundColor(flag);
			}
		}

		QString chosedQuestionNum = SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum);
		int itemSelectedNumber = GetItemSelectedNumber();
		ui.chosedQuestion->setText(chosedQuestionNum + QString::number(itemSelectedNumber));
	}
	);

	connect(ui.deleteBtn, &QPushButton::clicked, this, &SYAdminShowCreatingPaperWindow::on_delete_Btn_Clicked);
	connect(ui.returnBtn, &QPushButton::clicked, this, &SYAdminShowCreatingPaperWindow::on_return_Btn_Clicked);

	QString chosedQuestionNum = SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum);
	int itemSelectedNumber = GetItemSelectedNumber();
	ui.chosedQuestion->setText(chosedQuestionNum + QString::number(itemSelectedNumber));

}
void SYAdminShowCreatingPaperWindow::RefreshTable()
{
	
	ui.listWidget->clear();
	int index = 0;
	for (auto it = m_createPaperInfo->begin(); it!=m_createPaperInfo->end();it++)
	{	
		QSqlRecord record = (*it).record;
		ListWidgetItemFrame* widget = new ListWidgetItemFrame(ui.listWidget, record, index);
		widget->setQuestionFrameHintSize(1490, 0);
		QListWidgetItem *item = new QListWidgetItem(ui.listWidget);
		int itemHeight = widget->getItemHeight();
		item->setSizeHint(QSize(1490, itemHeight));
		
		ui.listWidget->setItemWidget(item, widget);
		QCheckBox* box = widget->getCheckBox();
		connect(box, &QCheckBox::stateChanged, this, [=](int state)
		{
			bool flag = state;

			widget->setChecked(flag);
		//	widget->SetBackGroundColor(flag);

			QString chosedQuestionNum = SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum);
			int itemSelectedNumber = GetItemSelectedNumber();
			ui.chosedQuestion->setText(chosedQuestionNum + QString::number(itemSelectedNumber));
		});

		index++;			
	}

	ui.listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  //横向滑动隐藏
	ui.listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	ui.listWidget->setSpacing(5);

}

void SYAdminShowCreatingPaperWindow::on_return_Btn_Clicked()
{
	done(RbShieldLayer::RC_Ok);

}

int SYAdminShowCreatingPaperWindow::GetItemSelectedNumber()
{

	int ItemSelectedNumber = 0;
	for (std::size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(ui.listWidget->itemWidget(item));
		if (widget)
		{
			bool flag = widget->IsChecked();
			if (flag)
				ItemSelectedNumber++;
		}

	}
	return ItemSelectedNumber;
}
void SYAdminShowCreatingPaperWindow::on_delete_Btn_Clicked()
{	
	std::list<int> checkboxSelected;
	for (std::size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* item = ui.listWidget->item(i);
		ListWidgetItemFrame* widget = dynamic_cast<ListWidgetItemFrame*>(ui.listWidget->itemWidget(item));
		if (widget)
		{
			bool flag = widget->IsChecked();
			if (flag)
				checkboxSelected.push_back(i);
		}
	}	
	auto it = m_createPaperInfo->begin();
	QVector<QVector<QuestionInfo>::iterator> delVec;
	for (auto offSet = checkboxSelected.begin(); offSet != checkboxSelected.end(); offSet++)
	{
		auto addres = it + *offSet;
		delVec.push_back(addres);
	}

	while (!delVec.isEmpty())
	{
		auto it = delVec[delVec.size()-1];
		delVec.pop_back();
		m_createPaperInfo->erase(it);

	}
	checkboxSelected.clear();
	//更新显示
	RefreshTable();

	QString chosedQuestionNum = SYStringTable::GetString(SYStringTable::STR_ChosedQuestionNum);
	int itemSelectedNumber = GetItemSelectedNumber();
	ui.chosedQuestion->setText(chosedQuestionNum + QString::number(itemSelectedNumber));


}


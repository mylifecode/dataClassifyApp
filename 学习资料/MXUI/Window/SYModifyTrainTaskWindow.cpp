#include "SYModifyTrainTaskWindow.h"
#include <QPushButton>
#include  <QSound>
#include <QDesktopWidget>
#include <QMouseEvent>
#include "MxDefine.h"
#include "MxGlobalConfig.h"
#include "SYDBMgr.h"
#include "SYUserInfo.h"
#include "SYStringTable.h"
#include <QMessageBox>
#include "SYMessageBox.h"
SYModifyTrainTaskWindow::SYModifyTrainTaskWindow(QWidget *parent)
:RbShieldLayer(parent)
{
	ui.setupUi(this);
	hideCloseButton();
	hideOkButton();

	setAttribute(Qt::WA_DeleteOnClose);
	Mx::setWidgetStyle(this,"qss:SYModifyTrainTaskWindow.qss");

	
	//connect(ui.list_other, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeOtherListItem(QListWidgetItem*)));
	//connect(ui.list_this,  SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeThisListItem(QListWidgetItem*)));
	connect(ui.trainlist, &QTreeWidget::itemClicked, this, &SYModifyTrainTaskWindow::onTreeWidgetItemClicked);

	m_TrainChoosed = -1;

	PullAllTrains();

	std::vector<SubTypeTrains> subTypeTrains;

	FilterTrainsInType(subTypeTrains, SYStringTable::GetString(SYStringTable::STR_TRAINCATEGORYVIRTUALSKIILL));

	RefreshFilteredTrainList(subTypeTrains);

	ui.edit_numday->setValidator(new QIntValidator(1, 120, this));

	ui.lineEdit_score->setValidator(new QIntValidator(1, 100, this));
	ui.lineEdit_score->setText("60");

	ui.line_Edit_Times->setValidator(new QIntValidator(1, 20, this));
	ui.line_Edit_Times->setText("1");
}

SYModifyTrainTaskWindow::~SYModifyTrainTaskWindow()
{

}

void SYModifyTrainTaskWindow::onTreeWidgetItemClicked(QTreeWidgetItem* item, int column)
{
	bool ok;
	m_TrainChoosed = item->data(0, Qt::UserRole).toInt(&ok);
	if (ok){
		
		TrainCanBeAssignInstance & unit = m_AllTrains[m_TrainChoosed];

		//refresh
		ui.lb_seltrainame->setText(unit.m_ShowName);
	}
	else{

		bool expanded = (!item->isExpanded());

		item->setExpanded(expanded);

		QPixmap pixmap;

		if (expanded)
		{
			pixmap.load("icons:/Shared/arrow_close.png");
		}
		else
		{
			pixmap.load("icons:/Shared/arrow_open.png");
		}
		item->setIcon(1, QIcon(pixmap));
	}
}

void SYModifyTrainTaskWindow::on_bt_add_clicked()
{
	if (m_TrainChoosed >= 0)
	{
		int selectedIndex = -1;
		for (int c = 0; c < (int)m_TrainSelected.size(); c++)
		{
			if (m_TrainSelected[c].m_MyIndex == m_TrainChoosed)//already added
			{
				selectedIndex = c;
				break;
			}
		}

		if (selectedIndex < 0)
		{
			TrainCanBeAssignInstance & trainunit = m_AllTrains[m_TrainChoosed];
			m_TrainSelected.push_back(trainunit);
			selectedIndex = m_TrainSelected.size() - 1;
		}

		m_TrainSelected[selectedIndex].m_PassNeedScore = ui.lineEdit_score->text().toInt();
		m_TrainSelected[selectedIndex].m_NeedPassTimes = ui.line_Edit_Times->text().toInt();

		RefreshSelectedTrainList();
		ui.edit_numtrains->setText(QString::number(m_TrainSelected.size()));
	}
	
}
void SYModifyTrainTaskWindow::PullAllTrains()
{
	std::map<QString, TrainCanBeAssign>::iterator itor = SYTaskTrainDataMgr::GetInstance().m_TransCanBeAssign.begin();
		
	while (itor != SYTaskTrainDataMgr::GetInstance().m_TransCanBeAssign.end())
	{
		TrainCanBeAssignInstance person(itor->second , m_AllTrains.size());

		m_AllTrains.push_back(person);

		itor++;
	}
}

void SYModifyTrainTaskWindow::FilterTrainsInType(std::vector<SubTypeTrains> & result, const QString & trainType)
{
	for (int c = 0; c < (int)m_AllTrains.size(); c++)
	{
		TrainCanBeAssignInstance & trainunit = m_AllTrains[c];
		
		 if (trainunit.m_MainCategory == trainType)
		 {
			 bool exist = false;

			 for (int t = 0; t < (int)result.size(); t++)
			 {
				 if (result[t].m_SubType == trainunit.m_SubCategory)
				 {
					 exist = true;
					 result[t].m_Trains.push_back(trainunit);
					 break;
				 }
			 }

			 if (!exist)
			 {
				 result.push_back(SubTypeTrains(trainunit));
			 }
		 }
	}
}

void SYModifyTrainTaskWindow::RefreshFilteredTrainList(const std::vector<SubTypeTrains> & subTypeTrains)
{
	ui.trainlist->clear();
	ui.trainlist->setIconSize(QSize(33, 33));
	ui.trainlist->setColumnCount(2);
	ui.trainlist->setColumnWidth(0, 250);
	ui.trainlist->setColumnWidth(1, 30);

	QPixmap pixmap;
	pixmap.load("icons:/Shared/arrow_open.png");

	QTreeWidgetItem* firstItem = nullptr;

	for (int c = 0; c < (int)subTypeTrains.size(); c++)
	{
	    QTreeWidgetItem * Item = new QTreeWidgetItem(ui.trainlist, QStringList());
		
		Item->setText(0, QString("   ") + subTypeTrains[c].m_SubType);
		
		Item->setIcon(1, QIcon(pixmap));
		
		for (int t = 0; t < (int)subTypeTrains[c].m_Trains.size(); t++)
		{
			const TrainCanBeAssignInstance & trainUnit = subTypeTrains[c].m_Trains[t];

			QTreeWidgetItem * subItem = new QTreeWidgetItem();

			subItem->setText(0, QString("      ") + trainUnit.m_ShowName);
			subItem->setData(0, Qt::UserRole, trainUnit.m_MyIndex);
			
			Item->addChild(subItem);

			if (firstItem == nullptr)
				firstItem = subItem;
		}
	}

	if (firstItem && firstItem->parent())
	{
		ui.trainlist->expandItem(firstItem->parent());
		firstItem->setSelected(true);
		onTreeWidgetItemClicked(firstItem, 0);

		QPixmap pixmap;
        pixmap.load("icons:/Shared/arrow_close.png");
		firstItem->parent()->setIcon(1, QIcon(pixmap));
	}
}


void SYModifyTrainTaskWindow::RefreshSelectedTrainList()
{
	ui.table_selectedtrain->clear();
	
	ui.table_selectedtrain->setRowCount(m_TrainSelected.size());
	
	ui.table_selectedtrain->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	for (int c = (int)m_TrainSelected.size() - 1 ; c >= 0; c--)
	{
		TrainCanBeAssignInstance & unit = m_TrainSelected[c];

		QTableWidgetItem * col0 = new QTableWidgetItem(unit.m_ShowName);
		col0->setTextAlignment(Qt::AlignCenter);

		QTableWidgetItem * col1 = new QTableWidgetItem(QString::number(unit.m_NeedPassTimes));
		col1->setTextAlignment(Qt::AlignCenter);

		QTableWidgetItem * col2 = new QTableWidgetItem(QString::number(unit.m_PassNeedScore));
		col2->setTextAlignment(Qt::AlignCenter);
		
		ui.table_selectedtrain->setItem(c, 0, col0);
		
		ui.table_selectedtrain->setItem(c, 1, col1);
		
		ui.table_selectedtrain->setItem(c, 2, col2);
	}
}
void SYModifyTrainTaskWindow::on_bt_conform_clicked()
{
	//submit train
	int AssignorID = SYUserInfo::Instance()->GetUserId();
	int ReceiverID = -1;
	bool Istemplte = true;

	QString AssignorName = SYUserInfo::Instance()->GetRealName();
	QString RecevierName("");

	QString taskName = ui.edit_taskname->text();
	QString taskTag  = ui.edit_tasktag->text();
	if (taskTag == "")
		taskTag = QString("-");

	if (taskName == "")
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("请输入任务名称"), 1);
		messageBox->showFullScreen();
		messageBox->exec();
		return;
	}

	if (ui.edit_numday->text() == "")
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("请输入任务时长"), 1);
		messageBox->showFullScreen();
		messageBox->exec();
		return;
	}

	if (m_TrainSelected.size() == 0)
	{
		SYMessageBox * messageBox = new SYMessageBox(this, CHS(""), CHS("您还没有选择任务"), 1);
		messageBox->showFullScreen();
		messageBox->exec();
		return;
	}

	int numday = ui.edit_numday->text().toInt();

	if (SYUserInfo::Instance()->GetUserPermission() <= UP_Student)
	{
		ReceiverID   = AssignorID;
		RecevierName = AssignorName;
		Istemplte = false;
	}
	int taskID = SYDBMgr::Instance()->AddOneTaskEntry(taskName, AssignorID, ReceiverID, AssignorName, RecevierName, m_TrainSelected.size(), Istemplte);

	for (int c = 0; c < (int)m_TrainSelected.size(); c++)
	{
		SYDBMgr::Instance()->AddOneTrainEntryInTask(taskID, m_TrainSelected[c].m_ShowName, m_TrainSelected[c].m_SheetCode, m_TrainSelected[c].m_PassNeedScore, m_TrainSelected[c].m_NeedPassTimes);
	}
	done(2);
}

void SYModifyTrainTaskWindow::on_bt_cancel_clicked()
{
	done(1);
}
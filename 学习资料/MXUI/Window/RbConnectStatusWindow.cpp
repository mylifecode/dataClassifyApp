#include "RbConnectStatusWindow.h"
#include "MxDefine.h"

RbConnectStatusWindow::RbConnectStatusWindow(QWidget* parent)
	:QWidget(parent,Qt::Window | Qt::FramelessWindowHint),
	m_nextShowIndex(0),
	m_nCol(5)
{
	m_ui.setupUi(this);

	setAttribute(Qt::WA_TranslucentBackground);

	m_ui.topFrame->setVisible(false);
	m_ui.tableWidget->setColumnCount(m_nCol);

	Mx::setWidgetStyle(this, "qss:RbConnectStatusWindow.qss");
}

RbConnectStatusWindow::~RbConnectStatusWindow()
{

}

void RbConnectStatusWindow::on_statusBtn_clicked()
{
	m_ui.topFrame->setVisible(!m_ui.topFrame->isVisible());
}

void RbConnectStatusWindow::AddUser(uint32_t id)
{
	auto itr = m_userInfoMap.find(id);
	if(itr != m_userInfoMap.end())
		return;

	UserInfo userInfo = {id,m_nextShowIndex,CHS("用户%1").arg(m_nextShowIndex+1)};
	//1 insert to map
	m_userInfoMap.insert(id, userInfo);
	//2 update tablewidget
	int row, col;
	row = m_nextShowIndex / m_nCol;
	col = m_nextShowIndex % m_nCol;

	if(row == m_ui.tableWidget->rowCount())
		m_ui.tableWidget->insertRow(row);

	QTableWidgetItem* item = m_ui.tableWidget->item(row, col);
	if(item)
		item->setText(userInfo.info);
	else
	{
		item = new QTableWidgetItem(userInfo.info);
		m_ui.tableWidget->setItem(row, col,item);
	}
	item->setData(Qt::UserRole, id);

	++m_nextShowIndex;

	m_ui.statusBtn->setText(CHS("已连接：%1人").arg(m_nextShowIndex));
}

void RbConnectStatusWindow::RemoveUser(uint32_t id)
{
	auto itr = m_userInfoMap.find(id);
	if(itr == m_userInfoMap.end())
		return;

	UserInfo& userInfo = itr.value();
	int row1 = userInfo.showIndex / m_nCol;
	int col1 = userInfo.showIndex % m_nCol;

	QTableWidgetItem* item = m_ui.tableWidget->takeItem(row1, col1);
	if(item == nullptr)
		throw QString("item is null.");
	delete item;

 	--m_nextShowIndex;
// 	if(m_nextShowIndex != 0)
	if(m_nextShowIndex != userInfo.showIndex)
	{
		int row2 = m_nextShowIndex / m_nCol;
		int col2 = m_nextShowIndex % m_nCol;
		item = m_ui.tableWidget->takeItem(row2, col2);
		if(item == nullptr)
			throw QString("item2 is null.");

		m_ui.tableWidget->setItem(row1, col1, item);

		//update show index of the UserInfo struct
		uint32_t id = item->data(Qt::UserRole).toUInt();
		auto newItr = m_userInfoMap.find(id);
		if(newItr == m_userInfoMap.end())
			throw QString("can not found the user info");

		newItr->showIndex = userInfo.showIndex;

		if(col2 == 0)
			m_ui.tableWidget->removeRow(row2);
	}
	else if(col1 == 0)
		m_ui.tableWidget->removeRow(row1);

	//remove user info
	m_userInfoMap.erase(itr);

	m_ui.statusBtn->setText(CHS("已连接：%1人").arg(m_nextShowIndex));
}

void RbConnectStatusWindow::UpdateUserInfo(uint32_t id,const QString& userName,const QString& realName)
{
	auto itr = m_userInfoMap.find(id);
	if(itr != m_userInfoMap.end())
	{
		//1 update map
		if(userName.size() == 0)
		{
			if(realName.size() == 0)
				return;
			itr->info = realName;
		}
		else
		{
			itr->info = userName;
			if(realName.size())
			{
				itr->info.append("(");
				itr->info.append(realName);
				itr->info.append(")");
			}
		}

		//2 update talewidget
		int row = itr->showIndex / m_nCol;
		int col = itr->showIndex % m_nCol;
		QTableWidgetItem* item = m_ui.tableWidget->item(row, col);
		
		if(item == nullptr)
			throw QString("item is null.");
		item->setText(itr->info);
	}
}

void RbConnectStatusWindow::on_closeBtn_clicked()
{
	m_ui.topFrame->hide();
}
#include "SYUserListWindow.h"
#include <MXDefine.h>
#include <SYDBMgr.h>
#include <QCheckBox>

SYUserListWindow::SYUserListWindow(QWidget* parent)
	:RbShieldLayer(parent),
	m_groupId(0)
{
	ui.setupUi(this);

	hideOkButton();
	hideCloseButton();

	setTableWidgetAttribute();

	connect(ui.lineEdit, &QLineEdit::returnPressed, this, &SYUserListWindow::on_searchBtn_clicked);

	Mx::setWidgetStyle(this, "qss:/SYUserListWindow.qss");
}

SYUserListWindow::~SYUserListWindow()
{

}

void SYUserListWindow::SetGroupFilter(int groupId)
{
	m_groupId = groupId;
}

void SYUserListWindow::on_cancelBtn_clicked()
{
	m_userNameMap.clear();
	done(RbShieldLayer::RC_Cancel);
}

void SYUserListWindow::on_okBtn_clicked()
{
	done(RbShieldLayer::RC_Ok);
}

void SYUserListWindow::setTableWidgetAttribute()
{
	ui.tableWidget->setColumnCount(3);

	QStringList labels;
	labels << CHS("Ñ¡Ôñ") << CHS("ÐÕÃû") << CHS("°à¼¶");
	ui.tableWidget->setHorizontalHeaderLabels(labels);

	ui.tableWidget->setFrameShape(QFrame::NoFrame);
	ui.tableWidget->setShowGrid(false);
	ui.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->setAlternatingRowColors(true);
	ui.tableWidget->setFocusPolicy(Qt::NoFocus);

	QHeaderView* headerView = ui.tableWidget->horizontalHeader();
	headerView->setStretchLastSection(true);
	headerView->setHighlightSections(false);
	headerView->setObjectName("horizontalHeader");
	//headerView->setSectionsMovable(false);
	headerView->setEnabled(false);

	//set section width
	ui.tableWidget->setColumnWidth(0,180);
	ui.tableWidget->setColumnWidth(1,160);
	ui.tableWidget->setColumnWidth(2,100);

	headerView = ui.tableWidget->verticalHeader();
	headerView->hide();
}

void SYUserListWindow::setTableWidgetContents(const std::vector<int>& recordIndexs)
{
	auto SetItemAttribute = [](QTableWidgetItem* item, const QString& text){
		item->setText(text);
		item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		item->setTextColor(QColor(0xb7, 0xc5, 0xd8));

		QFont font = item->font();
		font.setPixelSize(16);
		item->setFont(font);

		Qt::ItemFlags flags = item->flags();
		flags = flags & (~Qt::ItemIsEditable);
		item->setFlags(flags);
	};

	std::size_t nRecord = recordIndexs.size();// m_userDataRecords.size();

	ui.tableWidget->setRowCount(nRecord);
	for(std::size_t i = 0; i < nRecord; ++i){
		int recordIndex = recordIndexs[i];
		const auto& record = m_userDataRecords[recordIndex];
		int userId = record.value("id").toInt();
		QString name = record.value("realName").toString();

		if(name.size() == 0)
			name = record.value("userName").toString();

		//col 0
		QWidget* widget = new QWidget();
		QVBoxLayout* vLayout = new QVBoxLayout();
		QCheckBox* checkBox = new QCheckBox();
		checkBox->setObjectName("checkBox");
		checkBox->setProperty("id", userId);
		connect(checkBox, &QCheckBox::stateChanged, this, &SYUserListWindow::onCheckBoxStateChanged);
		checkBox->setProperty("name", name);

		vLayout->setMargin(0);
		vLayout->setSpacing(0);
		vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
		vLayout->addWidget(checkBox);
		vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));

		QHBoxLayout* hLayout = new QHBoxLayout();
		hLayout->setMargin(0);
		hLayout->setSpacing(0);
		hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
		hLayout->addLayout(vLayout);
		hLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

		widget->setLayout(hLayout);
		widget->setContentsMargins(0, 0, 0, 0);

		ui.tableWidget->setCellWidget(i, 0, widget);

		//col 1
		QTableWidgetItem* item = new QTableWidgetItem;
		//item->setText(name);
		SetItemAttribute(item, name);
		ui.tableWidget->setItem(i, 1, item);

		//col 2
		item = new QTableWidgetItem;
		//item->setText("-");
		SetItemAttribute(item, "-");
		ui.tableWidget->setItem(i, 2, item);

		ui.tableWidget->setRowHeight(i, 70);
	}
}

void SYUserListWindow::showEvent(QShowEvent* event)
{
	m_userNameMap.clear();
	ui.tableWidget->clearContents();
	ui.lineEdit->clear();

	if(m_groupId > 0){
		m_userDataRecords.clear();
		SYDBMgr::Instance()->QueryUserInGroup(m_groupId, m_userDataRecords);
	}
	else
		m_userDataRecords = SYDBMgr::Instance()->GetAllUserData(true);
	m_recordIndexs.clear();
	for(std::size_t i = 0; i < m_userDataRecords.size(); ++i)
		m_recordIndexs.push_back(i);

	setTableWidgetContents(m_recordIndexs);
}

void SYUserListWindow::on_searchBtn_clicked()
{
	QString searchText = ui.lineEdit->text();

	if(searchText.size() == 0){
		return;
	}

	m_recordIndexs.clear();
	for(std::size_t i = 0; i < m_userDataRecords.size(); ++i){
		const auto& record = m_userDataRecords[i];
		QString name = record.value("realName").toString();

		if(name.size() == 0)
			name = record.value("userName").toString();

		if(name.indexOf(searchText) != -1)
			m_recordIndexs.push_back(i);
	}

	setTableWidgetContents(m_recordIndexs);
}

void SYUserListWindow::onCheckBoxStateChanged(int state)
{
	QCheckBox* checkBox = static_cast<QCheckBox*>(sender());
	int id = checkBox->property("id").toInt();
	
	if(state == Qt::Unchecked){
		m_userNameMap.remove(id);
	}
	else{
		QString name = checkBox->property("name").toString();
		m_userNameMap.insert(id, name);
	}
}

void SYUserListWindow::on_lineEdit_textChanged(const QString& text)
{
	if(text.size() == 0){
		m_recordIndexs.clear();
		for(std::size_t i = 0; i < m_userDataRecords.size(); ++i)
			m_recordIndexs.push_back(i);

		setTableWidgetContents(m_recordIndexs);
	}
}
#include "SYAdminScreenshotDisplayer.h"
#include "SYScreenshotDisplayer.h"

SYAdminScreenshotDisplayer::SYAdminScreenshotDisplayer(QWidget* parent)
	:RbShieldLayer(parent),
	m_displayer(new SYScreenshotDisplayer)
{
	m_displayer->setFixedWidth(1260);
	m_displayer->setFixedHeight(1006);
	connect(m_displayer, &SYScreenshotDisplayer::back, this, &SYAdminScreenshotDisplayer::onBack);
		
	QHBoxLayout* hLayout = new QHBoxLayout;
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
	hLayout->addWidget(m_displayer);
	hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

	QVBoxLayout* vLayout = new QVBoxLayout;
	vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));
	vLayout->addLayout(hLayout);
	vLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding));

	setLayout(vLayout);

	hideCloseButton();
	hideOkButton();
}

SYAdminScreenshotDisplayer::~SYAdminScreenshotDisplayer()
{

}

void SYAdminScreenshotDisplayer::SetScoreId(int scoreId)
{
	if(m_displayer)
		m_displayer->setScoreId(scoreId);
}

void SYAdminScreenshotDisplayer::onBack()
{
	onOkButton();
}
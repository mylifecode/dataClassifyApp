#include "SYScreenshotDisplayer.h"
#include "MxDefine.h"
#include "SYDBMgr.h"
#include <QPainter>
#include "SYMainWindow.h"

SYScreenshotDisplayer::SYScreenshotDisplayer(QWidget* parent)
	:QWidget(parent),
	m_scoreId(0),
	m_fullPictureViewWindow(nullptr)
{
	ui.setupUi(this);

	//add default button
	QPoint offset(62, 76);
	int spacing = 30;
	int buttonWidth = 260;
	int buttonHeight = 206;

	for(int row = 0; row < 3; ++row){
		for(int col = 0; col < 4; ++col){
			QPushButton* button = new QPushButton(ui.centerFrame);
			int x = offset.x() + col * (buttonWidth + spacing);
			int y = offset.y() + row * (buttonHeight + spacing);
			button->move(x, y);
			button->resize(buttonWidth, buttonHeight);
			button->setObjectName("pictureBtn");
			button->setProperty("index", row * 4 + col);
			button->installEventFilter(this);

			connect(button, &QPushButton::clicked, this, &SYScreenshotDisplayer::on_pictureBtn_clicked);

			m_buttons.push_back(button);
		}
	}

	Mx::setWidgetStyle(this, "qss:SYScreenshotDisplayer.qss");
}

SYScreenshotDisplayer::~SYScreenshotDisplayer()
{

}

void SYScreenshotDisplayer::setScoreId(int scoreId)
{
	if(scoreId == m_scoreId)
		return;

	m_scoreId = scoreId;

	QVector<QSqlRecord> records;
	QString pixmapName;
	SYDBMgr::Instance()->QueryScreenshots(m_scoreId, records);

	const int nRecord = records.size();
	m_pixmaps.clear();
	m_pixmaps.resize(nRecord);
	for(int i = 0; i < nRecord; ++i){
		m_pixmaps[i].loadFromData(records[i].value("data").toByteArray());
		// 		pixmapName = records[i].value("name").toString();
		// 		int index = pixmapName.lastIndexOf(".");
		// 		if(index != -1)
		// 		{
		// 			pixmapName = pixmapName.left(index);
		// 		}
	}
}

void SYScreenshotDisplayer::on_backBtn_clicked()
{
	emit back();
}

void SYScreenshotDisplayer::on_pictureBtn_clicked()
{
	QPushButton* button = static_cast<QPushButton*>(sender());
	int buttonIndex = button->property("index").toInt();
	int nPixmap = m_pixmaps.size();

	if(buttonIndex < 0 || buttonIndex >= nPixmap)
		return;

	if(m_fullPictureViewWindow == nullptr)
		m_fullPictureViewWindow = new SYFullPictureViewWindow(SYMainWindow::GetInstance());

	m_fullPictureViewWindow->setPixmaps(m_pixmaps, buttonIndex);
	m_fullPictureViewWindow->showFullScreen();
}

bool SYScreenshotDisplayer::eventFilter(QObject* obj, QEvent* event)
{
	if(event->type() == QEvent::Paint){
		for(auto* button : m_buttons){
			if(button == obj){
				int buttonIndex = button->property("index").toInt();
				int nPixmap = m_pixmaps.size();

				if(buttonIndex < 0 || buttonIndex >= nPixmap){
					break;
				}

				//draw screen pixmap
				QPixmap& pixmap = m_pixmaps[buttonIndex];
				QPainter painter(button);
				painter.drawPixmap(0, 0, button->width(), button->height(),
								   pixmap,
								   0, 0, pixmap.width(), pixmap.height());

				return true;
			}
		}
	}

	return QWidget::eventFilter(obj, event);
}

//////////////////////////////////////////////////////////////////////////
SYFullPictureViewWindow::SYFullPictureViewWindow(QWidget* parent)
	:RbShieldLayer(parent),
	m_curPixmapIndex(-1),
	m_closeBtn(nullptr)
{
	ui.setupUi(this);

	ui.pictureFrame->installEventFilter(this);
	ui.backgroundFrame->installEventFilter(this);

	//RbshieldLayer
	hideCloseButton();
	hideOkButton();

	//
	m_closeBtn = new QPushButton(this);
	m_closeBtn->setObjectName("closeBtn");
	connect(m_closeBtn, &QPushButton::clicked, this, &SYFullPictureViewWindow::on_closeBtn_clicked);

	Mx::setWidgetStyle(this, "qss:SYFullPictureViewWindow.qss");
}

SYFullPictureViewWindow::~SYFullPictureViewWindow()
{
	m_pixmaps.clear();
}

void SYFullPictureViewWindow::setPixmaps(const QVector<QPixmap>& pixmaps,int curIndex)
{
	m_pixmaps = pixmaps;
	if(m_pixmaps.size() > 0){
		if(curIndex >= 0 && curIndex < m_pixmaps.size())
			m_curPixmapIndex = curIndex;
		else
			m_curPixmapIndex = 0;
	}
	
	else
		m_curPixmapIndex = -1;

	update();
}

bool SYFullPictureViewWindow::eventFilter(QObject* obj, QEvent* event)
{
	if(event->type() == QEvent::Paint && obj == ui.pictureFrame){
		if(m_curPixmapIndex >= 0 && m_curPixmapIndex < m_pixmaps.size()){
			QPixmap& pixmap = m_pixmaps[m_curPixmapIndex];
			QPainter painter(ui.pictureFrame);

			painter.drawPixmap(0, 0, ui.pictureFrame->width(), ui.pictureFrame->height(),
							   pixmap,
							   0, 0, pixmap.width(), pixmap.height());
			return true;
		}
	}

	if(event->type() == QEvent::Resize && obj == ui.backgroundFrame){
		QPoint newPos = ui.backgroundFrame->pos();
		newPos.setX(newPos.x() + ui.backgroundFrame->width() - m_closeBtn->width() / 2);
		newPos.setY(newPos.y() - m_closeBtn->height() / 2);

		m_closeBtn->move(newPos);
	}

	return RbShieldLayer::eventFilter(obj, event);
}

void SYFullPictureViewWindow::on_prePicBtn_clicked()
{
	if(m_curPixmapIndex > 0){
		--m_curPixmapIndex;
		update();
	}
}

void SYFullPictureViewWindow::on_nextPicBtn_clicked()
{
	if(m_curPixmapIndex >= 0 && m_curPixmapIndex < m_pixmaps.size() - 1){
		++m_curPixmapIndex;
		update();
	}
}

void SYFullPictureViewWindow::on_closeBtn_clicked()
{
	onOkButton();
}
#include "MxScreenshotDisplayer.h"
#include "SYDBMgr.h"
#include "MxDefine.h"
#include <QHBoxLayout>
#include <QSqlRecord>
#include <QPainter>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QScrollBar>
#include <QFontMetrics>

MxScreenshotDisplayer::MxScreenshotDisplayer(QWidget* parent)
	:RbShieldLayer(parent),
	m_scoreId(-1),
	m_selectedPixmapIndex(-1),
	m_hasMoved(false),
	m_oldValue(0)
{
	ui.setupUi(this);
	ui.content->installEventFilter(this);
	ui.scrollArea->installEventFilter(this);

	Mx::setWidgetStyle(this, "qss:RbScreenshotDisplayer.qss");
}

MxScreenshotDisplayer::~MxScreenshotDisplayer()
{

}

void MxScreenshotDisplayer::setScoreId(int scoreId)
{
	m_scoreId = scoreId;
	updateContent();
}

void MxScreenshotDisplayer::updateContent()
{
	if(m_scoreId < 0)
	{
		clearContent();
	}
	else
	{
		QHBoxLayout* layout = static_cast<QHBoxLayout*>(ui.scrollAreaWidgetContents->layout());

		if(layout == nullptr)
		{
			layout = new QHBoxLayout;
			layout->setContentsMargins(6, 3, 6, 3);
			layout->setSpacing(20);
			ui.scrollAreaWidgetContents->setLayout(layout);
		}

		//get pixmaps
		QPushButton* btn;
		QVector<QSqlRecord> records;
		QString pixmapName;
		SYDBMgr::Instance()->QueryScreenshots(m_scoreId, records);
		
		const int nRecord = records.size();
		m_pixmaps.resize(nRecord);
		for(int i = 0; i < nRecord;++i)
		{
			m_pixmaps[i].loadFromData(records[i].value("data").toByteArray());
			pixmapName = records[i].value("name").toString();
			int index = pixmapName.lastIndexOf(".");
			if(index != -1)
			{
				pixmapName = pixmapName.left(index);
			}

			btn = new QPushButton;
			btn->setObjectName("smallPixtureBtn");
			btn->setProperty("pixmapIndex", i);
			btn->setProperty("pixmapName", pixmapName);
			btn->installEventFilter(this);
			connect(btn, SIGNAL(clicked()), this, SLOT(onClickedSmallPicture()));

			//add it to layout
			layout->addWidget(btn);

			m_allButtons.insert(btn);

			//test
			//bool bRet = m_pixmaps[i].save(QString("d:\\1\\") + records[i].value("name").toString());
			//bRet = bRet;
		}
	}

	if(m_pixmaps.size() > 0)
		m_selectedPixmapIndex = 0;
	else
		m_selectedPixmapIndex = -1;

	ui.content->update();
}

void MxScreenshotDisplayer::clearContent()
{
	QLayout* layout = ui.scrollAreaWidgetContents->layout();
	
	if(layout)
	{
		for(auto button : m_allButtons)
			layout->removeWidget(button);
	}

	ui.nameLable->setText(QString::fromLocal8Bit("ÑµÁ·½ØÍ¼"));

	for(auto button : m_allButtons)
		delete button;
	m_allButtons.clear();

	m_pixmaps.clear();

	m_selectedPixmapIndex = -1;

	ui.scrollArea->horizontalScrollBar()->setValue(0);
}

void MxScreenshotDisplayer::onClickedSmallPicture()
{
	int oldSelectedIndex = m_selectedPixmapIndex;
	QPushButton* btn = static_cast<QPushButton*>(sender());
	m_selectedPixmapIndex = btn->property("pixmapIndex").toInt();

	if(oldSelectedIndex != -1)
	{
		for(auto button : m_allButtons)
		{
			int index = button->property("pixmapIndex").toInt();
			if(index == oldSelectedIndex || index == m_selectedPixmapIndex)
			{
				button->update();
			}
		}
	}

	//ui.nameLable->setText(btn->property("pixmapName").toString());
	ui.content->update();
}

bool MxScreenshotDisplayer::eventFilter(QObject* obj,QEvent* event)
{
	if(event->type() == QEvent::Paint)
	{
		if(obj == ui.content)
		{
			QPainter painter(ui.content);

			if(m_selectedPixmapIndex == -1)
			{
				//painter.fillRect(0, 0, ui.content->width(), ui.content->height(), QBrush(QColor(0, 0, 0)));
				QFontMetrics metrics(painter.fontMetrics());
				QString text = CHS("ÎÞ½ØÍ¼");
				QRect rect = metrics.boundingRect(ui.content->rect(),Qt::AlignCenter,text);
				painter.drawText(rect, Qt::AlignCenter, text);
			}
			else
			{
				QPixmap& pixmap = m_pixmaps[m_selectedPixmapIndex];
				painter.drawPixmap(ui.content->rect(),
								   pixmap);
			}

			return true;
		}
		else
		{
			QPushButton* button = static_cast<QPushButton*>(obj);
			auto itr = m_allButtons.find(button);

			if(itr != m_allButtons.end())
			{
				int pixmapIndex = button->property("pixmapIndex").toInt();
				if(pixmapIndex >= 0 && pixmapIndex < m_pixmaps.size())
				{
					QPainter painter(button);
					if(pixmapIndex == m_selectedPixmapIndex)
					{
						QRect rect = button->rect();
						painter.fillRect(rect, QColor(0, 255, 0));

						int borderWidth = 2;
						rect.setLeft(borderWidth);
						rect.setTop(borderWidth);
						rect.setRight(rect.width() - borderWidth);
						rect.setBottom(rect.height() - borderWidth);
						painter.drawPixmap(rect, m_pixmaps[pixmapIndex]);
					}
					else
					{
						painter.drawPixmap(button->rect(), m_pixmaps[pixmapIndex]);
					}

					return true;
				}
			}
		}
	}
	else if(event->type() == QEvent::MouseButtonPress)
	{
		const int maxValue = ui.scrollArea->width();
		const int nButton = m_allButtons.size();

		if(nButton > 0)
		{
			int spacing = 0;
			QLayout* layout = ui.scrollAreaWidgetContents->layout();

			if(layout)
			{
				int left, right, top, bottom;
				layout->getContentsMargins(&left, &right, &top, &bottom);
				spacing = left + right + (nButton - 1) * layout->spacing() + nButton * (*m_allButtons.begin())->width();
			}

			if(spacing > maxValue)
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				m_pressedPoint = mouseEvent->screenPos().toPoint();
				

				QScrollBar* bar = ui.scrollArea->horizontalScrollBar();
				m_oldValue = bar->value();
				bar->setMaximum(maxValue);
			}
		}

		m_hasMoved = false;
	}
	else if(event->type() == QEvent::MouseMove)
	{
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		QPoint curPoint = mouseEvent->screenPos().toPoint();
		float dx = curPoint.x() - m_pressedPoint.x();

		QScrollBar* bar = ui.scrollArea->horizontalScrollBar();
		bar->setValue(m_oldValue - dx);

		m_hasMoved = true;
	}
	else if(event->type() == QEvent::MouseButtonRelease)
	{
		if(m_hasMoved)
			return true;
	}

	return __super::eventFilter(obj, event);
}
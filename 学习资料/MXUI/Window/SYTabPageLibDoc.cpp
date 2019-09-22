#include "SYTabPageLibDoc.h"
#include "MxDefine.h"
#include <QHBoxLayout>

SYTabPageLibDoc::SYTabPageLibDoc(QWidget * parent)
	: QWidget(parent),
	m_textBrowser(nullptr)
{
	ui.setupUi(this);

	Mx::setWidgetStyle(this, "qss:SYTabPageLibDoc.qss");
}

SYTabPageLibDoc::~SYTabPageLibDoc(void)
{
	
}

// void SYTabPageLibDoc::setContents(const QVector<QString>& contents)
// {
// 	//create new controls
// 	QPoint offset(26, 44);
// 	QSize browserSize(380, 370);
// 	int spacingX = 30;
// 	int spacingY = 32;
// 	int numOfContent = contents.size();
// 
// 	if(numOfContent > 6)
// 		numOfContent = 6;
// 
// 	if(m_textBrowsers.size() == 0){
// 		for(int i = 0; i < 6; ++i){
// 			auto browser = new QTextBrowser(this);
// 			browser->setObjectName("browser");
// 			browser->resize(browserSize);
// 			m_textBrowsers.push_back(browser);
// 		}
// 	}
// 
// 	for(auto textBrowser : m_textBrowsers){
// 		textBrowser->hide();
// 	}
// 
// 	bool finish = false;
// 	for(int row = 0; row < 2; ++row){
// 		for(int col = 0; col < 3; ++col){
// 			int index = row * 3 + col;
// 			if(index >= numOfContent){
// 				finish = true;
// 				break;
// 			}
// 
// 			const QString& content = contents.at(index);
// 			QTextBrowser* browser = m_textBrowsers[index];
// 			//browser->setText(content);
// 			//browser->setHtml(content);
// 			browser->setSource(content);
// 			browser->move(col * (browserSize.width() + spacingX) + offset.x(), row * (browserSize.height() + spacingY) + offset.y());
// 			browser->setVisible(true);
// 
// 			//set search paths
// 			QString fileName;
// 			QString curPath;
// 			QStringList paths;
// 			int index1 = -1;
// 
// 			index1 = content.lastIndexOf("/");
// 			if(index1 == -1)
// 				index1 = content.lastIndexOf("\\");
// 
// 			paths << content.left(index1);
// 			browser->setSearchPaths(paths);
// 		}
// 
// 		if(finish)
// 			break;
// 	}
// }

void SYTabPageLibDoc::setContent(const QString& content)
{
	if(m_textBrowser == nullptr){
		m_textBrowser = new QTextBrowser();
		m_textBrowser->setObjectName("browser");

		QHBoxLayout* hLayout = new QHBoxLayout;
		hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
		hLayout->addWidget(m_textBrowser);
		hLayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));

		setLayout(hLayout);
	}
	
	m_textBrowser->setSource(content);

	//set search paths
	QString fileName;
	QString curPath;
	QStringList paths;
	int index1 = -1;

	index1 = content.lastIndexOf("/");
	if(index1 == -1)
		index1 = content.lastIndexOf("\\");

	paths << content.left(index1);
	m_textBrowser->setSearchPaths(paths);
}
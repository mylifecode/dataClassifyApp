#pragma once
#include "ui_MxDemonstrationWindow.h"
#include <QPixmap>
#include <QTimer>
#include "MxDefine.h"

class MxNetworkVideoDecoder;


/**  
	主要实现了学生端观看老师端的操作功能
*/
class MxDemonstrationWindow : public QWidget
{
	Q_OBJECT
public:
	MxDemonstrationWindow(QWidget* parent = nullptr);
	~MxDemonstrationWindow();

signals:
	void ExitCurrentWindow(WindowType type);

protected:
	bool eventFilter(QObject* obj, QEvent* event);
	void showEvent(QShowEvent* event);
	void hideEvent(QHideEvent* event);

	void closeEvent(QCloseEvent* event);

private slots:
	void onClickedExitBtn();
	void onDecodeFinish(const QPixmap& frame);
	void onDemonstrationBegin();
	void onDemonstrationStop();
	void onSendWatchCommand();

private:
	Ui::DemonstrationWindow m_ui;

	MxNetworkVideoDecoder* m_decoder;
	QPixmap m_pixmap;
	bool m_hasDemonstration;
	QTimer m_timer;
};

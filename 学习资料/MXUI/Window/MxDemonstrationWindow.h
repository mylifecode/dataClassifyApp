#pragma once
#include "ui_MxDemonstrationWindow.h"
#include <QPixmap>
#include <QTimer>
#include "MxDefine.h"

class MxNetworkVideoDecoder;


/**  
	��Ҫʵ����ѧ���˹ۿ���ʦ�˵Ĳ�������
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

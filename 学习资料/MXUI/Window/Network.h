#ifndef NETWORK_H
#define NETWORK_H

#include <QWidget>
#include "ui_Network.h"
#include "Windows.h"
#include "Wlanapi.h"
#include "QTimer"
#include "QPainter"
struct NetInfo{
	QString netName;
	QString SSID;
	int SignalQuality;
	bool isConn;
	bool sock;
};
class Network : public QWidget
{
	Q_OBJECT
public:
	Network(QWidget *parent = 0);
	~Network();
	void init();
	void updateList();
	void addLine();
	void paintEvent(QPaintEvent *);
public slots:
	void onCloseWIFI();
	void onOpenWIFI();
	void onLain();
	void getNework();
	void onClose();
private:
	Ui::Network ui;
	QMap<QString,PWLAN_AVAILABLE_NETWORK> LANList;
	QMap<QString,int> LANListS;
	QList<NetInfo> netList;
	QList<QObject*> objList;
	QList<QLayoutItem*> LayoutList;
	PWLAN_INTERFACE_INFO_LIST    pIfList ;
	PWLAN_INTERFACE_INFO         pIfInfo ;

	PWLAN_AVAILABLE_NETWORK_LIST pBssList ;
	PWLAN_AVAILABLE_NETWORK      pBssEntry ;

	QVBoxLayout *m_layout;
	void *hCilent;
	QTimer *timer;
	QString m_strBackPixmap_log;
};

#endif // NETWORK_H

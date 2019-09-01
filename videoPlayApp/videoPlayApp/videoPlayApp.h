#pragma once
#include <QtWidgets/QWidget>
//#include "ui_videoPlayApp.h"
#include<qmediaplayer.h>
#include<qmediaplaylist.h>
#include<qpushbutton.h>
#include<qvideowidget.h>
#include<qlabel.h>

class videoPlayApp : public QWidget
{
	Q_OBJECT
public:
	//����״̬
	enum PlayState
	{
		PLAY,
		PAUSE,
		STOP
	};
public:
	videoPlayApp(QWidget *parent = Q_NULLPTR);
	void initialize();
private slots:
	//���ļ�
	bool openFileSlots();
	//����
	void playSlots();
	//�˳�
	bool exitSlots();
	//�����¸��ļ�
	void nextSlots();
	//���ò�����Ϣ
	void initVideoPlayInfo();
private:
	//��ǰ�ļ�·��
	QString filePath;
	//��ǰ����״̬
	PlayState playState;
	//����������
	QMediaPlayer* player;
	//���������б�
	QMediaPlaylist* playerlist;
	//��Ƶ���Ŵ���
	QVideoWidget* videoWidget;
	//��¼��Ƶ���ƺ�·��
	QMap<QString,QString> videoNameAndPath;
	//��¼��Ƶ·��
	QVector<QString> videoPathVector;
	//��ǰ��Ƶ����
	QLabel* videoName;
	//Ui::videoPlayAppClass ui;

};

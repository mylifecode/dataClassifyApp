#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include "ui_Register.h"
#include "QPainter"
struct typeInfo{
	int id;
	QString typeName;
	QString adviseName;
	QString enName;
};
struct classroom{
	int id;
	QString roomName;
	QString floor;
	QString code;
};
struct FInfo{
	int id;
	QList<classroom> room;
};
struct floorInfo{
	int id;
	QString floorName;
	int floor_top;
	int floor_bottom;
	QList<FInfo> fInfo;
};
struct schoolInfo{
	int id;
	QString schoolName;
	QList<floorInfo> floor;
};

struct RegInfo{
	QList<typeInfo> type;
	QList<schoolInfo> school;

};
struct PutRegInfo{
	int eq_type_id;
	QString name;
	QString type;
	int eq_resouce;
	int lab_id;
	int admin_id;
	QString begin_time;
	QString end_time;
	QByteArray week;
};
class Register : public QWidget
{
	Q_OBJECT
signals:
	void RegisterData(PutRegInfo*);
public:
	Register(QWidget *parent = 0);
	~Register();
	void init(RegInfo *regInfo);
	void paintEvent(QPaintEvent *);
public slots:
	void onXiaoqu(int);
	void onDalou(int);
	void onOK();
	void onLoucheng(int);
	void onJiaoshi(int);
	void onType(int);
private:
	Ui::Register ui;
	RegInfo *pRegInfo;
	const schoolInfo *pSchoolInfo;
	const floorInfo *pFloorInfo;
	const FInfo *pFInfo;
	const classroom *pRoom;
	const typeInfo *pTypeInfo;
	QString m_strBackPixmap_log;
};

#endif // REGISTER_H

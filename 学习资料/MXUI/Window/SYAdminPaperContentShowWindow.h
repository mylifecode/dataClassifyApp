#pragma once
#include <QWidget>
#include<qsqlrecord.h>
#include<qvector.h>


class SYAdminPaperContentShowWindow : public QWidget
{
    Q_OBJECT

public:
   
	SYAdminPaperContentShowWindow(QVector<QSqlRecord> records, QWidget *parent = 0);
	void SetWidgetContent();
    ~SYAdminPaperContentShowWindow();

signals:void sendPaperInfo(int id, bool flag);

private:
	QVector<QSqlRecord> result;
};



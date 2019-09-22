#pragma once
#include "ui_SYEntityTrainReportFormWindow.h"
#include "RbShieldLayer.h"
#include <QDateTime>
#include <QSqlRecord>
#include <QVector>

class SYEntityTrainReportFormWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	SYEntityTrainReportFormWindow(QWidget* parent = nullptr);

	~SYEntityTrainReportFormWindow();

	void SetTableWidgetItemAttribute(QTableWidgetItem* item, const QString& text, const QColor& color);

	void SetTrainInfo(const QSqlRecord& scoreRecord);

	void UpdateTable();

private:
	void InitTable();

protected:
	void showEvent(QShowEvent* event);

private slots:
	void on_backBtn_clicked();

private:
	Ui::SYEntityTrainReportFormWindow ui;

	QSqlRecord m_record;
	QString m_trainCode;
	QString m_trainName;
	QDateTime m_trainTime;

	std::vector<std::pair <QString, QString>> m_Details;
	int m_nScreenshot;
};


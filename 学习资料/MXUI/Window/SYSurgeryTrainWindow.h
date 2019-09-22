#pragma once
#include "ui_SYSurgeryTrainWindow.h"
#include "ui_SYSurgeryTrainCase.h"
#include <QTimer>
#include <QToolButton>

enum WindowType;


class SYSurgeryTrainModule : public QWidget
{
	Q_OBJECT
public:
	struct CaseInfo{
		/// 用于进入训练
		QString trainEnName;
		QString caseName;
		QString caseFileName;
	};

	struct TrainInfo{
		/// 用于显示的训练名
		QString trainName;
		std::vector<CaseInfo> caseInfos;
		std::vector<QPushButton*> caseButtons;
	};

public:
	SYSurgeryTrainModule(QWidget *parent = nullptr);

	~SYSurgeryTrainModule(void);

	void LoadCaseFile(const QString & htmlfile);

	Ui::SYSurgeryTrainCase & GetUI(){ return ui; }

	void SetModuleName(const QString& moduleName);
	const QString& GetModuleName() const { return m_moduleName; }

	void AddTrainInfo(const QString& trainInfo);

	void AddCaseInfo(const QString& trainName,const QString& trainEnName,const QString& caseName,const QString& caseFileName);
	
	void SetTrainLayout();

	void SetId(int id) { m_id = id; }

private slots:
	void onClickedFlodBtn();

	void onCaseBtnClicked();

	void onStartButtonClicked();

protected:

	void showEvent(QShowEvent* event);
	
private:
	Ui::SYSurgeryTrainCase  ui;

	int m_id;

	QString m_moduleName;

	std::vector<TrainInfo> m_trainInfos;
	std::vector<QPushButton*> m_trainButtons;
	std::vector<QPushButton*> m_flodButtons;

	QPushButton* m_preFlodBtn;
	QPushButton* m_preTrainBtn;
	QPushButton* m_preCaseBtn;
};



class SYSurgeryTrainWindow : public QWidget
{
	Q_OBJECT
public:
	SYSurgeryTrainWindow(QWidget *parent = nullptr);
	
	~SYSurgeryTrainWindow(void);

signals:
	void showNextWindow(WindowType type);

public slots:

    void onStartButtonClicked();


private:
	SYSurgeryTrainModule* GetSurgeryTrainModule(const QString& moduleName);

protected:
	void showEvent(QShowEvent* event);

	void mousePressEvent(QMouseEvent* event);

	void LaunchRealTrainModule();
private:

	std::vector<SYSurgeryTrainModule*> m_moduleWindows;

	Ui::SYSurgeryTrainWindow ui;

};

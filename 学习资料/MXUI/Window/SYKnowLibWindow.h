#pragma once
#include "ui_SYKnowLibWindow.h"
#include <QVector>
#include <QPushButton>
#include <MxDefine.h>
#include <QTreeWidgetItem>

class SYTabPageToolIntro;
class SYTabPageLibDoc;
class SYTabPageLibVideo;

class SYKnowLibWindow : public QWidget
{
	Q_OBJECT
public:
	SYKnowLibWindow(QWidget *parent = nullptr);

	~SYKnowLibWindow(void);

signals:
	void showNextWindow(WindowType type);

public slots:
	void onTabBarClicked(int index);

	void onToolButtonClicked();

	void onSurgeryDocumentCatButtonClicked();

	void onSurgeryDocumentItemClicked(QTreeWidgetItem* item,int col);

	void onSurgeryVideoCatButtonClicked();

	void onRightButtonClicked();

protected:
	void showEvent(QShowEvent* event);

private:
	Ui::SYKnowLibWindow ui;

	struct ToolInfo{
		QPushButton* button;
		QString toolName;
		QString videoFileName;
		QString description;
	};
	QVector<ToolInfo> m_toolInfos;

	struct SurgeryDocumentInfo{
		//QPushButton* button;
		QString documentCategoryName;
		QVector<QString> documentFiles;
	};
	QVector<SurgeryDocumentInfo> m_surgeryDocumentInfos;

	struct SurgeryVideoInfo{
		QPushButton* button;
		QString videoCategoryName;
		QVector<QString> videoFiles;
	};
	QVector<SurgeryVideoInfo> m_surgeryVideoInfos;

	QPushButton* m_preToolButton;
	QPushButton* m_preDocumentCatButton;
	QPushButton* m_preVideoCatButton;

	QPushButton* m_preButton;

	QTreeWidget* m_treeWidget;
	SYTabPageToolIntro* m_pageTool;
	SYTabPageLibDoc* m_pageDoc;
	SYTabPageLibVideo* m_pageVideo;
};
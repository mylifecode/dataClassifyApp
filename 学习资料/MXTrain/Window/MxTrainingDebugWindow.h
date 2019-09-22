#pragma once
#include <QWidget>
#include "ui_RbTrainingDebugWindow.h"
#include "EditorTool/EditCamera.h"

class MisNewTraining;
class RbVirtualKeyboard;

class RbPointToolWidget;
class RbVeinConnEditorWidget;
class RbHotchpotchEditorWidget;
class RbVeinConnEditorV2Widget;
class RbAdjustShaderParametersWidget;

class MxTrainingDebugWindow : public QWidget
{
	Q_OBJECT
public:
	MxTrainingDebugWindow(QWidget* parent,  MisNewTraining * trainInEdit);
	~MxTrainingDebugWindow(void);

	void setTraining(MisNewTraining * pTraining);

public:
	void keyPressEvent(QKeyEvent *e){__super::keyPressEvent(e);}
	void keyReleaseEvent(QKeyEvent*e){__super::keyPressEvent(e);}
	void hideAllSubWindow();

protected:
	bool eventFilter(QObject *, QEvent *);
	void hideEvent(QHideEvent *);

private slots:
	void onSendBtn();
	void onHideBtn();
	void onClearListViewBtn();
	/** ����������Ϣ */
	void onExportMessageBtn();

	/**
		���˵�����Ϣ��ӵ�������Ϣ��ʾ�б���
	*/
	void onAddOneDebugInfo(const std::string& debugInfo);

	void onClickedSubToolBtn();
	
	void onClickedEditCamera();

private:
	/**
		�˺������𴴽��ӹ��ߴ��ڣ����Ҫ������Զ���Ĺ����ര�ڣ����ڴ˺�����д��Ӧ���룡����
	*/
	void createSubToolWindow();


private:
	Ui::TrainingDebugWindow m_ui;
	MisNewTraining * m_pTraining;
	RbVirtualKeyboard * m_pVirtualKeyboard;

	RbPointToolWidget * m_pointInputEditWidget;
	RbHotchpotchEditorWidget * m_hotchpotchEditorWidget;
	RbVeinConnEditorWidget *m_veinConnEditWidget;
	RbVeinConnEditorV2Widget *m_veinConnEditorV2Widget;
	RbAdjustShaderParametersWidget *m_adjustShaderParamWidget;

	int m_nextRow;
	int m_nextCol;
	const int m_maxCol;

	/// ��Ч�ĵ�����Ϣ��¼��
	int m_numRecord;

	/// ������Ϣ��ʾǰ׺
	const QString m_debugMessageShowPrefix;


};

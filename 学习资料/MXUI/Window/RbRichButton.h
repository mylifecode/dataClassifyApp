#pragma once
#include <qwidget.h>
#include <QPushButton.h>
#include <qlabel.h>
#include <QEvent.h>
#include <qstring.h>


class RbRichButton : public QWidget
{
	Q_OBJECT;
public:
	enum CLICKCONTROLTYPE
	{
		CLICK_NONE,
		CLICK_PARENT,
		CLICK_CHILD
	};
	
	enum ControlPosition
	{
		CP_Center,
		CP_RightTop,
		CP_RightBottom
	};

	RbRichButton(QWidget * parent = NULL,QString strCenterBtnName = "CenterBtn",QString id = "",bool bShowRightBottomBtn = true,bool bShowRightTopLabel = true,bool bShowBottomLabel = true);
	~RbRichButton(void);

	void showRightBottomBtn();
	void hideRightBottomBtn();

	inline void showRightTopLabel(){ if(m_pRightTopLabel) m_pRightTopLabel->show();}
	inline void hideRightTopLabel() { if(m_pRightTopLabel) m_pRightTopLabel->hide();}

	inline void setRightTopLabelText(const QString& strText) { if(m_pRightTopLabel) m_pRightTopLabel->setText(strText);}
	inline QString getRightTopLabelText() { return m_pRightTopLabel->text();}
	//inline void setBottomLabelText(const QString& strText) { if(m_pBottomLabel) m_pBottomLabel->setText(strText);}
	void setBottomLabelText(QString strText) ;

	inline QString getBottomLabelText() { return m_pBottomLabel->text();}

	inline void setCenterBtnObjectName(const QString& strObjName) {if(m_pCenterBtn) m_pCenterBtn->setObjectName(strObjName);}
	inline void setRightBottomBtnObjectName(const QString& strObjName) {if(m_pRightBottomBtn) m_pRightBottomBtn->setObjectName(strObjName);}

	inline void setRightBottomBtnText(const QString& strText) {if(m_pRightBottomBtn) m_pRightBottomBtn->setText(strText);}

	void setRightBottomBtnChecked(bool b) {m_pRightBottomBtn->setChecked(b);}

	void resize(int width,int height);
	void setCenterBtnSize(int width,int height);
	void setRightBottomBtnSize(int width,int height);
	void setRightTopLabelSize(int width,int height);

	CLICKCONTROLTYPE getClickType();

	void setId(QString& strId){m_strId = strId;}
	QString getId(){return m_strId;}

	void setRightBottomBtnPosition(ControlPosition cp);
	void setCenterBtnStyleSheet(const QString& strStyleSheet);
	
	void setText(const QString& text) {m_pBottomLabel->setText(text);}
	QString text(){return m_pBottomLabel->text();}
signals:
	void clickedBtn();
	void pressBtn();
	void releaseBtn();

public slots:
	void onClickedRightBottmBtn();
	void onClickedCenterBtn();
	void onPressCenterBtn();

protected:
	bool event(QEvent * e);
	bool eventFilter(QObject * watched,QEvent * e);

private:
	QString m_strId;
	CLICKCONTROLTYPE m_clickType;

	QLabel * m_pBottomLabel;
	QLabel * m_pRightTopLabel;
	QPushButton * m_pCenterBtn;
	QPushButton * m_pRightBottomBtn;

	ControlPosition m_rightBottomBtnPos;
};

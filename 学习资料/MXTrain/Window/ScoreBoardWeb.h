#ifndef SCOREBOARDWEB_H
#define SCOREBOARDWEB_H

#include "ui_ScoreBoardWeb.h"
#include <QNetworkReply>
#include <QSslError>
#include "MyLabel.h"
#include <QPoint>
#include <QDialog>


#define USE_IE 0
#if !USE_IE
#include <QWebEngineView>
#endif

class ScoreBoardWeb : public QDialog
{
	Q_OBJECT

public:
	ScoreBoardWeb(QWidget * parent);
	~ScoreBoardWeb();

	void SetUrl(QString strUrl);
	void SetWaitingText(QString strText);
	void LoadWebUrl(QString strUrl);

	static ScoreBoardWeb* GetActiveInstance(void);
	static void SetInstanceURL(QString strUrl);

	static ScoreBoardWeb* s_ScoreboardWeb;
	static QMutex* s_pMutexClose;

protected:
	virtual void showEvent(QShowEvent * event);
	virtual void hideEvent(QHideEvent * event);

public slots:
	void FinishLoading(bool bFinished);
	void SetProgress(int nProgress);
	void onClickedShutdownBtn();
	void onClickedAboutBtn();
	void onClickedClose();
#if USE_IE
	void NavigateComplete(IDispatch*, QVariant&);
#endif

private:
	Ui::ScoreBoardWeb ui;
	QString m_strUrl;
	QString m_strPreUrl;
	QPoint m_startPos;
	MyLabel *m_pLoadingLabel;
#if !USE_IE
	QWebEngineView *m_pWebView;
#endif
	int m_nProgress;
};

#endif // ScoreBoardWeb_H

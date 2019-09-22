#pragma once
#include <RbshieldLayer.h>

class SYScreenshotDisplayer;

class SYAdminScreenshotDisplayer : public RbShieldLayer
{
	Q_OBJECT
public:
	SYAdminScreenshotDisplayer(QWidget* parent = nullptr);

	~SYAdminScreenshotDisplayer();

	void SetScoreId(int scoreId);

private slots:
	void onBack();

private:
	SYScreenshotDisplayer* m_displayer;
};


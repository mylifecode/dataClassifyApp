#pragma  once
#include"ui_SYStRandomChooseQuestion.h"
#include"RbShieldLayer.h"
#include <QDialog>


namespace Ui {
class SYStRandomChooseQuestion;
}

class SYStRandomChooseQuestion : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYStRandomChooseQuestion(QWidget *parent = 0);
    ~SYStRandomChooseQuestion();
	void Initialize();


signals:
	void setPaperInfo(int num,int minutes);
private:
    Ui::SYStRandomChooseQuestion ui;
};



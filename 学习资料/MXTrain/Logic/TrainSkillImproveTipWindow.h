#ifndef TRAINSKILLIMPROVETIPWINDOW_H
#define TRAINSKILLIMPROVETIPWINDOW_H

#include <QDialog>
#include<QPair>
#include<QVector>
#include"RbShieldLayer.h"

namespace Ui {
class TrainSkillImproveTipWindow;
}

class TrainSkillImproveTipWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit TrainSkillImproveTipWindow(QWidget * parent = 0);
    ~TrainSkillImproveTipWindow();
	void setBtnVisble(QVector<QPair<QString, QString>> &trainBtnInfos);
	void setTrainLevelState(QVector < QPair<QString, bool> >trainLevelInfos);

public slots:
	void timeOut();
private:
    Ui::TrainSkillImproveTipWindow *ui;


};

#endif // TRAINSKILLIMPROVETIPWINDOW_H

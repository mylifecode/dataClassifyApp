#ifndef RbAdjustGyroTool_H
#define RbAdjustGyroTool_H

#include <QMainWindow>
#include "Ogre.h"

namespace Ui {
class RbAdjustGyroTool;
}

class RbAdjustGyroTool : public QMainWindow
{
    Q_OBJECT

public:
    explicit RbAdjustGyroTool(QWidget *parent = 0);
    ~RbAdjustGyroTool();
	
public:
	void initWidgetValue(Ogre::String strData);
	void resetWidgetValue();
	Ogre::String getWidgetValueString();

private:
	void getDragLength(double& dlength);
	void setDragLength(const double dlength);

	void getLookAtAngle(double& dangle);
	void setLookAtAngle(const double dangle);

protected:
		void closeEvent(QCloseEvent * e);

public :
signals:
	void closeExit();
private:
    Ui::RbAdjustGyroTool *ui;
};

#endif // RbAdjustGyroTool_H

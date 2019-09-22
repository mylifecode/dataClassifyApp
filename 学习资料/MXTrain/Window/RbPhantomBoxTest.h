#ifndef RBPHANTOMBOXTEST_H
#define RBPHANTOMBOXTEST_H

#include "ui_phantomBoxTest.h"
#include "Ogre.h"
#include <QWidget>
#include <QDialog>

class RbPhantomBoxTest : public QDialog
{

    Q_OBJECT

public:

    RbPhantomBoxTest(QWidget *parent = NULL);

    ~RbPhantomBoxTest();
	
	void initWidgetValue();
	void initWidgetValue(Ogre::String strData);

	void resetWidgetValue();
	Ogre::String getWidgetValueString();
protected:

    bool event( QEvent *e );
	void closeEvent(QCloseEvent * e);

private:
	// other group
	void setOtherGroupPos(double x, double y , double z);
	void getOtherGroupPos(Ogre::Vector3& pos);
	// box group
	void setBoxGroupPos(double x, double y , double z);
	void setBoxGroupRot(double x, double y , double z);
	void setBoxGroupSize(double x, double y , double z);
	
	void getBoxGroupPos(Ogre::Vector3& pos);
	void getBoxGroupRot(Ogre::Vector3& rot);
	void getBoxGroupSize(Ogre::Vector3& size);
	// instrument group
	void setInstrumentGroupPos(double x, double y , double z);
	void getInstrumentGroupPos(Ogre::Vector3& pos);

	void saveWidgetValueToFile(Ogre::String filename, Ogre::String strData);
protected:

    QPoint oldPos;

    bool press;

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

public slots:
	void on_mButtonReset_clicked();
	void on_mButtonSave_clicked();

public :
	signals:
	void closeExit();

private:
	Ui::phantomBoxTest ui;
};
#endif
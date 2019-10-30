/********************************************************************************
** Form generated from reading UI file 'DataClassifyApp.ui'
**
** Created by: Qt User Interface Compiler version 5.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DATACLASSIFYAPP_H
#define UI_DATACLASSIFYAPP_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DataClassifyAppClass
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_5;
    QLabel *modeLabel;
    QPushButton *mode1;
    QPushButton *mode2;
    QSpacerItem *horizontalSpacer_4;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *dataInputBtn;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *dataOutputBtn;
    QSpacerItem *horizontalSpacer;
    QHBoxLayout *horizontalLayout_8;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_10;
    QLabel *imgLabel;
    QSpacerItem *horizontalSpacer_8;
    QLabel *imgSeq;
    QSpacerItem *horizontalSpacer_9;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *preBtn;
    QWidget *widget;
    QPushButton *nextBtn;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_6;
    QHBoxLayout *horizontalLayout;
    QLabel *classifyLabel;
    QPushButton *number_0;
    QPushButton *number_1;
    QPushButton *number_2;
    QSpacerItem *horizontalSpacer_7;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *largerBtn;
    QPushButton *smallerBtn;
    QSpacerItem *horizontalSpacer_5;

    void setupUi(QWidget *DataClassifyAppClass)
    {
        if (DataClassifyAppClass->objectName().isEmpty())
            DataClassifyAppClass->setObjectName(QStringLiteral("DataClassifyAppClass"));
        DataClassifyAppClass->resize(983, 730);
        verticalLayout = new QVBoxLayout(DataClassifyAppClass);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(-1, -1, -1, 30);
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(6);
        horizontalLayout_5->setObjectName(QStringLiteral("horizontalLayout_5"));
        modeLabel = new QLabel(DataClassifyAppClass);
        modeLabel->setObjectName(QStringLiteral("modeLabel"));

        horizontalLayout_5->addWidget(modeLabel);

        mode1 = new QPushButton(DataClassifyAppClass);
        mode1->setObjectName(QStringLiteral("mode1"));

        horizontalLayout_5->addWidget(mode1);

        mode2 = new QPushButton(DataClassifyAppClass);
        mode2->setObjectName(QStringLiteral("mode2"));

        horizontalLayout_5->addWidget(mode2);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_4);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QStringLiteral("horizontalLayout_4"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_3);

        dataInputBtn = new QPushButton(DataClassifyAppClass);
        dataInputBtn->setObjectName(QStringLiteral("dataInputBtn"));

        horizontalLayout_4->addWidget(dataInputBtn);

        horizontalSpacer_2 = new QSpacerItem(18, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_2);

        dataOutputBtn = new QPushButton(DataClassifyAppClass);
        dataOutputBtn->setObjectName(QStringLiteral("dataOutputBtn"));

        horizontalLayout_4->addWidget(dataOutputBtn);

        horizontalSpacer = new QSpacerItem(588, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setObjectName(QStringLiteral("horizontalLayout_8"));
        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setObjectName(QStringLiteral("horizontalLayout_7"));
        horizontalSpacer_10 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_10);

        imgLabel = new QLabel(DataClassifyAppClass);
        imgLabel->setObjectName(QStringLiteral("imgLabel"));
        imgLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_7->addWidget(imgLabel);

        horizontalSpacer_8 = new QSpacerItem(40, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_8);

        imgSeq = new QLabel(DataClassifyAppClass);
        imgSeq->setObjectName(QStringLiteral("imgSeq"));

        horizontalLayout_7->addWidget(imgSeq);


        horizontalLayout_8->addLayout(horizontalLayout_7);

        horizontalSpacer_9 = new QSpacerItem(268, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_9);


        verticalLayout->addLayout(horizontalLayout_8);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QStringLiteral("horizontalLayout_3"));
        preBtn = new QPushButton(DataClassifyAppClass);
        preBtn->setObjectName(QStringLiteral("preBtn"));
        preBtn->setMinimumSize(QSize(200, 50));
        preBtn->setMaximumSize(QSize(200, 50));

        horizontalLayout_3->addWidget(preBtn);

        widget = new QWidget(DataClassifyAppClass);
        widget->setObjectName(QStringLiteral("widget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy);

        horizontalLayout_3->addWidget(widget);

        nextBtn = new QPushButton(DataClassifyAppClass);
        nextBtn->setObjectName(QStringLiteral("nextBtn"));
        nextBtn->setMinimumSize(QSize(200, 50));
        nextBtn->setMaximumSize(QSize(200, 50));

        horizontalLayout_3->addWidget(nextBtn);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setObjectName(QStringLiteral("horizontalLayout_6"));
        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_6);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        classifyLabel = new QLabel(DataClassifyAppClass);
        classifyLabel->setObjectName(QStringLiteral("classifyLabel"));

        horizontalLayout->addWidget(classifyLabel);

        number_0 = new QPushButton(DataClassifyAppClass);
        number_0->setObjectName(QStringLiteral("number_0"));

        horizontalLayout->addWidget(number_0);

        number_1 = new QPushButton(DataClassifyAppClass);
        number_1->setObjectName(QStringLiteral("number_1"));

        horizontalLayout->addWidget(number_1);

        number_2 = new QPushButton(DataClassifyAppClass);
        number_2->setObjectName(QStringLiteral("number_2"));

        horizontalLayout->addWidget(number_2);


        horizontalLayout_6->addLayout(horizontalLayout);

        horizontalSpacer_7 = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_7);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        largerBtn = new QPushButton(DataClassifyAppClass);
        largerBtn->setObjectName(QStringLiteral("largerBtn"));

        horizontalLayout_2->addWidget(largerBtn);

        smallerBtn = new QPushButton(DataClassifyAppClass);
        smallerBtn->setObjectName(QStringLiteral("smallerBtn"));

        horizontalLayout_2->addWidget(smallerBtn);


        horizontalLayout_6->addLayout(horizontalLayout_2);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_5);


        verticalLayout->addLayout(horizontalLayout_6);


        retranslateUi(DataClassifyAppClass);

        QMetaObject::connectSlotsByName(DataClassifyAppClass);
    } // setupUi

    void retranslateUi(QWidget *DataClassifyAppClass)
    {
        DataClassifyAppClass->setWindowTitle(QApplication::translate("DataClassifyAppClass", "DataClassifyApp", nullptr));
        modeLabel->setText(QApplication::translate("DataClassifyAppClass", "\346\250\241\345\274\217\351\200\211\346\213\251", nullptr));
        mode1->setText(QApplication::translate("DataClassifyAppClass", "\346\225\260\346\215\256\345\210\206\347\261\273", nullptr));
        mode2->setText(QApplication::translate("DataClassifyAppClass", "\345\274\202\345\270\270\346\225\260\346\215\256\346\214\221\351\200\211", nullptr));
        dataInputBtn->setText(QApplication::translate("DataClassifyAppClass", "\345\244\204\347\220\206\346\225\260\346\215\256\350\267\257\345\276\204", nullptr));
        dataOutputBtn->setText(QApplication::translate("DataClassifyAppClass", "\346\225\260\346\215\256\350\276\223\345\207\272\344\275\215\347\275\256", nullptr));
        imgLabel->setText(QApplication::translate("DataClassifyAppClass", "\345\233\276\347\211\207\345\220\215\347\247\260", nullptr));
        imgSeq->setText(QString());
        preBtn->setText(QApplication::translate("DataClassifyAppClass", "\344\270\212\344\270\200\345\274\240", nullptr));
        nextBtn->setText(QApplication::translate("DataClassifyAppClass", "\344\270\213\344\270\200\345\274\240", nullptr));
        classifyLabel->setText(QApplication::translate("DataClassifyAppClass", "\345\276\205\345\210\206\347\261\273\347\261\273\345\210\253", nullptr));
        number_0->setText(QApplication::translate("DataClassifyAppClass", "0\347\261\273", nullptr));
        number_1->setText(QApplication::translate("DataClassifyAppClass", "1\347\261\273", nullptr));
        number_2->setText(QApplication::translate("DataClassifyAppClass", "2\347\261\273", nullptr));
        largerBtn->setText(QApplication::translate("DataClassifyAppClass", "\346\224\276\345\244\247", nullptr));
        smallerBtn->setText(QApplication::translate("DataClassifyAppClass", "\347\274\251\345\260\217", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DataClassifyAppClass: public Ui_DataClassifyAppClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DATACLASSIFYAPP_H

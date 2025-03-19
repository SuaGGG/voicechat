/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionConnectServer;
    QAction *actionDisconnectServer;
    QAction *actionDisconnectAll;
    QAction *actionAudioInput;
    QAction *actionAudioOutput;
    QAction *actionSettings;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QListWidget *roomListWidget;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *joinRoomButton;
    QPushButton *leaveRoomButton;
    QMenuBar *menubar;
    QMenu *menuConnect;
    QMenu *menuTools;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        actionConnectServer = new QAction(MainWindow);
        actionConnectServer->setObjectName(QString::fromUtf8("actionConnectServer"));
        actionDisconnectServer = new QAction(MainWindow);
        actionDisconnectServer->setObjectName(QString::fromUtf8("actionDisconnectServer"));
        actionDisconnectAll = new QAction(MainWindow);
        actionDisconnectAll->setObjectName(QString::fromUtf8("actionDisconnectAll"));
        actionAudioInput = new QAction(MainWindow);
        actionAudioInput->setObjectName(QString::fromUtf8("actionAudioInput"));
        actionAudioOutput = new QAction(MainWindow);
        actionAudioOutput->setObjectName(QString::fromUtf8("actionAudioOutput"));
        actionSettings = new QAction(MainWindow);
        actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        roomListWidget = new QListWidget(centralwidget);
        roomListWidget->setObjectName(QString::fromUtf8("roomListWidget"));

        verticalLayout->addWidget(roomListWidget);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        joinRoomButton = new QPushButton(centralwidget);
        joinRoomButton->setObjectName(QString::fromUtf8("joinRoomButton"));

        horizontalLayout_2->addWidget(joinRoomButton);

        leaveRoomButton = new QPushButton(centralwidget);
        leaveRoomButton->setObjectName(QString::fromUtf8("leaveRoomButton"));

        horizontalLayout_2->addWidget(leaveRoomButton);


        verticalLayout->addLayout(horizontalLayout_2);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        menuConnect = new QMenu(menubar);
        menuConnect->setObjectName(QString::fromUtf8("menuConnect"));
        menuTools = new QMenu(menubar);
        menuTools->setObjectName(QString::fromUtf8("menuTools"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuConnect->menuAction());
        menubar->addAction(menuTools->menuAction());
        menuConnect->addAction(actionConnectServer);
        menuConnect->addAction(actionDisconnectServer);
        menuConnect->addAction(actionDisconnectAll);
        menuTools->addAction(actionAudioInput);
        menuTools->addAction(actionAudioOutput);
        menuTools->addSeparator();
        menuTools->addAction(actionSettings);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "voicechat", nullptr));
        actionConnectServer->setText(QCoreApplication::translate("MainWindow", "\350\277\236\346\216\245\346\234\215\345\212\241\345\231\250", nullptr));
#if QT_CONFIG(shortcut)
        actionConnectServer->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDisconnectServer->setText(QCoreApplication::translate("MainWindow", "\346\226\255\345\274\200\345\275\223\345\211\215\346\234\215\345\212\241\345\231\250", nullptr));
#if QT_CONFIG(shortcut)
        actionDisconnectServer->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionDisconnectAll->setText(QCoreApplication::translate("MainWindow", "\346\226\255\345\274\200\346\211\200\346\234\211\346\234\215\345\212\241\345\231\250", nullptr));
#if QT_CONFIG(shortcut)
        actionDisconnectAll->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+D", nullptr));
#endif // QT_CONFIG(shortcut)
        actionAudioInput->setText(QCoreApplication::translate("MainWindow", "\351\237\263\351\242\221\350\276\223\345\205\245", nullptr));
        actionAudioOutput->setText(QCoreApplication::translate("MainWindow", "\351\237\263\351\242\221\350\276\223\345\207\272", nullptr));
        actionSettings->setText(QCoreApplication::translate("MainWindow", "\350\256\276\347\275\256", nullptr));
#if QT_CONFIG(shortcut)
        actionSettings->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        joinRoomButton->setText(QCoreApplication::translate("MainWindow", "\345\212\240\345\205\245\346\210\277\351\227\264", nullptr));
        leaveRoomButton->setText(QCoreApplication::translate("MainWindow", "\347\246\273\345\274\200\346\210\277\351\227\264", nullptr));
        menuConnect->setTitle(QCoreApplication::translate("MainWindow", "\350\277\236\346\216\245", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "\345\267\245\345\205\267", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

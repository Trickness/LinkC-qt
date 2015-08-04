#ifndef MAINWINDOW
#define MAINWINDOW

#include <QWidget>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QScrollArea>
#include <QMap>
#include <QThread>
#include "LoginWindow.h"
#include "gurgle.h"
#include "LinkcUi.h"

class MessageReceiver : public QThread{
    Q_OBJECT
public:
    explicit MessageReceiver(QObject *parent,gurgle *_core = nullptr);
    ~MessageReceiver();
    void    setCore(gurgle *_core = nullptr);
    void    run();
signals:
    void    messageReceived(QString User,QString Message,int Id);
public slots:
    void    messageSaveDone(void);
private:
    gurgle  *core;
    bool    flag;
    char    *recvBuf;
};


class MainWindow : public QWidget{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void    resizeEvent(QResizeEvent *);
    void    closeEvent(QCloseEvent *);
    bool    refreshSubscribedList(void);
    bool    refreshePresence(void);
public slots:
    void SLOT_LoginWinSignInButtonClicked();
    void SLOT_LoginWinCancelButtonClicked();
    void SLOT_PresenceNameUpdated(QString);
    void SLOT_PresenceMoodUpdated(QString);
    void SLOT_ItemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*);
    void SLOT_SubscribedButtonClicked(bool);
    void SLOT_UnsubscribedButtonClicked(bool);
    void SLOT_RefreshSubscribeList(void);
    void SLOT_MessageReceived(QString, QString, int);
private:
    LoginWindow         *LoginW;
    gurgle              *core;
    packageList         *MessageList;
    LinkcSubscribeDialog*SubscribeDialog;
    MessageReceiver     *MsgReceiver;
    // UI
    LinkcPresenceEdit   *Ui_Name;
    LinkcPresenceEdit   *Ui_Mood;
    QLabel              *Ui_Status;
    QWidget             *Ui_PresenceBase;
    QWidget             *Ui_PresenceWidget;
    QWidget             *Ui_HeadWidget;
    QVBoxLayout         *Ui_PresenceLayout;
    QHBoxLayout         *Ui_PresenceBaseLayout;
    LinkcGroupSelect    *Ui_GroupSelect;
    QScrollArea         *Ui_GroupScrollArea;
    QPushButton         *Ui_SubscribedButton;
    QPushButton         *Ui_UnsubscribedButton;
};

#endif // MAINWINDOW


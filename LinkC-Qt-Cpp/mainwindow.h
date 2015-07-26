#ifndef MAINWINDOW
#define MAINWINDOW

#include <QWidget>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QScrollArea>
#include "LoginWindow.h"
#include "gurgle.h"
#include "LinkcUi.h"

class MainWindow : public QWidget{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void resizeEvent(QResizeEvent *);
public slots:
    void SLOT_LoginWinSignInButtonClicked();
    void SLOT_LoginWinCancelButtonClicked();
private:
    LoginWindow *LoginW;
    gurgle      *core;
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
};

#endif // MAINWINDOW


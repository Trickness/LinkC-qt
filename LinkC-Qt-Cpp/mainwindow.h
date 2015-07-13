#ifndef MAINWINDOW
#define MAINWINDOW

#include <QWidget>
#include <QMessageBox>
#include "LoginWindow.h"
#include "gurgle.h"

class MainWindow : public QWidget{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    void SLOT_LoginWinSignInButtonClicked();
    void SLOT_LoginWinCancelButtonClicked();
private:
    LoginWindow *LoginW;
    gurgle      *core;
};

#endif // MAINWINDOW


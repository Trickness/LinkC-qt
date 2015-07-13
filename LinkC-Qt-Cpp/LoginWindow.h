#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

class LoginWindow : public QWidget{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = 0);
    ~LoginWindow();
    QString     GetUsername(void);
    QString     GetPassword(void);
    QString     GetDomain(void);
    int         GetPort(void);
signals:
    void    SIGN_SignInButtonClicked();
    void    SIGN_CancelButtonClicked();
private slots:
    void    SLOT_SignInButtonClicked();
    void    SLOT_CancelButtonClicked();
private:
    QLineEdit   *UsernameInput;
    QLineEdit   *PasswordInput;
    QLineEdit   *HostInput;
    QLineEdit   *PortInput;
    QPushButton *SignInButton;
    QPushButton *CancelButton;
    QLabel      *UsernameLabel;
    QLabel      *PasswordLabel;
    QLabel      *HostLabel;
    QLabel      *PortLabel;
    QString     username;
    QString     password;
    QString     host;
    int         port;
};

#endif // MAINWINDOW_H

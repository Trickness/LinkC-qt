#include "LoginWindow.h"
#include <string>
#include <iostream>
using namespace std;

LoginWindow::LoginWindow(QWidget *parent)
    :QWidget(parent){
    this->setMaximumSize(300,165);
    this->setMinimumSize(300,165);
    this->HostLabel     = new QLabel(this);
    this->PortLabel     = new QLabel(this);
    this->UsernameInput = new QLineEdit(this);
    this->PasswordInput = new QLineEdit(this);
    this->HostInput     = new QLineEdit(this);
    this->PortInput     = new QLineEdit(this);
    this->SignInButton  = new QPushButton(this);
    this->CancelButton  = new QPushButton(this);
    this->UsernameLabel = new QLabel(this);
    this->PasswordLabel = new QLabel(this);

    this->HostLabel->setGeometry(30,10,50,27);
    this->HostInput->setGeometry(90,10,100,27);
    this->PortLabel->setGeometry(200,10,25,27);
    this->PortInput->setGeometry(230,10,40,27);
    this->UsernameLabel->setGeometry(30,45,50,27);
    this->UsernameInput->setGeometry(90,45,180,27);
    this->PasswordLabel->setGeometry(30,80,50,27);
    this->PasswordInput->setGeometry(90,80,180,27);
    this->CancelButton->setGeometry(185,120,85,30);
    this->SignInButton->setGeometry(90,120,85,30);

    this->HostLabel->setText(tr("Host"));
    this->PortLabel->setText(tr("Port"));
    this->SignInButton->setText(tr("SignIn"));
    this->CancelButton->setText(tr("Cancel"));
    this->UsernameLabel->setText(tr("Username"));
    this->PasswordLabel->setText(tr("Password"));

    this->HostInput->setText("localhost");
    this->PortInput->setText("40097");
    this->PasswordInput->setEchoMode(QLineEdit::Password);

    this->connect(this->SignInButton,SIGNAL(clicked(bool)),this,SLOT(SLOT_SignInButtonClicked()));
    this->connect(this->CancelButton,SIGNAL(clicked(bool)),this,SLOT(SLOT_CancelButtonClicked()));
}

LoginWindow::~LoginWindow(){
    delete this->UsernameInput;
    delete this->PasswordInput;
    delete this->SignInButton;
    delete this->CancelButton;
    delete this->UsernameLabel;
    delete this->PasswordLabel;
    delete this->HostInput;
    delete this->HostLabel;
    delete this->PortInput;
    delete this->PortLabel;
}


void LoginWindow::SLOT_CancelButtonClicked(){
    emit this->SIGN_CancelButtonClicked();
}

void LoginWindow::SLOT_SignInButtonClicked(){
    if(this->HostInput->text().isEmpty()){
        QMessageBox::warning(this,tr("Warning"),"Host is empty",QMessageBox::Ok);
        return;
    }
    if(this->UsernameInput->text().isEmpty()){
        QMessageBox::warning(this,tr("Warning"),"Username is empty",QMessageBox::Ok);
        return;
    }
    if(this->PasswordInput->text().isEmpty()){
        QMessageBox::warning(this,tr("Warning"),"Password is empty",QMessageBox::Ok);
        return;
    }
    if(this->PortInput->text().toInt() == 0){
        QMessageBox::warning(this,tr("Warning"),"Port is empty",QMessageBox::Ok);
        return;
    }
    this->host      = this->HostInput->text();
    this->username  = this->UsernameInput->text();
    this->password  = this->PasswordInput->text();
    this->port      = this->PortInput->text().toInt();
    emit this->SIGN_SignInButtonClicked();
}

QString LoginWindow::GetUsername(){
    return this->username;
}

QString LoginWindow::GetPassword(){
    return this->password;
}

QString LoginWindow::GetDomain(){
    return this->host;
}

int LoginWindow::GetPort(){
    return this->port;
}

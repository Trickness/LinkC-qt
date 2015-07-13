#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent):
    QWidget(parent){
    this->core      = new gurgle();
    this->LoginW    = new LoginWindow();
    this->connect(LoginW,SIGNAL(SIGN_SignInButtonClicked()),this,SLOT(SLOT_LoginWinSignInButtonClicked()));
    this->connect(LoginW,SIGNAL(SIGN_CancelButtonClicked()),this,SLOT(SLOT_LoginWinCancelButtonClicked()));
    this->LoginW->show();
    this->hide();
}

MainWindow::~MainWindow(){
    delete this->core;
}

void MainWindow::SLOT_LoginWinCancelButtonClicked(){
    exit(0);
}

void MainWindow::SLOT_LoginWinSignInButtonClicked(){
    char *host = new char[32];
    char *username = new char[32];
    char *password = new char[32];
    int port;
    memset(host,0,32);
    memset(username,0,32);
    memset(password,0,32);
    strncpy(host,this->LoginW->GetDomain().toUtf8().data(),this->LoginW->GetDomain().length());
    strncpy(username,this->LoginW->GetUsername().toUtf8().data(),this->LoginW->GetUsername().length());
    strncpy(password,this->LoginW->GetPassword().toUtf8().data(),this->LoginW->GetPassword().length());
    port = this->LoginW->GetPort();
    if(!this->core->connect_to_server(host,port,nullptr)){
        QMessageBox::warning(this,tr("Warning"),tr("Cannot connect to remote"),QMessageBox::Ok);
        return;
    }
    if(!this->core->plain_password_auth(this->core->analyse_full_id(this->core->make_up_full_id(username,host,nullptr)),password)){
        QMessageBox::warning(this,tr("Warning"),tr("Cannot authenticate [Worning username or password]"),QMessageBox::Ok);
        return;
    }
    this->show();
    this->LoginW->hide();
    this->core->write_log("Auth successfully");
}

#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent):
    QWidget(parent){
    this->core      = new gurgle();
    this->LoginW    = new LoginWindow();
    this->connect(LoginW,SIGNAL(SIGN_SignInButtonClicked()),this,SLOT(SLOT_LoginWinSignInButtonClicked()));
    this->connect(LoginW,SIGNAL(SIGN_CancelButtonClicked()),this,SLOT(SLOT_LoginWinCancelButtonClicked()));
    this->LoginW->show();
    this->hide();

    //  UI TEST
    //this->LoginW->hide();

    this->Ui_PresenceBase       = new QWidget(this);
    this->Ui_PresenceWidget     = new QWidget(this->Ui_PresenceBase);
    this->Ui_HeadWidget         = new QWidget(this);
    this->Ui_PresenceLayout     = new QVBoxLayout;
    this->Ui_PresenceBaseLayout = new QHBoxLayout;
    this->Ui_Name               = new LinkcPresenceEdit;
    this->Ui_Mood               = new LinkcPresenceEdit;
    this->Ui_GroupScrollArea    = new QScrollArea(this);
    this->Ui_GroupSelect        = new LinkcGroupSelect(this->Ui_GroupScrollArea);
    this->Ui_PresenceBase->setLayout(this->Ui_PresenceBaseLayout);
    this->Ui_PresenceWidget->setStyleSheet("background-color:white");
    this->Ui_HeadWidget->setStyleSheet("background-color:yellow");
    this->Ui_PresenceWidget->setLayout(this->Ui_PresenceLayout);
    this->Ui_Name->setDefaultText(tr("NoName"));
    this->Ui_Mood->setDefaultText(tr("Please press here to update your mood"));
    this->Ui_PresenceBaseLayout->setMargin(0);
    this->Ui_PresenceBaseLayout->addWidget(this->Ui_PresenceWidget);
    this->Ui_PresenceBaseLayout->addSpacing(50);
    this->Ui_PresenceLayout->setMargin(0);
    this->Ui_PresenceLayout->setSpacing(5);
    this->Ui_PresenceLayout->addWidget(this->Ui_Name);
    this->Ui_PresenceLayout->addWidget(this->Ui_Mood);
    this->setMaximumWidth(300);
    this->setMinimumWidth(200);
    this->setMinimumHeight(400);
    this->Ui_GroupScrollArea->setParent(this);
    this->Ui_GroupScrollArea->setGeometry(0,50,this->width(),this->height()-50);
    this->Ui_GroupScrollArea->setVisible(true);
    this->Ui_GroupScrollArea->setWidget(this->Ui_GroupSelect);

    this->connect(this->Ui_Name,SIGNAL(ContentUpdated(QString)),this,SLOT(SLOT_PresenceNameUpdated(QString)));
    this->connect(this->Ui_Mood,SIGNAL(ContentUpdated(QString)),this,SLOT(SLOT_PresenceMoodUpdated(QString)));
}

MainWindow::~MainWindow(){
    delete this->core;
    delete this->LoginW;
    delete this->Ui_Mood;
    delete this->Ui_Name;
    delete this->Ui_PresenceLayout;
    delete this->Ui_PresenceWidget;
    delete this->Ui_PresenceBaseLayout;
    delete this->Ui_PresenceBase;
}
void MainWindow::resizeEvent(QResizeEvent *){
    this->Ui_PresenceBase->setGeometry(0,0,this->width(),50);
    this->Ui_HeadWidget->setGeometry(this->width()-50,0,50,50);
    this->Ui_GroupScrollArea->setGeometry(0,50,this->width(),this->height()-50);
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
    if(this->refreshSubscribedList() == false){
        this->core->write_log("Failed to refresh list");
    }
    if(this->refreshePresence() == false){
        this->core->write_log("Failed to refresh presence");
    }
}

bool MainWindow::refreshSubscribedList(){
    int count = 0;
    gurgle_subscription_t   *list = nullptr;
    list = this->core->query_roster(count);
    if(list == nullptr)
        return false;
    LinkcGroupItem *item = new LinkcGroupItem();
    item->setParent(this->Ui_GroupSelect);
    item->setGroupName(tr("Unnamed"));
    int i;
    for(i=0;i<count;i++){
        item->InsertSubscribedItem(new LinkcSubscribedItem(item));
        this->core->write_log(list[i].presence.id);
    }
    this->Ui_GroupSelect->insertGroup(item);
    item->show();
    return true;
}

bool MainWindow::refreshePresence(){
    gurgle_presence_t *self_presence = this->core->query_presence();
    if(self_presence == nullptr){
        return false;
    }
    QString Name;
    if(strcmp(self_presence->last_name,"") != 0){
        Name.append(self_presence->last_name);
        Name.append(" ");
    }
    if(strcmp(self_presence->first_name,"") != 0){
        Name.append(self_presence->first_name);
    }
    if(Name == ""){
        Name = "NoName";
    }
    this->Ui_Name->setContent(Name,true);
    if(strcmp(self_presence->mood,"")!=0)
        this->Ui_Mood->setContent(self_presence->mood,true);
    return true;
}

void MainWindow::SLOT_PresenceNameUpdated(QString Name){
    if(Name.length() > 32){
        QMessageBox::warning(this,tr("Warning"),tr("Too long for name"));
    }
    gurgle_presence_t *t = new gurgle_presence_t;
    strncpy(t->last_name,Name.toUtf8().data(),Name.length());
    if(this->core->publish_self_presence_update(t) == false){
        this->core->write_log("Failed to publish presence");
        QMessageBox::warning(this,tr("Warning"),tr("Failed to publish presences"));
        return;
    }
}

void MainWindow::SLOT_PresenceMoodUpdated(QString Mood){
    if(Mood.length() > 512){
        QMessageBox::warning(this,tr("Warning"),tr("Too long for Mood"));
    }
    gurgle_presence_t *t = new gurgle_presence_t;
    strncpy(t->mood,Mood.toUtf8().data(),Mood.length());
    if(this->core->publish_self_presence_update(t) == false){
        this->core->write_log("Failed to publish presence");
        QMessageBox::warning(this,tr("Warning"),tr("Failed to publish presences"));
        return;
    }
}

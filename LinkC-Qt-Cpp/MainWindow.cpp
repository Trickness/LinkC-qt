#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent):
    QWidget(parent){
    this->core              = new gurgle();
    this->LoginW            = new LoginWindow();
    this->MessageList       = new packageList();
    this->SubscribeDialog   = new LinkcSubscribeDialog(NULL,this->core);
    this->MsgReceiver       = new MessageReceiver(this,this->core);
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
    this->Ui_SubscribedButton   = new QPushButton(this);
    this->Ui_UnsubscribedButton = new QPushButton(this);
    this->Ui_SubscribedButton->setText(tr("Subscribe"));
    this->Ui_UnsubscribedButton->setText(tr("Unsubscribe"));
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
    this->connect(this->Ui_GroupSelect,SIGNAL(itemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*)),this,SLOT(SLOT_ItemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*)));
    this->connect(this->Ui_SubscribedButton,SIGNAL(clicked(bool)),this,SLOT(SLOT_SubscribedButtonClicked(bool)));
    this->connect(this->Ui_UnsubscribedButton,SIGNAL(clicked(bool)),this,SLOT(SLOT_UnsubscribedButtonClicked(bool)));
    this->connect(this->SubscribeDialog,SIGNAL(subscribeDone()),this,SLOT(SLOT_RefreshSubscribeList()));
    this->connect(this->MsgReceiver,SIGNAL(messageReceived(QString,QString,int)),this,SLOT(SLOT_MessageReceived(QString,QString,int)));

    this->setWindowTitle(tr("LinkC"));
}

MainWindow::~MainWindow(){
    delete this->core;
    delete this->LoginW;
    delete this->MessageList;
    delete this->SubscribeDialog;
    delete this->Ui_Mood;
    delete this->Ui_Name;
    delete this->Ui_PresenceLayout;
    delete this->Ui_PresenceWidget;
    delete this->Ui_PresenceBaseLayout;
    delete this->Ui_PresenceBase;
    delete this->Ui_SubscribedButton;
    delete this->Ui_UnsubscribedButton;
}
void MainWindow::resizeEvent(QResizeEvent *){
    this->Ui_PresenceBase->setGeometry(0,0,this->width(),50);
    this->Ui_HeadWidget->setGeometry(this->width()-50,0,50,50);
    this->Ui_GroupScrollArea->setGeometry(0,50,this->width(),this->height()-100);
    this->Ui_GroupSelect->resize(this->width()-10,this->Ui_GroupSelect->height());
    this->Ui_SubscribedButton->setGeometry(10,this->Ui_GroupScrollArea->y()+this->Ui_GroupScrollArea->height()+5,140,30);
    this->Ui_UnsubscribedButton->setGeometry(this->Ui_SubscribedButton->x()+145,this->Ui_GroupScrollArea->y()+this->Ui_GroupScrollArea->height()+5,140,30);
}
void MainWindow::closeEvent(QCloseEvent *){
    this->Ui_GroupSelect->clearSelect();
}

void MainWindow::SLOT_LoginWinCancelButtonClicked(){
    exit(0);
}

void MainWindow::SLOT_SubscribedButtonClicked(bool){
    this->SubscribeDialog->show();
}

void MainWindow::SLOT_UnsubscribedButtonClicked(bool){
    LinkcSubscribedItem* tmpItem = this->Ui_GroupSelect->getCurrentSelectedSubscribedItem();
    if(tmpItem == nullptr){
        QMessageBox::warning(this,tr("Warning"),tr("You have not selected any item"));
        return;
    }
    if(this->core->update_roster(tmpItem->getId(),nullptr,nullptr,UNSUBSCRIBE) == false){
        QMessageBox::warning(this,tr("Warning"),tr("Failed to unsubscribe"));
        return;
    }
    this->refreshSubscribedList();
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
    if(!this->core->connect_to_server(host,port,nullptr,2)){
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
    char *buf = new char[512];
    char *tmpBuf = new char[512];
    char *tmpBuf2;
    rapidjson::Document d;
    while(1){
        memset(buf,0,512);
        memset(tmpBuf,0,512);
        if(this->core->gurgle_recv(buf,512,0,"message",1,1)<=0)
            break;
        d.Parse(buf);
        if(d.IsNull())
            break;
        memset(buf,0,512);
        tmpBuf2 = new char[512];
        memset(tmpBuf2,0,512);
        strncpy(tmpBuf2,d["params"]["message"].GetString(),d["params"]["message"].GetStringLength());
        strncpy(tmpBuf,d["params"]["from"].GetString(),d["params"]["from"].GetStringLength());
        this->MessageList->insert(tmpBuf2,d["id"].GetInt(),tmpBuf);
    }
    this->MsgReceiver->start();
}

bool MainWindow::refreshSubscribedList(){
    int count = 0;
    gurgle_subscription_t   *list = nullptr;
    this->Ui_GroupSelect->clearSelect();
    list = this->core->query_roster(count);
    if(list == nullptr)
        return false;
    LinkcGroupItem *item = new LinkcGroupItem();
    item->setCore(this->core);
    item->setParent(this->Ui_GroupSelect);
    int i;
    for(i=0;i<count;i++){
        if(strcmp(list[i].presence.id,"") == 0){
            continue;
        }
        item->InsertSubscribedItem(new LinkcSubscribedItem(item,&list[i]));
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
        Name = self_presence->id;
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
    memset(t,0,sizeof(gurgle_presence_t));
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
    memset(t,0,sizeof(gurgle_presence_t));
    strncpy(t->mood,Mood.toUtf8().data(),Mood.length());
    if(this->core->publish_self_presence_update(t) == false){
        this->core->write_log("Failed to publish presence");
        QMessageBox::warning(this,tr("Warning"),tr("Failed to publish presences"));
        return;
    }
}

void MainWindow::SLOT_ItemDoubleClicked(LinkcGroupItem *, LinkcSubscribedItem *item){
    if(item->ChatDialog != nullptr){
        item->ChatDialog->show();
        item->ChatDialog->setFocus();
        return;
    }
    item->ChatDialog = new LinkcChatDialog(NULL,this->core,&(item->getInfo()->presence));
    char *Message = nullptr;
    while(1){
        Message = this->MessageList->get_data(0,item->getInfo()->presence.id);
        if(Message == nullptr)
            break;
        item->ChatDialog->setMessage(Message);
        this->MessageList->remove(0,item->getInfo()->presence.id);
    }
    Message = nullptr;
    item->ChatDialog->show();
}

void MainWindow::SLOT_RefreshSubscribeList(){
    if(this->refreshSubscribedList() == false)
        QMessageBox::warning(this,tr("Warning"),tr("Failed to refresh subscribed list"));
}

void MainWindow::SLOT_MessageReceived(QString User, QString Msg,int id){
    LinkcSubscribedItem *tmpItem = this->Ui_GroupSelect->findItem(User);
    if(tmpItem){
        if(tmpItem->ChatDialog){
            tmpItem->ChatDialog->setMessage(Msg);
            this->MsgReceiver->messageSaveDone();
            return;
        }
    }
    int len = Msg.length();
    int len2 = User.length();
    char *buf = new char[len+1];
    char *buf_usr = new char[len2+1];
    memset(buf,0,len+1);
    memset(buf_usr,0,len2+1);
    memcpy(buf,Msg.toUtf8().data(),len);
    memcpy(buf_usr,User.toUtf8().data(),User.length());
    buf[len] = 0;
    buf_usr[len2] = 0;
    this->MessageList->insert(buf,id,buf_usr);
    this->MsgReceiver->messageSaveDone();
}


MessageReceiver::MessageReceiver(QObject *parent, gurgle *_core){
    this->setParent(parent);
    if(_core != nullptr)
        this->core = _core;
    else
        this->core = nullptr;
    this->recvBuf = new char[1024];
    this->flag = true;
}

MessageReceiver::~MessageReceiver(){
    delete this->recvBuf;
}

void MessageReceiver::run(){
    if(this->core == nullptr)
        return;
    int recved = 0;
    this->flag = true;
    rapidjson::Document d;
    QString Id,Message;
    while(1){
        while(!this->flag); // wait MainThread to save message
        recved = this->core->gurgle_recv(recvBuf,1024,0,"message",5,10);
        if(!(recved>=0)){
            if(this->core->is_connected() == false)
                break;
            continue;
        }
        d.Parse(recvBuf);
        if(d.IsNull())
            continue;
        if(d["params"].HasMember("from"))
            if(d["params"]["from"].IsString())
                Id = d["params"]["from"].GetString();
        if(d["params"].HasMember("message"))
            if(d["params"]["message"].IsString())
                Message = d["params"]["message"].GetString();
        if(Message!="" && Id!=""){
            this->flag = false;
            emit this->messageReceived(Id,Message,d["id"].GetInt());
        }
    }
}

void MessageReceiver::messageSaveDone(){
    this->flag = true;
}

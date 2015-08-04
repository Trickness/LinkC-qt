#include "LinkcUi.h"

LinkcPresenceEdit::LinkcPresenceEdit(QWidget *parent){
    this->setParent(parent);
    this->setStyleSheet("QLineEdit{border-width:0;border-style:outset}");
    this->setDisabled(true);
    this->_hasContent = false;
    this->connect(this,SIGNAL(returnPressed()),this,SLOT(LeaveFocus()));
}

LinkcPresenceEdit::~LinkcPresenceEdit(){

}

void LinkcPresenceEdit::enterEvent(QEvent *){
    this->setDisabled(false);
}

void LinkcPresenceEdit::leaveEvent(QEvent *){
    if(!this->hasFocus())
        this->setDisabled(true);
}

void LinkcPresenceEdit::LeaveFocus(){
    if(this->hasFocus())
        this->clearFocus();
    if(this->text().length() != 0)
        this->setContent(this->text());
}

void LinkcPresenceEdit::setContent(QString s, bool NoUpdating){
    this->clear();
    this->setPlaceholderText(s);
    this->_hasContent = true;
    this->setCursorPosition(0);
    this->_Content = s;
    this->update();
    if(NoUpdating == false)
        emit this->ContentUpdated(s);
}

bool LinkcPresenceEdit::hasContent(){
    return this->_hasContent;
}

void LinkcPresenceEdit::setDefaultText(QString s){
    this->setPlaceholderText(s);
    this->setCursorPosition(0);
    this->_defaultText = s;
}

void LinkcPresenceEdit::clearContent(){
    this->setPlaceholderText(this->_defaultText);
    this->_hasContent = false;
    this->setCursorPosition(0);
}

LinkcSubscribedItem::LinkcSubscribedItem(QWidget *parent, gurgle_subscription_t *info){
    this->setParent(parent);
    this->MainPresence  = new QLabel(this);
    this->Mood          = new QLabel(this);
    this->HeadIcon      = new QLabel(this);
    this->Subscription  = nullptr;
    this->ChatDialog    = nullptr;
    this->shown         = true;
    this->chatDialogVisible = false;
    this->doInfoSet     = false;
    this->Font.setPixelSize(12);
    this->MainPresence->setFont(this->Font);
    this->Font.setPixelSize(10);
    this->Mood->setFont(this->Font);
    this->resize(GROUP_WIDGET_WIDTH,ITEM_HEIGTH);

    this->HeadIcon->setStyleSheet("background-color:yellow");
    this->MainPresence->setStyleSheet("background-color:gray");

    QString  Name;
    if (info != nullptr){
        this->Subscription = info;
        Name = info->presence.last_name;
        Name.append(" ");
        Name.append(info->presence.id);
        this->doInfoSet = true;
        if(Name == ""){
            Name = info->presence.id;
        }
    }
    if(Name == "")
        Name = "NoName";
    this->MainPresence->setText(Name);

    this->show();
}

LinkcSubscribedItem::~LinkcSubscribedItem(){
    delete this->MainPresence;
    delete this->Mood;
    delete this->HeadIcon;
    if(this->ChatDialog)
        delete this->ChatDialog;
}

void LinkcSubscribedItem::resizeEvent(QResizeEvent *){
    this->HeadIcon->setGeometry(0,0,this->height(),this->height());   // xx
    this->MainPresence->setGeometry(this->height(),0,this->width()-50,this->height());
}

void LinkcSubscribedItem::mousePressEvent(QMouseEvent*){
    this->MainPresence->setStyleSheet("background-color:#B0B0B0");
    emit this->clicked(this);
}

void LinkcSubscribedItem::mouseDoubleClickEvent(QMouseEvent *){
    emit  this->doubleclicked(this);
}

void LinkcSubscribedItem::unClicked(){
    this->MainPresence->setStyleSheet("background-color:#F9F8F6;");
}
LinkcSubscribedItem* LinkcSubscribedItem::getPrev(){
    return this->prev;
}
LinkcSubscribedItem* LinkcSubscribedItem::getNext(){
    return this->next;
}

void LinkcSubscribedItem::setPrev(LinkcSubscribedItem *s){
    this->prev = s;
}

void LinkcSubscribedItem::setNext(LinkcSubscribedItem *s){
    this->next = s;
}

int LinkcSubscribedItem::getHeight(){
    return this->_height;
}

char *LinkcSubscribedItem::getId(){
    return this->Subscription->presence.id;
}

gurgle_subscription_t *LinkcSubscribedItem::getInfo(){
    if(this->Subscription == nullptr)
        return nullptr;
    gurgle_subscription_t *tmp = new gurgle_subscription_t;
    memcpy(tmp,this->Subscription,sizeof(gurgle_subscription_t));
    return tmp;
}


LinkcGroupItem::LinkcGroupItem(QWidget *parent){
    this->setParent(parent);
    setStyleSheet("background-color:yellow");
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->Title     = new QLabel(this);
    this->Icon      = new QLabel(this);
    this->shown     = true;
    this->pixSize.setHeight(16);
    this->pixSize.setWidth(16);
    this->ItemMap   = new s_map_t;

    //image.load("://image//expand.png");
    //pix.convertFromImage(image);
    //pix = pix.scaled(pixSize,Qt::KeepAspectRatio);
    //this->Icon->setPixmap(pix);
    this->Icon->setGeometry(0,0,16,GROUP_HEIGTH);
    this->Title->setGeometry(GROUP_SPACING,0,GROUP_WIDGET_WIDTH,GROUP_HEIGTH);

    TitleFont.setPixelSize(14);
    this->Title->setFont(TitleFont);
    this->Title->setText(tr("Unnamed group"));

    this->resize(GROUP_WIDGET_WIDTH,GROUP_HEIGTH);
}

LinkcGroupItem::~LinkcGroupItem(){
    if(this->ItemMap->isEmpty() == false){
        s_map_t::iterator i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            delete i.value();
            i++;
        }
        this->ItemMap->clear();
    }
    delete this->Title;
    delete this->Icon;
}

void LinkcGroupItem::setGroupName(QString name){
    this->Title->setText(name);
}

void LinkcGroupItem::clearClickedStatus(){
    if(this->ItemMap->isEmpty() == false){
        s_map_t::iterator i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            i.value()->unClicked();
            i++;
        }
    }
}

int LinkcGroupItem::getSize(){
    return this->ItemMap->size();
}

void LinkcGroupItem::mousePressEvent(QMouseEvent *){
    if (this->shown){
        //image.load("://image//collapsed.png");
        //pix.convertFromImage(image);
        //pix = pix.scaled(pixSize,Qt::KeepAspectRatio);
        //this->Icon->setPixmap(pix);
        this->resize(GROUP_WIDGET_WIDTH,GROUP_HEIGTH);
        if(this->ItemMap->isEmpty() == false){
            s_map_t::iterator i = this->ItemMap->begin();
            while(i != this->ItemMap->end()){
                i.value()->setVisible(false);
                i.value()->unClicked();
                i++;
            }
        }
    }else{
        //image.load("://image//expand.png");
        //pix.convertFromImage(image);
        //pix = pix.scaled(pixSize,Qt::KeepAspectRatio);
        //this->Icon->setPixmap(pix);
        this->resize(GROUP_WIDGET_WIDTH,GROUP_HEIGTH+(this->ItemMap->size())*ITEM_HEIGTH);
        if(this->ItemMap->isEmpty() == false){
            s_map_t::iterator i = this->ItemMap->begin();
            while(i != this->ItemMap->end()){
                i.value()->setVisible(true);
                i.value()->unClicked();
                i++;
            }
        }
    }
    this->shown = !this->shown;
    emit groupClicked(this,this->shown);
}


void LinkcGroupItem::InsertSubscribedItem(LinkcSubscribedItem* Item,int order){
    if(Item == nullptr){
        return;
    }
    if (order == -1)
        order = this->ItemMap->size();
    Item->setParent(this);
    this->connect(Item,SIGNAL(clicked(LinkcSubscribedItem*)),this,SLOT(onItemClicked(LinkcSubscribedItem*)));
    this->connect(Item,SIGNAL(doubleclicked(LinkcSubscribedItem*)),this,SLOT(onItemDoubleClicked(LinkcSubscribedItem*)));
    if(this->ItemMap->size() == 0){
        Item->setPrev(NULL);
        Item->setNext(NULL);
        this->ItemMap->insert(order,Item);
        Item->setGeometry(ITEM_SPACING,GROUP_HEIGTH+(ItemMap->size()-1)*ITEM_HEIGTH,GROUP_WIDGET_WIDTH,ITEM_HEIGTH);
        this->resize(this->width(),this->height()+Item->getHeight());
        return;
    }
    Item->setNext(NULL);
    this->ItemMap->insert(order,Item);
    s_map_t::iterator i = this->ItemMap->end();
    i -= 2;
    i.value()->setNext(Item);
    Item->setPrev(i.value());
    Item->setGeometry(ITEM_SPACING,GROUP_HEIGTH+(ItemMap->size()-1)*ITEM_HEIGTH,GROUP_WIDGET_WIDTH,ITEM_HEIGTH);
    this->resize(this->width(),this->height()+Item->getHeight());
    Item->show();
    Item->setParent(this);
    return;
}

void LinkcGroupItem::onItemClicked(LinkcSubscribedItem *item){
    if(this->ItemMap->isEmpty() == false){
        s_map_t::iterator i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            if(i.value() != item)
                i.value()->unClicked();
            i++;
        }
    }
    emit subscribedItemClicked(this,item);
}

void LinkcGroupItem::onItemDoubleClicked(LinkcSubscribedItem *item){
    if(this->ItemMap->isEmpty() == false){
        s_map_t::iterator i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            if(i.value() != item)
                i.value()->unClicked();
            i++;
        }
    }
    emit subscribedItemDoubleClicked(this,item);
}

void LinkcGroupItem::removeItem(LinkcSubscribedItem *Item){
    bool Found = false;
    s_map_t::iterator i;
    if(this->ItemMap->isEmpty() == false){
        i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            if(i.value() == Item){
                Found = true;
                break;
            }
            i++;
        }
    }
    int key = i.key();
    i++;
    this->ItemMap->remove(key);
    if(Found == false)
        return;
    if(Item->getPrev() != NULL)
        Item->getPrev()->setNext(Item->getNext());
    if(Item->getNext() != NULL)
        Item->getNext()->setPrev(Item->getPrev());
    QRect location;
    while(i != this->ItemMap->end()){
        location = i.value()->geometry();
        location.setY(location.y() - ITEM_HEIGTH);
        i.value()->setGeometry(location);
        i++;
    }
    Item->hide();
    return;
}

int LinkcGroupItem::getHeight(){
    s_map_t::iterator i;
    _height = GROUP_HEIGTH;
    if(this->ItemMap->isEmpty() == false){
        i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            if(i.value()->shown == 1){
                _height += ITEM_HEIGTH;
            }
            i++;
        }
    }
    return _height;
}
bool LinkcGroupItem::spreaded(){
    return shown;
}

int LinkcGroupItem::getVisibleHeight(){
    if(!this->spreaded())
        return GROUP_HEIGTH;
    s_map_t::iterator i;
    _height = GROUP_HEIGTH;
    if(this->ItemMap->isEmpty() == false){
        i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            if(i.value()->shown == 1){
                _height += ITEM_HEIGTH;
            }
            i++;
        }
    }
    return _height;
}

void LinkcGroupItem::hideItem(LinkcSubscribedItem *Item){
    bool Found = false;
    s_map_t::iterator i;
    if(this->ItemMap->isEmpty() == false){
        i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            if(i.value() == Item){
                Found = true;
                break;
            }
            i++;
        }
    }
    i++;
    if(Found == false)
        return;
    QRect location;
    while(i != this->ItemMap->end()){
        location = i.value()->geometry();
        location.setY(location.y() - ITEM_HEIGTH);
        i.value()->setGeometry(location);
        i++;
    }
    Item->setVisible(false);
    Item->shown = 0;
    return;
}

LinkcSubscribedItem* LinkcGroupItem::findItem(QString Id){
    s_map_t::iterator i;
    int a,b;
    string strA;
    string strB;
    char *idData = new char[256];
    memset(idData,0,256);
    memcpy(idData,Id.toUtf8().data(),Id.toUtf8().length());
    char *nodeData = nullptr;
    strB = Id.toUtf8().data();
    if(this->ItemMap->isEmpty() == false){
        i = this->ItemMap->begin();
        while(i != this->ItemMap->end()){
            nodeData = i.value()->getId();
            strA=nodeData;
            a = strA.find_first_of('/');
            b = strB.find_first_of('/');
            if(a>0 && b>0){
                if(a == b){
                    if(strcmp(idData,nodeData) == 0){
                        delete idData;
                        return i.value();
                    }
                }
            }else{
                if(strncmp(idData,nodeData,max(a,b)) == 0){
                    delete idData;
                    return i.value();
                }
            }
            i++;
        }
    }
    delete idData;
    return nullptr;
}


LinkcGroupSelect::LinkcGroupSelect(QWidget *parent):
    QWidget(parent){
    this->resize(GROUP_WIDGET_WIDTH,20);
    //this->setWindowFlags(Qt::FramelessWindowHint);
    GroupMap     = new g_map_t;
    this->setStyleSheet("background-color:white;");
    this->refreshSelect();
}

LinkcGroupSelect::~LinkcGroupSelect(){
    if(this->GroupMap->isEmpty() == false){
        g_map_t::iterator i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            delete i.value();
            i++;
        }
        this->GroupMap->clear();
    }
    delete GroupMap;
}

void LinkcGroupSelect::onGroupClicked(LinkcGroupItem *item, bool){
    if(this->GroupMap->isEmpty() == false){
        g_map_t::iterator i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            i.value()->clearClickedStatus();
            i++;
        }
    }
    this->refreshSize();
    emit this->groupClicked(item);
}

void LinkcGroupSelect::onItemClicked(LinkcGroupItem *group, LinkcSubscribedItem *item){
    g_map_t::iterator i;
    if(this->GroupMap->isEmpty() == false){
        i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            if(i.value() != group)
                i.value()->clearClickedStatus();
            i++;
        }
    }
    emit this->itemClicked(group,item);
}

void LinkcGroupSelect::onItemDoubleClicked(LinkcGroupItem *group, LinkcSubscribedItem *item){
    g_map_t::iterator i;
    if(this->GroupMap->isEmpty() == false){
        i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            if(i.value() != group)
                i.value()->clearClickedStatus();
            i++;
        }
    }
    emit this->itemDoubleClicked(group,item);
}

void LinkcGroupSelect::insertGroup(LinkcGroupItem *Item, int order){
    if(Item == nullptr)
        return;
    Item->setParent(this);
    if (order == -1)
        order = this->GroupMap->size();
    int h = 0;
    if(this->GroupMap->isEmpty() == false){
        g_map_t::iterator i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            h += GROUP_HEIGTH;
            if(i.value()->spreaded())
                h += i.value()->getSize()*ITEM_HEIGTH;
            i++;
        }
    }
    Item->setGeometry(0,h,GROUP_WIDGET_WIDTH,GROUP_HEIGTH+ITEM_HEIGTH*Item->getSize());
    this->GroupMap->insert(order,Item);
    h += Item->getSize()*ITEM_HEIGTH + GROUP_HEIGTH;

    this->resize(this->width(),h);
    this->connect(Item,SIGNAL(groupClicked(LinkcGroupItem*,bool)),this,SLOT(onGroupClicked(LinkcGroupItem*,bool)));
    this->connect(Item,SIGNAL(subscribedItemClicked(LinkcGroupItem*,LinkcSubscribedItem*)),this,SLOT(onItemClicked(LinkcGroupItem*,LinkcSubscribedItem*)));
    this->connect(Item,SIGNAL(subscribedItemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*)),this,SLOT(onItemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*)));
}

void LinkcGroupSelect::removeItemFromGroup(LinkcSubscribedItem *item, LinkcGroupItem *group){
    group->removeItem(item);
    g_map_t::iterator i;
    if(this->GroupMap->isEmpty() == false){
        i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            if(i.value() == group)
                break;
            i++;
        }
    }
    QRect location;
    i++;
    while(i != this->GroupMap->end()){
        location = i.value()->geometry();
        location.setY(location.y()-ITEM_HEIGTH);
        i.value()->setGeometry(location);
        i++;
    }
    return;
}

void LinkcGroupSelect::refreshSize(){
    g_map_t::iterator i;
    int _y = 0;
    if(this->GroupMap->isEmpty() == false){
        i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            i.value()->setGeometry(0,_y,this->width()-20,i.value()->getHeight());
            _y += i.value()->getVisibleHeight();
            i++;
        }
    }
    this->resize(this->width(),_y);
    return;
}

void LinkcGroupSelect::clearSelect(){
    if(this->GroupMap->isEmpty() == false){
        g_map_t::iterator i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            delete i.value();
            i++;
        }
        this->GroupMap->clear();
    }
}

void LinkcGroupSelect::refreshSelect(){
    this->clearSelect();
    return;
}

LinkcSubscribedItem* LinkcGroupSelect::findItem(QString Id){
    LinkcSubscribedItem *tmpItem = nullptr;
    if(this->GroupMap->isEmpty() == false){
        g_map_t::iterator i = this->GroupMap->begin();
        while(i != this->GroupMap->end()){
            tmpItem = i.value()->findItem(Id);
            if(tmpItem != nullptr)
                return tmpItem;
            i++;
        }
    }
    return nullptr;
}


LinkcChatDialog::LinkcChatDialog(QWidget *parent, gurgle *_core,gurgle_presence_t* _presence){
    this->setParent(parent);
    this->resize(400,400);
    QString Title = "<NULL> No connection";
    if(_presence != nullptr){
        this->presence = _presence;
        Title = _presence->id;
        Title.append(" [");
        Title.append(this->presence->status);
        Title.append("]");
    }
    if(_core != nullptr){
        this->core = _core;
        if(this->core->is_authenticated() == true)
            Title.append("Connected");
        else
            Title.append("No connection");
    }
    this->setWindowTitle(Title);

    this->Ui_History    = new QTextEdit(this);
    this->Ui_Input      = new QTextEdit(this);
    this->Ui_Send       = new QPushButton(this);
    this->Ui_Exit       = new QPushButton(this);

    this->Ui_History->setReadOnly(true);
    this->Ui_Send->setText(tr("Send"));
    this->Ui_Exit->setText(tr("Exit"));
    this->setMinimumSize(230,250);

    this->connect(this->Ui_Send,SIGNAL(clicked(bool)),this,SLOT(onSendClicked(bool)));
    this->connect(this->Ui_Exit,SIGNAL(clicked(bool)),this,SLOT(onExitClicked(bool)));
}

LinkcChatDialog::~LinkcChatDialog(){
    delete this->Ui_History;
    delete this->Ui_Input;
    delete this->Ui_Send;
    delete this->Ui_Exit;
}

void LinkcChatDialog::resizeEvent(QResizeEvent *){
    this->Ui_History->setGeometry(5,5,this->width()-10,this->height()-150);
    this->Ui_Input->setGeometry(5,this->Ui_History->height()+15,this->width()-10,85);
    this->Ui_Send->setGeometry(this->width()-100,this->height()-40,80,32);
    this->Ui_Exit->setGeometry(this->width()-200,this->height()-40,80,32);
}

void LinkcChatDialog::setCore(gurgle *_core){
    QString Title = "<NULL> No connection";
    if(_core == nullptr)
        return;
    if(this->presence != nullptr){
        Title = this->presence->id;
        Title.append(" [");
        Title.append(this->presence->status);
        Title.append("]");
    }
    if(_core != nullptr){
        this->core = _core;
        if(this->core->is_authenticated() == true)
            Title.append("Connected");
        else
            Title.append("No connection");
    }
    this->setWindowTitle(Title);
}


void LinkcChatDialog::onExitClicked(bool){
    this->close();
}

void LinkcChatDialog::onSendClicked(bool){
    if(this->Ui_Input->toPlainText() != ""){
        this->Ui_History->append("Me:");
        this->Ui_History->append(QString(" ").append(this->Ui_Input->toPlainText()));
        if(this->core != nullptr && this->presence != nullptr){
            this->core->forward_message(this->presence->id,this->Ui_Input->toPlainText().toUtf8().data());
        }
        this->Ui_Input->clear();
    }
}

void LinkcChatDialog::setMessage(QString str){
    if(this->presence != nullptr){
        this->Ui_History->append(QString(this->presence->id).append(":"));
        this->Ui_History->append(QString(" ").append(str));
    }else{
        this->Ui_History->append("Unkonwn: ");
        this->Ui_History->append(QString(" ").append(str));
    }
}


LinkcSubscribeDialog::LinkcSubscribeDialog(QWidget *parent, gurgle *_core){
    this->setParent(parent);
    if(_core != nullptr){
        this->core = _core;
    }
    this->Ui_AccpetButton   = new QPushButton(this);
    this->Ui_BackButton     = new QPushButton(this);
    this->Ui_IdEditor       = new QLineEdit(this);
    this->Ui_IdLabel        = new QLabel(this);

    this->Ui_IdLabel->setText(tr("Id goes here"));
    this->Ui_AccpetButton->setText(tr("Subscribe"));
    this->Ui_BackButton->setText(tr("Cancel"));

    this->connect(this->Ui_AccpetButton,SIGNAL(clicked(bool)),this,SLOT(onAcceptClicked(bool)));
    this->connect(this->Ui_BackButton,SIGNAL(clicked(bool)),this,SLOT(onBackClicked(bool)));

    this->setMaximumSize(700,80);
    this->resize(500,65);
    this->setWindowTitle(tr("Subscribe"));
}

LinkcSubscribeDialog::~LinkcSubscribeDialog(){
    delete this->Ui_AccpetButton;
    delete this->Ui_BackButton;
    delete this->Ui_IdEditor;
    delete this->Ui_IdLabel;
}

void LinkcSubscribeDialog::resizeEvent(QResizeEvent *){
    this->Ui_IdLabel->setGeometry(20,this->height()/2-13,100,26);
    this->Ui_IdEditor->setGeometry(120,this->height()/2-13,this->width()-300,26);
    this->Ui_AccpetButton->setGeometry(this->Ui_IdEditor->x()+this->Ui_IdEditor->width()+15,this->Ui_IdEditor->y(),65,26);
    this->Ui_BackButton->setGeometry(this->Ui_AccpetButton->x()+this->Ui_AccpetButton->width()+10,this->Ui_AccpetButton->y(),65,26);
}

void LinkcSubscribeDialog::setCore(gurgle *_core){
    if(_core == nullptr)
        return;
    this->core = _core;
}

void LinkcSubscribeDialog::onAcceptClicked(bool){
    if(this->Ui_IdEditor->text().isEmpty()){
        QMessageBox::warning(this,tr("Warning"),tr("Id can not be empty"));
        return;
    }
    if(this->core->update_roster(Ui_IdEditor->text().toUtf8().data(),nullptr,nullptr,SUBSCRIBE) == false){
        QMessageBox::warning(this,tr("Warning"),tr("Failed to subscribe"));
        return;
    }
    emit this->subscribeDone();
    this->close();
}

void LinkcSubscribeDialog::onBackClicked(bool){
    this->close();
}

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

void LinkcPresenceEdit::setContent(QString s){
    this->clear();
    this->setPlaceholderText(s);
    this->_hasContent = true;
    this->setCursorPosition(0);
    this->_Content = s;
    this->update();
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
    this->shown         = true;
    this->Font.setPixelSize(12);
    this->MainPresence->setFont(this->Font);
    this->Font.setPixelSize(10);
    this->Mood->setFont(this->Font);

    this->HeadIcon->setStyleSheet("background-color:yellow");
    this->MainPresence->setStyleSheet("background-color:red");

    this->MainPresence->setText("New 1");
    //this->Mood->setText("New 2");

    this->show();
    this->resize(GROUP_WIDGET_WIDTH,ITEM_HEIGTH);
}

LinkcSubscribedItem::~LinkcSubscribedItem(){
    delete this->MainPresence;
    delete this->Mood;
    delete this->HeadIcon;
}

void LinkcSubscribedItem::resizeEvent(QResizeEvent *){
    this->HeadIcon->setGeometry(0,0,this->height(),this->height());   // xx
    this->MainPresence->setGeometry(this->height(),0,this->width()-50,this->height());
}

void LinkcSubscribedItem::mousePressEvent(QMouseEvent *){
    this->setStyleSheet("background-color:#B0B0B0");
    emit this->clicked(this);
}

void LinkcSubscribedItem::unClicked(){
    this->setStyleSheet("background-color:#F9F8F6;");
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


LinkcGroupItem::LinkcGroupItem(QWidget *parent){
    this->setParent(parent);
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
    this->setStyleSheet("background-color:white;");

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
    if(this->ItemMap->size() == 0){
        Item->setPrev(NULL);
        Item->setNext(NULL);
        this->ItemMap->insert(order,Item);
        Item->setGeometry(ITEM_SPACING,GROUP_HEIGTH+(ItemMap->size()-1)*ITEM_HEIGTH,GROUP_WIDGET_WIDTH,ITEM_HEIGTH);
        this->resize(this->width(),this->height()+Item->getHeight());
        this->connect(Item,SIGNAL(clicked(LinkcSubscribedItem*)),this,SLOT(onItemClicked(LinkcSubscribedItem*)));
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
    this->connect(Item,SIGNAL(clicked(LinkcSubscribedItem*)),this,SLOT(onItemClicked(LinkcSubscribedItem*)));
    Item->show();
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


LinkcGroupSelect::LinkcGroupSelect(QWidget *parent):
    QWidget(parent){
    this->resize(GROUP_WIDGET_WIDTH,0);
    this->setWindowFlags(Qt::FramelessWindowHint);
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
            qDebug() << i.value()->getVisibleHeight();
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

void LinkcGroupSelect::insertGroup(LinkcGroupItem *Item, int order){
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
    Item->show();
    return;
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
            i.value()->setGeometry(0,_y,GROUP_WIDGET_WIDTH,i.value()->getHeight());
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


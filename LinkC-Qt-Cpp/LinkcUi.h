#ifndef LINKC_UI
#define LINKC_UI
#include <QLineEdit>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QFocusEvent>
#include <QFocusFrame>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <stdint.h>
#include "gurgle.h"

#define     ITEM_HEIGTH                     20
#define     GROUP_WIDGET_WIDTH              300
#define     GROUP_WIDGET_HEIGTH             450
#define     GROUP_HEIGTH                    20
#define     ITEM_SPACING                    15
#define     GROUP_SPACING                   15

class LinkcPresenceEdit:public QLineEdit{
    Q_OBJECT
public:
    LinkcPresenceEdit(QWidget* parent = 0);
    ~LinkcPresenceEdit(void);
    void    enterEvent(QEvent*);
    void    leaveEvent(QEvent*);
    void    setDefaultText(QString);
    void    setContent(QString,bool NoUpdating = false);
    bool    hasContent(void);
    void    clearContent(void);
signals:
    void    ContentUpdated(QString);
public slots:
    void    LeaveFocus(void);
private:
    bool    _hasContent;
    QString _defaultText;
    QString _Content;
};

class LinkcChatDialog : public QWidget{
    Q_OBJECT
public:
    explicit        LinkcChatDialog(QWidget *parent = 0, gurgle *_core = nullptr, gurgle_presence_t *_presence = nullptr);
                   ~LinkcChatDialog();
    void            resizeEvent(QResizeEvent*);
    void            setCore(gurgle *_core = nullptr);
    void            setPresence(gurgle_presence_t *_presence = nullptr);
    void            setMessage(QString);
public slots:
    void            onSendClicked(bool);
    void            onExitClicked(bool);
private:
    gurgle          *core;
    gurgle_presence_t *presence;
    QTextEdit       *Ui_History;
    QTextEdit       *Ui_Input;
    QPushButton     *Ui_Send;
    QPushButton     *Ui_Exit;
};

class LinkcSubscribeDialog : public QWidget{
    Q_OBJECT
public:
    explicit        LinkcSubscribeDialog(QWidget *parent = 0, gurgle *_core = nullptr);
                   ~LinkcSubscribeDialog();
    void            setCore(gurgle *_core = nullptr);
    void            resizeEvent(QResizeEvent *);
signals:
    void            subscribeDone(void);
public slots:
    void            onAcceptClicked(bool);
    void            onBackClicked(bool);
private:
    QLineEdit       *Ui_IdEditor;
    QPushButton     *Ui_BackButton;
    QPushButton     *Ui_AccpetButton;
    QLabel          *Ui_IdLabel;
    gurgle          *core;
};


class LinkcSubscribedItem : public QWidget{
    Q_OBJECT
public:
    explicit    LinkcSubscribedItem(QWidget *parent = 0,gurgle_subscription_t *info = NULL);
                ~LinkcSubscribedItem();
    void        mousePressEvent(QMouseEvent *);
    void        mouseDoubleClickEvent(QMouseEvent*);
    void        unClicked(void);
    void        setSInfo(gurgle_subscription_t *info = nullptr);
    void        resizeEvent(QResizeEvent *);
    LinkcSubscribedItem *getPrev(void);
    LinkcSubscribedItem *getNext(void);
    gurgle_subscription_t   *getInfo(void);
    void        setPrev(LinkcSubscribedItem*);
    void        setNext(LinkcSubscribedItem*);
    int         getHeight(void);
    char*       getId(void);
    uint8_t     Flag;
    bool        shown;
    bool        subShown;
    bool        doInfoSet;
    bool        chatDialogVisible;
    LinkcChatDialog *ChatDialog;
signals:
    void        clicked(LinkcSubscribedItem*);
    void        doubleclicked(LinkcSubscribedItem*);
public slots:
private:
    gurgle_subscription_t *Subscription;
    QLabel      *MainPresence;
    QLabel      *Mood;
    QLabel      *HeadIcon;
    QFont       Font;
    LinkcSubscribedItem *prev;
    LinkcSubscribedItem *next;
    int         _height;
    QPixmap     pix;
    QImage      image;
    QSize       pixSize;
};
typedef QMap<int,LinkcSubscribedItem*>   s_map_t;

class LinkcGroupItem : public QWidget{
    Q_OBJECT
public:
    explicit        LinkcGroupItem(QWidget *parent = 0);
                    ~LinkcGroupItem();
    void            setGroupName(QString name);
    void            InsertSubscribedItem(LinkcSubscribedItem* Item = nullptr,int order = -1);
    void            mousePressEvent(QMouseEvent *);
    void            clearClickedStatus();
    void            removeItem(LinkcSubscribedItem*);
    void            hideItem(LinkcSubscribedItem*);
    void            showItem(LinkcSubscribedItem*);
    int             getSize(void);
    int             getHeight(void);
    int             getVisibleHeight(void);
    bool            spreaded(void);
    void            setPrev(LinkcGroupItem*);
    void            setNext(LinkcGroupItem*);
    LinkcSubscribedItem* getNextSubscribedItem(LinkcSubscribedItem* Item = nullptr);
    LinkcSubscribedItem* findItem(QString Id);
signals:
    void            groupClicked(LinkcGroupItem*,bool spreaded);
    void            subscribedItemClicked(LinkcGroupItem*,LinkcSubscribedItem*);
    void            subscribedItemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*);
public slots:
    void            onItemClicked(LinkcSubscribedItem*);
    void            onItemDoubleClicked(LinkcSubscribedItem*);
private:
    QLabel          *Icon;
    QLabel          *Title;
    QFont            TitleFont;
    s_map_t         *ItemMap;
    bool             shown;
    QPixmap          pix;
    QImage           image;
    QSize            pixSize;
    int             _height;
    LinkcSubscribedItem  *prev;
    LinkcSubscribedItem  *next;
};
typedef QMap<int,LinkcGroupItem*>        g_map_t;

class LinkcGroupSelect : public QWidget{
    Q_OBJECT
public:
    explicit        LinkcGroupSelect(QWidget *parent = 0);
                    ~LinkcGroupSelect();
    void            insertGroup(LinkcGroupItem*,int order = -1);
    void            removeItemFromGroup(LinkcSubscribedItem*,LinkcGroupItem*);
    void            refreshSize(void);
    LinkcSubscribedItem* findItem(QString Id);
signals:
    void            groupClicked(LinkcGroupItem*);
    void            itemClicked(LinkcGroupItem*, LinkcSubscribedItem*);
    void            itemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*);
public slots:
    void            onGroupClicked(LinkcGroupItem*,bool);
    void            onItemClicked(LinkcGroupItem*, LinkcSubscribedItem*);
    void            onItemDoubleClicked(LinkcGroupItem*,LinkcSubscribedItem*);
    void            refreshSelect(void);
    void            clearSelect(void);
private:
    g_map_t         *GroupMap;
};
#endif // LINKC_UI


#ifndef GURGLE
#define GURGLE

#ifdef __WIN32__
#include <winsock2.h>
#elif _linux_
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#include <mutex>
#include <stdint.h>
#include <memory>
#include <stdlib.h>
#include <thread>
#include <string>

#include "rapidjson/include/rapidjson/document.h"
#include "rapidjson/include/rapidjson/writer.h"
#include "rapidjson/include/rapidjson/stringbuffer.h"
#include "rapidjson/include/rapidjson/rapidjson.h"
#include "rapidjson/include/rapidjson/prettywriter.h"

using namespace rapidjson;

#define GURGLE_CLIENT           1
#define GURGLE_VERSION          "Unusable"
#define GURGLE_LOG_LEVEL        3
#define GURGLE_LOG_MODE_ERROR   1
#define GURGLE_LOG_MODE_COMMON  2
#define GURGLE_LOG_MODE_DEBUG   3

class packageNode{
public:
    class packageNode*  nextNode;
    uint32_t            packageID;
    char                *data;
    packageNode(char* nodeData){
        packageID = 0;
        nextNode  = nullptr;
        this->data= nullptr;
        if (nodeData != nullptr){
            this->data = nodeData;
        }else{
            this->data =nullptr;
        }
    }
};

class packageList{
public:
    packageList(){
        this->size = 0;
        this->root = nullptr;
    }
    ~packageList(){
        if (this->root == nullptr)
            return;
        packageNode *curNode = this->root;
        packageNode *tmpNode;
        while (curNode->nextNode != nullptr){
            tmpNode = curNode;
            curNode = curNode->nextNode;
            delete tmpNode;
        }
        delete curNode;
        return;
    }
    void insert(char* newData,uint32_t packageID){
        packageNode *newNode = new packageNode(newData);
        newNode->packageID = packageID;
        if (this->root == nullptr){
            this->root = newNode;
            this->size += 1;
            return;
        }
        packageNode *tempNode = this->root;
        while (tempNode->nextNode != nullptr)
            tempNode = tempNode->nextNode;
        tempNode->nextNode = newNode;
        this->size += 1;
        return;
    }
    void clear(void){
        this->size = 0;
        this->root = nullptr;
    }

    char* get_data(uint32_t packageID){
        if (this->size == 0){
            return nullptr;
        }else{
            packageNode *tempNode = this->root;
            int i;
            for(i=0;i<this->size;i++){
                if (tempNode->packageID == packageID)
                    return tempNode->data;
                else
                    tempNode = tempNode->nextNode;
            }
        }
        return nullptr;
    }
    char* get_one(void){
        if (this->size == 0)
            return nullptr;
        packageNode *tempNode = this->root;
        this->root = this->root->nextNode;
        char *returnValue = new char[512];
        memset(returnValue,0,512);
        int len = strlen(tempNode->data);
        memcpy(returnValue,tempNode->data,strlen(tempNode->data));
        returnValue[len] = 0;
        delete tempNode->data;
        delete tempNode;
        this->size -= 1;
        return returnValue;
    }

    bool remove(uint32_t packageID){
        if (this->size == 0)
            return false;
        if (this->root->packageID == packageID){
            packageNode *tempNode = this->root->nextNode;
            delete this->root;
            this->size -= 1;
            this->root = tempNode;
            return true;
        }
        packageNode *curNode = this->root;
        while (curNode->nextNode != nullptr){
            if (curNode->nextNode->packageID == packageID){
                packageNode *tempNode = curNode->nextNode;
                curNode->nextNode = curNode->nextNode->nextNode;
                delete tempNode;
                this->size -= 1;
                return true;
            }else{
                curNode = curNode->nextNode;
            }
        }
        return false;
    }
    packageNode *get_root(void){
        return this->root;
    }
    int get_size(void){
        return this->size;
    }
    void print_list(void){
        packageNode *tempNode = this->root;
        while(tempNode != nullptr){
            printf ("%s",tempNode->data);
            tempNode = tempNode->nextNode;
        }
    }
protected:
    int         size;
    packageNode *root;
};

class door_lock{
public:
    door_lock(void){
        return;
    }
    ~door_lock(){
        return;
    }
    void door_open(void){
        this->lock.unlock();
        return;
    }
    void door_close(void){
        this->lock.lock();
        return;
    }
    void door_rush_into(void){
        this->lock.lock();
        this->lock.unlock();
        return;
    }
    void door_step_into(void){
        this->lock.lock();
    }
    void door_step_out(void){
        this->lock.unlock();
    }
private:
    std::mutex lock;
};


struct gurgle_id_t{
    char    protocol[16];
    char    username[32];
    char    domain[32];
    char    terminal[16];
};
typedef struct gurgle_id_t gurgle_id_t;

class gurgle{
public:
    gurgle(int mode = GURGLE_CLIENT);
    ~gurgle();
    char*       get_version(void);
    void        set_socket(int);
    int         get_socket(void);
    uint32_t    create_id(void);
    char*       create_random_str(size_t);
    char*       create_terminal_id(void);
    void        write_log(const char *log, int mode = GURGLE_LOG_MODE_COMMON, int level = 0);
    void        __recv_lock_release(void);
    int         gurgle_recv(char* buf, size_t buf_size = 512, uint32_t message_id = 0, int timeout = 5, int max_try = 2);
    int         gurgle_send(const char* buf, size_t len);
    void        set_remote_host(const char*);
    void        set_remote_prot(uint16_t);
    int         get_runtime_mode(void);
    char*       get_remote_host(void);
    uint16_t    get_remote_port(void);
    char*       get_auth_method(void);
    char*       get_roster_etag(void);
    char*       get_str_version(void);
    void        set_log_level(void);
    int         get_log_level(void);
    void        set_session_id(char* session);
    char*       get_session_id(void);
    char*       get_self_gurgle_id(void);
    bool        is_username_acceptable(char* username);
    bool        is_password_acceptable(char* password);
    gurgle_id_t analyse_full_id(const char*);
    char*       make_up_full_id(const char *username, const char *domain, const char *terminal = nullptr);
    bool        is_id_match(const char*,const char*);
    bool        is_remote_addr_set(void);
    // query roster
    // query roster update
    // push roster update
    bool        plain_password_auth(gurgle_id_t,const char* password);
    void        set_authenticated(bool);
    int         ping(void);
    bool        check_auth_method(const char*);
    bool        check_version(void);
    bool        query_self_presence(void);
    bool        is_connected(void);
    bool        is_authenticated(bool onlineCheck = false);
    bool        connect_to_server(const char *strDomain, uint16_t nPort, const char *session,int timeout = 5);
    int         publish_self_presence_update();
    int         forward_message();
    int         subscribe();
    int         unsubscribe();
    int         disconnect_from_remote(const char *reason=nullptr,uint32_t message_id=0);

    // C++ client Only methods


private:
    int         __runtime_mode;
    int         __gurgle_order;
    char        __gurgle_id[64];
    char        __gurgle_version[32];
    int         __log_level;
    bool        __is_authenticated;
    uint16_t    __remotePort;
    bool        __is_connected;
    int         __socket;
    int         __recv_roster;
    packageList *__package_list;
    char        __auth_method[32];
    char        __roster_etag[32];
    char        __remoteHost[32];
    char        __terminal_id[8];
    std::mutex  __send_mutex;
    std::mutex  __recv_mutex;
    door_lock * __recv_door_1;
    door_lock * __recv_door_2;
    char        __session[32];

    //C++ client Only vars
    rapidjson::Document global_document;
    rapidjson::Value    nullVar;
};

#endif // GURGLE


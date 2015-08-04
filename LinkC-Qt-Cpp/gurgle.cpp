#include "gurgle.h"
#include <QDebug>
#include <stdio.h>
#include <time.h>
#include <iostream>

using namespace std;

char passwordAllowed[]={'a','b','c','d','e','f','g','h','i','j','k','l','m',    \
                        'n','o','p','q','r','s','t','u','v','w','x','y','z',    \
                        'A','B','C','D','E','F','G','H','I','J','K','L','M',    \
                        'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',    \
                        '!','@','#','$','%','^','&','*','-','_','=','<','>',    \
                        '+','?','/','1','2','3','4','5','6','7','8','9','0'};

char usernameAllowed[]={'a','b','c','d','e','f','g','h','i','j','k','l','m',    \
                        'n','o','p','q','r','s','t','u','v','w','x','y','z',    \
                        'A','B','C','D','E','F','G','H','I','J','K','L','M',    \
                        'N','O','P','Q','R','S','T','U','V','W','X','Y','Z',    \
                        '+','-','_','1','2','3','4','5','6','7','8','9','0'};
char* strcut(const char* buf, const int from, const int to){
    int buf_len = strlen(buf);
    if (from > to)
        return nullptr;
    if (buf_len < to)
        return nullptr;
    int i;
    int distance = to-from+1;
    char *returnValue = new char[distance+2];
    memset(returnValue,0,distance+2);
    for(i=0;i<distance;i++)
        returnValue[i] = buf[from+i];
    returnValue[distance] = '\0';
    returnValue[distance+1] = '\0';
    return returnValue;
}

gurgle::gurgle(int mode){
    this->__runtime_mode        = mode;
    this->__is_authenticated    = false;
    this->__is_connected        = false;
    this->__remotePort          = 0;
    this->__socket              = 0;
    this->__recv_roster         = 0;
    this->__log_level           = 3;
    this->__package_list        = new packageList();
    this->__recv_door_1         = new door_lock();
    this->__recv_door_2         = new door_lock();
    this->__alias               = nullptr;
    this->__alias_size          = 0;
    memset(this->__gurgle_id    ,'\0', 64);
    memset(this->__terminal_id  ,'\0', 8);
    memset(this->__roster_etag  ,'\0', 32);
    memset(this->__session      ,'\0', 32);
    memset(this->__remoteHost   ,'\0', 32);
    memset(this->__auth_method  ,'\0', 32);
    sprintf(this->__auth_method   ,"plain_password");
    sprintf(this->__gurgle_version,GURGLE_VERSION);
    this->__recv_door_2->door_close();
    char log[64] = "Error starting gurgle";
    if (mode == GURGLE_CLIENT){
        sprintf(log,"Gurgle version %s initlalized as Client",this->get_version());
    }
    this->write_log(log);
}

gurgle::~gurgle(){
    this->write_log("Gurgle is deleting....");
    if (this->is_connected())
        this->disconnect_from_remote("Porgram was terminated");
    if (this->__package_list != nullptr)
        delete this->__package_list;
    if (this->__recv_door_1 != nullptr)
        delete this->__recv_door_1;
    if (this->__recv_door_2 != nullptr)
        delete this->__recv_door_2;
}

void gurgle::write_log(const char* log,int mode, int level){
    if (mode == 0)
        mode = GURGLE_LOG_MODE_COMMON;
    if (level == 0)
        level = this->get_log_level();
    if (mode > level)
        return;
    time_t current_time;
    current_time = time(NULL);
    struct  tm *tblock;
    tblock = localtime(&current_time);
    char* str_time = asctime(tblock);
    str_time[strlen(str_time)-1] = 0;
    if (mode == GURGLE_LOG_MODE_ERROR)
        fprintf(stderr,"[%s] : %s\n",str_time,log);
    else if (mode == GURGLE_LOG_MODE_COMMON)
        fprintf(stdout,"[%s] : %s\n",str_time,log);
    else
        fprintf(stdout,"[%s] : %s\n",str_time,log);
    return;
}

bool gurgle::is_connected(){
    return this->__is_connected;
}

int gurgle::get_log_level(){
    return this->__log_level;
}

char* gurgle::get_version(){
    return this->__gurgle_version;
}

int gurgle::disconnect_from_remote(const char *reason, uint32_t message_id){
    if (this->is_connected() == false){
        this->write_log("You have not connected to remote!",GURGLE_LOG_MODE_ERROR);
        return -1;
    }
    if (this->get_runtime_mode() != GURGLE_CLIENT){
        this->write_log("Such runtime mode has not been unsupported",GURGLE_LOG_MODE_ERROR);
        return -1;
    }
    char *_reason = new char[512];
    memset(_reason,0,512);
    if(reason == nullptr)
        sprintf(_reason,"Unknown reason");
    else
        strcpy(_reason,reason);
    rapidjson::Value send_data;
    rapidjson::Value params;
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    send_data.SetObject();
    params.SetObject();
    send_data.AddMember("id",message_id,d.GetAllocator());
    send_data.AddMember("cmd","quit",d.GetAllocator());
    params.AddMember("reason",StringRef(_reason),d.GetAllocator());
    send_data.AddMember("params",params,d.GetAllocator());
    send_data.Accept(writer);
    this->gurgle_send(sb.GetString(),sb.GetSize());
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(this->gurgle_recv(recv_buf,512,0)){
        d.Parse(recv_buf);
        if(d.HasMember("reply")){
            char *log = new char[512];
            memset(log,0,512);
            sprintf(log,"Server replied [%s]",d["reply"].GetString());
            this->write_log(log);
            delete log;
        }else{
            this->write_log("There's some errors ,Just quit");
        }
    }else{
        this->write_log("Connection was close by peer");
    }
    delete recv_buf;
    return 0;
#ifdef WIN32
    this->__is_connected = false;
    this->__is_authenticated = false;
    closesocket(this->__socket);
#endif
    return 0;
}

uint32_t gurgle::create_id(){
    return rand()%2147483647;
}

char* gurgle::create_random_str(size_t length){
    if (length == 0)
        return nullptr;
    int i;
    char* returnValue = new char[length+1];
    for(i=0;i<(int)length;i++)
        returnValue[i] = rand()%52 + 'a';
    returnValue[length] = 0;
    return returnValue;
}

char* gurgle::create_terminal_id(){
    return this->create_random_str(8);
}

void gurgle::__recv_lock_release(){
    this->__recv_mutex.unlock();
    this->__recv_door_1->door_close();
    this->__recv_door_2->door_open();
    while (this->__recv_roster)
        this->__recv_door_2->door_step_into();
    this->__recv_door_2->door_close();
    this->__recv_door_1->door_open();

}

int gurgle::gurgle_recv(char *buf, size_t buf_size, uint32_t message_id, const char *message_type, int timeout, int max_try){
    if (buf == nullptr)
        return -1;
    if (this->is_connected() == false)
        return -1;
    memset(buf,0,buf_size);
    int maxlen = 0;
    char *tempBuf = nullptr;
    while(1){
        this->__recv_door_1->door_rush_into();
        if (this->__recv_mutex.try_lock())
            break;
        this->__recv_roster += 1;
        this->__recv_door_2->door_step_into();
        tempBuf = this->__package_list->get_data(message_id,message_type);
        this->__recv_roster -= 1;
        this->__recv_door_2->door_step_out();
        if (tempBuf != nullptr){
            maxlen = max(buf_size,strlen(tempBuf));
            memcpy(buf,tempBuf,max(buf_size,strlen(tempBuf)));
            this->__package_list->remove(message_id,message_type);
            tempBuf = nullptr;
            return maxlen;
        }
        if(timeout == 0)
            return 0;
    }
    this->__recv_door_2->door_open();
    if (this->is_connected() == false){
        this->__recv_lock_release();
        this->write_log("Connection has not been established!",GURGLE_LOG_MODE_ERROR);
        return -1;
    }
    if(timeout != 0){
#ifdef __WIN32__
        int _timeout = timeout * 1000;
        if (SOCKET_ERROR ==  setsockopt(this->__socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&_timeout, sizeof(int))){
#elif __linux__
        struct timeval _timeout;
        _timeout.tv_sec = timeout;
        _timeout.tv_usec = 0;
        if (SOCKET_ERROR ==  setsockopt(this->__socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&_timeout, sizeof(struct timeval))){
#endif
            this->__recv_lock_release();
            this->write_log("Set Ser_RecTIMEO error ",GURGLE_LOG_MODE_ERROR);
            return -1;
        }
    }else{
        tempBuf = this->__package_list->get_data(message_id,message_type);
        if (tempBuf != nullptr){
            maxlen = max(buf_size,strlen(tempBuf));
            memcpy(buf,tempBuf,max(buf_size,strlen(tempBuf)));
            this->__package_list->remove(message_id);
            tempBuf = nullptr;
            this->__recv_lock_release();
            return maxlen;
        }
#ifdef __WIN32__
        int _timeout = 100;
        if (SOCKET_ERROR ==  setsockopt(this->__socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&_timeout, sizeof(int))){
#elif __linux__
        struct timeval _timeout;
        _timeout.tv_sec = 0;
        _timeout.tv_usec = 100;
        if (SOCKET_ERROR ==  setsockopt(this->__socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&_timeout, sizeof(struct timeval))){
#endif
            this->__recv_lock_release();
            this->write_log("Set Ser_RecTIMEO error ",GURGLE_LOG_MODE_ERROR);
            return -1;
        }
    }
    int recv_size = 0;
    int n_try = 0;
    char* recvBuf = new char[buf_size];
    char *veryTempStr = nullptr;
    packageList *tempList = new packageList();
    int start       = 0;
    int location    = 0;
    int count       = 0;
    int id          = 0;
    Document        recv_json;
    char *return_buf = nullptr;
    char *json_obj = new char[128];
    bool    is_obj_null = true;
    while (n_try < max_try){
        tempBuf = this->__package_list->get_data(message_id,message_type);
        if (tempBuf != nullptr){
            maxlen = max(buf_size,strlen(tempBuf));
            memcpy(buf,tempBuf,max(buf_size,strlen(tempBuf)));
            this->__package_list->remove(message_id);
            tempBuf = nullptr;
            this->__recv_lock_release();
            delete json_obj;
            return maxlen;
        }
        memset(recvBuf,0,buf_size);
        recv_size = recv(this->__socket,recvBuf,buf_size,0);
        if (recv_size == -1){
            n_try += 1;
            continue;
        }else if(recv_size < 0){
            this->write_log("Recv error",GURGLE_LOG_MODE_ERROR);
            this->__recv_lock_release();
            delete json_obj;
            return -1;
        }else if(recv_size == 0){
            this->write_log("Connection was closed by peer");
            this->__is_authenticated= false;
            this->__is_connected    = false;
            this->__recv_lock_release();
            delete json_obj;
            return 0;
        }
        start = 0;
        location = 0;
        count = 0;
        for (location=0;location<recv_size;location++){
            if (recvBuf[location] == '{'){
                if (count == 0)
                    start = location;
                count += 1;
            }else if(recvBuf[location] == '}'){
                count -= 1;
                if (count == 0){
                    veryTempStr = strcut(recvBuf,start,location);
                    tempList->insert(veryTempStr,0);
                }
            }
        }
        veryTempStr = nullptr;
        while(true){
            tempBuf = tempList->get_one();
            if(tempBuf == nullptr)
                break;
            recv_json.Parse(StringRef(tempBuf));
            if(recv_json.HasMember("id"))
                id = recv_json["id"].GetInt();
            else
                id = 0;
            if (recv_json.HasMember("cmd")){
                if (strcmp(recv_json["cmd"].GetString(),"kill") == 0){
                    char *error  = new char[128];
                    char *reason = new char[128];
                    memset(error,0,128);
                    memset(reason,0,128);
                    if(recv_json["params"].IsObject()){
                        if(recv_json["params"].HasMember("error"))
                            sprintf(error,"%s",recv_json["params"]["error"].GetString());
                        else
                            sprintf(error,"NULL");
                        if(recv_json["params"].HasMember("reason"))
                            sprintf(reason,"%s",recv_json["params"]["reason"].GetString());
                        else
                            sprintf(reason,"NULL");
                    }else{
                        sprintf(error,"NULL");
                        sprintf(reason,"NULL");
                    }
                    char *log = new char[512];
                    memset(log,0,512);
                    sprintf(log,"Connection was closed [%s,%s]",error,reason);
                    this->write_log(log,GURGLE_LOG_MODE_ERROR);
                    delete error;
                    delete reason;
                    delete log;
                    this->__is_connected = false;
                    this->__is_authenticated = false;
                    this->__recv_lock_release();
                    delete json_obj;
                    return -1;
                }
            }
            is_obj_null = true;
            if (recv_json.HasMember("obj")){
                if(recv_json["obj"].IsString()){
                    memset(json_obj,0,128);
                    strcpy(json_obj,StringRef(recv_json["obj"].GetString()));
                    is_obj_null = false;
                }
            }
            if (message_id == 0){
                if(return_buf != nullptr){
                    if(is_obj_null)
                        this->__package_list->insert(tempBuf,id,nullptr);
                    else
                        this->__package_list->insert(tempBuf,id,json_obj);
                }else{
                    if(is_obj_null){
                        if(message_type == nullptr)
                            return_buf = tempBuf;
                        else
                            this->__package_list->insert(tempBuf,id,nullptr);
                    }else{
                        if(message_type != nullptr){
                            if(strcmp(json_obj,message_type) == 0){
                                return_buf = tempBuf;
                                continue;
                            }
                        }
                        this->__package_list->insert(tempBuf,id,json_obj);
                    }
                    continue;
                }
            }else if((int)message_id == id){
                if(return_buf != nullptr)
                    this->write_log("WHAT HAPPEND?");
                return_buf = tempBuf;
                continue;
            }else{
                if(is_obj_null)
                    this->__package_list->insert(tempBuf,id,nullptr);
                else
                    this->__package_list->insert(tempBuf,id,json_obj);
            }
        }
        if(return_buf != nullptr){
            int len = strlen(return_buf);
            if (len > (int)buf_size){
                this->write_log("Buffer overload");
                delete tempList;
                this->__recv_lock_release();
                delete json_obj;
                return 0;
            }
            memcpy(buf,return_buf,len);
            buf[len] = 0;
            delete tempList;
            this->__recv_lock_release();
            delete json_obj;
            return len;
        }
    }
    this->__recv_lock_release();
    delete json_obj;
    return 0;
}

int gurgle::gurgle_send(const char *buf, size_t len){
    if (this->is_connected() == false)
        return -1;
    this->__send_mutex.lock();
    int bit = send(this->__socket,(const char*)buf,len,0);
    if(bit == (int)len){
        this->__send_mutex.unlock();
        return bit;
    }else{
        this->write_log("Overbuf --=-=-=-=-=-=");
        this->__send_mutex.unlock();
        return -1;
    }
}

void gurgle::set_remote_host(const char *strDomain){
    memset(this->__remoteHost,0,32);
    memcpy(this->__remoteHost,strDomain,strlen(strDomain));
    this->__remoteHost[strlen(strDomain)] = 0;
    return;
}

void gurgle::set_remote_prot(uint16_t nPort){
    this->__remotePort = nPort;
    return;
}

bool gurgle::is_remote_addr_set(void){
    if (strcmp(this->__remoteHost,"")==0)
        return false;
    if (this->__remotePort == 0)
        return false;
    return true;
}

int gurgle::get_runtime_mode(void){
    return this->__runtime_mode;
}

int gurgle::get_socket(void){
    return this->__socket;
}

void gurgle::set_socket(int sockfd){
    this->__socket = sockfd;
    this->__is_connected = true;
}

char* gurgle::get_remote_host(void){
    return this->__remoteHost;
}

uint16_t gurgle::get_remote_port(void){
    return this->__remotePort;
}

char* gurgle::get_auth_method(void){
    return this->__auth_method;
}

bool gurgle::connect_to_server(const char *strDomain, uint16_t nPort, const char *session = nullptr,int timeout){
    if (this->is_connected()){
        this->write_log("You have already connected to remote,You should disconnect first");
        return true;
    }
    if (strDomain != nullptr)
        this->set_remote_host(strDomain);
    if (nPort != 0)
        this->set_remote_prot(nPort);
    if (this->is_remote_addr_set() == false){
        this->write_log("Remote address has not been set");
        return false;
    }
    if (this->get_runtime_mode() != GURGLE_CLIENT){
        this->write_log("Unsupported runtime mode");
        return false;
    }
#ifdef __WIN32__
    WSADATA wsadata;
    if(WSAStartup(MAKEWORD(1,1),&wsadata)==SOCKET_ERROR){
        this->write_log("WSAStartup() Failed");
        return false;
    }
#endif
    this->__socket = socket(AF_INET,SOCK_STREAM,0);
    if (this->__socket == -1){
        this->write_log("Cannot create socket",GURGLE_LOG_MODE_ERROR);
        return false;
    }
    struct sockaddr_in serverAddr;
    memset(&serverAddr,0,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(this->__remotePort);
    uint32_t ip_32bit = inet_addr(this->__remoteHost);
    if (ip_32bit != INADDR_NONE){
        serverAddr.sin_addr.s_addr = ip_32bit;
    }else{
        struct hostent *host = gethostbyname(this->__remoteHost);
        if(host == NULL)
            return false;
        if(host->h_length != 4){
            this->write_log("Currently we do not support IPv6");
            return false;
        }
        char *ip = inet_ntoa(*(struct in_addr *)*host->h_addr_list);
        ip_32bit = inet_addr(ip);
        if(ip_32bit == INADDR_NONE){
            this->write_log("We can't analysis this addr");
            delete ip;
            return false;
        }
#ifdef __WIN32__
        delete ip;
#endif
        serverAddr.sin_addr.s_addr = ip_32bit;
    }
    if(timeout != 0){
#ifdef __WIN32__
        int _timeout = timeout * 1000;
        if (SOCKET_ERROR ==  setsockopt(this->__socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&_timeout, sizeof(int))){
#elif __linux__
        struct timeval _timeout = {timeout,0};
        if (SOCKET_ERROR ==  setsockopt(this->__socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&_timeout, sizeof(struct timeval))){
#endif
            this->__recv_lock_release();
            this->write_log("Set Ser_RecTIMEO error ",GURGLE_LOG_MODE_ERROR);
            return -1;
        }
    }
    if(connect(this->__socket,(struct sockaddr*)&serverAddr,sizeof(serverAddr)) == -1){
        this->write_log("Cannot connect to remote",GURGLE_LOG_MODE_ERROR);
        return false;
    }
    this->__is_connected = true;
    if(!this->check_version()){
        this->write_log("Version do not match!");
        this->__is_connected = false;
        this->disconnect_from_remote();
        return false;
    }
    this->__alias = this->query_server_alias(this->__alias_size);
    rapidjson::Value send_json(kObjectType);
    rapidjson::Value params;
    rapidjson::Document d;
    StringBuffer sb;
    Writer<StringBuffer> writer(sb);
    int message_id = this->create_id();
    params.SetObject();
    send_json.SetObject();
    params.AddMember("protocol","gurgle",d.GetAllocator());
    params.AddMember("version",StringRef(this->get_str_version()),d.GetAllocator());
    params.AddMember("encript","disabled",d.GetAllocator());
    if(session != nullptr)
        params.AddMember("session",StringRef(session),d.GetAllocator());
    send_json.AddMember("id",message_id,d.GetAllocator());
    send_json.AddMember("cmd","connect",d.GetAllocator());
    send_json.AddMember("obj","session",d.GetAllocator());
    send_json.AddMember("params",params,d.GetAllocator());
    send_json.Accept(writer);
    this->gurgle_send(sb.GetString(),sb.GetSize());
    char *tempBuf = new char[512];
    memset(tempBuf,0,512);
    this->gurgle_recv(tempBuf,512,message_id);
    d.Parse(tempBuf);
    if(d.HasMember("reply")){
        if(d["reply"].HasMember("status")){
            if(strcmp(d["reply"]["status"].GetString(),"connection established"))
                    this->__is_connected = true;
            else
                    this->disconnect_from_remote();
            this->write_log(d["reply"]["status"].GetString());
        }else{
            this->write_log("Bad reply");
            this->__is_connected = false;
            return false;
        }
    }else{
        this->write_log("Bad reply");
        this->__is_connected = false;
        return false;
    }
    return true;
}

char* gurgle::get_str_version(){
    char *returnVar = new char[16];
    memset(returnVar,0,16);
    strcpy(returnVar,"Unusable");
    return returnVar;
}

bool gurgle::check_version(){
    rapidjson::Value senddata;
    senddata.SetObject();
    int message_id = this->create_id();
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd","query",this->global_document.GetAllocator());
    senddata.AddMember("obj","version",this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize())){
        this->write_log("Failed to send",GURGLE_LOG_MODE_ERROR);
        return false;
    }
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id)){
        this->write_log("Failed to recv",GURGLE_LOG_MODE_ERROR);
        return -1;
    }
    d.Parse(StringRef(recv_buf));
    if(d.HasMember("reply"))
        if(d["reply"].HasMember("version"))
            if(d["reply"]["version"].IsString())
                if(strcmp(this->get_str_version(),d["reply"]["version"].GetString()) == 0)
                    return true;
    return false;
}

void gurgle::set_session_id(char *session){
    memset(this->__session,0,32);
    memcpy(this->__session,session,32);
    this->__session[31] = 0;
}

char* gurgle::get_session_id(){
    char *returnVar = new char[32];
    memset(returnVar,0,32);
    memcpy(returnVar,this->__session,32);
    return returnVar;
}

char* gurgle::get_self_gurgle_id(){
    char *returnVar = new char[64];
    memset(returnVar,0,64);
    memcpy(returnVar,this->__gurgle_id,64);
    return returnVar;
}

bool gurgle::is_username_acceptable(char *username){
    if(username == nullptr)
        return false;
    int len = strlen(username);
    int AllowedListLen =strlen(usernameAllowed);
    if(len > 32)
        return false;
    int i,t;
    bool isFound;
    for(i=0;i<len;i++){
        isFound = false;
        for(t=0;t<AllowedListLen;t++){
            if(username[i] == usernameAllowed[t]){
                isFound = true;
                break;
            }
        }
        if (isFound == false)
            return false;
    }
    return true;
}

bool gurgle::is_password_acceptable(char *password){
    if(password == nullptr)
        return false;
    int len = strlen(password);
    int AllowedListLen =strlen(passwordAllowed);
    if(len > 32)
        return false;
    int i,t;
    bool isFound;
    for(i=0;i<len;i++){
        isFound = false;
        for(t=0;t<AllowedListLen;t++){
            if(password[i] == passwordAllowed[t]){
                isFound = true;
                break;
            }
        }
        if (isFound == false)
            return false;
    }
    return true;
}


gurgle_id_t gurgle::analyse_full_id(const char *_ID){
    gurgle_id_t id_struct;
    memset((void*)&id_struct,0,sizeof(id_struct));
    if (_ID == nullptr)
        return id_struct;
    string FullSignInID = _ID;
    int location = FullSignInID.find_first_of(':');
    if(!location > 0)
        return id_struct;
    string protocol = FullSignInID.substr(0,location);
    FullSignInID = FullSignInID.substr(location+1,FullSignInID.length()-location-1);
    location = FullSignInID.find_first_of('@');
    if(!location > 0)
        return id_struct;
    string username = FullSignInID.substr(0,location);
    FullSignInID = FullSignInID.substr(location+1,FullSignInID.length()-location-1);
    location = FullSignInID.find_first_of('/');
    string terminal;
    if(location > 0){
        terminal = FullSignInID.substr(location+1,FullSignInID.length()-location-1);
        FullSignInID = FullSignInID.substr(0,location);
    }
    if(FullSignInID == "")
        return id_struct;
    if(protocol == "")
        return id_struct;
    if(username == "")
        return id_struct;
    strncpy(id_struct.protocol,protocol.data(),protocol.length());
    strncpy(id_struct.username,username.data(),username.length());
    strncpy(id_struct.domain,FullSignInID.data(),FullSignInID.length());
    if(terminal != "")
        strncpy(id_struct.terminal,terminal.data(),terminal.length());
    return id_struct;
}

char *gurgle::make_up_full_id(const char *username, const char *domain, const char *terminal){
    if(username == nullptr || domain == nullptr)
        return nullptr;
    char  *id = new char[64];
    memset(id,0,64);
    if (terminal != nullptr && strcmp(terminal,"") != 0)
        sprintf(id,"grgl:%s@%s/%s",username,domain,terminal);
    else
        sprintf(id,"grgl:%s@%s",username,domain);
    return id;
}

bool gurgle::is_authenticated(bool onlineCheck){
    if(!onlineCheck)
        return this->__is_authenticated;
    rapidjson::Value senddata;
    senddata.SetObject();
    int message_id = this->create_id();
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd","query",this->global_document.GetAllocator());
    senddata.AddMember("obj","auth_status",this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return false;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return false;
    d.Parse(recv_buf);
    delete recv_buf;
    recv_buf = nullptr;
    if(d.HasMember("reply"))
        if(d["reply"].HasMember("auth_status"))
            if(d["reply"]["auth_status"].IsString())
                if(strcmp(d["reply"]["auth_status"].GetString(),"Authenticated") == 0){
                    this->set_authenticated(true);
                    return true;
                }
    this->set_authenticated(false);
    return false;
}

void gurgle::set_authenticated(bool auth_status){
    this->__is_authenticated = auth_status;
}

bool gurgle::plain_password_auth(gurgle_id_t id, const char *password){
    if(this->is_authenticated(true))
        return true;
    if(password == nullptr)
        return false;
    if(strcmp(id.username,"") == 0)
        return false;
    int message_id = this->create_id();
    char *grgl_id = this->make_up_full_id(id.username,id.domain,id.terminal);
    if(grgl_id == nullptr)
        return false;
    rapidjson::Value senddata;
    rapidjson::Value params;
    senddata.SetObject();
    params.SetObject();
    params.AddMember("method",StringRef(this->get_auth_method()),this->global_document.GetAllocator());
    params.AddMember("password",StringRef(password),this->global_document.GetAllocator());
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd","auth",this->global_document.GetAllocator());
    senddata.AddMember("from",StringRef(grgl_id),this->global_document.GetAllocator());
    senddata.AddMember("params",params,this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return false;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return false;
    d.Parse(recv_buf);
    char *log = new char[512];
    memset(log,0,512);
    //delete recv_buf;
    //recv_buf = nullptr;
    if(d.HasMember("reply")){
        if(d["reply"].HasMember("error")){
            if(d["reply"]["error"].IsString()){
                sprintf(log,"Authenticate Error [%s]",d["reply"]["error"].GetString());
                this->set_authenticated(false);
            }else if(d["reply"]["error"].IsNull()){
                if(d.HasMember("to")){
                    if(d["to"].IsString()){
                        memset(this->__gurgle_id,0,64);
                        strncpy(this->__gurgle_id,d["to"].GetString(),d["to"].GetStringLength());
                        this->set_authenticated(true);
                    }else{
                        sprintf(log,"I have no ID?");
                    }
                }else{
                    sprintf(log,"I have no ID?");
                }
            }else{
                sprintf(log,"Bad reply");
            }
        }else{
            sprintf(log,"Bad reply");
        }
    }else{
        sprintf(log,"Bad reply");
    }
    if(this->is_authenticated()){
        delete log;
        return true;
    }else{
        this->write_log(log,GURGLE_LOG_MODE_ERROR);
        return false;
    }

}

gurgle_presence_t* gurgle::query_presence(void){
    gurgle_presence_t *presence = nullptr;
    if(this->is_authenticated(true)==false)
        return nullptr;
    rapidjson::Value senddata;
    rapidjson::Value params;
    senddata.SetObject();
    params.SetObject();
    int message_id = this->create_id();
    params.AddMember("gid",StringRef(this->__gurgle_id),this->global_document.GetAllocator());
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd",StringRef("query"),this->global_document.GetAllocator());
    senddata.AddMember("obj",StringRef("presence"),this->global_document.GetAllocator());
    senddata.AddMember("params",params,this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return nullptr;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return nullptr;
    d.Parse(recv_buf);
    char *log = new char[512];
    memset(log,0,512);
    presence = new gurgle_presence_t;
    memset(presence,0,sizeof(gurgle_presence_t));
    delete recv_buf;
    if(d.HasMember("reply")){
        params = d["reply"];
        if(params.HasMember("last_name"))
            if(params["last_name"].IsString())
                strncpy(presence->last_name,params["last_name"].GetString(),params["last_name"].GetStringLength());
        if(params.HasMember("first_name"))
            if(params["first_name"].IsString())
                strncpy(presence->first_name,params["first_name"].GetString(),params["first_name"].GetStringLength());
        if(params.HasMember("status"))
            if(params["status"].IsString())
                strncpy(presence->status,params["status"].GetString(),params["status"].GetStringLength());
        if(params.HasMember("mood"))
            if(params["mood"].IsString())
                strncpy(presence->mood,params["mood"].GetString(),params["mood"].GetStringLength());
        if(params.HasMember("error"))
            if(params["error"].IsString()){
                memset(log,0,512);
                sprintf(log,"Error %s",params["error"].GetString());
                this->write_log(log);
                if(params.HasMember("reason"))
                    if(params["reason"].IsString()){
                        memset(log,0,512);
                        sprintf(log,"Reason %s",params["reason"].GetString());
                        this->write_log(log);
                    }
                delete presence;
                delete log;
                return nullptr;
            }
    }
    delete log;
    return presence;
}

gurgle_subscription_t* gurgle::query_roster(int &size){
    if(!this->is_authenticated())
        return nullptr;
    gurgle_subscription_t *p;
    size = 0;
    rapidjson::Value senddata;
    rapidjson::Value params;
    senddata.SetObject();
    params.SetObject();
    int message_id = this->create_id();
    params.AddMember("limit",100,this->global_document.GetAllocator());
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd",StringRef("query"),this->global_document.GetAllocator());
    senddata.AddMember("obj",StringRef("roster"),this->global_document.GetAllocator());
    senddata.AddMember("params",params,this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return nullptr;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return nullptr;
    d.Parse(recv_buf);
    char *log = new char[512];
    memset(log,0,512);
    if(d.HasMember("reply")){
        if(d["reply"].HasMember("error")){
            if(d["reply"]["error"].IsNull() == false){
                sprintf(log,"Error : %s",d["reply"]["error"].GetString());
                this->write_log(log);
                delete log;
                delete recv_buf;
                return nullptr;
            }
        }
        if(d["reply"].HasMember("value")){
            if(d["reply"]["value"].IsArray() == false){
                sprintf(log,"Bad reply");
                this->write_log(log);
                delete log;
                delete recv_buf;
                return nullptr;
            }
        }
        size = d["reply"]["value"].Size();
        p = new gurgle_subscription_t[size];
        memset(p,0,size*sizeof(gurgle_subscription_t));
        int i;
        for(i=0;i<size;i++){
            if(d["reply"]["value"][i][0].IsString())
                memcpy(p[i].presence.id,        d["reply"]["value"][i][0].GetString(),d["reply"]["value"][i][0].GetStringLength());
            if(d["reply"]["value"][i][1].IsString())
                memcpy(p[i].nickname,           d["reply"]["value"][i][1].GetString(),d["reply"]["value"][i][1].GetStringLength());
            if(d["reply"]["value"][i][2].IsString())
                memcpy(p[i].group,              d["reply"]["value"][i][2].GetString(),d["reply"]["value"][i][2].GetStringLength());
            if(d["reply"]["value"][i][3].IsBool())
                p[i].sub_from   =               d["reply"]["value"][i][3].GetBool();
            if(d["reply"]["value"][i][4].IsBool())
                p[i].sub_to     =               d["reply"]["value"][i][4].GetBool();
            if(d["reply"]["value"][i][5].IsString())
                memcpy(p[i].presence.last_name, d["reply"]["value"][i][5].GetString(),d["reply"]["value"][i][5].GetStringLength());
            if(d["reply"]["value"][i][6].IsString())
                memcpy(p[i].presence.first_name,d["reply"]["value"][i][6].GetString(),d["reply"]["value"][i][6].GetStringLength());
            if(d["reply"]["value"][i][7].IsString())
                memcpy(p[i].presence.status,    d["reply"]["value"][i][7].GetString(),d["reply"]["value"][i][7].GetStringLength());
            if(d["reply"]["value"][i][8].IsString())
                memcpy(p[i].presence.mood,      d["reply"]["value"][i][8].GetString(),d["reply"]["value"][i][8].GetStringLength());
        }
    }
    delete recv_buf;
    return p;
}

bool gurgle::publish_self_presence_update(gurgle_presence_t *presence){
    if(presence == nullptr)
        return false;
    rapidjson::Value senddata;
    rapidjson::Value params;
    senddata.SetObject();
    params.SetObject();
    int message_id = this->create_id();
    if(strcmp(presence->first_name  ,"") != 0)
        params.AddMember("first_name",StringRef(presence->first_name),this->global_document.GetAllocator());
    if(strcmp(presence->last_name   ,"") != 0)
        params.AddMember("last_name",StringRef(presence->last_name),this->global_document.GetAllocator());
    if(strcmp(presence->mood        ,"") != 0)
        params.AddMember("mood",StringRef(presence->mood),this->global_document.GetAllocator());
    if(strcmp(presence->status      ,"") != 0)
        params.AddMember("status",StringRef(presence->status),this->global_document.GetAllocator());
    if(params.IsNull())
        return false;
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd",StringRef("push"),this->global_document.GetAllocator());
    senddata.AddMember("obj",StringRef("presence"),this->global_document.GetAllocator());
    senddata.AddMember("params",params,this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    this->write_log(sb.GetString());
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return nullptr;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return nullptr;
    d.Parse(recv_buf);
    if(d.HasMember("reply")){
        if(d["reply"].HasMember("error")){
            if(d["reply"]["error"].IsNull()){
                delete recv_buf;
                return true;
            }else{
                this->write_log(d["reply"]["error"].GetString());
                delete recv_buf;
                return false;
            }
        }
    }
    delete recv_buf;
    return false;
}

bool gurgle::forward_message(char* UserId, char* Message){
    if(UserId == nullptr || Message == nullptr){
        return false;
    }
    rapidjson::Value senddata;
    rapidjson::Value params;
    senddata.SetObject();
    params.SetObject();
    int message_id = this->create_id();
    params.AddMember("to",StringRef(UserId),this->global_document.GetAllocator());
    params.AddMember("message",StringRef(Message),this->global_document.GetAllocator());
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd",StringRef("forward"),this->global_document.GetAllocator());
    senddata.AddMember("obj",StringRef("message"),this->global_document.GetAllocator());
    senddata.AddMember("params",params,this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return false;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return false;
    d.Parse(recv_buf);
    char *log = new char[512];
    memset(log,0,512);
    delete recv_buf;
    if(d.HasMember("reply")){
        if(d["reply"].HasMember("error")){
            if(d["reply"]["error"].IsString()){
                this->write_log(d["reply"]["error"].GetString());
                return false;
            }else if(d["reply"]["error"].IsNull())
                return true;
            this->write_log("Unknown Error");
            return false;
        }
    }
    return false;
}

bool gurgle::update_roster(char *id, char *nickname, char *group, int sub_flag){
    if(id == nullptr)
        return false;
    rapidjson::Value senddata;
    rapidjson::Value params;
    senddata.SetObject();
    params.SetObject();
    int message_id = this->create_id();
    if(nickname)
        params.AddMember("nickname",StringRef(nickname),this->global_document.GetAllocator());
    if(group)
        params.AddMember("groups",StringRef(group),this->global_document.GetAllocator());
    if(sub_flag == SUBSCRIBE)
        params.AddMember("sub_to",true,this->global_document.GetAllocator());
    else if(sub_flag == UNSUBSCRIBE)
        params.AddMember("sub_to",false,this->global_document.GetAllocator());
    if(params.IsNull())
        return false;
    params.AddMember("gid",StringRef(id),this->global_document.GetAllocator());
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd",StringRef("update"),this->global_document.GetAllocator());
    senddata.AddMember("obj",StringRef("roster"),this->global_document.GetAllocator());
    senddata.AddMember("params",params,this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return nullptr;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return nullptr;
    d.Parse(recv_buf);
    char *log = new char[512];
    memset(log,0,512);
    if(d.HasMember("reply")){
        if(d["reply"].HasMember("error"))
            if(d["reply"]["error"].IsNull())
                return true;
    }
    return false;
}

bool gurgle::is_id_match(const char *idA, const char *idB){
    gurgle_id_t idA_s,idB_s;
    idA_s = this->analyse_full_id(idA);
    idB_s = this->analyse_full_id(idB);
    if (strcmp(idA_s.username,"")==0 || strcmp(idB_s.username,"")==0)
        return false;
    if(strcmp(idA_s.username,idB_s.username) == 0){
        if(this->__alias_size == 0){
            if(strcmp(idA_s.domain,idB_s.domain) == 0)
                return true;
            else
                return false;
        }else{
            bool a,b;
            a=false;
            b=false;
            int i;
            for(i=0;i<this->__alias_size;i++){
                if(a != true)
                    if(strcasecmp(idA_s.domain,this->__alias[i]) == 0){
                        a  = true;
                    }
                if(b != true)
                    if(strcasecmp(idB_s.domain,this->__alias[i]) == 0){
                        b = true;
                    }
            }
            if(a == true && b == true)
                return true;
        }
    }
    return false;
}

char** gurgle::query_server_alias(int &count){
    if(this->is_connected() == false)
        return nullptr;
    count = 0;
    rapidjson::Value senddata;
    senddata.SetObject();
    int message_id = this->create_id();
    senddata.AddMember("id",message_id,this->global_document.GetAllocator());
    senddata.AddMember("cmd",StringRef("query"),this->global_document.GetAllocator());
    senddata.AddMember("obj",StringRef("server_alias"),this->global_document.GetAllocator());
    rapidjson::Document d;
    rapidjson::StringBuffer sb;
    rapidjson::Writer<StringBuffer> writer(sb);
    senddata.Accept(writer);
    if(!this->gurgle_send(sb.GetString(),sb.GetSize()))
        return nullptr;
    char *recv_buf = new char[512];
    memset(recv_buf,0,512);
    if(!this->gurgle_recv(recv_buf,512,message_id))
        return nullptr;
    d.Parse(recv_buf);
    if(d.HasMember("reply")){
        if(d["reply"].HasMember("count")){
            count = d["reply"]["count"].GetInt();
        }
        if(d["reply"].HasMember("value")){
            if(d["reply"]["value"].IsArray()){
                int i;
                int len;
                char **returnVar = new char*[count];
                for(i=0;i<count;i++){
                    len = d["reply"]["value"][i].GetStringLength();
                    returnVar[i] = new char[len+1];
                    memset(returnVar[i],0,len+1);
                    memcpy(returnVar[i],d["reply"]["value"][i].GetString(),len);
                    returnVar[i][len] = 0;
                }
                return returnVar;
            }
        }
    }
    return nullptr;
}

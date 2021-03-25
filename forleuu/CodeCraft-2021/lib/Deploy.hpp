#pragma once

#include <vector>
#include <unordered_map>
#include <map>

#include "Types.hpp"

namespace hw{

#define link_vm(server,vs) do{ \
    if ((server)->vms!=nullptr){ \
        (vs)->prev = (server)->vms; \
        (server)->vms->next = vs; \
        (server)->vms = vs; \
    }else{(server)->vms = vs;} \
}while(0)

#define unlink_vm(server,vs) do{ \
    if ((vs)->next!=nullptr) \
        (vs)->next->prev = (vs)->prev; \
    if ((vs)->prev!=nullptr) \
        (vs)->prev->next = (vs)->next; \
    if ((server)->vms==vs) \
        (server)->vms = (vs)->prev; \
    (vs)->next = nullptr; \
    (vs)->prev = nullptr; \
}while(0)

class Deploy{
public:
    Deploy(hw::Types* types,int days,std::vector<Request_t>* reqs)
    :types(types),days(days),reqs(reqs),costnow(0),empty_servers(nullptr),cmratio(0),serverid(0),vmid(0)
    {
        lnodes_init(&serveridle);
        lnodes_init(&serverlive);
    }

    ~Deploy();

    double req_cmratio();

    void deal_req();

    void printr();

    unsigned long my_cost();

private:
    Server_t* new_server(int cpu,int mem,int day,int id,bool idle);

    void enter_server(Server_t* server,int cpu,int mem,int ntype);

    void leave_server(Server_t* server,int cpu,int mem,int ntype);

    int whichnode_vm(Server_t* now,int cpurq,int memrq,bool multi);

    void bind_server(const Request_t& req);

    void unbind_server(const Request_t& req);

    void optimize_servers(int day);

    Server_t* merge_server(std::vector<Server_t*>& servers,int a,int num,int day);

    void server_event(Server_t* server);

    void mark_empty(int day);

    unsigned long cal_cost();

    int days;
    hw::Types* types;
    std::vector<Request_t>* reqs;
    unsigned long costnow;
    Server_t* empty_servers;

    double cmratio;

    unsigned long serverid;
    unsigned long vmid;
    std::vector<VM_t*> vms;
    std::vector<Server_t*> serverseq;

    //lnodes(Server_t) serverseq;

    lnodes(Server_t) serverlive;
    lnodes(Server_t) serveridle;

    std::unordered_map<int,std::vector<Server_t*>> servers;
    std::unordered_map<unsigned long,VM_t*> vtos;
    std::unordered_map<unsigned long,Server_t*> idtos;
};

#define absub(a,b) ({ \
    double ret; \
    if (a>b) \
        ret = a-b; \
    else \
        ret = b-a; \
    ret; \
})

#define _max(a,b) ({ \
    __typeof__(a) ret; \
    ret = (a>b)?a:b; \
    ret; \
})

#define _min(a,b) ({ \
    __typeof__(a) ret; \
    ret = (a<b)?a:b; \
    ret; \
})

inline unsigned long Deploy::my_cost(){
    return costnow;
}

inline Server_t* Deploy::new_server(int cpu,int mem,int day,int id,bool idle){
    Server_t* server = types->new_server(cpu,mem,day);
    if (server==nullptr)
        return server;
    server->id = id;
    server->idle = idle;
    return server;
}

}
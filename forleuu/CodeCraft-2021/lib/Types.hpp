#pragma once

#include <string.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <algorithm>
#include <bitset>

#include "sList.hpp"

namespace hw{

struct Server;

#define DAYMAX 1000

#define A 0
#define B 1
#define AB 2

typedef struct VType{
    std::string name;
    int cpu;
    int mem;
    bool multi;

    VType(std::string& name,int cpu,int mem,bool multi)
    :name(name),cpu(cpu),mem(mem),multi(multi){}
}VType_t;

typedef struct VM:VType{
    int day;
    int pnode;
    unsigned long myid;
    struct VM *next,*prev;

    VM(struct VType& vt,int day,int pnode,unsigned long myid)
    :VType(vt.name,vt.cpu,vt.mem,vt.multi),
    day(day),pnode(pnode),myid(myid),next(nullptr),prev(nullptr){}
}VM_t;

typedef struct SType{
    int cpu;
    int mem;
    int hcost;
    int mcost;
    std::string name;

    SType(int cpu,int mem,int hcost,int mcost,std::string& name)
    :cpu(cpu),mem(mem),hcost(hcost),mcost(mcost),name(name){}
}SType_t;

#define RUNNING 0
#define IDLE 1

typedef struct Server:SType{
    bool idle;
    unsigned long id;
    int cpua,mema;
    int cpub,memb;
    int birth;
    int nvm;
    VM_t *vms;
    struct Server *eprev;
    std::bitset<DAYMAX> states;
    std::vector<int> myentry;
    lnode(struct Server) lidle;
    lnode(struct Server) llive;

    int _cpuamin,_memamin;
    int _cpubmin,_membmin;

    Server(SType_t& st,int birth)
    :SType(st.cpu,st.mem,st.hcost,st.mcost,st.name),
    id(-1),birth(birth),nvm(0),vms(nullptr),eprev(nullptr)
    {   
        cpua = cpub = cpu>>1;
        mema = memb = mem>>1;
        _cpuamin = cpua;
        _cpubmin = cpub;
        _memamin = mema;
        _membmin = memb;

        lnode_init(&lidle);
        lnode_init(&llive);
    }

}Server_t; 

#define server_cost(server,uday) ({ \
    int cost; \
    cost = server->hcost+server->mcost*(uday); \
    cost; \
})

typedef struct Request{
    bool add;
    int day;
    unsigned long id;
    std::string vm;
}Request_t;

class Types{
public:
    Types();

    void init_smodels_best(int nday);

    Server_t* new_server(int cpu,int mem,int day);

    VType_t& to_vt(const std::string& name);

    static unsigned long to_num(std::string& s,int& pos,char&& delimiter);

private:
    
    std::vector<SType_t> smodels;
    std::unordered_map<int,std::vector<SType_t>> smodels_best;
    std::unordered_map<std::string,VType_t> vmodels;
    std::vector<VType_t> vs;
};

inline VType_t& Types::to_vt(const std::string& name){
    return vmodels.find(name)->second;
}

}
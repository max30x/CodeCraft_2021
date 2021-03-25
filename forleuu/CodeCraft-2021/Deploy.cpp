#include "lib/Deploy.hpp"

namespace hw{

Deploy::~Deploy(){
    for (int i=0,ei=vms.size();i<ei;++i)
        delete vms[i];
    std::unordered_map<int,std::vector<Server_t*>>::iterator it,eit;
    for (it=servers.begin(),eit=servers.end();it!=eit;++it){
        for (int i=0,ei=it->second.size();i<ei;++i)
            delete it->second[i];
    }
}

void Deploy::mark_empty(int day){
    Server_t* ss = nullptr;
    while (empty_servers!=nullptr){
        Server_t* tmp;
        if (empty_servers->nvm!=0){
            tmp = empty_servers;
            empty_servers = empty_servers->eprev;
            tmp->eprev = nullptr;
            continue;
        }
        empty_servers->states.set(day);
        tmp = empty_servers->eprev;
        empty_servers->eprev = ss;
        ss = empty_servers;
        empty_servers = tmp;
    }
    empty_servers = ss;
}

unsigned long Deploy::cal_cost(){
    unsigned long cost = 0;
    std::unordered_map<int,std::vector<Server_t*>>::iterator it;
    for (int i=1;i<=days;++i){
        it = servers.find(i);
        if (it==servers.end())
            continue;
        for (int k=0,ek=it->second.size();k<ek;++k){
            Server_t* now = it->second[k];
            int edays = now->states.count();
            int udays = days-now->birth+1-edays;
            cost += now->mcost*udays;
            cost += now->hcost;
        }
    }

    return cost;
}

void Deploy::enter_server(Server_t* server,int cpu,int mem,int ntype){
    switch (ntype){
    case A:{
        server->cpua -= cpu;
        server->mema -= mem;
        break;
    }
    case B:{
        server->cpub -= cpu;
        server->memb -= mem;
        break;
    }
    case AB:{
        server->cpua -= cpu;
        server->cpub -= cpu;
        server->mema -= mem;
        server->memb -= mem;
        break;
    }
    }
    ++server->nvm;
    if (server->idle){
        server->idle = false;
        ldelete(&serveridle,server,lidle);
        lappend(&serverlive,server,llive);
    }
}

void Deploy::leave_server(Server_t* server,int cpu,int mem,int ntype){
    switch (ntype){
    case A:{
        server->cpua += cpu;
        server->mema += mem;
        break;
    }
    case B:{
        server->cpub += cpu;
        server->memb += mem;
        break;
    }
    case AB:{
        server->cpua += cpu;
        server->cpub += cpu;
        server->mema += mem;
        server->memb += mem;
        break;
    }
    }
    --server->nvm;
    if (server->nvm==0){
        server->idle = true;
        ldelete(&serverlive,server,llive);
        lappend(&serveridle,server,lidle);
    }
}

double Deploy::req_cmratio(){
    if (cmratio!=0)
        return cmratio;
    int num = 0;
    for (int i=0,ei=reqs->size();i<ei;++i){
        if (!(*reqs)[i].add)
            continue;
        VType_t vt = types->to_vt((*reqs)[i].vm);
        cmratio += (double)vt.cpu/(double)vt.mem;
        ++num;
    }
    cmratio /= (double)num;
    return cmratio;
}

int Deploy::whichnode_vm(Server_t* now,int cpurq,int memrq,bool multi){
    bool _a = (now->cpua>=cpurq&&now->mema>=memrq);
    bool _b = (now->cpub>=cpurq&&now->memb>=memrq);
    if (!multi){
        if (!_a&&!_b)
            return -1;
        else if (_a&&_b){
        
            int _mincpua = _min(now->cpua-cpurq,now->cpub);
            int _mincpub = _min(now->cpua,now->cpub-cpurq);
            int _minmema = _min(now->mema-memrq,now->memb);
            int _minmemb = _min(now->mema,now->mem-memrq);
            return ((_mincpua+_minmema)>(_mincpub+_minmemb))?A:B;
        #if 0
            int _cpua = absub(now->cpua,cpurq);
            int _mema = absub(now->mema,memrq);
            int _cpub = absub(now->cpub,cpurq);
            int _memb = absub(now->memb,memrq);
            int diff1 = _cpua+_mema;
            int diff2 = _cpub+_memb;
            //if (diff1==diff2){
                //double _cmratio = req_cmratio();
                //double _diff1 = absub((double)_cpua/(double)_mema,_cmratio);
                //double _diff2 = absub((double)_cpub/(double)_memb,_cmratio);
                //return (_diff1>_diff2) ? A : B;
                //return B;
            //}
            return (diff1>diff2) ? A : B;
        #endif
        }
        else if (_a)
            return A;
        else if (_b)
            return B;
    }
    else{
        if (_a&&_b)
            return AB;
    }
    return -1;
}

Server_t* servers0[3000][3000];
bool wanted[3000][3000];
bool vis[3000][3000];

void Deploy::optimize_servers(int day){
    std::unordered_map<int,std::vector<Server_t*>>::iterator it;
    it = servers.find(day);
    if (it==servers.end())
        return;
    std::vector<Server_t*> dservers = it->second;
    servers.erase(day);

    int _size = dservers.size();    
    for (int i=0;i<_size;++i){
        for (int j=0;j<_size;++j){
            wanted[i][j] = false;
            vis[i][j] = false;
        }
    }
        
    for (int i=0;i<_size;++i){
        for (int j=0;j<i;++j)
            servers0[i][j] = nullptr;
        servers0[i][i] = dservers[i];
        for (int j=i+1;j<_size;++j)
            servers0[i][j] = merge_server(dservers,i,j,day);
    }
#if 0
    std::cout<<day<<"\n";
    for (int i=0;i<_size;++i){
        for (int j=i;j<_size;++j){
            Server_t* snow = servers0[i][j];
            if (snow!=nullptr){
                std::cout<<i<<" "<<j<<" ";
                std::cout<<snow->cpu<<" "
                        <<snow->mem<<" "
                        <<snow->hcost<<" "
                        <<snow->mcost<<"\n";
            }
        }
    }
#endif
    unsigned long nowcost=0,mincost=INT32_MAX;
    std::vector<int> costs;
    std::vector<int> indexs;
    std::vector<int> path;
    indexs.push_back(-1);
    while (!indexs.empty()){
        int _isize = indexs.size();
        int _csize = costs.size();
        int _back=indexs[_isize-1]+1;
        if (_back==_size){
            if (nowcost<mincost){ 
                mincost=nowcost;
                path.clear();
                for (int i=0,ei=indexs.size();i<ei;++i)
                    path.push_back(indexs[i]);
            }
            indexs.pop_back();
            nowcost -= costs[_csize-1];
            costs.pop_back();
            continue;
        }
        bool found=false;
        for (int i=_back;i<_size;++i){
            Server_t* snow = servers0[_back][i];
            if (!vis[_back][i] && snow!=nullptr){
                found=true;
                vis[_back][i]=true;
                int cnow=server_cost(snow,days-day+1-snow->states.count());
                nowcost += cnow;
                costs.push_back(cnow);
                indexs.push_back(i);
                break;
            }
        }
        if (!found){
            indexs.pop_back();
            if (!costs.empty()){
                nowcost -= costs[_csize-1];
                costs.pop_back();
            }
        }
    }
    std::vector<Server_t*> __servers;
    for (int i=0,ei=path.size();i<ei-1;++i){
        int a=path[i]+1;
        int b=path[i+1];
        wanted[a][b] = true;
        __servers.push_back(servers0[a][b]);
        if (a==b)
            continue;
        for (int k=a;k<=b;++k){
            for (int j=0,ej=dservers[k]->myentry.size();j<ej;++j)
                idtos[dservers[k]->myentry[j]] = servers0[a][b];
        }
    }

    for (int i=0;i<_size;++i){
        for (int j=0;j<_size;++j){
            if (!wanted[i][j] && servers0[i][j]!=nullptr)
                delete servers0[i][j];
        }
    }
#if 0
    std::cout<<"costmin:"<<mincost<<" costnow:"<<nowcost<<"\n";
    //std::cout<<day<<" "<<_size<<"\n";
    for (int i=0,ei=path.size();i<ei;++i)
       std::cout<<path[i]<<" ";
    std::cout<<"\n";
    //std::cout<<day<<" "<<costmin<<"\n";
#endif
    servers.insert(std::make_pair(day,__servers));
}

Server_t* Deploy::merge_server(std::vector<Server_t*>& servers,int a,int b,int day){
    int cpurq=0,memrq=0;
    std::bitset<DAYMAX> edays;
    for (int i=a;i<=b;++i){
        Server_t* now = servers[i];
        int _cpu = now->cpu>>1;
        int _mem = now->mem>>1;
        cpurq += _max(_cpu-now->_cpuamin,_cpu-now->_cpubmin);
        memrq += _max(_mem-now->_memamin,_mem-now->_membmin);
        edays |= now->states;
    }
    Server_t* servern = types->new_server(cpurq<<1,memrq<<1,day);
    if (servern==nullptr)
        return nullptr;
    servern->states = edays;
    return servern;
}

void Deploy::server_event(Server_t* server){
    server->_cpuamin = (server->cpua<server->_cpuamin)?server->cpua:server->_cpuamin;
    server->_cpubmin = (server->cpub<server->_cpubmin)?server->cpub:server->_cpubmin;
    server->_memamin = (server->mema<server->_memamin)?server->mema:server->_memamin;
    server->_membmin = (server->memb<server->_membmin)?server->memb:server->_membmin;
}

void Deploy::bind_server(const Request_t& req){
    VType_t vt = types->to_vt(req.vm);
    int cpurq = vt.cpu;
    int memrq = vt.mem;
    if (vt.multi){
        cpurq>>=1;
        memrq>>=1;
    }
    int ntype = -1;
    Server_t* picked = nullptr;
    ltraverse(Server_t,&serverlive,llive,now){
        ntype = whichnode_vm(now,cpurq,memrq,vt.multi);
        if (ntype==-1)
            continue;
        picked = now;
        break;
    }
    if (picked==nullptr){
        ltraverse(Server_t,&serveridle,lidle,now){
            ntype = whichnode_vm(now,cpurq,memrq,vt.multi);
            if (ntype==-1)
                continue;
            picked = now;
            break;
        }
    }
#if 0
    if (!serverseq.empty()){
        std::vector<unsigned long> _servers;
        for (int i=0,ei=serverseq.size();i<ei;++i)
            _servers.push_back((unsigned long)serverseq[i]);
        auto sfunc = [&](const unsigned long& _s1,const unsigned long& _s2){
            Server_t* s1 = (Server_t*)_s1;
            Server_t* s2 = (Server_t*)_s2;
            return (s1->_cpuamin+s1->_memamin+s1->_cpubmin+s1->_membmin)<
                    (s2->_cpuamin+s2->_memamin+s2->_cpubmin+s2->_membmin);
        };
        std::sort(_servers.begin(),_servers.end(),sfunc);
        for (int i=0,ei=serverseq.size();i<ei;++i){
            Server_t* now = serverseq[i];
            ntype = whichnode_vm(now,cpurq,memrq,vt.multi);
            if (ntype==-1)
                continue;
            picked = now;
            break;
        }
    }
#endif
    if (picked==nullptr){
        int _cpurq = cpurq<<1;
        int _memrq = memrq<<1; 
        //picked = types->new_server(_cpurq,_memrq,req.day);
        picked = new_server(_cpurq,_memrq,req.day,serverid++,true);
        picked->idle = false;
        lappend(&serverlive,picked,llive);
        //serverseq.push_back(picked);
        
        std::unordered_map<int,std::vector<Server_t*>>::iterator it;
        it = servers.find(req.day);
        if (it==servers.end()){
            std::vector<Server_t*> _v;
            _v.push_back(picked);
            servers.insert(std::make_pair(req.day,_v));
        }
        else
            it->second.push_back(picked);
        ntype = (vt.multi)?AB:A;
    }
    enter_server(picked,cpurq,memrq,ntype);

    unsigned long _vmid = vmid++;
    idtos.insert(std::make_pair(_vmid,picked));
    picked->myentry.push_back(_vmid);

    VM_t* vm = new VM_t(vt,req.day,ntype,_vmid);
    vtos.insert(std::make_pair(req.id,vm));
    vms.push_back(vm);
    link_vm(picked,vm);
    server_event(picked);
}   

void Deploy::unbind_server(const Request_t& req){
    VM_t* vm = vtos.find(req.id)->second;
    Server_t* server = idtos.find(vm->myid)->second;
    int cpureq = vm->cpu;
    int memreq = vm->mem;
    if (vm->multi){
        cpureq >>= 1;
        memreq >>= 1;
    }
    leave_server(server,cpureq,memreq,vm->pnode);
    unlink_vm(server,vm);
    vtos.erase(req.id);
    if (server->nvm==0){
        server->eprev = empty_servers;
        empty_servers = server;
    }
}

void Deploy::deal_req(){
    int _day=1;
    for (int i=0,ei=reqs->size();i<ei;++i){
        if ((*reqs)[i].day!=_day){
            mark_empty(_day);
            _day = (*reqs)[i].day;
        }
        if ((*reqs)[i].add)
            bind_server((*reqs)[i]);
        else 
            unbind_server((*reqs)[i]);
    }
    for (int i=1;i<=days;++i)
        optimize_servers(i);
    costnow = cal_cost();
}

void Deploy::printr(){
    unsigned long seq = 0;
    for (int day=1,nday=days,_i=0,_ei=vms.size();day<=nday;++day){
        std::unordered_map<std::string,std::vector<Server_t*>> ss;
        std::unordered_map<std::string,std::vector<Server_t*>>::iterator it,eit;

        std::unordered_map<int,std::vector<Server_t*>>::iterator sit;
        sit = servers.find(day);
        if (sit!=servers.end()){
            for (int i=0,ei=sit->second.size();i<ei;++i){
                Server_t* now = sit->second[i];
                if (now->birth!=day)
                    break;
                it = ss.find(now->name);
                if (it!=ss.end()){
                    it->second.push_back(now);
                    continue;
                }
                std::vector<Server_t*> tv;
                tv.push_back(now);
                ss.insert(std::make_pair(now->name,tv));
            }
        }
        
        std::cout<<"(purchase, "<<ss.size()<<")\n";
        for (it=ss.begin(),eit=ss.end();it!=eit;++it){
            std::cout<<"("<<it->first<<", "<<it->second.size()<<")\n";
            for (int k=0;k<it->second.size();++k)
                it->second[k]->id = seq++;
        }
        std::cout<<"(migration, 0)\n";
        for (;_i<_ei;++_i){
            VM_t* vs = vms[_i];
            if (vs->day!=day)
                break;
            int _id = idtos.find(vs->myid)->second->id;
            std::cout<<"("<<_id;
            switch (vs->pnode){
            case A:
                std::cout<<", A)\n";
                break;
            case B:
                std::cout<<", B)\n";
                break;
            case AB:
                std::cout<<")\n";
                break;
            }
        }
        ss.clear();
    }
    std::cout<<std::flush;
}

}
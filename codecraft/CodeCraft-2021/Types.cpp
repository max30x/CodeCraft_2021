#include "lib/Types.hpp"

namespace hw{
Types::Types()
{
    int num;
    std::cin>>num;
    std::cin.ignore();
    for (int i=0;i<num;++i){
        std::string line;
        std::getline(std::cin,line);
        int pos = line.find_first_of(",");
        std::string name = line.substr(1,pos-1);
        ++pos;
        int cpu = to_num(line,pos,',');
        int mem = to_num(line,pos,',');
        int hcost = to_num(line,pos,',');
        int mcost = to_num(line,pos,')');
        SType_t st(cpu,mem,hcost,mcost,name);
        smodels.push_back(st);
    }

    std::cin>>num;
    std::cin.ignore();
    for (int i=0;i<num;++i){
        std::string line;
        std::getline(std::cin,line);
        int pos = line.find_first_of(",");
        std::string name = line.substr(3,pos-3);
        ++pos;
        int cpu = to_num(line,pos,',');
        int mem = to_num(line,pos,',');
        bool multi = to_num(line,pos,')')==1;
        VType_t vt(name,cpu,mem,multi);
        vmodels.insert(std::make_pair(name,vt));
    }
}

unsigned long Types::to_num(std::string& s,int& pos,char&& delimiter){
    while (s[pos]==' ')
        ++pos;
    unsigned long num = 0;
    for (;s[pos]!=delimiter;++pos)
        num = num*10+(s[pos]-'0');
    ++pos;
    return num;
}

void Types::init_smodels_best(int nday){
    for (int i=1;i<=nday;++i){
        std::vector<SType_t> _smodels;
        for (int k=0,ek=smodels.size();k<ek;++k)
            _smodels.push_back(smodels[k]);
        double _c=0.25,_m=0.18;
        auto sfunc = [&](const SType_t& s1,const SType_t& s2){
            double cost1 = s1.hcost+s1.mcost*(nday-i+1);
            double cost2 = s2.hcost+s2.mcost*(nday-i+1);
            double cmcost1 = cost1/s1.cpu*_c+cost1/s1.mem*_m;
            double cmcost2 = cost2/s2.cpu*_c+cost2/s2.mem*_m;
            return cmcost1<cmcost2;
        };
        std::sort(_smodels.begin(),_smodels.end(),sfunc);
        smodels_best.insert(std::make_pair(i,_smodels));
    }
}

Server_t* Types::new_server(int cpu,int mem,int day){
    std::unordered_map<int,std::vector<SType_t>>::iterator it;
    it = smodels_best.find(day);
    Server_t* s = nullptr;
    for (int i=0,ei=it->second.size();i<ei;++i){
        if (it->second[i].cpu<cpu||it->second[i].mem<mem)
            continue;
        s = new Server_t(it->second[i],day);
        break;
    }
    return s;
}

}
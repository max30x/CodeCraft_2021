#include <vector>

#include "lib/Deploy.hpp"
#include "lib/Types.hpp"

std::vector<hw::Request_t> requests;
double cmratio;
int nday;

void get_reqs(){
    int num;
    std::cin>>nday;
    for (int i=1;i<=nday;++i){
        std::cin>>num;
        std::cin.ignore();
        for (int j=0;j<num;++j){
            std::string line;
            std::getline(std::cin,line);
            hw::Request_t req;
            req.day = i;
            req.add = line[1]=='a';
            int pos = line.find_first_of(",");
            ++pos;
            if (req.add){
                while (line[pos]==' ')
                    ++pos;
                int npos = line.find(",",pos);
                req.vm = line.substr(pos+2,npos-pos-2);
                pos = npos+1;
            }
            req.id = hw::Types::to_num(line,pos,')');
            requests.push_back(req);
        }
    }
}

int main(){
    srand((unsigned)time(nullptr));
    std::ios_base::sync_with_stdio(false);

    hw::Types types;
    get_reqs();
    types.init_smodels_best(nday);

    hw::Deploy* dp = new hw::Deploy(&types,nday,&requests);
    //std::cout<<dp->req_cmratio()<<"\n";
    dp->deal_req();
    dp->printr();
    //std::cout<<"cost:"<<dp->my_cost()<<"\n";
    delete dp;

    return 0;
}
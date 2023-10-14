#ifndef __TOPOLOGICAL_SORT
#define __TOPOLOGICAL_SORT

#include <framework/logger/logger.h>



#include <cstdarg>
#include <algorithm>
#include <tuple>
#include <bitset>
#include <stack>
#include <ctime>
//#include <unistd.h>
#include <cstring>
#include <vector>
    /*===================================*/
    /* Topoligical sorting of subitem */
    /*===================================*/

using namespace std;
template <typename ID_T>
bool topological_sort(
        vector<unsigned int> & rank,
        vector<ID_T> & sequence,
        vector<tuple<ID_T, int> > & niceness,
        vector<vector<ID_T> > & dependencies,
        vector<const char *> & debugname
){
    /* Parameter Sanity Checks */
    size_t no_of_items = niceness.size();
    if (no_of_items == 0) {
        LOG_ERROR << "No items to sort in niceness.";
        return false;
    }
    if (!rank.empty()){
        LOG_ERROR << "Parameter rank is not empty.";
        return false;
    }
    if (!sequence.empty()){
        LOG_ERROR << "Parameter sequence is not empty.";
        return false;
    }
    if (dependencies.size() != no_of_items){
        LOG_ERROR << "Wrong number of dependencies.";
        return false;
    }
    if (debugname.size() != no_of_items){
        LOG_ERROR << "Wrong number of debugnames.";
        return false;
    }


    rank = vector<unsigned int>(no_of_items,0);
    vector<bool> discovered_items(no_of_items,false);
    int current_rank = 0;

    // Sort items by niceness
    stable_sort(niceness.begin(), niceness.end(),
            []( const tuple<ID_T, int>& a,
                const tuple<ID_T, int>& b
            ) -> bool
            {  return get<1>(a) < get<1>(b); }
        );

    // DFS to sort topologically
    for (auto ncs : niceness){
        vector<ID_T> topo_stack({get<0>(ncs)});
        while (not topo_stack.empty() ){
            ID_T item = topo_stack.back();
            if ( not discovered_items[item] ){
                discovered_items[item] = true;
                for (auto item_dependency : dependencies[item]){
                    topo_stack.push_back(item_dependency);
                }
            } else {
                if ( not rank[item] ) {
                    for (auto item_dependency : dependencies[item]){
                        if (rank[item_dependency] == 0){
                            LOG_ERROR << "Cyclic dependencies!";
                            LOG_ERROR << debugname[item] << " depends on " << debugname[item_dependency] << " which has no rank yet!";
                            LOG_ERROR << "Dumping Stack:";
                            for (auto item : topo_stack){
                                LOG_ERROR << "topo_stack>   " << debugname[item];
                            }
                            return false;
                        }
                    }
                    sequence.push_back(item);
                    rank[item] = ++current_rank;
                }
                topo_stack.pop_back();
            }
        }
    }

    // Sanity check
    if (size_t(current_rank) != no_of_items){
        LOG_ERROR << "Not all items were ranked";
        return false;
    }
    return true;
}

#endif // __TOPOLOGICAL_SORT

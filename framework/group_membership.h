#ifndef GROUP_MEMBERSHIP_H
#define GROUP_MEMBERSHIP_H

#include <list>
#include "utils/transaction.h"
#include "transport.h"
#include "mem_alloc.h"
#include "global.h"
#include "node.h"
#include "communication.h"

class TransactionF;
class Node;

class interfaceGroupMembership{
    public:
        virtual ~interfaceGroupMembership(){}
        virtual Node* getNode(uint64_t nodeId) = 0;
        virtual map<uint64_t,Node*>  getNodes() = 0;
        virtual list<Node*> getReplicationTargets(uint64_t nodeId) = 0;
        virtual int getTimestamper() = 0;
        virtual int getCoordinator(TransactionF* transaction) = 0;
};

class GroupMembership : public interfaceGroupMembership{

    private:
        map<uint64_t,Node*> mapOfNodes;
        uint64_t get_port_id(uint64_t src_node_id, uint64_t dest_node_id, uint64_t send_thread_id);
        void initNodes();
        void read_ifconfig(const char * ifaddr_file);
        string get_path(); 
        char ** ifaddr;

    public:
        ~GroupMembership(){}
        GroupMembership();
        Node* getNode(uint64_t nodeId);

        map<uint64_t,Node*> getNodes() override{return mapOfNodes;}
        list<Node*> getReplicationTargets(uint64_t nodeId) override;
        int getTimestamper() override;
        int getCoordinator(TransactionF* transaction) override;
};
#endif
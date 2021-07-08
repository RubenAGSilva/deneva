#ifndef GROUP_MEMBERSHIP_H
#define GROUP_MEMBERSHIP_H

#include <list>
#include "utils/transaction.h"
#include "node.h"

class TransactionF;
class Node;

class interfaceGroupMembership{
    public:
        virtual ~interfaceGroupMembership(){}
        virtual Node* getNode() = 0;
        virtual list<Node> getNodes() = 0;
        virtual list<Node>* getReplicationTargets() = 0;
        virtual int getTimestamper() = 0;
        virtual int getCoordinator(TransactionF* transaction) = 0;
};

class GroupMembership : public interfaceGroupMembership{

    private:
        Node* mySelf;
        
        uint64_t get_port_id(uint64_t src_node_id, uint64_t dest_node_id, uint64_t send_thread_id);

    public:
        ~GroupMembership(){}
        GroupMembership(Node* node1);
        GroupMembership() = default;
        Node* getNode() override {return mySelf;}

        list<Node> getNodes() override;
        list<Node>* getReplicationTargets() override;
        int getTimestamper() override;
        int getCoordinator(TransactionF* transaction) override;
};
#endif
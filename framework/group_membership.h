#ifndef GROUP_MEMBERSHIP_H
#define GROUP_MEMBERSHIP_H

#include <list>
#include "utils/transaction.h"
#include "node.h"
class GroupMembership{

    private:
        Node node;
        Role role;

    public:

        GroupMembership(Node node1);
        GroupMembership() = default;
        Role getRole(){ return role;}
        Node getNode(){return node;}

        list<Node> getReplicationTargets();
        int getTimestamper();
        int getCoordinator(TransactionF transaction);
        int getCoordinator();
};
#endif
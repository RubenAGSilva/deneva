#ifndef GROUP_MEMBERSHIP_H
#define GROUP_MEMBERSHIP_H

#include <list>
#include "utils/transaction.h"
#include "node.h"

class interfaceGroupMembership{
    public:
        virtual ~interfaceGroupMembership(){}
        virtual Role getRole() = 0;
        virtual Node getNode() = 0;
        virtual list<Node> getReplicationTargets() = 0;
        virtual int getTimestamper() = 0;
        virtual int getCoordinator(TransactionF transaction) = 0;
        virtual int getCoordinator() = 0;
};

class GroupMembership : public interfaceGroupMembership{

    private:
        Node node;
        Role role;

    public:
        ~GroupMembership(){}
        GroupMembership(Node node1);
        GroupMembership() = default;
        Role getRole() override { return role;}
        Node getNode() override {return node;}

        list<Node> getReplicationTargets() override;
        int getTimestamper() override;
        int getCoordinator(TransactionF transaction) override;
        int getCoordinator() override;
};
#endif
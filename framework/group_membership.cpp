#include "group_membership.h"
#include "utils/transaction.h"
#include "node.h"
#include <iostream>
#include <list>

using namespace std;

GroupMembership::GroupMembership(Node node1){
    node = node1;
    role = node.getRole();
}
list<Node> GroupMembership::getReplicationTargets(){ //not void but list<node>
    printf("get replication targets \n");
    fflush(stdout);
    return list<Node>();
} 
int GroupMembership::getTimestamper(){
    printf("get timestamper \n");
    fflush(stdout);
    return 1;
}
int GroupMembership::getCoordinator(TransactionF transaction){
    printf("get coordinator \n");
    fflush(stdout);
    return 1;
}
int GroupMembership::getCoordinator(){
    printf("get coordinator \n");
    fflush(stdout);
    return 1;
}
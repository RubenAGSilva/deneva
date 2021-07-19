#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "utils/version.h"
#include "utils/node.h"
#include "utils/concurrency.h"
#include "concurrency_controller.h"

namespace configuration{
    
    static interfaceGroupMembership* initGroupModule(Node* node){
        GroupMembership* g = new GroupMembership(node);
        return g;
    }

    static interfaceReplication* initReplicationModule(){
        Replication* r = new Replication();
        return r;
    }
    
    static interfaceOrder* initOrderModule(Node* node, InterfaceVersion* clockVersion){
        Order* o = new Order(node ,clockVersion);
        return o;
    }

    static InterfaceVersion* initClockVersion(){
        ClockF* clock = new ClockF();
        return clock;
    }

    static InterfaceConcurrencyControl* initConcurrencyControl(Content* c){
        //switch case - CCAlgorithm, case locking: ; case otimistic; case multiversion etc...
        CC_Lock* concurrencyControl = new CC_Lock(c);
        //CC_O* concurrencyControl = new CC_O(c);
        return concurrencyControl;
    }

    static InterfaceConcurrencyController* initConcurrencyController(){
        ConcurrencyControllerLocks* concurrencyController = new ConcurrencyControllerLocks();
        //ConcurrencyControllerOtimistic* concurrencyController = new ConcurrencyControllerOtimistic();
        return concurrencyController;
    }
}
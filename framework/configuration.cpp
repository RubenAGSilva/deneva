#include "group_membership.h"
#include "order.h"
#include "validation.h"
#include "replication.h"
#include "utils/version.h"
#include "utils/node.h"
#include "utils/concurrency.h"

namespace configuration{
    
    static interfaceGroupMembership* initGroupModule(Node* node){
        GroupMembership* g = new GroupMembership(node);
        return g;
    }

    static interfaceReplication* initReplicationModule(){
        Replication* r = new Replication();
        return r;
    }
    
    static interfaceValidation* initValidationModule(){
        Validation* v = new Validation();
        return v;
    }
    
    static interfaceOrder* initOrderModule(Node* node, InterfaceVersion* clockVersion){
        Order* o = new Order(node ,clockVersion);
        return o;
    }

    static InterfaceVersion* initClockVersion(){
        ClockF* clock = new ClockF();
        return clock;
    }

    static InterfaceConcurrencyControl* initConcurrencyControl(Content* content){
        CC_Lock* concurrencyControl = new CC_Lock(content);
        return concurrencyControl;
    }
}
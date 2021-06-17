#include "group_membership.h"
#include "order.h"
#include "validation.h"
#include "replication.h"

namespace configuration{
    
    static interfaceGroupMembership* initGroupModule(){
        GroupMembership* g = new GroupMembership(Node(Role::TIMESTAMPER));
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
    
    static interfaceOrder* initOrderModule(){
        Order* o = new Order();
        return o;
    }
}
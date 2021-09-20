#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "utils/version.h"
#include "utils/node.h"
#include "utils/concurrency.h"
#include "concurrency_manager.h"

namespace configuration{
    
    enum CC_ALGO{LOCK, OTIMISTIC};
    enum CLOCK_ALGO{DENEVA_CLOCKS};
    enum COMMUNICATION_ALGO{SOCKET};
    enum MODULES{DENEVA};

    const CC_ALGO ccAlgo = LOCK;
    const COMMUNICATION_ALGO commAlgo = SOCKET;
    const CLOCK_ALGO clockAlgo = DENEVA_CLOCKS;
    const MODULES module = DENEVA;

    static interfaceFramework* initFramework(uint64_t nodeId){
        switch(ccAlgo){
            case LOCK:{
                FrameworkPessimisticDeneva* framework = new FrameworkPessimisticDeneva(nodeId);
                return framework;
                break;
            }case OTIMISTIC:{
                FrameworkOptimisicDeneva* framework = new FrameworkOptimisicDeneva(nodeId);
                return framework;
                break;
            }
        }
    }


    static interfaceGroupMembership* initGroupModule(){
        //switch case with different groupmembership modules
        switch(module){
            case DENEVA:
                GroupMembership* g = new GroupMembership();
                return g;   
        }
        
    }

    static interfaceReplication* initReplicationModule(){
        //switch case with different replication modules
        switch(module){
            case DENEVA:
                Replication* r = new Replication();
                return r;
        }
    }
    
    static interfaceOrder* initOrderModule(Node* node, InterfaceVersion* clockVersion){
        //switch case with different order modules
        switch(module){
            case DENEVA:
                Order* o = new Order(node ,clockVersion);
                return o;
        }
    }

    static InterfaceVersion* initClockVersion(){
        //switch case with different clock algorithms
        switch(clockAlgo){
            case DENEVA_CLOCKS:
                ClockF* clock = new ClockF();
                return clock;
        }
        
    }

    static InterfaceConcurrencyControl* initConcurrencyControl(Content* c){
        switch(ccAlgo){
            case LOCK:{
                CC_Lock* concurrencyControl = new CC_Lock(c);
                return concurrencyControl;
                break;
            }case OTIMISTIC:{
                CC_O* concurrencyControl = new CC_O(c);
                return concurrencyControl;
                break;
            }
        }
    }

    static InterfaceConcurrencyManager* initConcurrencyManager(){
        switch(ccAlgo){
            case LOCK:{
                ConcurrencyManagerLocks* concurrencyManager = new ConcurrencyManagerLocks();
                return concurrencyManager;
                break;
            }case OTIMISTIC:{
                ConcurrencyManagerOtimistic* concurrencyManager = new ConcurrencyManagerOtimistic();
                return concurrencyManager;
                break;
            }
        }
    }

    static InterfaceCommunication* initCommunication(uint64_t id){
        //switch case with different communication algorithms
        switch(commAlgo){
            case SOCKET:
                InterfaceCommunication* communication = new Communication(id);
                return communication;
        }
        
    }
}
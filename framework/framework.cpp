#include "framework.h"
#include "utils/content.h"
#include "utils/transaction.h"
#include "utils/concurrency.h"
#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "concurrency_controller.h"
#include <map>
#include <string>
#include <iostream>
#include "configuration.cpp"

using namespace std;

Framework::Framework(){
    Node* node = new Node(Role::TIMESTAMPER);
    
    groupMembership = configuration::initGroupModule(node);
    order = configuration::initOrderModule(node ,configuration::initClockVersion());
    replication = configuration::initReplicationModule();
    concurrencyController = configuration::initConcurrencyController();

}

void Framework::beginTransaction(TransactionF* transaction){
    printf("Begin transaction %lu \n", transaction->getId());
    fflush(stdout);

    mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));

    int nodeId = groupMembership->getNode()->getId();
    if(groupMembership->getNode()->getRole() != Role::TIMESTAMPER){
        nodeId = groupMembership->getTimestamper();
    }

    transaction->setCoordinator(groupMembership->getCoordinator(transaction));
    order->timestampStartup(transaction, Metadata());
    replication->replicate(transaction, Metadata(), groupMembership->getReplicationTargets());
}

void Framework::endTransaction(TransactionF* transaction){
    if(transaction->isFinished()){
        concurrencyController->finish(transaction); //equivale ao return_row //remove
        printf("end transaction %lu \n", transaction->getId());
        printf("transaction %lu terminated \n", transaction->getId());
        printf("Startup ts %ld --- Commit ts %ld\n", transaction->getTimestampStartup(), transaction->getTimestampCommit());
        fflush(stdout);
        
        //Operacoes para terminar uma transacao. retirar a transaction da lista? Ou assim.
        //mapOfTransactions.erase(transaction->getId());
    }
}

void Framework::read(TransactionF* transaction, uint64_t key){
    printf("read an op from the transaction %lu \n", transaction->getId());
    fflush(stdout);
    
    try{ //try to retrieve
        //concurrencyControlMap.at(key)->read(transaction);
        concurrencyController->read(transaction, key);

    }catch(std::out_of_range){

        printf("out of range\n");
        fflush(stdout);
    }

}
void Framework::write(TransactionF* transaction, uint64_t key, row_t* row){
    printf("write an op from the transaction %lu \n", transaction->getId());
    fflush(stdout);
    
    try{ //try to retrieve
        concurrencyController->write(transaction, key, row);
    }
    catch(std::out_of_range)
    {
        initContent(row);
        write(transaction, key, row);
    }
    

}
void Framework::commit(TransactionF* transaction){

    groupMembership->getCoordinator(transaction);
    
    if(!transaction->isValidated()){ //para sistemas que nao precisam de validacao, isValidated(){return true;}
        if(validate(transaction)){
            printf("commit transaction %lu with %lu operations:\n", transaction->getId(), transaction->getReadSet().size());
            fflush(stdout);

            order->timestampCommit(transaction, Metadata());
            list<Node>* nodes = groupMembership->getReplicationTargets();

            concurrencyController->commit(transaction);
            
            
            replication->replicate(transaction, Metadata(), nodes);
            replication->replicateResult(transaction, Metadata(), nodes);
            transaction->setFinished(true);
            concurrencyController->finish(transaction);
        }
    }else{
        printf("Abort transaction %lu\n", transaction->getId());
        fflush(stdout);
        // not validated, retry, abort??
    }
    
}
void Framework::abort(TransactionF* transaction){
    printf("Abort transaction %lu \n", transaction->getId());
    fflush(stdout);
    transaction->setFinished(true);
    concurrencyController->finish(transaction);
    // UNDO, REDO, DISCARD, ?, etc.
}

bool Framework::validate(TransactionF* transaction){ 
    return concurrencyController->validate(transaction);
}
void Framework::replicate(TransactionF* transaction){
    replication->replicate(transaction, Metadata(), groupMembership->getReplicationTargets());
}
TransactionF* Framework::getTransaction(uint64_t id){
    return mapOfTransactions[id];
}

void Framework::initContent(row_t* row){ //Content has key = to the row primary key. This is displayed in the CCMap with key as primary key. EG. to retrieve
                                         // the concurrency control of a content just access with that key. 
                                         
    //InterfaceConcurrencyControl* concurrencyControl = configuration::initConcurrencyControl(row->get_primary_key(), row);
    //concurrencyControlMap.insert(std::pair<uint64_t, InterfaceConcurrencyControl*>(row->get_primary_key(), concurrencyControl));

    concurrencyController->initContent(row->get_primary_key(), row);
    
}



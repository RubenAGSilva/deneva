#include "framework.h"
#include "utils/content.h"
#include "utils/transaction.h"
#include "utils/concurrency.h"
#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "validation.h"
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
    validation = configuration::initValidationModule();

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
        releaseLocksOfTransaction(transaction);
        printf("end transaction %lu \n", transaction->getId());
        printf("transaction %lu terminated \n", transaction->getId());
        printf("Storage system size: %lu\n", storageSystem.size());
        printf("Startup ts %ld --- Commit ts %ld\n", transaction->getTimestampStartup(), transaction->getTimestampCommit());
        fflush(stdout);
        
        //Operacoes para terminar uma transacao. retirar a transaction da lista? Ou assim.
        //mapOfTransactions.erase(transaction->getId());
    }
}

void Framework::read(TransactionF* transaction, uint64_t key){
    printf("read an op from the transaction %lu \n", transaction->getId());
    fflush(stdout);
    
    try{
        fflush(stdout);
        InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
        concurrencyControl->read(transaction);

    }catch(std::out_of_range){

        printf("out of range\n");
        fflush(stdout);
    }

}
void Framework::write(TransactionF* transaction, uint64_t key, row_t* row){
    printf("write an op from the transaction %lu \n", transaction->getId());
    fflush(stdout);
    //try to retrieve
    try
    {
        InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
        concurrencyControl->write(transaction);
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
        if(validation->validate(transaction)){
            printf("commit transaction %lu with %lu operations:\n", transaction->getId(), transaction->getReadSet().size());
            fflush(stdout);

            order->timestampCommit(transaction, Metadata());
            list<Node>* nodes = groupMembership->getReplicationTargets();
            replication->replicate(transaction, Metadata(), nodes);
            replication->replicateResult(transaction, Metadata(), nodes);
            transaction->setFinished(true);
            releaseLocksOfTransaction(transaction);
            //makeDurable(transaction); //uncomment
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
    releaseLocksOfTransaction(transaction);
    // UNDO, REDO, DISCARD, ?, etc.
}

void Framework::validate(TransactionF* transaction){ 
    validation->validate(transaction);
    //percorrer r e wr sets e validar cada um deles.
    for(Content* c : transaction->getReadSet()){
        concurrencyControlMap.at(c->getKey())->validate(transaction);
    }
    for(Content* c : transaction->getWriteSet()){
        concurrencyControlMap.at(c->getKey())->validate(transaction);
    }
}
void Framework::replicate(TransactionF* transaction){
    replication->replicate(transaction, Metadata(), groupMembership->getReplicationTargets());
}
TransactionF* Framework::getTransaction(uint64_t id){
    return mapOfTransactions[id];
}

void Framework::makeDurable(TransactionF* transaction){
    list<Content*> listOfOperations = transaction->getReadSet();

    //for(Content* c : listOfOperations){
        //storageSystem.insert(std::pair<uint64_t, Content>(c.getKey(), c));
    //}
    
    listOfOperations=transaction->getWriteSet();

    //for(Content* c : listOfOperations){
        //storageSystem.insert(std::pair<uint64_t, Content>(c.getKey(), c));
    //}
    
}

void Framework::initContent(row_t* row){ //Content has key = to the row primary key. This is displayed in the CCMap with key as primary key. EG. to retrieve
                                         // the concurrency control of a content just access with that key. 
    Content* c = new Content(row->get_primary_key(), row);
    InterfaceConcurrencyControl* concurrencyControl = configuration::initConcurrencyControl(c);
    concurrencyControlMap.insert(std::pair<uint64_t, InterfaceConcurrencyControl*>(c->getKey(), concurrencyControl));
    
}

void Framework::releaseLocksOfTransaction(TransactionF* transaction){
    for(uint64_t i : transaction->getLocksDetained()){
        concurrencyControlMap.at(i)->releaseControl(transaction);
    }
    transaction->clearLocksDetained();
}

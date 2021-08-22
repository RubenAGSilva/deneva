#include "framework.h"
#include "utils/content.h"
#include "utils/transaction.h"
#include "utils/concurrency.h"
#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "concurrency_manager.h"
#include <map>
#include <string>
#include <iostream>
#include "configuration.cpp"

using namespace std;

interfaceFramework::~interfaceFramework(){}

Framework::Framework(uint64_t nodeId1){
    nodeId = nodeId1;
    groupMembership = configuration::initGroupModule();
    Node* node = groupMembership->getNode((uint64_t) g_node_id);

    order = configuration::initOrderModule(node ,configuration::initClockVersion());
    replication = configuration::initReplicationModule();
    concurrencyManager = configuration::initConcurrencyManager();
    transactionMapMutex = new pthread_mutex_t;
    pthread_mutex_init(transactionMapMutex, NULL);
    

}

void Framework::beginTransaction(TransactionF* transaction){
    printf("Begin transaction %lu \n", transaction->getId());
    fflush(stdout);

    int nodeId = transaction->getNodeId();

    addTransaction(transaction);

    if(groupMembership->getNode(nodeId)->getRole() != Role::TIMESTAMPER){ 
        nodeId = groupMembership->getTimestamper(); // enviar para o no para fazer ts, e depois voltar aqui
    }

    // read(transaction, 1);
    // read(transaction, 4);
    // read(transaction, 5);
    // read(transaction, 6);
    // read(transaction, 7);
    // read(transaction, 8);
    // read(transaction, 15);
    // read(transaction, 16);
    // read(transaction, 17);
    //read(transaction, 18);
    order->timestampStartup(transaction);
    replication->replicate(transaction, groupMembership->getReplicationTargets(transaction->getNodeId()));
}

void Framework::endTransaction(TransactionF* transaction){
    if(transaction->isFinished()){
        concurrencyManager->finish(transaction);
        pthread_mutex_lock(transactionMapMutex);
        mapOfTransactions.erase(transaction->getId()); //remove active txn - cuz no longe active
        pthread_mutex_unlock(transactionMapMutex);
    
        //Operacoes para terminar uma transacao.
        printf("TRANSACTION FINISHING IS: %lu \n", transaction->getId());
        fflush(stdout);

        if(local(transaction)){
            printf("end transaction %lu \n", transaction->getId());
            printf("Startup ts %ld --- Commit ts %ld\n", transaction->getTimestampStartup(), transaction->getTimestampCommit());
            printf("___ Deleting transaction with id: %lu \n", transaction->getId());
            fflush(stdout);
        }
    }
}

void Framework::read(TransactionF* transaction, uint64_t key){
    
    try{ //try to retrieve
        
        concurrencyManager->read(transaction, key);

    }catch(std::out_of_range){

        printf("out of range\n");
        fflush(stdout);
    }

}
void Framework::write(TransactionF* transaction, uint64_t key, row_t* row){
    
    try{ //try to retrieve
        
        concurrencyManager->write(transaction, key, row);
    }
    catch(std::out_of_range)
    {
        initContent(row);
        write(transaction, key, row);
    }
    

}

bool Framework::local(TransactionF* transaction){
    printf("transaction nodeId = %lu | nodeId = %lu", transaction->getNodeId(), nodeId);
    fflush(stdout);
    if(transaction->getNodeId() == nodeId){
        return true;
    }
    return false;
}

void Framework::commit(TransactionF* transaction){
    
    if(validate(transaction)){ //para sistemas que nao precisam de validacao, return true
        concurrencyManager->commit(transaction); // every partition will commit their changes
        transaction->setFinished(true);  
    }

    if(local(transaction) && validate(transaction)){ // all other nodes already committed their changes
        order->timestampCommit(transaction);
        concurrencyManager->commit(transaction);
        list<Node*> nodes = groupMembership->getReplicationTargets(transaction->getNodeId());  
        transaction->setFinished(true);  
        replication->replicateResult(transaction, nodes);
        printf("commit transaction %lu with %lu operations:\n", transaction->getId(), transaction->getReadSet().size());
        fflush(stdout);
    }

}
void Framework::abort(TransactionF* transaction){

    if(local(transaction)){
        concurrencyManager->abort(transaction);
        transaction->setFinished(true);
        printf("Abort transaction %lu \n", transaction->getId());
        fflush(stdout);
    }else{
        concurrencyManager->abort(transaction);
        transaction->setFinished(true);
    }
    // UNDO, REDO, DISCARD, ?, etc.
}

bool Framework::validate(TransactionF* transaction){ 
    return concurrencyManager->validate(transaction);
}
void Framework::replicate(TransactionF* transaction){
    replication->replicate(transaction, groupMembership->getReplicationTargets(transaction->getNodeId()));
} //might be useful? 

void Framework::initContent(row_t* row){ //Content has key = to the row primary key. This is displayed in the CCMap with key as primary key. EG. to retrieve
                                         // the concurrency control of a content just access with that key. 
                                         
    concurrencyManager->initContent(row->get_primary_key(), row);
    
}

void Framework::addTransaction(TransactionF* transaction){
    
    pthread_mutex_lock(transactionMapMutex);
    try{
        mapOfTransactions.erase(transaction->getId());
        mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
    }catch(std::out_of_range){
        mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
    }
    pthread_mutex_unlock(transactionMapMutex);
}
TransactionF* Framework::getTransaction(uint64_t transactionId){
    printf("___ Getting transactionId: %lu\n", transactionId);
    fflush(stdout);
    
    try{
        //pthread_mutex_lock(transactionMapMutex); // prob unnecessary
        return mapOfTransactions.at(transactionId); //cannot return before releasing the lock
        //pthread_mutex_unlock(transactionMapMutex);
        
    }catch(std::out_of_range){ //getNode()->comm->getTxn(); if we dont have it, contact the node that has it and receive it
        assert(false);
        return NULL;
    }
}



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

FrameworkPessimisticDeneva::FrameworkPessimisticDeneva(uint64_t nodeId1){
    nodeId = nodeId1;
    groupMembership = configuration::initGroupModule();
    Node* node = groupMembership->getNode((uint64_t) g_node_id);

    order = configuration::initOrderModule(node ,configuration::initClockVersion());
    replication = configuration::initReplicationModule();
    concurrencyManager = configuration::initConcurrencyManager();
    transactionMapMutex = new pthread_mutex_t;
    pthread_mutex_init(transactionMapMutex, NULL);
    

}

void FrameworkPessimisticDeneva::beginTransaction(TransactionF* transaction){
    //printf("Begin transaction %lu \n", transaction->getId());
    //fflush(stdout);

    addTransaction(transaction);

    int nodeId = transaction->getNodeId();
    
    //--- test
    //sleep(2);
    

    if(groupMembership->getNode(nodeId)->getRole() != Role::TIMESTAMPER){ 
        nodeId = groupMembership->getTimestamper(); // enviar para o no para fazer ts, e depois voltar aqui
    }

    order->timestampStartup(transaction);
    replication->replicate(transaction, groupMembership->getReplicationTargets(transaction->getNodeId()));

    //stats
    transaction->statStartTs = get_sys_clock();
    INC_STATS(0, framework_txn_cnt, 1);
}

void FrameworkPessimisticDeneva::endTransaction(TransactionF* transaction){
    if(transaction->isFinished()){
        concurrencyManager->finish(transaction);
        pthread_mutex_lock(transactionMapMutex);
        //mapOfTransactions.erase(transaction->getId()); //remove active txn - cuz no longe active
        pthread_mutex_unlock(transactionMapMutex);
    
        //Operacoes para terminar uma transacao.
        //printf("TRANSACTION FINISHING IS: %lu \n", transaction->getId());
        //fflush(stdout);
        
        if(local(transaction)){
            //printf("end transaction %lu \n", transaction->getId());
            //printf("Startup ts %ld --- Commit ts %ld\n", transaction->getTimestampStartup(), transaction->getTimestampCommit());
            //fflush(stdout);
        }
    }
}

void FrameworkPessimisticDeneva::read(TransactionF* transaction, uint64_t key){
    
    try{ //try to retrieve
        
        concurrencyManager->read(transaction, key);
        //sleep(2); // --- test

    }catch(std::out_of_range){

        //printf("out of range\n");
        //fflush(stdout);
    }

}
void FrameworkPessimisticDeneva::write(TransactionF* transaction, uint64_t key, row_t* row){
    
    try{ //try to retrieve
        
        concurrencyManager->write(transaction, key, row);
        //sleep(2);// --- test
    }
    catch(std::out_of_range)
    {
        initContent(row);
        write(transaction, key, row);
    }
    

}

bool FrameworkPessimisticDeneva::local(TransactionF* transaction){
    // printf("transaction nodeId = %lu | nodeId = %lu", transaction->getNodeId(), nodeId);
    // fflush(stdout);
    if(transaction->getNodeId() == nodeId){
        return true;
    }
    return false;
}

void FrameworkPessimisticDeneva::commit(TransactionF* transaction){

    if(!local(transaction)){
        concurrencyManager->commit(transaction); // every partition will commit their changes
        transaction->setFinished(true); 
    }else{
        order->timestampCommit(transaction);
        concurrencyManager->commit(transaction);
        list<Node*> nodes = groupMembership->getReplicationTargets(transaction->getNodeId());  
        transaction->setFinished(true);
        
        //stat
        statsCommit(transaction);
        //  

        replication->replicateResult(transaction, nodes);
        //printf("commit transaction %lu with %lu operations:\n", transaction->getId(), (transaction->getReadSet().size() + transaction->getWriteSet().size()));
        //fflush(stdout);
    }

}
void FrameworkPessimisticDeneva::abort(TransactionF* transaction){

    if(local(transaction)){
        concurrencyManager->abort(transaction);
        transaction->setFinished(true);
        //printf("Abort transaction %lu \n", transaction->getId());
        //fflush(stdout);

        //stat
        INC_STATS(0, framework_local_abort_cnt, 1);
    }else{
        concurrencyManager->abort(transaction);
        transaction->setFinished(true);
    }
    // UNDO, REDO, DISCARD, ?, etc.
}

bool FrameworkPessimisticDeneva::validate(TransactionF* transaction){ 
    return true;
}
void FrameworkPessimisticDeneva::replicate(TransactionF* transaction){
    replication->replicate(transaction, groupMembership->getReplicationTargets(transaction->getNodeId()));
} //might be useful? 

void FrameworkPessimisticDeneva::initContent(row_t* row){ //Content has key = to the row primary key. This is displayed in the CCMap with key as primary key. EG. to retrieve
                                         // the concurrency control of a content just access with that key. 
                                         
    concurrencyManager->initContent(row->get_primary_key(), row);
    
}

void FrameworkPessimisticDeneva::addTransaction(TransactionF* transaction){
    
    pthread_mutex_lock(transactionMapMutex);
    try{
        mapOfTransactions.erase(transaction->getId());
        mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
    }catch(std::out_of_range){
        mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
    }
    pthread_mutex_unlock(transactionMapMutex);
}
TransactionF* FrameworkPessimisticDeneva::getTransaction(uint64_t transactionId){
    
    try{
        pthread_mutex_lock(transactionMapMutex); // prob unnecessary
        TransactionF* txn = mapOfTransactions.at(transactionId); //cannot return before releasing the lock
        pthread_mutex_unlock(transactionMapMutex);
        return txn;
        
    }catch(std::out_of_range){ //getNode()->comm->getTxn(); if we dont have it, contact the node that has it and receive it
        //assert(false);
        pthread_mutex_unlock(transactionMapMutex);
        sleep(0.5);
        return getTransaction(transactionId);
    }
}


void FrameworkPessimisticDeneva::statsCommit(TransactionF* transaction){
    uint64_t timeCommit = get_sys_clock();
    uint64_t timespan_long  = timeCommit - transaction->statStartTs;
    
    if(!local(transaction)){
        INC_STATS(0, framework_remote_commit_cnt, 1);
    }else{
        INC_STATS(0, framework_local_commit_cnt, 1);
        INC_STATS(0, framework_txn_runtime, timespan_long);
    }

}


//


FrameworkOptimisicDeneva::FrameworkOptimisicDeneva(uint64_t nodeId1){
    nodeId = nodeId1;
    groupMembership = configuration::initGroupModule();
    Node* node = groupMembership->getNode((uint64_t) g_node_id);

    order = configuration::initOrderModule(node ,configuration::initClockVersion());
    replication = configuration::initReplicationModule();
    concurrencyManager = configuration::initConcurrencyManager();
    transactionMapMutex = new pthread_mutex_t;
    pthread_mutex_init(transactionMapMutex, NULL);
    

}

void FrameworkOptimisicDeneva::beginTransaction(TransactionF* transaction){
    //printf("Begin transaction %lu \n", transaction->getId());
    //fflush(stdout);

    addTransaction(transaction);

    int nodeId = transaction->getNodeId();
    
    //--- test
    //sleep(2);
    //stats
    

    if(groupMembership->getNode(nodeId)->getRole() != Role::TIMESTAMPER){ 
        nodeId = groupMembership->getTimestamper(); // enviar para o no para fazer ts, e depois voltar aqui
    }

    order->timestampStartup(transaction);
    replication->replicate(transaction, groupMembership->getReplicationTargets(transaction->getNodeId()));

    //stats
    transaction->statStartTs = get_sys_clock();
    INC_STATS(0, framework_txn_cnt, 1);
}

void FrameworkOptimisicDeneva::endTransaction(TransactionF* transaction){
    if(transaction->isFinished()){
        concurrencyManager->finish(transaction);
        pthread_mutex_lock(transactionMapMutex);
        //mapOfTransactions.erase(transaction->getId()); //remove active txn - cuz no longe active
        pthread_mutex_unlock(transactionMapMutex);
    
        //Operacoes para terminar uma transacao.
        //printf("TRANSACTION FINISHING IS: %lu \n", transaction->getId());
        //fflush(stdout);
        
        if(local(transaction)){
            //printf("end transaction %lu \n", transaction->getId());
            //printf("Startup ts %ld --- Commit ts %ld\n", transaction->getTimestampStartup(), transaction->getTimestampCommit());
            //fflush(stdout);
        }
    }
}

void FrameworkOptimisicDeneva::read(TransactionF* transaction, uint64_t key){
    
    try{ //try to retrieve
        
        concurrencyManager->read(transaction, key);
        //sleep(2); // --- test

    }catch(std::out_of_range){

        //printf("out of range\n");
        //fflush(stdout);
    }

}
void FrameworkOptimisicDeneva::write(TransactionF* transaction, uint64_t key, row_t* row){
    
    try{ //try to retrieve
        
        concurrencyManager->write(transaction, key, row);
        //sleep(2);// --- test
    }
    catch(std::out_of_range)
    {
        initContent(row);
        write(transaction, key, row);
    }
    

}

bool FrameworkOptimisicDeneva::local(TransactionF* transaction){
    // printf("transaction nodeId = %lu | nodeId = %lu", transaction->getNodeId(), nodeId);
    // fflush(stdout);
    if(transaction->getNodeId() == nodeId){
        return true;
    }
    return false;
}

void FrameworkOptimisicDeneva::commit(TransactionF* transaction){

    if(!transaction->isValidated()){
        if(!validate(transaction)){return;}
    }

    if(!local(transaction)){
        concurrencyManager->commit(transaction); // every partition will commit their changes
        transaction->setFinished(true); 
    }else{
        order->timestampCommit(transaction);
        concurrencyManager->commit(transaction);
        list<Node*> nodes = groupMembership->getReplicationTargets(transaction->getNodeId());  
        transaction->setFinished(true);
        //stat
        statsCommit(transaction);
        //  

        replication->replicateResult(transaction, nodes);
        //printf("commit transaction %lu with %lu operations:\n", transaction->getId(), (transaction->getReadSet().size() + transaction->getWriteSet().size()));
        //fflush(stdout);
    }

}
void FrameworkOptimisicDeneva::abort(TransactionF* transaction){

    if(local(transaction)){
        concurrencyManager->abort(transaction);
        transaction->setFinished(true);
        //printf("Abort transaction %lu \n", transaction->getId());
        //fflush(stdout);

        //stat
        INC_STATS(0, framework_local_abort_cnt, 1);
    }else{
        concurrencyManager->abort(transaction);
        transaction->setFinished(true);
    }
    // UNDO, REDO, DISCARD, ?, etc.
}

bool FrameworkOptimisicDeneva::validate(TransactionF* transaction){ 
    bool valid = concurrencyManager->validate(transaction);
    if(!valid){
        //printf("------- ABORT THIS TXN: %lu\n", transaction->getId());
        //fflush(stdout);
        abort(transaction);
    }
    return valid;
    //return concurrencyManager->validate(transaction);
}
void FrameworkOptimisicDeneva::replicate(TransactionF* transaction){
    replication->replicate(transaction, groupMembership->getReplicationTargets(transaction->getNodeId()));
} //might be useful? 

void FrameworkOptimisicDeneva::initContent(row_t* row){ //Content has key = to the row primary key. This is displayed in the CCMap with key as primary key. EG. to retrieve
                                         // the concurrency control of a content just access with that key. 
                                         
    concurrencyManager->initContent(row->get_primary_key(), row);
    
}

void FrameworkOptimisicDeneva::addTransaction(TransactionF* transaction){
    
    pthread_mutex_lock(transactionMapMutex);
    try{
        mapOfTransactions.erase(transaction->getId());
        mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
    }catch(std::out_of_range){
        mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
    }
    pthread_mutex_unlock(transactionMapMutex);
}
TransactionF* FrameworkOptimisicDeneva::getTransaction(uint64_t transactionId){
    
    try{
        pthread_mutex_lock(transactionMapMutex); // prob unnecessary
        TransactionF* txn = mapOfTransactions.at(transactionId); //cannot return before releasing the lock
        pthread_mutex_unlock(transactionMapMutex);
        return txn;
        
    }catch(std::out_of_range){ //getNode()->comm->getTxn(); if we dont have it, contact the node that has it and receive it
        //assert(false);
        pthread_mutex_unlock(transactionMapMutex);
        sleep(0.5);
        return getTransaction(transactionId);
    }
}


void FrameworkOptimisicDeneva::statsCommit(TransactionF* transaction){
    uint64_t timeCommit = get_sys_clock();
    uint64_t timespan_long  = timeCommit - transaction->statStartTs;
    
    if(!local(transaction)){
        INC_STATS(0, framework_remote_commit_cnt, 1);
    }else{
        INC_STATS(0, framework_local_commit_cnt, 1);
        INC_STATS(0, framework_txn_runtime, timespan_long);
    }

}

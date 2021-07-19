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

    int nodeId = groupMembership->getNode()->getId();
    //if se estamos no groupMembership
    groupMembership->getNode(transaction->getNodeId())->addTransaction(transaction);

    if(groupMembership->getNode()->getRole() != Role::TIMESTAMPER){ 
        nodeId = groupMembership->getTimestamper();
    }

    transaction->setCoordinator(groupMembership->getCoordinator(transaction));
    order->timestampStartup(transaction, Metadata());
    replication->replicate(transaction, Metadata(), groupMembership->getReplicationTargets(transaction->getNodeId()));
}

void Framework::endTransaction(TransactionF* transaction){
    if(transaction->isFinished()){
        concurrencyController->finish(transaction);
        printf("end transaction %lu \n", transaction->getId());
        printf("transaction %lu terminated \n", transaction->getId());
        printf("Startup ts %ld --- Commit ts %ld\n", transaction->getTimestampStartup(), transaction->getTimestampCommit());
        fflush(stdout);
        
        //Operacoes para terminar uma transacao. retirar a transaction da lista? Ou assim.
    }
}

void Framework::read(TransactionF* transaction, uint64_t key){
    
    try{ //try to retrieve
        //if(local)
        concurrencyController->read(transaction, key);

    }catch(std::out_of_range){

        printf("out of range\n");
        fflush(stdout);
    }

}
void Framework::write(TransactionF* transaction, uint64_t key, row_t* row){
    
    try{ //try to retrieve
        //if(local)
        concurrencyController->write(transaction, key, row);
    }
    catch(std::out_of_range)
    {
        initContent(row);
        write(transaction, key, row);
    }
    

}
void Framework::commit(TransactionF* transaction){
    
    if(validate(transaction)){ //para sistemas que nao precisam de validacao, return true
        printf("commit transaction %lu with %lu operations:\n", transaction->getId(), transaction->getReadSet().size());
        fflush(stdout);

        order->timestampCommit(transaction, Metadata());
        concurrencyController->commit(transaction);

        list<Node*> nodes = groupMembership->getReplicationTargets(transaction->getNodeId());    
            
        replication->replicate(transaction, Metadata(), nodes);
        transaction->setFinished(true);
        
    }
}
void Framework::abort(TransactionF* transaction){
    printf("Abort transaction %lu \n", transaction->getId());
    fflush(stdout);
    transaction->setFinished(true);
    concurrencyController->abort(transaction);
    // UNDO, REDO, DISCARD, ?, etc.
}

bool Framework::validate(TransactionF* transaction){ 
    return concurrencyController->validate(transaction);
}
void Framework::replicate(TransactionF* transaction){
    replication->replicate(transaction, Metadata(), groupMembership->getReplicationTargets(transaction->getNodeId()));
}
TransactionF* Framework::getTransaction(uint64_t transactionId){
    map<uint64_t, Node *> mapOfNodes = groupMembership->getNodes();
    map<uint64_t, Node *>::iterator it;
    for(it = mapOfNodes.begin(); it!= mapOfNodes.end(); it++){
        try{return it->second->getMapOfTransactions().at(transactionId);}catch(std::out_of_range){}
    }
    return NULL;
}
TransactionF* Framework::getTransaction(uint64_t transactionId, uint64_t nodeId){
    return groupMembership->getNode(nodeId)->getTransaction(transactionId);
}

void Framework::initContent(row_t* row){ //Content has key = to the row primary key. This is displayed in the CCMap with key as primary key. EG. to retrieve
                                         // the concurrency control of a content just access with that key. 
                                         
    concurrencyController->initContent(row->get_primary_key(), row);
    
}



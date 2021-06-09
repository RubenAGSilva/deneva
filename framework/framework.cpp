#include "framework.h"
#include "utils/content.cpp"
#include "utils/transaction.h"
#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "validation.h"
#include <map>
#include <string>
#include <iostream>

using namespace std;

Framework::Framework(){
    //config file of which modules init.
    groupMembership = GroupMembership(Node(Role::TIMESTAMPER));
    order = Order();
    replication = Replication();
    validation = Validation();
}

void Framework::beginTransaction(TransactionF transaction){
    printf("Begin transaction %lu \n", transaction.getId());
    fflush(stdout);

    mapOfTransactions.insert(std::pair<uint64_t,TransactionF>(transaction.getId(), transaction));

    string nodeId = groupMembership.getNode().getId();
    if(groupMembership.getRole() != Role::TIMESTAMPER){
        nodeId = groupMembership.getTimestamper();
    }

    //assign the transaction to a coordinator.
    order.timestampStartup(transaction, Metadata());
    replication.replicate(transaction, Metadata());
}

void Framework::endTransaction(TransactionF transaction){
    if(transaction.isFinished()){
        printf("end transaction %lu \n", transaction.getId());
        printf("transaction %lu terminated \n", transaction.getId());
        printf("Storage system size: %lu\n", storageSystem.size());
        fflush(stdout);
        //Operacoes para terminar uma transacao. retirar a transaction da lista? Ou assim.
        //mapOfTransactions.erase(transaction.getId());
    }
}

void Framework::read(TransactionF transaction, int key){
    printf("read an op from the transaction %lu \n", transaction.getId());
    fflush(stdout);
    
    try{
        //int value = storageSystem.at(key);
        transaction.addToReadSet(key,1); //not working. 
        storageSystem[key]; //remove

    }catch(std::out_of_range){

        printf("out of range");
        fflush(stdout);
    }
    //addLockDetained()?

    //return value;
}
void Framework::write(TransactionF transaction, int64_t key, int value){
    printf("write an op from the transaction %lu \n", transaction.getId());
    fflush(stdout);

    transaction.addToWriteSet(key, value);
    //addLockDetained()?
}
void Framework::commit(TransactionF transaction){
    printf("commit transaction %lu with %lu operations\n", transaction.getId(), transaction.getReadSet().size());
    fflush(stdout);

    groupMembership.getCoordinator(transaction);
    
    if(!transaction.isValidated()){ //para sistemas que nao precisam de validacao, isValidated(){return true;}
        if(validation.validate(transaction)){
            order.timestampCommit(transaction, Metadata());
            replication.replicate(transaction, Metadata());
            replication.replicateResult(transaction, Metadata());
            transaction.setFinished(true);
            //makeDurable(transaction);
        }
    }else{
        // not validated, retry, abort??
    }

    endTransaction(transaction);
    
}
void Framework::abort(TransactionF transaction){
    printf("Abort transaction %lu \n", transaction.getId());
    fflush(stdout);
    transaction.setFinished(true);
    // UNDO, REDO, DISCARD, ?, etc.
}

void Framework::validate(TransactionF transaction){ 
    validation.validate(transaction);
}
void Framework::replicate(TransactionF transaction){
    replication.replicate(transaction, Metadata());
}
TransactionF Framework::getTransaction(uint64_t id){
    return mapOfTransactions[id];
}

void Framework::makeDurable(TransactionF transaction){
    list<Content> listOfOperations = transaction.getReadSet();
    for(list<Content>::iterator i = listOfOperations.begin(); i!= listOfOperations.end(); i++){
        storageSystem.insert(pair<int,int>(i->getKey() ,i->getValue()));
        printf("YAY\n");
        fflush(stdout);
    }
    
    listOfOperations=transaction.getWriteSet();

    for(list<Content>::iterator i = listOfOperations.begin(); i!= listOfOperations.end(); i++){
        storageSystem.insert(pair<int,int>(i->getKey() ,i->getValue()));
    }
    
}
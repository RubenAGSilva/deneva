#include "concurrency_controller.h"
#include "configuration.cpp"

void ConcurrencyControllerLocks::initContent(uint64_t key, row_t* row){
    Content* c = new Content(key, row);
    InterfaceConcurrencyControl* concurrencyControl = configuration::initConcurrencyControl(c);
    concurrencyControlMap.insert(std::pair<uint64_t, InterfaceConcurrencyControl*>(c->getKey(), concurrencyControl));
}

void ConcurrencyControllerLocks::write(TransactionF* transaction, uint64_t key, row_t* row){

    InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
    Content* content = new Content(key, row);
    Content * returnContent = concurrencyControl->write(transaction, content);
    
    if(returnContent->getValue() == NULL){ //couldnt get the lock
        delete returnContent;
        return;
    }

    #if ISOLATION_LEVEL == READ_UNCOMMITTED // release lock after write - only in read uncommitted
        concurrencyControl->releaseControl(transaction);
    #endif
}

void ConcurrencyControllerLocks::read(TransactionF* transaction, uint64_t key){

    printf("----a |  %lu\n", key);
    fflush(stdout);
    InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
    fflush(stdout);
    Content* returnContent = concurrencyControl->read(transaction); 
    printf("----b |  %lu\n", concurrencyControl->getContent()->getKey());

    if(returnContent->getValue() == NULL){ //couldnt get the lock
        delete returnContent;
        return;
    }

    #if ISOLATION_LEVEL == READ_COMMITTED || ISOLATION_LEVEL == READ_UNCOMMITTED // release lock after read
        concurrencyControl->releaseControl(transaction);
    #endif

}

bool ConcurrencyControllerLocks::validate(TransactionF* transaction){
    return true;
}

void ConcurrencyControllerLocks::commit(TransactionF* transaction){
    makeDurable(transaction);
    releaseLocks(transaction);
}

void ConcurrencyControllerLocks::abort(TransactionF* transaction){
    finish(transaction);
}

void ConcurrencyControllerLocks::finish(TransactionF* transaction){
    if(transaction->getLocksDetained().size()>0)
        releaseLocks(transaction);

    transaction->clearSets();
    
}

void ConcurrencyControllerLocks::releaseLocks(TransactionF* transaction){
    for(uint64_t i : transaction->getLocksDetained()){
        try{
        InterfaceConcurrencyControl* a = concurrencyControlMap.at(i);
        a->releaseControl(transaction);
        }catch(std::out_of_range){
            printf("----! id: %lu | size: %lu | transaction: %lu\n", i, transaction->getLocksDetained().size(), transaction->getId());
            fflush(stdout);
            sleep(5000);
        }
    }
    transaction->clearLocksDetained();
}

void ConcurrencyControllerLocks::makeDurable(TransactionF* transaction){
    list<Content*> listOfOperations=transaction->getWriteSet();
    for(Content* c : listOfOperations){
        concurrencyControlMap.at(c->getKey())->setContent(c); //make durable the writes
    }
}


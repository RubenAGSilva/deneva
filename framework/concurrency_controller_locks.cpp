#include "concurrency_manager.h"
#include "configuration.cpp"

void ConcurrencyManagerLocks::initContent(uint64_t key, row_t* row){
    Content* c = new Content(key, row);
    InterfaceConcurrencyControl* concurrencyControl = configuration::initConcurrencyControl(c);
    concurrencyControlMap.insert(std::pair<uint64_t, InterfaceConcurrencyControl*>(c->getKey(), concurrencyControl)); //TODO maybe add a lock and unlock before insering
}

void ConcurrencyManagerLocks::write(TransactionF* transaction, uint64_t key, row_t* row){

    InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
    Content* content = new Content(key, row);
    Content * returnContent = concurrencyControl->write(transaction, content);
    printf("write %lu from the transaction %lu \n", returnContent->getValue()->get_primary_key(), transaction->getId());
    fflush(stdout);
    
    if(returnContent->getValue() == NULL){ //couldnt get the lock
        delete returnContent;
        return;
    }

    #if ISOLATION_LEVEL == READ_UNCOMMITTED // release lock after write - only in read uncommitted
        concurrencyControl->releaseControl(transaction);
    #endif
}

void ConcurrencyManagerLocks::read(TransactionF* transaction, uint64_t key){

    InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
    Content* returnContent = concurrencyControl->read(transaction); 
    printf("read %lu from the transaction %lu \n", returnContent->getValue()->get_primary_key(), transaction->getId());
    fflush(stdout);

    if(returnContent->getValue() == NULL){ //couldnt get the lock
        delete returnContent;
        return;
    }

    #if ISOLATION_LEVEL == READ_COMMITTED || ISOLATION_LEVEL == READ_UNCOMMITTED // release lock after read
        concurrencyControl->releaseControl(transaction);
    #endif

}

bool ConcurrencyManagerLocks::validate(TransactionF* transaction){
    transaction->setValidated(true);
    return true;
}

void ConcurrencyManagerLocks::commit(TransactionF* transaction){
    list<Content*> listOfOperations=transaction->getWriteSet();
    for(Content* c : listOfOperations){
        concurrencyControlMap.at(c->getKey())->commitWrites(); //remove previous content values
    }
    releaseLocks(transaction);
}

void ConcurrencyManagerLocks::abort(TransactionF* transaction){
    list<Content*> listOfOperations=transaction->getWriteSet();
    for(Content* c : listOfOperations){
        concurrencyControlMap.at(c->getKey())->abortWrites(); 
    }
    releaseLocks(transaction);
    
}

void ConcurrencyManagerLocks::finish(TransactionF* transaction){
    if(transaction->getLocksDetained().size()>0) //just to make sure
        releaseLocks(transaction);

    transaction->clearSets();
    
}

void ConcurrencyManagerLocks::releaseLocks(TransactionF* transaction){
    for(uint64_t i : transaction->getLocksDetained()){
        try{
        concurrencyControlMap.at(i)->releaseControl(transaction);
        }catch(std::out_of_range){
            printf("Out of range - bug?\n");
            fflush(stdout);
            sleep(50);
        }
    }
    transaction->clearLocksDetained();
}


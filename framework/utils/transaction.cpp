#include "transaction.h"
#include "metadata.cpp"
#include "content.cpp"
#include <list>
#include <string>

using namespace std;

TransactionF::TransactionF(uint64_t id1){
    metadata = Metadata();
    replicated = false;
    id=id1;
}

void TransactionF::addToReadSet(int64_t key, int value){
    readset.push_back(Content(key, value));
    //printf("Added to read set %lu \n", readset.size());
    //fflush(stdout);
}

void TransactionF::addToWriteSet(int64_t key, int value){
    writeset.push_back(Content(key, value));
}

void TransactionF::addLockDetained(int lock){
    locksDetained.push_back(lock);
}


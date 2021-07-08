#include "transaction.h"
#include "metadata.cpp"
#include "content.h"
#include <list>
#include <string>

using namespace std;

TransactionF::TransactionF(uint64_t id1){
    metadata = Metadata();
    replicated = false;
    id=id1;
}

void TransactionF::addToReadSet(Content* content){
    readset.push_back(content);
    //printf("Added to read set %lu \n", readset.size());
    //fflush(stdout);
}

void TransactionF::addToWriteSet(Content* content){
    writeset.push_back(content);
}

void TransactionF::addLockDetained(uint64_t lock){
    locksDetained.push_back(lock);
}

uint64_t TransactionF::incr_lr() {
  //ATOM_ADD(this->rsp_cnt,i);
  uint64_t result;
  sem_wait(&rsp_mutex);
  result = ++lock_ready_cnt;
  sem_post(&rsp_mutex);
  return result;
}

uint64_t TransactionF::decr_lr() {
  //ATOM_SUB(this->rsp_cnt,i);
  uint64_t result;
  sem_wait(&rsp_mutex);
  result = --this->lock_ready_cnt;
  sem_post(&rsp_mutex);
  return result;
}


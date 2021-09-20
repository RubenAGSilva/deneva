#include "transaction.h"
#include "content.h"
#include <list>
#include <string>

using namespace std;

TransactionF::TransactionF(uint64_t id1, uint64_t newNodeId){
    replicated = false;
    id=id1;
    nodeId = newNodeId;
    locksDetained = std::list<uint64_t>();
    timestampCommit = 0;
}

void TransactionF::addToReadSet(Content* content){
    readset.push_back(content);
    //printf("adding lock :%lu on transaction : %lu\n",content->getKey(), id);
    //fflush(stdout);
    locksDetained.push_back(content->getKey());
}

void TransactionF::addToWriteSet(Content* content){
    writeset.push_back(content);
    //printf("adding lock :%lu\n",content->getKey());
    //fflush(stdout);
    locksDetained.push_back(content->getKey());
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

static bool deleteAll(Content* elem){return true;}

void TransactionF::clearSets(){
  readset.remove_if(deleteAll);
  writeset.remove_if(deleteAll);
}


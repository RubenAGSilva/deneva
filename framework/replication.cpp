#include "replication.h"
#include "utils/transaction.h"
#include "utils/metadata.cpp"
#include "utils/node.h"
#include <iostream>

using namespace std;

void Replication::replicate(TransactionF* transaction, Metadata metadata, list<Node>* nodes){
    printf("replicate transaction %lu\n", transaction->getId());
    fflush(stdout);
}
void Replication::replicateResult(TransactionF* transaction, Metadata result, list<Node>* nodes){
    printf("replicate transaction result %lu\n", transaction->getId());
    fflush(stdout);
}
        
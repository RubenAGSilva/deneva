#include "replication.h"
#include "utils/transaction.h"
#include "utils/metadata.cpp"
#include <iostream>

using namespace std;

void Replication::replicate(TransactionF transaction, Metadata metadata){
    printf("replicate transaction %lu \n", transaction.getId());
    fflush(stdout);
}
void Replication::replicateResult(TransactionF transaction, Metadata result){
    printf("replicate transaction result %lu \n", transaction.getId());
    fflush(stdout);
}
        
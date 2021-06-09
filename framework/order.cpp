#include "order.h"
#include "utils/transaction.h"
#include "utils/metadata.cpp"
#include <iostream>

using namespace std;

void Order::timestampStartup(TransactionF transaction, Metadata metadata){
     printf("timestamp transaction startup %lu \n", transaction.getId());
     fflush(stdout);
}
void Order::timestampCommit(TransactionF transaction, Metadata metadata){
     printf("timestamp transaction commit %lu \n", transaction.getId());
     fflush(stdout);
}
TransactionF Order::compareTransactions(TransactionF transaction1, TransactionF transaction2){
    return transaction1;
}
#include "order.h"
#include "utils/transaction.h"
#include "utils/metadata.cpp"
#include "utils/version.h"
#include <iostream>

using namespace std;

Order::Order(Node* node1, InterfaceVersion* clockVersion){
     node = node1;
     clock = clockVersion;
}


void Order::timestampStartup(TransactionF* transaction, Metadata metadata){
     printf("timestamp transaction startup %lu \n", transaction->getId());
     fflush(stdout);

     clock->updateClock();
     transaction->setTimestampStart(clock->getTime());
}
void Order::timestampCommit(TransactionF* transaction, Metadata metadata){
     printf("timestamp transaction commit %lu \n", transaction->getId());
     fflush(stdout);

     clock->updateClock();
     transaction->setTimestampCommit(clock->getTime());
}
TransactionF* Order::compareTransactions(TransactionF* transaction1, TransactionF* transaction2){
     Occurred o = clock->compareTimestamps(transaction1->getTimestampStartup(), transaction2->getTimestampStartup());
     switch(o){
          case AFTER:
               return transaction2;
          case BEFORE:
               return transaction1;
          case TIE:
               return transaction1;
          case CONCURRENTLY:
               return transaction1;
          default:
               return transaction1;
     }
}
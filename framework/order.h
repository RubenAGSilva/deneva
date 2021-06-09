#ifndef ORDER_H
#define ORDER_H

#include "utils/transaction.h"
#include "utils/metadata.cpp"

class Order{
    public:
        void timestampStartup(TransactionF transaction, Metadata metadata);
        void timestampCommit(TransactionF transaction, Metadata metadata);
        TransactionF compareTransactions(TransactionF transaction1, TransactionF transaction2);
};
#endif
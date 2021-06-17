#ifndef ORDER_H
#define ORDER_H

#include "utils/transaction.h"
#include "utils/metadata.cpp"

class interfaceOrder{
    public:
        virtual ~interfaceOrder(){}
        virtual void timestampStartup(TransactionF transaction, Metadata metadata) = 0;
        virtual void timestampCommit(TransactionF transaction, Metadata metadata) = 0;
        virtual TransactionF compareTransactions(TransactionF transaction1, TransactionF transaction2) = 0;

};

class Order : public interfaceOrder{
    public:
        virtual ~Order(){}
        void timestampStartup(TransactionF transaction, Metadata metadata) override;
        void timestampCommit(TransactionF transaction, Metadata metadata) override;
        TransactionF compareTransactions(TransactionF transaction1, TransactionF transaction2) override;
};
#endif
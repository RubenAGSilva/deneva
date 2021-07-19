#ifndef ORDER_H
#define ORDER_H

#include "utils/transaction.h"
#include "utils/metadata.cpp"
#include "utils/node.h"
#include "utils/version.h"

class interfaceOrder{
    public:
        virtual ~interfaceOrder(){}
        virtual void timestampStartup(TransactionF* transaction, Metadata metadata) = 0;
        virtual void timestampCommit(TransactionF* transaction, Metadata metadata) = 0;
        virtual TransactionF* compareTransactions(TransactionF* transaction1, TransactionF* transaction2) = 0;
        //for a centralized protocol
        virtual void lockGet(Content* content) = 0;
        virtual void lockRelease(Content* content) = 0;
};

class Order : public interfaceOrder{

    private:
        Node* node; //necessary ?
        InterfaceVersion* clock;

    public:
        ~Order(){}
        Order(Node* node1, InterfaceVersion* clockVersion);
        Order() = default;

        void lockGet(Content* content);
        void lockRelease(Content* content);
        void timestampStartup(TransactionF* transaction, Metadata metadata) override;
        void timestampCommit(TransactionF* transaction, Metadata metadata) override;
        TransactionF* compareTransactions(TransactionF* transaction1, TransactionF* transaction2) override;
};
#endif
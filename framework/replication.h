#ifndef REPLICATION_H
#define REPLICATION_H

#include "utils/transaction.h"
#include "utils/node.h"

class interfaceReplication{
    public:
        virtual ~interfaceReplication(){}
        virtual void replicate(TransactionF* transaction, list<Node*> nodes) = 0;
        virtual void replicateResult(TransactionF* transaction, list<Node*> nodes) = 0;
};

class Replication : public interfaceReplication{
    public:
        virtual ~Replication(){}
        void replicate(TransactionF* transaction, list<Node*> nodes) override;
        void replicateResult(TransactionF* transaction, list<Node*> nodes) override;
};
#endif
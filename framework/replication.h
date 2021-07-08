#ifndef REPLICATION_H
#define REPLICATION_H

#include "utils/transaction.h"
#include "utils/metadata.cpp"
#include "utils/node.h"

class interfaceReplication{
    public:
        virtual ~interfaceReplication(){}
        virtual void replicate(TransactionF* transaction, Metadata metadata, list<Node>* nodes) = 0;
        virtual void replicateResult(TransactionF* transaction, Metadata result, list<Node>* nodes) = 0;
};

class Replication : public interfaceReplication{
    public:
        virtual ~Replication(){}
        void replicate(TransactionF* transaction, Metadata metadata, list<Node>* nodes) override;
        void replicateResult(TransactionF* transaction, Metadata result, list<Node>* nodes) override;
};
#endif
#ifndef REPLICATION_H
#define REPLICATION_H

#include "utils/transaction.h"
#include "utils/metadata.cpp"

class interfaceReplication{
    public:
        virtual ~interfaceReplication(){}
        virtual void replicate(TransactionF transaction, Metadata metadata) = 0;
        virtual void replicateResult(TransactionF transaction, Metadata result) = 0;
};

class Replication : public interfaceReplication{
    public:
        virtual ~Replication(){}
        void replicate(TransactionF transaction, Metadata metadata) override;
        void replicateResult(TransactionF transaction, Metadata result) override;
};
#endif
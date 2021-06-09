#ifndef REPLICATION_H
#define REPLICATION_H

#include "utils/transaction.h"
#include "utils/metadata.cpp"

class Replication{
    public:
        void replicate(TransactionF transaction, Metadata metadata);
        void replicateResult(TransactionF transaction, Metadata result);
        
};
#endif
#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "utils/content.cpp"
#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "validation.h"
#include "utils/transaction.h"
#include <map>
#include <list>
#include <string>

class Framework {

    private:
        GroupMembership groupMembership;
        Order order;
        Replication replication;
        Validation validation;
        map<uint64_t,TransactionF> mapOfTransactions;
        map<int, int> storageSystem;

        void makeDurable(TransactionF transaction);

    public:
        Framework();
        void beginTransaction(TransactionF transaction);
        void endTransaction(TransactionF transaction);
        void read(TransactionF transaction, int key);
        void write(TransactionF transaction, int64_t key, int value);
        void commit(TransactionF transaction);
        void abort(TransactionF transaction);
        void validate(TransactionF transaction);
        void replicate(TransactionF transaction);
        map<uint64_t,TransactionF> getMapOfTransactions(){return mapOfTransactions;}
        TransactionF getTransaction(uint64_t id);

};
#endif
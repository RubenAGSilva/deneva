#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "utils/content.h"
#include "group_membership.h"
#include "order.h"
#include "replication.h"
#include "utils/transaction.h"
#include <map>
#include <list>
#include <string>
#include "global.h"
#include "utils/concurrency.h"
#include "../storage/row.h"

//class row_t;

class InterfaceConcurrencyControl;
class interfaceGroupMembership;
class interfaceOrder;
class interfaceReplication;
class InterfaceConcurrencyController;

class interfaceFramework{

    public:
        virtual void beginTransaction(TransactionF* transaction) = 0;
        virtual void endTransaction(TransactionF* transaction) = 0;
        virtual void read(TransactionF* transaction, uint64_t key) = 0;
        virtual void write(TransactionF* transaction, uint64_t key, row_t* row) = 0;
        virtual void commit(TransactionF* transaction) = 0;
        virtual void abort(TransactionF* transaction) = 0;
        virtual bool validate(TransactionF* transaction) = 0;
        virtual void replicate(TransactionF* transaction) = 0;
        virtual std::map<uint64_t,TransactionF*> getMapOfTransactions() = 0;
        virtual TransactionF* getTransaction(uint64_t id) = 0;
        virtual void initContent(row_t* row) = 0;
};


class Framework : public interfaceFramework{

    private:
        interfaceGroupMembership* groupMembership;
        interfaceOrder* order;
        interfaceReplication* replication;
        InterfaceConcurrencyController* concurrencyController;
        map<uint64_t,TransactionF*> mapOfTransactions;

        //map<uint64_t, InterfaceConcurrencyControl*> concurrencyControlMap;

    public:
        Framework();
        void beginTransaction(TransactionF* transaction) override;
        void endTransaction(TransactionF* transaction) override;
        void read(TransactionF* transaction, uint64_t key) override;
        void write(TransactionF* transaction, uint64_t key, row_t* row) override;
        void commit(TransactionF* transaction) override;
        void abort(TransactionF* transaction) override;
        bool validate(TransactionF* transaction) override;
        void replicate(TransactionF* transaction) override;
        map<uint64_t,TransactionF*> getMapOfTransactions() override {return mapOfTransactions;}
        TransactionF* getTransaction(uint64_t id) override;
        void initContent(row_t* row) override;

};
#endif
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

class row_t;

class InterfaceConcurrencyControl;
class interfaceGroupMembership;
class interfaceOrder;
class interfaceReplication;
class InterfaceConcurrencyManager;

class interfaceFramework{

    public:
        virtual ~interfaceFramework();
        virtual void beginTransaction(TransactionF* transaction) = 0;
        virtual void endTransaction(TransactionF* transaction) = 0;
        virtual void read(TransactionF* transaction, uint64_t key) = 0;
        virtual void write(TransactionF* transaction, uint64_t key, row_t* row) = 0;
        virtual void commit(TransactionF* transaction) = 0;
        virtual void abort(TransactionF* transaction) = 0;
        virtual bool validate(TransactionF* transaction) = 0;
        virtual void replicate(TransactionF* transaction) = 0;
        
        virtual TransactionF* getTransaction(uint64_t transactionId) = 0;
        virtual void addTransaction(TransactionF* transaction) = 0;
        virtual void initContent(row_t* row) = 0;
};

class FrameworkPessimisticDeneva : public interfaceFramework{

    private:
        interfaceReplication* replication;
        InterfaceConcurrencyManager* concurrencyManager;
        uint64_t nodeId;
        map<uint64_t,TransactionF*> mapOfTransactions;
        pthread_mutex_t* transactionMapMutex;
        bool local(TransactionF* transaction); 

    public:
        FrameworkPessimisticDeneva(uint64_t nodeId1);
        ~FrameworkPessimisticDeneva(){}
        void beginTransaction(TransactionF* transaction) override;
        void endTransaction(TransactionF* transaction) override;
        void read(TransactionF* transaction, uint64_t key) override;
        void write(TransactionF* transaction, uint64_t key, row_t* row) override;
        void commit(TransactionF* transaction) override;
        void abort(TransactionF* transaction) override;
        bool validate(TransactionF* transaction) override;
        void replicate(TransactionF* transaction) override;
        void initContent(row_t* row) override;

        map<uint64_t,TransactionF*> getMapOfTransactions(){return mapOfTransactions;}
        void addTransaction(TransactionF* transaction) override;
        TransactionF* getTransaction(uint64_t transactionId) override;
        void statsCommit(TransactionF* transaction);
};

class FrameworkOptimisicDeneva : public interfaceFramework{
    private:
        interfaceReplication* replication;
        InterfaceConcurrencyManager* concurrencyManager;
        uint64_t nodeId;
        map<uint64_t,TransactionF*> mapOfTransactions;
        pthread_mutex_t* transactionMapMutex;
        bool local(TransactionF* transaction); 

    public:
        FrameworkOptimisicDeneva(uint64_t nodeId1);
        ~FrameworkOptimisicDeneva(){}
        void beginTransaction(TransactionF* transaction) override;
        void endTransaction(TransactionF* transaction) override;
        void read(TransactionF* transaction, uint64_t key) override;
        void write(TransactionF* transaction, uint64_t key, row_t* row) override;
        void commit(TransactionF* transaction) override;
        void abort(TransactionF* transaction) override;
        bool validate(TransactionF* transaction) override;
        void replicate(TransactionF* transaction) override;
        void initContent(row_t* row) override;

        map<uint64_t,TransactionF*> getMapOfTransactions(){return mapOfTransactions;}
        void addTransaction(TransactionF* transaction) override;
        TransactionF* getTransaction(uint64_t transactionId) override;
        void statsCommit(TransactionF* transaction);
};
#endif
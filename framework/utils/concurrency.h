#ifndef CONCURRENCY_H
#define CONCURRENCY_H

#include "global.h"
#include "content.h"
#include "transaction.h"
#include "order.h"

// enum lock_t {LOCK_EX = 0, LOCK_SH, LOCK_NONE };
// typedef uint64_t ts_t; // time stamp type
// enum access_t {RD, WR, XP, SCAN};
// typedef uint32_t UInt32;

class InterfaceConcurrencyControl{


    public:
        virtual Content* read(TransactionF* transaction1) = 0;
        virtual Content* write(TransactionF* transaction1, Content* content1) = 0;
        virtual bool getControl(TransactionF* transaction1, access_t operation) = 0;
        virtual void releaseControl(TransactionF* transaction1) = 0;
        virtual bool validate(TransactionF* transaction1) = 0;
        virtual void abortWrites() = 0;
        virtual void commitWrites() = 0;
        virtual Content* getContent()=0;
};

struct CC_Entry{ // struct used by all CC algorithms - each CC might not use some of the attributes.
    lock_t type; //Lock
    ts_t   ts;
    TransactionF* transaction;
	CC_Entry* next;
	CC_Entry* prev;

    // Content ** rows; //[MAX_WRITE_SET]; OCC
    // UInt32 set_size; // OCC
    // UInt64 tn; //OCC
};

class CC_Lock : public InterfaceConcurrencyControl{

    private:
        Content* content;
        Content* previousContent;
        pthread_mutex_t* latch;
        lock_t lockType;

        UInt32 owner_cnt;
        UInt32 waiter_cnt;
        CC_Entry ** owners;	
        uint64_t owners_size;
        CC_Entry * waiters_head;
        CC_Entry * waiters_tail;
        uint64_t max_owner_ts;

        CC_Entry * get_entry();
        void return_entry(CC_Entry * entry);
        uint64_t hash(uint64_t id) {return id % owners_size;};
        Content* return_invalidContent();

    public:
        CC_Lock(Content* c);
        Content* read(TransactionF* transaction1) override;
        Content* write(TransactionF* transaction1, Content* content1) override;
        bool getControl(TransactionF* transaction1, access_t operation) override;
        void releaseControl(TransactionF* transaction1) override;
        bool validate(TransactionF* transaction1) override;
        void abortWrites() override;
        void commitWrites() override;
        bool conflict_lock(lock_t l1, lock_t l2);
        lock_t getOperationLock(access_t operation);

        Content* getContent()override{return content;}
        pthread_mutex_t* getMutex(){return latch;}
        lock_t getLockType(){return lockType;}

};

class CC_O : public InterfaceConcurrencyControl{
    private:
 	    pthread_mutex_t * _latch;
        sem_t _semaphore;
        bool blatch;
        ts_t wts; // the last update time

        Content* content;
        Content* previousContent;

        

    public:
        CC_O(Content* content1);
        Content* read(TransactionF* transaction1) override;
        Content* write(TransactionF* transaction1, Content* content1) override;
        bool getControl(TransactionF* transaction1, access_t operation) override;
        void releaseControl(TransactionF* transaction1) override;
        bool validate(TransactionF* transaction1) override;
        void abortWrites() override;
        void commitWrites() override;
        Content* getContent()override{return content;}
};
class CC_TS : public InterfaceConcurrencyControl{
    private:


    public:
        Content* read(TransactionF* transaction1) override;
        Content* write(TransactionF* transaction1, Content* content1) override;
        bool getControl(TransactionF* transaction1, access_t operation) override;
        void releaseControl(TransactionF* transaction1) override;
        bool validate(TransactionF* transaction1) override;
        void abortWrites() override;
        void commitWrites() override;
};
class CC_MVCC : public InterfaceConcurrencyControl{
    private:


    public:
        Content* read(TransactionF* transaction1) override;
        Content* write(TransactionF* transaction1, Content* content1) override;
        bool getControl(TransactionF* transaction1, access_t operation) override;
        void releaseControl(TransactionF* transaction1) override;
        bool validate(TransactionF* transaction1) override;
        void abortWrites() override;
        void commitWrites() override;
};
class CC_MAAT : public InterfaceConcurrencyControl{
    private:


    public:
        Content* read(TransactionF* transaction1) override;
        Content* write(TransactionF* transaction1, Content* content1) override;
        bool getControl(TransactionF* transaction1, access_t operation) override;
        void releaseControl(TransactionF* transaction1) override;
        bool validate(TransactionF* transaction1) override;
        void abortWrites() override;
        void commitWrites() override;
        
};

#endif
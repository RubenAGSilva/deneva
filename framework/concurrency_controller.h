#ifndef CONCURRENCY_CONTROLLER_H
#define CONCURRENCY_CONTROLLER_H

#include "utils/transaction.h"
#include "utils/concurrency.h"
#include "mem_alloc.h"

class InterfaceConcurrencyController{
    public:
        virtual void initContent(uint64_t key, row_t* row)=0;
        virtual void write(TransactionF* transaction, uint64_t key, row_t* row)=0;
        virtual void read(TransactionF* transaction, uint64_t key)=0;
        virtual bool validate(TransactionF* transaction)=0;
        virtual void commit(TransactionF* transaction)=0;
        virtual void abort(TransactionF* transaction)=0;
        virtual void finish(TransactionF* transaction)=0;
        

};

class ConcurrencyControllerLocks : public InterfaceConcurrencyController{

    private:
        map<uint64_t, InterfaceConcurrencyControl*> concurrencyControlMap;

        void releaseLocks(TransactionF* transaction);
        void makeDurable(TransactionF* transaction);
    public:

        void initContent(uint64_t key, row_t* row) override;
        void write(TransactionF* transaction, uint64_t key, row_t* row) override;
        void read(TransactionF* transaction, uint64_t key) override;
        bool validate(TransactionF* transaction) override;
        void commit(TransactionF* transaction) override;
        void abort(TransactionF* transaction) override;
        void finish(TransactionF* transaction) override;
};

class ConcurrencyControllerOtimistic : public InterfaceConcurrencyController{
    
    class set_ent{
        public:
            set_ent();
            UInt64 tn;
            TransactionF * txn;
            UInt32 set_size;
            Content ** rows; //[MAX_WRITE_SET];
            set_ent * next;
    };
    
    private:
        map<uint64_t, InterfaceConcurrencyControl*> concurrencyControlMap;

        pthread_mutex_t latch;
 	    sem_t _semaphore;
        set_ent * history;
	    set_ent * active;
        uint64_t his_len;
        uint64_t active_len;
        volatile uint64_t tnc; // transaction number counter
        volatile bool lock_all;
	    uint64_t lock_txn_id;

        void makeDurable(TransactionF* transaction);
        bool per_row_validate(TransactionF * transaction); // per row validation similar to Hekaton.
        bool central_validate(TransactionF * transaction); // parallel validation in the original OCC paper.
        void per_row_finish(TransactionF * transaction);
        void central_finish(TransactionF * transaction);

        void get_rw_set(TransactionF * transaction, set_ent * &rset, set_ent *& wset);
        bool test_valid(set_ent * set1, set_ent * set2);
    public:

        ConcurrencyControllerOtimistic();
        ~ConcurrencyControllerOtimistic();
        void initContent(uint64_t key, row_t* row) override;
        void write(TransactionF* transaction, uint64_t key, row_t* row) override;
        void read(TransactionF* transaction, uint64_t key) override;
        bool validate(TransactionF* transaction) override;
        void commit(TransactionF* transaction) override;
        void abort(TransactionF* transaction) override;
        void finish(TransactionF* transaction) override;


};





#endif
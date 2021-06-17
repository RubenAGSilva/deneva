#include "utils/transaction.h"
#include <map>

class intFramework{
    public:
        int smt(){
            return 1;
        }

        virtual void beginTransaction(TransactionF transaction) = 0;
        virtual void endTransaction(TransactionF transaction) = 0;
        virtual void read(TransactionF transaction, int key) = 0;
        virtual void write(TransactionF transaction, int64_t key, int value) = 0;
        virtual void commit(TransactionF transaction) = 0;
        virtual void abort(TransactionF transaction) = 0;
        virtual void validate(TransactionF transaction) = 0;
        virtual void replicate(TransactionF transaction) = 0;
        virtual std::map<uint64_t,TransactionF> getMapOfTransactions() = 0;
        virtual TransactionF getTransaction(uint64_t id) = 0;
};
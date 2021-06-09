#ifndef VALIDATION_H
#define VALIDATION_H

#include "utils/content.cpp"
#include "utils/transaction.h"

class Validation{
    public:
        bool validate(TransactionF transaction);
        int acquireLocks(Content object);
};
#endif
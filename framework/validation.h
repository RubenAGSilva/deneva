#ifndef VALIDATION_H
#define VALIDATION_H

#include "utils/content.cpp"
#include "utils/transaction.h"

class interfaceValidation{
    
    public:
        virtual ~interfaceValidation(){}
        virtual bool validate(TransactionF transaction) = 0;
};


class Validation : public interfaceValidation{

    public:
        virtual ~Validation(){}
        bool validate(TransactionF transaction) override;
        int acquireLocks(Content object);
};
#endif
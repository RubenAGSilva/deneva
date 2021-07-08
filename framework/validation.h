#ifndef VALIDATION_H
#define VALIDATION_H

#include "utils/content.h"
#include "utils/transaction.h"

class Content;

class interfaceValidation{
    
    public:
        virtual ~interfaceValidation(){}
        virtual bool validate(TransactionF* transaction) = 0;
        //virtual bool checkConflict() = 0;
};


class Validation : public interfaceValidation{

    public:
        virtual ~Validation(){}
        bool validate(TransactionF* transaction) override;
        int acquireLocks(Content object);
};
#endif
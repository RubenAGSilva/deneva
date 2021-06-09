#include "validation.h"
#include "utils/transaction.h"
#include "utils/content.cpp"
#include <iostream>

using namespace std;

bool Validation::validate(TransactionF transaction){
    printf("validate transaction %lu \n", transaction.getId());
    fflush(stdout);
    transaction.setValidated(true);
    return transaction.isValidated();
}
int Validation::acquireLocks(Content object){
    printf("acquire locks \n");
    fflush(stdout);
    return 1;
}
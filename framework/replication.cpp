#include "replication.h"
#include "utils/transaction.h"
#include "utils/metadata.cpp"
#include "utils/node.h"
#include <iostream>

using namespace std;

void Replication::replicate(TransactionF* transaction, Metadata metadata, list<Node*> nodes){

    for(Node* node : nodes){
        if(node->getId() != transaction->getNodeId()){
            node->addTransaction(transaction); //overwrite of the transaction 
        }
    }
    printf("replicate transaction %lu\n", transaction->getId());
    fflush(stdout);
}
        
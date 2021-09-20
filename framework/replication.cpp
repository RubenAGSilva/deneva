#include "replication.h"
#include "utils/transaction.h"
#include "utils/node.h"
#include "communication.h"
#include <iostream>

using namespace std;

void Replication::replicate(TransactionF* transaction, list<Node*> nodes){
    Node * node = groupMembership->getNode(g_node_id);
    

    for(Node* targetNode : nodes){ // go to all nodes and add the transaction. what if it fails?
        if(targetNode->getId() != node->getId() && !targetNode->isClient){ // !isClient
            //printf("I am %lu and sending a msg to %lu\n", node->getId(), targetNode->getId());
            //fflush(stdout);
            MessageF message = MessageF(node->getId(), targetNode->getId(),*transaction, REPLICATE);
            node->communication->sendMessage(message, targetNode->communication);
        }
    }

    //printf("replicate transaction %lu\n", transaction->getId());
    //fflush(stdout);
    
}

void Replication::replicateResult(TransactionF* transaction, list<Node*> nodes){
    Node * node = groupMembership->getNode(g_node_id);
    
    for(Node* targetNode : nodes){ // go to all nodes and add the transaction. what if it fails?
        if(targetNode->getId() != node->getId() && !targetNode->isClient){ // !isClient
            //printf("I am %lu and sending a msg to %lu\n", node->getId(), targetNode->getId());
            //fflush(stdout);
            MessageF message = MessageF(node->getId(), targetNode->getId(),*transaction, REPLICATE_WRITES);
            node->communication->sendMessage(message, targetNode->communication);
        }
    }

    //printf("replicate result from %lu\n", transaction->getId());
    //fflush(stdout);

}
        
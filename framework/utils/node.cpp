#include <string>
#include "node.h"
#include <list>

using namespace std;

        Node::Node(Role role){
            role=role;
        }

        Node::Node(uint64_t id1, Role role1, string host1, int httpPort1, int socketPort1, int adminPort1, list<int> partitions1){
            id = id1;
            role = role1;
            host = host1;
            httpPort = httpPort1;
            socketPort = socketPort1;
            adminPort = adminPort1;
            partitions = partitions1;
        }
        Node::Node(uint64_t id1, Role role1, string host1, int httpPort1, int socketPort1, int adminPort1){
            id = id1;
            role = role1;
            host = host1;
            httpPort = httpPort1;
            socketPort = socketPort1;
            adminPort = adminPort1;
        }
        
        uint64_t Node::getId(){
            return id;
        }

        int Node::getZoneId(){
            return zoneId;
        } 

        string Node::getHost(){
            return host;
        }

        int Node::getHttpPort(){
            return httpPort;
        }

        int Node::getSocketPort(){
            return socketPort;
        }
        
        int Node::getAdminPort(){
            return adminPort;
        }
        
        Role Node::getRole(){
            return role;
        }
        list<int> Node::getPartitionsIds(){
            return partitions;
        }
        void Node::addTransaction(TransactionF* transaction){
            try{
                mapOfTransactions.erase(transaction->getId());
                mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
            }catch(std::out_of_range){
                mapOfTransactions.insert(std::pair<uint64_t,TransactionF*>(transaction->getId(), transaction));
            }
        }
        TransactionF* Node::getTransaction(uint64_t transactionId){
            return mapOfTransactions.at(transactionId);
        }
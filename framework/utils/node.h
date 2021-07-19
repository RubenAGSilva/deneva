#ifndef NODE_H
#define NODE_H

#include <string>
#include <list>
#include "transaction.h"
#include "../../transport/transport.h"


using namespace std;

enum Role{
    TIMESTAMPER, COORDINATOR, FORWARDER
};

class Node{
    private:
        uint64_t id;
        int zoneId;
        string host;
        int httpPort;
        int socketPort;
        int adminPort;
        list<int> partitions;
        Role role;
        Socket* recvSocket;
        Socket* sendSocket;

        map<uint64_t,TransactionF*> mapOfTransactions;
    public:

        Node(Role role);
        Node(uint64_t id, Role role, string host, int httpPort, int socketPort, int adminPort, list<int> partitions);
        Node(uint64_t id, Role role, string host, int httpPort, int socketPort, int adminPort);
        Node() = default;
        
        uint64_t getId();
        int getZoneId();
        string getHost();
        int getHttpPort();
        int getSocketPort();    
        int getAdminPort();    
        Role getRole();
        list<int> getPartitionsIds();
        void setRecvSocket(Socket* socket){recvSocket=socket;}
        void setSendSocket(Socket* socket){sendSocket=socket;}

        map<uint64_t,TransactionF*> getMapOfTransactions(){return mapOfTransactions;}
        void addTransaction(TransactionF* transaction);
        TransactionF* getTransaction(uint64_t transactionId);
};

#endif
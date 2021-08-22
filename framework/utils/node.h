#ifndef NODE_H
#define NODE_H

#include <string>
#include <list>
#include "transaction.h"
#include "../../transport/transport.h"
#include "blocking_queue.h"

class InterfaceCommunication;
class Communication;

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
        BlockingQueue<Content> writesDone;
        

        
    public:

        InterfaceCommunication* communication;
        bool isClient;

        Node(Role role, uint64_t id, bool client);
        Node(uint64_t id, Role role, bool client, string host, int httpPort, int socketPort, int adminPort, list<int> partitions);
        Node(uint64_t id, Role role, bool client, string host, int httpPort, int socketPort, int adminPort);
        Node() = default;
        
        uint64_t getId();
        int getZoneId();
        string getHost();
        int getHttpPort();
        int getSocketPort();    
        int getAdminPort();    
        Role getRole();
        list<int> getPartitionsIds();
        void addContents(std::list<Content*> writeset);
};

#endif
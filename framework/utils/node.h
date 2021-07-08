#ifndef NODE_H
#define NODE_H

#include <string>
#include <list>

using namespace std;

enum Role{
    TIMESTAMPER, COORDINATOR, FORWARDER
};

class Node{
    private:
        int id;
        int zoneId;
        string host;
        int httpPort;
        int socketPort;
        int adminPort;
        list<int> partitions;
        Role role;
    public:

        Node(Role role);
        Node(int id, Role role, string host, int httpPort, int socketPort, int adminPort, list<int> partitions);
        Node(int id, Role role, string host, int httpPort, int socketPort, int adminPort);
        Node() = default;
        
        int getId();

        int getZoneId();

        string getHost();

        int getHttpPort();

        int getSocketPort();
        
        int getAdminPort();
        
        Role getRole();

        list<int> getPartitionsIds();
};

#endif
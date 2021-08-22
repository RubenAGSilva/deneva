#include <string>
#include "node.h"
#include <list>
#include "configuration.cpp"
#include "communication.h"

using namespace std;

Node::Node(Role role, uint64_t id1, bool client){
    role=role;
    id = id1;
    communication = configuration::initCommunication(id1);
    isClient = client;
}

Node::Node(uint64_t id1, Role role1, bool client, string host1, int httpPort1, int socketPort1, int adminPort1, list<int> partitions1){
    id = id1;
    role = role1;
    host = host1;
    httpPort = httpPort1;
    socketPort = socketPort1;
    adminPort = adminPort1;
    partitions = partitions1;
    communication = configuration::initCommunication(id1);
    isClient = client;
}
Node::Node(uint64_t id1, Role role1, bool client, string host1, int httpPort1, int socketPort1, int adminPort1){
    id = id1;
    role = role1;
    host = host1;
    httpPort = httpPort1;
    socketPort = socketPort1;
    adminPort = adminPort1;
    communication = configuration::initCommunication(id1);
    isClient = client;
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
void Node::addContents(std::list<Content*> writeset){ 
    //check ts so its ordered TODO change to transaction argument and change from blockingqueue to a lock
    for(Content* c : writeset){
        writesDone.push(*c);
    }
}
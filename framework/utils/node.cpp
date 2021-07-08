#include <string>
#include "node.h"
#include <list>

using namespace std;

        Node::Node(Role role){
            role=role;
        }

        Node::Node(int id1, Role role1, string host1, int httpPort1, int socketPort1, int adminPort1, list<int> partitions1){
            id = id1;
            role = role1;
            host = host1;
            httpPort = httpPort1;
            socketPort = socketPort1;
            adminPort = adminPort1;
            partitions = partitions1;
        }
        Node::Node(int id1, Role role1, string host1, int httpPort1, int socketPort1, int adminPort1){
            id = id1;
            role = role1;
            host = host1;
            httpPort = httpPort1;
            socketPort = socketPort1;
            adminPort = adminPort1;
        }
        
        int Node::getId(){
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
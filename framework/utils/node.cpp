#include <string>
#include "node.h"
#include <list>

using namespace std;

        Node::Node(Role role){
            role=role;
        }
        
        string Node::getId(){
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
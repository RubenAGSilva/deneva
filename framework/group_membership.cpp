#include "group_membership.h"
#include "utils/transaction.h"
#include "../system/global.h"
#include "node.h"
#include "transport.h"
#include <iostream>
#include <list>

using namespace std;

GroupMembership::GroupMembership(){
    string path = get_path();
	  read_ifconfig(path.c_str());
    initNodes();
}

void GroupMembership::read_ifconfig(const char * ifaddr_file) {

	ifaddr = new char *[g_total_node_cnt];

	uint64_t cnt = 0;
  printf("--Reading ifconfig file: %s\n",ifaddr_file);
	ifstream fin(ifaddr_file);
	string line;
  while (getline(fin, line)) {
		ifaddr[cnt] = new char[line.length()+1];
    strcpy(ifaddr[cnt],&line[0]);
		printf("%ld: %s\n",cnt,ifaddr[cnt]);
		cnt++;
	}
  assert(cnt == g_total_node_cnt);
}

string GroupMembership::get_path() {
	string path;
	char * cpath;
  cpath = getenv("SCHEMA_PATH");
	if(cpath == NULL)
		path = "./";
	else
		path = string(cpath);
	path += "ifconfig.txt";
  return path;

}

uint64_t GroupMembership::get_port_id(uint64_t src_node_id, uint64_t dest_node_id, uint64_t send_thread_id) {
  uint64_t port_id = 10;
  port_id += g_total_node_cnt * dest_node_id;
  port_id += src_node_id;
  port_id += send_thread_id * g_total_node_cnt * g_total_node_cnt;
  port_id += TPORT_PORT;
  printf("--Port ID:  %ld, %ld -> %ld : %ld\n",send_thread_id,src_node_id,dest_node_id,port_id);
  return port_id;
}

list<Node*> GroupMembership::getReplicationTargets(uint64_t nodeId){ 
    printf("get replication targets \n");
    fflush(stdout);

    list<Node*> replicaNodes = list<Node*>();
    map<uint64_t,Node*>::iterator it;

    for(it = mapOfNodes.begin(); it!= mapOfNodes.end(); it++){ 
        if(nodeId == it->second->getId()){continue;}

        replicaNodes.push_back(it->second); //How many should it replicate to? 
        break;
    }
    return replicaNodes;
}


int GroupMembership::getTimestamper(){
    printf("get timestamper \n");
    fflush(stdout);

    map<uint64_t,Node*>::iterator it;

    for(it = mapOfNodes.begin(); it!= mapOfNodes.end(); it++){
        if(it->second->getRole()==TIMESTAMPER){
            return it->second->getId();
        }
    }
  return -1;
}

int GroupMembership::getCoordinator(TransactionF* transaction){
    printf("get coordinator \n");
    fflush(stdout);

    map<uint64_t,Node*>::iterator it;

    for(it = mapOfNodes.begin(); it!= mapOfNodes.end(); it++){
        if(it->second->getRole()==COORDINATOR){
            return it->second->getId();
        }
    }

    return -1;
}

Node* GroupMembership::getNode(uint64_t nodeId){
  try{
    return mapOfNodes.at(nodeId);
  }catch(std::exception){
    printf("Node not found");
    fflush(stdout);
    sleep(50);
    return NULL;
  }
}

void GroupMembership::initNodes(){

  for(uint64_t node_id = 0; node_id < g_total_node_cnt; node_id++) {
    if(node_id == g_node_id){ //este no seria o que e local, portanto eu mesmo, sockets estao a null;
      if(ISCLIENTN(g_node_id)){
        Node* n =  new Node(g_node_id, TIMESTAMPER, true, "client" + g_node_id, 0, 0, 0);
        mapOfNodes.insert(std::pair<uint64_t, Node*>(node_id, n));
      }else{
        Node* n =  new Node(g_node_id, TIMESTAMPER, false, "server" + g_node_id, 0, 0, 0);
        mapOfNodes.insert(std::pair<uint64_t, Node*>(node_id, n));
      }
      continue;
    }
    // Listening ports
    if(ISCLIENTN(node_id)) {
      for(uint64_t client_thread_id = g_client_thread_cnt + g_client_rem_thread_cnt; client_thread_id < g_client_thread_cnt + g_client_rem_thread_cnt + g_client_send_thread_cnt; client_thread_id++) {
        uint64_t port_id = get_port_id(node_id,g_node_id,client_thread_id % g_client_send_thread_cnt);

        Node* n = new Node(node_id, TIMESTAMPER, true, "client" + node_id, port_id, port_id, port_id); //TODO change because this is executed for each thread instead of node
        n->communication->initRecvSocket(client_thread_id, ifaddr, true);
        mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));
        
      }
    } else {
      for(uint64_t server_thread_id = g_thread_cnt + g_rem_thread_cnt; server_thread_id < g_thread_cnt + g_rem_thread_cnt + g_send_thread_cnt; server_thread_id++) {
        uint64_t port_id = get_port_id(node_id,g_node_id,server_thread_id % g_send_thread_cnt);

        Node* n = new Node(node_id, TIMESTAMPER, false, "server" + node_id, port_id, port_id, port_id);
        n->communication->initRecvSocket(server_thread_id, ifaddr, false);
        mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));
      }
    }
    // Sending ports
    if(ISCLIENTN(g_node_id)) {
      for(uint64_t client_thread_id = g_client_thread_cnt + g_client_rem_thread_cnt; client_thread_id < g_client_thread_cnt + g_client_rem_thread_cnt + g_client_send_thread_cnt; client_thread_id++) {
        uint64_t port_id = get_port_id(g_node_id,node_id,client_thread_id % g_client_send_thread_cnt);
        Node* n;
        try{
          n = mapOfNodes.at(node_id);
        }catch(std::out_of_range){
          assert(false);
          n = new Node(node_id, TIMESTAMPER, true, "client" + node_id, port_id, port_id, port_id);
          mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));
        }
        n->communication->initSendSocket(client_thread_id, ifaddr, true);
      }
    } else {
      for(uint64_t server_thread_id = g_thread_cnt + g_rem_thread_cnt; server_thread_id < g_thread_cnt + g_rem_thread_cnt + g_send_thread_cnt; server_thread_id++) {
        uint64_t port_id = get_port_id(g_node_id,node_id,server_thread_id % g_send_thread_cnt);
        Node* n;
        try{
          n = mapOfNodes.at(node_id);
        }catch(std::out_of_range){
          assert(false);
          n = new Node(node_id, TIMESTAMPER, false, "server" + node_id, port_id, port_id, port_id);
          mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));

        }
        n->communication->initSendSocket(server_thread_id, ifaddr, false);
      }
    }
  }
}

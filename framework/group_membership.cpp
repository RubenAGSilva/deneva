#include "group_membership.h"
#include "utils/transaction.h"
#include "../system/global.h"
#include "node.h"
#include "transport.h"
#include <iostream>
#include <list>

using namespace std;

GroupMembership::GroupMembership(Node* node){
    mySelf = node;
    initNodes();
    string path = get_path();
	  read_ifconfig(path.c_str());
}

uint64_t GroupMembership::get_port_id(uint64_t src_node_id, uint64_t dest_node_id, uint64_t send_thread_id) {
    uint64_t port_id = 0;
    port_id += g_total_node_cnt * dest_node_id;
    port_id += src_node_id;
    //  uint64_t max_send_thread_cnt = g_send_thread_cnt > g_client_send_thread_cnt ? g_send_thread_cnt : g_client_send_thread_cnt;
    //  port_id *= max_send_thread_cnt;
    port_id += send_thread_id * g_total_node_cnt * g_total_node_cnt;
    return port_id;
}

list<Node*> GroupMembership::getReplicationTargets(uint64_t nodeId){ 
    printf("get replication targets \n");
    fflush(stdout);

    list<Node*> replicaNodes = list<Node*>();
    map<uint64_t,Node*>::iterator it;

    for(it = mapOfNodes.begin(); it!= mapOfNodes.end(); it++){ // instead of 2 it is g_repl_cnt
        replicaNodes.push_back(it->second); //change TODO
        break;
    }
    return replicaNodes;
}


int GroupMembership::getTimestamper(){ // Ver se todos podem ser timestamppers ou nao
    printf("get timestamper \n");
    fflush(stdout);

    map<uint64_t,Node*>::iterator it;

    for(it = mapOfNodes.begin(); it!= mapOfNodes.end(); it++){
        if(it->second->getRole()==TIMESTAMPER){
            return it->second->getId();
        }
    }

    return mySelf->getId();
}

int GroupMembership::getCoordinator(TransactionF* transaction){ // Ver se todos ou so um podem ser coordinators ou nao
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

void GroupMembership::initNodes(){ //TODO remove comments

  for(uint64_t node_id = 0; node_id < g_total_node_cnt; node_id++) {
    if(node_id == g_node_id)
      continue;
    // Listening ports
    if(ISCLIENTN(node_id)) {
      for(uint64_t client_thread_id = g_client_thread_cnt + g_client_rem_thread_cnt; client_thread_id < g_client_thread_cnt + g_client_rem_thread_cnt + g_client_send_thread_cnt; client_thread_id++) {
        uint64_t port_id = get_port_id(node_id,g_node_id,client_thread_id % g_client_send_thread_cnt);
        //Socket * sock = bind(port_id);

        Node* n = new Node(node_id, TIMESTAMPER, "client" + node_id, port_id, port_id, port_id); //TODO change because this is executed for each thread instead of node
        //n->setRecvSocket(sock);
        mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));
        
      }
    } else {
      for(uint64_t server_thread_id = g_thread_cnt + g_rem_thread_cnt; server_thread_id < g_thread_cnt + g_rem_thread_cnt + g_send_thread_cnt; server_thread_id++) {
        uint64_t port_id = get_port_id(node_id,g_node_id,server_thread_id % g_send_thread_cnt);
        //Socket * sock = bind(port_id);

        Node* n = new Node(node_id, TIMESTAMPER, "server" + node_id, port_id, port_id, port_id);
        //n->setRecvSocket(sock);
        mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));
      }
    }
    // Sending ports
    if(ISCLIENTN(g_node_id)) {
      for(uint64_t client_thread_id = g_client_thread_cnt + g_client_rem_thread_cnt; client_thread_id < g_client_thread_cnt + g_client_rem_thread_cnt + g_client_send_thread_cnt; client_thread_id++) {
        uint64_t port_id = get_port_id(g_node_id,node_id,client_thread_id % g_client_send_thread_cnt);
        std::pair<uint64_t,uint64_t> sender = std::make_pair(node_id,client_thread_id);
        //Socket * sock = connect(node_id,port_id);
        Node* n;
        try{
          n = mapOfNodes.at(node_id);
        }catch(std::out_of_range){
          n = new Node(node_id, TIMESTAMPER, "client" + node_id, port_id, port_id, port_id);
        }
        //n->setSendSocket(sock);
        mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));
      }
    } else {
      for(uint64_t server_thread_id = g_thread_cnt + g_rem_thread_cnt; server_thread_id < g_thread_cnt + g_rem_thread_cnt + g_send_thread_cnt; server_thread_id++) {
        uint64_t port_id = get_port_id(g_node_id,node_id,server_thread_id % g_send_thread_cnt);
        std::pair<uint64_t,uint64_t> sender = std::make_pair(node_id,server_thread_id);
        //Socket * sock = connect(node_id,port_id);
        Node* n;
        try{
          n = mapOfNodes.at(node_id);
        }catch(std::out_of_range){
          n = new Node(node_id, TIMESTAMPER, "server" + node_id, port_id, port_id, port_id);
        }
        //n->setSendSocket(sock);
        mapOfNodes.insert(std::pair<uint64_t,Node*>(node_id, n));
      }
    }
  }
}

Socket * GroupMembership::bind(uint64_t port_id) {
  Socket * socket = get_socket();
  char socket_name[MAX_TPORT_NAME];
#if TPORT_TYPE == IPC
  sprintf(socket_name,"ipc://node_%ld.ipc",port_id);
#else
#if ENVIRONMENT_EC2
  sprintf(socket_name,"tcp://eth0:%ld",port_id);
#else
  sprintf(socket_name,"tcp://%s:%ld",ifaddr[g_node_id],port_id);
#endif
#endif
  printf("Sock Binding to %s %d\n",socket_name,g_node_id);
  int rc = socket->sock.bind(socket_name);
  if(rc < 0) {
    printf("Bind Error: %d %s\n",errno,strerror(errno));
    assert(false);
  }
  return socket;
}

Socket * GroupMembership::connect(uint64_t dest_id,uint64_t port_id) {
  Socket * socket = get_socket();
  char socket_name[MAX_TPORT_NAME];
#if TPORT_TYPE == IPC
  sprintf(socket_name,"ipc://node_%ld.ipc",port_id);
#else
#if ENVIRONMENT_EC2
  sprintf(socket_name,"tcp://eth0;%s:%ld",ifaddr[dest_id],port_id);
#else
  sprintf(socket_name,"tcp://%s;%s:%ld",ifaddr[g_node_id],ifaddr[dest_id],port_id);
#endif
#endif
  printf("Sock Connecting to %s %d -> %ld\n",socket_name,g_node_id,dest_id);
  int rc = socket->sock.connect(socket_name);
  if(rc < 0) {
    printf("Connect Error: %d %s\n",errno,strerror(errno));
    assert(false);
  }
  return socket;
}

Socket * GroupMembership::get_socket() {
  //Socket * socket = new Socket;
  Socket * socket = (Socket*) mem_allocator.align_alloc(sizeof(Socket));
  new(socket) Socket();
	int timeo = 1000; // timeout in ms
	int stimeo = 1000; // timeout in ms
  int opt = 0;
  socket->sock.setsockopt(NN_SOL_SOCKET,NN_RCVTIMEO,&timeo,sizeof(timeo));
  socket->sock.setsockopt(NN_SOL_SOCKET,NN_SNDTIMEO,&stimeo,sizeof(stimeo));
  // NN_TCP_NODELAY doesn't cause TCP_NODELAY to be set -- nanomsg issue #118
  socket->sock.setsockopt(NN_SOL_SOCKET,NN_TCP_NODELAY,&opt,sizeof(opt));
  return socket;
}

void GroupMembership::read_ifconfig(const char * ifaddr_file) {

	ifaddr = new char *[g_total_node_cnt];

	uint64_t cnt = 0;
  printf("Reading ifconfig file: %s\n",ifaddr_file);
	ifstream fin(ifaddr_file);
	string line;
  while (getline(fin, line)) {
		//memcpy(ifaddr[cnt],&line[0],12);
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
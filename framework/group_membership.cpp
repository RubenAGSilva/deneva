#include "group_membership.h"
#include "utils/transaction.h"
#include "../system/global.h"
#include "node.h"
#include <iostream>
#include <list>

using namespace std;

GroupMembership::GroupMembership(Node* node){
    mySelf = node;
}

list<Node> GroupMembership::getNodes(){
    printf("get nodes \n");
    fflush(stdout);

    list<Node> rtn_nodes;
    //create new list of Nodes, add them and return them.
  for(uint64_t node_id = 0; node_id < g_total_node_cnt; node_id++) {
    if(node_id == g_node_id)
      continue;
    // Listening ports
    if(ISCLIENTN(node_id)) {
      for(uint64_t client_thread_id = g_client_thread_cnt + g_client_rem_thread_cnt; client_thread_id < g_client_thread_cnt + g_client_rem_thread_cnt + g_client_send_thread_cnt; client_thread_id++) {
        uint64_t port_id = get_port_id(node_id,g_node_id,client_thread_id % g_client_send_thread_cnt);
        Node n = Node(client_thread_id, TIMESTAMPER, "client" + client_thread_id, port_id, port_id, port_id);
        rtn_nodes.push_back(n);
      }
    } else {
      for(uint64_t server_thread_id = g_thread_cnt + g_rem_thread_cnt; server_thread_id < g_thread_cnt + g_rem_thread_cnt + g_send_thread_cnt; server_thread_id++) {
        uint64_t port_id = get_port_id(node_id,g_node_id,server_thread_id % g_send_thread_cnt);
        Node n = Node(server_thread_id, TIMESTAMPER, "server" + server_thread_id, port_id, port_id, port_id);
        rtn_nodes.push_back(n);
      }
    }
    // Sending ports
    // if(ISCLIENTN(g_node_id)) {
    //   for(uint64_t client_thread_id = g_client_thread_cnt + g_client_rem_thread_cnt; client_thread_id < g_client_thread_cnt + g_client_rem_thread_cnt + g_client_send_thread_cnt; client_thread_id++) {
    //     uint64_t port_id = get_port_id(g_node_id,node_id,client_thread_id % g_client_send_thread_cnt);
    //   }
    // } else {
    //   for(uint64_t server_thread_id = g_thread_cnt + g_rem_thread_cnt; server_thread_id < g_thread_cnt + g_rem_thread_cnt + g_send_thread_cnt; server_thread_id++) {
    //     uint64_t port_id = get_port_id(g_node_id,node_id,server_thread_id % g_send_thread_cnt);
    //   }
    // }
  }
  return rtn_nodes;
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

list<Node>* GroupMembership::getReplicationTargets(){ 
    printf("get replication targets \n");
    fflush(stdout);

    list<Node>* replicaNodes = new list<Node>();
    list<Node> nodes = getNodes();
    for(uint32_t i = 0; i!=2; i++){ // instead of 2 it is g_repl_cnt
        replicaNodes->push_back(nodes.back()); //change
    }
    return replicaNodes;
}


int GroupMembership::getTimestamper(){ // Ver se todos podem ser timestamppers ou nao
    printf("get timestamper \n");
    fflush(stdout);

    list<Node> nodes = getNodes();
    for(Node node : nodes){
        if(node.getRole()==TIMESTAMPER){
            return node.getId();
        }
    }

    return mySelf->getId();
}

int GroupMembership::getCoordinator(TransactionF* transaction){ // Ver se todos ou so um podem ser coordinators ou nao
    printf("get coordinator \n");
    fflush(stdout);

    list<Node> nodes = getNodes();
    for(Node node : nodes){
        if(node.getRole()==COORDINATOR){
            return node.getId();
        }
    }

    return -1;
}
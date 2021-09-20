#include "communication.h"
#include <thread.h>
#include "configuration.cpp"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "framework.h"

Communication::Communication(uint64_t id){
  nodeId = id;
}

void Communication::initSendSocket(uint64_t thread_id, char** ifaddr1, bool isClient){
    ifaddr = ifaddr1;
    uint64_t port_id;
    //printf("Im running: %u, but this node is: %lu\n", g_node_id, nodeId);
    //fflush(stdout);
    if(isClient){
      port_id = get_port_id(g_node_id, nodeId,thread_id % g_client_send_thread_cnt);
    }else{
      port_id = get_port_id(g_node_id, nodeId,thread_id % g_send_thread_cnt);
    }
    sendSocket = connect(nodeId, port_id);
}

void Communication::initRecvSocket(uint64_t thread_id, char** ifaddr1, bool isClient){
    ifaddr = ifaddr1;
    uint64_t port_id;
    if(isClient){
      port_id = get_port_id(nodeId, g_node_id,thread_id % g_client_send_thread_cnt);
    }else{
      port_id = get_port_id(nodeId, g_node_id,thread_id % g_send_thread_cnt);
    }
    recvSocket = bind(port_id);
    receiveThread = std::thread(&Communication::recvTransaction, this);
}

Socket * Communication::bind(uint64_t port_id) {
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

Socket * Communication::connect(uint64_t dest_id,uint64_t port_id) {
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

Socket * Communication::get_socket() {
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

uint64_t Communication::get_port_id(uint64_t src_node_id, uint64_t dest_node_id, uint64_t send_thread_id) {
    uint64_t port_id = 10;
  port_id += g_total_node_cnt * dest_node_id;
  port_id += src_node_id;
  port_id += send_thread_id * g_total_node_cnt * g_total_node_cnt;
  port_id += TPORT_PORT;
  printf("--Port ID:  %ld, %ld -> %ld : %ld\n",send_thread_id,src_node_id,dest_node_id,port_id);
  return port_id;
}

void Communication::get(MessageF message){

    MessageType type = message.getMessageType();
    TransactionF transaction = message.getTransaction();
    TransactionF* transactionptr = new TransactionF(transaction);
    //printf("-Received transaction Id: %lu\n", transactionptr->getId());
    //fflush(stdout);
    switch(type){
        case REPLICATE_WRITES:
          groupMembership->getNodes().at(message.getSenderNodeId())->addContents(transaction.getWriteSet()); //replicate writes
          break;
        case REPLICATE:
          framework->addTransaction(transactionptr);
        break;
        default:
        break;
    }
}

void Communication::sendMessage(MessageF message, InterfaceCommunication* communication){
  Socket* socket = communication->getSendSocket();

  std::ostringstream ofs;
    {
    boost::archive::text_oarchive oa(ofs);
    oa << message; 

    const char* data = ofs.str().c_str(); //tem de ser char* apenas?
    size_t dataSize = std::string(data).length();


    void * buf = nn_allocmsg(dataSize,0);
    memcpy(buf,data,dataSize);
    

  size_t offset = 0;
  int result;
  while(offset < dataSize){

    if(dataSize - offset > MAX_SIZE){
      void * buf = nn_allocmsg(MAX_SIZE,0);
      memcpy(buf,data,MAX_SIZE);
      std::string str(data);
      str.erase(0, MAX_SIZE);
      data = str.c_str();

    }else{
      void * buf = nn_allocmsg(dataSize,0);
      memcpy(buf,data,dataSize);

    }
    result = socket->sock.send(&buf,NN_MSG,NN_DONTWAIT);
    if(result<0){printf("ERROR %i\n", result); fflush(stdout);}
    offset  += result;
    //printf("-Sending in total: %lu | sent now datasize of: %lu\n", dataSize, offset);
  }

  }
}

void Communication::recvTransaction(){

  //printf("Thread starting, receiving transactions \n");
  //fflush(stdout);

  while(true){ //thread running this
    MessageF message;

    {
       try{
      char * recv;
      bool complete = false;
      while(!complete){
      
        int result = recvSocket->sock.recv(&recv,NN_MSG,NN_DONTWAIT);
        if(recv == NULL || result == -1){
          sleep(0.5); //half second
          continue;
        }else if(result <= int(MAX_SIZE)){
          complete = true;
        }
      
        //printf("---Recevied, bytes: %i\n", result);
        //fflush(stdout);
      
        std::istringstream ifs((char*) recv);
        boost::archive::text_iarchive ia(ifs);
        ia >> message;
      }
      
        get(message);
        
        //printf("Received Message\n");
        //printf("transaction readset size: %lu | transactionNodeId: %lu | transactionTSStartup: %lu\n",message.getTransaction().getReadSet().size(), message.getTransaction().getNodeId(), message.getTransaction().getTimestampStartup());
        //fflush(stdout);  
       }catch(...){
         //printf("-___-Problem receiving message\n");
         //fflush(stdout);
       }
    }
  }
    
}

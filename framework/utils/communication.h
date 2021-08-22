#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "global.h"
#include "transaction.h"
#include "transport.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include "framework.h"

class interfaceFramework;

enum MessageType{
    REPLICATE, REPLICATE_WRITES
};

class MessageF{
        
    public:
        uint64_t senderNodeId;
        uint64_t receiverNodeId;
        MessageType type;
        TransactionF transaction;

        friend std::ostream & operator<<(std::ostream &os, const MessageF &msg){
            return os << msg.senderNodeId << ' ' << msg.receiverNodeId << ' ' << msg.type << ' ' << msg.transaction << '\n';
        }

        MessageF(){}

        MessageF(uint64_t senderNodeId1, uint64_t receiverNodeId1, MessageType type1){
            senderNodeId = senderNodeId1;
            receiverNodeId= receiverNodeId1;
            type = type1;
        }

        MessageF(uint64_t senderNodeId1, uint64_t receiverNodeId1, TransactionF transaction1, MessageType type1){
            senderNodeId = senderNodeId1;
            receiverNodeId= receiverNodeId1;
            type = type1;
            transaction = transaction1;
        }
        TransactionF getTransaction(){
            return transaction;
        }

        void setTransaction(TransactionF transaction1){
            transaction = transaction1;
        }

        MessageType getMessageType(){
            return type;
        }

        uint64_t getSenderNodeId(){return senderNodeId;}
        uint64_t getReceiverNodeId(){return receiverNodeId;}


};

namespace boost{
    namespace serialization{

        template<class Archive>
        void serialize(Archive & ar, MessageF &msg, const unsigned int version){
            ar &msg.senderNodeId;
            ar &msg.receiverNodeId;
            ar &msg.type;
            ar &msg.transaction;
        }

    }
} //namespace boost

class InterfaceCommunication{

    public:
    virtual void get(MessageF message) = 0;
    virtual void sendMessage(MessageF message, InterfaceCommunication* communication) = 0;
    

    //For Sockets//
    virtual Socket* getRecvSocket() = 0;
    virtual Socket* getSendSocket() = 0;
    virtual void initSendSocket(uint64_t thread_id, char** ifaddr, bool isClient) = 0;
    virtual void initRecvSocket(uint64_t thread_id, char** ifaddr, bool isClient) = 0;

};

class Communication : public InterfaceCommunication{

    private:
    const size_t MAX_SIZE = 223;
    Socket* recvSocket;
    Socket* sendSocket;
    uint64_t nodeId;
    char** ifaddr;
    std::thread receiveThread;

    ~Communication(){
        if(receiveThread.joinable()){
            sleep(4);
            receiveThread.join(); // problem out of scope
        }
    }    

    uint64_t get_port_id(uint64_t src_node_id, uint64_t dest_node_id, uint64_t send_thread_id);
    Socket * bind(uint64_t port_id);
    Socket * connect(uint64_t dest_id,uint64_t port_id);
    Socket * get_socket();
    void read_ifconfig(const char * ifaddr_file);
    string get_path(); 


    public:
    Communication(uint64_t id);

    void get(MessageF message) override;
    
    void sendMessage(MessageF message, InterfaceCommunication* communication) override;
    void recvTransaction();

    void initSendSocket(uint64_t thread_id, char** ifaddr, bool isClient) override;
    void initRecvSocket(uint64_t thread_id, char** ifaddr, bool isClient) override;

    Socket* getRecvSocket()override{return recvSocket;}
    Socket* getSendSocket()override{return sendSocket;}

};
#endif
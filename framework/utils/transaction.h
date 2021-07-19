#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "metadata.cpp"
#include "content.h"
#include "semaphore.h"
#include <list>
#include <string>

    class TransactionF{

        private:
            uint64_t id;
            int coordinator; // might not be an int
            std::list<Content*> writeset;
            std::list<Content*> readset;
            uint64_t timestampStartup;
            uint64_t timestampCommit;

            //uint64_t row_cnt;
            uint64_t nodeId;

            std::list<uint64_t> locksDetained;
            Metadata metadata;

            bool replicated=false;
            bool validated=false;
            bool finished=false;
            

        public:

            int MODEL_ID=1;
            int volatile lockReady;
            uint32_t lock_ready_cnt;
            sem_t rsp_mutex;

            TransactionF(uint64_t id = -1, uint64_t newNodeId = -1);
            void addToReadSet(Content* content);
            void addToWriteSet(Content* content);
            void addLockDetained(uint64_t lock);
            uint64_t incr_lr();
            uint64_t decr_lr();

            void setReplicated(bool replicated1){
                replicated = replicated1;
            }
            void setValidated(bool validated1){
                validated = validated1;
            }
            void setTimestampCommit(uint64_t ts){timestampCommit = ts;}
            void setTimestampStart(uint64_t ts){timestampStartup = ts;}
            //int getAcessCount(){return writeset.size()+readset.size();}

            uint64_t getId(){
                return id;
            }
            Metadata getMetadata(){
                return metadata;
            }
            std::list<Content*> getWriteSet(){
                return writeset;
            }
            std::list<Content*> getReadSet(){
                return readset;
            }
            uint64_t getTimestampStartup(){
                return timestampStartup;
            }
            uint64_t getTimestampCommit(){
                return timestampCommit;
            }
            std::list<uint64_t> getLocksDetained(){
                return locksDetained;
            }

            uint64_t getNodeId(){return nodeId;}
            void setNodeId(uint64_t newNodeId){nodeId = newNodeId;}

            bool isReplicated(){
                return replicated;
            }
            bool isValidated(){
                return validated;
            }
            bool isFinished(){
                return finished;
            }
            void setFinished(bool finish){
                finished = finish;
            }
            int getCoordinator(){
                return coordinator;
            }
            void setCoordinator(int id){
                coordinator = id;
            }
            void clearLocksDetained(){
                locksDetained.clear();
            }
            void clearSets();

};
#endif
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "content.h"
#include "semaphore.h"
#include <boost/serialization/list.hpp>
#include <string>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

    class TransactionF{

        private:
            uint64_t id;
            std::list<Content*> writeset;
            std::list<Content*> readset;
            uint64_t timestampStartup;
            uint64_t timestampCommit;

            uint64_t nodeId; //uint32_t ?

            std::list<uint64_t> locksDetained;

            bool replicated=false;
            bool validated=false;
            bool finished=false;
            
            friend class boost::serialization::access;

            friend std::ostream & operator<<(std::ostream &os, const TransactionF &txn){
                os << txn.id << ' ' << txn.timestampStartup << ' ' << txn.timestampCommit << ' ' << txn.nodeId << ' ';

                std::list<uint64_t>::const_iterator it;
                for(it = txn.locksDetained.begin(); it!= txn.locksDetained.end(); it++){
                    os << *it << '-';
                }        

                os << ' ' << txn.replicated << ' ' << txn.validated << txn.finished << ' ';

                std::list<Content*>::const_iterator jt;
                for(jt = txn.readset.begin(); jt!= txn.readset.end(); jt++){
                    os << **jt << '-';
                } 

                for(jt = txn.writeset.begin(); jt!= txn.writeset.end(); jt++){
                    os << **jt << '-';
                } 

                return os << '\n';
            } 

            template<class Archive>
            void serialize(Archive &ar, const unsigned int version){
                ar &id;
                ar &timestampStartup;
                ar &timestampCommit;
                ar &nodeId;
                ar &locksDetained;
                ar &replicated;
                ar &validated;
                ar &finished;
                ar &readset;
                ar &writeset;  
            }


        public:
            
            //stats
            uint64_t statStartTs = 0;
            uint64_t restart_starttime = 0;


            //int MODEL_ID=1;
            int volatile lockReady;
            uint32_t lock_ready_cnt;
            sem_t rsp_mutex;

            TransactionF(uint64_t id = -1, uint64_t newNodeId = -1);
            void addToReadSet(Content* content);
            void addToWriteSet(Content* content);
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

            uint64_t getId(){
                return id;
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
            void clearLocksDetained(){
                locksDetained.clear();
            }
            void clearSets();

};



#endif
#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "metadata.cpp"
#include "content.cpp"
#include <list>
#include <string>

    class TransactionF{

        private:
            uint64_t id;
            int transactionManager; // might not be an int
            std::list<Content> writeset;
            std::list<Content> readset;
            int timestampStartup;
            int timestampCommit;

            std::list<int> locksDetained;
            Metadata metadata;

            bool replicated=false;
            bool validated=false;
            bool finished=false;

        public:

            int MODEL_ID=1;

            TransactionF(uint64_t id = -1);
            void addToReadSet(int64_t key, int value);
            void addToWriteSet(int64_t key, int value);
            void addLockDetained(int lock);

            void setReplicated(bool replicated1){
                replicated = replicated1;
            }
            void setValidated(bool validated1){
                validated = validated1;
            }

            uint64_t getId(){
                return id;
            }
            Metadata getMetadata(){
                return metadata;
            }
            std::list<Content> getWriteSet(){
                return writeset;
            }
            std::list<Content> getReadSet(){
                return readset;
            }
            int getTimestampStartup(){
                return timestampStartup;
            }
            int getTimestampCommit(){
                return timestampCommit;
            }
            std::list<int> getLocksDetained(){
                return locksDetained;
            }
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
            int getTransactionManager(){
                return transactionManager;
            }
            void setTransactionManager(int id){
                transactionManager = id;
            }
};
#endif
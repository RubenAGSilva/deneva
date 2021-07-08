#ifndef CONTENT_H
#define CONTENT_H

#include <stdint.h>
#include "row.h"

    class Content{
        private:
            uint64_t key;
            //int* value;
            row_t* value;

        public:
            Content(uint64_t key1, row_t* value1){
                key=key1;
                value=value1;
            }
            Content() = default;
            
            uint64_t getKey(){
                return key;
            }
            row_t* getValue(){
                return value;
            }
            void setValue(row_t* value){
                value=value;
            }
            // int* getValue(){
            //     return value;
            // }
            // void setValue(int value){
            //     value=value;
            // }
    };
#endif
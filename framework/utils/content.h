#ifndef CONTENT_H
#define CONTENT_H

#include <stdint.h>
#include "row.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

    class Content{
        private:
            uint64_t key;
            row_t* value;

            friend class boost::serialization::access;
            friend std::ostream & operator<<(std::ostream &os, const Content &content){
                return os << content.key << ' ' << content.value << '\n';
            } 
            template<class Archive>
            void serialize(Archive &ar, const unsigned int version){
                ar &key;
                ar &value;
            }

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
    };


#endif
#ifndef CONTENT_H
#define CONTENT_H

    class Content{
        
        private:
            int key;
            int value;
        public:
            Content(int key, int value){
                key=key;
                value=value;
            }
            Content() = default;
            
            int getKey(){
                return key;
            }
            int getValue(){
                return value;
            }
            void setValue(int value){
                value=value;
            }
    };
#endif
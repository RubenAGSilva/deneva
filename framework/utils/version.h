#ifndef VERSION_H
#define VERSION_H

#include "content.h"

enum Occurred{
    BEFORE, AFTER, TIE, CONCURRENTLY
};

class InterfaceVersion{

    public:
        virtual ~InterfaceVersion(){}
        virtual Occurred compareClocks(InterfaceVersion* v2) = 0;
        virtual Occurred compareTimestamps(long ts1, long ts2) = 0;
        virtual void incrementVersion(long time) = 0;
        virtual void updateClock() = 0;
        virtual long getTime() = 0;
        virtual void lockContent(Content * content)=0;
        virtual void releaseContent(Content * content)=0;
        
};

class ClockF : public InterfaceVersion{

    private:
        long time;
        pthread_mutex_t ts_mutex;
	    pthread_mutex_t mutexes[BUCKET_CNT];

        uint64_t hash(Content * content);

    public:
        ClockF();
        ~ClockF(){}
        Occurred compareClocks(InterfaceVersion* v2) override;
        Occurred compareTimestamps(long ts1, long ts2) override;
        void incrementVersion(long time) override;
        long getTime() override;
        void updateClock() override;
        void lockContent(Content * content);
        void releaseContent(Content * content);
};

#endif
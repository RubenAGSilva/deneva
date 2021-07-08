#ifndef VERSION_H
#define VERSION_H


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
        
};

class ClockF : public InterfaceVersion{

    private:
        long time;
        
    public:
        ClockF();
        ~ClockF(){}
        Occurred compareClocks(InterfaceVersion* v2) override;
        Occurred compareTimestamps(long ts1, long ts2) override;
        void incrementVersion(long time) override;
        long getTime() override;
        void updateClock() override;
};

#endif
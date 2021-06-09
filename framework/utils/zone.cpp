#include <list>

using namespace std;

class Zone{
    private:
        int zoneId;
        list<int> proximityList;
    public:
        Zone(int zoneId){
            zoneId=zoneId;
        }
};
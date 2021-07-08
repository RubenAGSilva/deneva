#include "version.h"
#include "../../system/global.h"
#include <chrono>

using namespace std::chrono;

ClockF::ClockF(){
    time = -1;
}

Occurred ClockF::compareClocks(InterfaceVersion* v2){
    return compareTimestamps(time, v2->getTime());
    
}

Occurred ClockF::compareTimestamps(long ts1, long ts2){
    if(ts1 > ts2){
        return Occurred::AFTER;
    }else if(ts1 < ts2){
        return Occurred::BEFORE;
    }else{
        return Occurred::TIE;
    }
}

void ClockF::incrementVersion(long time){
    return;
}

long ClockF::getTime(){
    if(time == -1){
        updateClock();
    }
    return time;
}


void ClockF::updateClock(){

	switch(g_ts_alloc) {
	case TS_MUTEX :
		//pthread_mutex_lock( &time );
		time++;
		//pthread_mutex_unlock( &time );
		break;
	case TS_CAS :
		if (g_ts_batch_alloc)
			time = ATOM_FETCH_ADD(time, g_ts_batch_num);
		else 
			time = ATOM_FETCH_ADD(time, 1);
		break;
	case TS_HW :
		assert(false);
		break;
	case TS_CLOCK :
		time = get_wall_clock() * (g_node_cnt + g_thread_cnt) + (g_node_id * g_thread_cnt);
		break;
	default :
		assert(false);
	}

    timespec * tp = new timespec;
    clock_gettime(CLOCK_REALTIME, tp);
    time = tp->tv_sec * 1000000000 + tp->tv_nsec; 
    delete tp;
}
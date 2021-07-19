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
		pthread_mutex_lock( &ts_mutex );
		time++;
		pthread_mutex_unlock( &ts_mutex );
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

void ClockF::lockContent(Content * row) {
	int bid = hash(row);
	pthread_mutex_lock( &mutexes[bid] );	
}

void ClockF::releaseContent(Content * row) {
	int bid = hash(row);
	pthread_mutex_unlock( &mutexes[bid] );
}

uint64_t ClockF::hash(Content * row) {
	uint64_t addr = (uint64_t)row / MEM_ALLIGN;
    return (addr * 1103515247 + 12345) % BUCKET_CNT;
}
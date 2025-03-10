/*
   Copyright 2016 Massachusetts Institute of Technology

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include "helper.h"
#include "txn.h"
#include "row.h"
#include "wl.h"
#include "query.h"
#include "thread.h"
#include "mem_alloc.h"
#include "occ.h"
#include "row_occ.h"
#include "table.h"
#include "catalog.h"
#include "index_btree.h"
#include "index_hash.h"
#include "msg_queue.h"
#include "pool.h"
#include "message.h"
#include "ycsb_query.h"
#include "tpcc_query.h"
#include "pps_query.h"
#include "array.h"
#include "maat.h"


void TxnStats::init() {
  starttime=0;
  wait_starttime=get_sys_clock();
  total_process_time=0;
  process_time=0;
  total_local_wait_time=0;
  local_wait_time=0;
  total_remote_wait_time=0;
  remote_wait_time=0;
  total_twopc_time=0;
  twopc_time=0;
  write_cnt = 0;
  abort_cnt = 0;

   total_work_queue_time = 0;
   work_queue_time = 0;
   total_cc_block_time = 0;
   cc_block_time = 0;
   total_cc_time = 0;
   cc_time = 0;
   total_work_queue_cnt = 0;
   work_queue_cnt = 0;
   total_msg_queue_time = 0;
   msg_queue_time = 0;
   total_abort_time = 0;

   clear_short();
}

void TxnStats::clear_short() {

   work_queue_time_short = 0;
   cc_block_time_short = 0;
   cc_time_short = 0;
   msg_queue_time_short = 0;
   process_time_short = 0;
   network_time_short = 0;
}

void TxnStats::reset() {
  wait_starttime=get_sys_clock();
  total_process_time += process_time;
  process_time = 0;
  total_local_wait_time += local_wait_time;
  local_wait_time = 0;
  total_remote_wait_time += remote_wait_time;
  remote_wait_time = 0;
  total_twopc_time += twopc_time;
  twopc_time = 0;
  write_cnt = 0;

  total_work_queue_time += work_queue_time;
  work_queue_time = 0;
  total_cc_block_time += cc_block_time;
  cc_block_time = 0;
  total_cc_time += cc_time;
  cc_time = 0;
  total_work_queue_cnt += work_queue_cnt;
  work_queue_cnt = 0;
  total_msg_queue_time += msg_queue_time;
  msg_queue_time = 0;

  clear_short();

}

void TxnStats::abort_stats(uint64_t thd_id) {
    total_process_time += process_time;
    total_local_wait_time += local_wait_time;
    total_remote_wait_time += remote_wait_time;
    total_twopc_time += twopc_time;
    total_work_queue_time += work_queue_time;
    total_msg_queue_time += msg_queue_time;
    total_cc_block_time += cc_block_time;
    total_cc_time += cc_time;
    total_work_queue_cnt += work_queue_cnt;
    assert(total_process_time >= process_time);

    INC_STATS(thd_id,lat_s_rem_work_queue_time,total_work_queue_time);
    INC_STATS(thd_id,lat_s_rem_msg_queue_time,total_msg_queue_time);
    INC_STATS(thd_id,lat_s_rem_cc_block_time,total_cc_block_time);
    INC_STATS(thd_id,lat_s_rem_cc_time,total_cc_time);
    INC_STATS(thd_id,lat_s_rem_process_time,total_process_time);
}

void TxnStats::commit_stats(uint64_t thd_id, uint64_t txn_id, uint64_t batch_id, uint64_t timespan_long, uint64_t timespan_short) {
  total_process_time += process_time;
  total_local_wait_time += local_wait_time;
  total_remote_wait_time += remote_wait_time;
  total_twopc_time += twopc_time;
  total_work_queue_time += work_queue_time;
  total_msg_queue_time += msg_queue_time;
  total_cc_block_time += cc_block_time;
  total_cc_time += cc_time;
  total_work_queue_cnt += work_queue_cnt;
  assert(total_process_time >= process_time);

#if CC_ALG == CALVIN

    INC_STATS(thd_id,lat_s_loc_work_queue_time,work_queue_time);
    INC_STATS(thd_id,lat_s_loc_msg_queue_time,msg_queue_time);
    INC_STATS(thd_id,lat_s_loc_cc_block_time,cc_block_time);
    INC_STATS(thd_id,lat_s_loc_cc_time,cc_time);
    INC_STATS(thd_id,lat_s_loc_process_time,process_time);
  // latency from start of transaction at this node
  PRINT_LATENCY("lat_l %ld %ld %ld %f %f %f %f %f %f\n"
          , txn_id
          , batch_id
          , total_work_queue_cnt
          , (double) timespan_long / BILLION
          , (double) total_work_queue_time / BILLION
          , (double) total_msg_queue_time / BILLION
          , (double) total_cc_block_time / BILLION
          , (double) total_cc_time / BILLION
          , (double) total_process_time / BILLION
          );
#else
  // latency from start of transaction
  if (IS_LOCAL(txn_id)) {
    INC_STATS(thd_id,lat_l_loc_work_queue_time,total_work_queue_time);
    INC_STATS(thd_id,lat_l_loc_msg_queue_time,total_msg_queue_time);
    INC_STATS(thd_id,lat_l_loc_cc_block_time,total_cc_block_time);
    INC_STATS(thd_id,lat_l_loc_cc_time,total_cc_time);
    INC_STATS(thd_id,lat_l_loc_process_time,total_process_time);
    INC_STATS(thd_id,lat_l_loc_abort_time,total_abort_time);

    INC_STATS(thd_id,lat_s_loc_work_queue_time,work_queue_time);
    INC_STATS(thd_id,lat_s_loc_msg_queue_time,msg_queue_time);
    INC_STATS(thd_id,lat_s_loc_cc_block_time,cc_block_time);
    INC_STATS(thd_id,lat_s_loc_cc_time,cc_time);
    INC_STATS(thd_id,lat_s_loc_process_time,process_time);

    INC_STATS(thd_id,lat_short_work_queue_time,work_queue_time_short);
    INC_STATS(thd_id,lat_short_msg_queue_time,msg_queue_time_short);
    INC_STATS(thd_id,lat_short_cc_block_time,cc_block_time_short);
    INC_STATS(thd_id,lat_short_cc_time,cc_time_short);
    INC_STATS(thd_id,lat_short_process_time,process_time_short);
    INC_STATS(thd_id,lat_short_network_time,network_time_short);


  }
  else {
    INC_STATS(thd_id,lat_l_rem_work_queue_time,total_work_queue_time);
    INC_STATS(thd_id,lat_l_rem_msg_queue_time,total_msg_queue_time);
    INC_STATS(thd_id,lat_l_rem_cc_block_time,total_cc_block_time);
    INC_STATS(thd_id,lat_l_rem_cc_time,total_cc_time);
    INC_STATS(thd_id,lat_l_rem_process_time,total_process_time);
  }
  if (IS_LOCAL(txn_id)) {
    PRINT_LATENCY("lat_s %ld %ld %f %f %f %f %f %f\n"
          , txn_id
          , work_queue_cnt
          , (double) timespan_short / BILLION
          , (double) work_queue_time / BILLION
          , (double) msg_queue_time / BILLION
          , (double) cc_block_time / BILLION
          , (double) cc_time / BILLION
          , (double) process_time / BILLION
          );
   /*
  PRINT_LATENCY("lat_l %ld %ld %ld %f %f %f %f %f %f %f\n"
          , txn_id
          , total_work_queue_cnt
          , abort_cnt
          , (double) timespan_long / BILLION
          , (double) total_work_queue_time / BILLION
          , (double) total_msg_queue_time / BILLION
          , (double) total_cc_block_time / BILLION
          , (double) total_cc_time / BILLION
          , (double) total_process_time / BILLION
          , (double) total_abort_time / BILLION
          );
          */
  }
  else {
    PRINT_LATENCY("lat_rs %ld %ld %f %f %f %f %f %f\n"
          , txn_id
          , work_queue_cnt
          , (double) timespan_short / BILLION
          , (double) total_work_queue_time / BILLION
          , (double) total_msg_queue_time / BILLION
          , (double) total_cc_block_time / BILLION
          , (double) total_cc_time / BILLION
          , (double) total_process_time / BILLION
          );
  }
  /*
  if (!IS_LOCAL(txn_id) || timespan_short < timespan_long) {
    // latency from most recent start or restart of transaction
    PRINT_LATENCY("lat_s %ld %ld %f %f %f %f %f %f\n"
          , txn_id
          , work_queue_cnt
          , (double) timespan_short / BILLION
          , (double) work_queue_time / BILLION
          , (double) msg_queue_time / BILLION
          , (double) cc_block_time / BILLION
          , (double) cc_time / BILLION
          , (double) process_time / BILLION
          );
  }
  */
#endif

  if (!IS_LOCAL(txn_id)) {
      return;
  }

  INC_STATS(thd_id,txn_total_process_time,total_process_time);
  INC_STATS(thd_id,txn_process_time,process_time);
  INC_STATS(thd_id,txn_total_local_wait_time,total_local_wait_time);
  INC_STATS(thd_id,txn_local_wait_time,local_wait_time);
  INC_STATS(thd_id,txn_total_remote_wait_time,total_remote_wait_time);
  INC_STATS(thd_id,txn_remote_wait_time,remote_wait_time);
  INC_STATS(thd_id,txn_total_twopc_time,total_twopc_time);
  INC_STATS(thd_id,txn_twopc_time,twopc_time);
  if(write_cnt > 0) {
    INC_STATS(thd_id,txn_write_cnt,1);
  }
  if(abort_cnt > 0) {
    INC_STATS(thd_id,unique_txn_abort_cnt,1);
  }

}


void Transaction::init() {
  timestamp = UINT64_MAX;
  start_timestamp = UINT64_MAX;
  end_timestamp = UINT64_MAX;
  txn_id = UINT64_MAX;
  batch_id = UINT64_MAX;
  DEBUG_M("Transaction::init array insert_rows\n");
  insert_rows.init(g_max_items_per_txn + 10); 
  DEBUG_M("Transaction::reset array accesses\n");
  accesses.init(MAX_ROW_PER_TXN);  

  reset(0);
}

void Transaction::reset(uint64_t thd_id) {
  release_accesses(thd_id);
  accesses.clear();
  //release_inserts(thd_id);
  insert_rows.clear();  
  write_cnt = 0;
  row_cnt = 0;
  twopc_state = START;
  rc = RCOK;
}

void Transaction::release_accesses(uint64_t thd_id) {
  for(uint64_t i = 0; i < accesses.size(); i++) {
    access_pool.put(thd_id,accesses[i]);
  }
}

void Transaction::release_inserts(uint64_t thd_id) {
  for(uint64_t i = 0; i < insert_rows.size(); i++) {
    row_t * row = insert_rows[i];
#if CC_ALG != MAAT && CC_ALG != OCC
    DEBUG_M("TxnManager::cleanup row->manager free\n");
    mem_allocator.free(row->manager, 0);
#endif
    row->free_row();
    DEBUG_M("Transaction::release insert_rows free\n")
    row_pool.put(thd_id,row);
  }
}

void Transaction::release(uint64_t thd_id) {
  DEBUG("Transaction release\n");
  release_accesses(thd_id);
  DEBUG_M("Transaction::release array accesses free\n")
  accesses.release();
  release_inserts(thd_id);
  DEBUG_M("Transaction::release array insert_rows free\n")
  insert_rows.release();
}

void TxnManager::init(uint64_t thd_id, Workload * h_wl) {
  uint64_t prof_starttime = get_sys_clock();
  if(!txn)  {
    DEBUG_M("Transaction alloc\n");
    txn_pool.get(thd_id,txn);

  }
  INC_STATS(get_thd_id(),mtx[15],get_sys_clock()-prof_starttime);
  prof_starttime = get_sys_clock();
  //txn->init();
  if(!query) {
    DEBUG_M("TxnManager::init Query alloc\n");
    qry_pool.get(thd_id,query);
  }
  INC_STATS(get_thd_id(),mtx[16],get_sys_clock()-prof_starttime);
  //query->init();
  //reset();
  sem_init(&rsp_mutex, 0, 1);
  return_id = UINT64_MAX;

	this->h_wl = h_wl;
#if CC_ALG == MAAT
  uncommitted_writes = new std::set<uint64_t>();
  uncommitted_writes_y = new std::set<uint64_t>();
  uncommitted_reads = new std::set<uint64_t>();
#endif
#if CC_ALG == CALVIN
  phase = CALVIN_RW_ANALYSIS;
  locking_done = false;
  calvin_locked_rows.init(MAX_ROW_PER_TXN);
#endif
  
  txn_ready = true;
  twopl_wait_start = 0;

  txn_stats.init();
}

// reset after abort
void TxnManager::reset() {
	lock_ready = false;
  lock_ready_cnt = 0;
  locking_done = true;
	ready_part = 0;
  rsp_cnt = 0;
  aborted = false;
  return_id = UINT64_MAX;
  twopl_wait_start = 0;

  //ready = true;

  // MaaT
  greatest_write_timestamp = 0;
  greatest_read_timestamp = 0;
  commit_timestamp = 0;
#if CC_ALG == MAAT
  uncommitted_writes->clear();
  uncommitted_writes_y->clear();
  uncommitted_reads->clear();
#endif

#if CC_ALG == CALVIN
  phase = CALVIN_RW_ANALYSIS;
  locking_done = false;
  calvin_locked_rows.clear();
#endif

  assert(txn);
  assert(query);
  txn->reset(get_thd_id());

  // Stats
  txn_stats.reset();

}

void
TxnManager::release() {
  uint64_t prof_starttime = get_sys_clock();
  qry_pool.put(get_thd_id(),query);
  INC_STATS(get_thd_id(),mtx[0],get_sys_clock()-prof_starttime);
  query = NULL;
  prof_starttime = get_sys_clock();
  txn_pool.put(get_thd_id(),txn);
  INC_STATS(get_thd_id(),mtx[1],get_sys_clock()-prof_starttime);
  txn = NULL;

#if CC_ALG == MAAT
  delete uncommitted_writes;
  delete uncommitted_writes_y;
  delete uncommitted_reads;
#endif
#if CC_ALG == CALVIN
  calvin_locked_rows.release();
#endif
  txn_ready = true;
}

void TxnManager::reset_query() {
#if WORKLOAD == YCSB
  ((YCSBQuery*)query)->reset();
#elif WORKLOAD == TPCC
  ((TPCCQuery*)query)->reset();
#elif WORKLOAD == PPS
  ((PPSQuery*)query)->reset();
#endif
}

RC TxnManager::commit() {
  DEBUG("Commit %ld\n",get_txn_id());
  release_locks(RCOK);
#if CC_ALG == MAAT
  time_table.release(get_thd_id(),get_txn_id());
#endif
  commit_stats();
#if LOGGING
    LogRecord * record = logger.createRecord(get_txn_id(),L_NOTIFY,0,0);
    if(g_repl_cnt > 0) {
      msg_queue.enqueue(get_thd_id(),Message::create_message(record,LOG_MSG),g_node_id + g_node_cnt + g_client_node_cnt); 
    }
  logger.enqueueRecord(record);
  return WAIT;
#endif
  return Commit;
}

RC TxnManager::abort() {
  if(aborted)
    return Abort;
  DEBUG("Abort %ld\n",get_txn_id());
  txn->rc = Abort;
  INC_STATS(get_thd_id(),total_txn_abort_cnt,1);
  txn_stats.abort_cnt++;
  if(IS_LOCAL(get_txn_id())) {
    INC_STATS(get_thd_id(), local_txn_abort_cnt, 1);
  } else {
    INC_STATS(get_thd_id(), remote_txn_abort_cnt, 1);
    txn_stats.abort_stats(get_thd_id());
  }

  aborted = true;
  release_locks(Abort);
#if CC_ALG == MAAT
  //assert(time_table.get_state(get_txn_id()) == MAAT_ABORTED);
  time_table.release(get_thd_id(),get_txn_id());
#endif

  uint64_t timespan = get_sys_clock() - txn_stats.restart_starttime;
  if (IS_LOCAL(get_txn_id()) && warmup_done) {
      INC_STATS_ARR(get_thd_id(),start_abort_commit_latency, timespan);
  }
  /*
  // latency from most recent start or restart of transaction
  PRINT_LATENCY("lat_s %ld %ld 0 %f %f %f %f %f %f 0.0\n"
          , get_txn_id()
          , txn_stats.work_queue_cnt
          , (double) timespan / BILLION
          , (double) txn_stats.work_queue_time / BILLION
          , (double) txn_stats.msg_queue_time / BILLION
          , (double) txn_stats.cc_block_time / BILLION
          , (double) txn_stats.cc_time / BILLION
          , (double) txn_stats.process_time / BILLION
          );
          */
  //commit_stats();
  return Abort;
}

RC TxnManager::start_abort() {
  txn->rc = Abort;
  DEBUG("%ld start_abort\n",get_txn_id());
  if(query->partitions_touched.size() > 1) {
    send_finish_messages();
    abort();
    return Abort;
  } 
  return abort();
}

RC TxnManager::start_commit() {
  RC rc = RCOK;
  DEBUG("%ld start_commit RO?%d\n",get_txn_id(),query->readonly());
  if(is_multi_part()) {
    if(!query->readonly() || CC_ALG == OCC || CC_ALG == MAAT) {
      // send prepare messages
      send_prepare_messages();
      rc = WAIT_REM;
    } else {
      send_finish_messages();
      rsp_cnt = 0;
      rc = commit();
    }
  } else { // is not multi-part
    rc = validate();
    if(rc == RCOK)
      rc = commit();
    else
      start_abort();
  }
  return rc;
}

void TxnManager::send_prepare_messages() {
  rsp_cnt = query->partitions_touched.size() - 1;
  DEBUG("%ld Send PREPARE messages to %d\n",get_txn_id(),rsp_cnt);
  for(uint64_t i = 0; i < query->partitions_touched.size(); i++) {
    if(GET_NODE_ID(query->partitions_touched[i]) == g_node_id) {
      continue;
    }
    msg_queue.enqueue(get_thd_id(),Message::create_message(this,RPREPARE),GET_NODE_ID(query->partitions_touched[i]));
  }
}

void TxnManager::send_finish_messages() {
  rsp_cnt = query->partitions_touched.size() - 1;
  assert(IS_LOCAL(get_txn_id()));
  DEBUG("%ld Send FINISH messages to %d\n",get_txn_id(),rsp_cnt);
  for(uint64_t i = 0; i < query->partitions_touched.size(); i++) {
    if(GET_NODE_ID(query->partitions_touched[i]) == g_node_id) {
      continue;
    }
    msg_queue.enqueue(get_thd_id(),Message::create_message(this,RFIN),GET_NODE_ID(query->partitions_touched[i]));
  }
}

int TxnManager::received_response(RC rc) {
  assert(txn->rc == RCOK || txn->rc == Abort);
  if(txn->rc == RCOK)
    txn->rc = rc;
#if CC_ALG == CALVIN
  ++rsp_cnt;
#else
  --rsp_cnt;
#endif
  return rsp_cnt;
}

bool TxnManager::waiting_for_response() {
  return rsp_cnt > 0;
}

bool TxnManager::is_multi_part() {
  return query->partitions_touched.size() > 1;
  //return query->partitions.size() > 1;
}

void TxnManager::commit_stats() {
    uint64_t commit_time = get_sys_clock();
    uint64_t timespan_short = commit_time - txn_stats.restart_starttime;
    uint64_t timespan_long  = commit_time - txn_stats.starttime;
    INC_STATS(get_thd_id(),total_txn_commit_cnt,1);

    if(!IS_LOCAL(get_txn_id()) && CC_ALG != CALVIN) {
      INC_STATS(get_thd_id(),remote_txn_commit_cnt,1);
      txn_stats.commit_stats(get_thd_id(),get_txn_id(),get_batch_id(), timespan_long, timespan_short);
      return;
    }


    INC_STATS(get_thd_id(),txn_cnt,1);
    INC_STATS(get_thd_id(),local_txn_commit_cnt,1);
    INC_STATS(get_thd_id(), txn_run_time, timespan_long);
    if(query->partitions_touched.size() > 1) {
      INC_STATS(get_thd_id(),multi_part_txn_cnt,1);
      INC_STATS(get_thd_id(),multi_part_txn_run_time,timespan_long);
    } else {
      INC_STATS(get_thd_id(),single_part_txn_cnt,1);
      INC_STATS(get_thd_id(),single_part_txn_run_time,timespan_long);
    }
    /*if(cflt) {
      INC_STATS(get_thd_id(),cflt_cnt_txn,1);
    }*/
    txn_stats.commit_stats(get_thd_id(),get_txn_id(),get_batch_id(),timespan_long, timespan_short);
  #if CC_ALG == CALVIN
    return;
  #endif

    INC_STATS_ARR(get_thd_id(),start_abort_commit_latency, timespan_short);
    INC_STATS_ARR(get_thd_id(),last_start_commit_latency, timespan_short);
    INC_STATS_ARR(get_thd_id(),first_start_commit_latency, timespan_long);

    assert(query->partitions_touched.size() > 0);
    INC_STATS(get_thd_id(),parts_touched,query->partitions_touched.size());
    INC_STATS(get_thd_id(),part_cnt[query->partitions_touched.size()-1],1);
    for(uint64_t i = 0 ; i < query->partitions_touched.size(); i++) {
        INC_STATS(get_thd_id(),part_acc[query->partitions_touched[i]],1);
    }
}

void TxnManager::register_thread(Thread * h_thd) {
  this->h_thd = h_thd;
#if CC_ALG == HSTORE || CC_ALG == HSTORE_SPEC
  this->active_part = GET_PART_ID_FROM_IDX(get_thd_id());
#endif
}

void TxnManager::set_txn_id(txnid_t txn_id) {
	txn->txn_id = txn_id;
}

txnid_t TxnManager::get_txn_id() {
	return txn->txn_id;
}

Workload * TxnManager::get_wl() {
	return h_wl;
}

uint64_t TxnManager::get_thd_id() {
  if(h_thd)
    return h_thd->get_thd_id();
  else
    return 0;
}

BaseQuery * TxnManager::get_query() {
	return query;
}
void TxnManager::set_query(BaseQuery * qry) {
	query = qry;
}

void TxnManager::set_timestamp(ts_t timestamp) {
	txn->timestamp = timestamp;
}

ts_t TxnManager::get_timestamp() {
	return txn->timestamp;
}

void TxnManager::set_start_timestamp(uint64_t start_timestamp) {
	txn->start_timestamp = start_timestamp;
}

ts_t TxnManager::get_start_timestamp() {
	return txn->start_timestamp;
}

uint64_t TxnManager::incr_lr() {
  //ATOM_ADD(this->rsp_cnt,i);
  uint64_t result;
  sem_wait(&rsp_mutex);
  result = ++this->lock_ready_cnt;
  sem_post(&rsp_mutex);
  return result;
}

uint64_t TxnManager::decr_lr() {
  //ATOM_SUB(this->rsp_cnt,i);
  uint64_t result;
  sem_wait(&rsp_mutex);
  result = --this->lock_ready_cnt;
  sem_post(&rsp_mutex);
  return result;
}
uint64_t TxnManager::incr_rsp(int i) {
  //ATOM_ADD(this->rsp_cnt,i);
  uint64_t result;
  sem_wait(&rsp_mutex);
  result = ++this->rsp_cnt;
  sem_post(&rsp_mutex);
  return result;
}

uint64_t TxnManager::decr_rsp(int i) {
  //ATOM_SUB(this->rsp_cnt,i);
  uint64_t result;
  sem_wait(&rsp_mutex);
  result = --this->rsp_cnt;
  sem_post(&rsp_mutex);
  return result;
}

void TxnManager::release_last_row_lock() {
  assert(txn->row_cnt > 0);
  row_t * orig_r = txn->accesses[txn->row_cnt-1]->orig_row;
  access_t type = txn->accesses[txn->row_cnt-1]->type;
  orig_r->return_row(RCOK, type, this, NULL);
  //txn->accesses[txn->row_cnt-1]->orig_row = NULL;
}

void TxnManager::cleanup_row(RC rc, uint64_t rid) {
    access_t type = txn->accesses[rid]->type;
    if (type == WR && rc == Abort && CC_ALG != MAAT) {
        type = XP;
    }

    // Handle calvin elsewhere
#if CC_ALG != CALVIN
#if ISOLATION_LEVEL != READ_UNCOMMITTED
    row_t * orig_r = txn->accesses[rid]->orig_row;
    if (ROLL_BACK && type == XP &&
                (CC_ALG == DL_DETECT ||
                CC_ALG == NO_WAIT ||
                CC_ALG == WAIT_DIE ||
      CC_ALG == HSTORE ||
      CC_ALG == HSTORE_SPEC
      ))
    {
        orig_r->return_row(rc,type, this, txn->accesses[rid]->orig_data);
    } else {
#if ISOLATION_LEVEL == READ_COMMITTED
        if(type == WR) {
          orig_r->return_row(rc,type, this, txn->accesses[rid]->data);
        }
#else
        orig_r->return_row(rc,type, this, txn->accesses[rid]->data);
#endif
    }
#endif

#if ROLL_BACK && (CC_ALG == NO_WAIT || CC_ALG == WAIT_DIE || CC_ALG == HSTORE || CC_ALG == HSTORE_SPEC)
    if (type == WR) {
        //printf("free 10 %ld\n",get_txn_id());
              txn->accesses[rid]->orig_data->free_row();
        DEBUG_M("TxnManager::cleanup row_t free\n");
        row_pool.put(get_thd_id(),txn->accesses[rid]->orig_data);
        if(rc == RCOK) {
            INC_STATS(get_thd_id(),record_write_cnt,1);
            ++txn_stats.write_cnt;
        }
    }
#endif
#endif
    txn->accesses[rid]->data = NULL;

}

void TxnManager::cleanup(RC rc) {
#if CC_ALG == OCC && MODE == NORMAL_MODE
    occ_man.finish(rc,this);
#endif

    ts_t starttime = get_sys_clock();
    uint64_t row_cnt = txn->accesses.get_count();
    assert(txn->accesses.get_count() == txn->row_cnt);
    //assert((WORKLOAD == YCSB && row_cnt <= g_req_per_query) || (WORKLOAD == TPCC && row_cnt <= g_max_items_per_txn*2 + 3));


    DEBUG("Cleanup %ld %ld\n",get_txn_id(),row_cnt);
	for (int rid = row_cnt - 1; rid >= 0; rid --) {
	    cleanup_row(rc,rid);
	}
#if CC_ALG == CALVIN
	// cleanup locked rows
    for (uint64_t i = 0; i < calvin_locked_rows.size(); i++) {
        row_t * row = calvin_locked_rows[i];
        row->return_row(rc,RD,this,row);
    }
#endif

	if (rc == Abort) {
	    txn->release_inserts(get_thd_id());
	    txn->insert_rows.clear();

        INC_STATS(get_thd_id(), abort_time, get_sys_clock() - starttime);
	} 
}

RC TxnManager::get_lock(row_t * row, access_t type) {
    if (calvin_locked_rows.contains(row)) {
        return RCOK;
    }
    calvin_locked_rows.add(row);
    RC rc = row->get_lock(type, this);
    if(rc == WAIT) {
      INC_STATS(get_thd_id(), txn_wait_cnt, 1);
    }
    return rc;
}

RC TxnManager::get_row(row_t * row, access_t type, row_t *& row_rtn) {
    uint64_t starttime = get_sys_clock();
    uint64_t timespan;
    RC rc = RCOK;
    DEBUG_M("TxnManager::get_row access alloc\n");
    Access * access;
    access_pool.get(get_thd_id(),access);
    //uint64_t row_cnt = txn->row_cnt;
    //assert(txn->accesses.get_count() - 1 == row_cnt);

    this->last_row = row;
    this->last_type = type;

    rc = row->get_row(type, this, access->data); // Concurrency Control - lock get etc.

    TransactionF* transactionF;
    switch(type){
      case RD :
        transactionF = framework->getTransaction(txn->txn_id);
        framework->read(transactionF, row->get_primary_key());
        break;
      case WR :
        transactionF = framework->getTransaction(txn->txn_id);
        framework->write(transactionF, row->get_primary_key(), row);
        break;
      default:
        break;
    }//TODO - o codigo seguinte de logging e rollback tem de ser colocado na framework ou no CC.cpp

    if (rc == Abort || rc == WAIT) {
        row_rtn = NULL;
        DEBUG_M("TxnManager::get_row(abort) access free\n");
        access_pool.put(get_thd_id(),access);
        timespan = get_sys_clock() - starttime;
        INC_STATS(get_thd_id(), txn_manager_time, timespan);
        INC_STATS(get_thd_id(), txn_conflict_cnt, 1);
        //cflt = true;
#if DEBUG_TIMELINE
        printf("CONFLICT %ld %ld\n",get_txn_id(),get_sys_clock());
#endif
        return rc;
    }
	access->type = type;
	access->orig_row = row;
  

#if ROLL_BACK && (CC_ALG == DL_DETECT || CC_ALG == NO_WAIT || CC_ALG == WAIT_DIE || CC_ALG == HSTORE || CC_ALG == HSTORE_SPEC)
	if (type == WR) {
    //printf("alloc 10 %ld\n",get_txn_id());
    uint64_t part_id = row->get_part_id();
    DEBUG_M("TxnManager::get_row row_t alloc\n")
    row_pool.get(get_thd_id(),access->orig_data);
    access->orig_data->init(row->get_table(), part_id, 0);
    access->orig_data->copy(row);
    assert(access->orig_data->get_schema() == row->get_schema());

    // ARIES-style physiological logging
#if LOGGING
    //LogRecord * record = logger.createRecord(LRT_UPDATE,L_UPDATE,get_txn_id(),part_id,row->get_table()->get_table_id(),row->get_primary_key());
    LogRecord * record = logger.createRecord(get_txn_id(),L_UPDATE,row->get_table()->get_table_id(),row->get_primary_key());
    if(g_repl_cnt > 0) {
      msg_queue.enqueue(get_thd_id(),Message::create_message(record,LOG_MSG),g_node_id + g_node_cnt + g_client_node_cnt); 
    }
    logger.enqueueRecord(record);
#endif

	}
#endif

	++txn->row_cnt;
	if (type == WR)
		++txn->write_cnt;

  txn->accesses.add(access);

	timespan = get_sys_clock() - starttime;
	INC_STATS(get_thd_id(), txn_manager_time, timespan);
	row_rtn  = access->data;

  if(CC_ALG == HSTORE || CC_ALG == HSTORE_SPEC || CC_ALG == CALVIN)
    assert(rc == RCOK);
  return rc;
}

RC TxnManager::get_row_post_wait(row_t *& row_rtn) {
  assert(CC_ALG != HSTORE && CC_ALG != HSTORE_SPEC);

	uint64_t starttime = get_sys_clock();
  row_t * row = this->last_row;
  access_t type = this->last_type;
  assert(row != NULL);
  DEBUG_M("TxnManager::get_row_post_wait access alloc\n")
  Access * access;
  access_pool.get(get_thd_id(),access);

  row->get_row_post_wait(type,this,access->data);

	access->type = type;
	access->orig_row = row;
#if ROLL_BACK && (CC_ALG == DL_DETECT || CC_ALG == NO_WAIT || CC_ALG == WAIT_DIE)
	if (type == WR) {
	  uint64_t part_id = row->get_part_id();
    //printf("alloc 10 %ld\n",get_txn_id());
    DEBUG_M("TxnManager::get_row_post_wait row_t alloc\n")
    row_pool.get(get_thd_id(),access->orig_data);
		access->orig_data->init(row->get_table(), part_id, 0);
		access->orig_data->copy(row);
	}
#endif

	++txn->row_cnt;
	if (type == WR)
		++txn->write_cnt;

  TransactionF* transactionF;
  switch(type){ //Row_rtn = is the row that is returned! 
    case RD :
      transactionF = framework->getTransaction(txn->txn_id);
      framework->read(transactionF, row->get_primary_key()); //TODO
      break;
    case WR :
      transactionF = framework->getTransaction(txn->txn_id);
      framework->write(transactionF, row->get_primary_key(), row); //Colocar key e value apropriado. TODO
      break;
    default:
      break;
  }

  txn->accesses.add(access);
	uint64_t timespan = get_sys_clock() - starttime;
	INC_STATS(get_thd_id(), txn_manager_time, timespan);
	this->last_row_rtn  = access->data;
	row_rtn  = access->data;
  return RCOK;

}

void TxnManager::insert_row(row_t * row, table_t * table) {
	if (CC_ALG == HSTORE || CC_ALG == HSTORE_SPEC)
		return;
	assert(txn->insert_rows.size() < MAX_ROW_PER_TXN);
  txn->insert_rows.add(row);
  TransactionF* transactionF = framework->getTransaction(txn->txn_id);
  framework->write(transactionF, row->get_primary_key(), row); //Colocar value apropriado. TODO
}

itemid_t *
TxnManager::index_read(INDEX * index, idx_key_t key, int part_id) {
	uint64_t starttime = get_sys_clock();

	itemid_t * item;
	index->index_read(key, item, part_id, get_thd_id());

  uint64_t t = get_sys_clock() - starttime;
  INC_STATS(get_thd_id(), txn_index_time, t);
  //txn_time_idx += t;

	return item;
}

itemid_t *
TxnManager::index_read(INDEX * index, idx_key_t key, int part_id, int count) {
	uint64_t starttime = get_sys_clock();

	itemid_t * item;
	index->index_read(key, count, item, part_id);

  uint64_t t = get_sys_clock() - starttime;
  INC_STATS(get_thd_id(), txn_index_time, t);
  //txn_time_idx += t;

	return item;
}


RC TxnManager::validate() {
#if MODE != NORMAL_MODE
  return RCOK;
#endif
  if (CC_ALG != OCC && CC_ALG != MAAT) {
      return RCOK;
  }
  RC rc = RCOK;
  uint64_t starttime = get_sys_clock();
  if(CC_ALG == OCC && rc == RCOK)
    rc = occ_man.validate(this);
  if(CC_ALG == MAAT && rc == RCOK) {
    rc = maat_man.validate(this);
    // Note: home node must be last to validate
    if(IS_LOCAL(get_txn_id()) && rc == RCOK) {
      rc = maat_man.find_bound(this);
    }
  }
  INC_STATS(get_thd_id(),txn_validate_time,get_sys_clock() - starttime);
  return rc;
}

RC
TxnManager::send_remote_reads() {
  assert(CC_ALG == CALVIN);
#if !YCSB_ABORT_MODE && WORKLOAD == YCSB
  return RCOK;
#endif
  assert(query->active_nodes.size() == g_node_cnt);
  for(uint64_t i = 0; i < query->active_nodes.size(); i++) {
    if(i == g_node_id)
      continue;
    if(query->active_nodes[i] == 1) {
      DEBUG("(%ld,%ld) send_remote_read to %ld\n",get_txn_id(),get_batch_id(),i);
      msg_queue.enqueue(get_thd_id(),Message::create_message(this,RFWD),i);
    }
  }
  return RCOK;

}

bool TxnManager::calvin_exec_phase_done() {
  bool ready =  (phase == CALVIN_DONE) && (get_rc() != WAIT);
  if(ready) {
    DEBUG("(%ld,%ld) calvin exec phase done!\n",txn->txn_id,txn->batch_id);
  }
  return ready;
}

bool TxnManager::calvin_collect_phase_done() {
  bool ready =  (phase == CALVIN_COLLECT_RD) && (get_rsp_cnt() == calvin_expected_rsp_cnt);
  if(ready) {
    DEBUG("(%ld,%ld) calvin collect phase done!\n",txn->txn_id,txn->batch_id);
  }
  return ready;
}

void TxnManager::release_locks(RC rc) {
	uint64_t starttime = get_sys_clock();

  cleanup(rc);

	uint64_t timespan = (get_sys_clock() - starttime);
	INC_STATS(get_thd_id(), txn_cleanup_time,  timespan);
}

#include "concurrency.h"
#include "global.h"
#include "mem_alloc.h"
#include "content.h"
#include "transaction.h"
#include "order.h"

CC_Lock::CC_Lock(Content* c){
    content = c;
    owners_size = 1;//1031;
    owners = NULL;
    owners = (CC_Entry**) mem_allocator.alloc(sizeof(CC_Entry*)*owners_size);
    for(uint64_t i = 0; i < owners_size; i++)
        owners[i] = NULL;
    waiters_head = NULL;
    waiters_tail = NULL;
    owner_cnt = 0;
    waiter_cnt = 0;
    max_owner_ts = 0;
    latch = new pthread_mutex_t;
    pthread_mutex_init(latch, NULL);
    lockType = LOCK_NONE;
    
}

Content* CC_Lock::read(TransactionF* transaction){
    if(getControl(transaction, RD)){
        transaction->addToReadSet(content); 
        return content;
    }
    return return_invalidContent();
}

Content* CC_Lock::write(TransactionF* transaction, Content* content1){
   if(getControl(transaction, WR)){
        transaction->addToWriteSet(content1);

        previousContent = content;
        content = content1; 

        return content; //newly content
   }
    return return_invalidContent();
   
}

bool CC_Lock::validate(TransactionF* transaction){
    transaction->setValidated(true);
    return true;
}

bool CC_Lock::getControl(TransactionF* transaction, access_t operation){ 
    //Content* returnContent = NULL;
    bool success = false;

    if (g_central_man) {
        order->lockGet(content);
    }else{
        pthread_mutex_lock( latch );
    }
    lock_t type = getOperationLock(operation);
    bool conflict = conflict_lock(lockType, type);

    if (CC_ALG == WAIT_DIE && !conflict) {
		if (waiters_head && transaction->getTimestampStartup() < waiters_head->transaction->getTimestampStartup()) {
			conflict = true;
		}
	}
    if (CC_ALG == CALVIN && !conflict) {
        if(waiters_head)
          conflict = true;
    }
	
    if (conflict) {
    //printf("conflict! rid%ld txnid%ld ",_row->get_primary_key(),txn->get_txn_id());
        // Cannot be added to the owner list.
        if (CC_ALG == NO_WAIT) {
            //abort
            //returnContent = return_invalidContent();
            success = false;
            goto final;

        } else if (CC_ALG == WAIT_DIE) {
            ///////////////////////////////////////////////////////////
            //  - T is the txn currently running
            //  IF T.ts > min ts of owners
            //      T can wait
            //  ELSE
            //      T should abort
            //////////////////////////////////////////////////////////

      //bool canwait = txn->get_timestamp() > max_owner_ts;
            bool canwait = true;
            CC_Entry * en;
            for(uint64_t i = 0; i < owners_size; i++) {
              en = owners[i];
              while (en != NULL) {
                //assert(txn->get_txn_id() != en->txn->get_txn_id());
                //assert(txn->get_timestamp() != en->txn->get_timestamp());
                if (transaction->getTimestampStartup()> en->transaction->getTimestampStartup()) {
                  //printf("abort %ld %ld -- %ld -- %f\n",txn->get_txn_id(),en->txn->get_txn_id(),_row->get_primary_key(),(float)(txn->get_timestamp() - en->txn->get_timestamp()) / BILLION);
                  canwait = false;
                  break;
                }
                en = en->next;
              }
              if(!canwait)
                break;
            }
            if (canwait) {
                // insert txn to the right position
                // the waiter list is always in timestamp order
                CC_Entry * entry = get_entry();
                entry->ts = get_sys_clock();
                        entry->transaction = transaction;
                        entry->type = type;
                entry->ts = get_sys_clock();
                        entry->transaction = transaction;
                        entry->type = type;
                CC_Entry * en;
                //txn->lock_ready = false;
                ATOM_CAS(transaction->lockReady,1,0);
                transaction->incr_lr(); // txn.cpp implementation
                en = waiters_head;
                while (en != NULL && transaction->getTimestampStartup() < en->transaction->getTimestampStartup()) {
                    en = en->next;
                }
                if (en) {
                    LIST_INSERT_BEFORE(en, entry,waiters_head);
                } else {
                    LIST_PUT_TAIL(waiters_head, waiters_tail, entry);
                }

                waiter_cnt++;
                //rc = WAIT; wait TODO
                //returnContent = return_invalidContent();
                success = false;
            } else {
              //rc = Abort;
              //Abort TODO
              //returnContent = return_invalidContent();
              success = false;
            }
        } else if (CC_ALG == CALVIN){
            CC_Entry * entry = get_entry();
            entry->ts = get_sys_clock();
            entry->transaction = transaction;
            entry->type = type;
            LIST_PUT_TAIL(waiters_head, waiters_tail, entry);
            waiter_cnt++;
            /*
            if (txn->twopl_wait_start == 0) {
                txn->twopl_wait_start = get_sys_clock();
            }
            */
            //txn->lock_ready = false;
            ATOM_CAS(transaction->lockReady,true,false);
            transaction->incr_lr();
            //rc = WAIT;
            //WAIT TODO
            
            //returnContent = return_invalidContent();
            success = false;
        }
    }else{

#if DEBUG_TIMELINE
        printf("LOCK %ld %ld\n",entry->txn->get_txn_id(),entry->start_ts);
#endif
#if CC_ALG != NO_WAIT
        CC_Entry * entry = get_entry();
        entry->type = type;
        entry->ts = get_sys_clock();
        entry->transaction = transaction;
        STACK_PUSH(owners[hash(transaction->getId())], entry);
#endif
        if(owner_cnt > 0) {
          assert(type == LOCK_SH);
        }
        if(transaction->getTimestampStartup() > max_owner_ts) {
          max_owner_ts = transaction->getTimestampStartup();
        }
        owner_cnt ++;
        if(lockType == LOCK_NONE) {
          //own_starttime = get_sys_clock();
        }
        lockType = type;
        //returnContent = content;
        success = true;
    }
final:
    if (g_central_man){
        //manager release lock
        order->lockRelease(content);
    }else{
        pthread_mutex_unlock( latch );
    }

	//return rc;
    // if(returnContent!= NULL && returnContent->getValue()!= NULL){
    //     return content;
    // }
    return success;
    //return returnContent;

}

void CC_Lock::releaseControl(TransactionF* transaction){

#if CC_ALG == CALVIN
    // if (transaction->isRecon()) {
    //     return RCOK;
    // }
#endif
      if (g_central_man){
          order->lockGet(content);
          //manager get the lock
      }else {
          pthread_mutex_lock( latch );
    }

      // If CC is NO_WAIT or WAIT_DIE, txn should own this lock
      // What about Calvin?
#if CC_ALG == NO_WAIT
      if(owner_cnt > 0){
        owner_cnt--;
      }
      if (owner_cnt == 0) {
        lockType = LOCK_NONE;
      }

#else

      // Try to find the entry in the owners
      CC_Entry * en = owners[hash(transaction->getId())];
      CC_Entry * prev = NULL;

      while (en != NULL && en->transaction != transaction) {
          prev = en;
          en = en->next;
      }

      if (en) { // find the entry in the owner list
          if (prev) prev->next = en->next;
          else owners[hash(transaction->getId())] = en->next;
          return_entry(en);
          owner_cnt --;
      if (owner_cnt == 0) {
        lockType = LOCK_NONE;
      }

    } else {
      assert(false);
          en = waiters_head;
          while (en != NULL && en->transaction != transaction)
              en = en->next;
          ASSERT(en);

          LIST_REMOVE(en);
          if (en == waiters_head)
              waiters_head = en->next;
          if (en == waiters_tail)
              waiters_tail = en->prev;
          return_entry(en);
          waiter_cnt --;
      }
#endif

      if (owner_cnt == 0)
          ASSERT(lockType == LOCK_NONE);

      CC_Entry * entry;
      // If any waiter can join the owners, just do it!
      while (waiters_head && !conflict_lock(lockType, waiters_head->type)) {
          LIST_GET_HEAD(waiters_head, waiters_tail, entry);

#if CC_ALG != NO_WAIT
          STACK_PUSH(owners[hash(entry->transaction->getId())], entry);
#endif 
          owner_cnt ++;
          waiter_cnt --;
          if(entry->transaction->getTimestampStartup() > max_owner_ts) {
              max_owner_ts = entry->transaction->getTimestampStartup();
          }
          ASSERT(entry->transaction->lockReady == false);
          if(entry->transaction->decr_lr() == 0) {
              if(ATOM_CAS(entry->transaction->lockReady,false,true)) {
                  //txn_table.restart_txn(transaction->get_thd_id(),entry->transaction->get_txn_id(),entry->transaction->get_batch_id());
                  
              }
          }
          
          lockType = entry->type;
#if CC_AlG == NO_WAIT
          return_entry(entry);
#endif
      }

        if (g_central_man){
          order->lockRelease(content);
          //manager release latch
        }else{
          pthread_mutex_unlock( latch );
        }

    //return RCOK;
}

bool CC_Lock::conflict_lock(lock_t l1, lock_t l2) {
    if (l1 == LOCK_NONE || l2 == LOCK_NONE)
        return false;
    else if (l1 == LOCK_EX || l2 == LOCK_EX)
        return true;
    else
        return false;
}

lock_t CC_Lock::getOperationLock(access_t operation){
    switch(operation){
        case RD:
            return lock_t::LOCK_SH;
        case WR:
            return lock_t::LOCK_EX;
        default:
            return lock_t::LOCK_NONE;
    }
}

CC_Entry* CC_Lock::get_entry(){
    CC_Entry * entry = (CC_Entry *) mem_allocator.alloc(sizeof(CC_Entry));
        entry->type = LOCK_NONE;
        entry->transaction = NULL;
        //DEBUG_M("row_lock::get_entry alloc %lx\n",(uint64_t)entry);
        return entry;
}

void CC_Lock::return_entry(CC_Entry * entry) {
    //DEBUG_M("row_lock::return_entry free %lx\n",(uint64_t)entry);
    mem_allocator.free(entry, sizeof(CC_Entry));
}

Content* CC_Lock::return_invalidContent(){
    return new Content(-1, NULL);
}


void CC_Lock::commitWrites(){ //dont call after abort! only after commit
    delete previousContent; 
}

void CC_Lock::abortWrites(){
    delete content;
    content = previousContent;
}
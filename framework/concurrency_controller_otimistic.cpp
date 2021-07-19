#include "concurrency_controller.h"
#include "configuration.cpp"

ConcurrencyControllerOtimistic::ConcurrencyControllerOtimistic(){
    sem_init(&_semaphore, 0, 1);
	tnc = 0;
	his_len = 0;
	active_len = 0;
	active = NULL;
	lock_all = false;
}

ConcurrencyControllerOtimistic::set_ent::set_ent() {
	set_size = 0;
	txn = NULL;
	rows = NULL;
	next = NULL;
}

void ConcurrencyControllerOtimistic::initContent(uint64_t key, row_t* row){
	Content* c = new Content(key, row);
    InterfaceConcurrencyControl* concurrencyControl = configuration::initConcurrencyControl(c);
    concurrencyControlMap.insert(std::pair<uint64_t, InterfaceConcurrencyControl*>(c->getKey(), concurrencyControl));
}

void ConcurrencyControllerOtimistic::write(TransactionF* transaction, uint64_t key, row_t* row) {

    InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
    Content* content = new Content(key, row);
    Content* returnContent = concurrencyControl->write(transaction, content);

	printf("write %lu from the transaction %lu \n", returnContent->getValue()->get_primary_key(), transaction->getId());
    fflush(stdout);
}

void ConcurrencyControllerOtimistic::read(TransactionF* transaction, uint64_t key) {
	
    InterfaceConcurrencyControl* concurrencyControl = concurrencyControlMap.at(key);
    Content* returnContent = concurrencyControl->read(transaction);

	printf("read %lu from the transaction %lu \n", returnContent->getValue()->get_primary_key(), transaction->getId());
    fflush(stdout);
}

bool ConcurrencyControllerOtimistic::validate(TransactionF* transaction) {
	if(transaction->isValidated()) //change TODO - aborted txns are validated twice
		return true;
	
#if PER_ROW_VALID
	if(per_row_validate(transaction)){
		printf("validate transaction %lu \n", transaction->getId());
		fflush(stdout);
		transaction->setValidated(true);
	}
#else
	if(central_validate(transaction)){
		printf("validate transaction %lu \n", transaction->getId());
		fflush(stdout);
		transaction->setValidated(true);
	}
#endif
	return transaction->isValidated();
}

void ConcurrencyControllerOtimistic::commit(TransactionF* transaction) {
	list<Content*> listOfOperations=transaction->getWriteSet();

    for(Content* c : listOfOperations){
        concurrencyControlMap.at(c->getKey())->commitWrites(); //make durable the writes
    }
}

void ConcurrencyControllerOtimistic::abort(TransactionF* transaction) {
	list<Content*> listOfOperations=transaction->getWriteSet();

    for(Content* c : listOfOperations){
        concurrencyControlMap.at(c->getKey())->abortWrites(); //abort the writes
    }
}

void ConcurrencyControllerOtimistic::finish(TransactionF* transaction) {
	#if PER_ROW_VALID
		per_row_finish(transaction);
	#else
		central_finish(transaction);
	#endif

	transaction->clearSets();
}

//private

bool byPrimaryKey(Content *a, Content *b){
    return (a->getKey() < b->getKey());
}
bool ConcurrencyControllerOtimistic::per_row_validate(TransactionF * transaction){
	// sort all rows accessed in primary key order.
    std::list<Content*> listReadSet = transaction->getReadSet();
	std::list<Content*> listWriteSet = transaction->getWriteSet();
    listReadSet.sort(byPrimaryKey);
	listWriteSet.sort(byPrimaryKey);

	// lock all rows in the readset and writeset.
	// Validate each access
	bool ok = true;
	int lock_cnt = 0;

    for(Content* c : listReadSet){
        if(!ok)
            return ok;
        lock_cnt ++;
        concurrencyControlMap.at(c->getKey())->getControl(transaction, RD); // latch()
        ok = concurrencyControlMap.at(c->getKey())->validate(transaction);
    }
	for(Content* c : listWriteSet){
        if(!ok)
            return ok;
		lock_cnt ++;
        concurrencyControlMap.at(c->getKey())->getControl(transaction, WR); // latch()
        ok = concurrencyControlMap.at(c->getKey())->validate(transaction);
	}

	return ok;

}
bool ConcurrencyControllerOtimistic::central_validate(TransactionF * transaction){
    uint64_t start_tn = transaction->getTimestampStartup();
    uint64_t finish_tn;
	//set_ent ** finish_active;
	//set_ent * finish_active[f_active_len];
	uint64_t f_active_len;
	bool valid = true;
	// OptCC is centralized. No need to do per partition malloc.
	set_ent * wset;
	set_ent * rset;
	get_rw_set(transaction, rset, wset);
	bool readonly = (wset->set_size == 0);
	set_ent * his;
	set_ent * ent;
	int n = 0;
  int stop __attribute__((unused));

	//pthread_mutex_lock( &latch );
  sem_wait(&_semaphore);
	//finish_tn = tnc;
  assert(!g_ts_batch_alloc);
	order->timestampCommit(transaction, Metadata());
	finish_tn = transaction->getTimestampCommit();
	ent = active;
	f_active_len = active_len;
	set_ent * finish_active[f_active_len];
	//finish_active = (set_ent**) mem_allocator.alloc(sizeof(set_ent *) * f_active_len);
	while (ent != NULL) {
		finish_active[n++] = ent;
		ent = ent->next;
	}
	if ( !readonly ) {
		active_len ++;
		STACK_PUSH(active, wset);
	}
	his = history;
	//pthread_mutex_unlock( &latch );
  sem_post(&_semaphore);

  uint64_t checked = 0;
  uint64_t active_checked = 0;
  uint64_t hist_checked = 0;
  stop = 0;
	if (finish_tn > start_tn) {
		while (his && his->tn > finish_tn) 
			his = his->next;
		while (his && his->tn > start_tn) {
      ++hist_checked;
      ++checked;
			valid = test_valid(his, rset);
			if (!valid) { 
				goto final;
      }
			his = his->next;
		}
	}

  stop = 1;
	for (UInt32 i = 0; i < f_active_len; i++) {
		set_ent * wact = finish_active[i];
		++checked;
		++active_checked;
		valid = test_valid(wact, rset);
		if (valid) {
			++checked;
			++active_checked;
			valid = test_valid(wact, wset);
		} 
		if (!valid) {
			goto final;
		}
	}
final:
  /*
	if (valid) 
		txn->cleanup(RCOK);
    */
	mem_allocator.free(rset->rows, sizeof(row_t *) * rset->set_size);
	mem_allocator.free(rset, sizeof(set_ent));
	//mem_allocator.free(finish_active, sizeof(set_ent*)* f_active_len);


	if (valid) {
		return true;
	} else {
		//txn->cleanup(Abort);
    // Optimization: If this is aborting, remove from active set now
      sem_wait(&_semaphore);
        set_ent * act = active;
        set_ent * prev = NULL;
        while (act != NULL && act->txn != transaction) {
          prev = act;
          act = act->next;
        }
        if(act != NULL && act->txn == transaction) {
          if (prev != NULL)
            prev->next = act->next;
          else
            active = act->next;
          active_len --;
        }
      sem_post(&_semaphore);
	  return false;
	}
}
void ConcurrencyControllerOtimistic::per_row_finish(TransactionF * transaction){
if(transaction->isValidated()) {
		// advance the global timestamp and get the end_ts
		transaction->getTimestampCommit();
  }
}
void ConcurrencyControllerOtimistic::central_finish(TransactionF * transaction){
set_ent * wset;
	set_ent * rset;
	get_rw_set(transaction, rset, wset);
	bool readonly = (wset->set_size == 0);

	if (!readonly) {
		// only update active & tnc for non-readonly transactions
//		pthread_mutex_lock( &latch );
  sem_wait(&_semaphore);
		set_ent * act = active;
		set_ent * prev = NULL;
		while (act != NULL && act->txn != transaction) {
			prev = act;
			act = act->next;
		}
    if(act == NULL) {
      assert(!transaction->isValidated());
		  //pthread_mutex_unlock( &latch );
      sem_post(&_semaphore);
      return;
    }
		assert(act->txn == transaction);
		if (prev != NULL)
			prev->next = act->next;
		else
			active = act->next;
		active_len --;
		if (transaction->isValidated()) {
			// remove the assert for performance
      /*
			if (history)
				assert(history->tn == tnc);
      */
			tnc ++;
			wset->tn = tnc;
			STACK_PUSH(history, wset);
			his_len ++;
      //mem_allocator.free(wset->rows, sizeof(row_t *) * wset->set_size);
      //mem_allocator.free(wset, sizeof(set_ent));
		}
	//	pthread_mutex_unlock( &latch );
  sem_post(&_semaphore);
	}
}
void ConcurrencyControllerOtimistic::get_rw_set(TransactionF * transaction, set_ent * &rset, set_ent *& wset) {
	wset = (set_ent*) mem_allocator.alloc(sizeof(set_ent));
	rset = (set_ent*) mem_allocator.alloc(sizeof(set_ent));
	wset->set_size = transaction->getWriteSet().size();
	rset->set_size = transaction->getReadSet().size();
	wset->rows = (Content **) mem_allocator.alloc(sizeof(Content *) * wset->set_size);
	rset->rows = (Content **) mem_allocator.alloc(sizeof(Content *) * rset->set_size);
	wset->txn = transaction;
	rset->txn = transaction;

	UInt32 n = 0, m = 0;

    list<Content*> listOfOperations=transaction->getReadSet();
    for(Content* c : listOfOperations){
        rset->rows[m ++] = concurrencyControlMap.at(c->getKey())->getContent();
    }
    listOfOperations=transaction->getWriteSet();
    for(Content* c : listOfOperations){
        wset->rows[n ++] = concurrencyControlMap.at(c->getKey())->getContent();

    }

	assert(n == wset->set_size);
	assert(m == rset->set_size);
	//return RCOK;
}
bool ConcurrencyControllerOtimistic::test_valid(set_ent * set1, set_ent * set2) {
	for (UInt32 i = 0; i < set1->set_size; i++)
		for (UInt32 j = 0; j < set2->set_size; j++) {
			if (set1->rows[i] == set2->rows[j]) {
				return false;
			}
		}
	return true;
}

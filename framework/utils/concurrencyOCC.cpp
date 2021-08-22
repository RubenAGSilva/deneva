#include "concurrency.h"
#include "mem_alloc.h"

CC_O::CC_O(Content* content1){
	content = content1;
	_latch = (pthread_mutex_t *) mem_allocator.alloc(sizeof(pthread_mutex_t));
	pthread_mutex_init( _latch, NULL );
	sem_init(&_semaphore, 0, 1);
	wts = 0;
	blatch = false;
}

Content* CC_O::read(TransactionF* transaction){
  	sem_wait(&_semaphore);
	if (transaction->getTimestampStartup() < wts) {
			return return_invalidContent(); //abort, maybe remove?
    }else{
		transaction->addToReadSet(content);
	}
  	sem_post(&_semaphore);
	return content;
}
Content* CC_O::write(TransactionF* transaction, Content* content1){
	transaction->addToWriteSet(content1);

	previousContent = content;
	content = content1;
	
	if (PER_ROW_VALID) {
		assert(transaction->getTimestampCommit() > wts);
		wts = transaction->getTimestampCommit();
	}
	return content1;
}
bool CC_O::getControl(TransactionF* transaction, access_t operation){
	sem_wait(&_semaphore);
	return true;
}
void CC_O::releaseControl(TransactionF* transaction){
	sem_post(&_semaphore);
}
bool CC_O::validate(TransactionF* transaction){
	if (transaction->getTimestampCommit() < wts) return false;
	else return true;
}
void CC_O::commitWrites(){
	delete previousContent;
}
void CC_O::abortWrites(){
	delete content;
	content = previousContent;
}

Content* CC_O::return_invalidContent(){
    return new Content(-1, NULL);
}

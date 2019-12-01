/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, as well as the function signatures and
 * the semaphore values (which are to be changed as needed).
 ******************************************************************/


# include <stdio.h>
#include <map>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/time.h>
# include <math.h>
# include <errno.h>
# include <string.h>
# include <pthread.h>
# include <ctype.h>
# include <iostream>
using namespace std;

#define SEM_KEY 0x855556// Change this number as needed

union semun {
    int val;               /* used for SETVAL only */
    struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
    ushort *array;         /* used for GETALL and SETALL */
};

enum sem_index {
    MUTEX,  //mutual exclusion of buffer access
    SPACE,  //check if buffer is not full
    ITEMS,  //check if buffer is not empty
    ID,      //mutual exclusion of ID generation/assignment
    OUTPUT //only 1 thread accessing std::cout. otherwise, some of the output lines
	    //get merged together. 
}; 


/* provided semophore functions */ 
int check_arg (char *);
int sem_create (key_t, int);
int sem_init (int, int, int);
void sem_wait (int, short unsigned int);
void sem_signal (int, short unsigned int);
int sem_close (int);

/* sem_wait, except once @time reached returns non-0 value */ 
int sem_wait_till_time (int id, short unsigned int num, int time); 


/* data structure that contains jobs. 
 * circular queue functionality implemented with an array, the ID of a given job
 * is its index+1 (1-indexed jobs). Jobs are removed from
 * the array once they are accessed by consumers, allowing for this ID to be
 * reused. */
class Buffer {
	private:
		int* queue; 
		int capacity, front, rear; 
		bool empty = true; 
	
	public: 
		/*adds @duration to the back of the queue,
		 * retuns the job ID associated with this duration 
		 * (its index +1) */
		int pushJob(int duration); 

		/* removes the job at the front of the queue, returning its
		 * ID. duration of this job is stored in @duration*/
		int popJob(int &duration); 
		
		//param constructor. queue is initialised on heap 
		//with size of @max. front and rear set to null
		Buffer(int max); 

		~Buffer(); 
}; 

/* wrapping the provided functions in SemophoreSet for ease of use/clarity. 
 * singleton class as feel like it fits intention here well*/ 

enum Errors {
	NO_ERROR,
	INCORRECT_NUMBER_OF_PARAMETERS, 
	INCORRECT_PARAMETER_TYPE, 
	INVALID_PARAMETER_VALUE, 
	FAILURE_TO_INIT_SEMOPHORE, 
	ERROR_INITIALISING_SEMOPHORE_ARRAY, 
	ERROR_GENERATING_PTHREAD
};


std::map<Errors, std::string> const errorMessages {
	{INCORRECT_NUMBER_OF_PARAMETERS, "Incorrect number of parameters provided"}, 
	{INCORRECT_PARAMETER_TYPE, " is an incorrect parameter type: parameters must be numerical"}, 
	{INVALID_PARAMETER_VALUE, "This is an impossible configuration."}, 
	{FAILURE_TO_INIT_SEMOPHORE, " failed to initialise."}, 
	{ERROR_INITIALISING_SEMOPHORE_ARRAY, "Semohore array failed to initialise."}
}; 

int returnErr(int errorCode, const std::string &message = ""); 


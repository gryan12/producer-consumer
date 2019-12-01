/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"



//make queue structure, this way do not need to pass 
//BOTH queue size and pointer to queue object, just
//pointer to queueu structure 



void *producer (void *id);
void *consumer (void *id);
int initSemophores(int q_size); 
int parseArgs(int &q, int &job_no, int&prod_no, int &cons_no, char** argv); 

struct threadParameters {
    int sem_id_;
    int job_no_;   
    int thread_id_; 
    Buffer *buffer_;
    int producer_id; 
    threadParameters(int sem_id, int job_no, int thread_id, Buffer* buffer): sem_id_(sem_id),job_no_(job_no), thread_id_(thread_id), buffer_(buffer) {}
    threadParameters(): sem_id_(0),job_no_(0), buffer_(NULL) {}
};

int main (int argc, char **argv)
{
    if (argc != 5) {
        std::cerr <<"Incorrect number of parameters provided. Exiting.\n"; 
	return returnErr(INCORRECT_NUMBER_OF_PARAMETERS); 
    }

    int q_size, job_no, prod_no, cons_no, sem_id; 
    parseArgs(q_size, job_no, prod_no, cons_no, argv); 

    //shared resource
    Buffer buffer(q_size); 

    //generate our set of 4 semophores
    sem_id = initSemophores(q_size); 
    if (sem_id == -1) {
        return returnErr(ERROR_INITIALISING_SEMOPHORE_ARRAY); 
    }
    
    //declaring array of required no. of posix threads for consumer
    //and producers
    pthread_t p_ids[prod_no];  
    pthread_t c_ids[cons_no];  

    //creating arrays of parameter structures to pass to threads
    threadParameters parameters[prod_no]; 
    threadParameters cparameters[cons_no]; 

    //create posix threads, passing producer function as task
    for (int i =0; i < prod_no; i++) {
        parameters[i].sem_id_ = sem_id; 
        parameters[i].thread_id_ = i+1; 
        parameters[i].job_no_ = job_no; 
        parameters[i].buffer_ = &buffer; 
        pthread_create(&p_ids[i], NULL, producer, (void*)&parameters[i]); 
    }

    //create posix threds, passing consumer function as task 
    for (int i =0; i < cons_no; i++) {
        cparameters[i].sem_id_ = sem_id; 
        cparameters[i].thread_id_ = i+1; 
        cparameters[i].buffer_ = &buffer; 
        pthread_create(&c_ids[i], NULL, consumer, (void*)&cparameters[i]); 
    }

    //wait producers to finish
    for (int i = 0; i < prod_no; i++) {
        pthread_join(p_ids[i], NULL); 
    }
    //wait consumers to finish 
    for (int i = 0; i < cons_no; i++) {
        pthread_join(c_ids[i], NULL); 
    }

    //close semophore array, freeing up the semohpore key for resuse
    sem_close(sem_id); 
    return 0;
}

//producer thread function.
void *producer (void *parameter) 
{
    auto parameters = (threadParameters*)parameter; 

    //extract variables from threadParameter struct
    //(just for clarity)
    int sem_id, jobs, job_id, thread_id, duration; 
    sem_id = parameters->sem_id_; 
    jobs = parameters->job_no_; 
    thread_id = parameters->thread_id_; 
    Buffer *buffer = parameters->buffer_; 

    //while there are jobs left to produce
    while (jobs--) {
	    
        duration = (rand() %10 + 1); 
	
	//if there is no space in buffer, wait
        sem_wait(sem_id, SPACE); 
	//if another thread is accessing buffer, wait
        sem_wait(sem_id, MUTEX); 
	//input a job into the buffer
        job_id = buffer->pushJob(duration); 

	//if another thread is accessing std::cout wait
        sem_wait(sem_id, OUTPUT); 
        std::cout <<"Producer(" << thread_id << "): job id: "
                  << job_id 
                  << " duration: " << duration << '\n' ;
	//signal output 
        sem_signal(sem_id, OUTPUT); 
	//signal buffer mutex
        sem_signal(sem_id, MUTEX); 
	//signal items semophore (increment value)
        sem_signal(sem_id, ITEMS); 
	//sleep for random duration between 1 and 5
        sleep(rand()%5+1); 
    }

    //once no more jobs left to run
     sem_wait(sem_id, OUTPUT); 
     std::cout <<"Producer(" << thread_id << "): has no more jobs "
	  << "to produce. Exiting." << '\n'; 
     sem_signal(sem_id, OUTPUT); 
     pthread_exit(0); 
}

//args better
void *consumer (void *parameter) 
{

    //cast and extract
    int duration, job_id;
    auto parameters = (threadParameters*)parameter; 
    int sem_id = parameters->sem_id_; 
    int thread_id = parameters->thread_id_; 
    Buffer *buffer = parameters->buffer_; 
    
    while (1) {

        //if there are no items in the buffer, wait
        //for max 20s then exit
      if (sem_wait_till_time(sem_id, ITEMS, 20)) {
	      break; 
      }

        //if another thread accessing buffer, wait
        sem_wait(sem_id, MUTEX); 
        //remove job from buffer (this frees up the job id to be reused)
        job_id = buffer->popJob(duration); 
	//if another thread printing to output, wait
        sem_wait(sem_id, OUTPUT); 
        std::cout << "Consumer(" << thread_id << "): job id: "
                  << job_id 
                  << " executing sleep duration: " << duration 
                  << '\n';
        sem_signal(sem_id, OUTPUT); 
        //signal that finished accssing the buffer
        sem_signal(sem_id, MUTEX); 
        //signal that there is an extra space in the buffer 
        sem_signal(sem_id, SPACE); 

	//perform job as specified in buffer 
        sleep(duration); 

	//report completion
        sem_wait(sem_id, OUTPUT); 
        std::cout <<"Consumer(" << thread_id << "): Job id: " << job_id
                  << " completed" << '\n' ;
        sem_signal(sem_id, OUTPUT); 
    }

    //once thread has timed out
    std::cout << "Consumer thread: " << thread_id << " has no more jobs left."
            << '\n'; 

    pthread_exit (0);

}
int initSemophores(int q_size) {
    int sem; 
    //create semophoore set + gen id
    sem = sem_create(SEM_KEY, 4);

    /*initialise our 4 semophores*/
    //mutex: ensure mutual exclusivity for buffer access. 
    sem_init(sem, MUTEX, 1);

    //check if buffer is not full
    sem_init(sem, SPACE, q_size);

    //check if items present in buffer 
    sem_init(sem, ITEMS, 0);

    //mutex to ensure that only one thread generatung an ID at a time
    //this just servs to ensure that the first threads that re created are the first threads 
    //performing their tasks. it does not affect how well the jobs are done, just makes the output
    //more accurately reflect the example
    sem_init(sem, ID, 1);

    
    sem_init(sem, OUTPUT, 1);

    return sem; 
}

//parse command line arguments, return error if digit not provided
int parseArgs(int &q, int &job_no, int&prod_no, int &cons_no, char** argv) {
    q = check_arg(argv[1]); 
    if (q == -1) {
	returnErr(INCORRECT_PARAMETER_TYPE, "First argument"); 
    }

    job_no = check_arg(argv[2]); 
    if (job_no == -1) {
	    returnErr(INCORRECT_PARAMETER_TYPE, "Second argument"); 
    }

    prod_no = check_arg(argv[3]); 
    if (prod_no == -1) {
	    returnErr(INCORRECT_PARAMETER_TYPE, "Third argument"); 
    }

    cons_no = check_arg(argv[4]);
    if (cons_no == -1)  {
	    returnErr(INCORRECT_PARAMETER_TYPE, "Fourth argument"); 
    }

    return NO_ERROR; 
}

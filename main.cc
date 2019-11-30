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



//pass pointer to this into producer parameters 
//ok so lets initialise queue_ to an array of queue_size containing
//only 0; then rahter than checking if null, just c



//-1 means nothing, 0 means 
//now i need something to keep track of the ids. hm
class Buffer {
    private: 
         int *queue_;
         int capacity;
         int size_; 
    
    public: 
         //returns the total number of elements in queue_
         int currentSize() {
             return size_; 
         }

         //push a job into the next available space on queue, returm the ID (index +1) of the job 
         //
         int pushJob(int duration) {
             for (int i = 0; i < duration; i++) {
                 if (queue_[i] == 0) {
                     queue_[i] = duration; 
                     size_++; 
                     return i + 1; 
                 }
             }
             return -4; 
         }

        //find a job in the queue that is not being accesed by another consumer, 
        //and start processing it. 
        int startJob(int &duration) {
            for (int i = 0; i < capacity; i++) {
                if (queue_[i] > 0) {
                    duration = queue_[i]; 
                    queue_[i] = -1; 
                    return i+1; 
                }
            }
            return -3; 
        }

        //delete job from queue array, freeing it up for future use
        void finishJob(int jobID) {
            jobID--; 
            queue_[jobID] = 0; 
            size_--; 
        }

        Buffer(int size) : capacity(size) {
            queue_ = new int[size]();
            size_ = 0; 
        }

        ~Buffer() {
            delete[] queue_;
        }
};



struct threadParameters {
    int sem_id_;
    int job_no_;   
    int thread_id_; 
    Buffer *buffer_;
    int producer_id; 
    threadParameters(int sem_id, int job_no, int thread_id, Buffer* buffer): sem_id_(sem_id),job_no_(job_no), thread_id_(thread_id), buffer_(buffer) {}
    threadParameters(): sem_id_(0),job_no_(0), buffer_(NULL) {}
};

int parseArgs(int &q, int &job_no, int&prod_no, int &cons_no, char** argv) {
    q = check_arg(argv[1]); 
    job_no = check_arg(argv[2]); 
    prod_no = check_arg(argv[3]); 
    cons_no = check_arg(argv[4]);
}


int main (int argc, char **argv)
{
    if (argc != 5) {
        std::cerr <<"Incorrect number of parameters provided. Exiting.\n"; 
        return -1; 
    }

    int q_size, job_no, prod_no, cons_no, sem_id; 
    parseArgs(q_size, job_no, prod_no, cons_no, argv); 

    //shared resource; 
    Buffer buffer(q_size); 

    //generate our set of 4 semophores
    sem_id = initSemophores(q_size); 
    if (sem_id == -1) {
        std::cerr << "Error initialising semohpores"; 
        return sem_id; 
    }
    
    threadParameters parameters[prod_no]; 
    threadParameters cparameters[cons_no]; 
    //declaring array of required no. of posix threads
    pthread_t p_ids[prod_no];  
    pthread_t c_ids[cons_no];  


    //producers
    for (int i =0; i < prod_no; i++) {
        parameters[i].sem_id_ = sem_id; 
        parameters[i].thread_id_ = i+1; 
        parameters[i].job_no_ = job_no; 
        parameters[i].buffer_ = &buffer; 
        pthread_create(&p_ids[i], NULL, producer, (void*)&parameters[i]); 
    }

    //consumers
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

    sem_close(sem_id); 
    return 0;
}

//producer thread function.
void *producer (void *parameter) 
{
    auto parameters = (threadParameters*)parameter; 

    int sem_id, size, jobs, job_id, thread_id, duration; 
    sem_id = parameters->sem_id_; 
    jobs = parameters->job_no_; 
    thread_id = parameters->thread_id_; 

    Buffer *buffer = parameters->buffer_; 
    while (jobs--) {

        duration = rand() %10 + 1; 
        sem_wait(sem_id, SPACE); 

        sem_wait(sem_id, MUTEX); 
        job_id = buffer->pushJob(duration); 
        sem_signal(sem_id, MUTEX); 
        
        sem_signal(sem_id, ITEMS); 

        size = buffer->currentSize(); 

        std::cout <<"\tProducer(" << thread_id << "): job id: "
                  << job_id 
                  << " duration: " << duration << '\n' ;


        int pauseDuration = rand() %5 + 1; 
        sleep(pauseDuration); 
    }

    pthread_exit(0); 
}

//args better
void *consumer (void *parameter) 
{
    int duration, job_id;
    auto parameters = (threadParameters*)parameter; 
    int sem_id = parameters->sem_id_; 
    int thread_id = parameters->thread_id_; 
    Buffer *buffer = parameters->buffer_; 
    
    while (1) {

        //if there are no items in the buffer, wait
        //for max 20s
        if (sem_wait_till_time(sem_id, ITEMS, 20)) {
            break; 
        }
        
        //wait to access the buffer
        sem_wait(sem_id, MUTEX); 
        //take job from the buffer
        job_id = buffer->startJob(duration); 
        //signal that finished accssing the buffer
        sem_signal(sem_id, MUTEX); 
        //signal that there is an extra space in the buffer 

        std::cout << "Consumer(" << thread_id << "): job id: "
                  << job_id 
                  << " executing sleep duration: " << duration 
                  << '\n';

        sleep(duration); 

        std::cout <<"Consumer(" << thread_id << "): Job id: " << job_id
                    << " completed" << '\n' ;
        sem_wait(sem_id, MUTEX); 
        buffer->finishJob(job_id); 
        sem_signal(sem_id, MUTEX); 
        sem_signal(sem_id, SPACE); 
    }

    // TODO 

  std::cout << "Consumer thread: " << thread_id << " has no more jobs left to run. "
            << "Exiting.\n"; 

  pthread_exit (0);

}
int initSemophores(int q_size) {
    int sem; 
    //create semophoore set + gen id
    sem = sem_create(SEM_KEY, 4);

    /*initialise our 4 semophores*/
    //mutex: ensure mutual exclusivity for buffer access
    sem_init(sem, MUTEX, 1);

    //check if buffer is not full
    sem_init(sem, SPACE, q_size);

    //check if buffer is not empty (items present in it)
    sem_init(sem, ITEMS, 0);

    //mutex to ensure that only one thread generatung an ID at a time
    //(to prevent aberrant duplication of IDs)
    sem_init(sem, ID, 1);
    return sem; 
}


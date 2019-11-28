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
struct Buffer {
    int *queue_; 
    int capacity; 

    int currentSize() {
        int count = 0; 
        for (int i = 0; i < capacity; i++) {
            if (queue_[i]!= 0) {
                count++; 
            }
        }
        return count; 
    }
    //returns the ID. ID is just index+1; 
    int pushJob(int jobDuration) {
        for (int i = 0; i < capacity; i++) {
            if (queue_[i] == 0) {
                queue_[i] = jobDuration; 
                return i; 
            }
        }
        return 0; 
    }

    Buffer(int size) : capacity(size) {
        queue_ = new int[capacity](); 
    }

    ~Buffer() {
        delete[] queue_; 
    }
}; 

//rename this to params/args 
struct producerParameters {
    int sem_id_;
    int job_no_;
    Buffer *buffer_;
    int producer_id; 
    producerParameters(int sem_id, int q_size,int job_no,int* queue): sem_id_(sem_id), q_size_(q_size), job_no_(job_no), queue_(queue) {}
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

    //the shared resource
    int* buffer = new int[q_size]; 

    //generate our set of 4 semophores
    sem_id = initSemophores(q_size); 
    
    //declaring array of required no. of poxic threads
    pthread_t producers[prod_no];  

    producerParameters parameters(sem_id, q_size, job_no, buffer) ; 
    //for every space in producers, 
    //create a thread, pass the producer function as the start routine 
    //pass null as args just for now 
    /* pthread_create takes 4 args: a pthread, pthread attr pointer, 
     * pointer to start routine function, pointer to arguments*/
    for (int i =0; i < prod_no; i++) {
        pthread_create(&producers[i], NULL, producer, (void*)&parameters); 
    }




    return 0;
}

//producer thread tasl funciton 
void *producer (void *parameter) 
{
    auto parameters = (producerParameters*)parameter; 


    //wait until released by id semophore
    sem_wait(parameters->sem_id_, ID); 
    sem_signal(parameters->sem_id_, ID); 

    std::cout <<"\nIn producer function. the job number is: " 
              << parameters->job_no_ 
              << "and the producer id is: " 
              << id 
              <<"\n"; 

    //generate 'random' job duration 
    int duration; 
    duration = rand()%10+1; 

    //wait to access the buffer
    sem_wait(sem_id_, MUTEX); 
    paramers->buffer_->push(duration); 
    sem_signal(sem_id_, MUTEX); 
    
    sem_signal(sem_id_, EMPTY); 
    

    std::cout << "Producer of ID: " 
              << id 
              << " has added job of id: " 
              << job 
              << " with duration of: " 
              << duration 
              <<"\n"; 
  pthread_exit(0);
}

void *consumer (void *id) 
{
    // TODO 

  pthread_exit (0);

}
int initSemophores(int q_size) {

    //create semophoore set + gen id
    int sem = sem_create(SEM_KEY, 4);

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


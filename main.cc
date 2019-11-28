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
                return i + 1;
            }
        }
        return 0;
    }

    //puts value of job into @duration, returns the job ID
    //
    int popJob(int &duration) {
        for (int i = 0; i < capacity; i++) {
            if (queue_[i] != 0) {
                duration = queue_[i];
                queue_[i] = 0;
                return i+1;
            }
        }
    }

    Buffer(int size) : capacity(size) {
        queue_ = new int[size]();
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
    producerParameters(int sem_id, int job_no,Buffer* buffer): sem_id_(sem_id),job_no_(job_no), buffer_(buffer) {}
    producerParameters(): sem_id_(0),job_no_(0), buffer_(NULL) {}
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
    

    std::cout <<"\n Key: " << SEM_KEY <<"\n"; 

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
    
    producerParameters parameters[prod_no]; 
    //declaring array of required no. of posix threads
    pthread_t p_ids[prod_no];  


    std::cout <<"Testing"; 
    std::cout <<"id, perprod, bsize: " << sem_id << ", " 
                << job_no << ", " << q_size <<"\n"; 
    //for every space in producers, 
    //create a thread, pass the producer function as the start routine 
    //pass null as args just for now 
    /* pthread_create takes 4 args: a pthread, pthread attr pointer, 
     * pointer to start routine function, pointer to arguments*/
    for (int i =0; i < prod_no; i++) {
        parameters[i].sem_id_ = sem_id; 
        //same for every producer
        parameters[i].job_no_ = job_no; 
        parameters[i].buffer_ = &buffer; 
        pthread_create(&p_ids[i], NULL, producer, (void*)&parameters[i]); 
    }

    for (int i = 0; i < prod_no; i++) {
        pthread_join(p_ids[i], NULL); 
    }

    sem_close(sem_id); 
    return 0;
}

//producer thread function.
void *producer (void *parameter) 
{
    std::cout << "in strt"; 
    auto parameters = (producerParameters*)parameter; 

    int sem_id, size, jobs, job_id, duration; 
    sem_id = parameters->sem_id_; 
    jobs = parameters->job_no_; 
    Buffer *buffer = parameters->buffer_; 

    std::cout << "\nThread is executing with params: " 
                << sem_id << ", " << jobs
                << ", with current buffer size being: " 
                << buffer->currentSize() 
                << "\n"; 


    duration = rand() %10 + 1; 

    sem_wait(sem_id, MUTEX); 
    job_id = buffer->pushJob(duration); 
    sem_signal(sem_id, MUTEX); 
    size = buffer->currentSize(); 

    std::cout << "Producer thread has added job of id: "
        << job_id << " and duration: " << duration << " to the buffer, resulting in a buffer size of: "
        << size << "\n"; 

    pthread_exit(0); 
}

void *consumer (void *id) 
{
    // TODO 

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


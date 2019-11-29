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
         int capacity, front, rear;
    
    public: 
         int currentSize() {
             return std::abs(std::abs(capacity - front+1) - std::abs(capacity - rear-1));
         }
         //returns the ID. ID is just index+1;
         int pushJob(int duration) {

             if ((front == 0 && rear == capacity-1) || (rear +1 == front)) {
                 std::cerr <<"Something has gone wrong! Trying to add to "
                     << " the buffer when its full. ";
                 return -1;
             }

             if (rear == capacity-1) {
                 rear = 0;
             } else {
                 rear ++;
                 queue_[rear] = duration;
             }
             if (front == -1) {
                 front = 0;
             }
             //"id"
             return rear +1;
         }

        int popJob(int &duration) {

            if (front == -1) {
                std::cerr << "Something went wrong! attempting to take "
                    << "from empty buffer. ";
                return -1;
            }

            int k = queue_[front];
            if (front == rear) {
                front = -1;
                rear = -1;
            } else {
                if (front == capacity -1) {
                    front = 0;
                } else {
                    front++;
                }
            }
            duration = k;
            return front;
        }

        void finishJob(int jobID) {
            
        }

        Buffer(int size) : capacity(size) {
            queue_ = new int[size]();
            front = rear = -1;
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

        std::cout <<"Producer(" << thread_id << "): job id: "
                  << job_id 
                  << "duration: " << duration << '\n' ;

        sleep(5); 
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
        job_id = buffer->popJob(duration); 
        //signal that finished accssing the buffer
        sem_signal(sem_id, MUTEX); 
        //signal that there is an extra space in the buffer 
        sem_signal(sem_id, SPACE); 

        std::cout << "Consumer(" << thread_id << "): job id: "
                  << job_id 
                  << " executing sleep duration: " << duration 
                  << '\n';

        sleep(duration); 

        std::cout <<"Consumer(" << thread_id << "): Job id: " << job_id
                    << " completed" << '\n' ;
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


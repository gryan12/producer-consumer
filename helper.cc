/******************************************************************
 * The helper file that contains the following helper functions:
 * check_arg - Checks if command line input is a number and returns it
 * sem_create - Create number of sempahores required in a semaphore array
 * sem_init - Initialise particular semaphore in semaphore array
 * sem_wait - Waits on a semaphore (akin to down ()) in the semaphore array
 * sem_signal - Signals a semaphore (akin to up ()) in the semaphore array
 * sem_close - Destroy the semaphore array
 ******************************************************************/

# include "helper.h"

/* output message to std::cerr, return @errorcode */ 
int returnErr(int errorCode, const std::string &message) {
	Errors error = (Errors)errorCode; 
	std::cerr << errorMessages.at(error) << message << " Exiting.\n";
	return errorCode; 
}

int check_arg (char *buffer)
{
  int i, num = 0, temp = 0;
  if (strlen (buffer) == 0)
    return -1;
  for (i=0; i < (int) strlen (buffer); i++)
  {
    temp = 0 + buffer[i];
    if (temp > 57 || temp < 48)
      return -1;
    num += pow (10, strlen (buffer)-i-1) * (buffer[i] - 48);
  }
  return num;
}

//timed version of sem_wait function, one @time number of seconds have 
//passed function will return -1 
int sem_wait_till_time (int id, short unsigned int num, int time)
{
  struct sembuf op[] = {
    {num, -1, SEM_UNDO}
  };
  struct timespec timeout = {time, 0}; 
  int res = semtimedop (id, op, 1, &timeout);
  return res;
}

int sem_create (key_t key, int num)
{
  int id;
  if ((id = semget (key, num,  0666 | IPC_CREAT | IPC_EXCL)) < 0)
    return -1;
  return id;
}

int sem_init (int id, int num, int value)
{
  union semun semctl_arg;
  semctl_arg.val = value;
  if (semctl (id, num, SETVAL, semctl_arg) < 0) {
	  return -1; 
  }
  return 0;
}

void sem_wait (int id, short unsigned int num)
{
  struct sembuf op[] = {
    {num, -1, SEM_UNDO}
  };
  semop (id, op, 1);
}

void sem_signal (int id, short unsigned int num)
{
  struct sembuf op[] = {
    {num, 1, SEM_UNDO}
  };
  semop (id, op, 1);
}

int sem_close (int id)
{
  if (semctl (id, 0, IPC_RMID, 0) < 0)
    return -1;
  return 0;
}


/* START Buffer members */ 

/*adds @duration to the back of the queue,
 * returns the job ID associated with this duration
 * (its index +1) */
int Buffer::pushJob(int duration) {
    //circle, if rear at end move to start
    if(rear == capacity-1) {
	rear = 0;
    } else {
	rear++;
    }
    queue[rear] = duration;

	if (front ==  -1) {
		front = 0;
	} 
	else if (empty) {
		front = rear; 
	}

	if (empty) {
		empty = false; 
	}

    return rear+1;

}

/* returns job ID, value of element at front of queue put
 * into @duration */ 
int Buffer::popJob(int &duration) {

    int temp, rval;
    temp = queue[front];
    //if there is only 1 element in the queue
    if (front == rear) {
	rval = front + 1;
	empty = true; 
    } else {
	if (front == capacity - 1) {
	    front = 0;
	    rval = capacity;
	} else {
	    front++;
	    rval = front;
	}
    }
    duration = temp;
    return rval;
}

//allocate queue on heap
Buffer::Buffer(int max) : capacity(max) {
	queue = new int[max]; 
	front = rear = -1; 
}

//deallocate queue
Buffer::~Buffer() {
	delete[] queue; 
}

/* END Buffer members */ 

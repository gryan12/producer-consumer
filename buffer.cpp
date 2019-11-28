#include <iostream> 
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
        queue_ = new int[capacity]();
    }

    ~Buffer() {
        delete[] queue_;
    }
};



int main() {
    int one,two,three; 
    Buffer buffer(15); 
    one = buffer.pushJob(12); 
    two = buffer.pushJob(17); 
    three = buffer.pushJob(6); 

    std::cout << "Inputting jobs: " 
        <<one 
        <<", " << two << ", " << three << "\n"; 

    int four, five, six; 
    int fourDur, fiveDur, sixDur; 
    four = buffer.popJob(fourDur); 
    five = buffer.popJob(fiveDur); 
    six = buffer.popJob(sixDur); 

    std::cout << "Taking jobs of ids "
        << four << ", " << five << ", " << six 
        << " with duration of: " 
        << fourDur << ", " << fiveDur << ", " << sixDur << "\n"; 

   
}


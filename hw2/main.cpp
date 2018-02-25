#include <iostream>
#include <math.h>
#include <fstream>
#include <sstream>
#include <chrono>
#include <vector>
#include <tuple>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;


int main(int argc, char* argv[1]) {

    using namespace std::chrono;

    //Start timer
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    if (argc != 2){
        cout<<"usage: "<< argv[0] <<" <filename>\n";
    } else {

        ifstream data(argv[1]);



        //Make sure the file opens correctly
        if (!data.is_open()) std::cout << "Error on: File Open" << '\n';

        std::string str;

        //declare multimap containing argmax and it's corresponding (x,y) pair.
        std::unordered_multimap<int, std::pair<double, double>> arg_mm;


        double x;
        double y;
        double lon;
        double lat;
        double delta_x;
        double delta_y;
        double feature;
        std::vector<double> features;
        int argMax;

        double lines_count = 0;

        while (data.good()) {
//    for (int i = 0; i<10; i++){

            //clears the temporary feature vector at the start of each loop so that each line's features can be added to the map
            //as one vector and the other stuff isn't included from the previous iteration.
            features.clear();
            argMax = 0;

            //grab longitude from file
            std::getline(data, str, ':');
            lon = atof(str.c_str()); // convert string to double

            //get latitude
            std::getline(data, str, ':');
            lat = atof(str.c_str());

            //get delta X
            std::getline(data, str, ':');
            delta_x = atof(str.c_str());

            //get delta Y
            std::getline(data, str, ':');
            delta_y = atof(str.c_str());

            //loop through the feature columns 0-19
            for (int j = 0; j < 20; j++) {
                std::getline(data, str, ':');
                feature = atof(str.c_str());
                features.push_back(feature);
            }

            //handles the '\n' on the last feature column
            std::getline(data, str, '\n');
            feature = atof(str.c_str());
            features.push_back(feature);

            //increment line counter
            lines_count++;

            //gets argmax for each feature vector
            argMax = std::distance(features.begin(), std::max_element(features.begin(), features.end()));

            //calculate X and Y
            x = (delta_x / 221200) * cos(lat) + lon;
            y = (delta_y / 221200) + lat;

            //make x and y into a pair
            std::pair<double, double> xyPair = make_pair(x, y);

            //add argMax and it's corresponding x,y pairs to multimap
            arg_mm.insert(make_pair(argMax, xyPair));
        }
        //Close file
        data.close();

        //Record parse finish time
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        //calculate time to parse
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);



        std::cout << "Time to Parse: " << time_span.count() << " seconds." << "\n";
        std::cout << "Number of Lines Parsed: " + std::to_string(lines_count) << "\n";
        std::cout << "Number of Buckets in Multimap: " + std::to_string(arg_mm.bucket_count()) << "\n";

        //Start time for getting the count of buckets
        high_resolution_clock::time_point bucket_count_start = high_resolution_clock::now();



        pid_t pid;			// process identifier, pid_t is a process id type defined in sys/types

        // =============================
        // BEGIN: Do the Semaphore Setup
        // The semaphore will be a mutex
        // =============================
        int semId; 			// ID of semaphore set
        key_t semKey = 641344; 		// key to pass to semget(), key_t is an IPC key type defined in sys/types
        int semFlag = IPC_CREAT | 0666; // Flag to create with rw permissions

        int semCount = 1; 		// number of semaphores to pass to semget()
        int numOps = 1; 		// number of operations to do
        
        // Create the semaphore
        // The return value is the semaphore set identifier
        // The flag is a or'd values of CREATE and ugo permission of RW, 
        //			just like used for creating a file
        if ((semId = semget(semKey, semCount, semFlag)) == -1)
        {
            std::cerr << "Failed to semget(" << semKey << "," << semCount << "," << semFlag << ")" << std::endl;
            exit(1);
        }
        else
        {
            // std::cout << "Successful semget resulted in (" << semId << std::endl;
        }

        // Initialize the semaphore
        union semun {
            int val;
            struct semid_ds *buf;
            ushort * array;
        } argument;

        argument.val = 1; // NOTE: We are setting this to one to make it a MUTEX
        if( semctl(semId, 0, SETVAL, argument) < 0)
        {
            std::cerr << "Init: Failed to initialize (" << semId << ")" << std::endl; 
            exit(1);
        }
        else
        {
            // std::cout << "Init: Initialized (" << semId << ")" << std::endl; 
        }

        // =============================
        // END: Do the Semaphore Setup
        // =============================

        // =========================================
        // BEGIN: Do the Shared Memory Segment Setup
        // 
        // =========================================

        int shmId; 			// ID of shared memory segment
        key_t shmKey = 123461; 		// key to pass to shmget(), key_t is an IPC key type defined in sys/types
        int shmFlag = IPC_CREAT | 0666; // Flag to create with rw permissions

        // This will be shared:
        const unsigned int targetBucketNum = 20;
        unsigned long * shm;
        unsigned long * sharedBucketPtr = NULL;

        if ((shmId = shmget(shmKey, (targetBucketNum+2) * sizeof(unsigned long), shmFlag)) < 0)
	    {
		std::cerr << "Init: Failed to initialize shared memory (" << shmId << ")" << std::endl; 
		exit(1);
	    }

        if ((shm = (unsigned long *)shmat(shmId, NULL, 0)) == (unsigned long *) -1)
	    {
		std::cerr << "Init: Failed to attach shared memory (" << shmId << ")" << std::endl; 
		exit(1);
	    }

        // =========================================
        // END: Do the Shared Memory Segment Setup
        // =========================================

        // Initialize bucket index count stuff
        sharedBucketPtr = &shm[targetBucketNum+1];
        *sharedBucketPtr = 0;
        unsigned long lastIndexProcessed = 0; //use this variable to track what a process just did.

        pid = fork();	// fork, which replicates the process 
	

        if ( pid < 0 )
        { 
            std::cerr << "Could not fork!!! ("<< pid <<")" << std::endl;
            exit(1);
        }
        
        // std::cout << "I just forked without error, I see ("<< pid <<")" << std::endl;
        
        if ( pid == 0 ) // Child process 
        {
            // std::cout << "In the child (if): " << std::endl; 
            
            while (lastIndexProcessed < targetBucketNum)
            {
                // Get the semaphore (P-op)
                struct sembuf operations[1];
                operations[0].sem_num = 0; 	// use the first(only, because of semCount above) semaphore
                operations[0].sem_flg = 0;	// allow the calling process to block and wait

                // Set up the sembuf structure.
                operations[0].sem_op = -1; 	// this is the operation... the value is added to semaphore (a P-Op = -1)

                // std::cout << "In the child (if): about to blocking wait on semaphore" << std::endl; 
                int retval = semop(semId, operations, numOps);

                if(0 == retval)
                {
                    // Compute the next bucket count
                    const unsigned long index = *sharedBucketPtr;

                    // std::cout << "In the child (if): Successful P-operation on (" << semId << "), bucket index=" << index << std::endl; 

                    if (index <= targetBucketNum)
                    {
                        // std::cout << "In the child (if): computing bucket index (" << index << ")" ; 
                        shm[index] = arg_mm.count(index);
                        // std::cout << "as (" << shm[index] << ")" << std::endl; 
                    }
                    *sharedBucketPtr += 1;
                    // mark the last index processed
                    lastIndexProcessed = index;
                    //sleep(1);
                }
                else
                {
                    std::cerr << "In the child (if): Failed P-operation on (" << semId << ")" << std::endl; 
                    _exit(1);
                }

            
                // Release the semaphore (V-op)
                operations[0].sem_op = 1; 	// this the operation... the value is added to semaphore (a V-Op = 1)
            
                // std::cout << "In the child (if): about to release semaphore" << std::endl; 
                retval = semop(semId, operations, numOps);
                if(0 == retval)
                {
                    // std::cout << "In the child (if): Successful V-operation on (" << semId << ")" << std::endl; 
                }
                else
                {
                    // std::cerr << "In the child (if): Failed V-operation on (" << semId << ")" << std::endl; 
                }
            
            } // END of while we have not computed the full sequence
            
            _exit(0);
        } 
        else		// Parent Process
        {
            // std::cout << "In the parent (if-else): " << std::endl; 
            
            while (lastIndexProcessed <= targetBucketNum)
            {
                // Get the semaphore (P-op)
                struct sembuf operations[1];
                operations[0].sem_num = 0; 	// use the first(only, because of semCount above) semaphore
                operations[0].sem_flg = 0;	// allow the calling process to block and wait

                // Set up the sembuf structure.
                operations[0].sem_op = -1; 	// this is the operation... the value is added to semaphore (a P-Op = -1)

                // std::cout << "In the child (if): about to blocking wait on semaphore" << std::endl; 
                int retval = semop(semId, operations, numOps);

                if(0 == retval)
                {
                    // Compute the next bucket count
                    const unsigned long index = *sharedBucketPtr;
                    
                    // std::cout << "In the parent (if-else): Successful P-operation on (" << semId << "), index=" << index << std::endl; 

                    if (index <= targetBucketNum)
                    {
                        // std::cout << "In the parent (if-else): computing index (" << index << ")" ; 
                        shm[index] = arg_mm.count(index);
                        // std::cout << "as (" << shm[index] << ")" << std::endl; 
                    }
                    *sharedBucketPtr += 1;
                    // mark the last index processed
                    lastIndexProcessed = index;
                    //sleep(1);

                }
                else
                {
                    std::cerr << "In the parent (if-else): Failed P-operation on (" << semId << ")" << std::endl; 
                    _exit(1);
                }

            
                // Release the semaphore (V-op)
                operations[0].sem_op = 1; 	// this the operation... the value is added to semaphore (a V-Op = 1)
            
                // std::cout << "In the parent (if-else): about to release semaphore" << std::endl; 
                retval = semop(semId, operations, numOps);
                if(0 == retval)
                {
                    // std::cout << "In the parent (if-else): Successful V-operation on (" << semId << ")" << std::endl; 
                }
                else
                {
                    std::cerr << "In the parent (if-else): Failed V-operation on (" << semId << ")" << std::endl; 
                }
                
            } // END of while we have not computed the full sequence
        }

        //get the count of the buckets
        // for (auto& i: {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20}) {
        //     std::cout << i << ": " << arg_mm.count(i) << " entries.\n";
        // }


        // ============================== 
        // All this code is boiler-plate	
        // ============================== 

        // std::cout << "In the parent: " << std::endl; 

        int status;	// catch the status of the child

        do  // in reality, mulptiple signals or exit status could come from the child
        {

            pid_t w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
            if (w == -1)
            {
                std::cerr << "Error waiting for child process ("<< pid <<")" << std::endl;
                break;
            }
            
            if (WIFEXITED(status))
            {
                if (status > 0)
                {
                    std::cerr << "Child process ("<< pid <<") exited with non-zero status of " << WEXITSTATUS(status) << std::endl;
                    continue;
                }
                else
                {
                    std::cout << "Child process ("<< pid <<") exited with status of " << WEXITSTATUS(status) << std::endl;
                    continue;
                }
            }
            else if (WIFSIGNALED(status))
            {
                std::cout << "Child process ("<< pid <<") killed by signal (" << WTERMSIG(status) << ")" << std::endl;
                continue;			
            }
            else if (WIFSTOPPED(status))
            {
                std::cout << "Child process ("<< pid <<") stopped by signal (" << WSTOPSIG(status) << ")" << std::endl;
                continue;			
            }
            else if (WIFCONTINUED(status))
            {
                std::cout << "Child process ("<< pid <<") continued" << std::endl;
                continue;
            }
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));


        //bucket count end time
        high_resolution_clock::time_point bucket_count_end = high_resolution_clock::now();
        //calculate time to count buckets
        duration<double> count_time = duration_cast<duration<double>>(bucket_count_end - bucket_count_start);
        std::cout << "Bucket Count Time: " << count_time.count() << " seconds." << "\n";


        shmctl(shmId, IPC_RMID, NULL);
    }

    return 0;
}

// OPERATING SYSTEMS PROJECT
// 22k-4308 - Hamza Ismail
// 22k-4422 - Ahmed Raza
// 22k-4248 - Sheheryar Salman

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <queue>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
using namespace std;

unsigned int seed;

//structure to store the customer information that will be passed on to the worker using the queue.
typedef struct
{

    int customer_id;
    char name[15];
    char work[30];

} CustomerInfo;

//global veriables to control the code
int CUSTOMERNO = 3300;
int WAITSIZE = 200;
int WORKERNO = 4;

//synchronization veriable.
useconds_t processing_time = 38000;

//semaphore declaration.
sem_t customer_waiting_sem_C;
sem_t customer_waiting_sem_W;
sem_t worker_sem;

//mutex declaration for queue and filing
pthread_mutex_t queue_lock;
pthread_mutex_t file_lock;

//queue declaration
queue<CustomerInfo *> waiting_area;

//global filing pointer that will be shared between the customer threads.
FILE *input_file;

//choices that will be randomly assigned and their prices.
char choices[6][30] = {"IDCARD-RENEWAL\n", "BIOMETRIC-VERIFICATION\n", "INFORMATION-CHANGE\n", "MARRIAGE-REGISTRATION\n", "BIRTH-CERTIFICATE\n", "OVERSEAS-IDCARD-ISSUE\n"};
int price[6] = {200, 300, 150, 250, 300, 500};

//names that will be randomly assigned to customers.
char names[10][15] = {"Ayesha", "Fatima", "Zainab", "Hassan", "Ali", "Ahmed", "Sara", "Omar", "Mahmood", "Nadia"};

/* initailises the simulation requirements such as creating inputs for customers and makes bill folder to store bills and initializes
   semaphores and mutexes. */
void initialize()
{

    //buffer to store generated string.
    char info[60];
    //veriable to store mkdir error.
    int error;
    
    //storing seed.
    seed = time(NULL);

    //making folders to and settings its attributes to read, write and execute and sstoring error in error
    error = mkdir("inputs", 0777);
    //checking error
    if (error != 0)
    {

        if (errno != EEXIST)
        {

            fprintf(stdout, "Error occurred creating bills folder\n");
            exit(EXIT_FAILURE);

        }
        
    }
    error = mkdir("bills", 0777);
    if (error != 0)
    {

        if (errno != EEXIST)
        {

            fprintf(stdout, "Error occurred creating bills folder\n");
            exit(EXIT_FAILURE);

        }
        
    }

    //initializing mutexes and semaphores.
    if(pthread_mutex_init(&queue_lock, NULL) != 0)
    {

        fprintf(stdout, "error occured in mutex initialization\n");
        exit(EXIT_FAILURE);

    }
    if(pthread_mutex_init(&file_lock, NULL) != 0)
    {

        fprintf(stdout, "error occured in mutex initialization\n");
        exit(EXIT_FAILURE);  

    }
    if(sem_init(&worker_sem, 0, WORKERNO) != 0)
    {

        fprintf(stdout, "error occured in initializing worker_sem\n");
        exit(EXIT_FAILURE); 

    }
    if(sem_init(&customer_waiting_sem_C, 0, WAITSIZE) != 0)
    {

        fprintf(stdout, "error occured in initializing customer_waiting_sem_C\n");
        exit(EXIT_FAILURE); 

    }
    if(sem_init(&customer_waiting_sem_W, 0, 0) != 0) {

        fprintf(stdout, "error occured in initializing customer_waiting_sem_W\n");
        exit(EXIT_FAILURE); 

    }

    //creating input file to store the generated customer inputs in CSV format.
    input_file = fopen("inputs/customer data.csv", "w");
    if(input_file == NULL)
    {

        fprintf(stdout, "error creating and opeining input file\n");
        exit(EXIT_FAILURE); 

    }

    //generating and storing user inputs.
    for (int i = 0; i < CUSTOMERNO; i++)
    {

        fprintf(input_file, "%s,%s", names[rand_r(&seed) % 10], choices[rand_r(&seed) % 6]);

    }

    //closing input file.
    if(fclose(input_file) != 0)
    {

        fprintf(stdout, "failed closing input file\n");
        exit(EXIT_FAILURE);

    }

}

//worker thread.
void *worker(void *args)
{

    //assigning worker id to each worker
    int worker_id = *((int *)args);
    
    //CustomerInfo pointer to store the current customer that the worker is working for.
    CustomerInfo *customerinfo;

    //file pointer to make bill for each customer and billname buffer to store bill name.
    FILE *bill_file;
    char billname[40] = "", temp[3] = "";

    //worker is sleeping in the start.
    fprintf(stdout, "worker %d is sleeping\n", worker_id);
    usleep(500000);

    while (1)
    {

        //worker waits for customer_waiting_sem_W to post and stays in spin lock.
        sem_wait(&customer_waiting_sem_W);
        //once customer_waiting_sem_W decrements its releases a seat by incrementing customer_waiting_sem_C.
        sem_post(&customer_waiting_sem_C);

        //decrements worker_sem to indicate that a worker is busy.
        sem_wait(&worker_sem);

        //locking the queue so that no two threads can read or write to the symaltiously beacause queue size will be constantly changing.
        pthread_mutex_lock(&queue_lock); //critical section start.
        customerinfo = waiting_area.front();
        waiting_area.pop();
        pthread_mutex_unlock(&queue_lock); //critical section end.

        //billname stores the name of each customer bill.
        sprintf(billname, "bills/customer %d bill.txt", customerinfo->customer_id);
        //opening file for customer.
        bill_file = fopen(billname, "w");
        if(bill_file == NULL)
        {

            fprintf(stdout, "error creating and opeining bill file for customer %d\n", customerinfo->customer_id);
            exit(EXIT_FAILURE);

        }

        //worker printing message that he is working for this worker.
        fprintf(stdout, "worker %d is now working for customer %d\n", worker_id, customerinfo->customer_id);

        //writing customer name in customer bill.
        fprintf(bill_file, "customer name: %s\n", customerinfo->name);
        //sleep to show processing time.
        usleep(processing_time);
        //writing customer ID in customer bill.
        fprintf(bill_file, "customer ID: %d\n\n", customerinfo->customer_id);
        //sleep to show processing time.
        usleep(processing_time);
        //writing the service that customer wants in customer bill. 
        fprintf(bill_file, "serviced used: %s", customerinfo->work);
        //sleep to show processing time.
        usleep(processing_time);

        //writing service price to the customer bill.
        if (strcmp(customerinfo->work, choices[0]) == 0)
        {

            fprintf(bill_file, "service price: %d", price[0]);

        }
        else if (strcmp(customerinfo->work, choices[1]) == 0)
        {

            fprintf(bill_file, "service price: %d", price[1]);

        }
        else if (strcmp(customerinfo->work, choices[2]) == 0)
        {

            fprintf(bill_file, "service price: %d", price[2]);

        }
        else if (strcmp(customerinfo->work, choices[3]) == 0)
        {

            fprintf(bill_file, "service price: %d", price[3]);

        }
        else if (strcmp(customerinfo->work, choices[4]) == 0)
        {

            fprintf(bill_file, "service price: %d", price[4]);

        }
        else if (strcmp(customerinfo->work, choices[5]) == 0)
        {

            fprintf(bill_file, "service price: %d", price[5]);

        }

        //sleep to show processing time.
        usleep(processing_time);
        
        //closing the customer file
        if(fclose(bill_file) != 0)
        {

            fprintf(stdout, "failed closing input file\n");
            exit(EXIT_FAILURE);

        }

        //deleting the the CustomerInfo object to release memory.
        delete customerinfo;

        //posting the worker_sem to indicate that the worker is now free.
        sem_post(&worker_sem);

    }

}

//customer thread.
void *customer(void *args)
{

    //assigning the customer id to each customer.
    int customer_id = *((int *)args);
    int waiting_count, linecount = 0;
    //CustomerInfo object to to store details of each customer.
    CustomerInfo *customerinfo;
    //buffer to store customer inputs from shared file.
    char info[50], *extracted;

    //allocating dynamic memory to each customer object.
    customerinfo = new CustomerInfo(); 

    //sleep to simulate a random customer arrival time.
    sleep((rand_r(&seed) % 20));

    //locking the shared file so that no two customer read the file together.
    pthread_mutex_lock(&file_lock); //critical section start
    //each customer will read the file from start.
    if(fseek(input_file, 0, SEEK_SET) != 0)
    {

        fprintf(stdout, "customer %d failed to to fseek file\n", customer_id);
        exit(EXIT_FAILURE);

    }
    //storing file line into info.
    fgets(info, 50, input_file);
    //customer will read file lines untill the linecount is not equal to customer id. 
    while (linecount != customer_id)
    {

        linecount++;
        fgets(info, 50, input_file);

    }
    pthread_mutex_unlock(&file_lock); //critical section end.

    //assigning customer id to customer object.
    customerinfo->customer_id = customer_id;
    //using strtok to break string into token and extracting customer name
    extracted = strtok(info, ",");
    //assiging customer name to customer to customer object.
    strcat(customerinfo->name, extracted);
    //again using strtok to extract customer work
    extracted = strtok(NULL, ",");
    //assiging customer work to customer object.
    strcat(customerinfo->work, extracted);

    //getting customer_waiting_sem_C value and storing it in waiting_count.
    sem_getvalue(&customer_waiting_sem_C, &waiting_count);
    if (waiting_count == 0)
    {

        fprintf(stdout, "waiting area is full customer %d is leaving\n", customer_id);
        delete customerinfo; //deleting the customer from heap who is not waiting in line.

        //exiting customer to show that he is gone.
        pthread_exit(NULL);

    }
    else
    {
        //locking the queue so that no two threads can read or write to the symaltiously beacause queue size will be constantly changing.
        pthread_mutex_lock(&queue_lock); //critical section start.
        waiting_area.push(customerinfo);
        pthread_mutex_unlock(&queue_lock); //critical section end.

        fprintf(stdout, "customer %d is now waiting in waiting area\n", customer_id);
        sem_wait(&customer_waiting_sem_C); //decramenting customer_waiting_sem_C to indicate that one seat in waiting is taken.
        sem_post(&customer_waiting_sem_W); //incramenting customer_waiting_sem_W to tell worker that customer is waiting in the queue.

    }

    //customer waiting for worker to finish doing its job.
    usleep((processing_time * 4));

    fprintf(stdout, "customer %d has left nadra\n", customer_id);

    pthread_exit(NULL);

}

//menu function definition.
void menu() 
{

    char s; //variable to store selection.

    while(1) {

        system("clear");
        fprintf(stdout, "Welcome to Nadra Worker Problem\n");
        fprintf(stdout, "1.Start Simulation with current Settings\n");
        fprintf(stdout, "2.View Current Simulation Settings\n");
        fprintf(stdout, "3.Change Current Simulation Settings\n");
        fprintf(stdout, "4.Exit the Program\n");

        scanf(" %c", &s);

        //run simulation with current settings.
        if(s == '1')
        {

            break;

        }
        else if(s == '2') //displaying the current simulatuon settings.
        {

            system("clear");
            fprintf(stdout, "Number of Customers: %d\n", CUSTOMERNO);
            fprintf(stdout, "Size of Waiting Area: %d\n", WAITSIZE);
            fprintf(stdout, "Number of Workers Currently: %d\n", WORKERNO);
            fprintf(stdout, "Processing Time for each action: %u\n", processing_time);
            int y = getchar();

        }
        else if(s == '3') //menu for changing simulaiton settings.
        {

            char t; //store new choice

            system("clear");
            fprintf(stdout, "Number of Customers: %d\n", CUSTOMERNO);
            fprintf(stdout, "Size of Waiting Area: %d\n", WAITSIZE);
            fprintf(stdout, "Number of Workers Currently: %d\n", WORKERNO);
            fprintf(stdout, "Processing Time for each action: %u\n", processing_time);

            fprintf(stdout, "\n1.Change Number of Customers\n");
            fprintf(stdout, "2.Change the Size of the Waiting Area\n");
            fprintf(stdout, "3.Change the Number of Workers Currently\n");
            fprintf(stdout, "4.Change the processing time for each action\n");
            fprintf(stdout, "5.Return to previous menu\n");
            
            scanf(" %c", &t);

            //change number of customers
            if(t == '1')
            {

                int temp;

                system("clear");
                fprintf(stdout, "Enter the number of Customers you would like to set. Kindly choose between 1-10000: ");

                scanf("%d", &temp);

                //error checking condition
                if(temp >= 1 && temp <= 10000) 
                {

                    CUSTOMERNO = temp;

                }
                else
                {

                    fprintf(stdout, "Customer Number not in range\n");
                    fprintf(stdout, "Returning to Main Menu!!!\n");
                    int y = getchar();

                }

            }
            //changing waiting area size
            else if(t == '2') 
            {

                int temp;

                system("clear");
                fprintf(stdout, "Enter the Size of the Waiting Queue, Kindly make sure to Enter a Positive Number: ");

                scanf("%d", &temp);

                //error checking condition.
                if(temp > 0) 
                {
                    
                    WAITSIZE = temp;

                }
                else
                {

                    fprintf(stdout, "WaitSize not in valid Range. \n");
                    fprintf(stdout, "Returning to Main Menu!!!\n");
                    int y = getchar();

                }

            }
            //changing the number of workers
            else if(t == '3') 
            {

                int temp;

                system("clear");
                fprintf(stdout, "Enter the Number of Workers you would like in the simulation. Range: [1-10]: ");

                scanf("%d", &temp);
                
                //error checking condition
                if(temp > 0 && temp <= 10)
                {

                    WORKERNO = temp;

                }
                else 
                {
                    
                    fprintf(stdout, "Worker Numbers not in valid Range. \n");
                    fprintf(stdout, "Returning to Main Menu!!!\n");
                    int y = getchar();

                }

            }
            //changing the processing time
            else if(t == '4')
            {

                int temp;

                system("clear");
                fprintf(stdout, "Enter the Processing Time you would like in the simulation for each task. Range: [0-1000000] microseconds: ");

                scanf("%d", &temp);

                //error checking condition
                if(temp >= 0 && temp <= 1000000)
                {

                    processing_time = temp;

                }
                else 
                {

                    fprintf(stdout, "Processing Time is not in valid Range. \n");
                    fprintf(stdout, "Returning to Main Menu!!!\n");
                    int y = getchar();

                } 

            }
            //error message for selecting wrong input
            else
            {

                fprintf(stdout, "invalid option. Exiting to the Main Menu.\n");
                int y = getchar();

            }
            
        }
        //exit the program without running the simulation, else program will end after running similation
        else if(s == '4') 
        {

            fprintf(stdout, "Thankyou for using Nadra Worker Problem Implementation...\n");
            exit(0);

        }
        //error message for selecting wrong option.
        else
        {

            fprintf(stdout, "Wrong input detected, Kindly enter valid input\n");
            int y = getchar();
            
        }

        int x = getchar();

    }

}


int main()
{

    //calling the menu function.
    menu();

    //declaring id array and thread arrays and stop.
    int id[CUSTOMERNO + WORKERNO], stop;
    pthread_t worker_t[WORKERNO];
    pthread_t customer_t[CUSTOMERNO];

    //calling the initialize function to ready the simulation.
    initialize();

    //creating the worker threads
    for (int i = 0; i < WORKERNO; i++)
    {

        id[i] = i;

        if(pthread_create(&worker_t[i], NULL, worker, &id[i]) != 0)
        {

            fprintf(stdout, "failed to create thread for worker %d", i);
            exit(EXIT_FAILURE);

        }

    }

    //opening the inputs file so that it is shared between all the customer threads.
    input_file = fopen("inputs/customer data.csv", "r");
    if(input_file == NULL)
    {

        fprintf(stdout, "error creating and opeining input file for all customer threads\n");
        exit(EXIT_FAILURE); 

    }

    //creating the customer threads.
    for (int i = 0; i < CUSTOMERNO; i++)
    {

        id[i + WORKERNO] = i;

        if(pthread_create(&customer_t[i], NULL, customer, &id[i + WORKERNO]) != 0)
        {

            fprintf(stdout, "failed to create thread for customer %d\n", i);
            exit(EXIT_FAILURE);
            
        }

    }

    //waiting for the customer threads to join back to the main thread.
    for (int i = 0; i < CUSTOMERNO; i++)
    {

        if(pthread_join(customer_t[i], NULL) != 0)
        {

            fprintf(stdout, "error joining customer %d thread\n", i);
            exit(EXIT_FAILURE);

        }

    }

    //closing the inputs file.
    if(fclose(input_file) != 0)
    {

        fprintf(stdout, "failed to close file for all customers threads\n");
        exit(EXIT_FAILURE);

    }

    //loop to ensure customers in queue are also services before program finishes and then workers exit safely.
    while(1) {

        sem_getvalue(&customer_waiting_sem_W, &stop);
        if(stop == 0) {
            
            //to ensure that workers complete their work before terminating.
            sleep(1);

            for(int i = 0; i < WORKERNO; i++)
            {

                //terminating the worker threads.
                if(pthread_cancel(worker_t[i]) != 0)
                {

                    fprintf(stdout, "error canceling worker %d thread\n", i);
                    exit(EXIT_FAILURE);

                }
                else 
                {

                    fprintf(stdout, "worker %d is sleeping\n", i);

                }

            }

            break;

        }

    }

    //joining the worker threads.
    for (int i = 0; i < WORKERNO; i++)
    {

        if(pthread_join(worker_t[i], NULL) != 0)
        {

            fprintf(stdout, "error joining worker %d thread\n", i);
            exit(EXIT_FAILURE);

        }

    }

    //destroying all the mutexes and semaphores.
    if(pthread_mutex_destroy(&queue_lock) != 0)
    {

        fprintf(stdout, "failed to destory queue lock\n");
        exit(EXIT_FAILURE);

    }
    if(pthread_mutex_destroy(&file_lock) != 0)
    {

        fprintf(stdout, "failed to destory file lock\n");
        exit(EXIT_FAILURE);

    }
    if(sem_destroy(&worker_sem) != 0)
    {

        fprintf(stdout, "error occured in destroying worker_sem\n");
        exit(EXIT_FAILURE);

    }
    if(sem_destroy(&customer_waiting_sem_C) != 0)
    {

        fprintf(stdout, "error occured in initializing customer_waiting_sem_C\n");
        exit(EXIT_FAILURE);

    }
    if(sem_destroy(&customer_waiting_sem_W) != 0)
    {

        fprintf(stdout, "error occured in initializing customer_waiting_sem_W\n");
        exit(EXIT_FAILURE);

    }

    return 0;
    
}

#include <stdio.h>
#include <stdlib.h>

#include "defs.h"
#include "hash.h"
#include <pthread.h>    //import POSIX thread library
#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Team Name",                           /* Team name */
    "Everard Francis",                     /* First member full name */
    "999999999",                           /* First member student number */
    "everard.francis@mail.utoronto.ca",    /* First member email address */

    "Calvin Fernandes",                    /* Second member full name */
    "997588873",                           /* Second member student number */
    "calvin.fernandes@mail.utoronto.ca"    /* Second member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample 
{
    unsigned my_key;
    public:
    sample *next;
    unsigned count;

    sample(unsigned the_key){my_key = the_key; count = 0;};
    unsigned key(){return my_key;}
    void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample,unsigned> h;


/*
 *  Struct for passing information to threads
 */
struct thread_args
{
    int start;
    int end;
};

//mutex lock for data preservation
pthread_mutex_t mutex_h;    //mutex lock for hash table access

/*
 * Sample Collection function
 */
void* sampleCollection(void* input_params)
{
    struct thread_args* grabInput = (struct thread_args*) input_params; //cast properly then assign
    int i,j,k;
    int rnum;
    unsigned key;
    sample *s;

    for (i=(grabInput->start); i<(grabInput->end); i++)
    {
        rnum = i;
   
        // collect a number of samples
        for (j=0; j<SAMPLES_TO_COLLECT; j++)
        {
            // skip a number of samples
            for (k=0; k<samples_to_skip; k++)
            {
                rnum = rand_r((unsigned int*)&rnum);
            }
          
            // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
            key = rnum % RAND_NUM_UPPER_BOUND;
            //TM LOCK
            __transaction_atomic
            {
                // if this sample has not been counted before
                if (!(s = h.lookup(key)))   //this part of the code must be serialized - it would be nice to serialize lookups per individual bucket
                {

                    // insert a new element for it into the hash table
                    s = new sample(key);
                    h.insert(s);    //we may need to serialize this as well - we can put a big lock on this section
                }

                // increment the count for the sample
                s->count++;
            }
        }
    }
    pthread_exit(NULL);
}



int  
main (int argc, char* argv[])
{
    // Print out team information
    printf( "Team Name: %s\n", team.team );
    printf( "\n" );
    printf( "Student 1 Name: %s\n", team.name1 );
    printf( "Student 1 Student Number: %s\n", team.number1 );
    printf( "Student 1 Email: %s\n", team.email1 );
    printf( "\n" );
    printf( "Student 2 Name: %s\n", team.name2 );
    printf( "Student 2 Student Number: %s\n", team.number2 );
    printf( "Student 2 Email: %s\n", team.email2 );
    printf( "\n" );

    // Parse program arguments
    if (argc != 3)
    {
        printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
        exit(1);  
    }
    sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
    sscanf(argv[2], " %d", &samples_to_skip);

    // initialize a 16K-entry (2**14) hash of empty lists
    h.setup(14);

    // initialize mutual exclusion lock for h, the hash table
    pthread_mutex_init(&mutex_h, NULL);

    switch(num_threads)
    {
        case 1:
        {
            //1 thread scenario
            pthread_t tid1;
            struct thread_args t1_args;
            t1_args.start = 0;
            t1_args.end = NUM_SEED_STREAMS;
            pthread_create(&tid1, NULL, &sampleCollection, &t1_args);

            pthread_join(tid1, NULL);  //we are not interested in the function returns
            break;
        }

        case 2:
        {
            //2 thread scenario
            pthread_t tid[2];
            struct thread_args t1_args;
            struct thread_args t2_args;
            
            //set up thread 1 slice computation
            t1_args.start   = 0;
            t1_args.end     = 2;

            //set up thread 2 slice computation
            t2_args.start   = 2;   //continue from where last thread left off 
            t2_args.end     = 4;

            //start thread-work
            pthread_create(&tid[0], NULL, &sampleCollection, &t1_args);
            pthread_create(&tid[1], NULL, &sampleCollection, &t2_args);

            //join threads
            pthread_join(tid[0], NULL);  //we are not interested in the function returns
            pthread_join(tid[1], NULL);
            break;
        }

        case 4:
        {
            //2 thread scenario
            pthread_t tid[4];
            struct thread_args t1_args;
            struct thread_args t2_args;
            struct thread_args t3_args;
            struct thread_args t4_args;

            //set up thread 1 slice computation
            t1_args.start   = 0;
            t1_args.end     = 1;

            //set up thread 2 slice computation
            t2_args.start   = 1;   
            t2_args.end     = 2;

            //set up thread 3 slice computation
            t3_args.start   = 2;   
            t3_args.end     = 3;

            //set up thread 4 slice computation
            t4_args.start   = 3;   
            t4_args.end     = 4;

            //start thread-work
            pthread_create(&tid[0], NULL, &sampleCollection, &t1_args);
            pthread_create(&tid[1], NULL, &sampleCollection, &t2_args);
            pthread_create(&tid[2], NULL, &sampleCollection, &t3_args);
            pthread_create(&tid[3], NULL, &sampleCollection, &t4_args);

            //join threads
            pthread_join(tid[0], NULL);
            pthread_join(tid[1], NULL);
            pthread_join(tid[2], NULL);
            pthread_join(tid[3], NULL);
            break;
        }
    }
    // print a list of the frequency of all samples
    h.print();

    //destroy mutex
    pthread_mutex_destroy(&mutex_h);
    
    //exit program
    pthread_exit(NULL);
}

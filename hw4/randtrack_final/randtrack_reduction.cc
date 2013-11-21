
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
team_t team = 
{
    "Team Name",                           /* Team name */
    "Everard Francis",                     /* First member full name */
    "997412811",                           /* First member student number */
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

//create multiple hash tables, each thread gets their own
hash<sample,unsigned> h;
hash<sample,unsigned> i;
hash<sample,unsigned> j;
hash<sample,unsigned> k;
hash<sample,unsigned> master;

/*
 *  Struct for passing information to threads
 */
struct thread_args
{
    int start;
    int end;
    hash<sample,unsigned> * input_hash_table;
};

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


            // if this sample has not been counted before
            if (!(s = (grabInput->input_hash_table)->lookup(key)))   //this part of the code must be serialized - it would be nice to serialize lookups per individual bucket
            {

                // insert a new element for it into the hash table
                s = new sample(key);
                (grabInput->input_hash_table)->insert(s);    //we may need to serialize this as well - we can put a big lock on this section
            }
            // increment the count for the sample
            s->count++;
       }
    }
    pthread_exit(NULL);
}

//custom testing
void print_hash_table(hash<sample,unsigned> *ptr_ht)
{
    unsigned i;
    sample *s;
    for (i = 0; i < RAND_NUM_UPPER_BOUND ; i++)
    {
        if ((s = ptr_ht->lookup(i)))
            printf("%d %d\n", i, s->count); 
    }
}
//custom transfer
void transfer_results(hash<sample,unsigned> *ptr_dest, hash<sample,unsigned> *ptr_src)
{
    unsigned i;
    sample *s;
    sample *d;
    for (i = 0; i < RAND_NUM_UPPER_BOUND; i++)
    {
        //check if number exists in the source
        if ( (s = ptr_src->lookup(i)) )
        {
            //check if the number exists in the destination - if not then insert it
            if ( !(d = ptr_dest->lookup(i)) )
            {
                d = new sample(i);      //create a new sample of this random number
                ptr_dest->insert(d);    //insert number into hash table
            }
            d->count += s->count;       //include all the count from the src hash table and put it in the destination hash table
        }
    }
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
    i.setup(14);
    j.setup(14);
    k.setup(14);
    master.setup(14);

    switch(num_threads)
    {
        case 1:
        {
            //1 thread scenario
            pthread_t tid1;
            struct thread_args t1_args;
            t1_args.start = 0;
            t1_args.end = NUM_SEED_STREAMS;
            t1_args.input_hash_table = &h;
            pthread_create(&tid1, NULL, &sampleCollection, &t1_args);
            pthread_join(tid1, NULL);  //we are not interested in the function returns
            
            transfer_results(&master, &h);
            print_hash_table(&master);
	        //h.print();
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
            t1_args.input_hash_table = &h;

            //set up thread 2 slice computation
            t2_args.start   = 2;   //continue from where last thread left off 
            t2_args.end     = 4;
            t2_args.input_hash_table = &i;

            //start thread-work
            pthread_create(&tid[0], NULL, &sampleCollection, &t1_args);
            pthread_create(&tid[1], NULL, &sampleCollection, &t2_args);

            //join threads
            pthread_join(tid[0], NULL);  //we are not interested in the function returns
            pthread_join(tid[1], NULL);

            //synchronize information
            transfer_results(&master, &h);
            transfer_results(&master, &i);
            print_hash_table(&master);

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
            t1_args.input_hash_table = &h;

            //set up thread 2 slice computation
            t2_args.start   = 1;   
            t2_args.end     = 2;
            t2_args.input_hash_table = &i;

            //set up thread 3 slice computation
            t3_args.start   = 2;   
            t3_args.end     = 3;
            t3_args.input_hash_table = &j;

            //set up thread 4 slice computation
            t4_args.start   = 3;   
            t4_args.end     = 4;
            t4_args.input_hash_table = &k;

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

            //synchronize information
            transfer_results(&master, &h);
            transfer_results(&master, &i);
            transfer_results(&master, &j);
            transfer_results(&master, &k);
            print_hash_table(&master);

            break;
        }
    }
    //exit program
    pthread_exit(NULL);
}

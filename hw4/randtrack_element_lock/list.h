
#ifndef LIST_H
#define LIST_H

#include <pthread.h>
#include <stdio.h>

//declare mutex locks for the element
pthread_mutex_t element_lock[100000];

template<class Ele, class Keytype> class list;

template<class Ele, class Keytype> class list 
{
 private:
  Ele *my_head;
  unsigned long long my_num_ele;
  pthread_mutex_t list_lock;
  
 public:
  list()
  {
    my_head = NULL;
    my_num_ele = 0;
  }

  void setup();

  unsigned num_ele(){return my_num_ele;}

  Ele *head(){ return my_head; }
  Ele *lookup(Keytype the_key);

  void push(Ele *e);
  Ele *pop();
  void print(FILE *f=stdout);

  void cleanup();
};

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::setup()
{
  my_head = NULL;
  my_num_ele = 0;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::push(Ele *e)	//this is the insert function - messing with head means we need to lock for the entire duration
{
  pthread_mutex_lock  (&list_lock);  
  e->next = my_head;
  my_head = e;
  my_num_ele++;
  pthread_mutex_unlock(&list_lock);
}

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::pop()
{
  Ele *e;
  e = my_head;
  if (e){
    my_head = e->next;
    my_num_ele--;
  }
  return e;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::print(FILE *f)
{
  Ele *e_tmp = my_head;

  while (e_tmp){
    e_tmp->print(f);
    e_tmp = e_tmp->next;
  }
}

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::lookup(Keytype the_key)
{
  //need to make sure we have the list lock when we obtain this
  pthread_mutex_lock  (&list_lock);
  Ele *e_tmp = my_head;
  pthread_mutex_unlock(&list_lock);
  
  while (e_tmp)
  {
	
	//element lock here
	if (e_tmp->key() == the_key)	//modified for easy wrapping of locks
		break;
    e_tmp = e_tmp->next;
	//element unlock here for safety - we are done accessing the sample node here
  }
  return e_tmp;
}

template<class Ele, class Keytype> 
void
list<Ele,Keytype>::cleanup()
{
  Ele *e_tmp = my_head;
  Ele *e_next;

  while (e_tmp){
    e_next = e_tmp->next;
    delete e_tmp;
    e_tmp = e_next;
  }
  my_head = NULL;
  my_num_ele = 0;
}

#endif

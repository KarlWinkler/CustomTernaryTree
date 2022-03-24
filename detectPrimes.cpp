// detectPrimes.cpp
// Author: Karl Winkler

// Written for CPSC 457 - operating systems
// THis program will find all prime factors for a given list of numbers
// and is run using the provided main class

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <vector>
#include <pthread.h>
#include <iostream>

// returns true if n is prime, otherwise returns false

std::atomic<int> has_factor(0);
//volatile int has_factor = 0;
pthread_barrier_t barr_id;
std::vector<int64_t> result;
std::vector<int64_t> min_array;
std::vector<int64_t> num_array;
int64_t count;
int64_t interval; 

static void is_prime(int64_t n, int64_t min, int64_t max){

  // find factors by testing from min to max incrementing by 6
  int64_t i = min;
  while (i <= max) {
    if(has_factor == 1) { return ; }
    if (i > 0 && n % i == 0) {has_factor = 1 ; return ; }
    if (n % (i + 2) == 0) {has_factor = 1 ; return ; }
    i += 6;
 }

  // didn't find any divisors, so it must be a prime
  has_factor = 0;
  return;
}

int quick_check_is_prime(int64_t num){

  if (num < 2) {return 1;} // 0 and 1 are not prime
  if (num <= 3) {return 0;} // 2 and 3 are primes
  if (num % 2 == 0) {return 1;} // handle multiples of 2
  if (num % 3 == 0) {return 1;} // handle multiples of 3
  if (num < 25) {return 0;} // if it is not dividable by 2 or 3 and is less than 25 then it is prime
  return -1;	
}


void * run(void * thread_mem){

  long index = (long)thread_mem;

  for (auto num : num_array) {
    has_factor = 0; // reset has_factor 
    int res = (pthread_barrier_wait(& barr_id)); // wait for every thread so that we can start serial work
    if(res == PTHREAD_BARRIER_SERIAL_THREAD){ // start serial work
      // serial setup for parallel work
      min_array.clear();

      has_factor = quick_check_is_prime(num);
      if(has_factor == 0){result.push_back(num);} // number is prime, add it to the results

      // try to divide n by every number 5 .. sqrt(n)
      int64_t max = sqrt(num);
            
      int i = 0; // make an array of all min values needed so that threads know where to start
      while(i < count){
	int64_t val = (max/count) * i;
	if(val == 0){min_array.push_back(5); i++; continue;}
        min_array.push_back(val - ((val - 5) % 6) + 6); 
	i++;
      }

    }
    pthread_barrier_wait( & barr_id); // wait for all the threads, then start parallel work


    if(has_factor == 0){ // number is prime    
      continue; 
    } 
    if(has_factor == 1){ continue; }// number has factor
    
    pthread_barrier_wait(& barr_id); // need to regroup here because we dont want to create a race condition   

    // threads find their min and max values form the array
    int64_t minimum = min_array[index];
    int64_t maximum;
    if(index == (long)min_array.size() - 1){
    	maximum = sqrt(num);
    }
    else{
	maximum = min_array[index+1] - 1;
    }

    // run is_prime to do the parralel work
    is_prime(num, minimum, maximum); 
          
    if(pthread_barrier_wait(& barr_id) != 0 && has_factor == 0){ //only one thread puts the prime number in the array
      result.push_back(num);
    }

    pthread_barrier_wait(& barr_id); // wait until everyone is done before restarting

  }
  return NULL;
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{
  result.clear(); 
  count = n_threads; 
  num_array = nums;

  pthread_t thread_id[n_threads];

  pthread_barrier_init(& barr_id, NULL, n_threads);

  for(long i = 0 ; i < n_threads ; i++){
    pthread_create(& thread_id[i], NULL, &run, (void*) i);

  }
  for(int i = 0 ; i < n_threads ; i++){

    pthread_join(thread_id[i], NULL);
  }
  pthread_barrier_destroy(& barr_id);

  return result;
}

#include <iostream>
#include "thread.h"
#include "semaphore.h"
#include <unistd.h>


//jantar dos filosofos 
using namespace std;
 
const int DELAY = 10000000;
const int ITERATIONS = 5;
 
Semaphore chopstick[5];
Semaphore mensagem;
 
int philosopher(int n)
{
    cout << "Philosopher " << n << " was born!\n";
 
    int first = (n < 4)? n : 0; // left for phil 0 .. 3, right for phil 4
    int second = (n < 4)? n + 1 : 4; // right for phil 0 .. 3, left for phil 4
 
    // Foram removidos do laço abaixo:
    //  - uma chamada para chopstick[first].p()
    //  - uma chamada para chopstick[second].p()
    //  - uma chamada para chopstick[first].v()
    //  - uma chamada para chopstick[second].v()
    for(int i = 0; i < ITERATIONS; i++) {
	

	mensagem.p();
	cout << "Philosopher " << n << " thinking ...\n";
	mensagem.v();

	for(int i = 0; i < DELAY * 10; i++);
	
	chopstick[first].p();
	chopstick[second].p();

	
//	mensagem.p();
	cout << "Philosopher " << n << " eating ...\n";
//	mensagem.v();

	for(int i = 0; i < DELAY; i++);

	chopstick[first].v();
	chopstick[second].v();
	
    }
 
    return n;
}
 
int main()
{
    cout << "The Dining-Philosophers Problem\n";
 
    Thread *phil[5];
    for(int i = 0; i < 5; i++)
	phil[i] = new Thread(&philosopher, i);
 
    int status;
    for(int i = 0; i < 5; i++) {
	phil[i]->join(&status);
	if(status == i)
	    cout << "Philosopher " << i << " went to heaven!\n";
	else
	    cout << "Philosopher " << i << " went to hell!\n";
    }
 
    return 0;
}

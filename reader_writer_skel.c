#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

#define NUMREAD 10			// Number of Readers
#define NUMWRITE 10			// Number of Writers

#define DEVICE_NAME "/dev/DUMMY_DEVICE"
#define NUMLOOP 100


/* Writer function */
void *write_func(void *arg) {
	int id = *((int*) arg);
	int fd = open("/dev/DUMMY_DEVICE", O_RDWR);
	int i, num_to_write;

	srand(id);

	for (i=0; i<NUMLLOP; i++) {
		num_to_write = rand()%10;
		write(fd, *num_to_write, 1);
	}
}

/* Reader function */
void *read_func(void *arg) {
	int id = *((int*) arg);
	int fd = open("/dev/DUMMY_DEVICE", O_RDWR);
	int i, num_to_read;

	srand(id*10 + 10);

	for (i=0; i<NUMLLOP; i++) {
		num_to_read = rand()%10;
		read(fd, *num_to_read, 1);
	}
}

int main(void)
{
	pthread_t read_thread[NUMREAD];
	pthread_t write_thread[NUMWRITE];


	return 0;
}


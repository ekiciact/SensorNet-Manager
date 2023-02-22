#include "main.h"

void* start_conmgr(void * port) {
int port_number;
memcpy(&port_number, port, sizeof(int));
sbuffer_t* sbuffer = NULL;

//start listening for connections
connmgr_listen(port_number, &sbuffer);

//free connmgr
connmgr_free();

#ifdef DEBUG
printf("Terminate connmgr\n");
#endif

//detach the thread
pthread_detach(threads[2]);
return NULL;
}


void* start_storagemgr(void * arg){
DBCONN *conn = NULL;
int connection_tries = 0;
    while(connection_tries < 3){
            // open a connection to the database
            conn = init_connection(1);

            if(conn != NULL){
                    break;
            }
            connection_tries++;
            sleep(1);
    }

    if(conn == NULL){
            
            log_event("Failed to connect to the database after 3 attempts\n");
            terminate();
            return NULL;
    }
    else {
            #ifdef DEBUG
            printf("Connection to SQL server established\n");
            #endif
            log_event("Connection to SQL server established\n");
    }
    // let the storagemgr check the buffer and store the data to the database
    storagemgr_parse_sensor_data(conn, &sbuffer);

    // close the database connection
    disconnect(conn);

    #ifdef DEBUG
    printf("Terminate storagemgr\n");
    #endif
    //detach the thread
        pthread_detach(threads[2]);
        return NULL;
}

void* start_datamgr(void * arg){
//open the file with the sensor mapping
FILE * fp= fopen("room_sensor.map","r");
if(fp == NULL){
char* log_string;
asprintf(&log_string, "Error opening file room_sensor.map in datamgr");
fifomgr_write(log_string);
free(log_string);
pthread_exit(NULL);
}
// Create datamgr
datamgr_init();
//let the datamgr check the sbuffer
datamgr_parse_sensor_data(fp, &sbuffer);

//close the file with the sensor mapping
fclose(fp);

//free datamgr
datamgr_free();

#ifdef DEBUG
printf("Terminate datamgr\n");
#endif

//detach the thread
pthread_detach(threads[1]);
return NULL;
}

void run_child (void) {
// create a log file called "gateway.log"
FILE* logfile = fopen("gateway.log", "w");
FILE_OPEN_ERROR(logfile);
// open the FIFO for reading
int logFifo = open("logFifo", O_RDONLY);
OPEN_ERROR(logFifo);

// read from FIFO and write to logfile
char log_message[256];
int sequence_number = 0;
while (read(logFifo, log_message, sizeof(log_message)) > 0) {
    time_t current_time = time(NULL);
    struct tm* time_info = localtime(&current_time);
    char timestamp[26];
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", time_info);
    fprintf(logfile, "%d %s %s\n", sequence_number, timestamp, log_message);
    sequence_number++;
}

// close the logfile and the FIFO
fclose(logfile);
close(logFifo);
}

//initialise the FIFO
fifomgr_init();

//initialise the shared buffer
sbuffer_init(&sbuffer);

//initialise the mutex
pthread_mutex_init(&fifolock, NULL);

//spawn the threads
pthread_create(&threads[0], NULL, start_conmgr, &port);
pthread_create(&threads[1], NULL, start_datamgr, NULL);
pthread_create(&threads[2], NULL, start_storagemgr, NULL);

//wait for the threads
pthread_join(threads[0], NULL);
pthread_join(threads[1], NULL);
pthread_join(threads[2], NULL);

//cleanup resources
pthread_mutex_destroy(fifolock);
sbuffer_free(sbuffer);
fifomgr_destroy();
#ifdef DEBUG
printf("Terminate gateway\n");
#endif

void print_help(void) {
printf("Usage: ./gateway [port_number]\n");
printf("[port_number] is the port number on which the gateway will listen for incoming sensor node connections.\n");
}

int log_sequence_number = 0;
void log_event(char* log_message){
    FILE* fp = fopen("gateway.log", "a"); //open file in append mode
    if(fp == NULL){
        printf("Error opening log file\n");
        return;
    }
    //get current timestamp
    time_t current_time = time(NULL);
    struct tm* time_info = localtime(&current_time);
    char timestamp[20];
    strftime(timestamp, 20, "%Y-%m-%d %H:%M:%S", time_info);
    
    log_sequence_number++;
    //write log message to file
    fprintf(fp, "%d %s %s\n", log_sequence_number, timestamp, log_message);
    fclose(fp);
}

void terminate (){
//variables
pid_t child_pid;
int child_exit_status;
    //cancel all thread that has been started
    for(int i = 0; i<3; i++){
        pthread_cancel(threads[i]);
    }

    //join all thread that has been started
    for(int i = 0; i<3; i++){
        pthread_join(threads[i], NULL);
    }
    
    //disconnect from DB
    disconnect(conn);

    //wait for child process to finish
    child_pid = wait(&child_exit_status);
    SYSCALL_ERROR( child_pid );
    if ( WIFEXITED(child_exit_status) ) //succesfull exit
    {
      #ifdef DEBUG
            printf("Child %d terminated with exit status %d\n", child_pid, WEXITSTATUS(child_exit_status));
      #endif
    }
    else                           //abnormal exit
    {
            printf("Child %d terminated abnormally\n\n", child_pid);
    }

    printf("The sensor gateway has ended.\n");

    //destroy the sbuffer
    sbuffer_free(&sbuffer);
    //destroy the lock
    pthread_mutex_destroy(&fifolock);

    //terminate main thread
    pthread_exit(NULL);
}

void fifomgr_init(){
    //create a mutex lock
    pthread_mutex_init(&fifolock, NULL);

    //create the fifo if it does not exist
    if( access( FIFO_NAME, F_OK ) == -1 ) {
        //create the fifo
        mkfifo(FIFO_NAME, 0666);
    }
    #ifdef DEBUG
    printf("fifo has been initialized\n");
    #endif
}

void fifomgr_read(){
char buffer[256];
int num_bytes;
//lock the mutex
pthread_mutex_lock(&fifolock);

//open the FIFO for reading
int fd = open("logFifo", O_RDONLY);
SYSCALL_ERROR(fd);

//read from FIFO
num_bytes = read(fd, buffer, sizeof(buffer));
SYSCALL_ERROR(num_bytes);

//print the received log message
printf("Log: %s\n", buffer);

//close the FIFO
close(fd);

//unlock the mutex
pthread_mutex_unlock(&fifolock);
}

void fifomgr_write(char* text) {
// Open the FIFO for writing
int fd = open("logFifo", O_WRONLY);
if (fd == -1) {
perror("open");
return;
}
// lock mutex before writting to the FIFO
pthread_mutex_lock(&fifolock);
// write text to the FIFO
int num_bytes = write(fd, text, strlen(text));
if (num_bytes == -1) {
    perror("write");
}
pthread_mutex_unlock(&fifolock);

// Close the FIFO
close(fd);
}





int main( int argc, char *argv[] )
{
    //Check if user has entered the port number
    if(argc != 2){
        print_help();
        return -1;
    }

    //Initialize the shared buffer
    if(sbuffer_init(&sbuffer) == SBUFFER_FAILURE){
        fprintf(stderr, "Error: Unable to initialize shared buffer\n");
        return -1;
    }

    //Initialize the mutex
    if(pthread_mutex_init(&fifolock, NULL) != 0){
        fprintf(stderr, "Error: Unable to initialize mutex\n");
        return -1;
    }

    //Initialize the FIFO
    fifomgr_init();

    //Creating the child process
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Error: Unable to create child process\n");
        return -1;
    } else if (pid == 0) {
        run_child();
    } else {
        run_parent(argv);
    }

    //wait for the child process to finish
    int status;
    waitpid(pid, &status, 0);

    //disconnect
    terminate();
    return 0;
}

#include <iostream>
#include <cstring>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <signal.h>
using namespace std;

#define MAX_NUM_ARGS (10) 

int fg_proc_count=0;
pid_t main_pid = getpid();
int status;

//structure of process
struct Proc {
    int id;
    char proc_status[20];
    char program_name[20];
    char* arguments[MAX_NUM_ARGS+2];
    Proc * next = NULL;
};

Proc * starting_proc = NULL;

void add_process(Proc*);

void parse_command(char*, char**, char**, int &);

void sigchld_handler(int);

void print_list();

void kill_all_child_processes();

void ignore_signal(int);

void reset_signal(int);


int main(int argc, char* argv[]) {
    char* input_buffer = NULL; char* command = NULL;
    char prompt[] = "sh >";    

    //program name + args + NULL
    char* args[MAX_NUM_ARGS+2];
    args[MAX_NUM_ARGS + 1] = NULL;

    //block Ctrl+C & Ctrl+Z signals
    ignore_signal(SIGINT);
    ignore_signal(SIGTSTP);    

    //background process = -1 ; foreground process = 1  ; undefined = 0
    int process_layer;

    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        printf("Error in initilizing handle function for SIGCHLD occurs\n");
        exit(EXIT_FAILURE);
    }	

	//busy loop
    while (true) {
        //wait for a fg process to finish if any
        while(fg_proc_count!=0);
        input_buffer = readline(prompt);
        process_layer = 0;

        parse_command(input_buffer, &command, args, process_layer);
        //handle background process
        if(process_layer==-1){
        	pid_t pid = fork();
        	if(pid == 0){
        		execvp(command, args);
        		printf("Call to execvp() unsuccessful\n");
        		exit(EXIT_FAILURE);
        	}
        	else if(pid>0){
        		Proc* new_process = (Proc * ) malloc(sizeof(Proc));
        		new_process -> id = pid;
        		strcpy(new_process -> proc_status, "running");
        		strcpy(new_process -> program_name, args[0]);
        		int i=0;
        		while(args[i]!=NULL){
			        new_process-> arguments[i] = (char *) malloc(strlen(args[i]) + 1);
			        if (new_process-> arguments[i] != NULL){
					    strcpy(new_process-> arguments[i], args[i]);
			        }
				    i++;
			    }
			    new_process-> arguments[i]=NULL;        	
        		new_process -> next = NULL;
        		add_process(new_process);
        	}
        	else { //fork failed
                printf("Fork function failed to create a child process.\n");
                exit(EXIT_FAILURE);
            }
        }//foreground process
        else if(process_layer ==1){
        	main_pid = fork();        	
        	if(main_pid == 0){
                //unblock Ctrl+C & Ctrl+Z
        		reset_signal(SIGINT);
       			reset_signal(SIGTSTP);       		      		
        		execvp(command, args);
        		printf("Call to execvp() unsuccessful\n");
        		exit(EXIT_FAILURE);
        	}
        	else if(main_pid>0){
        		Proc* new_process = (Proc * ) malloc(sizeof(Proc));
        		new_process -> id = main_pid;
        		strcpy(new_process -> proc_status, "running");
        		strcpy(new_process -> program_name, args[0]);
        		int i=0;
        		while(args[i]!=NULL){
			        new_process-> arguments[i] = (char *) malloc(strlen(args[i]) + 1);
			        if (new_process-> arguments[i] != NULL){
					strcpy(new_process-> arguments[i], args[i]);
			        }
				    i++;
			    }
			    new_process-> arguments[i]=NULL;         		
        		new_process -> next = NULL;
        		add_process(new_process);
        	}
        	else { //fork failed
                	printf("Fork function failed to create a child process.\n");
                	exit(EXIT_FAILURE);
            }
		    fg_proc_count = 1;
        }
        else if (strcmp(command, "list") == 0){
            print_list();
        }
        else if (strcmp(command, "exit") == 0) {
            kill_all_child_processes();
            exit(EXIT_SUCCESS);
        }
        else if(command == NULL){
        	continue;
        }
        else {
            printf("Unrecognized command.\n");
        }    
    }
    return 0;
}

// Add a process to the end of process list
void add_process(Proc * new_process) {
    if (starting_proc == NULL)
        starting_proc = new_process;
    else {
        Proc * tmp = starting_proc;
        while (tmp -> next != NULL)
            tmp = tmp -> next;
        tmp -> next = new_process;
    }
}

void parse_command(char* input_buffer, char** command, char** args, int& process_layer) {
    *command = strtok(input_buffer, " \t");
    if(strcmp(*command, "fg")==0){
        process_layer = 1;
        *command = strtok(NULL, " \t");
    }
    else if (strcmp(*command, "bg") == 0) {
        process_layer = -1;
        *command = strtok(NULL, " \t");
    }
    args[0] = *command;
    char* tmp;
    //index holder
    int i = 1; 
    while (tmp = strtok(NULL, " \t")) {
        args[i] = tmp;
        i++;
    }
    args[i] = NULL;
}

//SIGCHLD signal handler - catches signal from child whenever it changes its state
void sigchld_handler(int sig) {
    char message[50] = "";
    char id_str[10];
    //catches status whenever child changes its state, ensures reap
    int waitId = waitpid(0, &status, WUNTRACED);

    if (waitId > 0) {
        if (WIFEXITED(status)) {
            Proc* tmp = starting_proc;
            while (tmp != NULL && tmp->id != waitId)
                tmp = tmp->next;
            if (strcmp(tmp->proc_status, "terminated") != 0) {
                strcpy(tmp->proc_status, "terminated");
                sprintf(id_str, "%d", waitId);
                strcat(message, "Process ");
                strcat(message, id_str);
                strcat(message, " completed\n");
                write(STDOUT_FILENO, message, sizeof(message));
            }
        }
        else if (WIFSIGNALED(status)) {
            Proc* pok = starting_proc;
            while (pok != NULL && pok->id != waitId) {
                pok = pok->next;
            }
            if (strcmp(pok->proc_status, "terminated") != 0) {
                strcpy(pok->proc_status, "terminated");
            }
            sprintf(id_str, "%d", waitId);
            strcat(message, "Process ");
            strcat(message, id_str);
            strcat(message, " terminated\n");
            write(STDOUT_FILENO, message, sizeof(message));
        }
        else if (WIFSTOPPED(status)) {
            Proc* pok = starting_proc;
            while (pok != NULL && pok->id != waitId) {
                pok = pok->next;
            }
            if (strcmp(pok->proc_status, "stopped") != 0) {
                strcpy(pok->proc_status, "stopped");
            }
            sprintf(id_str, "%d", waitId);
            strcat(message, "Process ");
            strcat(message, id_str);
            strcat(message, " stopped\n");
            write(STDOUT_FILENO, message, sizeof(message));
        }
    }
    else if (waitId == -1) {
        printf("error: waiting for child");
        exit(EXIT_FAILURE);
    }
    fg_proc_count = 0;
}

// handle list command
void print_list() {
    Proc* tmp = starting_proc;
    while (tmp != NULL) {
        if (strcmp(tmp->proc_status, "terminated") != 0) {
            printf("%d: %s ", tmp->id, tmp->proc_status);
            int i = 0;
            while (tmp->arguments[i] != NULL) {
                printf("%s ", tmp->arguments[i]);
                i++;
            }
            printf("\n");
        }
        tmp = tmp->next;
    }
}

//Handle exit;   terminate all children from parent
void kill_all_child_processes() {
    while (starting_proc != NULL) {
        if (strcmp(starting_proc->proc_status, "terminated") != 0) {
            strcpy(starting_proc->proc_status, "terminated");
            kill(starting_proc->id, SIGKILL);
            sleep(1);
        }
        starting_proc = starting_proc->next;
    }
}

void ignore_signal(int sig) {
    if (signal(sig, SIG_IGN) == SIG_ERR) {
        printf("Failed to install signal handler for signal %d\n", sig);
        exit(1);
    }
}

void reset_signal(int sig) {
    if (signal(sig, SIG_DFL) == SIG_ERR) {
        printf("Failed to reset signal handler for signal %d\n", sig);
        exit(1);
    }
}

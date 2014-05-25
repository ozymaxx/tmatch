#include <stdlib.h>
#include <mqueue.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define MAX_FILES 10
#define MAX_LINE 2048
#define MAX_KEYWORD 250

struct Node {
	char keyword[MAX_LINE];
	struct Node *next;
};

struct ThreadParameter {
	char keyword[MAX_KEYWORD];
	int arg;
};

struct Node* keywordsHeads[MAX_FILES];
char filenames[MAX_FILES][MAX_KEYWORD];
pthread_t finders[MAX_FILES];

void addLine( struct Node **head, char* word) {
	if ( *head == NULL) {
		*head = (struct Node *) malloc( sizeof( struct Node) );
		strcpy((*head)->keyword, word);
		(*head)->next = NULL;
	}
	else {
		if ( strcmp( word, (*head)->keyword) < 0 ) {
			struct Node *newNode = (struct Node *) malloc( sizeof( struct Node) );
			strcpy( newNode->keyword, word);
			newNode->next = *head;
			*head = newNode;
		}
		else {
			struct Node *ptr = *head;
			while ( ptr->next) {
				if ( strcmp( ptr->keyword, word) < 0 && strcmp( ptr->next->keyword, word) >= 0 ) {
					break;
				}
				ptr = ptr->next;
			}
			struct Node *newNode = (struct Node *) malloc( sizeof( struct Node) );
			strcpy( newNode->keyword, word);
			struct Node *temp = ptr->next;
			ptr->next = newNode;
			newNode->next = temp;
		}
	}
}

void displayList( struct Node *head) {
	struct Node *ptr = head;
	while ( ptr) {
		printf( "%s", ptr->keyword);
		ptr = ptr->next;
	}
}

void freeList( struct Node **head) {
	struct Node *temp;
	while ( *head) {
		temp = (*head)->next;
		free( *head);
		*head = temp;
	}
}

void* finder( void *arg) {
	FILE *input;
	char line[MAX_LINE];
	struct ThreadParameter *param;
	param = (struct ThreadParameter *) arg;
	
	input = fopen( filenames[param->arg], "r");
	if ( input == NULL) {
		fprintf(stderr,"ERROR: File opening failed! - %s\n", filenames[param->arg]);
		exit(1);
	}
	
	while ( fgets( line, MAX_LINE, input) != NULL) {
		if ( strstr( line, param->keyword) ) {
			//printf("%s",line);
			if ( line) {
				if ( strlen( line) > 0 ) {
					//fprintf( stderr, "%d adding %s", param->arg, line);
					addLine( &(keywordsHeads[param->arg]), line);
				}
			}
		}
	}
	
	//printf("%d\n",param->arg);
	//displayList(keywordsHeads[param->arg]);
	fclose( input);
	
	fprintf(stderr,"Thread %d exiting...\n",param->arg);
	
	pthread_exit(0);
} 

void* combiner( void *arg) {
	struct Node *resultLines = NULL;
	int param;
	param = (int) arg;
	
	/*
	for ( int i = 0; i < param; i++) {
		fprintf( stderr, "%d -> %d\n", i, finders[i]);
	}*/
	
	/*
	for ( int t = 0; t < param; t++) {
		fprintf( stderr, "%d\n", t);
		displayList( keywordsHeads[t]);
	}*/
	
	for ( int i = 0; i < param; i++) {
		if ( keywordsHeads[i]) {
			struct Node *ptr = keywordsHeads[i];
			while ( ptr) {
				//fprintf( stderr, "%s adding from %d",ptr->keyword,i);
				addLine( &resultLines, ptr->keyword);
				ptr = ptr->next;
			}
			
			freeList( &(keywordsHeads[i]) );
		}
	}
	
	displayList( resultLines);
	freeList( &resultLines);
	
	fprintf( stderr, "Combiner thread exiting...\n");
	
	pthread_exit(0);
}

int main( int argc, char** argv) {
	pthread_t combinerTID;
	int numOfFiles;
	
	numOfFiles = atoi( argv[2]);
	
	for ( int i = 0; i < numOfFiles; i++) {
		strcpy( filenames[i], argv[3+i]);
	}
	
	for ( int j = 0; j < numOfFiles; j++) {
		keywordsHeads[j] = NULL;
	}
	
	for ( int k = 0; k < numOfFiles; k++) {
		struct ThreadParameter *param = (struct ThreadParameter *) malloc( sizeof( struct ThreadParameter) );
		strcpy(param->keyword,argv[1]);
		param->arg = k;
		int finderReturn = pthread_create( &(finders[k]), NULL, finder, (void *) param );
		
		if ( finderReturn != 0) {
			fprintf( stderr, "ERROR: A finder thread creation failed!\n");
			exit(1);
		}
	}
	
	for ( int i = 0; i < numOfFiles; i++) {
		pthread_join( finders[i], NULL);
	}
	
	int combinerReturn = pthread_create( &combinerTID, NULL, combiner, (void *) numOfFiles);
	
	if ( combinerReturn != 0) {
		fprintf( stderr, "ERROR: Combiner thread creation failed!\n");
		exit(1);
	}

	pthread_join( combinerTID, NULL);
	
	fprintf( stderr, "Process done!\n");
	
	return 0;
}


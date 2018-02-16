#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "posts.h"
#include "config.h"
#include "database.h"

void db_thread_save(FILE **db_file_ptr, struct post_list **curr_post_list_ptr, int save_headers) {
    FILE *db_file = *db_file_ptr;
    struct post_list *curr_post_list = *curr_post_list_ptr;
 
    #define STR_VALUE_OR_EMPTY(x) (strlen(x) ? x : DATABASE_DELIM_EMPTY)
        
    struct post *post = curr_post_list->last;
    while(post != NULL) {
        if(!post->saved) {
            fprintf(db_file, "%i,%s,%s,%s,%li," DATABASE_DELIM_EMPTY "\n",
                    post->id, post->author,
                    STR_VALUE_OR_EMPTY(post->subject),
                    post->comment, post->created_time);
            post->saved = 1;
        }

        struct post *reply = post->replies->first;
        while(reply != NULL) {
            if(!reply->saved) {
                fprintf(db_file, "%i,%s,%s,%s,%li,%i\n",
                    reply->id, reply->author,
                    STR_VALUE_OR_EMPTY(reply->subject),
                    reply->comment, reply->created_time,
                    post->id);
                reply->saved = 1;
            }
            reply = reply->next;
        }
            
        post = post->prev;
    }
    
    fflush(db_file);
}

void *db_thread_main(void *db_thread_params_ptr) {
    printf("Database thread loaded!\n");
    
    // Params
    struct db_thread_params *params = db_thread_params_ptr;
    struct post_list *curr_post_list = params->curr_post_list;
    
    // Load file
    FILE *db_file = fopen(DATABASE_FILE, "w+");
    if(db_file == NULL) {
        printf("Database init error: %s\n", strerror(errno));
        return NULL;
    }
    
    // Poll for updates
    int header_saved = 0;
    while(1) {
        if(pthread_rwlock_rdlock(&params->dblock) == 0) {
            // Save the database
            if(!header_saved) {
                fwrite(DATABASE_HEADER, 1, strlen(DATABASE_HEADER), db_file);
                header_saved = 1;
            }
            
            if(params->should_save) {
                db_thread_save(&db_file, &curr_post_list, !header_saved);
                params->should_save = 0;
                printf("Database saved.\n");
            }
            
            if(pthread_rwlock_unlock(&params->dblock) < 0)
                printf("Unable to release rwlock: %s\n", strerror(errno));
        } else
            printf("Unable to acquire rwlock: %s\n", strerror(errno));
        
        usleep(LOOP_SLEEP);
    }
    
    fclose(db_file);
    
    return NULL;
}

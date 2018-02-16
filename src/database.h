#pragma once
#include "pthread.h"

struct db_thread_params {
    struct post_list *curr_post_list;
    int should_save;
    pthread_rwlock_t dblock;
};

void db_thread_save(FILE **db_file_ptr, struct post_list **curr_post_list_ptr, int save_headers);
void *db_thread_main(void *db_thread_params_ptr);

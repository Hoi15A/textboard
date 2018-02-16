#pragma once

#include <time.h>
#include <stdlib.h>

// Post list
struct post_list {
    size_t length;
    struct post *first, *last;
};

struct post_list *curr_post_list;

struct post_list *post_list_create();
void post_list_destroy(struct post_list *list);
void post_list_prepend(struct post_list *list, struct post *post);
void post_list_append(struct post_list *list, struct post *post);
char *post_list_render(struct post_list *list, const int is_reply);
struct post *post_list_find(struct post_list *list, int id);
void post_list_bump(struct post_list *list, struct post *post);
void post_list_debug(struct post_list *list);

// Post (NOTE: can only be in one list)
// too lazy to update the struct
struct post {
    unsigned int id;
    char *author;
    char *subject;
    char *comment;
    time_t created_time;
    int saved;
    struct post_list *replies;
    struct post *prev, *next;
    struct post *parent; // don't free me
};
unsigned int global_id;

struct post *post_create(unsigned int id, const char *author, const char *subject, const char *comment, time_t created_time, struct post *parent);
void post_destroy(struct post *post);
char *post_render(struct post *post);
void post_debug(struct post *post);

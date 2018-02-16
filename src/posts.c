#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "posts.h"
#include "helpers.h"
#include "static.h"
#include "config.h"
#include "database.h"

#define RENDER_POST(x, length, element) \
    char time_utc[TIME_LENGTH]; \
    memset(time_utc, 0, TIME_LENGTH); \
    strftime(time_utc, TIME_LENGTH, "%F %T", gmtime(&post->created_time)); \
\
    char time[TIME_LENGTH]; \
    memset(time, 0, TIME_LENGTH); \
    strftime(time, TIME_LENGTH, "%F %T", localtime(&post->created_time)); \
\
    snprintf(x, length, element, \
            post->subject, post->author, time_utc, time, \
            post->id, post->id, post->replies->length, post->comment);

#define GUESS_POST_RENDER_LENGTH(element) /* generous guesstimate */ \
    (strlen(element) + \
     strlen(post->author) + \
     strlen(post->subject) + \
     strlen(post->comment) + \
     digits(post->replies->length) + 2 + \
     TIME_LENGTH * 2)

extern struct db_thread_params *db_thread_params;

// Post list
struct post_list *post_list_create() {
    struct post_list *list = malloc(sizeof(struct post_list));
    if(list == NULL) return NULL;
    list->first = NULL;
    list->last = NULL;
    list->length = 0;
    return list;
}

void post_list_destroy(struct post_list *list) {
    struct post *curr = list->first, *next;
    while(curr != NULL) {
        next = curr->next;
        post_destroy(curr);
        curr = next;
    }
    free(list);
}

void post_list_prepend(struct post_list *list, struct post *post) {
    // Used only for frontpage/catalog list
    if(list->first == NULL) {
        list->first = post;
        list->last = post;
    } else {
        post->next = list->first;
        list->first->prev = post;
        list->first = post;
    }
    
    list->length++;
    
    // Remove from queue once big enough
    while(list->length > MAX_POSTS) {
        struct post *last = list->last->prev;
        post_destroy(list->last);
        last->next = NULL;
        list->last = last;
    }
}

void post_list_append(struct post_list *list, struct post *post) {
    // Used for comment replies
    if(list->first == NULL) {
        list->first = post;
        list->last = post;
    } else {
        post->prev = list->last;
        list->last->next = post;
        list->last = post;
    }
    
    list->length++;
}

char *post_list_render(struct post_list *list, const int is_reply) {
    char *element;
    if(is_reply) {
        element = POST_REPLY_ELEMENT;
    } else {
        element = POST_ELEMENT;
    }
    
    // guestimate the html size
    size_t list_length = 0;
    struct post *post = list->first;
    while(post != NULL) {
        list_length += GUESS_POST_RENDER_LENGTH(element);
        post = post->next;
    }
    if(list_length == 0) {
        return NULL;
    }
    
    char *html = malloc(list_length + 1);
    if(html == NULL) return NULL;
    memset(html, 0, list_length);
    post = list->first;
    while(post != NULL) {
        const size_t length = GUESS_POST_RENDER_LENGTH(element);
        
        char *tmp = malloc(length);
        memset(tmp, 0, length);
        RENDER_POST(tmp, length, element)
        strcat(html, tmp);
        free(tmp);
        
        post = post->next;
    }
    
    return html;
}

struct post *post_list_find(struct post_list *list, int id) {
    struct post *post = list->first;
    while(post != NULL) {
        if(post->id == id)
            return post;
        post = post->next;
    }
    return NULL;
}

void post_list_bump(struct post_list *list, struct post *right) {
    struct post *left = list->first;
    
    if(left == right) {
        return;
    }
    
    if(left->next == right) { // right next to each other
        left->next = right->next;
        right->prev = left->prev;
        
        if (left->next != NULL)
            left->next->prev = left;

        if (right->prev != NULL)
            right->prev->next = right;

        right->next = left;
        left->prev = right;
    } else {
        struct post *x = right->prev;
        struct post *y = right->next;
        
        right->prev = left->prev;
        right->next = left->next;
        
        left->prev = x;
        left->next = y;

        if (right->next != NULL)
            right->next->prev = right;
        if (right->prev != NULL)
            right->prev->next = right;

        if (left->next != NULL)
            left->next->prev = left;
        if (left->prev != NULL)
            left->prev->next = left;
    }
    
    list->first = right;
    if(left->next == NULL)
        list->last = left;
}

void post_list_debug(struct post_list *list) {
    #ifndef PRODUCTION
    struct post *curr = list->first;
    while(curr != NULL) {
        post_debug(curr);
        curr = curr->next;
    }
    #endif
}

// Post
struct post *post_create(unsigned int id, const char *author, const char *subject, const char *comment, time_t created_time, struct post *parent) {
    if(db_thread_params != NULL && pthread_rwlock_wrlock(&db_thread_params->dblock) < 0) {
        printf("Unable to acquire rwlock: %s\n", strerror(errno));
        return NULL;
    }
    
    struct post *post = malloc(sizeof(struct post));
    if(post == NULL) return NULL;
    if(id == (unsigned int)-1) {
        post->id = global_id++;
        post->author = encode_html(author);
        post->subject = encode_html(subject);
        post->comment = encode_html(comment);
    } else {
        post->id = id;
        post->author = clone_str(author);
        post->subject = clone_str(subject);
        post->comment = clone_str(comment);
    }
    if( post->author  == NULL ||
        post->subject == NULL ||
        post->comment == NULL ) {
        if(post->author != NULL) free(post->author);
        if(post->subject != NULL) free(post->subject);
        if(post->comment != NULL) free(post->comment);
        free(post);
        return NULL;
    }
    if(created_time)
        post->created_time = created_time;
    else
        post->created_time = time(NULL);
    post->saved = 0;
    post->replies = post_list_create();
    post->prev = NULL;
    post->next = NULL;
    
    if(parent == NULL) {
        post->parent = NULL;
        post_list_prepend(curr_post_list, post);
    } else {
        post->parent = parent;
        post_list_append(parent->replies, post);
        post_list_bump(curr_post_list, parent);
    }
    
    if(db_thread_params != NULL)
        db_thread_params->should_save = 1;
        
    post_debug(post);
    
    if(db_thread_params != NULL && pthread_rwlock_unlock(&db_thread_params->dblock) < 0) {
        printf("Unable to release rwlock: %s\n", strerror(errno));
        free(post);
        return NULL;
    }
    
    return post;
}

void post_destroy(struct post *post) {
    free(post->author);
    free(post->subject);
    free(post->comment);
    post_list_destroy(post->replies);
    if(post->parent == NULL) {
        curr_post_list->length--;
    } else {
        post->parent->replies->length--;
    }
    
    if(post->next != NULL)
        post->next->prev = post->prev;
    if(post->prev != NULL)
        post->prev->next = post->next;
    
    free(post);
}

char *post_render(struct post *post) {
    const size_t length = GUESS_POST_RENDER_LENGTH(POST_ELEMENT);
    char *html = malloc(length);
    memset(html, 0, length);
    RENDER_POST(html, length, POST_ELEMENT)
    return html;
}

void post_debug(struct post *post) {
    #ifndef PRODUCTION
    printf("Post(%s, %s, %s, %p, %p)\n",post->author,post->subject,post->comment,post->prev,post->next);
    #endif
}

#pragma once

// Site
// Name of the site (will be used in title/header)
#define SITE_NAME "/c/chan"

// Name of the board software. Build scripts append commit hash.
#ifndef FOOTER_VERSION
#define FOOTER_VERSION "textboard"
#endif

// Database
// Where the database will be located relative to the server binary
#define DATABASE_FILE "database.csv"
// Header of the database file
#define DATABASE_HEADER "id,author,subject,comment,created_time,parent\n"
// Ensures empty columns will be parsed correctly
// If you're not sure what to change it to, don't change it.
#define DATABASE_DELIM_EMPTY "&"

// Posts
// Default name of post if no name specified
// NOTE: don't do single quotes here, it breaks the form's HTML
#define DEFAULT_USERNAME "anon"
// Max number of posts in the server
#define MAX_POSTS 90

// Misc (should be left at default)
// Maximum number of characters the server should receive
#define BUFFSIZE 1024
// Length of string containing the HTTP header Date.
#define TIME_LENGTH 40
// Timeout in seconds for recv
#define TIMEOUT 60
// Time for event loop to sleep in milliseconds
#define LOOP_SLEEP 1

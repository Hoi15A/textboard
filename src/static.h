#pragma once

#include "config.h"

// Static files
// (state of the art shitty html4 that is)
#define RESPONSE_HEADER \
"HTTP/1.1 200 OK\n" \
"Content-Type: text/html\n" \
"Content-Length: "

#include "static_files.h"

#define HEADER_FILE \
"<html>" \
"<head>" \
    "<title>" SITE_NAME "</title>" \
    "<meta charset='utf-8'>" \
    "<link rel='stylesheet' href='/style.css'>" \
"</head>" \
"<body>"

#define FORM_TEMPLATE(prepend) \
    "<form action=/post method=post>" \
        "<table>" \
        prepend \
        "<tr>" \
            "<td valign=top align=right><label for=name>Name:</label></td>" \
            "<td><input name=name size=30 placeholder='" DEFAULT_USERNAME "'></td>" \
        "</tr>" \
        "<tr>" \
            "<td valign=top align=right><label for=subject>Subject:</label></td>" \
            "<td><input name=subject size=30></td>" \
        "</tr>" \
        "<tr>" \
            "<td valign=top align=right><label for=comment>Comment:</label></td>" \
            "<td><textarea required name=comment cols=30 rows=12></textarea></td>" \
        "</tr>" \
        "<tr>" \
            "<td></td>" \
            "<td><input type=submit value=Post!></td>" \
        "</tr>" \
        "</table>" \
    "</form>" \

#define INDEX_FILE_HEADER \
    HEADER_FILE \
    "<center id='header'>" \
    "<h1><a href=/>" SITE_NAME "</a></h1>" \
    HEADER_MESSAGES \
    FORM_TEMPLATE() \
    "</center>"

#define REPLY_FILE_HEADER \
    HEADER_FILE \
    "<center id='header'>" \
    "<h1><a href=/>" SITE_NAME "</a></h1>" \
    HEADER_MESSAGES \
    FORM_TEMPLATE(\
        "<tr>" \
            "<td colspan=2>" \
                "<center>Replying to post..." \
                "<input type=text readonly value='%i' size=3 name=reply_to style='display:none;'></center>" /* shitty html hack */ \
            "</td>" \
        "</tr>") \
    "</center>"

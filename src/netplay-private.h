#ifndef _MDFN_NETPLAY_PRIVATE_H
#define _MDFN_NETPLAY_PRIVATE_H

/* Migrate some formerly private structs and variables from netplay.cpp so that
other modules can gain access (eg. graffiti) */

struct CommandEntry
{
 const char *name;
 bool (*func)(const char* arg);
 const char *help_args;
 const char *help_desc;
};

extern char *OurNick;

#endif
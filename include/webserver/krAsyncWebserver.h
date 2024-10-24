#ifndef ASYNCWEBSERVER_H
#define ASYNCWEBSERVER_H

void webserver_init();
void webserver_notify_clients();
void webserver_cleanup_clients(void);

#endif
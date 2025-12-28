
#include "Notification.hpp"

Notification::Notification(std::string identification){
    program_ident = identification;
    openlog(program_ident.c_str(), LOG_PID | LOG_CONS, LOG_USER);
    syslog(priority_info, "Connection opens to the system logger");
}
void Notification::notice_info(std::string message){
    syslog(priority_info, "%s", message.c_str());
    }
void Notification::notice_err(std::string message){
    syslog(priority_err, "%s", message.c_str());
}
Notification::~Notification(){
    syslog(priority_info, "Connection closes to the system logger");
    closelog();
}
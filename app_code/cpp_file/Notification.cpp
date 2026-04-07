
#include "Notification.hpp"
#include <string>
#include <syslog.h>

/*
 * Summary:
 * Initialize syslog connection for the application.
 *
 * Parameters:
 * - identification (std::string): program identifier used by syslog.
 *
 * Returns:
 * - No return value.
 */
Notification::Notification(std::string identification){
    program_ident = identification;
    openlog(program_ident.c_str(), LOG_PID | LOG_CONS, LOG_USER);
    syslog(priority_info, "Connection opens to the system logger");
}

/*
 * Summary:
 * Send informational message to syslog.
 *
 * Parameters:
 * - message (std::string): message to log.
 *
 * Returns:
 * - No return value.
 */
void Notification::notice_info(std::string message){
    syslog(priority_info, "%s", message.c_str());
}

/*
 * Summary:
 * Send error message to syslog.
 *
 * Parameters:
 * - message (std::string): error message to log.
 *
 * Returns:
 * - No return value.
 */
void Notification::notice_err(std::string message){
    syslog(priority_err, "%s", message.c_str());
}

/*
 * Summary:
 * Close syslog connection when object is destroyed.
 *
 * Parameters:
 * - No parameters.
 *
 * Returns:
 * - No return value.
 */
Notification::~Notification(){
    syslog(priority_info, "Connection closes to the system logger");
    closelog();
}


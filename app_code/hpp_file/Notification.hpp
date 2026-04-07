#ifndef NOTIFICATION_HPP
#define NOTIFICATION_HPP
#include <syslog.h>
#include <string>

/*
 * Summary:
 * Notification class used to send informational and error messages to syslog.
 *
 * Parameters:
 * - None (class definition)
 *
 * Returns:
 * - Not applicable
 */
class Notification{
    public:

        /*
         * Summary:
         * Initialize notification system with program identification.
         *
         * Parameters:
         * - identification (std::string): program name used in syslog.
         *
         * Returns:
         * - No return value.
         */
        Notification(std::string identification);

        /*
         * Summary:
         * Send informational message to syslog.
         *
         * Parameters:
         * - message (std::string): information message to log.
         *
         * Returns:
         * - No return value.
         */
        void notice_info(std::string message);

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
        void notice_err(std::string message);

        /*
         * Summary:
         * Destroy notification object and close syslog.
         *
         * Parameters:
         * - No parameters.
         *
         * Returns:
         * - No return value.
         */
        ~Notification();

    private:
        // program identifier used by syslog
        std::string program_ident;

        // syslog info priority
        const int priority_info = LOG_INFO;

        // syslog error priority
        const int priority_err = LOG_ERR;
};
#endif


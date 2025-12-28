#ifndef NOTIFICATION_HPP
#define NOTIFICATION_HPP
#endif
#include <syslog.h>
#include <string>

class Notification{
    public:
        Notification(std::string identification);
        void notice_info(std::string message);
        void notice_err(std::string message);
        ~Notification();

    private:
        std::string program_ident;
        const int priority_info = LOG_ALERT;
        const int priority_err = LOG_ERR;
};
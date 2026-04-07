#include <iostream>
#include "System.hpp"
int main(){
        bool result = system_init();
        std::cout << "result: "<< result << "true: "<< true<<"\n";
        if (result){
                std::cout << "pour commencer cliquer sur s\n";
                char s;
                std::cin >>s;
                if (s == 's'){
                        result = start();
                        if (result){
                                result = system_end();
                        }
                        std::cout << "fin du programme\n";
                }
        }
        return 1 ;
}


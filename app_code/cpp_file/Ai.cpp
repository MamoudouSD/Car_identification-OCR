#include "Ai.hpp"

template <typename T>
Ai<T>::Ai(std::string path, Notification* n){
    model_path = path;
    notif = n;
}
template <typename T>
Ai<T>::~Ai(){
}


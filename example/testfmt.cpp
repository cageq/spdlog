#include <spdlog/sinks/shm_buffer.h>
#include <chrono> 
#include <thread> 
#include <iostream>




int main(int argc, char * argv[]){


    fmt::shm_buffer shmBuffer; 



    auto out = fmt::shm_buffer();

    uint32_t  index = 0; 
    while(1){

        format_to(std::back_inserter(out), "{} For a moment, {} happened.\n",index++ ,  "nothing");

        std::cout << "index is " << index  << std::endl ; 
        std::this_thread::sleep_for(std::chrono::microseconds(1000)); 
    }

    return 0; 
}

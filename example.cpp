#include <iostream>
#include "inc/scmp_queue.h"

int main(int argc, char *argv[])
{
    scmp_queue<int> queue;

    auto value = queue.pop();

    queue.push(5);
    queue.push(6);
    queue.push(7);

    //auto value = queue.pop();

    std::cout << value << std::endl;

    return 0;
}

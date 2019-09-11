#include "inc/scmp_queue.h"

int main(int argc, char *argv[])
{
    scmp_queue<int> queue;

    queue.push(5);
    queue.push(6);
    queue.push(7);

    return 0;
}

#include "test_framework.h"
#include "tcp_tracker.h"

//TH_SYN
#include <netinet/tcp.h>

void test_dummy() {
    TEST_ASSERT(1 == 1, "Dummy test runs successfully");
}

int main() {
    test_dummy();
    return 0;
}
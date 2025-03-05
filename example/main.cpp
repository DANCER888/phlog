#include <iostream>
#include "../include/phlog.h"

int main() {
    PHLOG("yo wassup!!");
    PHLOG_SUCCESS("yo wassup!!");
    PHLOG_WARNING("yo wassup!!");
    PHLOG_ERROR("yo wassup!!");
    PHLOG_CRITICAL("yo wassup!!");
    std::cin.get();
    return 0;
}
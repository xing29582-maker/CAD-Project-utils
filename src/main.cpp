#include <iostream>
#include "distance.h"
#include <nlohmann/json.hpp>

int main()
{
    const double d = cadutils::distance2D(0, 0, 3, 4);
    std::cout << "distance(0,0)-(3,4) = " << d << "\n";
    return 0;
}
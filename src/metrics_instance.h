#pragma once

#include <prometheus/registry.h>

class Metrics
{
public:
    Metrics();
    
    static prometheus::Registry& registry();
};

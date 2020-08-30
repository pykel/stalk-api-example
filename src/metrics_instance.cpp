#include "metrics_instance.h"

Metrics::Metrics()
{
}
    
prometheus::Registry& Metrics::registry()
{
    static prometheus::Registry r;
    return r;
}

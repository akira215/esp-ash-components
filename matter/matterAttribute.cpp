/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#include "matterAttribute.h"
#include <matterCluster.h>

static const char *TAG = "MatterAttribute";

MatterAttribute::MatterAttribute(MatterCluster* cluster,esp_matter::attribute_t* attribute) :
                         _cluster(cluster), _attribute(attribute)
{ 

}

MatterAttribute::~MatterAttribute()
{
    // TODO remove attribute and endpoints
}

/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once


#include "matterNode.h"

template<typename C, typename... Args>
void MatterAttribute::registerAttrUpdateHandler(void (C::* func)(Args...), C* instance) {
    MatterNode::getInstance()->registerAttrUpdateHandler(func, instance, _endpointId, _clusterId, _attributeId);
}


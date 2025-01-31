// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "pch.h"

#include "FacadeType.h"
#include "NativeAnimatedNodeManager.h"
#include "StyleAnimatedNode.h"

namespace Microsoft::ReactNative {
StyleAnimatedNode::StyleAnimatedNode(
    int64_t tag,
    const winrt::Microsoft::ReactNative::JSValueObject &config,
    const std::shared_ptr<NativeAnimatedNodeManager> &manager)
    : AnimatedNode(tag, manager) {
  for (const auto &entry : config[s_styleName].AsObject()) {
    m_propMapping.insert({entry.first, static_cast<int64_t>(entry.second.AsDouble())});
  }
}

void StyleAnimatedNode::CollectViewUpdates(const folly::dynamic & /*propsMap*/) {}

std::unordered_map<FacadeType, int64_t> StyleAnimatedNode::GetMapping() {
  std::unordered_map<FacadeType, int64_t> mapping;
  for (const auto &prop : m_propMapping) {
    if (const auto manager = m_manager.lock()) {
      if (const auto transformNode = manager->GetTransformAnimatedNode(prop.second)) {
        const auto transformMapping = transformNode->GetMapping();
        mapping.insert(transformMapping.begin(), transformMapping.end());
        break;
      }
    }

    const auto &facade = StringToFacadeType(prop.first);
    if (facade != FacadeType::None) {
      mapping.insert({facade, prop.second});
    }
  }
  return mapping;
}
} // namespace Microsoft::ReactNative

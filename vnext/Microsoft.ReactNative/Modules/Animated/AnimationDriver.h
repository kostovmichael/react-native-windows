// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once
#include "NativeAnimatedNodeManager.h"
#include "ValueAnimatedNode.h"

namespace Microsoft::ReactNative {
typedef std::function<void(bool)> Callback;

class ValueAnimatedNode;
class AnimationDriver : public std::enable_shared_from_this<AnimationDriver> {
 public:
  AnimationDriver(
      int64_t id,
      int64_t animatedValueTag,
      const Callback &endCallback,
      const winrt::Microsoft::ReactNative::JSValueObject &config,
      const std::shared_ptr<NativeAnimatedNodeManager> &manager);
  virtual ~AnimationDriver();
  void StartAnimation();
  void StopAnimation(bool ignoreCompletedHandlers = false);

  virtual std::tuple<comp::CompositionAnimation, comp::CompositionScopedBatch> MakeAnimation(
      const winrt::Microsoft::ReactNative::JSValueObject & /*config*/) {
    return std::make_tuple(nullptr, nullptr);
  };

  inline constexpr int64_t Id() {
    return m_id;
  };

  inline constexpr int64_t AnimatedValueTag() {
    return m_animatedValueTag;
  }

  inline Callback EndCallback() noexcept {
    return m_endCallback;
  }

  inline const winrt::Microsoft::ReactNative::JSValueObject &AnimationConfig() const noexcept {
    return m_config;
  }

  virtual double ToValue() {
    return 0;
  };

  virtual std::vector<double> Frames() {
    return std::vector<double>();
  }

  void DoCallback(bool value);

 private:
  Callback m_endCallback{};
#ifdef DEBUG
  int m_debug_callbackAttempts{0};
#endif // DEBUG

 protected:
  ValueAnimatedNode *GetAnimatedValue();

  int64_t m_id{0};
  int64_t m_animatedValueTag{};
  int64_t m_iterations{0};
  winrt::Microsoft::ReactNative::JSValueObject m_config{};
  std::weak_ptr<NativeAnimatedNodeManager> m_manager{};

  comp::CompositionAnimation m_animation{nullptr};
  comp::CompositionScopedBatch m_scopedBatch{nullptr};
  // auto revoker for scopedBatch.Completed is broken, tracked by internal bug
  // #22399779
  winrt::event_token m_scopedBatchCompletedToken{};
  bool m_started{false};
  bool m_stopped{false};
  bool m_ignoreCompletedHandlers{false};
};
} // namespace Microsoft::ReactNative

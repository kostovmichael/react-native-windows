// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include <jsi/jsi.h>
#include <math.h>
#include "SpringAnimationDriver.h"

namespace Microsoft::ReactNative {
SpringAnimationDriver::SpringAnimationDriver(
    int64_t id,
    int64_t animatedValueTag,
    const Callback &endCallback,
    const winrt::Microsoft::ReactNative::JSValueObject &config,
    const std::shared_ptr<NativeAnimatedNodeManager> &manager,
    const winrt::Microsoft::ReactNative::JSValueArray &dynamicToValues)
    : CalculatedAnimationDriver(id, animatedValueTag, endCallback, config, manager),
      m_dynamicToValues(dynamicToValues.Copy()) {
  m_springStiffness = config[s_springStiffnessParameterName].AsDouble();
  m_springDamping = config[s_springDampingParameterName].AsDouble();
  m_springMass = config[s_springMassParameterName].AsDouble();
  m_initialVelocity = config[s_initialVelocityParameterName].AsDouble();
  m_endValue = config[s_endValueParameterName].AsDouble();
  m_restSpeedThreshold = config[s_restSpeedThresholdParameterName].AsDouble();
  m_displacementFromRestThreshold = config[s_displacementFromRestThresholdParameterName].AsDouble();
  m_overshootClampingEnabled = config[s_overshootClampingEnabledParameterName].AsBoolean();
  m_iterations = static_cast<int>(config[s_iterationsParameterName].AsDouble());
}

bool SpringAnimationDriver::IsAnimationDone(
    double currentValue,
    std::optional<double> /*previousValue*/,
    double currentVelocity) {
  return (
      IsAtRest(currentVelocity, currentValue, m_endValue) ||
      (m_overshootClampingEnabled && IsOvershooting(currentValue)));
}

std::tuple<float, double> SpringAnimationDriver::GetValueAndVelocityForTime(double time) {
  const auto toValue = [this, time]() {
    const auto frameFromTime = static_cast<int>(time * 60.0);
    if (frameFromTime < static_cast<int>(m_dynamicToValues.size())) {
      return m_startValue + (m_dynamicToValues[frameFromTime].AsDouble() * (m_endValue - m_startValue));
    }
    return m_endValue;
  }();
  const auto c = m_springDamping;
  const auto m = m_springMass;
  const auto k = m_springStiffness;
  const auto v0 = -m_initialVelocity;

  const auto zeta = c / (2 * std::sqrt(k * m));
  const auto omega0 = std::sqrt(k / m);
  const auto omega1 = omega0 * std::sqrt(1.0 - (zeta * zeta));
  const auto x0 = toValue - m_startValue;

  if (zeta < 1) {
    const auto envelope = std::exp(-zeta * omega0 * time);
    const auto value = static_cast<float>(
        toValue -
        envelope * ((v0 + zeta * omega0 * x0) / omega1 * std::sin(omega1 * time) + x0 * std::cos(omega1 * time)));
    const auto velocity = zeta * omega0 * envelope *
            (std::sin(omega1 * time) * (v0 + zeta * omega0 * x0) / omega1 + x0 * std::cos(omega1 * time)) -
        envelope * (std::cos(omega1 * time) * (v0 + zeta * omega0 * x0) - omega1 * x0 * std::sin(omega1 * time));
    return std::make_tuple(value, velocity);
  } else {
    const auto envelope = std::exp(-omega0 * time);
    const auto value = static_cast<float>(m_endValue - envelope * (x0 + (v0 + omega0 * x0) * time));
    const auto velocity = envelope * (v0 * (time * omega0 - 1) + time * x0 * (omega0 * omega0));
    return std::make_tuple(value, velocity);
  }
}

bool SpringAnimationDriver::IsAtRest(double currentVelocity, double currentValue, double endValue) {
  return std::abs(currentVelocity) <= m_restSpeedThreshold &&
      (std::abs(currentValue - endValue) <= m_displacementFromRestThreshold || m_springStiffness == 0);
}

bool SpringAnimationDriver::IsOvershooting(double currentValue) {
  return m_springStiffness > 0 &&
      ((m_startValue < m_endValue && currentValue > m_endValue) ||
       (m_startValue > m_endValue && currentValue < m_endValue));
}

double SpringAnimationDriver::ToValue() {
  return m_endValue;
}

} // namespace Microsoft::ReactNative

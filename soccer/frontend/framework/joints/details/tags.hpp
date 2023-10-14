#pragma once


namespace joints {
namespace details {

struct FromSensorsType {};
constexpr FromSensorsType fromSensors;

struct FromActuatorsType {};
constexpr FromActuatorsType fromActuators;

struct FromArrayType {};
constexpr FromArrayType fromArray;

struct ToActuatorsType {};
constexpr ToActuatorsType toActuators;

} // namespace details
} // namespace joints

#pragma once
#include <framework/rt/message_utils.h>

struct WhistleCommand {};

struct WhistleRecordOnly {};
RT_REGISTER_COMMAND(WhistleRecordOnly, WhistleCommand);

struct WhistleStart {};
RT_REGISTER_COMMAND(WhistleStart, WhistleCommand);

struct WhistleStop {};
RT_REGISTER_COMMAND(WhistleStop, WhistleCommand);

#pragma once

#include <framework/rt/message_utils.h>
#include <whistle_commands_generated.h>

RT_REGISTER_COMMAND(bbapi::WhistleRecordOnlyT, bbapi::WhistleCommandT);
RT_REGISTER_COMMAND(bbapi::WhistleStartT, bbapi::WhistleCommandT);
RT_REGISTER_COMMAND(bbapi::WhistleStopT, bbapi::WhistleCommandT);


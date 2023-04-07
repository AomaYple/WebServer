#include "Log.h"

using std::string_view, std::atomic_ref, std::source_location;

Log Log::log;

auto Log::add(source_location &sourceLocation, Level &level, string_view &data) -> void {
    log.addLog(sourceLocation, level, data);
}

auto Log::stopWork() -> void {
    log.stopWorkLog();
}

auto Log::addLog(source_location &sourceLocation, Level &level, string_view &data) -> void {
}

auto Log::stopWorkLog() -> void {
    atomic_ref<bool> atomicStop {this->stop};
    atomicStop = true;
}
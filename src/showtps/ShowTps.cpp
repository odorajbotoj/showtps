#include "showtps/showtps.h"

#include <memory>

#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"

namespace showtps {

static std::unique_ptr<ShowTps> instance;

ShowTps& ShowTps::getInstance() { return *instance; }

bool ShowTps::load() {
    getSelf().getLogger().info("Loading...");
    // Code for loading the plugin goes here.
    return true;
}

bool ShowTps::enable() {
    getSelf().getLogger().info("Enabling...");
    // Code for enabling the plugin goes here.
    return true;
}

bool ShowTps::disable() {
    getSelf().getLogger().info("Disabling...");
    // Code for disabling the plugin goes here.
    return true;
}

} // namespace showtps

LL_REGISTER_PLUGIN(showtps::ShowTps, showtps::instance);

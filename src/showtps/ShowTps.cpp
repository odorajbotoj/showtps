#include "showtps/showtps.h"

#include <chrono>
#include <format>
#include <memory>
#include <string>

#include "ll/api/plugin/NativePlugin.h"
#include "ll/api/plugin/RegisterHelper.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/data/KeyValueDB.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/Bedrock.h"

#include "mc/network/packet/TextPacket.h"
#include "mc/server/commands/CommandOrigin.h"
#include "mc/server/commands/CommandOutput.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/util/ProfilerLite.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

namespace {
std::unique_ptr<ll::data::KeyValueDB> playerDb;
ll::schedule::GameTickScheduler       mTickScheduler;
} // namespace

namespace showtps {

static std::unique_ptr<ShowTps> instance;

ShowTps& ShowTps::getInstance() { return *instance; }

bool ShowTps::load() {
    // open database
    const auto& playerDbPath = getSelf().getDataDir() / "players";
    ::playerDb               = std::make_unique<ll::data::KeyValueDB>(playerDbPath);

    return true;
}

bool ShowTps::enable() {
    // new schedule
    using ll::chrono_literals::operator""_tick;
    ::mTickScheduler.add<ll::schedule::RepeatTask>(20_tick, []() {
        auto level = ll::service::getLevel();
        if (level.has_value()) {
            level->forEachPlayer([](Player& player) -> bool {
                if ((!::playerDb->get(player.getUuid().asString()))
                    || (::playerDb->get(player.getUuid().asString()) == std::to_string(false))) {
                    return true;
                }
                double mspt       = (double)ProfilerLite::gProfilerLiteInstance.getServerTickTime().count() / 1000000.0;
                double currentTps = mspt <= 50 ? 20 : (double)(1000.0 / mspt);
                TextPacketType type = TextPacketType::Tip;
                TextPacket     pkt  = TextPacket();
                pkt.mType           = type;
                pkt.mMessage        = std::format(
                    "MSPT:{}{:.2f}§r TPS:{}{:.2f}§r",
                    mspt > 50 ? "§c" : "§a",
                    mspt,
                    currentTps < 20 ? "§c" : "§a",
                    currentTps
                );
                player.sendNetworkPacket(pkt);
                return true;
            });
        }
    });

    // reg command
    struct cmdArgs {
        bool show;
    };
    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("showtps", "ShowTps", CommandPermissionLevel::Any);
    cmd.overload<cmdArgs>().required("show").execute([](CommandOrigin const& origin, CommandOutput& output, cmdArgs args
                                                     ) {
        auto* entity = origin.getEntity();
        if (entity == nullptr || !entity->isType(ActorType::Player)) {
            output.error("Only players can do this action!");
            return;
        }
        auto* player = static_cast<Player*>(entity);
        if (!playerDb->set(player->getUuid().asString(), std::to_string(args.show))) {
            output.error("Error!");
            return;
        }
        output.success("Success.");
    });

    // return
    return true;
}

bool ShowTps::disable() {
    ::mTickScheduler.clear();
    return true;
}

} // namespace showtps

LL_REGISTER_PLUGIN(showtps::ShowTps, showtps::instance);

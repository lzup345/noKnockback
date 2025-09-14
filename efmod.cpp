//
// Created by lzup on 2025/8/26.
//

#include "efmod_core.hpp"
#include "TEFMod.hpp"
#include "BaseType.hpp"
#include "Logger.hpp"


// 全局组件
TEFMod::Logger* g_log;
TEFMod::TEFModAPI* g_api;


struct PlayerFields {
    TEFMod::Field<bool>* noKnockback;
} l_player;

// 原始函数声明
void (*original_ResetEffects)(TEFMod::TerrariaInstance);

// Hook转发函数
void ResetEffects_T(TEFMod::TerrariaInstance i);

// Hook模板定义
inline TEFMod::HookTemplate HookTemplate_ResetEffects {
    reinterpret_cast<void*>(ResetEffects_T),
    {}
};

// 转发函数实现
void ResetEffects_T(TEFMod::TerrariaInstance i) {
    original_ResetEffects(i);
    for (const auto fun : HookTemplate_ResetEffects.FunctionArray) {
        if (fun) reinterpret_cast<void(*)(TEFMod::TerrariaInstance)>(fun)(i);
    }
}

// 实际Hook逻辑
void Hook_ResetEffects(TEFMod::TerrariaInstance player) {
    l_player.noKnockback->Set(true,player);
}

class noKnockback final : public EFMod {
public:
    int Initialize(const std::string &path, MultiChannel *multiChannel) override {
        return 0;
    }

    int UnLoad(const std::string &path, MultiChannel *multiChannel) override {
        return 0;
    }

    int Load(const std::string &path, MultiChannel* channel) override {
        // 初始化日志和API
        g_log = channel->receive<TEFMod::Logger*(*)(const std::string&, const std::string&, const std::size_t)>(
            "TEFMod::CreateLogger")("noKnockback-lzup", "", 0);
            g_api = channel->receive<TEFMod::TEFModAPI*>("TEFMod::TEFModAPI");
            g_log->init();
            g_log->i("noKnockback已加载");
            return 0;
    }

    void Send(const std::string &path, MultiChannel* channel) override {
        // 注册Hook
        g_api->registerFunctionDescriptor({
            "Terraria",
            "Player",
            "ResetEffects",
            "hook>>void",
            0,
            &HookTemplate_ResetEffects,
            { reinterpret_cast<void*>(Hook_ResetEffects) }
        });

        // 注册需要使用的字段
        g_api->registerApiDescriptor({"Terraria", "Player", "noKnockback", "Field"});
    }

    void Receive(const std::string &path, MultiChannel* channel) override {
        // 获取字段解析器
        auto ParseBoolField = channel->receive<TEFMod::Field<bool>*(*)(void*)>(
            "TEFMod::Field<Bool>::ParseFromPointer");

        // 获取原始函数指针
        original_ResetEffects = g_api->GetAPI<void(*)(TEFMod::TerrariaInstance)>({
            "Terraria",
            "Player",
            "ResetEffects",
            "old_fun",
            0
        });

        // 初始化字段
        l_player.noKnockback = ParseBoolField(g_api->GetAPI<void*>({
            "Terraria",
            "Player",
            "noKnockback",
            "Field"
        }));
    }

    Metadata GetMetadata() override {
        return {
            "noKnockback",  // Mod名称
            "lzup",        // 作者
            "1.0.0",       // 版本
            20250826,      // 日期
            ModuleType::Game,
            { false }
        };
    }
};

EFMod* CreateMod() {
    static noKnockback instance;
    return &instance;
}

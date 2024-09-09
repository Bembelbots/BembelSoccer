#include "nao.h"
#include "modules/naothread.h"
#include "modules/imagethread.h"
#include "say/nao_say_backend.h"
#include <representations/blackboards/settings.h>
#include <framework/rt/kernel.h>
#include <memory>

struct NaoModules {
    std::unique_ptr<NaoThread> nao;
    std::unique_ptr<ImageThread> image;
    std::unique_ptr<XLogger> xlogsay;
    rt::Context<SettingsBlackboard> settings;
    NaoSayBackend* xlog_naosay;

    NaoModules() {
        nao = std::make_unique<NaoThread>();
        image = std::make_unique<ImageThread>();
    }

    void load(rt::Kernel &soccer) {
        soccer.hook("NaoModule", [&](rt::Linker &link) {
            link(settings);
        }, [&](){
            xlogsay = std::make_unique<XLogger>(LOGSAYID);
            xlog_naosay = new NaoSayBackend(LOGSAYID, settings->simulator);
            xlogsay->add_backend(xlog_naosay, "%%MSG%%", false);
        });
        soccer.load(nao.get());
        soccer.load(image.get());
    }
};

Nao::Nao() {
    modules = std::make_shared<NaoModules>();
}

void Nao::connect(rt::Linker &link) {
    link.name = "Nao";
}

void Nao::load(rt::Kernel &soccer) {
    modules->load(soccer);
}

void Nao::stop() {
    modules->xlogsay->stop();
}

void Nao::set_sensor_callback(SensorCallback callback) {
    modules->nao->set_sensor_callback(callback);
}

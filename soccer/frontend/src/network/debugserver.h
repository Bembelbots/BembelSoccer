#pragma once

#include <framework/network/udp.h>
#include <framework/network/tcp-server.h>
#include <framework/util/clock.h>
#include <framework/rt/module.h>
#include <representations/bembelbots/nao_info.h>
#include <memory>
#include <string>
#include <mutex>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <representations/vision/image.h>
#include <representations/bembelbots/types.h>
#include <representations/debugserver/debugstate.h>


class Config;
class SettingsBlackboard;

class DebugServer : public rt::Module {
public:
    DebugServer();
    virtual ~DebugServer();

    DebugServer(const DebugServer&) = delete;
    DebugServer& operator=(const DebugServer&) = delete;

    void connect(rt::Linker &) override;
    void setup() override;
    void stop() override;
    void process() override;

    void send(const boost::property_tree::ptree &pt,
              const boost::asio::ip::udp::endpoint &to);

    void send(const std::string &s, const boost::asio::ip::udp::endpoint &to);

    void send(const char *message, const size_t &size,
              const boost::asio::ip::udp::endpoint &to);

    void send(const boost::property_tree::ptree &pt);

    void send(const std::string &s);

    void send(const char *message, const size_t &size);

    void sendIMG(const char *message, const size_t &size);

    bool isSending();

private:
    static constexpr uint8_t PNG_LOW_COMPRESSION_LVL{2};
    static constexpr uint8_t JPEG_BAD_QUALITY{80};
    static constexpr uint8_t LQ_WIDTH{160};
    static constexpr uint8_t LQ_HEIGHT{120};

    std::mutex mtx;

    std::unique_ptr<TCPServer> _netTCP;
    std::unique_ptr<UDP> _netBcast;
    std::unique_ptr<UDP> _netUcast;
    std::shared_ptr<NetworkIO> _io;
    boost::asio::ip::udp::endpoint *_debug_client{nullptr};
    TimestampMs _lastAlive; ///< timestamp of last message received by receiver
    using buf_t = std::vector<uchar>;

    rt::Context<NaoInfo> nao;
    rt::Context<SettingsBlackboard> settings;
    rt::Context<Config, rt::Write> config;
    rt::Input<VisionImageProcessed, rt::Snoop> images;
    
    boost::asio::ip::udp::endpoint lastDebug;
    rt::Output<DebugState> debugState;

    void recv(const char *msg, const size_t &bytes_recvd,
              const udp::endpoint &sender);

    void connectClient(const boost::asio::ip::udp::endpoint &client);

    void disconnectClient();

    // Broadcasts all values to required symbol names.
    void broadcastSymbolValues();
    // Broadcasts symbol names.
    void broadcastSymbolNames();

    void broadcastImages();

    void updateDebugSymbols(boost::property_tree::ptree &pt,
                            const bool &change_value) const;
    std::vector<bool> getDebugValues();
    std::vector<std::string> getDebugSymbols();
    void setDebugSymbol(const std::string &name, const bool &value);
    buf_t encodeImage(const RgbImage &img, const ImageCodec &codec);
};

// vim: set ts=4 sw=4 sts=4 expandtab:

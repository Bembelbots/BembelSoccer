#include "debugserver.h"

#include "config.h"

#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <framework/logger/logger.h>
#include <boost/thread.hpp>
#include <thread>
#include <framework/blackboard/blackboard.h>
#include <framework/blackboard/blackboardregistry.h>
#include <representations/blackboards/settings.h>
#include <framework/math/directed_coord.h>
#include <representations/camera/camera.h>
#include <framework/image/rgb.h>
#include "debugheader.h"

using boost::asio::ip::udp;
using boost::asio::ip::tcp;
using boost::asio::ip::address_v4;
using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

DebugServer::DebugServer()
    : _lastAlive(0) {
    //TODO: move network io control to better place
    _io = std::make_shared<NetworkIO>();
    Network::set_network_io(_io);

    _netBcast = std::make_unique<UDP>(Network::DEBUG, &DebugServer::recv, this);
    _netUcast = std::make_unique<UDP>(Network::RANDOM, &DebugServer::recv, this);
    _netTCP = std::make_unique<TCPServer>(Network::DEBUG, false, 0, 10485760ULL);
}


DebugServer::~DebugServer() {
    _netBcast.reset();
    _netUcast.reset();
    Network::reset_network_io();
    _io->stop();
    disconnectClient();
}

void DebugServer::connect(rt::Linker &link) {
    link.name = "DebugServer";
    link(nao);
    link(settings);
    link(config);
    link(images);
    link(debugState);
}

void DebugServer::setup() {

}

void DebugServer::stop() {
    _io->stop();
}

void DebugServer::process() {
    microTime start = getMicroTime();
    broadcastSymbolValues();
    broadcastImages();
    {
        std::scoped_lock lock(mtx);
        debugState->debug_ep = lastDebug;
    }
    microTime us = (1e6 / (camera::fps + 5)) - (getMicroTime() - start);
    if (us > 0)
        std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void DebugServer::recv(const char *msg, const size_t &bytes_recvd,
                        const udp::endpoint &sender) {
    bool connected = isSending();
    bool authorized = false;
    if (connected && _debug_client && (sender == *_debug_client)) {
        authorized = true;
    }

    try {
        boost::this_thread::interruption_point();
        ptree pt;
        std::stringstream ss(std::string(msg, bytes_recvd));
        read_json(ss, pt);
        const std::string msg_type = pt.get<std::string>("debugv2.msg_type", "undefined");

        ptree response;
        response.clear();
        ptree debugV2;

        debugV2.put("connected", connected);

        if (connected && _debug_client) {
            std::stringstream ss;
            ss << *(_debug_client);
            debugV2.put("debug_client", ss.str());
        }

        debugV2.put("robotname", nao->getName());

        if (msg_type == "findnao") {
            debugV2.put("msg_type", "hellodebug");
            response.push_back(std::make_pair("debugv2", debugV2));
            send(response, sender);
            {
                std::scoped_lock lock(mtx);
                lastDebug = sender;
            }
        } else if (msg_type == "keepalive") {
            if (!connected || (connected && authorized)) {
                connectClient(sender);
            }
        } else if (msg_type == "change_symbol") {
            if (!connected || (connected && authorized)) {
                connectClient(sender);
                updateDebugSymbols(pt, false);
            }
        } else if (msg_type == "change_value") {
            if (!connected || (connected && authorized)) {
                connectClient(sender);
                updateDebugSymbols(pt, true);
            }
        } else if (msg_type == "groundtruth") {
            if (pt.get<std::string>("debugv2.robot", "") == nao->getName() ||
                    pt.get<std::string>("debugv2.robot", "") == nao->getHostname()) {
                auto p = pt.get_child("debugv2.position");
                if (p.size() != 3) {
                    LOG_ERROR << "received groundtruth data with invalid position array";
                    return;
                }
                // parsing json arrays with boost ptree sucks
                float x = p.front().second.get_value<float>();
                p.pop_front();
                float y = p.front().second.get_value<float>();
                p.pop_front();
                float a = p.front().second.get_value<float>();
                // parsing json arrays with boost ptree still sucks
                

                //auto pose = reinterpret_cast<PoseBlackboard*>(BlackboardRegistry::getBlackboardInstance("PoseData"));
                //if (pose) {
                //    pose->gtPosition = DirectedCoord(x, y, Rad{a});
                //    pose->gtTimestamp = getTimestampMs();
                //}
            }
        } else if (msg_type == "get_cabsl_graph") {
            //debugV2.put("msg_type", "cabsl_graph");
            //debugV2.push_back(std::make_pair("graph", BehaviorGraph::toPropertyTree()));
            //response.push_back(std::make_pair("debugv2", debugV2));
            //send(response, sender);
        } else if (msg_type == "save_blackboards") {
            LOG_INFO << "Saving blackboards.";
            LOG_SAY << "Saving blackboards.";
            bool saveOk = config->saveSettings();
            std::string answer =
                (saveOk) ? "save_blackboards_ok" : "save_blackboards_error";
            debugV2.put("msg_type", answer);
            response.push_back(std::make_pair("debugv2", debugV2));
            send(response, sender);
        }
    } catch (std::exception &e) {
        LOG_ERROR << "error: " << e.what();
        //DEBUG
        LOG_ERROR << "blackboardio: recv";
        //END DEBUG
    } catch (...) {
        LOG_ERROR << "handle_receive_from: unknown error";
    }
}

void DebugServer::broadcastSymbolValues() {
    if (!isSending()) {
        return;
    }

    // fetch debug symbols from all included blackboards
    std::vector<BlackboardDataContainer> allDebugSymbols;

    for (auto &blackboard : BlackboardRegistry::GetBlackboards()) {
        auto lock = blackboard->scopedLock();
        for (auto &debugValues : blackboard->_debugValues) {
            auto &[symbol, symbolVal] = debugValues;
            bool hasGetter = blackboard->_get_funcs.find(symbol) != blackboard->_get_funcs.end();
            if (*symbolVal && hasGetter) {
                allDebugSymbols.push_back(blackboard->_get_funcs.at(symbol)());
            }
        }
    }

    if (allDebugSymbols.size() != 0) {
        // convert to json data structure
        ptree pt;
        pt.put("debugv2.msg_type", "symbol_values");
        for (auto &symbol : allDebugSymbols) {
            pt.put("debugv2." + symbol.getKey(), symbol.getValue());
        }
        send(pt);
    }
}

void DebugServer::broadcastSymbolNames() {
    // stream symbols
    std::vector<std::string> symbols = getDebugSymbols();
    // stream values
    std::vector<bool> values = getDebugValues();

    if (values.size() != symbols.size()) {
        LOG_WARN << "symbols and values does not match!";
        return;
    }

    ptree pt;
    pt.put("debugv2.msg_type", "symbol_names");
    for (size_t i = 0; i < symbols.size(); ++i) {
        pt.put("debugv2." + symbols[i], values[i]);
    }

    send(pt);
}

void DebugServer::updateDebugSymbols(ptree &pt,
                                    const bool &change_value) const {

    for (auto &v: pt.get_child("debugv2")) {

        std::string bbName = v.first.data();

        if (bbName == "msg_type") {
            continue;
        }

        for (auto *bb : BlackboardRegistry::GetBlackboards()) {

            std::string name = bb->getBlackboardName();

            if (bbName == name) {

                std::vector<std::pair<std::string, std::string> > pairs;

                for (auto &v2: pt.get_child("debugv2." + bbName)) {
                    std::string key = v2.first.data();
                    std::string value = v2.second.data();
                    std::pair<std::string, std::string> n(key, value);
                    pairs.push_back(n);
                }

                if (change_value) {
                    for (auto &kv: pairs) {
                        if (bb->_set_funcs.find(kv.first) != bb->_set_funcs.end()) {
                            LOG_INFO << "Setting var: " << kv.first << " to: " << kv.second;
                            bb->_set_funcs.at(kv.first)(kv.second);
                        }
                    }
                } else {
                    bb->updateDebugSymbols(pairs);
                }

            }
        }
    }
}

void DebugServer::broadcastImages() {
    for(auto &img_context : images.fetch()) {
        /*
        if(!img_context.results.empty()) {
            LOG_DEBUG << "----";
            for(auto vr : img_context.results) {
                LOG_DEBUG << vr;
            }
        }
        */

        //LOG_INFO << "received image " << img_context.id;

        if (!isSending()) {
            img_context.release();
            continue;
        }

        //if (ic.fullRes)
        //    buf = encodeImage(ic.img.toRGB(), ic.codec);
        //else
        auto camera = img_context.image.camera;
        auto codec = ImageCodec::JPG;
        
        //auto image = encodeImage(img_context.image.toRGB(LQ_WIDTH, LQ_HEIGHT), codec);
        auto image = encodeImage(img_context.image.toRGB(), codec);
        size_t vrSize = img_context.results.size() * sizeof(VisionResult);
        size_t size = image.size() + sizeof(DebugImageHeader) + vrSize;
        
        char *data = new char[size];
        DebugImageHeader *dbgHdr = reinterpret_cast<DebugImageHeader *>(data);
        dbgHdr->version = DEBUG_IMAGE_VERSION;
        dbgHdr->camera = camera;
        dbgHdr->codec = codec;
        dbgHdr->timestamp = img_context.image.timestamp;
        dbgHdr->imageSize = image.size();
        dbgHdr->vrSize = vrSize; 
        
        char *offset = data;
        
        std::memcpy(offset, DEBUG_IMAGE_MAGIC, sizeof(dbgHdr->magic));
        offset += sizeof(DebugImageHeader);

        std::memcpy(offset, img_context.results.data(), dbgHdr->vrSize);
        offset += dbgHdr->vrSize;

        std::memcpy(offset, image.data(), dbgHdr->imageSize);

        img_context.release();

        sendIMG(data, size);
        delete[] data;
    }
}

std::vector<bool> DebugServer::getDebugValues() {
    std::vector<bool> values;

    auto &blackboards_list = BlackboardRegistry::GetBlackboards();
    for (std::set<BlackboardBase *>::iterator it = blackboards_list.begin();
            it != blackboards_list.end(); ++it) {
        auto lock = (*it)->scopedLock();
        std::vector<bool> tmpValues = (*it)->getDebugValues();
        values.insert(values.end(), tmpValues.begin(), tmpValues.end());
    }

    // need concat
    return values;
}

std::vector<std::string> DebugServer::getDebugSymbols() {

    // fetch all debug symbols
    std::vector<std::string> debugSymbols;

    auto &blackboards_list = BlackboardRegistry::GetBlackboards();
    for (std::set<BlackboardBase *>::iterator it = blackboards_list.begin();
            it != blackboards_list.end(); ++it) {
        auto lock = (*it)->scopedLock();
        std::vector<std::string> tmpSymbols = (*it)->getDebugSymbols();
        // std::cout << "got " << tmpSymbols.size() << " symbols from " <<
        // (*it)->getBlackboardName() << std::endl;
        for (size_t i = 0; i < tmpSymbols.size(); ++i)
            debugSymbols.push_back((*it)->getBlackboardName() + std::string(".") +
                                   tmpSymbols[i]);
    }

    // return everything
    return debugSymbols;
}

void DebugServer::setDebugSymbol(const std::string &name, const bool &value) {
    size_t pos = name.find(".");
    std::string board = name.substr(0, pos);
    std::string key = name.substr(pos + 1);

    auto &blackboards_list = BlackboardRegistry::GetBlackboards();
    for (std::set<BlackboardBase *>::iterator it = blackboards_list.begin();
            it != blackboards_list.end(); ++it) {
        if (board == (*it)->getBlackboardName()) {
            auto lock = (*it)->scopedLock();
            (*it)->setDebugSymbol(key, value);
        } else {
            LOG_WARN << "can't locate blackboard with name " << board;
        }
    }
}

bool DebugServer::isSending() {
    if (_debug_client && ((getTimestampMs() - _lastAlive) < 5000)) {
        return true;
    } else {
        disconnectClient();
        return false;
    }
}


void DebugServer::send(const boost::property_tree::ptree &pt) {
    if (_debug_client) {
        send(pt, *_debug_client);
    }
}


void DebugServer::send(const boost::property_tree::ptree &pt,
                        const udp::endpoint &to) {
    std::ostringstream buf;
    try {
        write_json(buf, pt, false);
    } catch (std::exception &e) {
        LOG_ERROR << "send error: ", e.what();
    } catch (...) {
        LOG_ERROR << "send: unknown error";
    }
    send(buf.str(), to);
}


void DebugServer::send(const std::string &s) {
    if (_debug_client) {
        send(s, *_debug_client);
    }
}


void DebugServer::send(const std::string &s, const udp::endpoint &to) {
    send(s.c_str(), s.size(), to);
}


void DebugServer::send(const char *message, const size_t &size) {
    if (_debug_client) {
        send(message, size, *_debug_client);
    }
}


void DebugServer::send(const char *message, const size_t &size,
                        const udp::endpoint &to) {
    _netUcast->sendTo(message, size, to);
}


void DebugServer::sendIMG(const char *message, const size_t &size) {
    if (!isSending()) {
        return;
    }

    _netTCP->write(message, size);
}


void DebugServer::connectClient(const boost::asio::ip::udp::endpoint &client) {
    if (_debug_client && (*_debug_client != client)) {
        delete _debug_client;
        _debug_client = nullptr;
    }

    if (!_debug_client) {
        _debug_client = new udp::endpoint(client);
        broadcastSymbolNames();
    }

    _lastAlive = getTimestampMs();
}


void DebugServer::disconnectClient() {
    if (!_debug_client) {
        return;
    }

    delete _debug_client;
    _debug_client = nullptr;
    _lastAlive = 0;
}

DebugServer::buf_t DebugServer::encodeImage(const RgbImage &img, const ImageCodec &codec) {
    std::string ext;
    std::vector<int> parms;

    if (codec == ImageCodec::PNG) {
        ext = ".png";
        parms = {cv::IMWRITE_PNG_COMPRESSION, PNG_LOW_COMPRESSION_LVL};
    } else {
        ext = ".jpg";
        parms = {cv::IMWRITE_JPEG_QUALITY, JPEG_BAD_QUALITY};
    }

    buf_t buf;
    cv::imencode(ext, img.mat(), buf, parms);

    return buf;
}


// vim: set ts=4 sw=4 sts=4 expandtab:

// SPDX-License-Identifier: MIT
#include "peer/peer.h"
#include "utility/address.h"
#include <openssl/sha.h>
#include <spdlog/spdlog.h>

namespace Candy {

void PeerInfo::reset() {
    updateState(PeerState::INIT);
    this->tun = 0;
    this->ip = 0;
    this->port = 0;
    this->count = 0;
    this->key.clear();
}

int PeerInfo::updateKey(const std::string &password) {
    if (!this->tun) {
        spdlog::error("tun ip is emtpy, cannot update key");
        return -1;
    }
    std::string data;
    data.append(password);
    data.append((char *)&this->tun, sizeof(this->tun));
    this->key.resize(SHA256_DIGEST_LENGTH);
    SHA256((unsigned char *)data.data(), data.size(), (unsigned char *)this->key.data());
    return 0;
}

std::string PeerInfo::getKey() const {
    return this->key;
}

void PeerInfo::updateState(PeerState state) {
    if (this->state != state) {
        spdlog::info("conn state: {} {} => {}", Address::ipToStr(this->tun), getStateStr(this->state), getStateStr(state));
        this->state = state;
    }
}

PeerState PeerInfo::getState() const {
    return this->state;
}

std::string PeerInfo::getStateStr(PeerState state) {
    switch (state) {
    case PeerState::INIT:
        return "INIT";
    case PeerState::PERPARING:
        return "PERPARING";
    case PeerState::SYNCHRONIZING:
        return "SYNCHRONIZING";
    case PeerState::CONNECTING:
        return "CONNECTING";
    case PeerState::CONNECTED:
        return "CONNECTED";
    case PeerState::FAILED:
        return "FAILED";
    default:
        return "UNKNOWN";
    }
}

}; // namespace Candy

//
// Created by Sol Harter on 27/08/2025.
//

#ifndef AUDIO_NEURO_MOD_LCL_CONNECTOR_H
#define AUDIO_NEURO_MOD_LCL_CONNECTOR_H

#pragma once
#include <lsl_cpp.h>
#include <juce_core/juce_core.h>
#include <iostream>

class LslConnector
{
public:
    enum class Status { idle, connected, error };

    bool connectFirstEEG (double timeoutSec = 1.5)
    {
        try {
            auto v = lsl::resolve_stream("type", "EEG", 1, timeoutSec);
            if (v.empty()) { std::cout << "No EEG streams found\n"; return false; }
            return connect (v.front());
        } catch (...) {
            status_ = Status::error;
            std::cout << "Resolve failed\n";
            return false;
        }
    }

    bool connect (const lsl::stream_info& info)
    {
        try {
            disconnect();

            streamInfo_ = info;
            hasStreamInfo_ = true;

            inlet_ = std::make_unique<lsl::stream_inlet>(info, /*maxbuflen*/ 360, /*recover*/ true);
            inlet_->set_postprocessing (lsl::post_ALL);

            status_ = Status::connected;

            std::cout << "Connected to LSL stream: "
                      << info.name() << " [" << info.type() << "] "
                      << info.channel_count() << "ch @ " << info.nominal_srate() << " Hz\n";

            return true;
        } catch (...) {
            status_ = Status::error;
            std::cout << "Connect failed\n";
            inlet_.reset();
            return false;
        }
    }

    void disconnect()
    {
        if (inlet_) {
            try { inlet_->close_stream(); } catch (...) {}
            inlet_.reset();
        }
        status_ = Status::idle;
        hasStreamInfo_ = false;
        std::cout << "Disconnected LSL\n";
    }

    [[nodiscard]] double getSampleRate() const {
        return hasStreamInfo_ ? streamInfo_.nominal_srate() : 0.0;
    }

    [[nodiscard]] int getChannelCount() const {
        return hasStreamInfo_ ? streamInfo_.channel_count() : 0;
    }

    [[nodiscard]] std::string getStreamName() const {
        return hasStreamInfo_ ? streamInfo_.name() : "";
    }

    [[nodiscard]] Status status() const { return status_; }
    [[nodiscard]] bool isConnected() const { return status_ == Status::connected; }

    [[nodiscard]] lsl::stream_inlet* inlet() const { return inlet_.get(); }

private:
    std::unique_ptr<lsl::stream_inlet> inlet_;
    lsl::stream_info streamInfo_;
    bool hasStreamInfo_ = false;
    Status status_ { Status::idle };
};

#endif //AUDIO_NEURO_MOD_LCL_CONNECTOR_H
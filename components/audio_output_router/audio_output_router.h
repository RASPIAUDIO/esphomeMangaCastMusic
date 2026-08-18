#pragma once

#ifdef USE_ESP32

#include <atomic>
#include <string>

#include "esphome/components/sendspin/sendspin_hub.h"
#include "esphome/components/speaker/speaker.h"
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <sendspin/metadata_role.h>

namespace esphome {
namespace audio_output_router {

class AudioOutputRouter : public Component, public speaker::Speaker {
 public:
  float get_setup_priority() const override { return setup_priority::DATA; }

  void set_analog_speaker(speaker::Speaker *speaker) { this->analog_speaker_ = speaker; }
  void set_spdif_speaker(speaker::Speaker *speaker) { this->spdif_speaker_ = speaker; }
  void set_sendspin_hub(sendspin_::SendspinHub *hub) { this->sendspin_hub_ = hub; }

  void setup() override {
    // Le switch template restaure son état avant les composants audio.
    this->active_speaker_.store(this->spdif_enabled_ ? this->spdif_speaker_ : this->analog_speaker_);
    this->attach_callback_(this->analog_speaker_);
    this->attach_callback_(this->spdif_speaker_);
    if (this->sendspin_hub_ != nullptr) {
      this->sendspin_hub_->add_metadata_update_callback(
          [this](const sendspin::ServerMetadataStateObject &metadata) {
            this->title_ = metadata.title.value_or("");
            this->artist_ = metadata.artist.value_or("");
            this->album_ = metadata.album.value_or("");
          });
    }
  }

  void dump_config() override {
    ESP_LOGCONFIG("audio_output_router", "Audio output router:");
    ESP_LOGCONFIG("audio_output_router", "  Active output: %s", this->spdif_enabled_ ? "SPDIF" : "analog");
  }

  void loop() override {
    auto *active = this->active_speaker_.load();
    if (active == nullptr) return;

    if (this->state_ == speaker::STATE_STARTING && active->is_running()) {
      this->state_ = speaker::STATE_RUNNING;
    } else if (this->state_ == speaker::STATE_STOPPING && active->is_stopped()) {
      this->state_ = speaker::STATE_STOPPED;
    }
  }

  void set_spdif_enabled(bool enabled) {
    if (enabled == this->spdif_enabled_) return;

    auto *old_output = this->active_speaker_.load();
    auto *new_output = enabled ? this->spdif_speaker_ : this->analog_speaker_;
    if (new_output == nullptr) return;

    const bool restart = this->state_ == speaker::STATE_STARTING || this->state_ == speaker::STATE_RUNNING;
    if (old_output != nullptr) old_output->stop();

    this->spdif_enabled_ = enabled;
    new_output->set_audio_stream_info(this->audio_stream_info_);
    new_output->set_volume(this->volume_);
    new_output->set_mute_state(this->mute_state_);
    new_output->set_pause_state(this->pause_state_);
    this->active_speaker_.store(new_output);

    if (restart) {
      this->state_ = speaker::STATE_STARTING;
      new_output->start();
    } else {
      this->state_ = speaker::STATE_STOPPED;
    }

    ESP_LOGI("audio_output_router", "Audio output switched to %s", enabled ? "SPDIF" : "analog");
  }

  bool is_spdif_enabled() const { return this->spdif_enabled_; }

  const std::string &get_title() const { return this->title_; }
  const std::string &get_artist() const { return this->artist_; }
  const std::string &get_album() const { return this->album_; }

  void clear_metadata() {
    this->title_.clear();
    this->artist_.clear();
    this->album_.clear();
  }

  size_t play(const uint8_t *data, size_t length, TickType_t ticks_to_wait) override {
    if (this->is_stopped()) this->start();
    auto *active = this->active_speaker_.load();
    return active == nullptr ? 0 : active->play(data, length, ticks_to_wait);
  }

  size_t play(const uint8_t *data, size_t length) override { return this->play(data, length, 0); }

  void start() override {
    auto *active = this->active_speaker_.load();
    if (active == nullptr || this->state_ == speaker::STATE_STARTING || this->state_ == speaker::STATE_RUNNING) return;
    active->set_audio_stream_info(this->audio_stream_info_);
    active->set_volume(this->volume_);
    active->set_mute_state(this->mute_state_);
    active->set_pause_state(this->pause_state_);
    this->state_ = speaker::STATE_STARTING;
    active->start();
  }

  void stop() override {
    auto *active = this->active_speaker_.load();
    if (active == nullptr || this->state_ == speaker::STATE_STOPPED) return;
    this->state_ = speaker::STATE_STOPPING;
    active->stop();
  }

  void finish() override {
    auto *active = this->active_speaker_.load();
    if (active == nullptr || this->state_ == speaker::STATE_STOPPED) return;
    this->state_ = speaker::STATE_STOPPING;
    active->finish();
  }

  bool has_buffered_data() const override {
    auto *active = this->active_speaker_.load();
    return active != nullptr && active->has_buffered_data();
  }

  void set_pause_state(bool paused) override {
    this->pause_state_ = paused;
    auto *active = this->active_speaker_.load();
    if (active != nullptr) active->set_pause_state(paused);
  }

  bool get_pause_state() const override { return this->pause_state_; }

  void set_volume(float volume) override {
    this->volume_ = volume;
    auto *active = this->active_speaker_.load();
    if (active != nullptr) active->set_volume(volume);
  }

  void set_mute_state(bool muted) override {
    this->mute_state_ = muted;
    auto *active = this->active_speaker_.load();
    if (active != nullptr) active->set_mute_state(muted);
  }

 protected:
  void attach_callback_(speaker::Speaker *output) {
    if (output == nullptr) return;
    output->add_audio_output_callback(
        [this, output](uint32_t frames_played, int64_t timestamp) {
          if (this->active_speaker_.load() == output) {
            this->audio_output_callback_(frames_played, timestamp);
          }
        });
  }

  speaker::Speaker *analog_speaker_{nullptr};
  speaker::Speaker *spdif_speaker_{nullptr};
  sendspin_::SendspinHub *sendspin_hub_{nullptr};
  std::atomic<speaker::Speaker *> active_speaker_{nullptr};
  bool spdif_enabled_{false};
  bool pause_state_{false};
  std::string title_;
  std::string artist_;
  std::string album_;
};

}  // namespace audio_output_router
}  // namespace esphome

#endif  // USE_ESP32

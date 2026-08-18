import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import audio, speaker
from esphome.components.sendspin import (
    CONF_SENDSPIN_ID,
    SendspinHub,
    request_metadata_support,
)
from esphome.const import CONF_ID

AUTO_LOAD = ["audio"]
CODEOWNERS = ["@RASPIAUDIO"]
DEPENDENCIES = ["sendspin"]

CONF_ANALOG_SPEAKER = "analog_speaker"
CONF_SPDIF_SPEAKER = "spdif_speaker"

audio_output_router_ns = cg.esphome_ns.namespace("audio_output_router")
AudioOutputRouter = audio_output_router_ns.class_(
    "AudioOutputRouter", cg.Component, speaker.Speaker
)


def _set_stream_limits(config):
    audio.set_stream_limits(
        min_bits_per_sample=8,
        max_bits_per_sample=16,
        min_channels=1,
        max_channels=2,
        min_sample_rate=16000,
        max_sample_rate=48000,
    )(config)
    return config


CONFIG_SCHEMA = cv.All(
    speaker.SPEAKER_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(AudioOutputRouter),
            cv.Required(CONF_ANALOG_SPEAKER): cv.use_id(speaker.Speaker),
            cv.Required(CONF_SPDIF_SPEAKER): cv.use_id(speaker.Speaker),
            cv.GenerateID(CONF_SENDSPIN_ID): cv.use_id(SendspinHub),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    _set_stream_limits,
)


async def to_code(config):
    request_metadata_support()
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await speaker.register_speaker(var, config)

    analog = await cg.get_variable(config[CONF_ANALOG_SPEAKER])
    spdif = await cg.get_variable(config[CONF_SPDIF_SPEAKER])
    cg.add(var.set_analog_speaker(analog))
    cg.add(var.set_spdif_speaker(spdif))

    sendspin_hub = await cg.get_variable(config[CONF_SENDSPIN_ID])
    cg.add(var.set_sendspin_hub(sendspin_hub))

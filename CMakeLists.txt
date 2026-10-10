cmake_minimum_required(VERSION 3.22)
project(DonswayCleanTake VERSION 0.1.0)

# Set JUCE_DIR to your JUCE checkout, or install JUCE and point CMake at it.
add_subdirectory("${JUCE_DIR}" JUCE)

juce_add_plugin(DonswayCleanTake
    COMPANY_NAME "Donsway ENT"
    BUNDLE_ID "com.donswayent.cleantake"
    PLUGIN_MANUFACTURER_CODE Dnwy
    PLUGIN_CODE ClTk
    FORMATS VST3 Standalone
    PRODUCT_NAME "Donsway CleanTake"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(DonswayCleanTake PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_compile_features(DonswayCleanTake PRIVATE cxx_std_17)
target_compile_definitions(DonswayCleanTake PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(DonswayCleanTake PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

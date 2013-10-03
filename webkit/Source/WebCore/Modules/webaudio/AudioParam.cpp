/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if ENABLE(WEB_AUDIO)

#include "AudioParam.h"

#include "AudioNode.h"
#include "AudioNodeOutput.h"
#include "AudioUtilities.h"
#include "FloatConversion.h"
#include <wtf/MathExtras.h>

namespace WebCore {

const double AudioParam::DefaultSmoothingConstant = 0.05;
const double AudioParam::SnapThreshold = 0.001;

float AudioParam::value()
{
    // Update value for timeline.
    if (context() && context()->isAudioThread()) {
        bool hasValue;
        float timelineValue = m_timeline.valueForContextTime(context(), narrowPrecisionToFloat(m_value), hasValue);

        if (hasValue)
            m_value = timelineValue;
    }

    return narrowPrecisionToFloat(m_value);
}

void AudioParam::setValue(float value)
{
    // Check against JavaScript giving us bogus floating-point values.
    // Don't ASSERT, since this can happen if somebody writes bad JS.
    if (!isnan(value) && !isinf(value))
        m_value = value;
}

float AudioParam::smoothedValue()
{
    return narrowPrecisionToFloat(m_smoothedValue);
}

bool AudioParam::smooth()
{
    // If values have been explicitly scheduled on the timeline, then use the exact value.
    // Smoothing effectively is performed by the timeline.
    bool useTimelineValue = false;
    if (context())
        m_value = m_timeline.valueForContextTime(context(), narrowPrecisionToFloat(m_value), useTimelineValue);
    
    if (m_smoothedValue == m_value) {
        // Smoothed value has already approached and snapped to value.
        return true;
    }
    
    if (useTimelineValue)
        m_smoothedValue = m_value;
    else {
        // Dezipper - exponential approach.
        m_smoothedValue += (m_value - m_smoothedValue) * m_smoothingConstant;

        // If we get close enough then snap to actual value.
        if (fabs(m_smoothedValue - m_value) < SnapThreshold) // FIXME: the threshold needs to be adjustable depending on range - but this is OK general purpose value.
            m_smoothedValue = m_value;
    }

    return false;
}

void AudioParam::calculateSampleAccurateValues(float* values, unsigned numberOfValues)
{
    bool isSafe = context() && context()->isAudioThread() && values && numberOfValues;
    ASSERT(isSafe);
    if (!isSafe)
        return;

    if (m_audioRateSignal)
        calculateAudioRateSignalValues(values, numberOfValues);
    else
        calculateTimelineValues(values, numberOfValues);
}

void AudioParam::calculateAudioRateSignalValues(float* values, unsigned numberOfValues)
{
    // FIXME: support fan-in (multiple audio connections to this parameter with unity-gain summing).
    // https://bugs.webkit.org/show_bug.cgi?id=83610
    ASSERT(m_audioRateSignal);

    AudioBus* bus = m_audioRateSignal->pull(0, numberOfValues);
    bool isBusGood = bus && bus->numberOfChannels() && bus->length() >= numberOfValues;
    ASSERT(isBusGood);
    if (!isBusGood)
        return;

    if (bus->numberOfChannels() == 1) {
        // The normal case is to deal with a mono audio-rate signal.
        memcpy(values, bus->channel(0)->data(), sizeof(float) * numberOfValues);
    } else {
        // Do a standard mixdown to one channel if necessary.
        AudioBus wrapperBus(1, numberOfValues, false);
        wrapperBus.setChannelMemory(0, values, numberOfValues);
        wrapperBus.copyFrom(*bus); // Mixdown.
    }
    m_value = values[0]; // Update to first value.
}

void AudioParam::calculateTimelineValues(float* values, unsigned numberOfValues)
{
    // Calculate values for this render quantum.
    // Normally numberOfValues will equal AudioNode::ProcessingSizeInFrames (the render quantum size).
    float sampleRate = context()->sampleRate();
    float startTime = narrowPrecisionToFloat(context()->currentTime());
    float endTime = startTime + numberOfValues / sampleRate;

    // Note we're running control rate at the sample-rate.
    // Pass in the current value as default value.
    m_value = m_timeline.valuesForTimeRange(startTime, endTime, narrowPrecisionToFloat(m_value), values, numberOfValues, sampleRate, sampleRate);
}

void AudioParam::connect(AudioNodeOutput* audioRateSignal)
{
    ASSERT(context()->isGraphOwner());
    ASSERT(audioRateSignal);
    if (!audioRateSignal)
        return;

    if (m_audioRateSignal && m_audioRateSignal != audioRateSignal) {
        // Because we don't currently support fan-in we must explicitly disconnect from an old output.
        m_audioRateSignal->removeParam(this);
    }

    audioRateSignal->addParam(this);
    m_audioRateSignal = audioRateSignal;
}

void AudioParam::disconnect(AudioNodeOutput* audioRateSignal)
{
    ASSERT(context()->isGraphOwner());
    ASSERT(audioRateSignal);
    if (!audioRateSignal)
        return;

    // FIXME: support fan-in (multiple audio connections to this parameter with unity-gain summing).
    // https://bugs.webkit.org/show_bug.cgi?id=83610
    if (m_audioRateSignal == audioRateSignal)
        m_audioRateSignal = 0;
}

} // namespace WebCore

#endif // ENABLE(WEB_AUDIO)

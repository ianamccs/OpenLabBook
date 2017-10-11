/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2016 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef MovementDetector_H_INCLUDED
#define MovementDetector_H_INCLUDED


#ifdef _WIN32
#include <Windows.h>
#endif

#include <ProcessorHeaders.h>

/**

    This is a simple filter that subtracts the average of all other channels from 
    each channel. The gain parameter allows you to subtract a percentage of the total avg.

    See Ludwig et al. 2009 Using a common average reference (resting) to improve cortical
    neuron recordings from microelectrode arrays. J. Neurophys, 2009 for a detailed
    discussion


*/

class MovementDetector : public GenericProcessor
{
public:
    /** The class constructor, used to initialize any members. */
    MovementDetector();

    /** The class destructor, used to deallocate memory */
    ~MovementDetector();

    /** Determines whether the processor is treated as a source. */
    bool isSource() override { return false; }

    /** Determines whether the processor is treated as a sink. */
    bool isSink()   override { return false; }

    /** Defines the functionality of the processor.

        The process method is called every time a new data buffer is available.

        Processors can either use this method to add new data, manipulate existing
        data, or send data to an external target (such as a display or other hardware).

        Continuous signals arrive in the "buffer" variable, event data (such as TTLs
        and spikes) is contained in the "events" variable, and "nSamples" holds the
        number of continous samples in the current buffer (which may differ from the
        size of the buffer).
         */

	bool hasEditor() const { return true; }

    void process (AudioSampleBuffer& buffer, MidiBuffer& events) override;

    /** Returns the current gain level that is set in the processor */
    float getGainLevel();

    /** Sets the new gain level that will be used in the processor */
    void setGainLevel (float newGain);

    /** Creates the MovementDetectorEditor. */
    AudioProcessorEditor* createEditor() override;

	void setParameter(int parameterIndex, float newValue);

    Array<int> getRestingChannels() const     { return m_RestingChannels; }
    Array<int> getMovingChannels()  const     { return m_MovingChannels; }

    void setRestingChannels (const Array<int>& newRestingChannels);
    void setMovingChannels  (const Array<int>& newMovingChannels);

    void setRestingChannelState (int channel, bool newState);
    void setMovingChannelState  (int channel, bool newState);

    void addModule();

private:
    LinearSmoothedValueAtomic<float> m_gainLevel;

    AudioSampleBuffer m_avgBuffer;

    /** We should add this for safety to prevent any app crashes or invalid data processing.
        Since we use m_RestingChannels and m_MovingChannels arrays in the process() function,
        which works in audioThread, we may stumble upon the situation when we start changing
        either reference (resting) or affected (moving) channels by copying array and in the middle of copying process
        we will be interrupted by audioThread. So it most probably will lead to app crash or
        processing incorrect channels.
    */
    CriticalSection objectLock;

    /** Array of channels which will be used to calculate mean signal. */
    Array<int> m_RestingChannels;

    /** Array of channels that will be affected (moving) by adding/substracting of mean signal of reference (resting) channels */
    Array<int> m_MovingChannels;

	Array<float> initialWindow;
    Array<float> ampWindow;
    Array<float> sdWindow;
    float initialMean = 0;
    float initialRMS = 0;
    float standardDev = 0;
	float threshold = 2.4;
	int bufferSize = 0;
    int timeThreshold = 1800000; //6s //1 minuto
    int overlap = 50;
    int k = 0;
    int initialTimeWindow = 60*30000;
    int overlapTime = (60-overlap)*30000;
    int waitTime = overlapTime;
    int contador = 0;

    bool hasInitialStats = false;
    bool awake = true;
	bool resting = true; //event of interest

    void calculateBufferSize(AudioSampleBuffer& buffer);
    void calculateInitialStats(AudioSampleBuffer& buffer);
	void classifier(AudioSampleBuffer& buffer, MidiBuffer& events);

    // ==================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MovementDetector);
};



#endif  // MovementDetector_H_INCLUDED


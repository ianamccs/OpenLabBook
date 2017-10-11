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

#include <stdio.h>
#include <math.h>

#include "MovementDetector.h"
#include "MovementDetectorEditor.h"

using namespace std;

MovementDetector::MovementDetector()
    : GenericProcessor ("Movement Detector") //, threshold(200.0), state(true)
{
    m_avgBuffer = AudioSampleBuffer (1, 10000); // 1-dimensional buffer to hold the avg
}

MovementDetector::~MovementDetector()
{

}

AudioProcessorEditor* MovementDetector::createEditor()
{
    editor = new MovementDetectorEditor (this, true);
    return editor;
}

void MovementDetector::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);
}

float MovementDetector::getGainLevel()
{
    m_gainLevel.updateTarget();
    return m_gainLevel.getNextValue();
}


void MovementDetector::setGainLevel (float newGain)
{
    m_gainLevel.setValue (newGain);
	
	threshold = newGain;

//	cout << "---------------------------------------------" << threshold << endl;

}


void MovementDetector::process (AudioSampleBuffer& buffer, MidiBuffer& events)
{
    const int numSamples            = buffer.getNumSamples();
    const int numRestingChannels  = m_RestingChannels.size();
    const int numMovingChannels   = m_MovingChannels.size();

    // There are no sense to do any processing if either number of reference (resting) or affected (moving) channels is zero.
    if (! numRestingChannels
        || ! numMovingChannels)
    {
        return;
    }

    m_avgBuffer.clear();

    for (int i = 0; i < numRestingChannels; ++i)
    {
        m_avgBuffer.addFrom (0,                         // destChannel
                             0,                         // destStartSample
                             buffer,                    // source
                             m_RestingChannels[i],    // sourceChannel
                             0,                         // sourceStartSample
                             numSamples,                // numSamples
                             1.0f);                     // gain to apply
    }

    m_avgBuffer.applyGain (1.0f / float (numRestingChannels));

    m_gainLevel.updateTarget();
    const float gain = -1.0f * m_gainLevel.getNextValue() / 50.f;

    for (int i = 0; i < numMovingChannels; ++i)
    {
        buffer.addFrom (m_MovingChannels[i],  // destChannel
                        0,                      // destStartSample
                        m_avgBuffer,            // source
                        0,                      // sourceChannel
                        0,                      // sourceStartSample
                        numSamples,             // numSamples
                        gain);                  // gain to apply
    }
//---------------------------------------------------Iana's
	if(bufferSize == 0) 
	{
		calculateBufferSize(buffer);
	}

	if(hasInitialStats == false)
	{
		calculateInitialStats(buffer);
	}

	else
	{
		classifier(buffer, events);
	}
}


void MovementDetector::setRestingChannels (const Array<int>& newRestingChannels)
{
    const ScopedLock myScopedLock (objectLock);

    m_RestingChannels = Array<int> (newRestingChannels);
}


void MovementDetector::setMovingChannels (const Array<int>& newMovingChannels)
{
    const ScopedLock myScopedLock (objectLock);

    m_MovingChannels = Array<int> (newMovingChannels);
}


void MovementDetector::setRestingChannelState (int channel, bool newState)
{
    if (! newState)
        m_RestingChannels.removeFirstMatchingValue (channel);
    else
        m_RestingChannels.addIfNotAlreadyThere (channel);
}


void MovementDetector::setMovingChannelState (int channel, bool newState)
{
    if (! newState)
        m_MovingChannels.removeFirstMatchingValue (channel);
    else
        m_MovingChannels.add (channel);
}

void MovementDetector::addModule()
{/*
  cout << "--------------------------------------------d--------------------------------------------" << endl;
    DetectorModule m = DetectorModule();
    m.inputChan = -1;
    m.outputChan = -1;
    m.gateChan = -1;
    m.isActive = true;
    m.lastSample = 0.0;
    m.samplesSinceTrigger = 5000;
    m.wasTriggered = false;
    m.MED = 0.00;
    m.STD = 0.00;
    m.AvgCount = 0;
    m.flag = 0;
    m.tReft = 0.0;
    m.count = 0;
    
    
    modules.add(m);
*/
}

void MovementDetector::calculateBufferSize(AudioSampleBuffer& buffer) 
{
	int numSamples = (int) buffer.getNumSamples();
	float currentSample;

	for(int i = 0; i < numSamples; i++)
	{
		currentSample = (float) buffer.getSample(0,i);

		if(currentSample > 0) 
		{
			bufferSize = i + 1;
		}
	} 

	cout << "\n Tamanho do buffer: " << bufferSize << "\n";
}

void MovementDetector::calculateInitialStats(AudioSampleBuffer& buffer)
{
	for(int i = 0; i < bufferSize; i++)
	{
		float currentSample = (float) buffer.getSample(0,i);

		// Fill an array with initial 10s of data
		if((initialWindow.size() == (initialTimeWindow-1)) && (hasInitialStats == false))
		{
			hasInitialStats = true;

			// add the last term
			initialWindow.add(currentSample);

			initialMean += currentSample;

            //Compute the mean of 2s of data
			initialMean = (initialMean/initialTimeWindow);

			// Compute the the SD for 10s of data
			for(int j = 0; j < initialTimeWindow; j++)
			{
				standardDev += pow((initialWindow[j] - initialMean),2);
			}
			standardDev = sqrt(standardDev/initialTimeWindow);

            sdWindow = initialWindow; // copia o primeiro minuto em sdWindow

            cout<<"\n Desvio padrao: "<<standardDev<<"\n";
            cout<<"\n Media: "<<initialMean<<"\n";
			cout<<"\n sdWindow_size: "<<sdWindow.size()<<"\n";
		}

		else if((hasInitialStats == false) && (initialWindow.size() < (initialTimeWindow-1)))
		{
			initialWindow.add(currentSample);
			initialMean += currentSample;
		}
	}
}

void MovementDetector::classifier(AudioSampleBuffer& buffer,
									MidiBuffer& events)
{
	for(int i = 0; i < bufferSize; i++)
	{	
		float currentSample = (float) buffer.getSample(0,i);

     	if(waitTime > 0)
       	{
        	sdWindow.set(k, currentSample);
        	k++;
        	if(k == timeThreshold) { k = 0; }
        	waitTime--;
       	}        	
       	else
       	{
       		int len = sdWindow.size(); // timeThreshold seconds =1min
       		float sumAmp = 0;
       		float winDev = 0;

       		for(int j = 0; j < len; j++)
       		{
       			sumAmp += sdWindow[j];
       		}

    		float winMean = (sumAmp/len);

    		for(int k = 0; k < len; k++)
    		{
    			winDev += pow((sdWindow[k] - winMean),2);
    		}

    		winDev = sqrt(winDev/len);
    		
    		cout<<"----------\n";
    		cout<<"std inicial: " << standardDev;
    		cout<<"\nstd janela: " << winDev;

            if(winDev <= threshold*standardDev)
    		{
    			cout << "\ndormindo \n\n";

    			if(awake)
    			{
    				awake = false;
    				addEvent(events, TTL, 0, resting, 0);
    			}
    		}
    		else
    		{
    			cout << "\nacordado \n\n";

    			if(!awake) 
    			{
    				awake = true;
    				addEvent(events, TTL, 0, !resting, 0);
    			}
    		}

    		waitTime = overlapTime;
    	}
	}
}


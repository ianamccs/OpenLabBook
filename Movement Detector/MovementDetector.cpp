/*
    ------------------------------------------------------------------
    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys
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
#include "MovementDetector.h"
#include "MovementDetectorEditor.h"
#include <time.h>
#include <math.h>

//If the processor uses a custom editor, it needs its header to instantiate it
//#include "ExampleEditor.h"

MovementDetector::MovementDetector()
    : GenericProcessor("Movement Detector") //, threshold(200.0), state(true)

{
	fout.open("std.txt");
	//Without a custom editor, generic parameter controls can be added
    //parameters.add(Parameter("thresh", 0.0, 500.0, 200.0, 0));

}

MovementDetector::~MovementDetector()
{

}

/**
	If the processor uses a custom editor, this method must be present.
*/

AudioProcessorEditor* MovementDetector::createEditor()
{
	editor = new MovementDetectorEditor(this, true);

	//std::cout << "Creating editor." << std::endl;

	return editor;
}


void MovementDetector::setParameter(int parameterIndex, float newValue)
{

    //Parameter& p =  parameters.getReference(parameterIndex);
    //p.setValue(newValue, 0);

    //threshold = newValue;

    //std::cout << float(p[0]) << std::endl;
    editor->updateParameterButtons(parameterIndex);
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

	std::cout << "\n Tamanho do buffer: " << bufferSize << "\n";
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
			//initialRMS += pow(currentSample,2);
			initialMean += currentSample;
			//fout << currentSample;
			//fout.close();	

			// Compute the RMS of 2s of data
            //initialRMS = sqrt(initialRMS/initialTimeWindow);

            //Compute the mean of 2s of data
			initialMean = (initialMean/initialTimeWindow);

			// Compute the the SD for 10s of data
			for(int j = 0; j < initialTimeWindow; j++)
			{
				standardDev += pow((initialWindow[j] - initialMean),2);
			}
			standardDev = sqrt(standardDev/initialTimeWindow);

            sdWindow = initialWindow; // copia o primeiro minuto em sdWindow

            std::cout<<"\n Desvio padrao: "<<standardDev<<"\n";
            std::cout<<"\n Media: "<<initialMean<<"\n";
			std::cout<<"\n sdWindow_size: "<<sdWindow.size()<<"\n";
		}

		else if((hasInitialStats == false) && (initialWindow.size() < (initialTimeWindow-1)))
		{
			initialWindow.add(currentSample);
			//initialRMS += pow(currentSample,2);
			initialMean += currentSample;
			//fout << currentSample << std::endl;
		}
	}
}

void MovementDetector::classifier(AudioSampleBuffer& buffer,
									MidiBuffer& events)
{
	for(int i = 0; i < bufferSize; i++)
	{	
		float currentSample = (float) buffer.getSample(0,i);

		// enche ampWindow com timeThreshold segundos
		// if(sdWindow.size() < timeThreshold)
  //       {
  //       	//ampWindow.add(pow(currentSample,2));
  //       	sdWindow.add(currentSample);
  //       }
        // else 
        // {
        	if(waitTime > 0)
        	{
	        	//ampWindow.set(k, pow(currentSample,2));
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

        		//float ampMean = sqrt(sumAmp/len); // assumindo que a média das amplitudes
        										  // na janela de tempo será o critério de
        										  // classificação
        		float winMean = (sumAmp/len);

        		for(int k = 0; k < len; k++)
        		{
        			winDev += pow((sdWindow[k] - winMean),2);
        		}

        		winDev = sqrt(winDev/len);

        		//std::cout << "initialRMS: " << (initialRMS) << "\n";
        		//std::cout << "ampMean: " << (ampMean) << "\n";
        		
        		std::cout<<"----------\n";
        		std::cout<<"std inicial: " << standardDev;
        		std::cout<<"\nstd janela: " << winDev;

                if(contador == 178)
                {
                    fout.close();
                }
                else
                {
                    fout << winDev << std::endl;
                    contador++;
                }
        		
                if(winDev <= 2.4*standardDev)
        		{
        			std::cout << "\ndormindo \n\n";

        			if(awake)
        			{
        				awake = false;
        				addEvent(events, TTL, 0, 1, 0);
        			}
        		}
        		else
        		{
        			std::cout << "\nacordado \n\n";

        			if(!awake) 
        			{
        				awake = true;
        				addEvent(events, TTL, 0, 0, 0);
        			}
        		}

        		waitTime = overlapTime;
        	}
        //} 
	}	
}

void MovementDetector::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{
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

        // Refazer o valor referência se pressionado o botao
        // if recalculateInitialStats.pressed -> 
        //     hasInitialStats = false;
        //     initialWindow.clear();
	}
}
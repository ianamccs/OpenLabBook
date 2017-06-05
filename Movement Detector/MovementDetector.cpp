
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

// float MovementDetector::getSquareAverage(float array, int size)
// {
// 	float mean = 0;
// 	for(int i = 0; i < size; i++)
// 	{
// 		mean += mean + pow(array[i],2);
// 	}

// 	return mean/size;
// }


void MovementDetector::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{

	int bufferSize = (int) buffer.getNumSamples();
	std::cout << "\n Tamanho do buffer: " << bufferSize << "\n";

	int bufferChannels = (int) buffer.getNumChannels();
	std::cout << "\n Canais do buffer: " << bufferChannels << "\n";

	// Fill an array with 2s of data 
	for(int i = 0; i < bufferSize; i++)
	{
		float currentSample = (float) buffer.getSample(0,i);

		if((hasEntered == false) && (initialWindow.size() == (60000-1)))
		{
			hasEntered == true;

			buffer.setSample(1,i,currentSample); //grava valor no canal 1

			// add the last term to initialMean
			initialWindow.add(currentSample);
			initialMean += currentSample;

			// Compute the mean of 2s of data
			initialMean = initialMean/60000;

			// Compute the the SD for 2s of data
			for(int i = 0; i < 60000; i++)
			{
				standardDev += pow((initialWindow[i] - initialMean),2);
			}
			standardDev = sqrt(standardDev/60000);
		}

		else if((hasEntered == false) && (initialWindow.size() < (60000-1)))
		{
			initialWindow.add(currentSample);
			initialMean += currentSample;

			buffer.setSample(1,i,currentSample); // grava valor no canal 1
		}
		
	}

	float windowMean = 0;
	for(int i = 0; i < bufferSize; i++)
	{
		windowMean += pow(buffer.getSample(0,i),2);
		
	}

	windowMean = windowMean/bufferSize;

	std::cout << "\n\n tamanho vetor: " << initialWindow.size() << "\n\n";
	std::cout << "\n\n media vetor: " << initialMean << "\n\n";
	std::cout << "\n\n std vetor: " << standardDev << "\n\n";

	// std::cout << "\n\n 20 valores finais: \n";
	// for(int i = 59999-20; i < 59999; i++) {
	// 	std::cout << "\n\n valor " << i << ": " << initialWindow[i] << "\n";
	// }
	
	// // Using 10 points to estimate the mean
 //    int sizeMeanWindow = 10;

 //    double arrSized = round(getNumSamples(module.inputChan)/sizeMeanWindow);
 //    int arrSize = (int) arrSized;
 //    float meanBlocks[arrSize];
 //    float sumPoints = 0;

 //    // create arrSize blocks with sizeMeanWindow points
 //    for (int i = 0; i < arrSize; i++)
 //    {           
 //        for (int j = 0; j < sizeMeanWindow; j++)
 //        {
 //            sumPoints += pow(buffer.getSample(module.inputChan,(i*sizeMeanWindow)+j),2);
 //        }

 //        meanBlocks[i] = (sumPoints/sizeMeanWindow);
 //    }
    

}

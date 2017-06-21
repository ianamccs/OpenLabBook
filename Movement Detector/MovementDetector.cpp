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
	fout.open("teste.txt");
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

void MovementDetector::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{
	if(bufferSize == 0) 
	{
		calculateBufferSize(buffer);
	}
	 
	for(int i = 0; i < bufferSize; i++)
	{
		float currentSample = (float) buffer.getSample(0,i);

		// Fill an array with initial 2s of data
		if((hasEntered == false) && (initialWindow.size() == (60000-1)))
		{
			hasEntered == true;

			// add the last term to initialMean
			initialWindow.add(currentSample);
			initialMean += currentSample;
			fout << currentSample;
			fout.close();	

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
			fout << currentSample << std::endl;
		}

		
		
		std::cout << "bufferSize: "<< bufferSize;
		std::cout << "\n\n tamanho vetor: " << initialWindow.size() << "\n\n";
		std::cout << "\n\n media vetor: " << initialMean << "\n\n";
		std::cout << "\n\n std vetor: " << standardDev << "\n\n";
	   
	    arrSize = floor(bufferSize/sizeMeanWindow);
	    
	    timeThreshold = timeThreshold/sizeMeanWindow;	    
	    

	    // // create arrSize blocks with sizeMeanWindow points
	    // for (int i = 0; i < arrSize; i++)
	    // {           
	    //     for (int j = 0; j < sizeMeanWindow; j++)
	    //     {
	    //         sumPoints += sqrt(pow(buffer.getSample(0,(i*sizeMeanWindow)+j),2));
	    //     }

	    //     if(meanAmp.size() > timeThreshold-1)
	    //     {

	    //     	/* inserir aqui lógica que trabalha a janela de tempo e faz a classificação */


	    //     	// substitui os 10s iniciais do vetor por novos 10s
	    //     	if(k >= floor((60-overlap)*30000/sizeMeanWindow)) // if k >= 10s
	    //     	{
	    //     		for(int l = 0; l < meanAmp.size();l++)
	    //     		{
	    //     			sumAmp += meanAmp[l];
	    //     		}

	    //     		windowMean = sumAmp/meanAmp.size(); // assumindo que a média das amplitudes
	    //     											// na janela de tempo será o critério de
	    //     											// classificação
	        		
	    //     		std::cout << "initialMean: " << (initialMean + 0.2*standardDev) << "\n";
	    //     		std::cout << "windowMean: " << (windowMean) << "\n";
	        		
	    //     		if(standardDev != 0)
	    //     		{
		   //      		if(windowMean <= (initialMean + 0.2*standardDev))
		   //      		{
		   //      			std::cout << "dormindo \n\n";
		   //      		}
		   //      		else if(windowMean > (initialMean + 0.2*standardDev))
		   //      		{
		   //      			std::cout << "acordado \n\n";
		   //      		}
	    //     		}
	
	    //     		k = 0;
	    //     	}

	    //     	meanAmp.insert(k, sumPoints/sizeMeanWindow);
	    //     	k++;

	    //     }

	    //     else 
	    //     {
	    //     	meanAmp.add(sumPoints/sizeMeanWindow);
	    //     }
	    // }	    
		
	}
}
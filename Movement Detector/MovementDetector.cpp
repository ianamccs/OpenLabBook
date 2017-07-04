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

		// Fill an array with initial 2s of data
		if((initialWindow.size() == (60000-1)) && (hasEntered == false))
		{
			hasEntered = true;

			// add the last term to initialMean
			initialWindow.add(currentSample);
			initialMean += pow(currentSample,2);
			//fout << currentSample;
			//fout.close();	

			// Compute the mean of 2s of data
			initialMean = sqrt(initialMean/60000);

			// Compute the the SD for 2s of data
			for(int j = 0; j < 60000; j++)
			{
				standardDev += pow((initialWindow[j] - initialMean),2);
			}
			standardDev = sqrt(standardDev/60000);

			std::cout<<"\n Desvio padrao: "<<standardDev<<"\n";
		}

		else if((hasEntered == false) && (initialWindow.size() < (60000-1)))
		{
			initialWindow.add(currentSample);
			initialMean += pow(currentSample,2);
			//fout << currentSample << std::endl;
		}
	}
}

void MovementDetector::classifier(AudioSampleBuffer& buffer)
{
	for(int i = 0; i < bufferSize; i++)
		{	
			float currentSample = (float) buffer.getSample(0,i);

			if(meanAmp.size() >= timeThreshold)
	        {

	        	/* inserir aqui lógica que trabalha a janela de tempo e faz a classificação */

	 	       	// substitui os 10s iniciais do vetor por novos 10s
	        	if(k == ((60-overlap)*30000)-1)
	        	{
	        		for(int l = 0; l < meanAmp.size();l++)
	        		{
	        			sumAmp += meanAmp[l];
	        		}

	        		windowMean = sqrt(sumAmp/meanAmp.size()); // assumindo que a média das amplitudes
	        											// na janela de tempo será o critério de
	        											// classificação
	        		sumAmp = 0;
	        		
	        		std::cout << "initialMean: " << (initialMean) << "\n";
	        		std::cout << "windowMean: " << (windowMean) << "\n";
	        		
	        		
	        		if(windowMean <= (initialMean))
	        		{
	        			std::cout << "dormindo \n\n";
	        		}
	        		else
	        		{
	        			std::cout << "acordado \n\n";
	        		}

	        		k = 0;
	        	}
	        	if(k < (60-overlap)*30000)
	        	{
		        	meanAmp.set(k,pow(currentSample,2));
		        	k++;
	        	}
	        }

	        else 
	        {
	        	meanAmp.add(pow(currentSample,2));
	        } 
		}	
}

void MovementDetector::process(AudioSampleBuffer& buffer,
                               MidiBuffer& events)
{
	if(bufferSize == 0) 
	{
		calculateBufferSize(buffer);
	}

	if(hasEntered == false)
	{
		calculateInitialStats(buffer);
	}

	else
	{
		classifier(buffer);
	}
}
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

#include "MovementDetectorEditor.h"
#include "MovementDetector.h"
#include "../../UI/LookAndFeel/MaterialButtonLookAndFeel.h"
#include "../../Processors/Parameter/ParameterEditor.h"

static const Colour COLOUR_PRIMARY (Colours::black.withAlpha (0.87f));
static const Colour COLOUR_ACCENT  (Colour::fromRGB (3, 169, 244));

MovementDetectorEditor::MovementDetectorEditor (GenericProcessor* parentProcessor, bool useDefaultParameterEditors)
    : GenericEditor (parentProcessor, useDefaultParameterEditors)
    , m_currentChannelsView          (Resting_CHANNELS)
    , m_channelSelectorButtonManager (new LinearButtonGroupManager)
    , m_gainSlider                   (new ParameterSlider (0.0, 100.0, 100.0, Font("Default", 13.f, Font::plain)))
{
    TextButton* RestingChannelsButton = new TextButton ("Resting", "Switch to resting channels");
    RestingChannelsButton->setClickingTogglesState (true);
    RestingChannelsButton->setToggleState (true, dontSendNotification);
    RestingChannelsButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    RestingChannelsButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    RestingChannelsButton->setColour (TextButton::textColourOffId,    COLOUR_PRIMARY);
    RestingChannelsButton->setColour (TextButton::textColourOnId,     COLOUR_ACCENT);

    TextButton* MovingChannelsButton  = new TextButton ("Moving", "Switch to moving channels");
    MovingChannelsButton->setClickingTogglesState (true);
    MovingChannelsButton->setColour (TextButton::buttonColourId,     Colour (0x0));
    MovingChannelsButton->setColour (TextButton::buttonOnColourId,   Colour (0x0));
    MovingChannelsButton->setColour (TextButton::textColourOffId,    COLOUR_PRIMARY);
    MovingChannelsButton->setColour (TextButton::textColourOnId,     COLOUR_ACCENT);

    m_channelSelectorButtonManager->addButton (RestingChannelsButton);
    m_channelSelectorButtonManager->addButton (MovingChannelsButton);
    m_channelSelectorButtonManager->setRadioButtonMode (true);
    m_channelSelectorButtonManager->setButtonListener (this);
    m_channelSelectorButtonManager->setButtonsLookAndFeel (m_materialButtonLookAndFeel);
    m_channelSelectorButtonManager->setColour (ButtonGroupManager::backgroundColourId,   Colours::white);
    m_channelSelectorButtonManager->setColour (ButtonGroupManager::outlineColourId,      Colour (0x0));
    m_channelSelectorButtonManager->setColour (LinearButtonGroupManager::accentColourId, COLOUR_ACCENT);
    addAndMakeVisible (m_channelSelectorButtonManager);

    m_gainSlider->setColour (Slider::rotarySliderFillColourId, Colour::fromRGB (255, 193, 7));
    m_gainSlider->setName ("PERCENTAGE");
    m_gainSlider->addListener (this);
    addAndMakeVisible (m_gainSlider);

    channelSelector->paramButtonsToggledByDefault (false);

    setDesiredWidth (280);
    
    inputLabel = new Label("Input label", "INPUT");
    inputLabel->setBounds(140,80,80,20);
    inputLabel->setFont(Font("Small Text", 10, Font::plain));
    inputLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(inputLabel);
    
    inputSelector = new ComboBox();
    inputSelector->setBounds(180,80,50,20);
    inputSelector->addItem("-",1);
    inputSelector->addListener(this);
    inputSelector->setSelectedId(1);
    addAndMakeVisible(inputSelector);
    
	gateLabel = new Label("Input label", "GATE");
    gateLabel->setBounds(140,105,80,20);
    gateLabel->setFont(Font("Small Text", 10, Font::plain));
    gateLabel->setColour(Label::textColourId, Colours::darkgrey);
    addAndMakeVisible(gateLabel);
    
    gateSelector = new ComboBox();
    gateSelector->setBounds(180,105,50,20);
    gateSelector->addItem("-",1);
    gateSelector->addListener(this);
    gateSelector->setSelectedId(1);
    addAndMakeVisible(gateSelector);

}

void MovementDetectorEditor::labelTextChanged(Label* label)
{

}

void MovementDetectorEditor::comboBoxChanged(ComboBox* c)
{

}

void MovementDetectorEditor::paint (Graphics& g)
{
    GenericEditor::paint (g);

    // Draw slider's label
    // ========================================================================
    g.setColour (Colours::darkgrey);
    g.setFont (Font("Default", 13.f, Font::plain));

    auto gainSliderBounds = m_gainSlider->getBounds();
    g.drawText (m_gainSlider->getName().toUpperCase(),
                gainSliderBounds.getX(), gainSliderBounds.getBottom() - 15,
                gainSliderBounds.getWidth(), 30,
                Justification::centred,
                false);
    // ========================================================================
}

void MovementDetectorEditor::resized()
{
    m_channelSelectorButtonManager->setBounds (110, 35, 150, 36);

    m_gainSlider->setBounds (15, 30, 80, 80);

    GenericEditor::resized();
}

void MovementDetectorEditor::buttonClicked (Button* buttonThatWasClicked)
{
    const String buttonName = buttonThatWasClicked->getName().toLowerCase();

    // "Resting channels" button clicked
    if (buttonName.startsWith ("Resting"))
    {
        channelSelector->setActiveChannels (static_cast<MovementDetector*> (getProcessor())->getRestingChannels());

        m_currentChannelsView = Resting_CHANNELS;
    }
    // "Moving channels" button clicked
    else if (buttonName.startsWith ("Moving"))
    {
        channelSelector->setActiveChannels (static_cast<MovementDetector*> (getProcessor())->getMovingChannels());

        m_currentChannelsView = Moving_CHANNELS;
    }

    GenericEditor::buttonClicked (buttonThatWasClicked);
}

void MovementDetectorEditor::channelChanged (int channel, bool newState)
{
    auto processor = static_cast<MovementDetector*> (getProcessor());
    if (m_currentChannelsView == Resting_CHANNELS)
    {
        processor->setRestingChannelState (channel, newState);
    }
    else
    {
        processor->setMovingChannelState (channel, newState);
    }
}

void MovementDetectorEditor::sliderEvent (Slider* sliderWhichValueHasChanged)
{
    auto processor = static_cast<MovementDetector*> (getProcessor());

    processor->setGainLevel ( (float)sliderWhichValueHasChanged->getValue());
}


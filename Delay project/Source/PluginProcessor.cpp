/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayProjectAudioProcessor::DelayProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    //======================================================================================================
    //PARAMETERS
    
    addParameter(mDryWetParameter = new AudioParameterFloat("drywet",
                                                  "Dry Wet",
                                                   0.0,
                                                   1.0,
                                                  0.5));
    
    
    addParameter(mFeedbackParameter = new AudioParameterFloat("feedback",
                                                              "Feedback",
                                                              0,
                                                              0.98,
                                                              0.5));
    addParameter(mDelayTimeParameter = new AudioParameterFloat("delaytime",
                                                             "Delay TIme",
                                                             0.01,
                                                             MAX_DELAY_TIME,
                                                             0.5));
    
   
   //======================================================================================================
    
    
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    mDelayTimeInSamples = 0;
    mDelayReadHead = 0;
    mFeedbackLeft = 0;
    mFeedbackRight = 0;
    mDelayTimeSmoothed = 0;
   // mDryWet = 0.5; dont need any more b/c we made a parameter
    
}

DelayProjectAudioProcessor::~DelayProjectAudioProcessor()
{
//    if (mCircularBufferLeft != nullptr){
//       delete [] mCircularBufferLeft;
//        mCircularBufferLeft = nullptr;
//    }
//    if (mCircularBufferRight != nullptr){
//        delete [] mCircularBufferRight;
//        mCircularBufferRight = nullptr;
//    }
}

//==============================================================================
const String DelayProjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayProjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayProjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayProjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayProjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayProjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayProjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayProjectAudioProcessor::setCurrentProgram (int index)
{
}

const String DelayProjectAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayProjectAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DelayProjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mDelayTimeInSamples = sampleRate * *mDelayTimeParameter;
    
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
//    if (mCircularBufferLeft == nullptr){
//        mCircularBufferLeft = new float[(int)(mCircularBufferLength)];
//    }
    
    
//    if (mCircularBufferRight == nullptr){
//        mCircularBufferRight = new float[(int)(mCircularBufferLength)];
//    }
    
    mCircularBufferLeft.reset(new float[mCircularBufferLength]);
    mCircularBufferRight.reset(new float[mCircularBufferLength]);
    
    mCircularBufferWriteHead = 0;
    mDelayTimeSmoothed = *mDelayTimeParameter;
    
}

void DelayProjectAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayProjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif
//==================================================================================================
//PROCESSING BLOCK!
void DelayProjectAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    
    
    
    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        
        mDelayTimeSmoothed = mDelayTimeSmoothed - 0.001 * (mDelayTimeSmoothed - *mDelayTimeParameter);
        
        
        mDelayTimeInSamples = getSampleRate() * mDelayTimeSmoothed; //Parameter for Delay Time!
        
        
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
        
        mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;
       
//=========================================================================================================
        if (mDelayReadHead < 0){                        //check if readHead is less than 0 if so it index is out
            mDelayReadHead += mCircularBufferLength;    // of bounds and it is corrected!
        }
//==========================================================================================================
        
//        float delay_sample_left = mCircularBufferLeft[(int)mDelayReadHead];
//        float delay_sample_right = mCircularBufferRight[(int)mDelayReadHead];
        
//        buffer.addSample(0, i, mCircularBufferLeft[(int)mDelayReadHead]);
//        buffer.addSample(1, i, mCircularBufferRight[(int)mDelayReadHead]);
//
//============================================================================================================
        /*
         
         this block of code is for delay time and we are interpulating the values!
         
         */
        int readHead_x = (int)mDelayReadHead;
        int readHead_x1 =  readHead_x + 1;
        
        float readHeadFloat = mDelayReadHead - readHead_x;
        
        
        if (readHead_x1 >= mCircularBufferLength){ //checks if x1 is equal to or greater the buffer lenght
            readHead_x1 -= mCircularBufferLength; //if true x1 is decreased index.
        }
        
        float delay_sample_left = lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);
        
//================================================================================================================
        //FEEDBACK KNOB!
        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;
//=================================================================================================================
        mCircularBufferWriteHead++; //moves index of circular buffer
        
        
//==================================================================================================================
        //DRY/WET KNOB!
        buffer.setSample(0, i, buffer.getSample(0, i) * (1 - *mDryWetParameter) + delay_sample_left * *mDryWetParameter );
        buffer.setSample(1, i, buffer.getSample(1, i) * (1 - *mDryWetParameter) + delay_sample_left * *mDryWetParameter );
        
        //checks if writehead is greater if so it initializes writehead back to 0.
        if (mCircularBufferLength <= mCircularBufferWriteHead){
            mCircularBufferWriteHead = 0;
        }
        // ..do something to the data...
    }
}

//==============================================================================
bool DelayProjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DelayProjectAudioProcessor::createEditor()
{
    return new DelayProjectAudioProcessorEditor (*this);
}

//==============================================================================
void DelayProjectAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayProjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayProjectAudioProcessor();
}

float DelayProjectAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;

}

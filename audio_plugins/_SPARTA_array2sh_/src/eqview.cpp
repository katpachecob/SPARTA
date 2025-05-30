/*
 ==============================================================================
 
 This file is part of SPARTA; a suite of spatial audio plug-ins.
 Copyright (c) 2018 - Leo McCormack.
 
 SPARTA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 SPARTA is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with SPARTA.  If not, see <http://www.gnu.org/licenses/>.
 
 ==============================================================================
*/

#include "eqview.h"

const int border_pixels_top = 10;
const int border_pixels_bottom = 35;
const int border_pixels_left = 45;
const int border_pixels_right = 10;
const String ylabel = String("Magnitude (dB)");
const String xlabel = String("Frequency (Hz)");
const int axis_dB_label_interval = 30; /* dB per label */


static int round_nearest(int number, int multiple) {
    int result = number + multiple/2;
    result -= result % multiple;
    return result;
}

eqview::eqview (int _width, int _height, float _min_freq, float _max_freq, float _min_dB, float _max_dB, float _fs)
{
    setSize (_width, _height);

    eqview_windowIncluded.reset (new eqview_window(_width-(border_pixels_left+border_pixels_right),
                                                   _height-(border_pixels_top+border_pixels_bottom),
                                                   _min_freq, _max_freq,
                                                   _min_dB, _max_dB, _fs)); /* TODO: switch to the more general "anaview"  */
    addAndMakeVisible (eqview_windowIncluded.get());
    eqview_windowIncluded->setAlwaysOnTop(true);
    eqview_windowIncluded->setTopLeftPosition(border_pixels_left, border_pixels_top );

    width = _width;
    height =_height;
    min_freq = _min_freq;
    max_freq = _max_freq;
    min_dB = _min_dB;
    max_dB = _max_dB;
    fs = _fs;
}

eqview::~eqview()
{
}

void eqview::paint (juce::Graphics& g)
{
    localBounds = getBounds();
    int pow10;
    int min_pow10, max_pow10;
    float val, freq;
    float c = log10f(min_freq);
    float m = (log10f(max_freq) - c)/(float)eqview_windowIncluded->getWidth();

    /* round specified frequency limits to 10^x for labels */
    for (pow10 = 0; pow10<7; pow10++){
        val =  powf(10.0f, (float)pow10);
        if ( ((min_freq/ val) <= val +0.001f) && (min_freq >=val-0.001f) ){
            min_pow10 = pow10;
        }
        if ( ((max_freq/ val) <= val) && (max_freq >=val) ){
            max_pow10 = pow10;
        }
    }

    /* draw frequency labels for each 10^x Hz */
    int textWidth = 60;
    int textHeight = 25;
    g.setColour(Colours::white);
    for (pow10 = min_pow10; pow10<=max_pow10; pow10++){
        freq = powf(10.0f, (float)pow10);
        float Xpixel = (log10f(freq) - c)/m;
        if (freq > 999.0f){
            g.drawText(String((int)((freq/1000.0f)+0.05f)) + String("k"), border_pixels_left+Xpixel-textWidth/2,
                       border_pixels_top + eqview_windowIncluded->getHeight(), textWidth, textHeight, Justification::centred);
        }
        else {
        g.drawText(String((int)freq), border_pixels_left+Xpixel-textWidth/2,
                   border_pixels_top + eqview_windowIncluded->getHeight(), textWidth, textHeight, Justification::centred);
        }
    }

    /* draw Magnitude labels */
    textWidth = 60;
    textHeight = 25;
    float start_dB= round_nearest(min_dB<0 ? min_dB-axis_dB_label_interval : min_dB, axis_dB_label_interval);
    float end_dB= round_nearest(max_dB, axis_dB_label_interval);
    for (int i = start_dB; i<=end_dB; i+=axis_dB_label_interval){
        float Ypixel = ((float)i-min_dB)* ((float)eqview_windowIncluded->getHeight()/(max_dB-min_dB));

        if ((i>min_dB-1) && (i< max_dB+1)){
            g.drawText(String(i), (int)(3.0f*(float)border_pixels_left/4.0f) - textWidth/2,
                       border_pixels_top + (eqview_windowIncluded->getHeight()-Ypixel) - textHeight/2,
                       textWidth, textHeight, Justification::centred);
        }
    }

    /* Axis labels */
    textWidth = 100;
    textHeight = 50;
    g.drawText(xlabel, (eqview_windowIncluded->getWidth())/2+border_pixels_left - textWidth/2,
               eqview_windowIncluded->getHeight() + border_pixels_top,
               textWidth, textHeight, Justification::centred);

    // Guessed this one...
    g.addTransform(AffineTransform::rotation(-M_PI/2).followedBy(AffineTransform::translation(-(localBounds.getHeight()/2 - border_pixels_top - (int)((float)textWidth/2.5f)),
                                                                                                localBounds.getHeight()- border_pixels_top - textWidth/2)));
    g.drawText(ylabel, 0,
               localBounds.getHeight()/2 - border_pixels_top - textWidth/2,
               textWidth, 50, Justification::centred);
    g.addTransform(AffineTransform());

    /* draw curves */
    //eqview_windowIncluded->repaint(); /* No need to call, as eqview will repaint any children too */
}

void eqview::resized()
{
}

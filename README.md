# OBS Mixed Reality Fake Lighting

This plugin was generated from the OBS plugin template, and is meant to provide an in-OBS way of simply applying an environmental lighting effect to any source. The intended application is to colour the webcam input of a mixed reality scene to light the subject in a way more sympathetic to the scene that they are in. 

The plugin works by creating a blurred version of a lighting source (typically the scene background), and then applying that to your source. There are options to make the blurring more or less accurate depending on your GPU headroom.

## Prerequisites
OBS Minimum version 28.0.0.

## Usage

Apply as a filter to the image source you want to re-light. The first filter option allows you to select the lighting source (which for a mixed reality application would be your background). Other options allow you to trade off performance for blur accuracy, and how intense the effect is. Finally there is an option to increase contrast, as the fake lighting can wash out the input source.

## Building from source

Nothing exotic going on here, so this can be built using the scripts in the scripts folder.
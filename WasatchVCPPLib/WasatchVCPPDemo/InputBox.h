/**
    @file InputBox.h
    @see  https://stackoverflow.com/a/52808263/11615696
*/
#pragma once

extern "C" char *InputBox(char *Prompt, char *Title = (char *)"", char *Default = (char *)"");
extern "C" char *PasswordBox(char *Prompt, char *Title = (char *)"", char *Default = (char *)"");

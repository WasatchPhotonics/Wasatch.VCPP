/**
    @file InputBox.h
    @see  https://stackoverflow.com/a/52808263/11615696
*/
#pragma once

#include <string>

std::string InputBox(const char *Prompt, const char *Title = "", const char *Default = "");
std::string PasswordBox(const char *Prompt, const char *Title = "", const char *Default = "");

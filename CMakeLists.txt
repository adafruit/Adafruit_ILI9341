# Adafruit ILI9341 Library
# MIT License

cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "Adafruit_ILI9341.cpp" INCLUDE_DIRS "." REQUIRES Adafruit-GFX-Library arduino-esp32)

project(Adafruit_ILI9341)

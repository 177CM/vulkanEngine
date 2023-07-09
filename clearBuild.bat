@echo off
if exist build% (
    rmdir /s /q build
)

cd shaders
if exist spv (
    rmdir /s /q spv
)
cd ..
cls

set path=C:\Program Files (x86)\WiX Toolset v3.7\bin;%path%

candle.exe setup.wxs
light.exe -ext WixUIExtension setup.wixobj

pause
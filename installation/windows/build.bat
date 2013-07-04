set path=C:\Program Files (x86)\WiX Toolset v3.7\bin;%path%

candle.exe SampleWixUI.wxs
light.exe -ext WixUIExtension SampleWixUI.wixobj

pause
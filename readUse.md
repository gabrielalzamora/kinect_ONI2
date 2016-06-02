# k_ver

pay attention to libraries and include stuff referenced at the end.

1-select sensor.
	if there's more than one the combobox will contain from 0 to number-1 reference to connected kinects.
	select one.
	=0 by default.
	"no device" if none connected.

2-push 'start sensor'.
	initialize context (related to usb), device, streams, buffers and flags.

3-push 'whatever you want to see'.
	if everything was ok push buttons of started streams will activate allowing to start show.

4-push 'stop bucle'
	to stop image flow (originaly to debug program)

5-when bored push x on upper left corner to close window.


references:

QT
https://wiki.qt.io/Install_Qt_5_on_Ubuntu

take a look to .pro file to modify depending on where do you hide libs & includes

OpenNI
http://structure.io/openni
https://github.com/occipital/OpenNI2

OpenKinect
http://openkinect.org/wiki/Getting_Started

last one is for OpenNI2-FreenectDriver ./OpenNI2/Driver/libFreenectDriver.so
to be included on every project (but not extensively tested by me)

Anyway there are many tutorials to install kinnect Linux/Win/Mac

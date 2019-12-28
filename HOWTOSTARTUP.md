begin Warning!!!! 
	Do NOT hit "DEBUG/CONTINUE"/"the red play button in codeblocks" if do not want to overwrite your current firmware! 
	The project is setup in such a way, that if "Compile and Upload on Debug" is selected instead of "Just Compile" it will automatically flash the firmware, when you start the debugger.
end   Warning!!!!

begin Warning!!!! 
	The project is not yet finished, check the current status in the readme.md and see if your essential features are supported yet.
end   Warning!!!!


The normal "Compile" Button's behavior (gear symbol) is unaffected by this.

Setup your hardware according to the automatic_flashing.md

Then run START_codeblocks_portable.bat, which will launch CodeBlocks in a portable manner and will setup the project's compiler/debugger options correctely.
A terminal window will get created which is from the GNU debugger used for flashing. 
		If this window disappears you don't have installed the driver or the device is not attached or another error has ocurred.
		In this case start "cmd" and drag and drop the "START_debugger_manually" script into the command line and watch for errors.
	
	
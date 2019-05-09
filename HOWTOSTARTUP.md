Warning! Do NOT hit compile if don't want to overwrite your current firmware! The project is setup in such a way that if
"Compile and Upload" is selected instead of "Just Compile" it will automatically flash the firmware when you have your programmer attached.
The normal "Compile" Button's behavior (gear symbol) is altered depending on the selected config from the dropdown, so if you have "Compile and Upload" selected it will overwrite the existing firmware without warning.

Run START_codeblocks_portable.bat, which will launch CodeBlocks in a portable manner and will setup the project's compiler/debugger options correctely. A terminal window will get created which is from the GNU debugger used for flashing.
Just leave it in the background.

rem see: http://wiki.codeblocks.org/index.php/FAQ-Settings and Q: Where does C::B's configuration file store? How do I make Code::Blocks portable?
set userdata=%~dp0CodeBlocks1908usersettings
mkdir %userdata%
START %~dp0CodeBlocks1908\codeblocks.exe --user-data-dir=%userdata% --prefix=%~dp0Codeblocks1908 --file=%~dp0firmware_c\ty71m.cbp
CALL START_debugger_manually.bat
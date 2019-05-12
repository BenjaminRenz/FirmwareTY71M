rem see: http://wiki.codeblocks.org/index.php/FAQ-Settings and Q: Where does C::B's configuration file store? How do I make Code::Blocks portable?
set userdata=%~dp0CodeBlocks1712usersettings
mkdir %userdata%
START %~dp0CodeBlocks1712\codeblocks.exe --user-data-dir=%userdata% --prefix=%~dp0Codeblocks1712 --file=%~dp0firmware_c\ty71m.cbp

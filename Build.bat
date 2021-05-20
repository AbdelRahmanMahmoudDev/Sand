@echo off

set CommonCompilerFlags=-DSAND_DEBUG=1 -DSAND_INTERNAL=1 -DSAND_WIN32=1 -Gm- -MT -nologo -EHa- -GR- -Od -Oi -WX -W4 -wd4201 -wd4100 -FC -Z7 
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib Gdi32.lib Winmm.lib

IF NOT EXIST build mkdir build
pushd build
cl %CommonCompilerFlags%  ..\src\sand\Win32EntryPoint.cpp -FmWin32EntryPoint.map /link %CommonLinkerFlags%
cl %CommonCompilerFlags%  ..\src\sand\Game.cpp -FmGame.map /LD /link /EXPORT:GameGenerateAudio /EXPORT:GameUpdate
popd
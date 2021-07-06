@echo off

set CommonCompilerFlags=-DSAND_DEBUG=1 -DSAND_INTERNAL=1 -DSAND_WIN32=1 -Gm- -MTd -nologo -EHa- -GR- -Od -Oi -WX -W4 -wd4505 -wd4201 -wd4100 -wd4101 -wd4189 -FC -Z7 
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib Gdi32.lib Winmm.lib

IF NOT EXIST build mkdir build
pushd build
del *.pdb > NUL 2> NUL
cl %CommonCompilerFlags%  ..\src\sand\Game.cpp -FmGame.map -LD /link -incremental:no -opt:ref -PDB:Game_%Random%.pdb /EXPORT:GameGenerateAudio /EXPORT:GameUpdate
cl %CommonCompilerFlags%  ..\src\sand\Win32EntryPoint.cpp -FmWin32EntryPoint.map /link %CommonLinkerFlags%
popd
@echo off
rem Epic Games Launcher 없이 Visual Studio 프로젝트 파일(.sln)을 생성한다.
rem 소스 파일(.h/.cpp)을 추가하거나 삭제한 뒤 다시 실행하면 된다.
chcp 65001 > nul
call "C:\Program Files\Epic Games\UE_5.5\Engine\Build\BatchFiles\Build.bat" -projectfiles -project="%~dp0NumBaseball.uproject" -game -rocket -progress
pause

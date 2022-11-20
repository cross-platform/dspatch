@echo off

cd %~dp0..

call :processDir
goto :finish

:processDir
for %%f in ("*.cpp", "*.h") do (
	clang-format --style=file --verbose -i %%f
)

for /D %%d in (*) do (
	cd %%d
	if not %%d==build if not %%d==builddir if not %%d==subprojects call :processDir
	cd ..
)

exit /b

:finish

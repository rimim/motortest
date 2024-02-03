all: motortest.cpp
	g++ motortest.cpp -Iinclude -Llib -lUnitreeMotorSDK_Linux64 -Xlinker -rpath -Xlinker '$$'ORIGIN/lib -lm -o motortest

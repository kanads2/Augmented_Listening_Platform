CXX = g++
FLAGS = -I/usr/local/Cellar/python/3.7.7/Frameworks/Python.framework/Versions/3.7/include/python3.7m -I/usr/local/Cellar/python/3.7.7/Frameworks/Python.framework/Versions/3.7/include/python3.7m -Wno-unused-result -Wsign-compare -Wunreachable-code -fno-common -dynamic -DNDEBUG -g -fwrapv -O3 -Wall -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk -I/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include -I/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/System/Library/Frameworks/Tk.framework/Versions/8.5/Headers -I/usr/local/lib/python3.7/site-packages/numpy/core/include -L/usr/local/opt/python/Frameworks/Python.framework/Versions/3.7/lib/python3.7/config-3.7m-darwin -lpython3.7m -ldl -framework CoreFoundation --debug=True 

beam: beam_wrap.cpp
	$(CXX) $(FLAGS) $? -o $@

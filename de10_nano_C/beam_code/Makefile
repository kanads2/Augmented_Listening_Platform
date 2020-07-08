CXX = clang++
FLAGS = -I/usr/include/python3.7m -I/usr/include/python3.7m -I/home/john/.local/lib/python3.7/site-packages/numpy/core/include -Wno-unused-result -Wsign-compare -g -fdebug-prefix-map=/build/python3.7-KiiRVh/python3.7-3.7.3=. -specs=/usr/share/dpkg/no-pie-compile.specs -fstack-protector -Wformat -Werror=format-security  -DNDEBUG -g -fwrapv -O3 -Wall \
	-L/usr/lib/python3.7/config-3.7m-x86_64-linux-gnu -L/usr/lib -lpython3.7m -lcrypt -lpthread -ldl  -lutil -lm  -Xlinker -export-dynamic -Wl,-O1 -Wl,-Bsymbolic-functions 

beam: beam_wrap.cpp
	$(CXX) $(FLAGS) $? -o $@

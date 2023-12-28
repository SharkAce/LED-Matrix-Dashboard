LIB=lib/librgbmatrix.a
OPT=-lcurl

bin: main.cpp
	$(CXX) $^ -o $@ $(LIB) $(OPT)

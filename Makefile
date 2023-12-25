LIB=lib/librgbmatrix.a


bin: main.cpp
	$(CXX) $^ -o $@ $(LIB)

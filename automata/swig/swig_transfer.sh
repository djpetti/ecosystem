# Make output directory
mkdir -p $1/swig_modules

# Copy output files.
cp $1/build/out/Default/lib/libswig_automata.so $1/swig_modules/_automata.so
cp automata.py $1/swig_modules/

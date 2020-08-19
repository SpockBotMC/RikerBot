# Remove the pre-installed cmake from path
PATH=`echo ${PATH} | awk -v RS=: -v ORS=: '/cmake/ {next} {print}'`
# Cleanup some weird whitespace issues
PATH={$PATH%[:space:]*}
CC=gcc-10
CXX=g++-10
cmake . -G Ninja
cmake --build . --target rikerbot_all
python setup.py bdist_wheel

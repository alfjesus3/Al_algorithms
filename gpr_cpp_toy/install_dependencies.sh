#!/bin/bash

set -e

sudo apt update
sudo apt install -y libboost-all-dev
sudo apt install -y libeigen3-dev
sudo apt-get install libgtest-dev


ROOT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
echo $ROOT_DIR
SOURCE_DIR=$ROOT_DIR/deps

nProcs=2

mkdir -p $SOURCE_DIR
cd $SOURCE_DIR


folder_boost="boost"
if [ ! -d "$folder_boost" ]; then   
    wget --no-verbose --show-progress --progress=bar:force https://boostorg.jfrog.io/artifactory/main/release/1.74.0/source/boost_1_74_0.tar.gz
    tar -xf boost_1_74_0.tar.gz
    rm boost_1_74_0.tar.gz
    
    mkdir "$folder_boost"
    
    cd boost_1_74_0
        ./bootstrap.sh --prefix=$SOURCE_DIR/$folder_boost
        ./b2 --with=all -j$nProcs install
    cd ..
    rm -rfv boost_1_74_0
fi

folder_eigen="eigen"
if [ ! -d "$folder_eigen" ]; then
    repo_url="https://gitlab.com/libeigen/eigen.git"
    git clone "$repo_url" "$folder_eigen"
    cd "$folder_eigen"
        mkdir -p build
        cd build
            cmake .. -DBUILD_TESTING=Off 
            make -j$nProcs
            sudo make install
        cd ..
    cd ..
fi

folder_nlopt="nlopt"
if [ ! -d "$folder_nlopt" ]; then
    wget http://members.loria.fr/JBMouret/mirrors/nlopt-2.4.2.tar.gz
    tar -xf nlopt-2.4.2.tar.gz
    rm nlopt-2.4.2.tar.gz

    mv nlopt-2.4.2 $folder_nlopt
    cd "$folder_nlopt"
        ./configure --with-cxx --enable-shared --without-python --without-matlab --without-octave
        sudo make install
    cd ..
fi

folder_gtest=/usr/src/gtest
if [ ! -d "$folder_gtest"/build ]; then
    cd "$folder_gtest"
        sudo mkdir -p build
            cd build
            sudo cmake ..
            sudo make
            # sudo cp *.a /usr/lib
        cd ..
fi
cd $SOURCE_DIR

folder_libcmaes="libcmaes"
if [ ! -d "$folder_libcmaes" ]; then
    repo_url="https://github.com/resibots/libcmaes.git"
    git clone "$repo_url" "$folder_libcmaes"
    cd "$folder_libcmaes"
        git checkout fix_flags_native
        mkdir -p build
        cd build
            cmake .. -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3
            make -j$nProcs
        cd ..
    cd ..
fi

folder_limbo="limbo"
if [ ! -d "$folder_limbo" ]; then
    repo_url="https://github.com/resibots/limbo.git"
    git clone "$repo_url" "$folder_limbo"
fi 

folder_plt="matplotlib-cpp"
if [ ! -d "$folder_plt" ]; then
    repo_url="https://github.com/lava/matplotlib-cpp.git"
    git clone "$repo_url" "$folder_plt"
    cd "$folder_plt"
        mkdir -p build
        cd build
            cmake .. -DBUILD_TESTING=Off  # -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR
            make -j$nProcs
            sudo make install
        cd ..
    cd ..
fi 

# folder_nlohmann="nlohmann"
# if [ ! -d "$folder_nlohmann" ]; then
#     repo_url="https://github.com/nlohmann/json.git"
#     git clone "$repo_url" "$folder_nlohmann"
#     cd "$folder_nlohmann"
#         mkdir -p build
#         mkdir -p lib
#         cd build
#             cmake .. -DCMAKE_INSTALL_PREFIX=$(realpath ../lib) 
#             make -j$nProcs
#             make install
#         cd ..
#     cd ..
# fi


cd $ROOT_DIR

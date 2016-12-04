# CEGIS

Counter-example guided inductive synthesis (CEGIS) implementation for the SMT solver
[Z3](https://github.com/Z3Prover/z3) by Microsoft Research. CEGIS is an approach to tackle
complexity of exact syntheses using satisfiability solvers. It was first introduced by
*Armando Solar-Lezama* in his Ph.D. Thesis
[Program Synthesis by Sketching](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.207.9048&rep=rep1&type=pdf)
back in 2008.

## Building

Git, g++, cmake, a Python interpreter and the Boost Library are necessary in order to build CEGIS. In *Ubuntu* the packages can be installed with

```sh
sudo apt-get install git g++ cmake python libboost-all-dev
```

Afterwards, CEGIS is ready to be installed.

```sh
git clone https://github.com/marcelwa/CEGIS.git
cd CEGIS
mkdir build
cd build
cmake ..
make
sudo make install
```

Since Z3 is automatically download, configured and installed during this procedure, this step may take a while. Pleaser enter your super user password to provide installation rights.

The standard install prefix is `/usr/local`. If another prefix is desired, just replace `cmake ..` by `cmake -DCMAKE_INSTALL_PREFIX:PATH=/YOUR_PREFIX ..`
